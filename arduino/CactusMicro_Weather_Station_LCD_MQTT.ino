/**
* This program sends weather data to an MQTT broker, and displays the messages onto the attached LCD screen
* Hardware: Cactus Micro Rev2 - http://wiki.aprbrother.com/wiki/Cactus_Micro_Rev2
* Written by: Cory Guynn with some snippets from public examples ;)
* http://www.InternetOfLego.com
*/

// Global Variables
unsigned long time; // used to limit publish frequency

// LED Status
#define LEDRED 10

// LCD Screen
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); 

// ESP8266 WiFi 
#include <espduino.h>
#define PIN_ENABLE_ESP 13
#define SSID  "yourssid"
#define PASS  "yourpass"

// MQTT Messaging
#include <mqtt.h>
ESP esp(&Serial1, &Serial, PIN_ENABLE_ESP);
MQTT mqtt(&esp);
boolean wifiConnected = false;
#define mqttBroker "yourbrokeraddress"


// DHT11 Temperature & Humidity Sensor
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

// Photoresistor Ligh Detection
int lightPin = A0;        // LDR Sensor

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
      Serial.println("WIFI CONNECTED");
      lcd.clear();
      lcd.print("WiFi ONLINE");
      lcd.setCursor(0, 1);
      lcd.print(SSID);
      mqtt.connect(mqttBroker, 1883, false);
      wifiConnected = true;
      //or mqtt.connect("host", 1883); /*without security ssl*/
    } else {
      wifiConnected = false;
      mqtt.disconnect();
      lcd.clear();
      lcd.print("WiFi OFFLINE");
    }
    
  }
}

void mqttConnected(void* response)
{
  Serial.println("MQTT Connected");
  //mqtt.subscribe("/topic/0"); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.publish("/sensors/iolcity/weather/news", "weatherstation is online");
}

void mqttDisconnected(void* response)
{
  Serial.println("MQTT Disconnected");
}

void mqttData(void* response)
{
  RESPONSE res(response);

  Serial.print("Received: topic=");
  String topic = res.popString();
  Serial.println(topic);
  lcd.clear();
  lcd.home();
  lcd.print(topic);


  Serial.print("data=");
  String data = res.popString();
  Serial.println(data);
  lcd.setCursor(1, 1);
  lcd.print(data);


}
void mqttPublished(void* response)
{

}
void setup() {
  // initialize leds
  pinMode(LEDRED, OUTPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Weather Station");
  delay(2000);
  lcd.clear();
  
  Serial1.begin(19200);
  Serial.begin(19200);
  esp.enable();
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready());

  Serial.println("Setup MQTT client");
  if(!mqtt.begin("DVES_duino", "admin", "Isb_C4OGD4c3", 120, 1)) {
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
  if(wifiConnected) {

    /*******************
    // Sensors 
    *******************/
    // publish sensor reading every 5 seconds
    if (millis() > (time + 5000)) {
      time = millis(); 

      // blink LED to indicate sensor read
      digitalWrite(LEDRED, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(500);                   // wait
      digitalWrite(LEDRED, LOW);    // turn the LED off by making the voltage LOW   

      // Read light sensor
      int l = analogRead (lightPin);
      l = map (l, 0, 1023, 0, 100); // scaled from 100 - 0 (lower is darker)
      
      // Reading temperature and humidity
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);
    
      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }
    
      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);
    
    
      /*******************
      // Print to LCD
      ********************/
      lcd.clear();
      lcd.home();
      lcd.print("Humidity: ");
      lcd.print(int(h));
      lcd.print(" %  ");
      lcd.setCursor(0, 1);
      lcd.print("Temp: ");
      lcd.print(int(t));
      lcd.print(" *C ");
    
      /*******************
      // Publish to MQTT
      ********************/

      // Convert data to character array
      char tChar[10];  
      char hChar[10];
      char hicChar[10];
      char lChar[10];
      dtostrf(t, 4, 2, tChar);
      dtostrf(h, 4, 2, hChar);
      dtostrf(hic, 4, 2, hicChar);
      dtostrf(l, 4, 2, lChar);
      
      // Publish data character array to MQTT topics
      mqtt.publish("/sensors/iolcity/weather/humidity", hChar);
      mqtt.publish("/sensors/iolcity/weather/temperature", tChar);
      mqtt.publish("/sensors/iolcity/weather/heatindex", hicChar);
      mqtt.publish("/sensors/iolcity/weather/light", lChar);

  
      // Convert data to JSON string 
      String json =
      "{\"data\":{"
      "\"humidity\": \"" + String(h) + "\","
      "\"temperature\": \"" + String(t) + "\","
      "\"heatindex\": \"" + String(hic) + "\","
      "\"light\": \"" + String(l) + "\"}"
      "}";
     
      // Convert JSON string to character array
      char jsonChar[100];
      json.toCharArray(jsonChar, json.length()+1);
      
      // Publish JSON character array to MQTT topic
      mqtt.publish("/sensors/iolcity/weather/json", jsonChar);  

      /*******************
      // Print to Console
      ********************/
      
      Serial.println(" ");
      Serial.println("Data");
      Serial.println(json);   
  
    } 
  }
}