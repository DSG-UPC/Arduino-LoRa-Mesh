// Tested with RadioLib v4.6.0. Release v5.0.0 introduces backwards-incompatible
// changes to the public API.
// For more details, see https://github.com/jgromes/RadioLib/releases/tag/5.0.0
#include <RadioLib.h>

// BSFrance LoRa32u4II v1.2 pins:              8,  7,  4,  1
// LILYGO TTGO ESP32 LoRa32 v2.1 T3_1.6 pins: 18, 26, 23, 33
SX1276 radio = new Module(18, 26, 23, 33);

void setup() {
  // Serial port speed
  Serial.begin(115200);
  delay(100);

  Serial.print(F("[SX1276] Initializing... "));

  // Carrier frequency:           868.1 MHz
  // Bandwidth:                   125.0 kHz
  // Spreading factor:            7
  // Coding rate:                 4/5
  // Sync word:                   0x14
  // Output power:                2 dBm
  // Preamble length:             16 symbols
  // Amplifier gain:              1 (maximum gain)
  int state = radio.begin(868.1, 125, 7, 5, 0x13, 2, 16, 1);

  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
}

void loop() {
  Serial.print(F("[SX1276] Transmitting packet ... "));

  // A C-string, or an Arduino string, up to 256 characters long can be sent.
  int state = radio.transmit("0123456789ABCDEF");

  if (state == ERR_NONE) {
    // the packet was successfully transmitted
    //Serial.println(F(" success!"));

    // print measured data rate
    Serial.print(F("[SX1276] Datarate:\t"));
    Serial.print(radio.getDataRate());
    Serial.println(F(" bps"));

  } else if (state == ERR_PACKET_TOO_LONG) {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("packet too long!"));

  } else if (state == ERR_TX_TIMEOUT) {
    // timeout occurred while transmitting packet
    Serial.println(F("timeout!"));

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);

  }

  // Wait 5 seconds before transmitting the next packet
  delay(5000);
}
