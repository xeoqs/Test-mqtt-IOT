#include "WiFi.h"
#include "PubSubClient.h"
#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4
#define DHTTYPE    DHT11


DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;


//DHT dht(DHT11_PIN, DHT11, 15)
//#include <Preferences.h>

#include <mqtt_client.h>

#define EEPROM_SIZE 64

char ssid[32] = "Koderman";
char password[32] =  "kodermancki";


//Preferences preferences;

const char* default_ssid = "K8";
const char* default_password = "dogsminusone";
 
WiFiClient ethClient;
 
//IPAddress  mqtt_server(192,168,1,2);
PubSubClient client(ethClient);
const char* mqtt_server = "xeoqs.ddns.net";

void setup(){
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);

  dht.begin();

  sensor_t sensor;
  dht.temperature().getSensor(&sensor);

  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
  Serial.begin(115200); 

  while (WiFi.status() != WL_CONNECTED) {
    for(int i = 0; i< 10 && WiFi.status() != WL_CONNECTED; i++)
    {
    delay(500);
    Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, password);
    }

    for(int i = 0; i<10 && WiFi.status() != WL_CONNECTED; i++)
    {
    delay(500);
    Serial.println("Could not connect searching for default...");
    //WiFi.begin(default_ssid, default_password);
    }
  }
 if(WiFi.status() == WL_CONNECTED)
    {
      //digitalWrite(BUILTIN_LED, HIGH);
      Serial.println("Connected to the WiFi network");
      Serial.println("Ip: ");
      Serial.println(WiFi.localIP());  
    }
}

void mqttconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print(client.state());
    Serial.print("MQTT connecting ...");
    // client ID 
    const char* clientId = "ESP32";
    // connect now 
    if (client.connect(clientId, "","")) {
      Serial.println("connected");
      // subscribe topic with default QoS 0
      client.subscribe("LED");
      client.subscribe("SSID");
      client.subscribe("PASS");
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:"); 
    Serial.write(payload, length);
    Serial.println();
    String messageTemp;
    int adress = 0;
    
    for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    messageTemp += ((char)payload[i]);
    }
     //Serial.println();
    //Serial.println(messageTemp);


    if(String(topic)=="LED")
    {//Serial.println("Topic: LED");
      
      if(messageTemp == "on"){
        //Serial.println("on");
        digitalWrite(2, HIGH);
      }
       else if(messageTemp == "off"){
        digitalWrite(2, LOW);
        // Serial.println("off");
       }
    }

    if(String(topic) =="SSID")
    {
      
    }
    if(String(topic) =="PASS")
    {
      
    }
      
}
unsigned long int timeSensor = millis();
void readTemp()
{
  if (millis() - timeSensor > 1000)
  {
  // Get temperature event and print its value.
  float temperature;
  float humidity;
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) 
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
     client.publish("Temperature", String(event.temperature).c_str());
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) 
    {
      Serial.println(F("Error reading humidity!"));
    }
    else 
    {
      client.publish("Humidity", String(event.relative_humidity).c_str());
    }
  }
  timeSensor = millis();
  }
}



void loop(){

  if (!client.connected()) {  // reconnect
    mqttconnect();
  }
  
  readTemp();

  
  client.loop();
}
