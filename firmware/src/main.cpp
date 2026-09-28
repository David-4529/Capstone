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

// Manual shaft RPM entry: interim workaround for open-item-4 (Lucas Nulle SERVO
// system's electrical interface isn't confirmed yet, but its display is readable
// by eye). Set via serial command RPM <value>, see handleSerialCommands().
float manualActualRpm = 0.0f;
bool haveManualRpm = false;

// Manual frequency entry: fallback for open-item-3 (RS-485 wiring from the ESP32
// to the VFD's control terminal block isn't confirmed/landed yet, so the Modbus
// read below may not be reachable). Set via serial command FREQ <hz>, see
// handleSerialCommands(). Modbus is still tried first and preferred when it works.
float manualFreqHz = 0.0f;
bool haveManualFreq = false;

// Manual current entry: fallback for open-item-2/3 (current register unconfirmed
// and/or RS-485 to the VFD not wired yet). Read off the GSoft2 live monitor over
// the drive's USB connection instead. Set via serial command CURRENT <amps>, see
// handleSerialCommands(). Modbus is still tried first and preferred when it works.
float manualCurrentA = 0.0f;
bool haveManualCurrent = false;

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

// Reads output current (A) from the VFD over Modbus RTU. Register 0x2104,
// format XXX.X A -> divide by 10 (not 100 - different scale than frequency).
bool readOutputCurrentA(float &ampsOut) {
  uint8_t result = vfd.readHoldingRegisters(REG_OUTPUT_CURRENT, 1);
  if (result != vfd.ku8MBSuccess) {
    return false;
  }
  ampsOut = vfd.getResponseBuffer(0) / 10.0f;
  return true;
}

// TODO(open-item-4): Lucas Nulle SERVO Machine Test System's electrical interface
// is not yet confirmed (analog voltage / digital pulse / serial) - see
// docs/wiring.md section 2.4 for the candidate wiring per case. Until then, this
// returns whatever was last entered via the RPM <value> serial command (read by
// eye off the SERVO system's own display). Replace with a real sensor read once
// the interface is known.
bool readActualRpm(float &rpmOut) {
  if (!haveManualRpm) {
    return false;
  }
  rpmOut = manualActualRpm;
  return true;
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
  } else if (line.startsWith("RPM ")) {
    manualActualRpm = line.substring(4).toFloat();
    haveManualRpm = true;
    Serial.print(F("# actual RPM set to "));
    Serial.println(manualActualRpm, 1);
  } else if (line.startsWith("FREQ ")) {
    manualFreqHz = line.substring(5).toFloat();
    haveManualFreq = true;
    Serial.print(F("# commanded frequency set to "));
    Serial.print(manualFreqHz, 2);
    Serial.println(F(" Hz"));
  } else if (line.startsWith("CURRENT ")) {
    manualCurrentA = line.substring(8).toFloat();
    haveManualCurrent = true;
    Serial.print(F("# output current set to "));
    Serial.print(manualCurrentA, 2);
    Serial.println(F(" A"));
  }
}

void logDataPoint() {
  float freqHz;
  const char *freqSource;
  if (readOutputFrequencyHz(freqHz)) {
    freqSource = "modbus";
  } else if (haveManualFreq) {
    freqHz = manualFreqHz;
    freqSource = "manual";
  } else {
    Serial.println(F("# no frequency available (modbus read failed and no FREQ <hz> entered)"));
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
  const char *currentSource;
  bool haveCurrent;
  if (readOutputCurrentA(currentA)) {
    currentSource = "modbus";
    haveCurrent = true;
  } else if (haveManualCurrent) {
    currentA = manualCurrentA;
    currentSource = "manual";
    haveCurrent = true;
  } else {
    currentSource = "NA";
    haveCurrent = false;
  }

  // CSV: millis,freq_hz,freq_source,sync_rpm,actual_rpm,slip,current_a,current_source,known_load_kg
  Serial.print(millis());
  Serial.print(',');
  Serial.print(freqHz, 2);
  Serial.print(',');
  Serial.print(freqSource);
  Serial.print(',');
  Serial.print(syncRpm, 1);
  Serial.print(',');
  Serial.print(haveRpm ? String(actualRpm, 1) : String("NA"));
  Serial.print(',');
  Serial.print(haveRpm ? String(slip, 4) : String("NA"));
  Serial.print(',');
  Serial.print(haveCurrent ? String(currentA, 2) : String("NA"));
  Serial.print(',');
  Serial.print(currentSource);
  Serial.print(',');
  Serial.println(knownLoadKg, 3);
}

} // namespace

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RS485_DE_RE, OUTPUT);
  digitalWrite(PIN_RS485_DE_RE, LOW);

  // 8N2: the drive's P09.04 has no 8N1 option for RTU mode - see pins.h note.
  vfdSerial.begin(MODBUS_BAUD, SERIAL_8N2, PIN_RS485_RX, PIN_RS485_TX);
  vfd.begin(VFD_MODBUS_SLAVE_ID, vfdSerial);
  vfd.preTransmission(preTransmission);
  vfd.postTransmission(postTransmission);

  Serial.println(F("# millis,freq_hz,freq_source,sync_rpm,actual_rpm,slip,current_a,current_source,known_load_kg"));
  Serial.println(F("# send 'LOAD <kg>' over serial before each weighted test point"));
  Serial.println(F("# send 'RPM <value>' over serial each time the SERVO display reading changes"));
  Serial.println(F("# send 'FREQ <hz>' and 'CURRENT <amps>' over serial if modbus isn't wired up yet"));
  Serial.println(F("# (read FREQ/CURRENT off the GSoft2 live monitor over USB; source column will read 'manual')"));
}

void loop() {
  handleSerialCommands();

  uint32_t now = millis();
  if (now - lastLogMs >= LOG_INTERVAL_MS) {
    lastLogMs = now;
    logDataPoint();
  }
}
