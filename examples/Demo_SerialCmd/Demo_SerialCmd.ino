/*
   Demo_SerialCmd - A simple program to demostrate the use of SerialCmd
   library to show the capability to receive commands via serial port.

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
#include <stdlib.h>
#include <SerialCmd.h>

#define LED_OFF   LOW               // adjust for your board
#define LED_ON    HIGH              // adjust for your board

bool     isBlinking   = false;      // Indicates whether blinking is active or not
uint8_t  ledStatus    = LED_OFF;    // BUILTIN_LED status (OFF/ON)
uint8_t  blinkingCnt  = 0;          // Number of led status changes before turning off blinking
uint32_t blinkingTime = 0;          // Time of led status change
uint32_t blinkingLast = 0;          // Last millis() in which the status of the led was changed

SerialCmd mySerCmd ( Serial );      // Initialize the SerialCmd constructor using the "Serial" port

void sendOK ( void ) {
#ifdef __AVR__
   mySerCmd.Print ( F ( "OK \r\n" ) );
#else
   mySerCmd.Print ( ( char * ) "OK \r\n" );
#endif
}

void set_LEDON ( void ) {
   isBlinking = false;
   ledStatus  = LED_ON;
   digitalWrite ( LED_BUILTIN, LED_ON );
   sendOK();
}

void set_LEDOF ( void ) {
   isBlinking = false;
   ledStatus  = LED_OFF;
   digitalWrite ( LED_BUILTIN, LED_OFF );
   sendOK();
}

void set_LEDBL ( void ) {
   char *   sParam;
   //
   sParam = mySerCmd.ReadNext();
   if ( sParam == NULL ) {
#ifdef __AVR__
      mySerCmd.Print ( F ( "ERROR: Missing blinking time \r\n" ) );
#else
      mySerCmd.Print ( ( char * ) "ERROR: Missing blinking time \r\n" );
#endif
      return;
   }
   blinkingCnt  = 0;
   blinkingTime = strtoul ( sParam, NULL, 10 );
   blinkingLast = millis();
   isBlinking = true;
   sendOK();
}

void setup() {
   delay ( 500 );
   pinMode ( LED_BUILTIN, OUTPUT );
   digitalWrite ( LED_BUILTIN, ledStatus );
   Serial.begin ( 9600 );
   //
#ifdef ARDUINO_ARCH_STM32
   for ( uint8_t i = 0; i < 7; i++ ) {
      // create a 3500 msec delay with blink for STM Nucleo boards
      delay ( 500 );
      ledStatus = !ledStatus;
      digitalWrite ( LED_BUILTIN, ledStatus );
   }
#else
   while ( !Serial ) {
      delay ( 100 );
      ledStatus = !ledStatus;
      digitalWrite ( LED_BUILTIN, ledStatus );
   }
#endif
   //
#ifdef __AVR__
   mySerCmd.AddCmd ( F ( "LEDON" ) , SERIALCMD_FROMALL, set_LEDON );
   mySerCmd.AddCmd ( F ( "LEDOF" ) , SERIALCMD_FROMALL, set_LEDOF );
   mySerCmd.AddCmd ( F ( "LEDBL" ) , SERIALCMD_FROMALL, set_LEDBL );
   //
   mySerCmd.Print ( F ( "INFO: Program running on AVR ... \r\n" ) );
#else
   mySerCmd.AddCmd ( "LEDON", SERIALCMD_FROMALL, set_LEDON );
   mySerCmd.AddCmd ( "LEDOF", SERIALCMD_FROMALL, set_LEDOF );
   mySerCmd.AddCmd ( "LEDBL", SERIALCMD_FROMALL, set_LEDBL );
   //
   mySerCmd.Print ( ( char * ) "INFO: Program running ... \r\n" );
#endif
}

void loop() {
   uint8_t ret;
   //
   if ( isBlinking && ( millis() - blinkingLast > blinkingTime ) ) {
      ledStatus = !ledStatus;
      digitalWrite ( LED_BUILTIN, ledStatus );
      blinkingCnt++;
      blinkingLast += blinkingTime;
   }
   //
   if ( blinkingCnt >= 10 ) {
      blinkingCnt  = 0;
#ifdef __AVR__
      ret = mySerCmd.ReadString ( F ( "LEDOF" ) );
#else
      ret = mySerCmd.ReadString ( ( char * ) "LEDOF" );
#endif
      if ( ret == false ) {
         // error processing command from string ...
         // ... insert here error handling.

      }
   }
   //
   mySerCmd.ReadSer();
}
