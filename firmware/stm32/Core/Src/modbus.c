#include "modbus.h"

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

void Modbus_Init(ModbusMaster *mb, UART_HandleTypeDef *huart, GPIO_TypeDef *dePort,
                  uint16_t dePin, uint8_t slaveId) {
  mb->huart = huart;
  mb->dePort = dePort;
  mb->dePin = dePin;
  mb->slaveId = slaveId;
  HAL_GPIO_WritePin(dePort, dePin, GPIO_PIN_RESET); // start in receive mode
}

bool Modbus_ReadHoldingRegister(ModbusMaster *mb, uint16_t regAddr, uint16_t *value) {
  uint8_t request[8];
  request[0] = mb->slaveId;
  request[1] = 0x03; // read holding registers
  request[2] = (regAddr >> 8) & 0xFF;
  request[3] = regAddr & 0xFF;
  request[4] = 0x00; // quantity high byte
  request[5] = 0x01; // quantity = 1 register
  uint16_t crc = ModbusCrc16(request, 6);
  request[6] = crc & 0xFF;        // CRC low byte first (Modbus RTU convention)
  request[7] = (crc >> 8) & 0xFF; // CRC high byte

  HAL_GPIO_WritePin(mb->dePort, mb->dePin, GPIO_PIN_SET);
  HAL_Delay(1); // let the transceiver settle before driving the line
  HAL_StatusTypeDef txStatus = HAL_UART_Transmit(mb->huart, request, sizeof(request), 100);
  HAL_Delay(1); // safety margin before releasing the bus back to receive mode
  HAL_GPIO_WritePin(mb->dePort, mb->dePin, GPIO_PIN_RESET);
  if (txStatus != HAL_OK) {
    return false;
  }

  // slaveId, func, byteCount, data_hi, data_lo, crc_lo, crc_hi
  uint8_t response[7];
  if (HAL_UART_Receive(mb->huart, response, sizeof(response), 200) != HAL_OK) {
    return false; // timeout - check wiring (A/B swapped?), baud, or slave ID
  }

  if (response[0] != mb->slaveId || response[1] != 0x03 || response[2] != 2) {
    return false; // wrong slave ID, exception response, or unexpected byte count
  }

  uint16_t receivedCrc = ModbusCrc16(response, 5);
  uint16_t frameCrc = (uint16_t)response[5] | ((uint16_t)response[6] << 8);
  if (receivedCrc != frameCrc) {
    return false; // CRC mismatch - usually a framing mismatch (check 8N2 vs 8N1)
  }

  *value = ((uint16_t)response[3] << 8) | response[4];
  return true;
}
