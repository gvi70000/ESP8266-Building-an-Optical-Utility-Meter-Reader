// The Pulses to kWh conversion will be done in OpenHAB or Home Assistant
// This will only publish counts
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

// EEPROM save location
uint8_t saveLocation = 2;
// Pin for interrupt (pulse counting)
constexpr uint8_t interruptPin = 2; // GPIO2

// EEPROM settings
constexpr uint8_t sizeEEPROM = 22;           // EEPROM size in bytes, 2 for SendToEvery, 5 x 4bytes for 5 locations of TotalPulses
constexpr uint8_t sendToEveryLocation = 0;          // EEPROM address for pulses per kWh
constexpr uint8_t totalPulsesLocation = 2;  // EEPROM address for totalEnergy
constexpr uint8_t pulsesLocations = 5;  // EEPROM address for totalEnergy
// Delay for WiFi connection and MQTT reconnect
constexpr uint16_t delayWiFi = 1000;
// Holds the pulses counted value. It is lost at reset or power loss
// as the hardware is lacking power loss ciruit. To overcome this
// the total energy value can be set from MQTT
volatile uint32_t totalPulses = 0;
uint32_t prevPulses = 0;
// If you have a meter with 1000 pulses/kWh you can use 
// sendEveryN_Pulses = 100 to avoid overloading Zigbee network
uint16_t sendEveryN_Pulses = 0;
uint32_t tim1 = 0;

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
      client.subscribe(topic_setTotalPulses);
      client.subscribe(topic_setSendEvery);
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
    client.publish(topic_TotalPulses, String(totalPulses).c_str());
    client.publish(topic_SendEvery, String(sendEveryN_Pulses).c_str());
  } else {
    reconnect();
  }
}

// Interrupt service routine for pulse counting
void ICACHE_RAM_ATTR countPulse() {
  totalPulses++;
}

// Callback function for handling MQTT messages
void callback(char *topic, byte *payload, unsigned int length) {
  // Ignore any callback for 250ms after we publish
  String payloadStr;
  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }
  if (strcmp(topic, topic_setTotalPulses) == 0) {
    totalPulses = payloadStr.toInt();
    #ifdef DEBUG
      Serial.print("callback settotalPulses = "); Serial.println(totalPulses);
    #endif
    saveLocation = 4;
    setTotalPulsesToEEPROM();
  }
  if (strcmp(topic, topic_setSendEvery) == 0) {
    sendEveryN_Pulses = payloadStr.toInt();
    #ifdef DEBUG
      Serial.print("callback setsendEveryN_Pulses = "); Serial.println(sendEveryN_Pulses);
    #endif
    EEPROM.put(sendToEveryLocation, sendEveryN_Pulses);
    EEPROM.commit();
  }
}

uint32_t getTotalPulsesFromEEPROM(uint8_t crtLocation) {
  uint32_t value = 0;
  uint8_t location = 2 + (crtLocation * 4);
  EEPROM.get(location, value);
  return value;
}

void setTotalPulsesToEEPROM() {
  saveLocation++;
  if(saveLocation == pulsesLocations){
    saveLocation = 0;
  }
  #ifdef DEBUG
    Serial.print("Save location ");Serial.println(saveLocation);
  #endif
  // Volatile variable will not work here
  uint32_t val = totalPulses;
  EEPROM.put(2 + (saveLocation * 4), val);
  EEPROM.commit();
}

void setup() {
  #ifdef DEBUG
    Serial.begin(serialSpeed);
    delay(1000);
  #endif
  EEPROM.begin(sizeEEPROM);
    // Get the number of pulses
  EEPROM.get(sendToEveryLocation, sendEveryN_Pulses);

  // Get the last saved energy
  //0-1 2bytes
  //2-5   4bytes Location 0
  //6-9   4bytes Location 1
  //10-13 4bytes Location 2
  //14-17 4bytes Location 3
  //le18-21 4bytes Location 4
  for(uint8_t cnt = 0; cnt < pulsesLocations; cnt++) {
    // Get the value from EEPROM
    uint32_t val = getTotalPulsesFromEEPROM(cnt);
    // If value > totalPulses then we have a location
    if(val > totalPulses) {
      totalPulses = val;
      saveLocation = cnt;
    }
  }
  // Set up the interrupt
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), countPulse, FALLING);
  // Start WiFi
  setup_wifi();
  // Start MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  // Add your MQTT broker username and password
  reconnect();
  //client.connect(clientID, mqtt_user, mqtt_pass);
  mqttPublish();
  #ifdef DEBUG
    Serial.print("sendEveryN_Pulses ");Serial.println(sendEveryN_Pulses);
    Serial.print("totalPulses ");Serial.println(totalPulses);
    Serial.println("Done setup");
    Serial.println("_________________________________________");
  #endif
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  // This condition is for low power consumption
  if(prevPulses != totalPulses) {
    // Check if we need to publish
    if ((totalPulses % sendEveryN_Pulses) == 0) {
      mqttPublish();
      setTotalPulsesToEEPROM();
    }
    prevPulses = totalPulses;
  }
  client.loop();
  yield();
}
