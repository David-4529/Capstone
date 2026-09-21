#pragma once

// Pin assignments and Modbus register map.
// Must stay in sync with docs/wiring.md - update both together.

// --- RS-485 transceiver module (ESP32 UART2), docs/wiring.md section 2.2 ---
constexpr int PIN_RS485_RX = 16; // ESP32 GPIO16 (RX2) <- module RO   (yellow)
constexpr int PIN_RS485_TX = 17; // ESP32 GPIO17 (TX2) -> module DI   (orange)
constexpr int PIN_RS485_DE_RE = 4; // ESP32 GPIO4 -> module DE+RE tied together (green)

// --- Shaft RPM input, docs/wiring.md section 2.4 ---
// TODO(open-item-4): interface not yet confirmed (analog / pulse / serial).
// Placeholder assumes an analog-capable ADC pin; revisit once the Lucas Nulle
// SERVO Machine Test System's actual output type is known.
constexpr int PIN_RPM_INPUT = 34;

// --- Modbus RTU config ---
constexpr uint8_t VFD_MODBUS_SLAVE_ID = 1; // set to match VFD's configured station address (P09.xx)
constexpr uint32_t MODBUS_BAUD = 9600; // must match VFD's configured baud rate (P09.xx)

// --- Confirmed Modbus register (GS20/GS20X manual, Ch.5, Status Monitor block) ---
constexpr uint16_t REG_OUTPUT_FREQUENCY = 0x2103;

// TODO(open-item-2): confirm from GS20/GS20X manual Ch.5 Status Monitor register table.
// Placeholder value only - do not trust until verified against the manual.
constexpr uint16_t REG_OUTPUT_CURRENT = 0x0000;

// --- Motor nameplate constants ---
constexpr int MOTOR_POLE_COUNT = 4; // Lucas Nulle SE2673-1K7
