/*
   SD_SerialCmd - A simple program to demostrate the use of SerialCmd
   library to show the capability to receive commands via text file on SD.

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

#include <SPI.h>
#include <SD.h>
#include <SerialCmd.h>

#define LED_OFF      LOW                     // Adjust for your board
#define LED_ON      HIGH                     // Adjust for your board
#define LED_PIN        7                     // Pin where a LED is connected
#define SD_CS         10                     // Pin where the SD CS is connected

File dataFile;
SerialCmd mySerCmd ( dataFile );

// --------------- Functions for SerialCmd ---------------

void set_LEDON ( void ) {
   digitalWrite ( LED_PIN, LED_ON );
   Serial.println ( "LEDON" );
}

void set_LEDOF ( void ) {
   digitalWrite ( LED_PIN, LED_OFF );
   Serial.println ( "LEDOF" );
}

void set_DELAY ( void ) {
   char *   sParam;
   uint32_t delayTime;
   //
   sParam = mySerCmd.ReadNext();
   if ( sParam == NULL )
      return;
   delayTime = strtoul ( sParam, NULL, 10 );
   Serial.print ( "DELAY," );
   Serial.println ( delayTime );
   delay ( delayTime );
}

// ----------------------- setup() -----------------------

void setup() {
   int8_t retval;
   //
   delay ( 500 );
   pinMode ( LED_PIN, OUTPUT );
   //
   Serial.begin ( 9600 );
   while ( !Serial ) {
      delay ( 500 );
   }
   Serial.println();
   Serial.println ( "Program started ..." );
   //
   if ( !SD.begin ( SD_CS ) ) {
      Serial.println ( "Initialization failed!" );
      while ( 1 ) delay ( 500 );
   }
   //
   dataFile = SD.open ( "BLINK.TXT", FILE_READ );
   if ( !dataFile ) {
      Serial.println ( "File open in READ failed!" );
      while ( 1 ) delay ( 500 );
   }
   //
   mySerCmd.AddCmd ( "LEDON", SERIALCMD_FROMALL, set_LEDON );
   mySerCmd.AddCmd ( "LEDOF", SERIALCMD_FROMALL, set_LEDOF );
   mySerCmd.AddCmd ( "DELAY", SERIALCMD_FROMALL, set_DELAY );
   //
   delay ( 500 );
   //
   while ( dataFile.available() ) {
      retval = mySerCmd.ReadSer();
      if ( !retval )
         Serial.println ( ( char * ) "ERROR: Urecognized command. \r\n" );
   }
   dataFile.close();
   //
   Serial.println ( "Program ended." );
}

// ----------------------- loop() ------------------------

void loop() {

}
