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


// LoRa channels to use
#define MINCHAN 1
#define MAXCHAN 2
float FREQS[8] = {868.1, 868.3, 868.5, 867.1, 867.3, 867.5, 867.7, 867.9};
int CHANindex = MINCHAN - 1;
//CHANINDEX GOES FROM MINCHAN-1 to MAXCHAN-1

// LoRa hardware-defined MIN and MAX SFs
#define MINSF_HW 6  //DO NOT MODIFY
#define MAXSF_HW 12 //DO NOT MODIFY

//LoRa SF equal-weighted array to perform CAD scan
#define EqWSFsize 127
int EqWSFarray[EqWSFsize] = {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
int EqWSFindex = 0;

// Actual minimum and maximum SFs to be used
#define MINSF 7
#define MAXSF 8

// SFindex will go from 0 (corresponding to MINSF) to MAXSF-MINSF (corresponding to MAXSF)
int SFindex = 0;

// LoRa packets counter
int CHANSFpackets[(MAXCHAN - MINCHAN + 1) * (MAXSF - MINSF + 1)][2];


long packets = 0;

// Function declarations
void localDisplayDisplay();

void initializeLoRa () {
  Serial.println("LoRa module initialization...");

  Serial.print(F("[SX1276] Initializing ... "));
  // carrier frequency:           868.1 MHz
  // bandwidth:                   125.0 kHz
  // spreading factor:            7
  // coding rate:                 4/5
  // sync word:                   0x14
  // output power:                2 dBm
  // preamble length:             20 symbols
  // amplifier gain:              1 (maximum gain)

  int state = radio.begin(FREQS[CHANindex+MINCHAN-1], 125.0, MINSF, 5, 0x13, 2, 20, 1);

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

  // Serial monitor initialization
  Serial.begin(115200);
  while (!Serial);

  // OLED display initialization
  if (HASOLEDSSD1306) {
    initializeOLED();
    delay(1000);
  }

  // LoRa initialization
  initializeLoRa();
  delay(1000);

  // Local address initialization
  initializeLocalAddress();

  delay(1000);

  localDisplayClearDisplay();
}

void loop() {
  // Start scanning current channel on SFindex+MINSF
  //  Serial.print("Scanning on SF");
  //  Serial.println(MINSF + SFindex);
  int preambleState = radio.scanChannel();

  if (preambleState == PREAMBLE_DETECTED) {
    // LoRa preamble was detected
    Serial.print(F("Preamble detected on SF"));
    Serial.println(MINSF + SFindex);

    // Data are received as an Arduino String.
    String rxStr;

    // Note: receive() is a blocking method.
    int rxState = radio.receive(rxStr);

    if (rxState == ERR_RX_TIMEOUT) {
      // Most of the times a timeout will occur.
      // Go to the beginning of the loop doing nothing
      // to start reception again.
    } else if (rxState == ERR_NONE) {

      // packet was successfully received
      packets++;
      CHANSFpackets[CHANindex][SFindex]++;
      Serial.println(F("Packet received."));

      // print the data of the packet
      Serial.print(F("[SX1276] Data:\t\t\t"));
      Serial.println(rxStr);

      // print the frequency
      // of the last received packet
      Serial.print(F("[SX1276] Freq:\t\t\t"));
      Serial.print(FREQS[CHANindex]);
      Serial.println(F(" MHz"));

      // print the RSSI (Received Signal Strength Indicator)
      // of the last received packet
      Serial.print(F("[SX1276] RSSI:\t\t\t"));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));

      // print the SNR (Signal-to-Noise Ratio)
      // of the last received packet
      Serial.print(F("[SX1276] SNR:\t\t\t"));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

      // print frequency error
      // of the last received packet
      Serial.print(F("[SX1276] Freq. dev.:\t\t\t"));
      Serial.print(radio.getFrequencyError());
      Serial.println(F(" Hz"));
      Serial.println(F(""));
      localDisplayClearDisplay();
      printOLED();
    } else if (rxState == ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println("");
      Serial.println(F("CRC error!"));
    } else {
      // some other error occurred
      Serial.println("");
      Serial.print(F("failed, code "));
      Serial.println(rxState);
    }

  } // End if preamble detected clause

  do {
    EqWSFindex = (EqWSFindex + 1 ) % (EqWSFsize);
  } while ( EqWSFarray[EqWSFindex] + MINSF_HW < MINSF || EqWSFarray[EqWSFindex] + MINSF_HW > MAXSF );

  SFindex = (EqWSFarray[EqWSFindex] - MINSF + MINSF_HW);
  CHANindex = (CHANindex + 1) % (MAXCHAN - MINCHAN + 1);

  //    Serial.print(EqWSFindex);
  //    Serial.print(" ");
  //    Serial.print(SFindex);
  //    Serial.print(" ");
  //    Serial.println(SFindex + MINSF);
  if (radio.setFrequency(FREQS[CHANindex+MINCHAN-1]) == ERR_INVALID_FREQUENCY) {
      Serial.println(F("Selected frequency is invalid for this module!"));
    }
//  else {
//    Serial.println("Frequency set to " + (String)(FREQS[CHANindex+MINCHAN-1]));
//  }

  if (radio.setSpreadingFactor(SFindex + MINSF) == ERR_INVALID_SPREADING_FACTOR) {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
  }
//  else {
//      Serial.println("SF set to " + (String)(MINSF + SFindex));
//  }
}


void receiveDataPacket() {

  byte packetType = 0x03;
  String outMessage = "AAA";

  Serial.print("Sending DATA packet ");
  //Serial.println(dataCounter);
  Serial.println();
  if (HASOLEDSSD1306) {
    printOLED();
  }

  //dataCounter++;
}


void onReceive(int packetSize) {
  if (packetSize == 0)
    return;
}

void printOLED() {
  if (HASOLEDSSD1306) {
    display.setCursor(0, 0);
    //display.print("Dev: ");
    display.print(String(ARDUINO_BOARD).substring(0, 21));
    //display.setCursor(0, 10);
    //display.print("Address: 0x");
    display.print(" ");
    display.print(localAddress, HEX);
    display.setCursor(0, 10);
    display.print("Received packets: ");
    display.print(packets);

    display.setCursor(0, 20);
    for ( int i = 0; i < MAXCHAN - MINCHAN + 1; i++) {
      display.print("CH");
      display.print(i + MINCHAN);
      display.print(" ");
      for ( int j = 0; j < MAXSF - MINSF + 1; j++) {
        display.print("SF");
        display.print(j + MINSF);
        display.print(": ");
        display.print(CHANSFpackets[i][j]);
        display.print(" ");
      }
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
