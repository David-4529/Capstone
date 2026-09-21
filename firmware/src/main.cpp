#include <Arduino.h>
#include <ModbusMaster.h>

#include "pins.h"

// Data-acquisition node for the sensorless load estimation rig.
// Confirmed data path: VFD output frequency via Modbus RTU (see docs/wiring.md).
// Everything else is stubbed per docs/open-items.md until its interface is confirmed.

namespace {

ModbusMaster vfd;
HardwareSerial &vfdSerial = Serial2;

constexpr uint32_t LOG_INTERVAL_MS = 2000;
uint32_t lastLogMs = 0;

float knownLoadKg = 0.0f; // set via serial command LOAD <kg>, see handleSerialCommands()

void preTransmission() { digitalWrite(PIN_RS485_DE_RE, HIGH); }
void postTransmission() { digitalWrite(PIN_RS485_DE_RE, LOW); }

// Reads output frequency (Hz) from the VFD over Modbus RTU. Confirmed register.
bool readOutputFrequencyHz(float &hzOut) {
  uint8_t result = vfd.readHoldingRegisters(REG_OUTPUT_FREQUENCY, 1);
  if (result != vfd.ku8MBSuccess) {
    return false;
  }
  // GS20-series reports frequency in 0.01 Hz units - confirm scaling against
  // the manual's register table if logged values look off by 100x.
  hzOut = vfd.getResponseBuffer(0) / 100.0f;
  return true;
}

// TODO(open-item-2): register address is a placeholder until confirmed from the
// GS20/GS20X manual Ch.5 Status Monitor table. Returns false until then.
bool readOutputCurrentA(float &ampsOut) {
  if (REG_OUTPUT_CURRENT == 0x0000) {
    return false;
  }
  uint8_t result = vfd.readHoldingRegisters(REG_OUTPUT_CURRENT, 1);
  if (result != vfd.ku8MBSuccess) {
    return false;
  }
  ampsOut = vfd.getResponseBuffer(0) / 100.0f;
  return true;
}

// TODO(open-item-4): Lucas Nulle SERVO Machine Test System interface not yet
// confirmed (analog voltage / digital pulse / serial). Replace this stub once
// known - see docs/wiring.md section 2.4 for the candidate wiring per case.
bool readActualRpm(float &rpmOut) {
  (void)rpmOut;
  return false;
}

void handleSerialCommands() {
  if (!Serial.available()) {
    return;
  }
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.startsWith("LOAD ")) {
    knownLoadKg = line.substring(5).toFloat();
    Serial.print(F("# known load set to "));
    Serial.print(knownLoadKg, 3);
    Serial.println(F(" kg"));
  }
}

void logDataPoint() {
  float freqHz;
  if (!readOutputFrequencyHz(freqHz)) {
    Serial.println(F("# modbus read failed (output frequency)"));
    return;
  }

  float syncRpm = 120.0f * freqHz / MOTOR_POLE_COUNT;

  float actualRpm;
  bool haveRpm = readActualRpm(actualRpm);

  float slip = NAN;
  if (haveRpm && syncRpm > 0.0f) {
    slip = (syncRpm - actualRpm) / syncRpm;
  }

  float currentA;
  bool haveCurrent = readOutputCurrentA(currentA);

  // CSV: millis,freq_hz,sync_rpm,actual_rpm,slip,current_a,known_load_kg
  Serial.print(millis());
  Serial.print(',');
  Serial.print(freqHz, 2);
  Serial.print(',');
  Serial.print(syncRpm, 1);
  Serial.print(',');
  Serial.print(haveRpm ? String(actualRpm, 1) : String("NA"));
  Serial.print(',');
  Serial.print(haveRpm ? String(slip, 4) : String("NA"));
  Serial.print(',');
  Serial.print(haveCurrent ? String(currentA, 2) : String("NA"));
  Serial.print(',');
  Serial.println(knownLoadKg, 3);
}

} // namespace

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RS485_DE_RE, OUTPUT);
  digitalWrite(PIN_RS485_DE_RE, LOW);

  vfdSerial.begin(MODBUS_BAUD, SERIAL_8N1, PIN_RS485_RX, PIN_RS485_TX);
  vfd.begin(VFD_MODBUS_SLAVE_ID, vfdSerial);
  vfd.preTransmission(preTransmission);
  vfd.postTransmission(postTransmission);

  Serial.println(F("# millis,freq_hz,sync_rpm,actual_rpm,slip,current_a,known_load_kg"));
  Serial.println(F("# send 'LOAD <kg>' over serial before each test point"));
}

void loop() {
  handleSerialCommands();

  uint32_t now = millis();
  if (now - lastLogMs >= LOG_INTERVAL_MS) {
    lastLogMs = now;
    logDataPoint();
  }
}
