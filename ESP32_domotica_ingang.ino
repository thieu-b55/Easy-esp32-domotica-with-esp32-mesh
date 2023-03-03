/*
* MIT License
*
* Copyright (c) 2023 thieu-b55
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "painlessMesh.h"
#include <Preferences.h>

#define   MESH_PREFIX     "ESP32"
#define   MESH_PASSWORD   "ESP32_pswd"
#define   MESH_PORT       6666

#define   BIT_0           25
#define   BIT_1           26
#define   BIT_2           27
#define   INGANG          33

painlessMesh  mesh;
Preferences   pref;

int adres;
int type_int;
int waarde_int;
int ingang_int = 0;
int ingang_vorig_int;
unsigned long wacht_long;
String type_string = "   ";
String waarde_string = "    ";
String zend_string = "          ";

void uitsturen(){
  mesh.sendBroadcast( zend_string );
}

void receivedCallback( uint32_t from, String &msg ) {
  type_string = msg.substring(0,1);
  waarde_string = msg.substring(2, 5);
  type_int = type_string.toInt();
  if(type_int == 2){
    zend_string = String(5) + ":" + String(adres) + ":" + String(digitalRead(INGANG));
    uitsturen();
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.println("nieuwe verbinding");
}

void changedConnectionCallback() {
  Serial.println("veranderde verbinding");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    
}

void setup(){
  delay(2500);
  Serial.begin(115200);
  pinMode(BIT_0, INPUT_PULLDOWN);
  pinMode(BIT_1, INPUT_PULLDOWN);
  pinMode(BIT_2, INPUT_PULLDOWN);
  adres = digitalRead(BIT_0) + (digitalRead(BIT_1) * 2) + (digitalRead(BIT_2) * 4);
  Serial.println(adres);
  pinMode(INGANG, INPUT_PULLDOWN);
  mesh.setDebugMsgTypes( ERROR | STARTUP ); 
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  ingang_vorig_int = digitalRead(INGANG);
  wacht_long = millis();
}

void loop() {
  if((millis() - wacht_long) > 500){
    ingang_int = digitalRead(INGANG);
    if(ingang_vorig_int != ingang_int){
      wacht_long = millis();
      ingang_vorig_int = ingang_int;
      zend_string = String(5) + ":" + String(adres) + ":" + String(ingang_int);
      uitsturen();
    }
  }
  mesh.update();
}
