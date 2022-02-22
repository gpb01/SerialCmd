/*
   SerialCmd - A Wiring/Arduino library to tokenize and parse commands
   received over a phisical/software serial port and optimized to run
   also on ATtiny series.

   Copyright (C) 2013 - 2022 Guglielmo Braguglia

   Based on the SerialCommand library :
      Copyright (C) 2012 Stefan Rado
      Copyright (C) 2011 Steven Cogswell <steven.cogswell@gmail.com>
                         http://husks.wordpress.com

   Version 20220112

   Please note:

   1. Adjust the #define(s) following your requirements :
      Use the real necessary values for SERIALCMD_MAXCMDNUM, SERIALCMD_MAXCMDLNG
      and SERIALCMD_MAXBUFFER to minimize the memory usage.

   2. Allowed string terminator from serial are:
        SERIALCMD_CR                 : Carriage Return (0x0D - char - default)
        SERIALCMD_LF                 : Line Feed       (0x0A - char)
        SERIALCMD_NULL               : NULL            (0x00 - char)

   3. Allowed command source parameter are:
        SERIALCMD_FROMSTRING (or -1) : valid only as ReadString command
        SERIALCMD_FROMALL            : always valid - default
        SERIALCMD_FROMSERIAL         : valid only as ReadSer command

   4. You MUST initialize the serial port (phisical or virtual) on your Setup()
      and pass the Stream as parameter to the class constructor.

   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   This library is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SerialCmd.h"

/*
   --- Private methods ---
*/

void SerialCmd::ClearBuffer() {
	memset (SerialCmd_Buffer, 0x00, SERIALCMD_MAXBUFFER + 1);
   SerialCmd_BufferIdx = 0;
}

void SerialCmd::ConvertUC() {
   for ( uint8_t i = 0; i < strlen ( SerialCmd_Command ); i++ ) {
      if ( ( SerialCmd_Command[i] >= 'a' ) && ( SerialCmd_Command[i] <= 'z' ) ) {
         SerialCmd_Command[i] -= 32;
      }
   }
}

void SerialCmd::ReadStringCommon () {
   SerialCmd_Command = strtok_r ( SerialCmd_Buffer, SerialCmd_Sep, &SerialCmd_Last );
   SerialCmd_Found = 0;
   if ( SerialCmd_Command != NULL ) {
      for ( SerialCmd_Idx = 0; SerialCmd_Idx < SerialCmd_CmdCount; SerialCmd_Idx++ ) {
         if ( strncmp ( SerialCmd_Command, SerialCmd_CmdList[SerialCmd_Idx].command, SERIALCMD_MAXCMDLNG ) == 0 ) {
            if ( SerialCmd_CmdList[SerialCmd_Idx].allowedSource <= 0 ) {
               ( *SerialCmd_CmdList[SerialCmd_Idx].function ) ();
               SerialCmd_Found = 1;
               break;
            }
         }
      }
   }
   ClearBuffer();
}

/*
   --- Public methods ---
*/

SerialCmd::SerialCmd ( Stream &mySerial, char TermCh, char * SepCh ) {
   ClearBuffer();
   SerialCmd_CmdCount = 0;
   theSerial = &mySerial;
   SerialCmd_Term = TermCh;
   strlcpy ( SerialCmd_Sep, SepCh, 2 );
}

uint8_t SerialCmd::AddCmd ( const char *command, char allowedSource, void ( *function ) () ) {
   if ( SerialCmd_CmdCount < SERIALCMD_MAXCMDNUM ) {
      strncpy ( SerialCmd_CmdList[SerialCmd_CmdCount].command, command, SERIALCMD_MAXCMDLNG );
      SerialCmd_CmdList[SerialCmd_CmdCount].allowedSource = allowedSource;
      SerialCmd_CmdList[SerialCmd_CmdCount].function = function;
      SerialCmd_CmdCount++;
      return 1;
   } else {
      return 0;
   }
}

#ifdef __AVR__
uint8_t SerialCmd::AddCmd ( const __FlashStringHelper *command, char allowedSource, void ( *function ) () ) {
   if ( SerialCmd_CmdCount < SERIALCMD_MAXCMDNUM ) {
      strncpy_P ( SerialCmd_CmdList[SerialCmd_CmdCount].command, ( const char* ) command, SERIALCMD_MAXCMDLNG );
      SerialCmd_CmdList[SerialCmd_CmdCount].allowedSource = allowedSource;
      SerialCmd_CmdList[SerialCmd_CmdCount].function = function;
      SerialCmd_CmdCount++;
      return 1;
   } else {
      return 0;
   }
}
#endif

int8_t SerialCmd::ReadSer() {
	SerialCmd_Found = -1;
   while ( theSerial->available() > 0 ) {
      SerialCmd_InChar = theSerial->read();
      if ( SerialCmd_InChar == SerialCmd_Term ) {
         SerialCmd_Command = strtok_r ( SerialCmd_Buffer, SerialCmd_Sep, &SerialCmd_Last );
         SerialCmd_Found = 0;
         if ( SerialCmd_Command != NULL ) {
            if ( SERIALCMD_FORCEUC ) ConvertUC();
            for ( SerialCmd_Idx = 0; SerialCmd_Idx < SerialCmd_CmdCount; SerialCmd_Idx++ ) {
               if ( strncmp ( SerialCmd_Command, SerialCmd_CmdList[SerialCmd_Idx].command, SERIALCMD_MAXCMDLNG ) == 0 ) {
                  if ( SerialCmd_CmdList[SerialCmd_Idx].allowedSource >= 0 ) {
                     ( *SerialCmd_CmdList[SerialCmd_Idx].function ) ();
                     SerialCmd_Found = 1;
                     break;
                  }
               }
            }
         }
         ClearBuffer();
      } else {
         if ( SerialCmd_BufferIdx < SERIALCMD_MAXBUFFER ) {
				if ( (SerialCmd_InChar != SERIALCMD_CR) && (SerialCmd_InChar != SERIALCMD_LF) ) {
               SerialCmd_Buffer[SerialCmd_BufferIdx++] = SerialCmd_InChar;
               SerialCmd_Buffer[SerialCmd_BufferIdx] = 0x00;
				}
         } else {
            ClearBuffer();
         }
      }
   }
	return SerialCmd_Found;
}

int8_t SerialCmd::ReadString ( char * theCmd ) {
   if ( strlen ( theCmd ) >= SERIALCMD_MAXBUFFER ) return 0;
   //
   strcpy ( SerialCmd_Buffer, theCmd );
   ReadStringCommon();
   return SerialCmd_Found;
}

#ifdef __AVR__
int8_t SerialCmd::ReadString ( const __FlashStringHelper * theCmd ) {
   if ( strlen_P ( ( const char* ) theCmd ) >= SERIALCMD_MAXBUFFER ) return 0;
   //
   strcpy_P ( SerialCmd_Buffer, ( const char* ) theCmd );
   ReadStringCommon();
   return SerialCmd_Found;
}
#endif

char * SerialCmd::ReadNext() {
   return strtok_r ( NULL, SerialCmd_Sep, &SerialCmd_Last );
}

void SerialCmd::Print ( String &theClassString ) {
   theSerial->write ( theClassString.c_str(), theClassString.length() );
}

void SerialCmd::Print ( char theString[] ) {
   if ( ( theSerial ) )
      theSerial->write ( theString );
}

#ifdef __AVR__
void SerialCmd::Print ( const __FlashStringHelper * theString ) {
   if ( ( theSerial ) )
      theSerial->print ( theString );
}
#endif

void SerialCmd::Print ( char theChar ) {
   if ( ( theSerial ) )
      theSerial->write ( theChar );
}

void SerialCmd::Print ( unsigned char theUChar ) {
   if ( ( theSerial ) )
      theSerial->print ( theUChar );
}

void SerialCmd::Print ( int theInt ) {
   if ( ( theSerial ) )
      theSerial->print ( theInt );
}

void SerialCmd::Print ( unsigned int theUInt ) {
   if ( ( theSerial ) )
      theSerial->print ( theUInt );
}

void SerialCmd::Print ( long theLong ) {
   if ( ( theSerial ) )
      theSerial->print ( theLong );
}

void SerialCmd::Print ( unsigned long theULong ) {
   if ( ( theSerial ) )
      theSerial->print ( theULong );
}

void SerialCmd::Print ( float theFloat, int numDec ) {
   if ( ( theSerial ) )
      theSerial->print ( theFloat, numDec );
}

void SerialCmd::Print ( double theDouble, int numDec ) {
   if ( ( theSerial ) )
      theSerial->print ( theDouble, numDec );
}