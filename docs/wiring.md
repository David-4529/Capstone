# Wiring Reference

Two independent wiring systems on this rig:

1. **VFD control station wiring** — the 9-pair control cable running pushbuttons and a
   speed pot into the DURApulse GS23-21P0's control terminal block. **Confirmed,
   already landed on the rig.**
2. **STM32 data-acquisition wiring** — new for this project: STM32 Nucleo-F401RE, RS-485
   transceiver module, and (once resolved) the SERVO system's RPM signal. Termination
   style is **Dupont / pin headers** throughout (per project decision — see
   `docs/open-items.md` if that changes).

AC/mains wiring color convention (3-phase input to the VFD) is **CONFIRMED** — see
section 1b below.

---

## 1. VFD Control Station Wiring — CONFIRMED

9-pair control cable, home-run from the control station to the GS23-21P0's control
terminal block. Each pair is a colored conductor + its own black or red return, so **the
black/red conductors are not a single shared return bus** — each pair carries its own
return wire back to the drive, and only the ones actually landed are physically tied to
a common point at the terminal block (see notes).

| Pair | Colored wire → Terminal | Black/Red wire → Terminal | Status |
|---|---|---|---|
| Orange/Black | Orange → **FWD** | Black → **DCM** | **Landed** — Forward pushbutton |
| Yellow/Black | Yellow → **REV** | Black → **DCM** | **Landed** — Reverse pushbutton |
| White/Black | White → **DI3** (preset-speed select) | Black → **DCM** | Pulled & terminated at drive; not landed on a component |
| Blue/Black | Blue → **DI4** (spare / future Stop button) | Black → **DCM** | Pulled & terminated at drive; not landed on a component |
| Green/Black | Green → **DI5** (spare) | Black → **DCM** | Pulled & terminated at drive; not landed on a component |
| White/Red | White → **AI1** (pot wiper) | Red → **ACM** | **Landed** — Potentiometer wiper |
| Red/Green | Green → **+10V** (pot supply) | Red → **ACM** | **Landed** — Potentiometer supply |
| Black/Red | spare | spare | Not landed |
| Black/Brown | spare | spare | Not landed |

**Currently landed and confirmed working:**

| Component | Wire → Terminal |
|---|---|
| Forward pushbutton | Orange → FWD, Black → DCM |
| Reverse pushbutton | Yellow → REV, Black → DCM |
| Potentiometer | Green → +10V, White → AI1, Red → ACM |

Note: the two Red conductors (from the White/Red and Red/Green pairs) both land on
**ACM** — that's the same common point, not two different signals.

**Reserved for later, pulled and terminated at the drive end only:**
- DI3 (White/Black pair) — planned preset-speed select input.
- DI4 (Blue/Black pair) — spare, earmarked for a future dedicated Stop pushbutton.
- DI5 (Green/Black pair) — spare, no assignment yet.
- Black/Red and Black/Brown pairs — fully spare, no terminal assignment yet.

**End devices on this circuit:** Forward pushbutton, Reverse pushbutton, speed-reference
potentiometer, all at the operator control station; drive end terminates at the GS23-21P0
control terminal block.

### Reference: GS20-series terminal labels (confirmed from drive documentation)

| Function | Terminals |
|---|---|
| 3-phase input power | R/L1, S/L2, T/L3 |
| Motor output | U/T1, V/T2, W/T3 |
| Safe Torque Off (STO) jumper | +24V, S1, S2 — factory-installed, **do not remove** |
| Speed reference pot | +10V, ACM, AI1 |
| Run command inputs | FWD, REV, DCM |

⚠ The STO jumper across +24V/S1/S2 must stay intact unless an external safety circuit is
deliberately being wired in its place. Don't disturb it while landing the DI3/DI4/DI5
spares above — they're on separate terminals.

---

## 1b. AC/Mains Power Wiring — CONFIRMED

Incoming 3-phase power and the motor leads, landed and confirmed working.

| From | Wire color | To (VFD terminal) |
|---|---|---|
| Incoming 3-phase, phase R | Black | R/L1 |
| Incoming 3-phase, phase S | Grey | S/L2 |
| Incoming 3-phase, phase T | Brown | T/L3 |

| VFD output | Wire color | To (motor terminal) |
|---|---|---|
| U/T1 | Black | U2 |
| V/T2 | Grey | V2 |
| W/T3 | Brown | W2 |

Each phase keeps the same wire color from the VFD's input side through to the motor's
output side (black/grey/brown throughout). The motor is landed on its **U2/V2/W2**
terminals specifically, not U1/V1/W1 — this motor's terminal board exposes both halves of
each winding (U1/U2, V1/V2, W1/W2) so it can be reconfigured between its 208V delta and
360V wye ratings (see the nameplate table, Table I); U2/V2/W2 is the set wired for this
project's 208V delta configuration, matching P01.02 (MAX Out-VOLT 1 = 208.0V, confirmed
in `docs/vfd-parameters.md`).

Equipment ground/PE is bonded from both the incoming supply and the motor frame
(unchanged from the general safety note above).

---

## 2. Data-Acquisition Wiring — STM32 Nucleo-F401RE

Controller is the **STM32 Nucleo-F401RE** (STM32CubeIDE, bare HAL/LL — no Arduino
framework; the earlier ESP32 plan was superseded, see `docs/open-items.md` item 7).
This is new wiring for the logging node, independent of the control cable above.

### 2.1 Power

| From | To | Notes |
|---|---|---|
| STM32 `3.3V` pin | MAX485 `VCC` | Do not power the module from 5V — see electrical note below |
| STM32 `GND` | MAX485 `GND` | |
| STM32 `GND` | VFD control common (DCM/ACM ground reference) | Single-point ground reference; see §4 |

**⚠ Electrical note — 3.3V vs 5V:** the Nucleo's GPIOs are **3.3V logic and not
5V-tolerant**. Many generic "MAX485 module" breakouts are built around a plain MAX485
chip that expects a 5V supply, and if powered at 5V its `RO` (receiver output) pin would
drive the STM32's RX pin at 5V logic levels — this can damage the MCU over time. Confirm
the module is 3.3V-tolerant (or add a logic-level shifter) before wiring; this project's
module has been run successfully at 3.3V.

### 2.2 STM32 ↔ MAX485 transceiver module — CONFIRMED, landed

| STM32 pin | Board label | Module pin | Function |
|---|---|---|---|
| PA10 (USART1_RX) | D2 | `RO` | Data from VFD → STM32 |
| PA9 (USART1_TX) | D8 | `DI` | Data from STM32 → VFD |
| PA8 (GPIO output) | D7 | `DE` **and** `RE` tied together | Half-duplex direction control (driven HIGH to transmit, LOW to receive) |
| 3.3V | — | `VCC` | See electrical note above |
| GND | — | `GND` | |

### 2.3 MAX485 transceiver module ↔ VFD — CONFIRMED, landed

This drive exposes RS-485 two ways that share the same signals — a front/side **RJ45
jack**, and terminal-block `SG+`/`SG-` screw terminals. This project uses the RJ45 jack.

| VFD cable | Signal | MAX485 pin | Notes |
|---|---|---|---|
| Blue | RS-485 B | `B` | |
| White/Blue | RS-485 A | `A` | Twisted pair with the Blue (B) line |
| White/Green | SGND (common) | `GND` | Single-point ground reference — don't also tie to earth ground at both ends (ground loop) |

If using the RJ45 jack, a cheap RJ45-to-screw-terminal breakout is the simplest way to
land pins 4/5/3 onto the module without crimping a custom connector. Standard Ethernet
patch cable is fine per the manual (not a crossover cable).

If the run is long or noisy, terminate the RS-485 bus with a 120Ω resistor across A/B at
each end (module end and drive end) — check whether the GS20 has switchable internal
termination before adding an external resistor.

**Serial framing:** the drive's P09.04 has no 8N1 option for RTU mode — only 8N2, 8E1,
or 8O1. If running without parity, set the drive to **8N2** and configure the MCU's UART
to match (8 data bits, no parity, 2 stop bits), not 8N1.

### 2.4 Shaft RPM input (Lucas Nülle SERVO Machine Test System)

**TBD** — see `docs/open-items.md`, item 4. Interface type not yet confirmed. Candidate
wiring, to be finalized once the interface is known:

| If the SERVO system outputs... | STM32 side | Color (convention) | Notes |
|---|---|---|---|
| Analog tachometer voltage (e.g. 0–10V) | Voltage divider into an ADC-capable GPIO | White (signal), Black (return) | STM32 ADC pins max ~3.3V — a divider or scaling op-amp is required, do not feed 0–10V directly |
| Digital pulse / encoder output | GPIO interrupt pin (e.g. GPIO27) | White (signal), Black (return) | Check output logic level; if 5V/12V TTL, level-shift or opto-isolate before the GPIO |
| Serial (RS-232/RS-485/USB) | Second UART or a second RS-485 channel | per §2.2 convention | Would reuse the same electrical-note caution as §2.1 if RS-485 |

Whichever it turns out to be, favor opto-isolation between the SERVO system and the
the STM32 if practical — it sits electrically close to the motor/drive and isolation limits
noise coupling into the logging electronics.

### 2.5 Load reference (QWORK crane scale)

**No wiring.** Treated as a standalone display with manual data entry — see
`docs/open-items.md`, item 5. The known/applied test weight is typed over USB serial
(the STM32's ST-LINK virtual COM port connection to the logging PC) before each test point; see
`handleSerialCommands()` in `firmware/src/main.cpp`.

---

## 3. End-device summary

| End device | Connects via | Status |
|---|---|---|
| Forward pushbutton | VFD control cable (Orange/Black pair) | Landed, confirmed |
| Reverse pushbutton | VFD control cable (Yellow/Black pair) | Landed, confirmed |
| Speed potentiometer | VFD control cable (White/Red, Red/Green pairs) | Landed, confirmed |
| Preset-speed select (future) | VFD control cable (White/Black pair → DI3) | Pulled, not landed |
| Stop pushbutton (future) | VFD control cable (Blue/Black pair → DI4) | Pulled, not landed |
| Spare digital input | VFD control cable (Green/Black pair → DI5) | Pulled, not landed |
| STM32 Nucleo-F401RE | USB (ST-LINK VCP) to logging PC (power + serial) | New, this project |
| RS-485 transceiver module (MAX485) | STM32 USART1 (PA8/PA9/PA10) | Landed, confirmed |
| VFD Modbus RTU (frequency/current) | RS-485 transceiver ↔ VFD RJ45 jack | Landed, confirmed |
| Lucas Nülle SERVO system (shaft RPM) | TBD | Open item 4 |
| QWORK crane scale (load) | Manual entry, no wiring | Open item 5 |

---

## 4. Grounding & EMI notes

- Keep the RS-485 twisted pair (A/B) physically separated from VFD output power leads
  (motor cable) — VFD output switching is a strong EMI source. Cross power cables at 90°
  only if a crossing is unavoidable; don't run signal and motor-power cable in parallel
  runs.
- Use a single-point ground reference between the STM32's `GND`, the RS-485 module `GND`, and
  the VFD's control common — avoid creating a second ground path elsewhere that could
  form a ground loop.
- The control station cable (§1) and the RS-485/data wiring (§2) should be run as
  physically separate cables, not bundled together, since one carries switched pushbutton
  contacts, and the RS-485 differential pair is comparatively low-signal.
