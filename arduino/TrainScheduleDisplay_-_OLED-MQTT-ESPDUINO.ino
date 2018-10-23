/**
* Internet of Lego - Train Schedule Display
* This program connects to WiFi, utilizes MQTT and displays the local train schedule on an OLED screen
* The OLED graphics library is slimmed down to only text for memory efficiency
* https://github.com/greiman/SSD1306Ascii
* 
* Hardware: Cactus Micro Rev2 - http://wiki.aprbrother.com/wiki/Cactus_Micro_Rev2
* Written by: Cory Guynn with some snippets from public examples ;)
* 2016
*
* Complete write-up: http://www.internetoflego.com/train-schedule-display-2/
*/

// Global Variables
unsigned long time; // used to limit publish frequency


// OLED
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

// ESP8266 WiFi 
#include <espduino.h>
#define PIN_ENABLE_ESP 13
#define SSID  ""
#define PASS  ""

// MQTT Messaging
#include <mqtt.h>
ESP esp(&Serial1, &Serial, PIN_ENABLE_ESP);
MQTT mqtt(&esp);
boolean wifiConnected = false;
#define mqttBroker "aws.internetoflego.com"
#define mqttSub "/trainschedule/910GLONFLDS"
String mqttMessage = "";

/*******************
// Functions
*******************/

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      wifiConnected = true;
      Serial.println("WIFI CONNECTED");
      oled.print("WiFi Online: ");
      oled.println(SSID);
      mqtt.connect(mqttBroker, 1883, false);    
      //or mqtt.connect("host", 1883); /*without security ssl*/
    } else {
      wifiConnected = false;
      mqtt.disconnect();
      oled.println("WiFi OFFLINE");
    }
    
  }
}

void mqttConnected(void* response)
{
  Serial.println("MQTT Connected");
  oled.println("MQTT Connected");
  mqtt.subscribe(mqttSub); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.publish("/news", "OLED display is online");
}

void mqttDisconnected(void* response)
{
  oled.clear();
  Serial.println("MQTT Disconnected");
}

void mqttData(void* response)
{
  RESPONSE res(response);
  Serial.println("mqttTopic:");
  String topic = res.popString();
  Serial.println(topic);
  oled.println(topic);
  Serial.println("mqttMessage:");
  mqttMessage = res.popString();
  Serial.println(mqttMessage);
}

void mqttPublished(void* response)
{

}

void setup() {
  // OLED initilize
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);  
  oled.clear();  
  oled.println("OLED online");

  // Hardware Serial (for ESP8266) and Serial Concole initialization
  Serial1.begin(19200);
  Serial.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());

  Serial.println("Setup MQTT client");
  if(!mqtt.begin("TrainScheduleDisplay-LF", "admin", "Isb_C4OGD4c3", 120, 1)) {
    Serial.println("Failed to setup mqtt");
    while(1);
  }

  Serial.println("Setup MQTT lwt");
  mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");

  /*setup mqtt events */
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);

  /*setup wifi*/
  Serial.println("Setup WiFi");
  esp.wifiCb.attach(&wifiCb);
  esp.wifiConnect(SSID, PASS);
  
  Serial.println("System Online");
}



void loop() {
 
  esp.process();
  
  if (millis() > (time + 10000)) {
    time = millis(); 
    oled.clear();
    oled.set2X();
    oled.println(" IoL City ");
    oled.println("");
    oled.set1X();
    oled.println("   Train Network");   
    delay(2000);
    oled.clear();
    oled.println(mqttMessage);
    if(wifiConnected) {
      // publish stuff here
    }
   }
}