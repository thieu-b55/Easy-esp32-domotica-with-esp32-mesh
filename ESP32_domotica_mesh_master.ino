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


#define   MESH_PREFIX     "ESP32"
#define   MESH_PASSWORD   "ESP32_pswd"
#define   MESH_PORT        6666
#define   RX2              16
#define   TX2              17

painlessMesh  mesh;

typedef struct{
  int in_int;
  uint8_t in;
}input_byte;

typedef struct{
  int code_int;       // 1 = uitgang    2 = uitgang   3 = temperatuur   9 = nieuwe of veranderde verbinding
  int nummer_int;     // welke in of uitgang
  int waarde_int;     // waarde voor temperatuur = temperatuur * 100
}melding;

input_byte    in_byte;
melding       bericht;

bool nieuw_bericht_bool = false;
String zend_string = "    ";


void uitsturen(){
  mesh.sendBroadcast( zend_string );
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.println("bericht ontvangen");
  Serial.println(msg);
  bericht.code_int = msg.substring(0, 1).toInt();
  bericht.nummer_int = msg.substring(2, 3).toInt();
  bericht.waarde_int = msg.substring(4, 7).toInt();
  Serial.println(bericht.code_int);
  Serial.println(bericht.nummer_int);
  Serial.println(bericht.waarde_int);
  Serial2.write((char*)&bericht, sizeof(bericht));
}

void newConnectionCallback(uint32_t nodeId) {
  
}

void changedConnectionCallback() {
  
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  delay(2500);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX2, TX2);
  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  while(Serial2.available() > 0){
    nieuw_bericht_bool = true;
    Serial2.readBytes((char*)&in_byte, sizeof(in_byte));
  }
  if(nieuw_bericht_bool){
    Serial.println("zend bericht");
    Serial.println(in_byte.in_int);
    Serial.println(in_byte.in);
    nieuw_bericht_bool = false;
    zend_string = String(in_byte.in_int) + ":" +  String(in_byte.in);
    Serial.println(zend_string);
    uitsturen();
    
  }
  mesh.update();
}
