// Tested with RadioLib v4.6.0. Release v5.0.0 introduces backwards-incompatible
// changes to the public API.
// For more details, see https://github.com/jgromes/RadioLib/releases/tag/5.0.0
#include <RadioLib.h>

// SSD1306 OLED display libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi libraries
#include <WiFi.h>

// Tasks scheduling with TaskScheduler library
#include <TaskScheduler.h>
void txCallback();

// Tasks: Task(repetition period (ms), number of times (or TASK_FOREVER), &callback function)
Task txTask(1000, TASK_FOREVER, &txCallback);

Scheduler runner;

// LoRa pins for RadioLib (NSS-CS/DIO0/RESET/DIO1)
//  - LILYGO TTGO ESP32 LoRa32 v2.1 T3_1.6: 18, 26, 23, 33
//  - Heltec WiFi LoRa32 v1:                18, 26, 14, 33
//  - BSFrance LoRa32u4II v1.2 pins:         8,  7,  4,  1
SX1276 radio = new Module(18, 26, 23, 33);

// SSD1306 OLED display pins
// For TTGO T3_V1.6: SDA/SCL/RST 21/22/16
// For Heltec ESP32: SDA/SCL/RST 4/15/16

#ifdef ARDUINO_TTGO_LoRa32_V1
#define HASOLEDSSD1306 true
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#endif

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// LoRa local address
byte localAddress = 0x00;

// Duty cycle end
unsigned long dutyCycleEnd = 0;

// Minimum and maximum SFs to transmit at
#define MINSF 7
#define MAXSF 9

// LoRa packets counter
int SFpackets[MAXSF - MINSF + 1];
int SFindex = 0;

long packets = 0;

// Function declarations
void localDisplayDisplay();

void initializeLoRa () {
  Serial.println("LoRa module initialization...");

  Serial.print(F("[SX1276] Initializing ... "));

  // Carrier frequency:           868.1 MHz
  // Bandwidth:                   125.0 kHz
  // Spreading factor:            7
  // Coding rate:                 4/5
  // Sync word:                   0x14
  // Output power:                2 dBm
  // Preamble length:             16 symbols
  // Amplifier gain:              1 (maximum gain)
  int state = radio.begin(868.1, 125.0, MINSF, 5, 0x13, 2, 20, 1);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  Serial.println("Done!");
  Serial.println("");

  if (HASOLEDSSD1306) {
    display.setCursor(0, 0);
    display.print("LoRa initialized.");
    localDisplayDisplay();
  }
  delay (100);
}

void initializeLocalAddress () {
  byte WiFiMAC[6];

  WiFi.macAddress(WiFiMAC);
  localAddress = WiFiMAC[5];

  Serial.print("Local LoRa address (from WiFi MAC): ");
  Serial.println(localAddress, HEX);

  if (HASOLEDSSD1306) {
    display.setCursor(0, 10);
    display.print("LoRa address: ");
    display.print(localAddress, HEX);
    localDisplayDisplay();
  }
}

void initializeOLED () {
  //reset OLED display via software
  Serial.println("OLED initialization...");
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println("Done!");
  Serial.println("");

  localDisplayClearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  localDisplayDisplay();
}

void setup() {
  // Serial port initialization
  Serial.begin(115200);
  while (!Serial);

  // OLED display initialization
  if (HASOLEDSSD1306) {
    initializeOLED();
    delay(100);
  }

  // LoRa initialization
  initializeLoRa();
  delay(100);

  // Local address initialization
  initializeLocalAddress();

  delay(100);

  // Scheduler initialization
  runner.init();

  runner.addTask(txTask);
  txTask.enable();

  localDisplayClearDisplay();

  delay(5000);
}

void loop() {
  runner.execute();

  localDisplayClearDisplay();
  printOLED();
}

void sendPacket() {

  byte packetType = 0x04;

  Serial.print("Sending packet ");
  Serial.print(packets);
  Serial.print(" using SF");
  Serial.print(SFindex + MINSF);
  Serial.println();

  if (HASOLEDSSD1306) {
    printOLED();
  }

  //Send LoRa packet
  int state = radio.transmit("SF"+(String)(SFindex + MINSF)+"ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");

}



void txCallback() {
  Serial.print("TX callback at t=");
  Serial.print(millis());
  Serial.println(" millis.");

  if (dutyCycleEnd < micros()) {

    if (radio.setSpreadingFactor(MINSF + SFindex) == ERR_INVALID_SPREADING_FACTOR) {
      Serial.println(F("Selected spreading factor is invalid for this module!"));
    }
    else {
      Serial.println("SF set to " + (String)(MINSF + SFindex));
    }

    unsigned long transmissionStart = micros();
    sendPacket();
    unsigned long transmissionEnd = micros();

    packets++;
    SFpackets[SFindex]++;

    SFindex = (SFindex + 1) % (MAXSF - MINSF + 1);

    unsigned long timeToNextPacket = 0;

    // Avoid micros() rollover
    if ( transmissionEnd < transmissionStart ) {
      timeToNextPacket = 99 * (timeToNextPacket - 1 - transmissionStart + transmissionEnd);
    }
    // Default behaviour
    else {
      timeToNextPacket = 99 * (micros() - transmissionStart);
    }

    // Enforce a 1% duty cycle
    dutyCycleEnd = millis() + timeToNextPacket / 1000 + 1;

    Serial.print("Scheduling next HELLO packet in ");
    Serial.print(timeToNextPacket / 1000);
    Serial.println(" ms.");

    txTask.setInterval(5 * (timeToNextPacket) / 1000);


    Serial.println("");
  }
}

void printOLED() {
  if (HASOLEDSSD1306) {
    display.setCursor(0, 0);
    //display.print("Dev: ");
    display.print(String(ARDUINO_BOARD).substring(0, 21));
    display.setCursor(0, 10);
    display.print("Address: 0x");
    display.print(localAddress, HEX);
    display.setCursor(0, 20);
    display.print("Sent packets: ");
    display.print(packets);

    display.setCursor(0, 40);
    for ( int i = 0; i < MAXSF - MINSF + 1; i++) {
      display.print("SF");
      display.print(i + MINSF);
      display.print(": ");
      display.print(SFpackets[i]);
      display.print(" ");
    }
  }
  localDisplayDisplay();
}

void localDisplayClearDisplay() {
  if (HASOLEDSSD1306) {
    display.clearDisplay();
  }
}

void localDisplayDisplay() {
  if (HASOLEDSSD1306) {
    display.display();
  }
}
