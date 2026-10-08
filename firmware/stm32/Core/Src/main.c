/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VFD_SLAVE_ID 1          // must match the VFD's P09.00 - check in GSoft2
#define REG_OUTPUT_FREQUENCY 0x2103
#define REG_OUTPUT_CURRENT 0x2104
#define REG_FREQ_COMMAND 0x2102 // frequency setpoint the drive is following (= pot position when P00.20 = 2)
// Parameter Pxx.yy lives at Modbus address 0xXXYY with xx/yy converted to hex (GS20 manual ch. 5)
#define PARAM_ADDR(group, num) ((uint16_t)(((group) << 8) | (num)))
#define REG_P00_20_FREQ_SOURCE PARAM_ADDR(0, 20)
#define REG_P00_21_RUN_SOURCE PARAM_ADDR(0, 21)
#define REG_P03_00_AI1_FUNCTION PARAM_ADDR(3, 0)
#define REG_P00_22_STOP_METHOD PARAM_ADDR(0, 22)
#define REG_P01_10_FREQ_UPPER_LIMIT PARAM_ADDR(1, 10)
#define REG_P01_12_ACCEL_TIME PARAM_ADDR(1, 12)
#define REG_P01_13_DECEL_TIME PARAM_ADDR(1, 13)
#define EXPECTED_FREQ_SOURCE 2 // P00.20 = 2: external analog input (pot on +10V/AI1/ACM)
#define EXPECTED_RUN_SOURCE 1  // P00.21 = 1: external terminals (FWD/REV pushbuttons)
#define EXPECTED_AI1_FUNCTION 1 // P03.00 = 1: AI1 is the frequency command
#define EXPECTED_STOP_METHOD 0  // P00.22 = 0: ramp to stop
// Highest frequency the cable travel can safely handle right now. VFDCHECK fails if
// P01.10 is set above this, and the CSV log warns if output frequency exceeds it.
// Raise it (together with P01.10) only after each step has been run safely.
#define SAFE_MAX_FREQ_HZ 10.0f
#define MOTOR_POLE_COUNT 4
// 250 ms so a run of only a few seconds (64 in of cable travel) still logs plenty of
// rows. Each row's 3 Modbus reads take ~75 ms at 9600 baud.
#define LOG_INTERVAL_MS 250
// A row counts as steady when output frequency is above this and moved no more
// than STEADY_FREQ_TOL_HZ since the previous row (i.e. not ramping).
#define STEADY_MIN_FREQ_HZ 0.5f
#define STEADY_FREQ_TOL_HZ 0.05f
// Test averages use only the highest steady speed held during the test. Steady rows
// more than this below it (pauses while turning the pot up, slowing down) are skipped.
#define PLATEAU_TOL_HZ 0.2f
// Nucleo's green user LED LD2 (PA5): a status light that works with no serial link.
// Slow blink (1 s) = running and the VFD is answering Modbus.
// Fast blink (0.1 s) = running but the VFD is not answering (check RS-485 wiring).
// Solid on = stopped in Error_Handler. Off = not running at all.
#define STATUS_LED_PORT GPIOA
#define STATUS_LED_PIN GPIO_PIN_5
#define LED_BLINK_OK_MS 1000
#define LED_BLINK_FAIL_MS 100

// 16x2 HD44780 character LCD via a PCF8574 I2C backpack (I2C1, PB8=SCL/PB9=SDA -
// both free; see firmware/stm32/README.md for the CubeMX step to enable I2C1).
// Most backpacks are at 0x27; some clones are 0x3F - change this if the display
// stays blank (a backlight that lights but shows no characters is still a wiring
// problem, not an address problem).
#define LCD_I2C_ADDR (0x27 << 1)
#define LCD_COLS 16
// Redraw at most this often - the predicted weight doesn't need to update faster
// than a human can read it, and it keeps I2C traffic off the RS-485 polling loop.
#define LCD_UPDATE_INTERVAL_MS 500

// Live weight estimate: load_kg = slope_kg_per_a * current_a + intercept_kg, using
// whichever row's freq_hz is closest to the drive's current output frequency.
// UP (LIFTING) DIRECTION ONLY - see tools/model/fit_model.py and
// docs/results-log.md for why down-direction current doesn't track load on this
// open-loop V/Hz drive. There is no automatic way yet to tell whether the rig is
// lifting or lowering (FWD/REV wires straight to the VFD, not through the MCU), so
// this number should be read assuming an active lift and ignored while lowering.
//
// Regenerate this table after adding calibration data:
//   python tools/model/fit_model.py
// and paste the printed LOAD_MODEL array back in here (tools/model/load_model.h
// has the same table, kept as a single self-contained main.c on purpose - see
// firmware/stm32/README.md).
typedef struct {
  float freq_hz;
  float slope_kg_per_a;
  float intercept_kg;
} LoadModelEntry;
#define LOAD_MODEL_N 4
static const LoadModelEntry LOAD_MODEL[LOAD_MODEL_N] = {
  { 1.39f, 8.6762f, -4.1364f }, // r2=0.998, n=3, current 0.476-0.542 A
  { 2.44f, 9.7665f, -4.6141f }, // r2=0.991, n=4, current 0.466-0.619 A
  { 4.85f, 11.9175f, -9.7091f }, // r2=0.983, n=4, current 0.814-0.933 A
  { 6.36f, 11.2032f, -7.9392f }, // r2=0.992, n=4, current 0.704-0.837 A
};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
static uint32_t lastLogMs = 0;

static float knownLoadKg = 0.0f;
static float manualActualRpm = 0.0f;
static bool haveManualRpm = false;
static float manualFreqHz = 0.0f;
static bool haveManualFreq = false;
static float manualCurrentA = 0.0f;
static bool haveManualCurrent = false;

static uint8_t rxByte;
static char lineBuf[64];      // line being typed (filled by the UART interrupt)
static char cmdBuf[64];       // last complete line, handed to the main loop
static volatile uint8_t lineLen = 0;
static volatile bool lineReady = false;

// Latest readings, for watching in the debugger's Live Expressions view.
volatile float liveFreqHz = 0.0f;
volatile float liveCurrentA = 0.0f;
volatile bool liveCurrentValid = false;
volatile float livePotSetpointHz = 0.0f;
volatile uint32_t liveModbusOkCount = 0;
volatile uint32_t liveModbusFailCount = 0;
static volatile bool lastModbusOk = false;
static uint32_t lastLcdUpdateMs = 0;
static bool lcdReady = false;

// Test sessions: TEST asks for the weight and starts a numbered test, STOP ends it and
// prints a summary. Rows logged during a test carry its test_id (0 = no test running).
static uint16_t testId = 0;
static bool testActive = false;
static bool awaitingWeight = false;
static uint32_t testRows = 0;
// Steady-state averages only count rows where the motor was running at a constant
// speed (see STEADY_*). RPM can be typed any time before STOP - even after the motor
// has stopped - and the summary slip uses it with the steady-state average speed.
static uint32_t testSteadyRows = 0;
static float sumFreqHz = 0.0f;
static float sumCurrentA = 0.0f;
static float sumSyncRpm = 0.0f;
static float prevFreqHz = 0.0f;
static uint32_t lastLedToggleMs = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
static uint16_t ModbusCrc16(const uint8_t *buf, uint16_t len);
static bool ModbusReadHoldingRegister(uint16_t regAddr, uint16_t *value);
static void UartPrint(const char *s);
static void HandleLine(const char *line);
static void LogDataPoint(void);
static void CheckVfdPotControl(void);
static void StartTest(float loadKg);
static void EndTest(void);
static void Lcd_Init(void);
static void UpdateLoadDisplay(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(RS485_DE_RE_GPIO_Port, RS485_DE_RE_Pin, GPIO_PIN_RESET); // start in receive mode

  GPIO_InitTypeDef ledInit = {0};
  ledInit.Pin = STATUS_LED_PIN;
  ledInit.Mode = GPIO_MODE_OUTPUT_PP;
  ledInit.Pull = GPIO_NOPULL;
  ledInit.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STATUS_LED_PORT, &ledInit);
  HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET); // on during the startup check

  HAL_UART_Receive_IT(&huart2, &rxByte, 1);

  UartPrint("# millis,freq_hz,freq_source,sync_rpm,actual_rpm,slip,current_a,current_source,known_load_kg,test_id,steady\r\n");
  UartPrint("# send 'LOAD <kg>' 'RPM <value>' 'FREQ <hz>' 'CURRENT <amps>' over serial\r\n");
  UartPrint("# send 'DEBUG 0' to silence raw Modbus TX/RX hex dumps, 'DEBUG 1' to re-enable\r\n");
  UartPrint("# send 'VFDCHECK' to re-check that the drive is set up for pot speed control\r\n");

  CheckVfdPotControl();
  UartPrint("# type TEST to start a test (it asks for the weight), STOP to end it\r\n");

  Lcd_Init();
  lcdReady = true;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (lineReady) {
      HandleLine(cmdBuf);
      lineReady = false;
    }

    uint32_t now = HAL_GetTick();
    if (now - lastLogMs >= LOG_INTERVAL_MS) {
      lastLogMs = now;
      LogDataPoint();
    }

    now = HAL_GetTick();
    if (lcdReady && now - lastLcdUpdateMs >= LCD_UPDATE_INTERVAL_MS) {
      lastLcdUpdateMs = now;
      UpdateLoadDisplay();
    }

    now = HAL_GetTick();
    if (now - lastLedToggleMs >= (lastModbusOk ? LED_BLINK_OK_MS : LED_BLINK_FAIL_MS)) {
      lastLedToggleMs = now;
      HAL_GPIO_TogglePin(STATUS_LED_PORT, STATUS_LED_PIN);
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_2;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RS485_DE_RE_GPIO_Port, RS485_DE_RE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RS485_DE_RE_Pin */
  GPIO_InitStruct.Pin = RS485_DE_RE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RS485_DE_RE_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Minimal Modbus RTU master, function code 03 (Read Holding Registers), single
// register reads only - all this project needs (Output Frequency 0x2103, Output
// Current 0x2104 on the DURApulse GS20-series VFD). Half-duplex RS-485 via the
// DE/RE-tied MAX485 module, driven by RS485_DE_RE_Pin.
//
// Known simplification: assumes every successful response is exactly 7 bytes (a
// normal 1-register read). A Modbus exception response is shorter (5 bytes), so
// on an exception this will time out waiting for bytes that never arrive rather
// than decoding the exception code - it still correctly reports failure, just
// slower (up to the receive timeout) and without saying why.
static uint16_t ModbusCrc16(const uint8_t *buf, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for (int i = 0; i < 8; i++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// Raw TX/RX hex dumps. Off by default now that the RS-485 link is confirmed working;
// send 'DEBUG 1' over the serial port to turn them back on.
static bool modbusDebug = false;

static void UartPrintHexBytes(const char *label, const uint8_t *buf, uint16_t len) {
  char line[96];
  int pos = snprintf(line, sizeof(line), "#   %s:", label);
  for (uint16_t i = 0; i < len && pos < (int)sizeof(line) - 4; i++) {
    pos += snprintf(line + pos, sizeof(line) - pos, " %02X", buf[i]);
  }
  snprintf(line + pos, sizeof(line) - pos, "\r\n");
  UartPrint(line);
}

static bool ModbusReadHoldingRegister(uint16_t regAddr, uint16_t *value) {
  uint8_t request[8];
  request[0] = VFD_SLAVE_ID;
  request[1] = 0x03; // read holding registers
  request[2] = (regAddr >> 8) & 0xFF;
  request[3] = regAddr & 0xFF;
  request[4] = 0x00; // quantity high byte
  request[5] = 0x01; // quantity = 1 register
  uint16_t crc = ModbusCrc16(request, 6);
  request[6] = crc & 0xFF;        // CRC low byte first (Modbus RTU convention)
  request[7] = (crc >> 8) & 0xFF; // CRC high byte

  if (modbusDebug) {
    char hdr[48];
    snprintf(hdr, sizeof(hdr), "# modbus read reg 0x%04X:\r\n", regAddr);
    UartPrint(hdr);
    UartPrintHexBytes("TX", request, sizeof(request));
  }

  HAL_GPIO_WritePin(RS485_DE_RE_GPIO_Port, RS485_DE_RE_Pin, GPIO_PIN_SET);
  HAL_Delay(1); // let the transceiver settle before driving the line
  HAL_StatusTypeDef txStatus = HAL_UART_Transmit(&huart1, request, sizeof(request), 100);
  HAL_Delay(1); // safety margin before releasing the bus back to receive mode
  HAL_GPIO_WritePin(RS485_DE_RE_GPIO_Port, RS485_DE_RE_Pin, GPIO_PIN_RESET);
  if (txStatus != HAL_OK) {
    if (modbusDebug) {
      UartPrint("#   FAIL: HAL_UART_Transmit did not return HAL_OK\r\n");
    }
    liveModbusFailCount++;
    lastModbusOk = false;
    return false;
  }

  // slaveId, func, byteCount, data_hi, data_lo, crc_lo, crc_hi
  uint8_t response[7];
  if (HAL_UART_Receive(&huart1, response, sizeof(response), 200) != HAL_OK) {
    if (modbusDebug) {
      UartPrint("#   FAIL: no response within 200ms (check A/B wiring, baud, slave ID)\r\n");
    }
    liveModbusFailCount++;
    lastModbusOk = false;
    return false; // timeout - check wiring (A/B swapped?), baud, or slave ID
  }

  if (modbusDebug) {
    UartPrintHexBytes("RX", response, sizeof(response));
  }

  if (response[0] != VFD_SLAVE_ID || response[1] != 0x03 || response[2] != 2) {
    if (modbusDebug) {
      UartPrint("#   FAIL: unexpected slave ID / function code / byte count (exception response?)\r\n");
    }
    liveModbusFailCount++;
    lastModbusOk = false;
    return false; // wrong slave ID, exception response, or unexpected byte count
  }

  uint16_t receivedCrc = ModbusCrc16(response, 5);
  uint16_t frameCrc = (uint16_t)response[5] | ((uint16_t)response[6] << 8);
  if (receivedCrc != frameCrc) {
    if (modbusDebug) {
      UartPrint("#   FAIL: CRC mismatch (check 8N2 vs 8N1 framing)\r\n");
    }
    liveModbusFailCount++;
    lastModbusOk = false;
    return false; // CRC mismatch - usually a framing mismatch (check 8N2 vs 8N1)
  }

  *value = ((uint16_t)response[3] << 8) | response[4];
  liveModbusOkCount++;
  lastModbusOk = true;
  if (modbusDebug) {
    char ok[48];
    snprintf(ok, sizeof(ok), "#   OK: raw value = %u\r\n", *value);
    UartPrint(ok);
  }
  return true;
}

// Every message goes to both the ST-LINK virtual COM port (PuTTY/Tera Term) and the
// debugger's SWV ITM Data Console in STM32CubeIDE (via the SWO pin, PB3), so live
// data can be watched in the IDE with no serial terminal. ITM output is skipped
// unless a debugger has SWV tracing enabled.
static void UartPrint(const char *s) {
  HAL_UART_Transmit(&huart2, (uint8_t *)s, strlen(s), 100);
  for (const char *c = s; *c != '\0'; c++) {
    ITM_SendChar((uint32_t)*c);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    if (rxByte == '\n' || rxByte == '\r') {
      // Enter sends \r\n: the line ends on the first one, and resetting lineLen
      // here means the second one is ignored instead of repeating the command.
      if (lineLen > 0 && !lineReady) {
        lineBuf[lineLen] = '\0';
        memcpy(cmdBuf, lineBuf, lineLen + 1);
        lineReady = true;
      }
      lineLen = 0;
    } else if (lineLen < sizeof(lineBuf) - 1) {
      lineBuf[lineLen++] = (char)rxByte;
    }
    HAL_UART_Receive_IT(&huart2, &rxByte, 1);
  }
}

static void HandleLine(const char *line) {
  float value;
  char msg[112];
  if (awaitingWeight) {
    if (strcmp(line, "CANCEL") == 0) {
      awaitingWeight = false;
      UartPrint("# test cancelled\r\n");
    } else if (sscanf(line, "%f", &value) == 1 && value >= 0.0f) {
      awaitingWeight = false;
      StartTest(value);
    } else {
      UartPrint("# enter the weight in kg as a number (e.g. 2.5, or 0 for no load), or CANCEL\r\n");
    }
    return;
  }

  if (strcmp(line, "TEST") == 0) {
    if (testActive) {
      snprintf(msg, sizeof(msg), "# test %u is still running - type STOP first\r\n", testId);
      UartPrint(msg);
    } else {
      awaitingWeight = true;
      UartPrint("# TEST: what weight are you using? Enter it in kg (0 for no load):\r\n");
    }
  } else if (sscanf(line, "START %f", &value) == 1) {
    if (testActive) {
      snprintf(msg, sizeof(msg), "# test %u is still running - type STOP first\r\n", testId);
      UartPrint(msg);
    } else {
      StartTest(value);
    }
  } else if (strcmp(line, "STOP") == 0) {
    if (testActive) {
      EndTest();
    } else {
      UartPrint("# no test running - type TEST to start one\r\n");
    }
  } else if (strcmp(line, "VFDCHECK") == 0) {
    CheckVfdPotControl();
  } else if (sscanf(line, "LOAD %f", &value) == 1) {
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
  } else if (sscanf(line, "DEBUG %f", &value) == 1) {
    modbusDebug = (value != 0.0f);
    snprintf(msg, sizeof(msg), "# modbus debug logging %s\r\n", modbusDebug ? "ON" : "OFF");
    UartPrint(msg);
  } else {
    snprintf(msg, sizeof(msg), "# unknown command: '%s'\r\n", line);
    UartPrint(msg);
  }
}

static void StartTest(float loadKg) {
  char msg[96];
  testId++;
  testActive = true;
  knownLoadKg = loadKg;
  haveManualRpm = false; // force a fresh RPM reading for this test
  testRows = 0;
  testSteadyRows = 0;
  sumFreqHz = 0.0f;
  sumCurrentA = 0.0f;
  sumSyncRpm = 0.0f;
  snprintf(msg, sizeof(msg), "# TEST %u START load_kg=%.3f\r\n", testId, knownLoadKg);
  UartPrint(msg);
  UartPrint("# hold UP at a steady speed, read the RPM on the SERVO display, release before\r\n");
  UartPrint("# the cable runs out, then type RPM <value> and STOP.\r\n");
}

static void EndTest(void) {
  char msg[200];
  char freqField[12] = "NA";
  char currentField[12] = "NA";
  char rpmField[12] = "NA";
  char slipField[12] = "NA";
  testActive = false;
  if (testSteadyRows > 0) {
    float n = (float)testSteadyRows;
    float avgSync = sumSyncRpm / n;
    snprintf(freqField, sizeof(freqField), "%.2f", sumFreqHz / n);
    snprintf(currentField, sizeof(currentField), "%.3f", sumCurrentA / n);
    if (haveManualRpm && avgSync > 0.0f) {
      snprintf(slipField, sizeof(slipField), "%.4f", (avgSync - manualActualRpm) / avgSync);
    }
  }
  if (haveManualRpm) {
    snprintf(rpmField, sizeof(rpmField), "%.1f", manualActualRpm);
  }
  snprintf(msg, sizeof(msg),
           "# TEST %u END load_kg=%.3f rows=%lu steady_rows=%lu avg_freq_hz=%s avg_current_a=%s avg_rpm=%s avg_slip=%s\r\n",
           testId, knownLoadKg, (unsigned long)testRows, (unsigned long)testSteadyRows,
           freqField, currentField, rpmField, slipField);
  UartPrint(msg);
  if (testSteadyRows == 0) {
    UartPrint("# WARNING: no steady rows - the motor never held a constant speed during this test\r\n");
  }
  if (!haveManualRpm) {
    UartPrint("# WARNING: no RPM entered - slip not calculated (type RPM <value> before STOP)\r\n");
  }
  haveManualRpm = false; // don't carry this test's RPM into idle rows or the next test
  UartPrint("# type TEST to start the next test\r\n");
}

static void LogDataPoint(void) {
  uint16_t raw;
  float freqHz;
  const char *freqSource;
  if (ModbusReadHoldingRegister(REG_OUTPUT_FREQUENCY, &raw)) {
    freqHz = raw / 100.0f;
    freqSource = "modbus";
  } else if (haveManualFreq) {
    freqHz = manualFreqHz;
    freqSource = "manual";
  } else {
    UartPrint("# no frequency available (modbus read failed and no FREQ <hz> entered)\r\n");
    return;
  }

  liveFreqHz = freqHz;
  if (ModbusReadHoldingRegister(REG_FREQ_COMMAND, &raw)) {
    livePotSetpointHz = raw / 100.0f;
  }

  if (freqHz > SAFE_MAX_FREQ_HZ + 0.5f) {
    UartPrint("# WARNING: output frequency above SAFE_MAX_FREQ_HZ - release FWD and check P01.10\r\n");
  }

  float syncRpm = 120.0f * freqHz / MOTOR_POLE_COUNT;

  bool haveRpm = haveManualRpm;
  float actualRpm = manualActualRpm;
  float slip = 0.0f;
  if (haveRpm && syncRpm > 0.0f) {
    slip = (syncRpm - actualRpm) / syncRpm;
  }

  float currentA = 0.0f;
  const char *currentSource;
  bool haveCurrent;
  if (ModbusReadHoldingRegister(REG_OUTPUT_CURRENT, &raw)) {
    currentA = raw / 100.0f; // confirmed against GSoft2's own monitor: raw 69 = 0.69 A
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
  liveCurrentValid = haveCurrent;
  if (haveCurrent) {
    liveCurrentA = currentA;
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

  float freqChange = freqHz - prevFreqHz;
  if (freqChange < 0.0f) {
    freqChange = -freqChange;
  }
  bool steady = freqHz > STEADY_MIN_FREQ_HZ && freqChange <= STEADY_FREQ_TOL_HZ;
  prevFreqHz = freqHz;

  if (testActive) {
    testRows++;
    if (steady && haveCurrent) {
      float plateauHz = testSteadyRows > 0 ? sumFreqHz / testSteadyRows : 0.0f;
      if (testSteadyRows > 0 && freqHz > plateauHz + PLATEAU_TOL_HZ) {
        // reached a higher steady speed - restart the averages there
        testSteadyRows = 0;
        sumFreqHz = 0.0f;
        sumCurrentA = 0.0f;
        sumSyncRpm = 0.0f;
      }
      if (testSteadyRows == 0 || freqHz >= plateauHz - PLATEAU_TOL_HZ) {
        testSteadyRows++;
        sumFreqHz += freqHz;
        sumCurrentA += currentA;
        sumSyncRpm += syncRpm;
      }
    }
  }

  char line[160];
  snprintf(line, sizeof(line), "%lu,%.2f,%s,%.1f,%s,%s,%s,%s,%.3f,%u,%d\r\n",
           (unsigned long)HAL_GetTick(), freqHz, freqSource, syncRpm, rpmField,
           slipField, currentField, currentSource, knownLoadKg,
           (unsigned)(testActive ? testId : 0), steady ? 1 : 0);
  UartPrint(line);
}

// Read-only check that the drive takes its speed from the pot, its run command from
// the FWD/REV pushbuttons, and that the speed cap/ramps are in place. This firmware
// never writes to the drive - it only reports, so any setting flagged here must be
// changed in GSoft2.
static uint8_t vfdReadsTried;
static uint8_t vfdReadsOk;

static bool ReadVfdRegister(uint16_t regAddr, uint16_t *value) {
  vfdReadsTried++;
  if (!ModbusReadHoldingRegister(regAddr, value)) {
    return false;
  }
  vfdReadsOk++;
  return true;
}

static bool CheckVfdParam(const char *name, uint16_t regAddr, uint16_t expected, const char *meaning) {
  uint16_t value;
  char msg[128];
  if (!ReadVfdRegister(regAddr, &value)) {
    snprintf(msg, sizeof(msg), "# VFDCHECK %s: could not read (check RS-485 link)\r\n", name);
    UartPrint(msg);
    return false;
  }
  if (value == expected) {
    snprintf(msg, sizeof(msg), "# VFDCHECK %s = %u OK (%s)\r\n", name, value, meaning);
    UartPrint(msg);
    return true;
  }
  snprintf(msg, sizeof(msg), "# VFDCHECK %s = %u WRONG - set it to %u in GSoft2 (%s)\r\n",
           name, value, expected, meaning);
  UartPrint(msg);
  return false;
}

// Accel/decel times assume the drive's default 0.01 s time unit (P01.45 = 0). If the
// printed seconds are 10x off from GSoft2, the unit is 0.1 s - compare the raw value.
static bool PrintVfdTime(const char *name, uint16_t regAddr) {
  uint16_t raw;
  char msg[96];
  if (!ReadVfdRegister(regAddr, &raw)) {
    snprintf(msg, sizeof(msg), "# VFDCHECK %s: could not read (check RS-485 link)\r\n", name);
    UartPrint(msg);
    return false;
  }
  snprintf(msg, sizeof(msg), "# VFDCHECK %s = %.2f s (raw %u)\r\n", name, raw / 100.0f, raw);
  UartPrint(msg);
  return true;
}

static void CheckVfdPotControl(void) {
  UartPrint("# VFDCHECK: verifying drive is set up for potentiometer speed control...\r\n");
  vfdReadsTried = 0;
  vfdReadsOk = 0;
  bool ok = true;
  ok &= CheckVfdParam("P00.20 freq source", REG_P00_20_FREQ_SOURCE, EXPECTED_FREQ_SOURCE,
                      "speed from pot on AI1");
  ok &= CheckVfdParam("P00.21 run source", REG_P00_21_RUN_SOURCE, EXPECTED_RUN_SOURCE,
                      "start/stop from FWD/REV buttons");
  ok &= CheckVfdParam("P03.00 AI1 function", REG_P03_00_AI1_FUNCTION, EXPECTED_AI1_FUNCTION,
                      "AI1 = frequency command");
  ok &= CheckVfdParam("P00.22 stop method", REG_P00_22_STOP_METHOD, EXPECTED_STOP_METHOD,
                      "ramp to stop");

  uint16_t raw;
  char msg[112];
  if (ReadVfdRegister(REG_P01_10_FREQ_UPPER_LIMIT, &raw)) {
    float limitHz = raw / 100.0f;
    bool limitOk = limitHz > 0.0f && limitHz <= SAFE_MAX_FREQ_HZ;
    snprintf(msg, sizeof(msg), "# VFDCHECK P01.10 freq upper limit = %.2f Hz %s (safe max %.2f Hz)\r\n",
             limitHz, limitOk ? "OK" : "TOO HIGH", SAFE_MAX_FREQ_HZ);
    UartPrint(msg);
    ok &= limitOk;
  } else {
    UartPrint("# VFDCHECK P01.10 freq upper limit: could not read (check RS-485 link)\r\n");
    ok = false;
  }
  ok &= PrintVfdTime("P01.12 accel time", REG_P01_12_ACCEL_TIME);
  ok &= PrintVfdTime("P01.13 decel time", REG_P01_13_DECEL_TIME);

  if (ReadVfdRegister(REG_FREQ_COMMAND, &raw)) {
    snprintf(msg, sizeof(msg), "# VFDCHECK frequency command (pot setpoint) = %.2f Hz\r\n", raw / 100.0f);
    UartPrint(msg);
  }
  snprintf(msg, sizeof(msg), "# VFDCHECK modbus link: %u/%u reads OK\r\n", vfdReadsOk, vfdReadsTried);
  UartPrint(msg);
  UartPrint(ok ? "# VFDCHECK: PASS - turn the pot to set speed, FWD/REV to run\r\n"
               : "# VFDCHECK: FAIL - fix the settings above (see docs/vfd-parameters.md)\r\n");
}

// ---- 16x2 HD44780 LCD over a PCF8574 I2C backpack (4-bit mode) -----------------
// Standard backpack pin mapping: P0=RS, P1=RW (tied low, we never read), P2=E,
// P3=backlight, P4-P7=D4-D7. Every transfer uses a short timeout so a disconnected
// or unwired display just no-ops instead of blocking the RS-485 polling loop.
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_RS 0x01
#define LCD_I2C_TIMEOUT_MS 20

static void Lcd_I2cWrite(uint8_t data) {
  HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &data, 1, LCD_I2C_TIMEOUT_MS);
}

static void Lcd_PulseEnable(uint8_t data) {
  Lcd_I2cWrite(data | LCD_ENABLE);
  Lcd_I2cWrite(data & (uint8_t)~LCD_ENABLE);
}

static void Lcd_WriteNibble(uint8_t nibbleHigh, bool rs) {
  uint8_t data = (nibbleHigh & 0xF0) | LCD_BACKLIGHT | (rs ? LCD_RS : 0);
  Lcd_I2cWrite(data);
  Lcd_PulseEnable(data);
}

static void Lcd_WriteByte(uint8_t value, bool rs) {
  Lcd_WriteNibble(value & 0xF0, rs);
  Lcd_WriteNibble((uint8_t)(value << 4) & 0xF0, rs);
}

static void Lcd_Command(uint8_t cmd) { Lcd_WriteByte(cmd, false); }
static void Lcd_Data(uint8_t data) { Lcd_WriteByte(data, true); }

static void Lcd_Init(void) {
  HAL_Delay(50); // HD44780 power-on settle time
  // Force 8-bit mode three times per the HD44780 datasheet init sequence, then
  // drop to 4-bit - this works regardless of whatever state the display woke up in.
  Lcd_WriteNibble(0x30, false);
  HAL_Delay(5);
  Lcd_WriteNibble(0x30, false);
  HAL_Delay(1);
  Lcd_WriteNibble(0x30, false);
  HAL_Delay(1);
  Lcd_WriteNibble(0x20, false); // now 4-bit mode
  HAL_Delay(1);
  Lcd_Command(0x28); // 4-bit, 2 line, 5x8 font
  Lcd_Command(0x0C); // display on, cursor off, blink off
  Lcd_Command(0x06); // entry mode: increment, no shift
  Lcd_Command(0x01); // clear display
  HAL_Delay(2);
}

static void Lcd_SetCursor(uint8_t col, uint8_t row) {
  static const uint8_t rowOffset[2] = { 0x00, 0x40 };
  Lcd_Command((uint8_t)(0x80 | (col + rowOffset[row])));
}

// Writes exactly `width` characters, space-padded, so stale digits from a longer
// previous value never linger on screen.
static void Lcd_PrintPadded(const char *s, uint8_t width) {
  uint8_t i = 0;
  for (; s[i] != '\0' && i < width; i++) {
    Lcd_Data((uint8_t)s[i]);
  }
  for (; i < width; i++) {
    Lcd_Data(' ');
  }
}

// Predicts load_kg from current using whichever LOAD_MODEL row's freq_hz is
// closest to freqHz. UP (LIFTING) DIRECTION ONLY - see the LOAD_MODEL comment
// above and tools/model/fit_model.py.
static float PredictLoadKg(float freqHz, float currentA) {
  const LoadModelEntry *best = &LOAD_MODEL[0];
  float bestDist = fabsf(LOAD_MODEL[0].freq_hz - freqHz);
  for (uint8_t i = 1; i < LOAD_MODEL_N; i++) {
    float dist = fabsf(LOAD_MODEL[i].freq_hz - freqHz);
    if (dist < bestDist) {
      bestDist = dist;
      best = &LOAD_MODEL[i];
    }
  }
  float load = best->slope_kg_per_a * currentA + best->intercept_kg;
  return load > 0.0f ? load : 0.0f;
}

static void UpdateLoadDisplay(void) {
  char line[LCD_COLS + 1];
  float freqHz = liveFreqHz;
  bool running = freqHz > STEADY_MIN_FREQ_HZ;

  if (!running) {
    Lcd_SetCursor(0, 0);
    Lcd_PrintPadded("Load estimator", LCD_COLS);
    Lcd_SetCursor(0, 1);
    Lcd_PrintPadded("stopped", LCD_COLS);
    return;
  }

  snprintf(line, sizeof(line), "F%5.2fHz I%4.2fA", freqHz, liveCurrentA);
  Lcd_SetCursor(0, 0);
  Lcd_PrintPadded(line, LCD_COLS);

  Lcd_SetCursor(0, 1);
  if (!liveCurrentValid) {
    Lcd_PrintPadded("No current data", LCD_COLS);
  } else {
    float loadKg = PredictLoadKg(freqHz, liveCurrentA);
    snprintf(line, sizeof(line), "Load~%5.2f kg", loadKg);
    Lcd_PrintPadded(line, LCD_COLS);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  HAL_GPIO_WritePin(STATUS_LED_PORT, STATUS_LED_PIN, GPIO_PIN_SET); // solid on = stuck here
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
