#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

// Variables to track packet reception
int packetCount = 0;
String lastMessage = "";
float weightReading = 0;
int receivedCounter = 0;

void setup() {
  // Initialize Serial for debugging and communication with Raspberry Pi
  Serial.begin(9600);
  
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
  display.println("LoRa Receiver");
  display.println("Initializing...");
  display.display();
  
  // Initialize LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("{\"error\":\"Starting LoRa failed\"}");
    display.println("LoRa init failed!");
    display.display();
    while(1);
  }
  
  // Match sender's configuration
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.setPreambleLength(8);
  LoRa.enableCrc();
  
  // Show initialization success
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("LoRa Initialized!");
  display.println("Waiting for packets...");
  display.display();
  Serial.println("{\"status\":\"ready\",\"message\":\"LoRa Receiver initialized\"}");
}

void loop() {
  // Check if a packet has been received
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Read packet
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    
    // Get RSSI and SNR for signal quality metrics
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    
    // Parse the message to extract counter and weight reading
    // Message format: "Hello from Lora32 #[counter][reading]"
    // Find the position of "#" and extract what follows
    int hashPos = message.indexOf('#');
    if (hashPos >= 0) {
      String dataSection = message.substring(hashPos + 1);
      
      // Find where the number ends (the counter) and extract it
      int i = 0;
      while (i < dataSection.length() && isDigit(dataSection.charAt(i))) {
        i++;
      }
      
      // Extract counter and weight
      if (i > 0) {
        receivedCounter = dataSection.substring(0, i).toInt();
        weightReading = dataSection.substring(i).toFloat();
      }
    }
    
    // Update display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LoRa Receiver");
    display.drawLine(0, 10, display.width(), 10, SSD1306_WHITE);
    display.setCursor(0, 13);
    display.print("Packet #: ");
    display.println(packetCount);
    display.print("RSSI: ");
    display.println(rssi);
    display.print("SNR: ");
    display.println(snr);
    display.print("Counter: ");
    display.println(receivedCounter);
    display.print("Weight: ");
    display.println(weightReading);
    display.display();
    
    // Send data to Raspberry Pi as JSON
    Serial.print("{");
    Serial.print("\"message\":\"packet_received\",");
    Serial.print("\"rssi\":");
    Serial.print(rssi);
    Serial.print(",\"snr\":");
    Serial.print(snr);
    Serial.print(",\"packets\":");
    Serial.print(packetCount);
    Serial.print(",\"weight\":");
    Serial.print(weightReading);
    Serial.print(",\"counter\":");
    Serial.print(receivedCounter);
    Serial.println("}");
    
    packetCount++;
    lastMessage = message;
  }
  
  // Small delay to prevent CPU hogging
  delay(10);
}