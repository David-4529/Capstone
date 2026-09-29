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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VFD_SLAVE_ID 1          // must match the VFD's P09.00 - check in GSoft2
#define REG_OUTPUT_FREQUENCY 0x2103
#define REG_OUTPUT_CURRENT 0x2104
#define MOTOR_POLE_COUNT 4
#define LOG_INTERVAL_MS 2000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

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
static char lineBuf[64];
static volatile uint8_t lineLen = 0;
static volatile bool lineReady = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static uint16_t ModbusCrc16(const uint8_t *buf, uint16_t len);
static bool ModbusReadHoldingRegister(uint16_t regAddr, uint16_t *value);
static void UartPrint(const char *s);
static void HandleLine(const char *line);
static void LogDataPoint(void);
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
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(RS485_DE_RE_GPIO_Port, RS485_DE_RE_Pin, GPIO_PIN_RESET); // start in receive mode

  HAL_UART_Receive_IT(&huart2, &rxByte, 1);

  UartPrint("# millis,freq_hz,freq_source,sync_rpm,actual_rpm,slip,current_a,current_source,known_load_kg\r\n");
  UartPrint("# send 'LOAD <kg>' 'RPM <value>' 'FREQ <hz>' 'CURRENT <amps>' over serial\r\n");
  UartPrint("# send 'DEBUG 0' to silence raw Modbus TX/RX hex dumps, 'DEBUG 1' to re-enable\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

// Set to false once the RS-485 link is confirmed working, to quiet the log.
static bool modbusDebug = true;

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
    return false;
  }

  // slaveId, func, byteCount, data_hi, data_lo, crc_lo, crc_hi
  uint8_t response[7];
  if (HAL_UART_Receive(&huart1, response, sizeof(response), 200) != HAL_OK) {
    if (modbusDebug) {
      UartPrint("#   FAIL: no response within 200ms (check A/B wiring, baud, slave ID)\r\n");
    }
    return false; // timeout - check wiring (A/B swapped?), baud, or slave ID
  }

  if (modbusDebug) {
    UartPrintHexBytes("RX", response, sizeof(response));
  }

  if (response[0] != VFD_SLAVE_ID || response[1] != 0x03 || response[2] != 2) {
    if (modbusDebug) {
      UartPrint("#   FAIL: unexpected slave ID / function code / byte count (exception response?)\r\n");
    }
    return false; // wrong slave ID, exception response, or unexpected byte count
  }

  uint16_t receivedCrc = ModbusCrc16(response, 5);
  uint16_t frameCrc = (uint16_t)response[5] | ((uint16_t)response[6] << 8);
  if (receivedCrc != frameCrc) {
    if (modbusDebug) {
      UartPrint("#   FAIL: CRC mismatch (check 8N2 vs 8N1 framing)\r\n");
    }
    return false; // CRC mismatch - usually a framing mismatch (check 8N2 vs 8N1)
  }

  *value = ((uint16_t)response[3] << 8) | response[4];
  if (modbusDebug) {
    char ok[48];
    snprintf(ok, sizeof(ok), "#   OK: raw value = %u\r\n", *value);
    UartPrint(ok);
  }
  return true;
}

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
  } else if (sscanf(line, "DEBUG %f", &value) == 1) {
    modbusDebug = (value != 0.0f);
    snprintf(msg, sizeof(msg), "# modbus debug logging %s\r\n", modbusDebug ? "ON" : "OFF");
    UartPrint(msg);
  }
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
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
