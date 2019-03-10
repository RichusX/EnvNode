#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <PubSubClient.h> // MQTT
#include <DHT.h> // DHT Sensors
#include <Streaming.h>
#include <ArduinoJson.h>

#define DHTTYPE DHT22

const char* mqtt_server = "";
const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_topic = "";
const char* ap_name = "EnvNode"

uint8_t DHTPin = D1; // DHT Sensor
DHT dht(DHTPin, DHTTYPE); // Initialize DHT sensor.

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
String publishString;
char strCopy[50];

const size_t capacity = JSON_OBJECT_SIZE(2);
DynamicJsonDocument doc(capacity);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
    Serial.begin(115200);

    //WiFiManager
    WiFiManager wifiManager;
    
    wifiManager.autoConnect(ap_name);
    //or use this for auto generated name ESP + ChipID

    Serial.println("Connection to WiFi successful.");

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    pinMode(DHTPin, INPUT);
    dht.begin();    

}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  

  long now = millis();
  if (now - lastMsg > 30000) {
    lastMsg = now;
    publishString = "";
    memset(strCopy, 0, sizeof(strCopy));

    doc["temperature"] = dht.readTemperature(); // Gets the temperature
    doc["humidity"] = dht.readHumidity(); // Gets the humidity 

    serializeJson(doc, publishString);
    publishString.toCharArray(strCopy, 50);

    client.publish(mqtt_topic, strCopy);

  }
}
