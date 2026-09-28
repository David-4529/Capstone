# STM32CubeIDE Setup + main.c Integration

## 1. CubeMX configuration (do this first, in STM32CubeIDE)

If you haven't created the project yet: **File → New → STM32 Project → Board Selector
tab → search "NUCLEO-F401RE" → select it → Next → name it → Finish**. When it asks
"Initialize all peripherals with their default mode?", click **Yes** (this auto-sets up
USART2 on the ST-LINK virtual COM port for you).

In the Pinout & Configuration view:

1. Click pin **PA9** → set to `USART1_TX`.
2. Click pin **PA10** → set to `USART1_RX`.
   (These are Nucleo Arduino-header D8/D2. Selecting them auto-enables USART1.)
3. Left panel → **Connectivity → USART1** → confirm/set:
   - Mode: Asynchronous
   - Baud Rate: `9600` (must match the VFD's P09.01 - change both together if you use a different baud)
   - Word Length: 8 Bits
   - Parity: None
   - Stop Bits: 2
   (This is 8N2 - the drive's P09.04 has no 8N1 option for RTU mode, only 8N2/8E1/8O1.)
4. Click pin **PA8** → set to `GPIO_Output`. Right-click it → **Enter User Label** →
   type `RS485_DE_RE` (this makes the generated code use `RS485_DE_RE_Pin` /
   `RS485_DE_RE_GPIO_Port` instead of raw `GPIOA`/`GPIO_PIN_8` - just for readability).
5. Left panel → **System Core → NVIC** → check **USART2 global interrupt** (needed so
   we can receive serial commands from your PC without blocking the main loop).
6. **Project → Generate Code** (or the gear icon).

## 2. Add the Modbus files

Copy `Core/Inc/modbus.h` and `Core/Src/modbus.c` (from this same `firmware/stm32/`
folder in the repo) into your generated project's `Core/Inc/` and `Core/Src/`
directories.

## 3. Edit `Core/Src/main.c`

CubeMX regenerates everything outside `/* USER CODE BEGIN ... */` /
`/* USER CODE END ... */` markers, so all custom code goes inside those. Add the
following to the matching sections (search for the marker comment in your generated
file):

### `USER CODE BEGIN Includes`
```c
#include "modbus.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
```

### `USER CODE BEGIN PD` (private defines)
```c
#define VFD_SLAVE_ID 1          // must match the VFD's P09.00 - check in GSoft2
#define REG_OUTPUT_FREQUENCY 0x2103
#define REG_OUTPUT_CURRENT 0x2104
#define MOTOR_POLE_COUNT 4
#define LOG_INTERVAL_MS 2000
```

### `USER CODE BEGIN PV` (private variables)
```c
ModbusMaster vfd;

static uint32_t lastLogMs = 0;

static float knownLoadKg = 0.0f;
static float manualActualRpm = 0.0f;
static bool haveManualRpm = false;
static float manualFreqHz = 0.0f;
static bool haveManualFreq = false;
static float manualCurrentA = 0.0f;
static bool haveManualCurrent = false;

static uint8_t rxByte;
static char lineBuf[64];
static volatile uint8_t lineLen = 0;
static volatile bool lineReady = false;
```

### `USER CODE BEGIN PFP` (private function prototypes)
```c
static void UartPrint(const char *s);
static void HandleLine(const char *line);
static void LogDataPoint(void);
```

### `USER CODE BEGIN 2` (after all `MX_..._Init()` calls, before `while (1)`)
```c
Modbus_Init(&vfd, &huart1, RS485_DE_RE_GPIO_Port, RS485_DE_RE_Pin, VFD_SLAVE_ID);

HAL_UART_Receive_IT(&huart2, &rxByte, 1);

UartPrint("# millis,freq_hz,freq_source,sync_rpm,actual_rpm,slip,current_a,current_source,known_load_kg\r\n");
UartPrint("# send 'LOAD <kg>' 'RPM <value>' 'FREQ <hz>' 'CURRENT <amps>' over serial\r\n");
```

### `USER CODE BEGIN WHILE` (inside `while (1) { ... }`)
```c
if (lineReady) {
  lineReady = false;
  HandleLine(lineBuf);
  lineLen = 0;
}

uint32_t now = HAL_GetTick();
if (now - lastLogMs >= LOG_INTERVAL_MS) {
  lastLogMs = now;
  LogDataPoint();
}
```

### `USER CODE BEGIN 4` (function definitions, after `MX_GPIO_Init` etc.)
```c
static void UartPrint(const char *s) {
  HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    if (rxByte == '\n' || rxByte == '\r') {
      if (lineLen > 0) {
        lineBuf[lineLen] = '\0';
        lineReady = true;
      }
    } else if (lineLen < sizeof(lineBuf) - 1) {
      lineBuf[lineLen++] = (char)rxByte;
    }
    HAL_UART_Receive_IT(&huart2, &rxByte, 1);
  }
}

static void HandleLine(const char *line) {
  float value;
  char msg[48];
  if (sscanf(line, "LOAD %f", &value) == 1) {
    knownLoadKg = value;
    snprintf(msg, sizeof(msg), "# known load set to %.3f kg\r\n", knownLoadKg);
    UartPrint(msg);
  } else if (sscanf(line, "RPM %f", &value) == 1) {
    manualActualRpm = value;
    haveManualRpm = true;
    snprintf(msg, sizeof(msg), "# actual RPM set to %.1f\r\n", manualActualRpm);
    UartPrint(msg);
  } else if (sscanf(line, "FREQ %f", &value) == 1) {
    manualFreqHz = value;
    haveManualFreq = true;
    snprintf(msg, sizeof(msg), "# commanded frequency set to %.2f Hz\r\n", manualFreqHz);
    UartPrint(msg);
  } else if (sscanf(line, "CURRENT %f", &value) == 1) {
    manualCurrentA = value;
    haveManualCurrent = true;
    snprintf(msg, sizeof(msg), "# output current set to %.2f A\r\n", manualCurrentA);
    UartPrint(msg);
  }
}

static void LogDataPoint(void) {
  uint16_t raw;
  float freqHz;
  const char *freqSource;
  if (Modbus_ReadHoldingRegister(&vfd, REG_OUTPUT_FREQUENCY, &raw)) {
    freqHz = raw / 100.0f;
    freqSource = "modbus";
  } else if (haveManualFreq) {
    freqHz = manualFreqHz;
    freqSource = "manual";
  } else {
    UartPrint("# no frequency available (modbus read failed and no FREQ <hz> entered)\r\n");
    return;
  }

  float syncRpm = 120.0f * freqHz / MOTOR_POLE_COUNT;

  bool haveRpm = haveManualRpm;
  float actualRpm = manualActualRpm;
  float slip = 0.0f;
  if (haveRpm && syncRpm > 0.0f) {
    slip = (syncRpm - actualRpm) / syncRpm;
  }

  float currentA;
  const char *currentSource;
  bool haveCurrent;
  if (Modbus_ReadHoldingRegister(&vfd, REG_OUTPUT_CURRENT, &raw)) {
    currentA = raw / 10.0f;
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

  char rpmField[16];
  char slipField[16];
  char currentField[16];
  if (haveRpm) {
    snprintf(rpmField, sizeof(rpmField), "%.1f", actualRpm);
    snprintf(slipField, sizeof(slipField), "%.4f", slip);
  } else {
    strcpy(rpmField, "NA");
    strcpy(slipField, "NA");
  }
  if (haveCurrent) {
    snprintf(currentField, sizeof(currentField), "%.2f", currentA);
  } else {
    strcpy(currentField, "NA");
  }

  char line[160];
  snprintf(line, sizeof(line), "%lu,%.2f,%s,%.1f,%s,%s,%s,%s,%.3f\r\n",
           (unsigned long)HAL_GetTick(), freqHz, freqSource, syncRpm, rpmField,
           slipField, currentField, currentSource, knownLoadKg);
  UartPrint(line);
}
```

## 4. Build and flash

Build (hammer icon), then Run/Debug to flash over the ST-LINK (built into the Nucleo
board - no separate programmer needed).

## 5. Verify it's working (see chat for the full checklist)

Open a serial terminal to the Nucleo's ST-LINK VCP port at 9600 baud (check baud - this
matches USART2's default from the board init, not the Modbus 9600 above; if your
terminal shows garbage, try the board's default VCP baud, commonly 115200, and let me
know what you see so we can reconcile it). You should see the header lines, then a new
CSV row every 2 seconds with `freq_source`/`current_source` showing `modbus` if the
RS-485 link is working, or the fields reading `NA` if not (with an error line explaining
why). Cross-check any `modbus`-sourced value against GSoft2's live monitor at the same
moment.
