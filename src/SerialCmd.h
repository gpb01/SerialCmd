/*
   SerialCmd - A Wiring/Arduino library to tokenize and parse commands
   received over a phisical/software serial port and optimized to run
   also on ATtiny series.

   Copyright (C) 2013 - 2023 Guglielmo Braguglia

   Based on the SerialCommand library :
      Copyright (C) 2012 Stefan Rado
      Copyright (C) 2011 Steven Cogswell <steven.cogswell@gmail.com>
                         http://husks.wordpress.com

   Version 20231009

   Please note:

   1. Adjust the #define(s) following your requirements :
      Use the real necessary values for SERIALCMD_MAXCMDNUM, SERIALCMD_MAXCMDLNG
      and SERIALCMD_MAXBUFFER to minimize the memory usage.
      If you need a second, program-accessible buffer, containing the command
      received before being processed, set SERIALCMD_PUBBUFFER to 1 otherwise leave
      it to 0.

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

#ifndef SERIALCMD
#define SERIALCMD

#if defined(WIRING) && WIRING >= 100
#include <Wiring.h>
#elif defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#endif

// SerialCmd version

#define SERIALCMD_VER    "1.1.5"                                  // SerialCmd library internal string   version
#define SERIALCMD_VER_NUM 10105                                   // SerialCmd library internal numeric  version
#define SERIALCMD_VER_MAJ     1                                   // SerialCmd library internal major    version
#define SERIALCMD_VER_MIN     1                                   // SerialCmd library internal minor    version
#define SERIALCMD_VER_REV     5                                   // SerialCmd library internal revision version

// SerialCmd configuration. Adjust following your needs

#define SERIALCMD_FORCEUC    0                                    // If set to 1 force uppercase for serial command
#define SERIALCMD_MAXCMDNUM  8                                    // Max Number of Command
#define SERIALCMD_MAXCMDLNG  6                                    // Max Command Length
#define SERIALCMD_MAXBUFFER 30                                    // Max Buffer  Length

#define SERIALCMD_PUBBUFFER  0                                    // If set to 1 create a public double buffer to read lines

// Command source validity
#define SERIALCMD_FROMSTRING -1                                   // Valid only as SerialCmd_ReadString command
#define SERIALCMD_FROMALL     0                                   // Always valid
#define SERIALCMD_FROMSERIAL  1                                   // Valid only as SerialCmd_ReadSer command

// End Command possible characters
#define SERIALCMD_CR      0x0D                                    // Carriage Return (char)
#define SERIALCMD_LF      0x0A                                    // Line Feed       (char)
#define SERIALCMD_NULL    0x00                                    // NULL            (char)

// Parameter separators possible strings
#define SERIALCMD_COMMA   ","                                     // COMMA           (C string)
#define SERIALCMD_SEMICOL ";"                                     // SEMI COLUMN     (C string)
#define SERIALCMD_DOT     "."                                     // DOT             (C string)
#define SERIALCMD_SPACE   " "                                     // SPACE           (C string)

// SerialCmd class definition

class SerialCmd {
   public:

      SerialCmd ( Stream &mySerial, char TermCh = SERIALCMD_CR, char * SepCh = ( char * ) SERIALCMD_COMMA );   // Constructor
      int8_t  ReadSer ( void );
      uint8_t AddCmd ( const char *, char, void ( * ) () );
      char *  ReadNext ( void );
      int8_t  ReadString ( char *, uint8_t fValidate = false );
      void Print ( String & );
      void Print ( char[] );
      void Print ( char );
      void Print ( unsigned char );
      void Print ( int );
      void Print ( unsigned int );
      void Print ( long );
      void Print ( unsigned long );
      void Print ( float, int numDec = 2 );
      void Print ( double, int numDec = 2 );
#ifdef __AVR__
      uint8_t AddCmd ( const __FlashStringHelper *, char, void ( * ) () );
      int8_t  ReadString ( const __FlashStringHelper *, uint8_t fValidate = false );
      void Print ( const __FlashStringHelper * );
#endif

#if ( SERIALCMD_PUBBUFFER == 1 )
      char lastLine[SERIALCMD_MAXBUFFER + 1];                     // Create a double buffer read lines public
#endif

   private:

      struct SerialCmd_Callback {                                 // Structure to record Command/Function pairs
         char command[SERIALCMD_MAXCMDLNG + 1];
         signed char allowedSource;
         void ( *function ) ();
      };

      SerialCmd_Callback SerialCmd_CmdList[SERIALCMD_MAXCMDNUM];  // Definition for Command/Function array
      uint8_t SerialCmd_CmdCount;                                 // Number of defined Command/Function

      char SerialCmd_Buffer[SERIALCMD_MAXBUFFER + 1];             // Serial buffer for Command
      char SerialCmd_BuffCmd[SERIALCMD_MAXCMDLNG + 1];            // Buffer for ONLY the commend part of the Buffer (NO parameters)
      char SerialCmd_InChar;                                      // Serial input character
      char * SerialCmd_Command = NULL;                            // Working variable used by strtok_r
      char * SerialCmd_Last = NULL;                               // State variable used by strtok_r

      char SerialCmd_Term;                                        // Default terminator for command (default CR)
      char SerialCmd_SepCh[2] = ",";                              // Allocate spece for separator characther (default = COMMA)
      char * SerialCmd_Sep = SerialCmd_SepCh;                     // Pointer to separator character

      uint8_t SerialCmd_Idx;                                      // General index for FOR loops
      uint8_t SerialCmd_BufferIdx;                                // Serial buffer Index
      int8_t  SerialCmd_Found;                                    // Valid command found

      Stream* theSerial;										            // Serial stream in use

      void    ClearBuffer ( void );                               // Clear the SerialCmd_Buffer filling with 0x00
      void    ConvertUC ( void );                                 // Convert the lower case characters of SerialCmd_Command to upper case
      void    ReadStringCommon ( void );                          // Common function used by the two version of ReadString to EXECUTE a command
      void    ValidateCommand ( void );                           // Common function used by the two version of ReadString to VALIDATE a command
      uint8_t AddCmdCommon ( const char *command, char allowedSource, void ( *function ) () ); // Common function used by the two version of AddCmd to ADD/MOD a command
};
#endif
