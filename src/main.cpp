#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <WebSocketsServer.h> 
#include <Preferences.h>
#include "SPIFFS.h"

int x;
int y;

int pdc_2 ;
int pdc_3 ;
unsigned long CurrentTime = 0 ;
unsigned long ElapsedTime = 0;
unsigned long StartTime = 0;

const int outputPWM_03 = 23;
const int outputPWM_02 = 26;
const int outputPWM_01 = 14;

const int freq = 16000;
const int resolution = 10;
const int ledChannel_1 = 1;
const int ledChannel_2 = 2;
const int ledChannel_3 = 3;

const char *ssid =  "ESP32";   
const char *pass =  "00000000";

File file_wrinting;
File file_reading;
String MyDuty;


int pdc1_from_spiffs[700];
int counter = 0; 
String pdc = "";

int LenPDC ;

WebSocketsServer webSocket = WebSocketsServer(81); 

void readingFromFile(){
    SPIFFS.begin();
    file_reading = SPIFFS.open("/pdc1.txt","r");
    String My_pdc  = file_reading.readString();
    file_reading.close();
    int lenOfPdc =  My_pdc.length();
    counter = lenOfPdc/4;
    pdc_2 = round(counter*0.333);
    pdc_3 = round(counter*0.666);

    int j=0;
     for (size_t i = 0; i < lenOfPdc; i=i+4)
     {
        j++;
        MyDuty = My_pdc.substring(i,i+4);
        pdc1_from_spiffs[j] = MyDuty.toInt();
     }
      
     
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
//webscket event method
    
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("Websocket is disconnected");
            
            //case when Websocket is disconnected
            break;
        case WStype_CONNECTED:{
            //wcase when websocket is connected
            Serial.println("Websocket is connected");
            Serial.println(webSocket.remoteIP(num).toString());
            webSocket.sendTXT(num, "hello");
        }
            break;
        case WStype_TEXT:
            pdc = "";
            for(int i = 0; i < length; i++) {
                pdc = pdc + (char) payload[i]; 
            }

            Serial.println(pdc);

            LenPDC  = pdc.length();
            counter = LenPDC/4;
            Serial.println("counter");
            Serial.println(counter);

            file_wrinting =  SPIFFS.open("/pdc1.txt","w");
            file_wrinting.print(pdc);
            file_wrinting.close();

            readingFromFile();
      
            webSocket.sendTXT(num, "esp32 : executed after spiffs writing");
             
             //this response(to mobile) can be used to track down the success of command in mobile app.
            break;
        case WStype_FRAGMENT_TEXT_START:
            break;
        case WStype_FRAGMENT_BIN_START:
            break;
        default:
            break;
    }
}



void setup() {

Serial.begin(115200); 
SPIFFS.begin();

   readingFromFile();

    Serial.println("nombre de commande");
   Serial.println(counter);//serial start
       
       
     // configure LED PWM functionalitites
  ledcSetup(ledChannel_1, freq, resolution);
  ledcSetup(ledChannel_2, freq, resolution);
  ledcSetup(ledChannel_3, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(outputPWM_01, ledChannel_1);
  ledcAttachPin(outputPWM_02, ledChannel_2);
  ledcAttachPin(outputPWM_03, ledChannel_3);

   Serial.println("Connecting to wifi");
   
   IPAddress apIP(192, 168, 0, 1);   //Static IP for wifi gateway
   WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //set Static IP gateway on NodeMCU
   WiFi.softAP(ssid, pass); //turn on WIFI

   webSocket.begin(); //websocket Begin
   webSocket.onEvent(webSocketEvent); //set Event for websocket
   Serial.println("Websocket is started");

}


void loop() {



    

    webSocket.loop(); 
    StartTime = millis();
    x = pdc_2;
    y = pdc_3;
    
   for (size_t i = 1; i < counter; i++)
   {      
        x++;
        y++;

        ledcWrite(ledChannel_1,pdc1_from_spiffs[i]);
        ledcWrite(ledChannel_2,pdc1_from_spiffs[x]);
        ledcWrite(ledChannel_3,pdc1_from_spiffs[y]);

        if(x == counter ){
            x = 0;
        }
        if(y == counter ){
            y = 0;
        }
        
        delayMicroseconds(44);
  }

    CurrentTime = millis();
    ElapsedTime = CurrentTime - StartTime;

    Serial.print("time of Duty :  ");
    
    Serial.println(ElapsedTime);
   
}

