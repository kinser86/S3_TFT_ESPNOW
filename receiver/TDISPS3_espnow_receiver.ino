// Libraries
#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "esp_system.h"       // For the watchdog timer
#include "NotoSansBold15.h"   // Smooth Font
#include "NotoSansBold36.h"   // Smooth Font

// Display 170 x 320
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite labelSprite = TFT_eSprite(&tft);
TFT_eSprite valueSprite = TFT_eSprite(&tft);
TFT_eSprite timeSprite = TFT_eSprite(&tft);

// Definitions
#define brightnessPin 14   // Display brightness

// Variables
// Sprite Width/Height
int labelW = 160;
int labelH = 20;
int valueW = 160;
int valueH = 56;
int timeW = 160;
int timeH = 18;

bool spriteExist = false;
bool updateDisplay;

unsigned long currentMillis;
unsigned long previousMillis = 0;
const long interval = 200;    // 200ms = 5-hz

// Watchdog
const int wdtTimeout = 6000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;
// Display
int brightnesses[5] = {40,80,120,150,240};
int selectedBrightness = 3;

// Watchdog interupt
void ARDUINO_ISR_ATTR resetModule() {
  ets_printf("reboot\n");
  esp_restart();
}

// Define data structure
typedef struct struct_message {
  float sensor1C;
  float sensor2C;
  float tmpCPU;
} struct_message;

// Create structured data object
struct_message myData;

// Callback function
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  timerWrite(timer, 0); //reset timer (feed watchdog)
  memcpy(&myData, incomingData, sizeof(myData));
  updateDisplay = true; //update the display
}

// Label sprites only generated once after connection to server is established.
void tftLabels(){
  // tft.fillScreen(TFT_BLACK);
  labelSprite.loadFont(NotoSansBold15);
  labelSprite.createSprite(labelW,labelH);
  labelSprite.setTextDatum(MC_DATUM);
  labelSprite.setTextColor(TFT_BLACK);
  //Reading Sprite
  labelSprite.fillSprite(TFT_RED);
  labelSprite.drawString(String("Temp1 ") + String((char)176) + String("C"),labelW/2,labelH/2);
  labelSprite.pushSprite(0,timeH);
  //RSSI Sprite
  labelSprite.fillSprite(TFT_BLUE);
  labelSprite.drawString(String("Temp2 ") + String((char)176) + String("C"),labelW/2,labelH/2);
  labelSprite.pushSprite(labelW,timeH);
  //CPU Sprite
  labelSprite.fillSprite(TFT_YELLOW);
  labelSprite.drawString(String("CPU ") + String((char)176) + String("C"),labelW/2,labelH/2);
  labelSprite.pushSprite(0,timeH+labelH+valueH);
  //TempC Sprite
  labelSprite.fillSprite(TFT_WHITE);
  labelSprite.drawString(String("Delta ") + String((char)176) + String("C"),labelW/2,labelH/2);
  labelSprite.pushSprite(labelW,timeH+labelH+valueH);
  labelSprite.unloadFont();
  labelSprite.deleteSprite();
}

// Value sprites updated each monitor loop.
void tftValues(){
  // Don't create the sprites every time!
  if (!spriteExist){
    valueSprite.createSprite(valueW,valueH);
    valueSprite.setTextDatum(MC_DATUM);
    valueSprite.setTextColor(TFT_BLACK);

    timeSprite.createSprite(timeW,timeH);
    timeSprite.setTextDatum(MC_DATUM);
    timeSprite.setTextColor(TFT_BLACK);
    spriteExist = true;   // its been created, no need to keep recreating
  }
  // Output to serial
  Serial.print(currentMillis - previousMillis);
  Serial.print("\t");
  Serial.print(myData.sensor1C);
  Serial.print("\t");
  Serial.print(myData.sensor2C);
  Serial.print("\t");
  Serial.println(myData.tmpCPU);

  timeSprite.fillSprite(TFT_GREEN);
  timeSprite.drawString(String(currentMillis - previousMillis),timeW/2,timeH/2,2);
  timeSprite.pushSprite(80,0);
  // Draw value
  valueSprite.fillSprite(TFT_CYAN);
  valueSprite.drawString(String(myData.sensor1C),valueW/2,valueH/2,7);
  valueSprite.pushSprite(0,timeH+labelH);
  // Draw value
  valueSprite.fillSprite(TFT_MAGENTA);
  valueSprite.drawString(String(myData.sensor2C),valueW/2,valueH/2,7);
  valueSprite.pushSprite(valueW,timeH+labelH);
  // Draw value
  valueSprite.fillSprite(TFT_SILVER);
  valueSprite.drawString(String(myData.tmpCPU),valueW/2,valueH/2,7);
  valueSprite.pushSprite(0,timeH+labelH+valueH+labelH);
  // Draw value
  valueSprite.fillSprite(TFT_GOLD);
  valueSprite.drawString(String(myData.sensor2C-myData.sensor1C),valueW/2,valueH/2,7);
  valueSprite.pushSprite(valueW,timeH+labelH+valueH+labelH);
}

void setup() {
  // Serial
  Serial.begin(115200);
  // watchdog
  timer = timerBegin(0, 80, true);                  //timer 0, div 80
  timerAttachInterrupt(timer, &resetModule, true);  //attach callback
  timerAlarmWrite(timer, wdtTimeout * 1000, false); //set time in us
  timerAlarmEnable(timer);                          //enable interrupt
  // Display
  tft.init();
  delay(50);    // give the screen time to initialize otherwise artifacts can appear.
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  // Brightness
  ledcSetup(0, 10000, 8);
  ledcAttachPin(38, 0);
  ledcWrite(0, brightnesses[selectedBrightness]);  //brightness of screen
  // WiFi
  WiFi.mode(WIFI_STA);
  esp_now_init();
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
  // Labels
  tftLabels();
}

void loop() {
  currentMillis = millis();

  // When it is time to update the display
  while (updateDisplay){
    // Serial.print(ESP.getFreeHeap());
    // Serial.print("\t");
    // Serial.print(ESP.getCpuFreqMHz());
    // Serial.print("\t");
    // How much time passed since the last update? Is it near the interval?
    tftValues();
    previousMillis = currentMillis;
    updateDisplay = false;    // reset and wait for next batch of data
    //Adjust brightness
    if(digitalRead(brightnessPin)==0){
      selectedBrightness++;
      if(selectedBrightness == 5){
        selectedBrightness = 0;
      }
    ledcWrite(0, brightnesses[selectedBrightness]);  //brightness of screen
    }
  }
}
