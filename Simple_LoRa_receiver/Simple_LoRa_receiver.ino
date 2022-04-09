// Tested with RadioLib v4.6.0. Release v5.0.0 introduces backwards-incompatible
// changes to the public API.
// For more details, see https://github.com/jgromes/RadioLib/releases/tag/5.0.0
#include <RadioLib.h>

// BSFrance LoRa32u4II v1.2 pins:              8,  7,  4,  1
// LILYGO TTGO ESP32 LoRa32 v2.1 T3_1.6 pins: 18, 26, 23, 33
SX1276 radio = new Module(18, 26, 23, 33);

int received = 0;

void setup() {
  // Serial port speed
  Serial.begin(115200);
  delay(100);

  Serial.print(F("[SX1276] Initializing ... "));

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
  Serial.print(F("[SX1276] Waiting for incoming transmission... "));

  // A C-string, or an Arduino string, up to 256 characters long can be received.
  String rxstr;
  int state = radio.receive(rxstr);

  // you can also receive data as byte array
  /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */

  if (state == ERR_NONE) {
    // Packet successfully received
    Serial.println(F("success!"));
    received++;

    Serial.print(F("Packet #"));
    Serial.println(received);
    Serial.println(rxstr);

    Serial.print(F("Length: "));
    Serial.println(sizeof(rxstr));

    Serial.print(F("RSSI: "));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    Serial.print(F("SNR: "));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    Serial.print(F("Freq. deviation: "));
    Serial.print(radio.getFrequencyError());
    Serial.println(F(" Hz"));

  } else if (state == ERR_RX_TIMEOUT) {
    // Timeout occurred while waiting for a packet
    Serial.println(F("timeout!"));

  } else if (state == ERR_CRC_MISMATCH) {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}
