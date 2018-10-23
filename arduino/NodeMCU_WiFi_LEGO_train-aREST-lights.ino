/*
  This program provides a WiFi connected REST interface for a LEGO Power Functions motor and lights.
  
  Written in 2016 by Cory Guynn under a GPL license.
  www.InternetOfLEGO.com
*/


// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "yourSSID";
const char* password = "yourpassword";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int speed = 0; //stopped
int direction = 1; //forward
int lights = 1; //lights on

int motor = 5;
int led = 4;

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // 4 pins control the motor shield. 
  // Pins 4 and 5 control the speed of the motors with PWM in a default range of 0..1023 
  // while pins 0 and 2 define the direction the motors will turn. 
  // Since several LEDs will be connected in the coaches, the second motor power source will be used for the lights.
  pinMode(motor, OUTPUT);
  pinMode(0, OUTPUT); 
  
  pinMode(led, OUTPUT);
  pinMode(2, OUTPUT);
  
  // initialize motor and set direction
  digitalWrite(motor, speed); 
  digitalWrite(0, direction);

  // initialize lights
  digitalWrite(led, lights);
  digitalWrite(2, 1); // forward polarity through motor driver for LEDs

  // expose variables to REST API
  rest.variable("speed", &speed);
  rest.variable("lights", &lights);
  rest.variable("direction", &direction);

  // Function to be exposed to REST API
  rest.function("motor",motorControl);
  rest.function("lights",lightControl);
  
  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("Horizon Express");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {

  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

}

// Custom function accessible by the API
int lightControl(String command) {

  // Get new state from command
  lights = command.toInt();

  digitalWrite(led,lights);
  return 1;
}

int motorControl(String command){

  // set motor direction
  if (command.toInt() < 0) { // reveresed motor
      direction = 1; //forward
    } else {
      direction = 0; //reverse
  }
  
  speed = abs(command.toInt());
  analogWrite(motor, speed);
  digitalWrite(0, direction);
}

