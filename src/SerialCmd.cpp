/*
   SerialCmd - A Wiring/Arduino library to tokenize and parse commands
   received over a phisical/software serial port and optimized to run
   also on ATtiny series.

   Copyright (C) 2013 - 2022 Guglielmo Braguglia

   Based on the SerialCommand library :
      Copyright (C) 2012 Stefan Rado
      Copyright (C) 2011 Steven Cogswell <steven.cogswell@gmail.com>
                         http://husks.wordpress.com

   Version 20220104

   Please note:

   1. Adjust the #define(s) following your requirements :
      Use the real necessary values for SERIALCMD_MAXCMDNUM, SERIALCMD_MAXCMDLNG
      and SERIALCMD_MAXBUFFER to minimize the memory usage.

   2. If you want a different parametes separator, modifiy the line
      char* SerialCmd_Sep  = SERIALCMD_COMMA; with your separator.

   3. AllowedSource parameter can be:
        SERIALCMD_FROMSTRING (or -1) : valid only as ReadString command
        SERIALCMD_FROMALL            : always valid
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

void SerialCmd::ClearBuffer() {
   SerialCmd_Buffer[0] = 0x00;
   SerialCmd_BufferIdx = 0;
}

void SerialCmd::ConvertUC() {
   for ( uint8_t i = 0; i < strlen ( SerialCmd_Command ); i++ ) {
      if ( ( SerialCmd_Command[i] >= 'a' ) && ( SerialCmd_Command[i] <= 'z' ) ) {
         SerialCmd_Command[i] -= 32;
      }
   }
}

SerialCmd::SerialCmd ( Stream &mySerial, char TermCh, char * SepCh ) {
   ClearBuffer();
   SerialCmd_CmdCount = 0;
   theSerial = &mySerial;
   SerialCmd_Term = TermCh;
   strlcpy ( SerialCmd_Sep, SepCh, 2 );
}

void SerialCmd::AddCmd ( const char *command, char allowedSource, void ( *function ) () ) {
   if ( SerialCmd_CmdCount < SERIALCMD_MAXCMDNUM ) {
      strncpy ( SerialCmd_CmdList[SerialCmd_CmdCount].command, command, SERIALCMD_MAXCMDLNG );
      SerialCmd_CmdList[SerialCmd_CmdCount].allowedSource = allowedSource;
      SerialCmd_CmdList[SerialCmd_CmdCount].function = function;
      SerialCmd_CmdCount++;
   }
}

void SerialCmd::ReadSer() {
   while ( theSerial->available() > 0 ) {
      SerialCmd_InChar = theSerial->read();
      if ( SerialCmd_InChar == SerialCmd_Term ) {
         SerialCmd_Command = strtok_r ( SerialCmd_Buffer, SerialCmd_Sep, &SerialCmd_Last );
         SerialCmd_Found = false;
         if ( SerialCmd_Command != NULL ) {
            if ( SERIALCMD_FORCEUC ) ConvertUC();
            for ( SerialCmd_Idx = 0; SerialCmd_Idx < SerialCmd_CmdCount; SerialCmd_Idx++ ) {
               if ( strncmp ( SerialCmd_Command, SerialCmd_CmdList[SerialCmd_Idx].command, SERIALCMD_MAXCMDLNG ) == 0 ) {
                  if ( SerialCmd_CmdList[SerialCmd_Idx].allowedSource >= 0 ) {
                     ( *SerialCmd_CmdList[SerialCmd_Idx].function ) ();
                     SerialCmd_Found = true;
                     break;
                  }
               }
            }
         }
         ClearBuffer();
         if ( !SerialCmd_Found ) {
            // Command NOT found
            Print ( ( char* ) "ERROR: Command not found.\r\n" );
         }
      } else {
         if ( SerialCmd_BufferIdx < SERIALCMD_MAXBUFFER ) {
            SerialCmd_Buffer[SerialCmd_BufferIdx++] = SerialCmd_InChar;
            SerialCmd_Buffer[SerialCmd_BufferIdx] = 0x00;
         } else {
            ClearBuffer();
         }
      }
   }
}

void SerialCmd::ReadString ( char * theCmd ) {
   if ( strlen ( theCmd ) >= SERIALCMD_MAXBUFFER ) return;
   //
   strcpy ( SerialCmd_Buffer, theCmd );
   SerialCmd_Command = strtok_r ( SerialCmd_Buffer, SerialCmd_Sep, &SerialCmd_Last );
   SerialCmd_Found = false;
   if ( SerialCmd_Command != NULL ) {
      for ( SerialCmd_Idx = 0; SerialCmd_Idx < SerialCmd_CmdCount; SerialCmd_Idx++ ) {
         if ( strncmp ( SerialCmd_Command, SerialCmd_CmdList[SerialCmd_Idx].command, SERIALCMD_MAXCMDLNG ) == 0 ) {
            if ( SerialCmd_CmdList[SerialCmd_Idx].allowedSource <= 0 ) {
               ( *SerialCmd_CmdList[SerialCmd_Idx].function ) ();
               SerialCmd_Found = true;
               break;
            }
         }
      }
   }
   ClearBuffer();
}

char * SerialCmd::ReadNext() {
   return strtok_r ( NULL, SerialCmd_Sep, &SerialCmd_Last );
}

void SerialCmd::Print ( char * theString ) {
   if ( ( theSerial ) )
      theSerial->print ( theString );
}

void SerialCmd::Print ( char theChar ) {
   if ( ( theSerial ) )
      theSerial->print ( theChar );
}

void SerialCmd::Print ( uint32_t theUInt ) {
   if ( ( theSerial ) )
      theSerial->print ( theUInt );
}

void SerialCmd::Print ( int32_t theInt ) {
   if ( ( theSerial ) )
      theSerial->print ( theInt );
}
