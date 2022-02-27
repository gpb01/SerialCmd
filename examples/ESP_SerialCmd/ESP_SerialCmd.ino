/*
   ESP_SerialCmd - A simple program to demostrate the use of SerialCmd
   library with an ESP32 or ESP8266 module to show the capability to receive
   commands via WiFi: http://esp_IP/command_to_send

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

#if defined ( ARDUINO_ARCH_ESP8266 )
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <string.h>
#include "SerialCmd.h"
#include "credential.h"

#define LED_ON       HIGH              // set following your hardware
#define LED_OFF      LOW               // set following your hardware

WiFiServer  server ( 80 );
const char* ssid     = NET_SSID;       // Your network SSID in credential.h file
const char* password = NET_PSW;        // Your network PSW  in credential.h file

SerialCmd   mySerCmd ( Serial );
bool        isBlinking   = false;      // Indicates whether blinking is active or not
uint8_t     ledStatus    = LED_OFF;    // BUILTIN_LED status (OFF/ON)
uint8_t     blinkingCnt  = 0;          // Number of led status changes before turning off blinking
uint32_t    blinkingTime = 0;          // Time of led status change
uint32_t    blinkingLast = 0;          // Last millis() in which the status of the led was changed

// ------------------------------------------------------------------

void set_LEDON ( void ) {
   isBlinking = false;
   ledStatus  = LED_ON;
   digitalWrite ( LED_BUILTIN, LED_ON );
}

void set_LEDOF ( void ) {
   isBlinking = false;
   ledStatus  = LED_OFF;
   digitalWrite ( LED_BUILTIN, LED_OFF );
}

void set_LEDBL ( void ) {
   char * sParam;
   //
   sParam = mySerCmd.ReadNext();
   if ( sParam == NULL ) {
      return;
   }
   blinkingCnt  = 0;
   blinkingTime = strtoul ( sParam, NULL, 10 );
   blinkingLast = millis();
   isBlinking = true;
}

// ------------------------------------------------------------------


char* checkWiFiCommand ( void ) {
   static const uint8_t  BUF_LNG      = 200;      // HTML buffer max lenght
   static const uint8_t  CMD_LNG      = 128;      // Command buffer max lenght
   static const uint16_t NET_TIMEOUT  = 2000;     // Browser session timeout
   static char           clientBuffer[BUF_LNG];   // HTML buffer
   static char           clientCommand[CMD_LNG];  // Command buffer for SerialCmd
   //
   uint8_t               buf_idx      = 0;
   int8_t                lstCmdStatus = -1;       // indicate the last command status -1:none, 0:NOT valid, 1:valid
   uint32_t              lastMillis   = 0;
   char                  c            = 0;
   char*                 cmdStart     = NULL;
   char*                 cmdStop      = NULL;
   char*                 retVal       = NULL;
   //
   WiFiClient client = server.available();
   if ( client ) {
      // Client connected ...
      memset ( clientBuffer,  0x00, BUF_LNG );
      memset ( clientCommand, 0x00, CMD_LNG );
      buf_idx = 0;
      lastMillis = millis();
      //
      while ( client.connected() ) {
         if ( client.available() ) {
            c = client.read();
            lastMillis = millis();
            //
            if ( c == '\n' ) {
               //
               // New Line received ...
               if ( strlen ( clientBuffer ) == 0 ) {
                  //
                  // ... empty line, send response to client
                  client.println ( "HTTP/1.1 200 OK" );
                  client.println ( "Content-type:text/html" );
                  client.println ( "Connection: close" );
                  client.println();
                  //
                  // Display the HTML web page
                  client.println ( "<!DOCTYPE html><html>" );
                  client.println ( "<body><h1>ESP SerialCmd Server</h1>" );
                  client.println ( "<p>Please, enter a valid command using the following syntax: http://" );
                  client.print   ( WiFi.localIP() );
                  client.println ( "/command,parameters</p>" );
                  //
                  if ( SERIALCMD_FORCEUC != 0 ) {
                     client.println ( "<p>Note: lower case characters will be converted to upper case.</p>" );
                  }
                  //
                  if ( lstCmdStatus == 0 ) {
                     client.println ( "<p>Last entered command was NOT recognized.</p>" );
                     lstCmdStatus = -1;
                     retVal = NULL;
                  } else if ( lstCmdStatus == 1 ) {
                     client.println ( "<p>Last entered command WAS recognized and will be executed.</p>" );
                     lstCmdStatus = -1;
                  }
                  client.println ( "</body></html>" );
                  //
                  client.println();
                  break;
               } else {
                  //
                  // ... search for HTTP GET line
                  cmdStart = strstr ( clientBuffer, "GET /" );
                  if ( cmdStart != NULL ) {
                     cmdStop = strstr ( clientBuffer, "HTTP" );
                     if ( cmdStop != NULL ) {
                        if ( ( int ) ( cmdStop - cmdStart - 5 ) < CMD_LNG ) {
                           strlcpy ( clientCommand, ( cmdStart + 5 ), ( int ) ( cmdStop - cmdStart - 5 ) );
                           if ( strcmp ( "favicon.ico", clientCommand ) != 0 ) {
                              retVal = clientCommand;
                              lstCmdStatus = mySerCmd.ReadString ( retVal, true );
                           }
                        }
                     }
                  }
                  memset ( clientBuffer,  0x00, BUF_LNG );
                  buf_idx = 0;
               }
            } else {
               if ( c != '\r' ) {
                  if ( buf_idx < ( BUF_LNG - 1 ) )
                     clientBuffer[buf_idx++] = c;
               }
            }
         }
         if ( millis() - lastMillis > NET_TIMEOUT ) break;
      }
      client.stop();
   }
   return retVal;
}

// ------------------------------------------------------------------

void setup() {
   delay ( 500 );
   //
   ledStatus = LED_OFF;
   pinMode ( LED_BUILTIN, OUTPUT );
   digitalWrite ( LED_BUILTIN, ledStatus );
   //
   Serial.begin ( 115200 );
   Serial.println();
   Serial.println();
   Serial.print ( "Connecting to " );
   Serial.print ( NET_SSID );
   Serial.print ( " " );
   //
   WiFi.begin ( ssid, password );
   //
   while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
   }
   Serial.println();
   //
   Serial.println ( "WiFi connected." );
   Serial.print ( "IP address: " );
   Serial.println ( WiFi.localIP() );
   //
   mySerCmd.AddCmd ( "LEDON", SERIALCMD_FROMALL, set_LEDON );
   mySerCmd.AddCmd ( "LEDOF", SERIALCMD_FROMALL, set_LEDOF );
   mySerCmd.AddCmd ( "LEDBL", SERIALCMD_FROMALL, set_LEDBL );
   //
   Serial.println();
   Serial.print   ( "You are using SerialCmd ver. " );
   Serial.println ( SERIALCMD_VER );
   Serial.println ( "Valid commands are:" );
   Serial.println ( "   LEDON" );
   Serial.println ( "   LEDOF" );
   Serial.println ( "   LEDBL,msec" );
   Serial.println();
   //
   server.begin();
}

// ------------------------------------------------------------------

void loop() {
   char*  retVal;
   int8_t cmdStatus;
   //
   retVal = checkWiFiCommand();
   if ( retVal != NULL ) {
      Serial.print ( "HTTP received command: " );
      Serial.println ( retVal );
      cmdStatus    = mySerCmd.ReadString ( retVal );
   }
   //
   if ( isBlinking && ( millis() - blinkingLast > blinkingTime ) ) {
      ledStatus = !ledStatus;
      digitalWrite ( LED_BUILTIN, ledStatus );
      blinkingCnt++;
      blinkingLast += blinkingTime;
   }
   if ( blinkingCnt >= 10 ) {
      blinkingCnt  = 0;
      mySerCmd.ReadString ( ( char * ) "LEDOF" );
   }
   //
   cmdStatus = mySerCmd.ReadSer();
   if ( cmdStatus == false )
      mySerCmd.Print ( ( char * ) "ERROR: Urecognized command. \r\n" );
   //
   yield();
}
