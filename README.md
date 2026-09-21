# Sensorless Load Estimation Using Motor Slip on a Low-Cost VFD-Driven Induction Motor

Capstone project (M.S. Applied Energy and Electromechanical Engineering, UNC Charlotte).
Tests whether motor slip — the gap between a VFD's commanded synchronous speed and the
motor's real shaft speed — can be used to estimate an unknown mechanical load on a
low-cost, open-loop (scalar V/Hz) VFD, without a dedicated torque sensor. A parallel
current-based estimate is built from the same data for comparison.

## Repo layout

- `firmware/` — PlatformIO project for the ESP32 data-acquisition node.
- `docs/wiring.md` — wire-by-wire connection reference (colors, terminals, end devices).
- `docs/open-items.md` — blockers that must be resolved before the firmware can be
  completed end-to-end, and what's confirmed vs. still TBD.

## Hardware summary

| Item | Model |
|---|---|
| Motor | Lucas Nülle SE2673-1K7, 0.37 kW, 4-pole, 1650 RPM @ 60 Hz, 208/380V Δ/Y |
| VFD | AutomationDirect DURApulse GS23-21P0 (GS20 series), 1 HP, 230V 3-phase in, scalar V/Hz, RS-485 Modbus RTU |
| Coupling | Gates PowerGrip SF43 rubber sleeve coupling (Lucas Nülle SE2662-2A) |
| Shaft speed reference | Lucas Nülle SERVO Machine Test System |
| Load reference | QWORK 200kg/440lb digital crane scale |
| Controller | ESP32 dev board |

Full mechanical BOM, formulas, and test workflow are in the project proposal; this repo
tracks firmware and wiring only.

## Confirmed VFD terminal labels

Pulled from the AutomationDirect DURApulse GS20-series lab reference material for this
drive family (main circuit + control terminal block are standard across the GS20 line):

| Function | Terminals |
|---|---|
| 3-phase input power | R/L1, S/L2, T/L3 |
| Motor output | U/T1, V/T2, W/T3 |
| Safe Torque Off (STO) jumper | +24V, S1, S2 (factory-installed; do not remove) |
| Speed reference pot (if used) | +10V, ACM, AI1 |
| Run command inputs (if used) | FWD, REV, DCM |

RS-485/Modbus terminal labels on the control terminal block are **not yet confirmed** —
see `docs/open-items.md`.

## Status

Mechanical rig is built. Calibration/validation testing has not started. This repo
currently provides a firmware skeleton that implements the one fully-confirmed data
path (VFD output frequency via Modbus RTU) and stubs the three still-open interfaces
(VFD output current register, shaft RPM from the Lucas Nülle SERVO system, and load
value from the crane scale) so they can be filled in without restructuring the rest of
the code.
