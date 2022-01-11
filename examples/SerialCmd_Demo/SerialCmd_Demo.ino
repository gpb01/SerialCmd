#include <stdlib.h>
#include "SerialCmd.h"

bool     isBlinking   = false;      // Indicates whether blinking is active or not
uint8_t  ledStatus    = LOW;        // BUILTIN_LED status (OFF/ON)
uint8_t  blinkingCnt  = 0;          // Number of led status changes before turning off blinking
uint32_t blinkingTime = 0;          // Time of led status change
uint32_t blinkingLast = 0;          // Last millis() in which the status of the led was changed

SerialCmd mySerCmd ( Serial );      // Initialize the SerialCmd constructor using the "Serial" port

void sendOK ( void ) {
   mySerCmd.Print ( ( char * ) "OK \r\n" );
}

void set_LEDON ( void ) {
   isBlinking = false;
   ledStatus  = HIGH;
   digitalWrite ( LED_BUILTIN, HIGH );
   sendOK();
}

void set_LEDOF ( void ) {
   isBlinking = false;
   ledStatus  = LOW;
   digitalWrite ( LED_BUILTIN, LOW );
   sendOK();
}

void set_LEDBL ( void ) {
   char *   sParam;
   //
   sParam = mySerCmd.ReadNext();
   if ( sParam == NULL ) {
      mySerCmd.Print ( ( char * ) "ERROR: Missing blinking time \r\n" );
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
   mySerCmd.AddCmd ( "LEDON" , SERIALCMD_FROMALL, set_LEDON );
   mySerCmd.AddCmd ( "LEDOF" , SERIALCMD_FROMALL, set_LEDOF );
   mySerCmd.AddCmd ( "LEDBL" , SERIALCMD_FROMALL, set_LEDBL );
   //
   mySerCmd.Print ( ( char * ) "INFO: Program running ... \r\n" );
}

void loop() {
   if ( isBlinking && ( millis() - blinkingLast > blinkingTime ) ) {
      ledStatus = !ledStatus;
      digitalWrite ( LED_BUILTIN, ledStatus );
      blinkingCnt++;
      blinkingLast += blinkingTime;
   }
   //
   if ( blinkingCnt >= 10 ) {
      blinkingCnt  = 0;
      mySerCmd.ReadString ( ( char * ) "LEDOF" );
   }
   //
   mySerCmd.ReadSer();
}
