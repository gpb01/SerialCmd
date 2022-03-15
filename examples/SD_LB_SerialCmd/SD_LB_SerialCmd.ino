/*
   SD_LB_SerialCmd - A simple program to demostrate the use of SerialCmd
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

#if ( SERIALCMD_VER_NUM < 10103 )
#error "This program can compile only with SerialCmd version 1.1.3 or greater"
#endif

#define SD_CS         10                     // Pin where the SD CS is connected
#define MAX_LBL        5                     // Maximun number of labels in text file

File dataFile;
SerialCmd mySerCmd ( dataFile, SERIALCMD_LF );

uint32_t  labelPtr[MAX_LBL];

// --------------- Functions for SerialCmd ---------------

void f_LABEL ( void ) {
   char *   sParam;
   uint8_t  numLbl;
   //
   sParam = mySerCmd.ReadNext();
   if ( sParam == NULL ) {
      Serial.println ( "ERROR: Missing first paramemter on LABEL command" );
      while ( true ) delay ( 500 );
   }
   numLbl = atoi ( sParam );
   if ( numLbl >= MAX_LBL ) {
      Serial.println ( "ERROR: First paramemter on LABEL command too big" );
      while ( true ) delay ( 500 );
   }
   //
   labelPtr[numLbl] = dataFile.position();
   //
   Serial.print ( "Label " );
   Serial.print ( numLbl );
   Serial.print ( " set to " );
   Serial.println ( labelPtr[numLbl] );
}

void f_PRINT ( void ) {
   uint8_t prtNum;
   //
   prtNum = atoi ( mySerCmd.ReadNext() );
   Serial.print ( "PRINT command value: " );
   Serial.println ( prtNum );
}

void f_DELAY ( void ) {
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

void f_IFPIN ( void ) {
   uint8_t pinNum, trueLbl, falseLbl;
   //
   pinNum   = atoi ( mySerCmd.ReadNext() );
   trueLbl  = atoi ( mySerCmd.ReadNext() );
   falseLbl = atoi ( mySerCmd.ReadNext() );
   //
   if ( ( trueLbl >= MAX_LBL ) || ( falseLbl >= MAX_LBL ) ) {
      Serial.println ( "ERROR: true/false paramemter on IFPIN command too big" );
      while ( true ) delay ( 500 );
   }
   //
   pinMode ( pinNum, INPUT_PULLUP );
   delay ( 50 );
   if ( digitalRead ( pinNum ) ) {
      // Jump to the true label
      dataFile.seek ( labelPtr[trueLbl] );
      Serial.print ( "Jumping to the true label : " );
      Serial.println ( trueLbl );
   } else {
      // jump to the false label
      dataFile.seek ( labelPtr[falseLbl] );
      Serial.print ( "Jumping to the false label : " );
      Serial.println ( falseLbl );
   }
}

// ----------------------- setup() -----------------------

void setup() {
   int8_t retval;
   //
   delay ( 500 );
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
   dataFile = SD.open ( "LABEL.TXT", FILE_READ );
   if ( !dataFile ) {
      Serial.println ( "File open in READ failed!" );
      while ( 1 ) delay ( 500 );
   }
   //
   // Set the SerialCmd to execute only LABEL command
   mySerCmd.AddCmd ( "LABEL", SERIALCMD_FROMALL, f_LABEL );
   mySerCmd.AddCmd ( "PRINT", SERIALCMD_FROMALL, NULL );
   mySerCmd.AddCmd ( "DELAY", SERIALCMD_FROMALL, NULL );
   mySerCmd.AddCmd ( "IFPIN", SERIALCMD_FROMALL, NULL );
   //
   delay ( 500 );
   //
   // Read one first time all the file searcing for LABELs
   Serial.println();
   Serial.println ( "Reading lables ..." );
   while ( dataFile.available() ) {
      retval = mySerCmd.ReadSer();
      if ( !retval )
         Serial.println ( ( char * ) "ERROR: Urecognized command. \r\n" );
   }
   //
   // Set the SerialCmd to execute all commands but NOT LABELs
   mySerCmd.AddCmd ( "LABEL", SERIALCMD_FROMALL, NULL );
   mySerCmd.AddCmd ( "PRINT", SERIALCMD_FROMALL, f_PRINT );
   mySerCmd.AddCmd ( "DELAY", SERIALCMD_FROMALL, f_DELAY );
   mySerCmd.AddCmd ( "IFPIN", SERIALCMD_FROMALL, f_IFPIN );
   //
   // Return to the begin of file
   dataFile.seek ( 0 );
   delay ( 100 );
   //
   // Read a second time the file executing commands
   Serial.println();
   Serial.println ( "Executing commands ..." );
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
