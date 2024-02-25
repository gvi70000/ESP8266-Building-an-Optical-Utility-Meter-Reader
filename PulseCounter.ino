#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// Uncomment line below for debug
#define DEBUG

// WiFi credentials
constexpr char *clientID = "SmartMeter1";
constexpr char *ssid = "WIFI_SSID";
constexpr char *password = "WIFI_PASSWORD";

// MQTT broker details
constexpr char *mqtt_server = "XXX.XXX.XXX.XXX";
constexpr char *mqtt_user = "MQTT_USER";
constexpr char *mqtt_pass = "MQTT_PASSWORD";
constexpr char *topic_energy = "energy1/Energy";
// Pulses per kWh
constexpr char *topic_ppk = "energy1/ppk";
constexpr uint16_t mqtt_port = 1883;

#ifdef DEBUG
// Serial communication speed
constexpr uint32_t serialSpeed = 115200;
#endif

// Pulses per kWh
uint16_t ppk = 1;

// Pin for interrupt (pulse counting)
constexpr uint8_t interruptPin = 2; // GPIO2

// EEPROM settings
constexpr uint8_t sizeEEPROM = 6;           // EEPROM size in bytes
constexpr uint8_t ppkLocation = 0;          // EEPROM address for pulses per kWh
constexpr uint8_t totalEnergyLocation = 2;  // EEPROM address for totalEnergy

// Delay for WiFi connection and MQTT reconnect
constexpr uint16_t delayWiFi = 1000;
// Holds the pulses counted value. It is lost at reset or power loss
// as the hardware is lacking power loss ciruit. To overcome this
// the total energy value can be set from MQTT
volatile uint16_t pulseCount = 0;
// Holds the total energy value
uint32_t totalEnergy = 0;

#ifdef DEBUG
  uint32_t timer = 0;
#endif

WiFiClient espClient;
PubSubClient client(espClient);

// Function to set up WiFi connection
void setup_wifi() {
  IPAddress staticIP(10, 0, 1, 111);
  IPAddress gateway(10, 0, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(10, 0, 1, 1);
  WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(clientID);
  WiFi.disconnect();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    #ifdef DEBUG
      Serial.print(".");
    #endif
  }
}

// Function to reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, mqtt_user, mqtt_pass)) {
      client.subscribe(topic_energy);
      // First publish the ppk value
      String ppkString = String(ppk);
      client.publish(topic_ppk, ppkString.c_str());
      client.subscribe(topic_ppk);
    } else {
      #ifdef DEBUG
        Serial.print("*");
      #endif
      delay(delayWiFi);
    }
  }
}

// Function to publish total energy to MQTT broker
void mqttPublish() {
  if (client.connected()) {
    String energyString = String(totalEnergy);
    client.publish(topic_energy, energyString.c_str());
  } else {
    reconnect();
  }
}

// Interrupt service routine for pulse counting
void ICACHE_RAM_ATTR countPulse() {
  pulseCount++;
}

// Callback function for handling MQTT messages
void callback(char *topic, byte *payload, unsigned int length) {
  String payloadStr;
  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }
  if (strcmp(topic, topic_energy) == 0) {
    totalEnergy = payloadStr.toInt();
    EEPROM.put(totalEnergyLocation, totalEnergy);
    EEPROM.commit();
  }
  if (strcmp(topic, topic_ppk) == 0) {
    ppk = payloadStr.toInt();
    EEPROM.put(ppkLocation, ppk);
    EEPROM.commit();
  }
}

void setup() {
  #ifdef DEBUG
    Serial.begin(serialSpeed);
    delay(1000);
  #endif
  EEPROM.begin(sizeEEPROM);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), countPulse, FALLING);
  EEPROM.get(ppkLocation, ppk);
  EEPROM.get(totalEnergyLocation, totalEnergy);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  // Add your MQTT broker username and password
  reconnect();
  //client.connect(clientID, mqtt_user, mqtt_pass);
  mqttPublish();
  #ifdef DEBUG
    Serial.println("Done setup");
    timer = millis();
      Serial.println("_________________________________________");
  #endif

}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (pulseCount >= ppk) {
    totalEnergy++;
    mqttPublish();
    #ifdef DEBUG
      timer = millis() - timer;
      Serial.print("Pulse count = ");Serial.print(pulseCount);Serial.print(" In "); Serial.print(timer); Serial.println(" ms");
      timer = millis();
    #endif
    pulseCount = 0;
    EEPROM.put(totalEnergyLocation, totalEnergy);
    EEPROM.commit();
  }
  client.loop();
  yield();
}
