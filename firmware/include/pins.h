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

// --- Confirmed Modbus registers (GS20/GS20X manual, Ch.5, Status Monitor block) ---
// Output Frequency: format XXX.XX Hz -> divide raw register by 100.
constexpr uint16_t REG_OUTPUT_FREQUENCY = 0x2103;
// Output Current: divide raw register by 100 (same scale as frequency).
// Confirmed on real hardware against GSoft2's own monitor: raw 69 = 0.69 A.
// (The manual-text-derived guess of /10 was wrong by 10x - trust this value,
// not documentation summaries, for any future register scale questions.)
constexpr uint16_t REG_OUTPUT_CURRENT = 0x2104;

// RS-485 serial framing: the drive's P09.04 has no 8N1 option for RTU mode - only
// 8N2, 8E1, or 8O1. If using no parity, set P09.04 to 8N2 on the drive AND
// configure the microcontroller UART for 8 data bits, no parity, 2 stop bits
// (not 8N1) to match.

// --- Motor nameplate constants ---
constexpr int MOTOR_POLE_COUNT = 4; // Lucas Nulle SE2673-1K7
