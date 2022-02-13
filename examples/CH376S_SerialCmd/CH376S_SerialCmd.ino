/*
   CH376S_SerialCmd - A simple program to demostrate the use of SerialCmd
   library with one CH376S module to read and execute a list of commans
   from a text file.

   Copyright (C) 2013 - 2022 Guglielmo Braguglia

   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   This is free software: you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This software is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
   
*/ 

#define MAX_BUFFER	255                               // Record buffer max size
#define LED_OFF      LOW                               // Adjust for your board
#define LED_ON      HIGH                               // Adjust for your board

#if defined ( ESP_PLATFORM )
#pragma message "Compiling for ESP32"
#include <HardwareSerial.h>
#endif
#include <Ch376msc.h>
#include <SerialCmd.h>

SerialCmd mySerCmd ( Serial );                         // Initialize the SerialCmd constructor using the "Serial" port

#if defined ( ESP_PLATFORM )
HardwareSerial mySerial ( 2 );
Ch376msc flashDrive ( mySerial );                      // Ch376 object with hardware Serial2 on ESP32 baudrate: 9600, 19200, 57600, 115200
#else
Ch376msc flashDrive ( Serial1, 57600 );                // Ch376 object with hardware Serial1 on arduino mega baudrate: 9600, 19200, 57600, 115200
#endif

uint16_t dataSize = 0;
bool     fAttached  = false;
char     recordBuffer[MAX_BUFFER];                     // max length 255 = 254 char + 1 NULL character

void set_LEDON ( void ) {
   digitalWrite ( LED_BUILTIN, LED_ON );
}

void set_LEDOF ( void ) {
   digitalWrite ( LED_BUILTIN, LED_OFF );
}

void set_DELAY ( void ) {
   char *   sParam;
   uint32_t delayTime;
   //
   sParam = mySerCmd.ReadNext();
   if ( sParam == NULL )
      return;
   delayTime = strtoul ( sParam, NULL, 10 );
   delay ( delayTime );
}

void setup() {
   delay ( 500 );
   //
   pinMode ( LED_BUILTIN, OUTPUT );
   digitalWrite ( LED_BUILTIN, LED_OFF );
   //
   Serial.begin ( 115200 );
   delay ( 50 );
#if defined ( ESP_PLATFORM )
   mySerial.begin ( 57600, SERIAL_8N1, 16, 17 );
   delay ( 100 );
#else
   Serial1.begin ( 57600 );
#endif
   //
   mySerCmd.AddCmd ( "LEDON", SERIALCMD_FROMALL, set_LEDON );
   mySerCmd.AddCmd ( "LEDOF", SERIALCMD_FROMALL, set_LEDOF );
   mySerCmd.AddCmd ( "DELAY", SERIALCMD_FROMALL, set_DELAY );
   //
   flashDrive.init();
   delay ( 100 );
   mySerCmd.Print ( "INFO: Program started ... \r\n" );
}

void loop() {
   //
   // Check if USB drievr is connected or disconnected
   if ( flashDrive.checkIntMessage() ) {
      if ( flashDrive.getDeviceStatus() ) {
         mySerCmd.Print ( "INFO: Flash drive attached! \r\n" );
         fAttached = true;
      } else {
         mySerCmd.Print ( "INFO: Flash drive detached! \r\n" );
         fAttached = false;
      }
   }
   //
   //
   if ( fAttached ) {
      if ( flashDrive.driveReady() ) {
         flashDrive.setFileName ( "BLINK.TXT" );          // set the file name to Open
         if ( flashDrive.openFile() == ANSW_USB_INT_SUCCESS ) {
            // File successfully opened ...
            mySerCmd.Print ( "INFO: File opened ... \r\n" );
            while ( !flashDrive.getEOF() ) {
               flashDrive.readFileUntil ( 0x0A, recordBuffer, MAX_BUFFER );
               // remove both 0x0D and 0x0A at end of record
               dataSize = strlen ( recordBuffer );
               if ( recordBuffer[dataSize - 1] == 0x0A )
                  recordBuffer[dataSize - 1] = 0x00;
               if ( recordBuffer[dataSize - 2] == 0x0D )
                  recordBuffer[dataSize - 2] = 0x00;
               //
               mySerCmd.Print ( recordBuffer );
               mySerCmd.Print ( " \r\n" );
               mySerCmd.ReadString ( recordBuffer );
            }
            flashDrive.closeFile();                       // at the end, Close the file
            mySerCmd.Print ( "INFO: File closed. \r\n" );
         } else {
            // File Open Error ...
            mySerCmd.Print ( "ERROR opening file \r\n" );
         }
         //
         digitalWrite ( LED_BUILTIN, LED_OFF );
         delay ( 5000 );
      }
   }
}
