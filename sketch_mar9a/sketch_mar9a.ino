/*
  Made on 6 July 2021
  By Amirmohammad Shojaei
  Home
  based on “ajaynikam2410” Example
*/

//for esp32 use <wifi.h>, for esp8266 use <esp8266wifi.h>
// this code is for esp32 for using esp8266 you just need to change <wifi.h> into <esp8266wifi.h> and also change the relay pin
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ESP32Servo.h>

Servo myservo;

int pos = 0;

int servoPin = 18;

/************************* Pin Definition *********************************/

//Relays for switching appliances
#define Relay1 4
#define servo 18

//buzzer to know the status of MQTT connections and can be used for any other purpose according to your project need.




/************************* WiFi Access Point *********************************/

#define WLAN_SSID "Tostatronic"
#define WLAN_PASS "Jorge19951$"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883  // use 8883 for SSL
#define AIO_USERNAME "Tostatronic"
#define AIO_KEY "aio_AHwl45f1JEzbtp1xGGj26U7v8BsK"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

Adafruit_MQTT_Publish Light = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/light");


// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/door");




/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);

  delay(10);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);            // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000);  // attaches the servo on pin 18 to the servo object

  pinMode(Relay1, OUTPUT);


  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&Light1);
}


void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();
  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char *)Light1.lastread);
      if(Light1_State==1)
      {
        openDoor();
      }
      else
        CloseDoor();
      digitalWrite(Relay1, Light1_State);
    }
  }



  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {  // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}

void openDoor() {
  for (pos = 0; pos <= 180; pos += 1) {  // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);  // tell servo to go to position in variable 'pos'
    delay(15);           // waits 15ms for the servo to reach the position
  }
}
void CloseDoor()
{
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
		myservo.write(pos);    // tell servo to go to position in variable 'pos'
		delay(15);             // waits 15ms for the servo to reach the position
	}
}