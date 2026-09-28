#pragma once

// Minimal Modbus RTU master, function code 03 (Read Holding Registers), single
// register reads only - that's all this project needs (Output Frequency 0x2103,
// Output Current 0x2104 on the DURApulse GS20-series VFD). Half-duplex RS-485 via
// a DE/RE-tied transceiver module (e.g. MAX485), driven by one GPIO.
//
// Known simplification: assumes every successful response is exactly 7 bytes
// (a normal 1-register read). A Modbus exception response is shorter (5 bytes),
// so on an exception this will time out waiting for bytes that never arrive
// rather than decoding the exception code - it still correctly reports failure,
// just slower (up to the receive timeout) and without saying why.

#include <stdbool.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"

typedef struct {
  UART_HandleTypeDef *huart;
  GPIO_TypeDef *dePort;
  uint16_t dePin;
  uint8_t slaveId;
} ModbusMaster;

void Modbus_Init(ModbusMaster *mb, UART_HandleTypeDef *huart, GPIO_TypeDef *dePort,
                  uint16_t dePin, uint8_t slaveId);

// Reads one holding register at regAddr. Returns true and fills *value on success;
// returns false on timeout, CRC mismatch, or an unexpected/exception response.
bool Modbus_ReadHoldingRegister(ModbusMaster *mb, uint16_t regAddr, uint16_t *value);
