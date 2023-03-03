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

#include "Arduino.h"
#include "WiFi.h"
#include <SPI.h>
#include <Preferences.h>
#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Wire.h"
#include "time.h"
#include <Adafruit_NeoPixel.h>


Preferences pref;
AsyncWebServer server(80);

#define DS3231SN            0x68

#define I2C_SDA             21
#define I2C_SCL             22

#define RX2                 16
#define TX2                 17

#define CS                  5

#define MINUUT_PULS         4

#define LED_PIN             2
#define LED_COUNT           16

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef struct{
  int uit_int = 1;      // bericht naar uitgangmodule 1   bericht terug van uitgangmodule 4
  uint8_t uit;
}output_byte;

typedef struct{
  int in_int = 2;       // bericht naar ingangmodule 2    bericht terug van ingangmodule 5
  uint8_t dummy_int = 0;
}input_byte;

typedef struct{
  int code_int;
  int nummer_int;
  int waarde_int;
}melding;

output_byte     uit_byte;
input_byte      in_byte;
melding         bericht;

/*
 * netwerk
 */
bool netwerk_bool = false;
char pswd_char[40];
char ssid_char[40];
const char* APssid = "ESP32rc";
const char* APpswd = "ESP32pswd";
const char* STA_SSID = "sta_ssid";
const char* STA_PSWD = "sta_pswd";
const char* IP_1_KEUZE = "ip_1_keuze";
const char* IP_2_KEUZE = "ip_2_keuze";
const char* IP_3_KEUZE = "ip_3_keuze";
const char* IP_4_KEUZE = "ip_4_keuze";
int ip_1_int = 192;
int ip_2_int = 168;
int ip_3_int = 1;
int ip_4_int = 222;
String pswd_string = "                                        ";
String ssid_string = "                                        ";
String ip_1_string = "    "; 
String ip_2_string = "    "; 
String ip_3_string = "    "; 
String ip_4_string = "    "; 
unsigned long wacht_op_netwerk_long;
/*
 * tijd / display
 */
bool tijd_init_bool;
bool tijd_update_bool;
bool minuut_interrupt_bool;
int utc_dag_int;
int utc_uren_int;
int utc_minuten_int;
int utc_offset_uren_int = 0;
int utc_offset_minuten_int = 0;
int winter_zomer_int = 0;         // winter = 0 <> zomer = 1
int intensiteit_int = 0;;
int local_dag_int;
int local_uren_int;
int local_minuten_int;
int local_seconden_int;
int dag_int;
int uren_int;
int minuten_int;
int twaalf_uur_int = 0;
uint8_t cijfers[][11] = {{0x38, 0x44, 0x4c, 0x54, 0x64, 0x44, 0x44, 0x38},  //0
                         {0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38},  //1
                         {0x38, 0x44, 0x04, 0x04, 0x08, 0x10, 0x20, 0x7c},  //2
                         {0x7c, 0x08, 0x10, 0x08, 0x04, 0x04, 0x44, 0x38},  //3
                         {0x08, 0x18, 0x28, 0x48, 0x48, 0x7c, 0x08, 0x08},  //4
                         {0x7c, 0x40, 0x78, 0x04, 0x04, 0x04, 0x44, 0x38},  //5
                         {0x18, 0x20, 0x40, 0x78, 0x44, 0x44, 0x44, 0x38},  //6
                         {0x7c, 0x04, 0x04, 0x08, 0x10, 0x20, 0x20, 0x20},  //7
                         {0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x44, 0x38},  //8
                         {0x38, 0x44, 0x44, 0x44, 0x3c, 0x04, 0x08, 0x30},  //9
                         {0x3c, 0x42, 0xa5, 0x81, 0xa5, 0x99, 0x42, 0x3c}}; //smiley
/*
 * domotica
 */
bool terugmelding_uit_bool = false;
bool terugmelding_in_bool = false;
bool nieuw_bericht_bool = false;
char uitgang_label_char_array[8][12];
char uitgang_in_char_array[][8] = {"x", "x", "x", "x", "x", "x", "x", "x"};
char ingang_label_char_array[8][12];
char temperatuur_label_char_array[3][12];
char aan_tijd_0_char[6];
char aan_tijd_1_char[6];
char aan_tijd_2_char[6];
char aan_tijd_3_char[6];
char aan_tijd_4_char[6];
char aan_tijd_5_char[6];
char aan_tijd_6_char[6];
char aan_tijd_7_char[6];
char uit_tijd_0_char[6];
char uit_tijd_1_char[6];
char uit_tijd_2_char[6];
char uit_tijd_3_char[6];
char uit_tijd_4_char[6];
char uit_tijd_5_char[6];
char uit_tijd_6_char[6];
char uit_tijd_7_char[6];
int dag_aan_int_array[] = {8, 8, 8, 8, 8, 8, 8, 8};
int uur_aan_int_array[] = {24, 24, 24, 24, 24, 24, 24, 24};
int minuut_aan_int_array[] = {0, 0, 0, 0, 0, 0, 0, 0};
int uur_uit_int_array[] = {24, 24, 24, 24, 24,24, 24, 24};
int minuut_uit_int_array[] = {0, 0, 0, 0, 0, 0, 0, 0};
String lees_string = "                         ";
/*
 * Domotica instellen
 */
char label_char[15]; 
const char* week_dagen_char[] = {"dummy", "Maandag", "Dinsdag", "Woensdag", "Donderdag", "Vrijdag", "Zaterdag", "Zondag", "Dagelijks", "Weekdagen", "Weekend"}; 
const char* UITGANG_PLUS = "uitgang_plus";
const char* UITGANG_MIN = "uitgang_min";
const char* LABEL_IN = "label_in";
const char* DAG_IN = "dag_in";
const char* AAN_IN = "aan_in";
const char* UIT_IN = "uit_in";
const char* IN_IN = "in_in";
const char* BEVESTIG_UIT = "bevestig_uit";
const char* INGANG_PLUS = "ingang_plus";
const char* INGANG_MIN = "ingang_min";
const char* IN_LABEL_IN = "in_label_in";
const char* BEVESTIG_IN = "bevestig_in";
int uitgang_teller_int = 0;
int ingang_teller_int = 0;
String label_string = "              ";
String aan_string = "     ";
String uit_string = "     ";
String in_string = "  ";
/*
 * Domotica uitgang sturen
 */
byte manueel_byte = 0b00000000;
byte uitgang_actief_byte = 0b00000000;
byte vorig_uitgang_actief_byte;
byte uitgang_byte = 0b00000000;
byte vorig_uitgang_byte;
byte uitgang_terug_byte = 0b00000000;
byte temp_uitgang_terug_byte;
byte dag_byte = 0b00000000;
byte tijd_byte = 0b00000000;
byte tijd_override_byte = 0b00000000;
byte uit_in_byte = 0b00000000;
byte ingang_actief_byte = 0b00000000;
byte vorig_ingang_actief_byte;
byte ingang_byte = 0b00000000;
byte ingang_terug_byte = 0b00000000;
byte temp_ingang_terug_byte;
const char* UITGANG_0 = "uitgang_0";
const char* UITGANG_1 = "uitgang_1";
const char* UITGANG_2 = "uitgang_2";
const char* UITGANG_3 = "uitgang_3";
const char* UITGANG_4 = "uitgang_4";
const char* UITGANG_5 = "uitgang_5";
const char* UITGANG_6 = "uitgang_6";
const char* UITGANG_7 = "uitgang_7";
const char* MANUEEL_0 = "manueel_0";
const char* MANUEEL_1 = "manueel_1";
const char* MANUEEL_2 = "manueel_2";
const char* MANUEEL_3 = "manueel_3";
const char* MANUEEL_4 = "manueel_4";
const char* MANUEEL_5 = "manueel_5";
const char* MANUEEL_6 = "manueel_6";
const char* MANUEEL_7 = "manueel_7";
const char* INGANG_0 = "ingang_0";
const char* INGANG_1 = "ingang_1";
const char* INGANG_2 = "ingang_2";
const char* INGANG_3 = "ingang_3";
const char* INGANG_4 = "ingang_4";
const char* INGANG_5 = "ingang_5";
const char* INGANG_6 = "ingang_6";
const char* INGANG_7 = "ingang_7";
/*
 * Ledstrip
 */
bool ingang_update_bool = false;
int update_int = 60000;
unsigned long inlezen_long;
unsigned long terugmelding_uit_long;
unsigned long terugmelding_in_long;
unsigned long tussentijd_long;
 /*
  * SPIFFS
  */
char file_char[26];
char data_char[26];
char lees_char[26];
String file_string = "                        ";
String data_string = "                        ";
String uitgang_label_string ="/ulabel";
String ingang_label_string = "/ilabel";
String temperatuur_label_string = "/tlabel";
String dag_aan_string = "/daan";
String uur_aan_string = "/uaan";
String minuut_aan_string = "/maan";
String uur_uit_string = "/uuit";
String minuut_uit_string = "/muit";
String uitgang_in_string = "/uin";
String uitgang_actief_string = "/uact";
String uitgang_hand_string = "/uhand";
/*
 * Klok instellen
 */
const char* H_CHAR = "h";
const char* TZ_UUR = "tz_uur";
const char* TZ_MINUUT = "tz_minuut";
const char* ZOMER = "zomer";
const char* TZ_BEVESTIG = "tz_bevestig";
const char* INTENSITEIT_GEWENST = "intensiteit_gewenst";
const char* INTENSITEIT_BEVESTIG = "intensiteit_bevestig";


      
void display_setup(uint8_t adres, uint8_t waarde){
  SPI.beginTransaction(SPISettings(15000, SPI_MSBFIRST, SPI_MODE2));
  digitalWrite(CS, LOW);
  SPI.transfer(adres);
  SPI.transfer(waarde);
  SPI.transfer(adres);
  SPI.transfer(waarde);
  SPI.transfer(adres);
  SPI.transfer(waarde);
  SPI.transfer(adres);
  SPI.transfer(waarde);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

void display_digits(uint8_t adres, uint8_t digit_1, uint8_t digit_2, uint8_t digit_3, uint8_t digit_4){
  SPI.beginTransaction(SPISettings(15000, SPI_MSBFIRST, SPI_MODE2));
  digitalWrite(CS, LOW);
  SPI.transfer(adres);
  SPI.transfer(digit_1);
  SPI.transfer(adres);
  SPI.transfer(digit_2);
  SPI.transfer(adres);
  SPI.transfer(digit_3);
  SPI.transfer(adres);
  SPI.transfer(digit_4);
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
}

void smiley(){
  display_digits(0x01, cijfers[10][0], cijfers[10][0], cijfers[10][0], cijfers[10][0]);
  display_digits(0X02, cijfers[10][1], cijfers[10][1], cijfers[10][1], cijfers[10][1]);
  display_digits(0X03, cijfers[10][2], cijfers[10][2], cijfers[10][2], cijfers[10][2]);
  display_digits(0X04, cijfers[10][3], cijfers[10][3], cijfers[10][3], cijfers[10][3]);
  display_digits(0X05, cijfers[10][4], cijfers[10][4], cijfers[10][4], cijfers[10][4]);
  display_digits(0X06, cijfers[10][5], cijfers[10][5], cijfers[10][5], cijfers[10][5]);
  display_digits(0X07, cijfers[10][6], cijfers[10][6], cijfers[10][6], cijfers[10][6]);
  display_digits(0X08, cijfers[10][7], cijfers[10][7], cijfers[10][7], cijfers[10][7]);
}

byte dec_naar_bcd(byte waarde){
  return (((waarde / 10) << 4) + (waarde % 10));
}

byte bcd_naar_dec(byte waarde){
  return (((waarde >> 4) * 10) + (waarde % 16));
}

void initTime(){
  struct tm timeinfo;
  configTime(0, 0, "pool.ntp.org");
  if(!getLocalTime(&timeinfo)){
    return;
  }
  tijd_init_bool = true;
}

void tijdzone_correctie(){
  if(tijd_update_bool == true){
    utc_offset_uren_int = pref.getShort("uur_of");
    utc_offset_minuten_int = pref.getShort("min_of");
    winter_zomer_int = pref.getShort("w_z");
    local_uren_int = utc_uren_int + utc_offset_uren_int + winter_zomer_int;
    if(utc_offset_uren_int > -1){
      local_minuten_int = utc_minuten_int + utc_offset_minuten_int;
      if(local_minuten_int > 59){
        local_minuten_int -= 60;
        local_uren_int += 1;
      }
      if(local_uren_int > 23){
        local_uren_int -= 24;
        local_dag_int += 1;
        if(local_dag_int == 8){
          local_dag_int = 1;
        }
      }
    }
    if(utc_offset_uren_int < 0){
      local_minuten_int = utc_minuten_int - utc_offset_minuten_int;
      if(local_minuten_int < 0){
        local_minuten_int += 60;
        local_uren_int -= 1;
      }
      if(local_uren_int < 0){
        local_uren_int += 24;
        local_dag_int -= 1;
        if(local_dag_int == 0){
          local_dag_int = 7;
        }
      }
    }
    Serial.print(week_dagen_char[local_dag_int]);
    Serial.print(" : ");
    Serial.print(local_uren_int);
    Serial.print(" : ");
    Serial.println(local_minuten_int);
    Wire.beginTransmission(DS3231SN);
    Wire.write(0x00);
    Wire.write(dec_naar_bcd(local_seconden_int));
    Wire.write(dec_naar_bcd(local_minuten_int));
    Wire.write(dec_naar_bcd(local_uren_int) &0x3f);
    Wire.write(local_dag_int);
    Wire.endTransmission();
  }
}

void tijd_update(){
  tijd_update_bool = false;
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){
    tijd_update_bool = true;
    Serial.println("update ok");
    utc_dag_int = timeinfo.tm_wday;
    utc_uren_int = timeinfo.tm_hour;
    utc_minuten_int = timeinfo.tm_min;
    local_seconden_int = timeinfo.tm_sec;
    local_dag_int = utc_dag_int + 7;
    if(local_dag_int > 7){
      local_dag_int -= 7;
    }
    tijdzone_correctie();
    return;
  }
  Serial.println("update nok");
}

void tijd_naar_led(){
  int tiental_uur_int;
  int eenheden_uur_int;
  int tiental_minuut_int;
  int eenheden_minuut_int;
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.requestFrom(DS3231SN, 3);
  minuten_int = (bcd_naar_dec(Wire.read()));
  uren_int = (bcd_naar_dec(Wire.read()));
  dag_int = Wire.read();
  tiental_uur_int = uren_int / 10;
  eenheden_uur_int = uren_int - (tiental_uur_int * 10);
  tiental_minuut_int = minuten_int / 10;
  eenheden_minuut_int = minuten_int - (tiental_minuut_int * 10);
  display_digits(0x01, cijfers[tiental_uur_int][0], cijfers[eenheden_uur_int][0], cijfers[tiental_minuut_int][0], cijfers[eenheden_minuut_int][0]);
  display_digits(0X02, cijfers[tiental_uur_int][1], cijfers[eenheden_uur_int][1], cijfers[tiental_minuut_int][1], cijfers[eenheden_minuut_int][1]);
  display_digits(0X03, cijfers[tiental_uur_int][2], cijfers[eenheden_uur_int][2] + 1, cijfers[tiental_minuut_int][2], cijfers[eenheden_minuut_int][2]);
  display_digits(0X04, cijfers[tiental_uur_int][3], cijfers[eenheden_uur_int][3], cijfers[tiental_minuut_int][3], cijfers[eenheden_minuut_int][3]);
  display_digits(0X05, cijfers[tiental_uur_int][4], cijfers[eenheden_uur_int][4], cijfers[tiental_minuut_int][4], cijfers[eenheden_minuut_int][4]);
  display_digits(0X06, cijfers[tiental_uur_int][5], cijfers[eenheden_uur_int][5] + 1, cijfers[tiental_minuut_int][5], cijfers[eenheden_minuut_int][5]);
  display_digits(0X07, cijfers[tiental_uur_int][6], cijfers[eenheden_uur_int][6], cijfers[tiental_minuut_int][6], cijfers[eenheden_minuut_int][6]);
  display_digits(0X08, cijfers[tiental_uur_int][7], cijfers[eenheden_uur_int][7], cijfers[tiental_minuut_int][7], cijfers[eenheden_minuut_int][7]);
}

void write_char(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("write char niet gelukt");
      return;
  }
  if(file.print(message)){
  } 
  file.close();
}

void read_string(fs::FS &fs, const char * path){
  File file = fs.open(path);
  if(!file){
    Serial.println("string niet gevonden");
    return;
  }
  int teller = 0;
  memset(lees_char, 0, sizeof(lees_char));
  while(file.available()){
    lees_char[teller] = file.read();
    teller++;
  }
  file.close();
  lees_string = String(lees_char);
}

void write_string(fs::FS &fs, const char * path, const String message){
  char temp_char[25];
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("write string niet gelukt");
      return;
  }
  memset(temp_char, 0, sizeof(temp_char));
  message.toCharArray(temp_char, message.length() + 1);
  file.print(temp_char);
 }

void schrijf_spiffs(){
  for(int teller = 0; teller < 8; teller++){
    file_string = uitgang_label_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    write_char(SPIFFS, file_char, uitgang_label_char_array[teller]); 
    
    file_string = ingang_label_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    write_char(SPIFFS, file_char, ingang_label_char_array[teller]);

    file_string = uitgang_in_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    write_char(SPIFFS, file_char, uitgang_in_char_array[teller]);
    
    file_string = dag_aan_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    data_string = String(dag_aan_int_array[teller]);
    data_string.toCharArray(data_char, data_string.length() + 1);
    write_char(SPIFFS, file_char, data_char);
    
    file_string = uur_aan_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    data_string = String(uur_aan_int_array[teller]);
    data_string.toCharArray(data_char, data_string.length() + 1);
    write_char(SPIFFS, file_char, data_char);
    
    file_string = minuut_aan_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    data_string = String(minuut_aan_int_array[teller]);
    data_string.toCharArray(data_char, data_string.length() + 1);
    write_char(SPIFFS, file_char, data_char);
    
    file_string = uur_uit_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    data_string = String(uur_uit_int_array[teller]);
    data_string.toCharArray(data_char, data_string.length() + 1);
    write_char(SPIFFS, file_char, data_char);
    
    file_string = minuut_uit_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    data_string = String(minuut_uit_int_array[teller]);
    data_string.toCharArray(data_char, data_string.length() + 1);
    write_char(SPIFFS, file_char, data_char);
  }
  for(int teller = 0; teller < 3; teller++){
    file_string = temperatuur_label_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    write_char(SPIFFS, file_char, temperatuur_label_char_array[teller]);
  }
}

void lees_spiffs(){
  for(int teller = 0; teller < 8; teller++){
    file_string = uitgang_label_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    memset(uitgang_label_char_array[teller], 0, sizeof(uitgang_label_char_array[teller]));
    strcpy(uitgang_label_char_array[teller], lees_char);
    
    file_string = ingang_label_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    memset(ingang_label_char_array[teller], 0, sizeof(ingang_label_char_array[teller]));
    strcpy(ingang_label_char_array[teller], lees_char);

    file_string = uitgang_in_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    memset(uitgang_in_char_array[teller], 0, sizeof(uitgang_in_char_array[teller]));
    strcpy(uitgang_in_char_array[teller], lees_char);
    
    file_string = dag_aan_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    dag_aan_int_array[teller] = lees_string.toInt();

    file_string = uur_aan_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    uur_aan_int_array[teller] = lees_string.toInt();

    file_string = minuut_aan_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    minuut_aan_int_array[teller] = lees_string.toInt();

    file_string = uur_uit_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    uur_uit_int_array[teller] = lees_string.toInt();
    
    file_string = minuut_uit_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    minuut_uit_int_array[teller] = lees_string.toInt();
  }
  for(int teller = 0; teller < 3; teller++){
    file_string = temperatuur_label_string + String(teller);
    file_string.toCharArray(file_char, file_string.length() + 1);
    read_string(SPIFFS, file_char);
    memset(temperatuur_label_char_array[teller], 0, sizeof(temperatuur_label_char_array[teller]));
    strcpy(temperatuur_label_char_array[teller], lees_char);
  }
}

void tijd_controle(){
  for(int temp_int = 0; temp_int < 8; temp_int++){
    if((dag_aan_int_array[temp_int] == 8) || ((dag_aan_int_array[temp_int] == 9) && (local_dag_int < 6)) 
      || ((dag_aan_int_array[temp_int] == 10) && (local_dag_int == 6) || (local_dag_int == 7)) 
      || (dag_aan_int_array[temp_int] == local_dag_int)){
      bitWrite(dag_byte, temp_int, 1);
    }
    else{
      bitWrite(dag_byte, temp_int, 0);
    }
  }

  for(int temp_int = 0; temp_int < 8; temp_int++){
    if(uur_aan_int_array[temp_int] == 24){
      bitWrite(tijd_override_byte, temp_int, 1);
    }
    else{
      bitWrite(tijd_override_byte, temp_int, 0);
    }
  }
  
  for(int temp_int = 0; temp_int < 8; temp_int++){
    if((uur_aan_int_array[temp_int] == uren_int) && (minuut_aan_int_array[temp_int] == minuten_int)){
      bitWrite(tijd_byte, temp_int, 1);
    }
  }
  
  for(int temp_int = 0; temp_int < 8; temp_int++){
    if((uur_uit_int_array[temp_int] == uren_int) && (minuut_uit_int_array[temp_int] == minuten_int)){
      bitWrite(tijd_byte, temp_int, 0);
    }
  }
}

void ledstrip_uitgang(){
  if(!(bitRead(uitgang_actief_byte, 0))){
      strip.setPixelColor(0, strip.Color(0, 0, 0)); 
    }
    else{
      if(!bitRead(uitgang_terug_byte, 0)){
        strip.setPixelColor(0, strip.Color(0xff, 0xff, 0)); 
      }
      else{
        if(!bitRead(uitgang_byte, 0)){
          strip.setPixelColor(0, strip.Color(0, 0xff, 0));
        }
        else{
          strip.setPixelColor(0, strip.Color(0xff, 0, 0));
        }
      }
    }
  if(!(bitRead(uitgang_actief_byte, 1))){
    strip.setPixelColor(1, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 1)){
      strip.setPixelColor(1, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 1)){
        strip.setPixelColor(1, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(1, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!(bitRead(uitgang_actief_byte, 2))){
    strip.setPixelColor(2, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 2)){
      strip.setPixelColor(2, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 2)){
        strip.setPixelColor(2, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(2, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!(bitRead(uitgang_actief_byte, 3))){
    strip.setPixelColor(3, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 3)){
      strip.setPixelColor(3, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 3)){
        strip.setPixelColor(3, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(3, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!(bitRead(uitgang_actief_byte, 4))){
    strip.setPixelColor(4, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 4)){
      strip.setPixelColor(4, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 4)){
        strip.setPixelColor(4, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(4, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!(bitRead(uitgang_actief_byte, 5))){
    strip.setPixelColor(5, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 5)){
      strip.setPixelColor(5, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 5)){
        strip.setPixelColor(5, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(5, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!(bitRead(uitgang_actief_byte, 6))){
    strip.setPixelColor(6, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 6)){
      strip.setPixelColor(6, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 6)){
        strip.setPixelColor(6, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(6, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!(bitRead(uitgang_actief_byte, 7))){
    strip.setPixelColor(7, strip.Color(0, 0, 0)); 
  }
  else{
    if(!bitRead(uitgang_terug_byte, 7)){
      strip.setPixelColor(7, strip.Color(0xff, 0xff, 0)); 
    }
    else{
      if(!bitRead(uitgang_byte, 7)){
        strip.setPixelColor(7, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(7, strip.Color(0xff, 0, 0));
      }
    }
  }
}

void ledstrip_ingang(){
  if(!bitRead(ingang_actief_byte, 0)){
      strip.setPixelColor(8, strip.Color(0, 0, 0));
    }
    else{
      if(!bitRead(ingang_terug_byte, 0)){
        strip.setPixelColor(8, strip.Color(0xff, 0xff, 0));
      }
      else{
        if(!bitRead(ingang_byte, 0)){
          strip.setPixelColor(8, strip.Color(0, 0xff, 0));
        }
        else{
          strip.setPixelColor(8, strip.Color(0xff, 0, 0));
        }
      }
    }
  if(!bitRead(ingang_actief_byte, 1)){
    strip.setPixelColor(9, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 1)){
      strip.setPixelColor(9, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 1)){
        strip.setPixelColor(9, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(9, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!bitRead(ingang_actief_byte, 2)){
    strip.setPixelColor(10, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 2)){
      strip.setPixelColor(10, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 2)){
        strip.setPixelColor(10, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(10, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!bitRead(ingang_actief_byte, 3)){
    strip.setPixelColor(11, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 3)){
      strip.setPixelColor(11, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 3)){
        strip.setPixelColor(11, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(11, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!bitRead(ingang_actief_byte, 4)){
    strip.setPixelColor(12, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 4)){
      strip.setPixelColor(12, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 4)){
        strip.setPixelColor(12, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(12, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!bitRead(ingang_actief_byte, 5)){
    strip.setPixelColor(13, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 5)){
      strip.setPixelColor(13, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 5)){
        strip.setPixelColor(13, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(13, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!bitRead(ingang_actief_byte, 6)){
    strip.setPixelColor(14, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 6)){
      strip.setPixelColor(14, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 6)){
        strip.setPixelColor(14, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(14, strip.Color(0xff, 0, 0));
      }
    }
  }
  if(!bitRead(ingang_actief_byte, 7)){
    strip.setPixelColor(15, strip.Color(0, 0, 0));
  }
  else{
    if(!bitRead(ingang_terug_byte, 7)){
      strip.setPixelColor(15, strip.Color(0xff, 0xff, 0));
    }
    else{
      if(!bitRead(ingang_byte, 7)){
        strip.setPixelColor(15, strip.Color(0, 0xff, 0));
      }
      else{
        strip.setPixelColor(15, strip.Color(0xff, 0, 0));
      }
    }
  }
}

void IRAM_ATTR minuut_voorbij() {
  minuut_interrupt_bool = true;
}

/*
 * HOOFDPAGINA
 */
 
const char rc_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>domotica</title>
    <meta name="viewport" content="width=device-width, initial-scale=0.9">
    <style>
        div.kader {
          position: relative;
          left: 0px;
          width: 400px;
        }
        div.links{
          position: absolute;
          left: 0px;
          width: 200px;
        }
        div.rechts{
          position: absolute;
          left: 200px;
          width: 200px;
        }
        div.positie_1{
          position: absolute;
          left: 0px;
          width: 20px;
        }
        div.positie_2{
          position: absolute;
          left: 50px;
          width: 80px;
        }
        div.positie_3{
          position: absolute;
          left: 150px;
          width: 30px;
        }
        div.positie_4{
          position: absolute;
          left: 190px;
          width: 40px;
        }
        div.positie_5{
          position: absolute;
          left: 250px;
          width: 40px;
        }
        div.positie_6{
          position: absolute;
          left: 310px;
          width: 20px;
        }
        div.positie_7{
          position: absolute;
          left: 360px;
          width: 20px;
        }
        div.positie_8{
          position: absolute;
          left: 200px;
          width: 90px;
        }
        div.positie_9{
          position: absolute;
          left: 320px;
          width: 40px;
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_30{
          width: auto;
          height: 30px;
        }
        div.blanco_35{
          width: auto;
          height: 35px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
        div.blanco_60{
          width: auto;
          height: 60px;
        }
    </style>
  </head>
  <body>
    <b><center>uitgangen</center></b>
    <div class="blanco_20">&nbsp;</div>
    <b>
      <div class="kader">
        <div class="positie_3">
          <center>dag</center>
        </div>
        <div class="positie_4">
          <center>aan</center>
        </div>
        <div class="positie_5">
          <center>uit</center>
        </div>
        <div class="positie_6">
          <center>in</center>
        </div>
        <div class="positie_7">
          <center>M</center>
        </div>
      </div>
    </b>
    <div class="blanco_35">&nbsp;</div>
    <form action="/get" target="hidden-form">
    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="0" name="uitgang_0" onclick="bevestig()" style="background-color: %kleur_0%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_0%</left>
      </div>
      <div class="positie_3">
        <center>%dag_0%</center>
      </div>
      <div class="positie_4">
        <center>%aan_0%</center>
      </div>
      <div class="positie_5">
        <center>%uit_0%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_0%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_0" onclick="bevestig()" style="background-color: %a_m_0%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>
    
    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="1" name="uitgang_1" onclick="bevestig()" style="background-color: %kleur_1%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_1%</left>
      </div>
      <div class="positie_3">
        <center>%dag_1%</center>
      </div>
      <div class="positie_4">
        <center>%aan_1%</center>
      </div>
      <div class="positie_5">
        <center>%uit_1%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_1%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_1" onclick="bevestig()" style="background-color: %a_m_1%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>

    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="2" name="uitgang_2" onclick="bevestig()" style="background-color: %kleur_2%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_2%</left>
      </div>
      <div class="positie_3">
        <center>%dag_2%</center>
      </div>
      <div class="positie_4">
        <center>%aan_2%</center>
      </div>
      <div class="positie_5">
        <center>%uit_2%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_2%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_2" onclick="bevestig()" style="background-color: %a_m_2%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>

    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="3" name="uitgang_3" onclick="bevestig()" style="background-color: %kleur_3%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_3%</left>
      </div>
      <div class="positie_3">
        <center>%dag_3%</center>
      </div>
      <div class="positie_4">
        <center>%aan_3%</center>
      </div>
      <div class="positie_5">
        <center>%uit_3%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_3%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_3" onclick="bevestig()" style="background-color: %a_m_3%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>

    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="4" name="uitgang_4" onclick="bevestig()" style="background-color: %kleur_4%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_4%</left>
      </div>
      <div class="positie_3">
        <center>%dag_4%</center>
      </div>
      <div class="positie_4">
        <center>%aan_4%</center>
      </div>
      <div class="positie_5">
        <center>%uit_4%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_4%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_4" onclick="bevestig()" style="background-color: %a_m_4%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>

    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="5" name="uitgang_5" onclick="bevestig()" style="background-color: %kleur_5%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_5%</left>
      </div>
      <div class="positie_3">
        <center>%dag_5%</center>
      </div>
      <div class="positie_4">
        <center>%aan_5%</center>
      </div>
      <div class="positie_5">
        <center>%uit_5%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_5%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_5" onclick="bevestig()" style="background-color: %a_m_5%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>

    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="6" name="uitgang_6" onclick="bevestig()" style="background-color: %kleur_6%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_6%</left>
      </div>
      <div class="positie_3">
        <center>%dag_6%</center>
      </div>
      <div class="positie_4">
        <center>%aan_6%</center>
      </div>
      <div class="positie_5">
        <center>%uit_6%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_6%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_6" onclick="bevestig()" style="background-color: %a_m_6%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>

    <div class="kader">
      <div class="positie_1">
        <left><input type="submit"  value="7" name="uitgang_7" onclick="bevestig()" style="background-color: %kleur_7%; opacity: 0.5;" ></left>
      </div>
      <div class="positie_2">
        <left>%label_7%</left>
      </div>
      <div class="positie_3">
        <center>%dag_7%</center>
      </div>
      <div class="positie_4">
        <center>%aan_7%</center>
      </div>
      <div class="positie_5">
        <center>%uit_7%</center>
      </div>
      <div class="positie_6">
        <center>%ingang_7%</center>
      </div>
      <div class="positie_7">
        <center><input type="submit" value=" " name="manueel_7" onclick="bevestig()" style="background-color: %a_m_7%; opacity: 0.5;"></center>
      </div>
    </div>
    <div class="blanco_35">&nbsp;</div>
    </form>
    <b>
      <div class="kader">
        <div class="links">
          <center>ingangen</center>
        </div>
      </div>
    </b>
    <div class="blanco_35">&nbsp;</div>
    <form action="/get" target="hidden-form">
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="0" name="ingang_0" onclick="bevestig()" style="background-color: %in_kleur_0%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_0%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
      
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="1" name="ingang_1" onclick="bevestig()" style="background-color: %in_kleur_1%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_1%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
      
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="2" name="ingang_2" onclick="bevestig()" style="background-color: %in_kleur_2%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_2%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
  
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="3" name="ingang_3" onclick="bevestig()" style="background-color: %in_kleur_3%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_3%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
  
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="4" name="ingang_4" onclick="bevestig()" style="background-color: %in_kleur_4%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_4%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
  
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="5" name="ingang_5" onclick="bevestig()" style="background-color: %in_kleur_5%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_5%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
  
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="6" name="ingang_6" onclick="bevestig()" style="background-color: %in_kleur_6%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_6%</center>
        </div>
      </div>
      <div class="blanco_35">&nbsp;</div>
  
      <div class="kader">
        <div class="positie_1">
          <left><input type="submit" value="7" name="ingang_7" onclick="bevestig()" style="background-color: %in_kleur_7%; opacity: 0.5;"></left>
        </div>
        <div class="positie_2">
          <center>%in_label_7%</center>
        </div>
      </div>
    </form>    
    <div class="blanco_40">&nbsp;</div>
    <div class="kader">
      <div class="links">
        <center><input type="submit" value="klok" onclick="klok()"></button></center>
      </div>
      <div class="rechts">
        <center><input type="submit" value="instellen" onclick="instellen()"></button></center>
      </div>
    </div>
    <div class="blanco_40">&nbsp;</div> 
    <small><b>thieu-b55 februari 2023</b></small>
    <script>
      function bevestig(){
        setTimeout(function(){document.location.reload();},1000);
      }
      function klok(){
        location.assign("/klok");
      }
      function instellen(){
        location.assign("/instellen");
      }
    </script>
  </body>  
</html>
)rawliteral";

/*
 * 
 * INSTELLEN DOMOTICA
 * 
 */

const char instellen_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Domotica instellen</title>
    <meta name="viewport" content="width=device-width, initial-scale=0.9">
    <style>
        div.kader {
          position: relative;
          left: 0px;
          width: 400px;
        }
        div.kader_1 {
          position: absolute;
          left : 0px;
          width: 80px;
        }
        div.kader_2 {
          position: absolute;
          left : 80px;
          width: 80px;
        }
        div.kader_3 {
          position: absolute;
          left : 160px;
          width: 80px;
        }
        div.kader_4 {
          position: absolute;
          left : 240px;
          width: 80px;
        }
        div.kader_5 {
          position: absolute;
          left : 320px;
          width: 80px;
        }
        div.vak_3 {
          position: absolute;
          left : 134px;
          width: 67px;
        }
        div.vak_4 {
          position: absolute;
          left : 201px;
          width: 67px;
        }
        div.blanco_10{
          width: auto;
          height: 10px;
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_30{
          width: auto;
          height: 30px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
        div.blanco_50{
          width: auto;
          height: 50px;
        }
        div.blanco_60{
          width: auto;
          height: 60px;
        }
    </style>
  </head>
  <body>
    <h4><center>Instellen domotica</center></h4>
    <h5><center>Instellen uitgangen</center></h5>
    <form action="/get" target="hidden-form">
      <div class="kader">
        <div class="kader_2">
          <center><input type="submit" value=" - " name="uitgang_min" onclick="bevestig()"></center>
        </div>
        <div class="kader_3">
          <center>%uitgang_nu%</center>
        </div>
        <div class="kader_4">
          <center><input type="submit" value=" + " name="uitgang_plus" onclick="bevestig()"></center>
        </div>
      </div>
    </form>
    <form action="/get" target="hidden-form">
      <small>
        <b>
          <div class="blanco_50">&nbsp;</div>
          <div class="kader">
            <div class="kader_1">
              <center>Label</center>
            </div>
            <div class="kader_2">
              <center>Dag</center>
            </div>
            <div class="kader_3">
              <center>Aan</center>
            </div>
            <div class="kader_4">
              <center>Uit</center>
            </div>
            <div class="kader_5">
              <center>In</center>
            </div>
          </div>
        </b>
        <div class="blanco_40">&nbsp;</div>
        <div class="kader">
          <div class="kader_1">
            <center><input type="text" value="%label_nu%" name="label_in" style="text-align:center" size=3></center>
          </div>
            <div class="kader_2">
              <center><input type="text" value="%dag_int%" name="dag_in" style="text-align:center" size=1></center>
            </div>
          <div class="kader_3">
            <center><input type="text" value="%aan_nu%" name="aan_in" style="text-align:center" size=1></center>
          </div>
          <div class="kader_4">
            <center><input type="text" value="%uit_nu%" name="uit_in" style="text-align:center" size=1></center>
          </div>
          <div class="kader_5">
            <center><input type="text" value="%in_nu%" name="in_in" style="text-align:center" size=1></center>
          </div>
        </div>
        <div class="blanco_30">&nbsp;</div> 
        <div class="kader">
          <div class="kader_2">
              <center>%dag_nu%</center>
          </div>
        </div>
        <div class="blanco_40">&nbsp;</div> 
        <div class="kader">
          <center><input type="submit" value="Bevestig" name="bevestig_uit" onclick="bevestig()" size=2></center>
        </div>
      </small>
    </form>  
    <div class="blanco_20">&nbsp;</div>     
    <h5><center>Instellen ingangen</center></h5>
    <form action="/get" target="hidden-form">
      <div class="kader">
        <div class="kader_2">
          <center><input type="submit" value=" - " name="ingang_min" onclick="bevestig()"></center>
        </div>
        <div class="kader_3">
          <center>%ingang_nu%</center>
        </div>
        <div class="kader_4">
          <center><input type="submit" value=" + " name="ingang_plus" onclick="bevestig()"></center>
        </div>
      </div>
    </form>
    <form action="/get" target="hidden-form">
      <small>
        <b>
          <div class="blanco_50">&nbsp;</div>
          <div class="kader">
            <div class="kader_3">
              <center>Label</center>
            </div>
          </div>
        </b>
        <div class="blanco_40">&nbsp;</div>
        <div class="kader">
          <div class="kader_3">
            <center><input type="text" value="%in_label_nu%" name="in_label_in" style="text-align:center" size=3></center>
          </div>
        </div>
        <div class="blanco_50">&nbsp;</div> 
        <div class="kader">
          <center><input type="submit" value="Bevestig" name="bevestig_in" onclick="bevestig()" size=2></center>
        </div>
      </small>
    </form>
    <div class="blanco_60">&nbsp;</div>
    <div class="kader">
      <center><input type="submit" value="begin pagina" onclick="begin_pagina()" size=2></center>
    </div>
    <script>
      function bevestig(){
        setTimeout(function(){document.location.reload();},250);
      }
      function begin_pagina(){
        location.assign("/");
      }
    </script>
  </body>  
</html>
)rawliteral";
/*
 * INSTELLEN KLOK
 */
const char klok_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Klok instellen</title>
    <meta name="viewport" content="width=device-width, initial-scale=0.9">
    <style>
        div.kader {
          position: relative;
          left: 0px;
          width: 400px;
        }
        div.kader_2 {
          position: absolute;
          left : 80px;
          width: 80px;
        }
        div.kader_3 {
          position: absolute;
          left : 160px;
          width: 80px;
        }
        div.kader_4 {
          position: absolute;
          left : 240px;
          width: 80px;
        }
        div.vak_3 {
          position: absolute;
          left : 134px;
          width: 67px;
        }
        div.vak_4 {
          position: absolute;
          left : 201px;
          width: 67px;
        }
        div.blanco_10{
          width: auto;
          height: 10px;
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_30{
          width: auto;
          height: 30px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
        div.blanco_60{
          width: auto;
          height: 60px;
        }
    </style>
  </head>
  <body>
    <h5><center>Instellen klok</center></h5>
    <h5><center>Instellen tijdzone en zomeruur</center></h5>
    <small>
      <form action="/get" target="hidden-form">
        <div class="kader">
          <div class="vak_3">
            <center>%dag%</center>
          </div>    
          <div class="vak_4">
            <center>%tijd%</center> 
          </div>
        </div>
        <div class="blanco_30">&nbsp;</div>
        <div class="kader">
          <div class="kader_2">
            <center>uren</center>
          </div>
          <div class="kader_3">
            <center>minuten</center>
          </div>    
          <div class="kader_4">
            <center>zomertijd</center> 
          </div>
        </div>
        <div class="blanco_30">&nbsp;</div>
        <div class="kader">
          <div class="kader_2">
            <center><input type= "text" style="text-align:center;" value=%tijd_zone_uur% name="tz_uur" size = 1></center>
          </div>
          <div class="kader_3">
            <center><input type= "text" style="text-align:center;" value=%tijd_zone_minuut% name="tz_minuut" size = 1></center>
          </div>    
          <div class="kader_4">
            <center><input type= "text" style="text-align:center;" value=%zomertijd% name="zomer" size = 1></center>
          </div>
        </div>
        <div class="blanco_40">&nbsp;</div>
        <div class="kader">
          <div class="kader_3">
            <center><input type="submit" value="OK" name="tz_bevestig" onclick="bevestig()"></center>
         </div>
        </div>
      </form>
    </small>
    <div class="blanco_40">&nbsp;</div>
    <div class="kader">
      <center><small><b>Led helderheid (0 <-> 15)</b></small></center>
    </div>
    <div class="blanco_20">&nbsp;</div>
    <form action="/get" target="hidden-form">
      <div class="kader">
        <center><input type= "text" style="text-align:center;" value=%intensiteit% name="intensiteit_gewenst" size = 1></center>
      </div>   
      <div class="blanco_20">&nbsp;</div>
      <div class="kader">
        <center><input type="submit" value="OK" name="intensiteit_bevestig" onclick="bevestig()"></center>
      </div>
    </form>
    <div class="blanco_60">&nbsp;</div>
    <form action="/get" target="hidden-form">
    <div class="kader">
      <center>
        <input type="submit" value="begin pagina" onclick="begin_pagina()"></button>
      </center>
    </div>
    </form>
    <script>
      function bevestig(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
    <script>
      function begin_pagina(){
        location.assign("/");
      }
    </script>
  </body>  
</html>
)rawliteral";

/*
 * Instellen NETWERK
 */

const char netwerk_html[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <iframe style="display:none" name="hidden-form"></iframe>
    <title>Internetradio bediening</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        div.kader {
          position: relative;
          width: 400px;
          height: 12x;
        }
        div.links{
          position: absolute;
          left : 0px;
          width; auto;
          height: 12px;
        }
        div.links_midden{
          position:absolute;
          left:  80px;
          width: auto;
          height: 12px; 
        }
        div.blanco_20{
          width: auto;
          height: 20px;
        }
        div.blanco_40{
          width: auto;
          height: 40px;
        }
    </style>
  </head>
  <body>
    <h5><center><strong>ESP32 Netwerk instellingen</strong></center></h5>
    <form action="/get">
      <small>
        <div class="kader">
          <div class="links"><b>ssid :</b></div>
          <div class="links_midden"><input type="text" style="text-align:center;" name="sta_ssid"></div>
        </div>
        <div class="blanco_40">&nbsp;</div>
        <div class="kader">
          <div class="links"><b>pswd :</b></div>
          <div class="links_midden"><input type="text" style="text-align:center;" name="sta_pswd"></div>
        </div>
        <div class="blanco_20">&nbsp;</div>
      </small>
      <h5><center>Gewenst IP address (default 192.168.1.222)</center></h5>
      <small>
        <div class="kader">
          <center>
            <input type="text" style="text-align:center;" value="%ip_address_1%" name="ip_1_keuze" size=1>
            &nbsp;&nbsp;
            <input type="text" style="text-align:center;" value="%ip_address_2%" name="ip_2_keuze" size=1>
            &nbsp;&nbsp;
            <input type="text" style="text-align:center;" value="%ip_address_3%" name="ip_3_keuze" size=1>
            &nbsp;&nbsp;
            <input type="text" style="text-align:center;" value="%ip_address_4%" name="ip_4_keuze" size=1>
          </center>
        </div>
        <div class="blanco_20">&nbsp;</div>
      </small>
      <center><input type="submit" value="Bevestig" onclick="ok()"></center>
    </form>
    <br>
    <script>
      function ok(){
        setTimeout(function(){document.location.reload();},250);
      }
    </script>
  </body>  
</html>
)rawliteral";


String processor(const String& var){
  char tijd_aan_char[6];
  char tijd_uit_char[6];

  /*
   * Netwerk instellen
   */
   
  if(var == "ip_address_1"){
    return(String(ip_1_int));
  }
  if(var == "ip_address_2"){
    return(String(ip_2_int));
  }
  if(var == "ip_address_3"){
    return(String(ip_3_int));
  }
  if(var == "ip_address_4"){
    return(String(ip_4_int));
  }
  /*
   * Klok instellen
   */
  if(var == "dag"){
    return(week_dagen_char[dag_int]);
  }
  if(var == "tijd"){
    char tijd_char[6];
    sprintf(tijd_char, "%02d:%02d", uren_int, minuten_int);
    return(tijd_char);
  }
  if(var == "tijd_zone_uur"){
    return(String(utc_offset_uren_int));
  }
  if(var == "tijd_zone_minuut"){
    return(String(utc_offset_minuten_int));
  }
  if(var == "zomertijd"){
    return(String(winter_zomer_int));
  }
  if(var == "intensiteit"){
    return(String(intensiteit_int));
  }
  /* 
   *  Hoofdpagine
   */
  if(var == "kleur_0"){
    if(!bitRead(uitgang_actief_byte, 0)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 0)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 0)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_0"){
    return(uitgang_label_char_array[0]);
  }
  if(var == "dag_0"){
    return(String(dag_aan_int_array[0]));
  }
  if(var == "aan_0"){
    sprintf(aan_tijd_0_char, "%02d:%02d", uur_aan_int_array[0], minuut_aan_int_array[0]);
    return(aan_tijd_0_char);
  }
  if(var == "uit_0"){
    sprintf(uit_tijd_0_char, "%02d:%02d", uur_uit_int_array[0], minuut_uit_int_array[0]);
    return(uit_tijd_0_char);
  }
  if(var == "ingang_0"){
    return(uitgang_in_char_array[0]);
  }
  if(var == "a_m_0"){
    if(!(bitRead(manueel_byte, 0))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_1"){
    if(!(bitRead(uitgang_actief_byte, 1))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 1)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 1)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_1"){
    return(uitgang_label_char_array[1]);
  }
  if(var == "dag_1"){
    return(String(dag_aan_int_array[1]));
  }
  if(var == "aan_1"){
    sprintf(aan_tijd_1_char, "%02d:%02d", uur_aan_int_array[1], minuut_aan_int_array[1]);
    return(aan_tijd_1_char);
  }
  if(var == "uit_1"){
    sprintf(uit_tijd_1_char, "%02d:%02d", uur_uit_int_array[1], minuut_uit_int_array[1]);
    return(uit_tijd_1_char);
  }
  if(var == "ingang_1"){
    return(uitgang_in_char_array[1]);
  }
  if(var == "a_m_1"){
     if(!(bitRead(manueel_byte, 1))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_2"){
    if(!(bitRead(uitgang_actief_byte, 2))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 2)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 2)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_2"){
    return(uitgang_label_char_array[2]);
  }
  if(var == "dag_2"){
    return(String(dag_aan_int_array[3]));
  }
  if(var == "aan_2"){
    sprintf(aan_tijd_2_char, "%02d:%02d", uur_aan_int_array[2], minuut_aan_int_array[2]);
    return(aan_tijd_2_char);
  }
  if(var == "uit_2"){
    sprintf(uit_tijd_2_char, "%02d:%02d", uur_uit_int_array[2], minuut_uit_int_array[2]);
    return(uit_tijd_2_char);
  }
  if(var == "ingang_2"){
    return(uitgang_in_char_array[2]);
  }
  if(var == "a_m_2"){
     if(!(bitRead(manueel_byte, 2))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_3"){
    if(!(bitRead(uitgang_actief_byte, 3))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 3)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 3)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_3"){
    return(uitgang_label_char_array[3]);
  }
  if(var == "dag_3"){
    return(String(dag_aan_int_array[3]));
  }
  if(var == "aan_3"){
    sprintf(aan_tijd_3_char, "%02d:%02d", uur_aan_int_array[3], minuut_aan_int_array[3]);
    return(aan_tijd_3_char);
  }
  if(var == "uit_3"){
    sprintf(uit_tijd_3_char, "%02d:%02d", uur_uit_int_array[3], minuut_uit_int_array[3]);
    return(uit_tijd_3_char);
  }
  if(var == "ingang_3"){
    return(uitgang_in_char_array[3]);
  }
  if(var == "a_m_3"){
     if(!(bitRead(manueel_byte, 3))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_4"){
    if(!(bitRead(uitgang_actief_byte, 4))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 4)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 4)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_4"){
    return(uitgang_label_char_array[4]);
  }
  if(var == "dag_4"){
    return(String(dag_aan_int_array[4]));
  }
  if(var == "aan_4"){
    sprintf(aan_tijd_4_char, "%02d:%02d", uur_aan_int_array[4], minuut_aan_int_array[4]);
    return(aan_tijd_4_char);
  }
  if(var == "uit_4"){
    sprintf(uit_tijd_4_char, "%02d:%02d", uur_uit_int_array[4], minuut_uit_int_array[4]);
    return(uit_tijd_4_char);
  }
  if(var == "ingang_4"){
    return(uitgang_in_char_array[4]);
  }
  if(var == "a_m_4"){
     if(!(bitRead(manueel_byte, 4))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_5"){
    if(!(bitRead(uitgang_actief_byte, 5))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 5)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 5)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_5"){
    return(uitgang_label_char_array[5]);
  }
  if(var == "dag_5"){
    return(String(dag_aan_int_array[5]));
  }
  if(var == "aan_5"){
    sprintf(aan_tijd_5_char, "%02d:%02d", uur_aan_int_array[5], minuut_aan_int_array[5]);
    return(aan_tijd_5_char);
  }
  if(var == "uit_5"){
    sprintf(uit_tijd_5_char, "%02d:%02d", uur_uit_int_array[5], minuut_uit_int_array[5]);
    return(uit_tijd_5_char);
  }
  if(var == "ingang_5"){
    return(uitgang_in_char_array[5]);
  }
  if(var == "a_m_5"){
     if(!(bitRead(manueel_byte, 5))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_6"){
    if(!(bitRead(uitgang_actief_byte, 6))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 6)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 6)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_6"){
    return(uitgang_label_char_array[6]);
  }
  if(var == "dag_6"){
    return(String(dag_aan_int_array[6]));
  }
  if(var == "aan_6"){
    sprintf(aan_tijd_6_char, "%02d:%02d", uur_aan_int_array[6], minuut_aan_int_array[6]);
    return(aan_tijd_6_char);
  }
  if(var == "uit_6"){
    sprintf(uit_tijd_6_char, "%02d:%02d", uur_uit_int_array[6], minuut_uit_int_array[6]);
    return(uit_tijd_6_char);
  }
  if(var == "ingang_6"){
    return(uitgang_in_char_array[6]);
  }
  if(var == "a_m_6"){
     if(!(bitRead(manueel_byte, 6))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "kleur_7"){
    if(!(bitRead(uitgang_actief_byte, 7))){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(uitgang_terug_byte, 7)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(uitgang_byte, 7)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "label_7"){
    return(uitgang_label_char_array[7]);
  }
  if(var == "dag_7"){
    return(String(dag_aan_int_array[7]));
  }
  if(var == "aan_7"){
    sprintf(aan_tijd_7_char, "%02d:%02d", uur_aan_int_array[7], minuut_aan_int_array[7]);
    return(aan_tijd_7_char);
  }
  if(var == "uit_7"){
    sprintf(uit_tijd_7_char, "%02d:%02d", uur_uit_int_array[7], minuut_uit_int_array[7]);
    return(uit_tijd_7_char);
  }
  if(var == "ingang_7"){
    return(uitgang_in_char_array[7]);
  }
  if(var == "a_m_7"){
     if(!(bitRead(manueel_byte, 7))){
      return("#FFFFFF");
    }
    else{
      return("#0000FF");
    }
  }
  if(var == "in_label_0"){
    return(ingang_label_char_array[0]);
  }
  if(var == "in_label_1"){
    return(ingang_label_char_array[1]);
  }
  if(var == "in_label_2"){
    return(ingang_label_char_array[2]);
  }
  if(var == "in_label_3"){
    return(ingang_label_char_array[3]);
  }
  if(var == "in_label_4"){
    return(ingang_label_char_array[4]);
  }
  if(var == "in_label_5"){
    return(ingang_label_char_array[5]);
  }
  if(var == "in_label_6"){
    return(ingang_label_char_array[6]);
  }
  if(var == "in_label_7"){
    return(ingang_label_char_array[7]);
  }
  if(var == "in_kleur_0"){
    if(!bitRead(ingang_actief_byte, 0)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 0)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 0)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_1"){
    if(!bitRead(ingang_actief_byte, 1)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 1)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 1)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_2"){
    if(!bitRead(ingang_actief_byte, 2)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 2)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 2)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_3"){
    if(!bitRead(ingang_actief_byte, 3)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 3)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 3)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_4"){
    if(!bitRead(ingang_actief_byte, 4)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 4)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 4)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_5"){
    if(!bitRead(ingang_actief_byte, 5)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 5)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 5)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_6"){
    if(!bitRead(ingang_actief_byte, 6)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 6)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 6)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  if(var == "in_kleur_7"){
    if(!bitRead(ingang_actief_byte, 7)){
      return("#FFFFFF");
    }
    else{
      if(!bitRead(ingang_terug_byte, 7)){
        return("#FFFF00");
      }
      else{
        if(!bitRead(ingang_byte, 7)){
          return("#00FF00");
        }
        else{
          return("#FF0000");
        }
      }
    }
  }
  /*
   * Instellen ingangen / uitgangen
   */
  if(var == "uitgang_nu"){
    return(String(uitgang_teller_int)); 
  }
  if(var == "label_nu"){
    return(uitgang_label_char_array[uitgang_teller_int]);
  }
  if(var == "dag_int"){
    return(String(dag_aan_int_array[uitgang_teller_int]));
  }
  if(var == "dag_nu"){
    return(week_dagen_char[dag_aan_int_array[uitgang_teller_int]]);
  }
  if(var == "aan_nu"){
    sprintf(tijd_aan_char, "%02d:%02d", uur_aan_int_array[uitgang_teller_int], minuut_aan_int_array[uitgang_teller_int]);
    return(tijd_aan_char);
  }
  if(var == "uit_nu"){
    sprintf(tijd_uit_char, "%02d:%02d", uur_uit_int_array[uitgang_teller_int], minuut_uit_int_array[uitgang_teller_int]);
    return(tijd_uit_char);
  }
  if(var == "in_nu"){
    return(uitgang_in_char_array[uitgang_teller_int]);
  }
  if(var == "ingang_nu"){
    return(String(ingang_teller_int)); 
  }
  if(var == "in_label_nu"){
    return(ingang_label_char_array[ingang_teller_int]);
  }
}

void html_input(){
  server.begin();
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
  
  if(netwerk_bool){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", rc_html, processor);
    });
    server.on("/klok", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", klok_html, processor);
    });
    server.on("/instellen", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", instellen_html, processor);
    });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      char terminator_char = char(0x0a);
      char in_char[5];
      char x_char[] = {"x"};
      int dag_in_int;
      int temp;
      int temp_uur_int;
      int temp_minuut_int;
      /*
       * DOMOTICA 
       */
      if(request->hasParam(UITGANG_0)){
        bitWrite(uitgang_actief_byte, 0, (bitRead(uitgang_actief_byte, 0) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_1)){
        bitWrite(uitgang_actief_byte, 1, (bitRead(uitgang_actief_byte, 1) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_2)){
        bitWrite(uitgang_actief_byte, 2, (bitRead(uitgang_actief_byte, 2) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_3)){
        bitWrite(uitgang_actief_byte, 3, (bitRead(uitgang_actief_byte, 3) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_4)){
        bitWrite(uitgang_actief_byte, 4, (bitRead(uitgang_actief_byte, 4) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_5)){
        bitWrite(uitgang_actief_byte, 5, (bitRead(uitgang_actief_byte, 5) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_6)){
        bitWrite(uitgang_actief_byte, 6, (bitRead(uitgang_actief_byte, 6) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(UITGANG_7)){
        bitWrite(uitgang_actief_byte, 7, (bitRead(uitgang_actief_byte, 7) ^ 1));
        pref.putShort("act", uitgang_actief_byte);
        uitgang_actief_byte = pref.getShort("act");
      }
      if(request->hasParam(MANUEEL_0)){
        bitWrite(manueel_byte, 0, (bitRead(manueel_byte, 0) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_1)){
        bitWrite(manueel_byte, 1, (bitRead(manueel_byte, 1) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_2)){
        bitWrite(manueel_byte, 2, (bitRead(manueel_byte, 2) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_3)){
        bitWrite(manueel_byte, 3, (bitRead(manueel_byte, 3) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_4)){
        bitWrite(manueel_byte, 4, (bitRead(manueel_byte, 4) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_5)){
        bitWrite(manueel_byte, 5, (bitRead(manueel_byte, 5) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_6)){
        bitWrite(manueel_byte, 6, (bitRead(manueel_byte, 6) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(MANUEEL_7)){
        bitWrite(manueel_byte, 7, (bitRead(manueel_byte, 7) ^ 1));
        pref.putShort("man", manueel_byte);
        manueel_byte = pref.getShort("man");
      }
      if(request->hasParam(INGANG_0)){
        bitWrite(ingang_actief_byte, 0, (bitRead(ingang_actief_byte, 0) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_1)){
        bitWrite(ingang_actief_byte, 1, (bitRead(ingang_actief_byte, 1) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_2)){
        bitWrite(ingang_actief_byte, 2, (bitRead(ingang_actief_byte, 2) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_3)){
        bitWrite(ingang_actief_byte, 3, (bitRead(ingang_actief_byte, 3) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_4)){
        bitWrite(ingang_actief_byte, 4, (bitRead(ingang_actief_byte, 4) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_5)){
        bitWrite(ingang_actief_byte, 5, (bitRead(ingang_actief_byte, 5) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_6)){
        bitWrite(ingang_actief_byte, 6, (bitRead(ingang_actief_byte, 6) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      if(request->hasParam(INGANG_7)){
        bitWrite(ingang_actief_byte, 7, (bitRead(ingang_actief_byte, 7) ^ 1));
        pref.putShort("inact", ingang_actief_byte);
        ingang_actief_byte = pref.getShort("inact");
      }
      /*
       * Klok
       */
      if(request->hasParam(TZ_UUR)){
        utc_offset_uren_int = ((request->getParam(TZ_UUR)->value()) + String(terminator_char)).toInt();
      }
      if(request->hasParam(TZ_MINUUT)){
        utc_offset_minuten_int = ((request->getParam(TZ_MINUUT)->value()) + String(terminator_char)).toInt();
        if((utc_offset_minuten_int != 0) && (utc_offset_minuten_int != 30)){
          utc_offset_minuten_int = 0;
        }
      }
      if(request->hasParam(ZOMER)){
         winter_zomer_int = ((request->getParam(ZOMER)->value()) + String(terminator_char)).toInt();
         if((winter_zomer_int != 0) && (winter_zomer_int != 1)){
          winter_zomer_int = 0;
         }
      }
      if(request->hasParam(TZ_BEVESTIG)){
        pref.putShort("uur_of", utc_offset_uren_int);
        pref.putShort("min_of", utc_offset_minuten_int);
        pref.putShort("w_z", winter_zomer_int);
        tijd_update();
        tijd_naar_led();
      }
      if(request->hasParam(INTENSITEIT_GEWENST)){
        intensiteit_int = ((request->getParam(INTENSITEIT_GEWENST)->value()) + String(terminator_char)).toInt();
        if((intensiteit_int < 0) || (intensiteit_int > 15)){
          intensiteit_int = pref.getShort("intens");
        }
      }
      if(request->hasParam(INTENSITEIT_BEVESTIG)){
        pref.putShort("intens", intensiteit_int);
        intensiteit_int = pref.getShort("intens");
        display_setup(0x0A, intensiteit_int);
      }
      /*
       * instellen DOMOTICA
       */
      if(request->hasParam(UITGANG_PLUS)){
        uitgang_teller_int ++;
        if(uitgang_teller_int > 7){
          uitgang_teller_int = 7;
        }
      }
      if(request->hasParam(UITGANG_MIN)){
        uitgang_teller_int --;
        if(uitgang_teller_int < 0){
          uitgang_teller_int = 0;
        }
      }
      if(request->hasParam(LABEL_IN)){
        label_string = ((request->getParam(LABEL_IN)->value())  + String(terminator_char));
        memset(label_char, 0, sizeof(label_char));
        label_string.toCharArray(label_char, label_string.length());
      }
      if(request->hasParam(DAG_IN)){
        dag_in_int = ((request->getParam(DAG_IN)->value())  + String(terminator_char)).toInt();
      }
      if(request->hasParam(AAN_IN)){
        aan_string = (request->getParam(AAN_IN)->value()) + String(terminator_char);
      }
      if(request->hasParam(UIT_IN)){
        uit_string = (request->getParam(UIT_IN)->value()) + String(terminator_char);
      }
      if(request->hasParam(IN_IN)){
        in_string = (request->getParam(IN_IN)->value()) + String(terminator_char);
      }
      if(request->hasParam(BEVESTIG_UIT)){
        file_string = uitgang_label_string + String(uitgang_teller_int);
        file_string.toCharArray(file_char, file_string.length() + 1);
        write_char(SPIFFS, file_char, label_char);
        read_string(SPIFFS, file_char);
        strcpy(uitgang_label_char_array[uitgang_teller_int], lees_char);
        
        if((dag_in_int > 0) && (dag_in_int < 11)){
          file_string = dag_aan_string + String(uitgang_teller_int);
          file_string.toCharArray(file_char, file_string.length() + 1);
          write_string(SPIFFS, file_char, String(dag_in_int));
          read_string(SPIFFS, file_char);
          dag_aan_int_array[uitgang_teller_int] = lees_string.toInt(); 
        }
        
        temp = aan_string.indexOf(":");
        if(temp != -1){
          temp_uur_int = aan_string.substring(0, temp).toInt();
          temp_minuut_int = aan_string.substring(temp + 1).toInt();
          if((temp_uur_int > -1) && (temp_uur_int < 25) && (temp_minuut_int > - 1) && (temp_minuut_int < 60)){
            file_string = uur_aan_string + String(uitgang_teller_int);
            file_string.toCharArray(file_char, file_string.length() + 1);
            write_string(SPIFFS, file_char, String(temp_uur_int));
            read_string(SPIFFS, file_char);
            uur_aan_int_array[uitgang_teller_int] = lees_string.toInt();
            file_string = minuut_aan_string + String(uitgang_teller_int);
            file_string.toCharArray(file_char, file_string.length() + 1);
            write_string(SPIFFS, file_char, String(temp_minuut_int));
            read_string(SPIFFS, file_char);
            minuut_aan_int_array[uitgang_teller_int] = lees_string.toInt();
          }
        }
        
        temp = uit_string.indexOf(":");
        if(temp != -1){
          temp_uur_int = uit_string.substring(0, temp).toInt();
          temp_minuut_int = uit_string.substring(temp + 1).toInt();
          if((temp_uur_int > -1) && (temp_uur_int < 25) && (temp_minuut_int > - 1) && (temp_minuut_int < 60)){
            file_string = uur_uit_string + String(uitgang_teller_int);
            file_string.toCharArray(file_char, file_string.length() + 1);
            write_string(SPIFFS, file_char, String(temp_uur_int));
            read_string(SPIFFS, file_char);
            uur_uit_int_array[uitgang_teller_int] = lees_string.toInt();
            file_string = minuut_uit_string + String(uitgang_teller_int);
            file_string.toCharArray(file_char, file_string.length() + 1);
            write_string(SPIFFS, file_char, String(temp_minuut_int));
            read_string(SPIFFS, file_char);
            minuut_uit_int_array[uitgang_teller_int] = lees_string.toInt();
          }
        }
        
        in_string.toCharArray(in_char, 2);
        file_string = uitgang_in_string + String(uitgang_teller_int);
        file_string.toCharArray(file_char, file_string.length() + 1);
        if(!((in_char[0] > 0x29) && (in_char[0] < 0x38))){
          write_char(SPIFFS, file_char, x_char);
        }
        else{
          write_char(SPIFFS, file_char, in_char);
        }
        read_string(SPIFFS, file_char);
        memset(uitgang_in_char_array[uitgang_teller_int], 0, sizeof(uitgang_in_char_array[uitgang_teller_int]));
        strcpy(uitgang_in_char_array[uitgang_teller_int], lees_char);
        tijd_controle();
      }
      if(request->hasParam(INGANG_PLUS)){
        ingang_teller_int ++;
        if(ingang_teller_int > 7){
          ingang_teller_int = 7;
        }
      }
      if(request->hasParam(INGANG_MIN)){
        ingang_teller_int --;
        if(ingang_teller_int < 0){
          ingang_teller_int = 0;
        }
      }
      if(request->hasParam(IN_LABEL_IN)){
        label_string = ((request->getParam(IN_LABEL_IN)->value())  + String(terminator_char));
        memset(label_char, 0, sizeof(label_char));
        label_string.toCharArray(label_char, label_string.length());
      }
      if(request->hasParam(BEVESTIG_IN)){
        file_string = ingang_label_string + String(ingang_teller_int);
        file_string.toCharArray(file_char, file_string.length() + 1);
        write_char(SPIFFS, file_char, label_char);
        read_string(SPIFFS, file_char);
        strcpy(ingang_label_char_array[ingang_teller_int], lees_char);
      }
    });
  }
  /*
   * Netwerk instellen
   */
  if(!netwerk_bool){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", netwerk_html, processor);
    });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      char terminator_char = char(0x0a);
       
      if(request->hasParam(STA_SSID)){
        ssid_string = (request->getParam(STA_SSID)->value());
        ssid_string = ssid_string + String(terminator_char);
        pref.putString("ssid", ssid_string);
      }  
      if(request->hasParam(STA_PSWD)){
        pswd_string = (request->getParam(STA_PSWD)->value());
        pswd_string = pswd_string + String(terminator_char);
        pref.putString("pswd", pswd_string);
      }
      if(request->hasParam(IP_1_KEUZE)){
        ip_1_string = (request->getParam(IP_1_KEUZE)->value()) +String(terminator_char);
        ip_1_int = ip_1_string.toInt();
        pref.putShort("ip_1", ip_1_int);
      }
      if(request->hasParam(IP_2_KEUZE)){
        ip_2_string = (request->getParam(IP_2_KEUZE)->value()) +String(terminator_char);
        ip_2_int = ip_2_string.toInt();
        pref.putShort("ip_2", ip_2_int);
      }
      if(request->hasParam(IP_3_KEUZE)){
        ip_3_string = (request->getParam(IP_3_KEUZE)->value()) +String(terminator_char);
        ip_3_int = ip_3_string.toInt();
        pref.putShort("ip_3", ip_3_int);
      }
      if(request->hasParam(IP_4_KEUZE)){
        ip_4_string = (request->getParam(IP_4_KEUZE)->value()) +String(terminator_char);
        ip_4_int = ip_4_string.toInt();
        pref.putShort("ip_4", ip_4_int);
      }
      Serial.println("Restart over 5 seconden");
      delay(5000);
      ESP.restart();
    });
  }
}

void setup(){
  delay(2500);
  strip.begin(); 
  strip.clear();          
  strip.show();            
  strip.setBrightness(25);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX2, TX2);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  pinMode(MINUUT_PULS, INPUT_PULLDOWN);
  SPI.begin();
  display_setup(0x0F, 0x00);
  delay(10);
  display_setup(0x0C, 0x00);
  display_setup(0x0C, 0x01);
  display_setup(0x09, 0x00);
  display_setup(0x0A, 0x00);
  display_setup(0x0B, 0x07);
  smiley();
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(25000);               // slow chinese DS3231 modules
  delay(2500);
  Wire.beginTransmission(DS3231SN);
  Wire.write(0x0B);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x80);
  Wire.write(0x46);
  Wire.write(0x00);
  Wire.endTransmission();
  SPIFFS.begin();
  pref.begin("rc", false);
  //pref.clear();
  if(pref.getString("controle") != "dummy geladen"){
    Serial.println("SPIFFS wordt geformatteerd");
    SPIFFS.format();
    Serial.println("SPIFFS is geformatteerd");
    schrijf_spiffs();
    pref.putShort("ip_1", ip_1_int);
    pref.putShort("ip_2", ip_2_int);
    pref.putShort("ip_3", ip_3_int);
    pref.putShort("ip_4", ip_4_int);
    pref.putShort("uur_of", utc_offset_uren_int);
    pref.putShort("min_of", utc_offset_minuten_int);
    pref.putShort("w_z", winter_zomer_int);
    pref.putShort("intens", intensiteit_int);
    pref.putString("ssid", ssid_string);
    pref.putString("pswd", pswd_string);
    pref.putShort("man", manueel_byte);
    pref.putShort("act", uitgang_actief_byte);
    pref.putShort("inact", ingang_actief_byte);
    pref.putString("controle", "dummy geladen");
  }
  lees_spiffs();
  ip_1_int = pref.getShort("ip_1");
  ip_2_int = pref.getShort("ip_2");
  ip_3_int = pref.getShort("ip_3");
  ip_4_int = pref.getShort("ip_4");
  utc_offset_uren_int = pref.getShort("uur_of");
  utc_offset_minuten_int = pref.getShort("min_of");
  winter_zomer_int = pref.getShort("w_z");
  intensiteit_int = pref.getShort("intens");
  ssid_string = pref.getString("ssid");
  ssid_string.toCharArray(ssid_char, ssid_string.length());
  pswd_string = pref.getString("pswd");
  pswd_string.toCharArray(pswd_char, pswd_string.length());
  manueel_byte = pref.getShort("man");
  uitgang_actief_byte = pref.getShort("act");
  ingang_actief_byte = pref.getShort("inact");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_char, pswd_char);
  netwerk_bool = true;
  wacht_op_netwerk_long = millis();
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    if(millis() - wacht_op_netwerk_long > 15000){
      netwerk_bool = false;
      break;
    }
  }
  if(netwerk_bool == true){  
    IPAddress subnet(WiFi.subnetMask());
    IPAddress gateway(WiFi.gatewayIP());
    IPAddress dns(WiFi.dnsIP(0));
    IPAddress static_ip(ip_1_int, ip_2_int, ip_3_int, ip_4_int);
    WiFi.disconnect();
    if (WiFi.config(static_ip, gateway, subnet, dns, dns) == false) {
      Serial.println("Configuration failed.");
      netwerk_bool = false;
    }
    if(netwerk_bool == true){
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid_char, pswd_char);
      wacht_op_netwerk_long = millis();
      while(WiFi.status() != WL_CONNECTED){
        delay(500);
        if(millis() - wacht_op_netwerk_long > 15000){
          netwerk_bool = false;
          break;
        }
      }
    }
    if(netwerk_bool == true){
      tijd_init_bool = false;
      initTime();
      if(tijd_init_bool == true){
        tijd_update();
      }
      attachInterrupt(digitalPinToInterrupt(MINUUT_PULS), minuut_voorbij, FALLING);
      tijd_naar_led();
      html_input();
      tijd_controle();
      vorig_uitgang_byte = uitgang_byte;
      vorig_uitgang_actief_byte = uitgang_actief_byte;
      vorig_ingang_actief_byte = ingang_actief_byte;
      inlezen_long = millis();
    }
  }
  if(netwerk_bool == false){
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APssid, APpswd);
    html_input();
  }
}

void loop(){
  if(minuut_interrupt_bool == true){
    Wire.beginTransmission(DS3231SN);
    Wire.write(0x0F);
    Wire.write(0x00);
    Wire.endTransmission();
    minuut_interrupt_bool = false;
    twaalf_uur_int ++;
    if(twaalf_uur_int == 720){
      twaalf_uur_int = 0;
      if(tijd_init_bool == false){
        initTime();
      }
      if(tijd_init_bool == true){
        tijd_update();
      }
    }
    tijd_naar_led();
    char dag_uren_char[25];
    sprintf(dag_uren_char, "%s   %02d:%02d", week_dagen_char[dag_int], uren_int, minuten_int);
    Serial.println(dag_uren_char);
    tijd_controle();
  }

/*
 * ingangen inlezen
 */
  for(int temp_int = 0; temp_int < 8; temp_int++){
    bitWrite(uit_in_byte, temp_int, 0);
    if(!strcmp(uitgang_in_char_array[temp_int], "x")){
      bitWrite(uit_in_byte, temp_int, 1);
    }
    else{
      int bit_int = String(uitgang_in_char_array[temp_int]).toInt();
      if((bitRead(ingang_byte, bit_int)) && (bitRead(ingang_actief_byte, bit_int))){
        bitWrite(uit_in_byte, temp_int, 1);
      }
    }
  }
/*
 * uitgangen sturen
 */
  for(int temp_int = 0; temp_int < 8; temp_int++){
    if(((bitRead(uitgang_actief_byte, temp_int)) && (bitRead(manueel_byte, temp_int))) ||  
       ((bitRead(uitgang_actief_byte, temp_int)) && (bitRead(dag_byte, temp_int)) &&
       ((bitRead(tijd_byte, temp_int)) || (bitRead(tijd_override_byte, temp_int))) && (bitRead(uit_in_byte, temp_int)))){
      bitWrite(uitgang_byte, temp_int, 1);   
    }
    else{
      bitWrite(uitgang_byte, temp_int, 0);    
    }
  }
  /* 
   *  bericht naar uitgangmodule als uitgang_byte veranderd is
   */
  if((vorig_uitgang_byte != uitgang_byte) || (vorig_uitgang_actief_byte != uitgang_actief_byte)){
    vorig_uitgang_byte = uitgang_byte;
    vorig_uitgang_actief_byte = uitgang_actief_byte;
    uit_byte.uit = uitgang_byte;
    terugmelding_uit_long = millis();
    terugmelding_uit_bool = true;
    temp_uitgang_terug_byte = 0;
    Serial2.write((char*)&uit_byte, sizeof(uit_byte));
  }
  /*
   * bericht naar ingangmodules als ingang_actief_byte veranderd is
   */
  if(vorig_ingang_actief_byte != ingang_actief_byte){
    vorig_ingang_actief_byte = ingang_actief_byte;
    terugmelding_in_long = millis();
    terugmelding_in_bool = true;
    temp_ingang_terug_byte = 0;
    Serial2.write((char*)&in_byte, sizeof(in_byte));
  }
  
  /* 
   *  bericht van ESP32_domotica_mesh_master 
   */  
  while(Serial2.available() > 0){
    nieuw_bericht_bool = true;
    Serial2.readBytes((char*)&bericht, sizeof(bericht));
  }
  
  if(nieuw_bericht_bool){
    nieuw_bericht_bool = false;
    
    if(bericht.code_int == 4){                    //bericht van uitgangmodule
      Serial.print("Bericht van uitgangmodule : ");
      Serial.println(bericht.nummer_int);
      switch(bericht.nummer_int){
        case 0:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 0, 1);
            break;
          }
        case 1:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 1, 1);
            break;
          }
        case 2:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 2, 1);
            break;
          }
        case 3:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 3, 1);
            break;
          }
        case 4:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 4, 1);
            break;
          }
        case 5:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 5, 1);
            break;
          }
        case 6:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 6, 1);
            break;
          }
        case 7:
          if(bericht.waarde_int == uit_byte.uit){
            bitWrite(temp_uitgang_terug_byte, 7, 1);
            break;
          }
      }
      ledstrip_uitgang();
    }
    
    if(bericht.code_int == 5){                      //bericht van ingangmodule
      Serial.print("Bericht van ingangmodule : ");
      Serial.println(bericht.nummer_int);
      switch(bericht.nummer_int){
        case 0:
          bitWrite(temp_ingang_terug_byte, 0, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 0, 1);
          }
          else{
            bitWrite(ingang_byte, 0, 0);
          }
          break;
        case 1:
          bitWrite(temp_ingang_terug_byte, 1, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 1, 1);
          }
          else{
            bitWrite(ingang_byte, 1, 0);
          }
          break;
        case 2:
          bitWrite(temp_ingang_terug_byte, 2, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 2, 1);
          }
          else{
            bitWrite(ingang_byte, 2, 0);
          }
          break;
        case 3:
          bitWrite(temp_ingang_terug_byte, 3, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 3, 1);
          }
          else{
            bitWrite(ingang_byte, 3, 0);
          }
          break;
        case 4:
          bitWrite(temp_ingang_terug_byte, 4, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 4, 1);
          }
          else{
            bitWrite(ingang_byte, 4, 0);
          }
          break;
        case 5:
          bitWrite(temp_ingang_terug_byte, 5, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 5, 1);
          }
          else{
            bitWrite(ingang_byte, 5, 0);
          }
          break;
        case 6:
          bitWrite(temp_ingang_terug_byte, 6, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 6, 1);
          }
          else{
            bitWrite(ingang_byte, 6, 0);
          }
          break;
        case 7:
          bitWrite(temp_ingang_terug_byte, 7, 1);
          if(bericht.waarde_int == 1){
            bitWrite(ingang_byte, 7, 1);
          }
          else{
            bitWrite(ingang_byte, 7, 0);
          }
          break;
      }
      ledstrip_ingang();
    }
    strip.show();
  }
  
  if(terugmelding_uit_bool){
    if((millis() - terugmelding_uit_long) > 10000){
      Serial.println("Terugmelding uitgang");
      terugmelding_uit_bool = false;
      uitgang_terug_byte = temp_uitgang_terug_byte;
    }
    ledstrip_uitgang();
    strip.show();
  }
  
  if(terugmelding_in_bool){
    if((millis() - terugmelding_in_long) > 10000){
      Serial.println("Terugmelding ingang");
      terugmelding_in_bool = false;
      ingang_terug_byte = temp_ingang_terug_byte;
    }
    ledstrip_ingang();
    strip.show();
  }

  /*
   * elke minuut configuratie controleren
   */
  if((millis() - inlezen_long) > update_int){
    Serial.println("update uitgang");
    inlezen_long = millis();
    tussentijd_long = millis();
    ingang_update_bool = true;
    uit_byte.uit = uitgang_byte;
    terugmelding_uit_long = millis();
    terugmelding_uit_bool = true;
    temp_uitgang_terug_byte = 0;
    Serial2.write((char*)&uit_byte, sizeof(uit_byte));
  }
  /*
   * 5 seconden tussen controle uitgangmodules en ingangmodules
   */
  if(((millis() - tussentijd_long) > 5000) && ingang_update_bool){
    Serial.println("update ingang");
    ingang_update_bool = false;
    terugmelding_in_long = millis();
    terugmelding_in_bool = true;
    temp_ingang_terug_byte = 0;
    Serial2.write((char*)&in_byte, sizeof(in_byte));
  }
}

 
