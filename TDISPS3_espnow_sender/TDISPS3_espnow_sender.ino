// Libraries
#include <esp_now.h>      // esp_now library
#include <WiFi.h>         // WiFi library
#include "driver/temp_sensor.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Definitions
#define sensorPin 1
#define ONE_WIRE_BUS 4   // D2 is GPIO4

// Variables
unsigned long previousMillis = 0;
const long interval = 200;    // 200ms = 5-hz
float sensor1C, sensor2C, tmpCPU;
String sta;

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);	
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
// Dallas Temperature Addresses
uint8_t sensor1[8] = {0x28, 0x45, 0x97, 0x2A, 0x07, 0x00, 0x00, 0x77};
uint8_t sensor2[8] = {0x28, 0x58, 0x79, 0x2A, 0x07, 0x00, 0x00, 0x1E};

// Responder MAC address
uint8_t receiverAdd[] = {0x3C, 0x84, 0x27, 0xC1, 0x39, 0x54};

// Define data structure
typedef struct struct_message {
  float sensor1C;
  float sensor2C;
  float tmpCPU;
} struct_message;

// Create structured data object
struct_message myData;

// Register Peer
esp_now_peer_info_t peerInfo;

// Sent data callback function
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status == ESP_NOW_SEND_SUCCESS) 
    sta="Delivery Success";
  else sta="Delivery Fail";
  // Update display
}

void read(){
  // Analog read sensorPin
  sensors.requestTemperatures();
  sensor1C = sensors.getTempC(sensor1);
  sensor2C = sensors.getTempC(sensor2);
  // CPU Temperature
  temp_sensor_read_celsius(&tmpCPU);
  // Serial print
  // Serial.print(reading);
  // Serial.print("\t");
  Serial.print(sensor1C, 2);
  Serial.print("\t");
  Serial.print(sensor2C, 2);
  Serial.print("\t");
  Serial.print(tmpCPU, 2);
  Serial.print("\t");
  Serial.println(String(sta));
  // Add to structured data object
  // myData.reading = reading;
  myData.sensor1C = sensor1C;
  myData.sensor2C = sensor2C;
  myData.tmpCPU = tmpCPU;
}

void setup() {
  Serial.begin(115200);
  // WiFi
  WiFi.mode(WIFI_STA);
  // Initialize ESP-NOW
  if(esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
  }
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  memcpy(peerInfo.peer_addr, receiverAdd, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer.");
  }
  sensors.begin();
  sensors.setResolution(10);  //9-12 bits
 }

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    read();
    esp_now_send(receiverAdd, (uint8_t *) &myData, sizeof(myData));
  }
}
