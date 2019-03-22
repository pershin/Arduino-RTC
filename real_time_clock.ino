/*
  Arduino Real Time Clock (RTC)

  created 21 Mar 2019
  by Sergey Pershin
*/

#include <Wire.h>

#define DS1307_ADDRESS 0b1101000
#define DS1307_SECONDS 0
#define DS1307_MINUTES 1
#define DS1307_HOURS   2

byte timekeeperRegisters[3] = {0, 0, 0};
volatile bool refresh = false;
volatile uint8_t button = 0;

void isrRefresh() {
  refresh = true;
}

void setup() {
  Wire.begin();

  attachInterrupt(digitalPinToInterrupt(2), isrRefresh, RISING);

  Serial.begin(9600);
}

void loop() {
  if (refresh) {
    refresh = false;

    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(DS1307_ADDRESS, 3);

    timekeeperRegisters[DS1307_SECONDS] = Wire.read();
    timekeeperRegisters[DS1307_MINUTES] = Wire.read();
    timekeeperRegisters[DS1307_HOURS] = Wire.read();

    if (0 < button) {
      switch (button) {
        case 1:
          if (0b00100011 == timekeeperRegisters[DS1307_HOURS]) { // 23
            timekeeperRegisters[DS1307_HOURS] = 0b0;
          } else {
            timekeeperRegisters[DS1307_HOURS]++;

            if (9 < (timekeeperRegisters[DS1307_HOURS] & ~0b11110000)) {
              timekeeperRegisters[DS1307_HOURS] += 0b00000110;
            }
          }

          Wire.beginTransmission(DS1307_ADDRESS);
          Wire.write(DS1307_HOURS);
          Wire.write(timekeeperRegisters[DS1307_HOURS]);
          Wire.endTransmission();
          break;
        case 2:
          if (0b01011001 == timekeeperRegisters[DS1307_MINUTES]) { // 59
            timekeeperRegisters[DS1307_MINUTES] = 0b0;
          } else {
            timekeeperRegisters[DS1307_MINUTES]++;

            if (9 < (timekeeperRegisters[DS1307_MINUTES] & ~0b11110000)) {
              timekeeperRegisters[DS1307_MINUTES] += 0b00000110;
            }
          }

          Wire.beginTransmission(DS1307_ADDRESS);
          Wire.write(DS1307_MINUTES);
          Wire.write(timekeeperRegisters[DS1307_MINUTES]);
          Wire.endTransmission();
      }

      button = 0;
    }

    updateDisplay();
  }
}

void updateDisplay() {
  char timeStr[9] = "00:00:00";

  timeStr[0] = (timekeeperRegisters[DS1307_HOURS] >> 4) + 48;
  timeStr[1] = (timekeeperRegisters[DS1307_HOURS] & ~0b11110000) + 48;
  timeStr[3] = (timekeeperRegisters[DS1307_MINUTES] >> 4) + 48;
  timeStr[4] = (timekeeperRegisters[DS1307_MINUTES] & ~0b11110000) + 48;
  timeStr[6] = (timekeeperRegisters[DS1307_SECONDS] >> 4) + 48;
  timeStr[7] = (timekeeperRegisters[DS1307_SECONDS] & ~0b11110000) + 48;

  Serial.println(timeStr);
}

void serialEvent() {
  while (Serial.available()) {
    int value = Serial.parseInt();
    if (0 < value) {
      button = value;
      refresh = true;
    }
  }
}
