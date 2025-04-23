#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"
#include <esp_sleep.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 25;
const int LOADCELL_SCK_PIN = 4;
const int Calibration_Weight = 1000;
HX711 scale;


// Pin definitions for LilyGO LoRa32
#define SCK 5      // GPIO5  -- SX1278's SCK
#define MISO 19    // GPIO19 -- SX1278's MISO
#define MOSI 27    // GPIO27 -- SX1278's MOSI
#define SS 18      // GPIO18 -- SX1278's CS
#define RST 14     // GPIO14 -- SX1278's RESET
#define DI0 26     // GPIO26 -- SX1278's IRQ(Interrupt Request)

// OLED Display settings
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C address (0x3C for 128x64)

// LoRa settings
#define LORA_FREQUENCY 868E6  // Set to your region's frequency
#define SYNC_WORD 0xF3        // Must match on sender and receiver (like a network ID)

// Initialize the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int counter = 0;
float reading = 0;
float lastReading = -1;
int detectionThreshold = 20000;

void setup() {

  // Initialize Serial for debugging
  Serial.begin(9600);
  while (!Serial);

  // Initialize Scale (Load/Weight Sensor)
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare();
  Serial.println("Calibration");
  Serial.println("Put a known weight on the scale");
  delay(5000);
  float x = scale.get_units(10);
  x = x / Calibration_Weight;
  scale.set_scale(x);
  Serial.println("Calibration finished...");
  delay(3000);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initialization message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Sender");
  display.println("Initializing...");
  display.display();

  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("Starting LoRa failed");
    display.println("LoRa init failed!");
    display.display();
    while(1);
  }

  // Enhanced LoRa parameter configuration
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(SYNC_WORD);     // Set sync word - important!
  LoRa.setPreambleLength(8);       // Set preamble length
  LoRa.setTxPower(20);             // Set to maximum power (20dBm)
  LoRa.enableCrc();

  // Show initialization success
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("LoRa Initialized!");
  display.display();
  Serial.println("LoRa Initializing OK!");
  
  delay(2000); // Allow time to read the display
}

void loop() {
  esp_sleep_enable_timer_wakeup(5 * 60 * 1000000);  // Wake up every 5 * 60 seconds
  esp_light_sleep_start();

  float temporaryAverage = 0;

  for(int i=0;i<11;i++){
    if (scale.is_ready()) {
      reading = scale.get_units(10);
    }
    temporaryAverage += abs(reading * 0.1);
    delay(60);
  }
  unsigned long currentMillis = millis();

  String message = String(temporaryAverage);
   
  

  float readingDifference = temporaryAverage - lastReading;

  if (readingDifference > detectionThreshold) {
    // Print to Serial
    Serial.print("Sending packet: ");
    Serial.println(counter);
    // Send packet (using sync mode)
    LoRa.beginPacket();
    LoRa.print(readingDifference);
    LoRa.endPacket(); // Changed to synchronous mode
    counter++;
  }

  lastReading = temporaryAverage;

}