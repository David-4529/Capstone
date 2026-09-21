# Wiring Reference

Two independent wiring systems on this rig:

1. **VFD control station wiring** — the 9-pair control cable running pushbuttons and a
   speed pot into the DURApulse GS23-21P0's control terminal block. **Confirmed,
   already landed on the rig.**
2. **ESP32 data-acquisition wiring** — new for this project: ESP32 dev board, RS-485
   transceiver module, and (once resolved) the SERVO system's RPM signal. Termination
   style is **Dupont / pin headers** throughout (per project decision — see
   `docs/open-items.md` if that changes).

AC/mains wiring color convention (3-phase input to the VFD) is **TBD** pending user
input — see `docs/open-items.md`, item 6. Everything below that isn't AC/mains does not
depend on that choice and is documented now.

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

## 2. ESP32 Data-Acquisition Wiring — Dupont / pin headers

This is new wiring for the ESP32 logging node, independent of the control cable above.
Dupont jumper kits ship with arbitrary rainbow colors that carry no inherent meaning, so
the colors below are a **functional convention this project is adopting** — if your kit's
actual wire colors differ, sleeve or flag both ends with tape/heat-shrink labeled per the
"Signal" column so the mapping in this table stays trustworthy.

### 2.1 Power

| From | To | Color (convention) | Notes |
|---|---|---|---|
| ESP32 `3V3` pin | RS-485 module `VCC` | Red | See electrical note below — do not power the module from 5V |
| ESP32 `GND` | RS-485 module `GND` | Black | |
| ESP32 `GND` | VFD control common (DCM/ACM ground reference) | Black | Single-point ground reference; see §4 |

**⚠ Electrical note — 3.3V vs 5V:** ESP32 GPIOs are **3.3V logic and not 5V-tolerant**.
Many generic "MAX485 module" breakout boards are built around a plain MAX485 chip that
expects a 5V supply, and if powered at 5V its `RO` (receiver output) pin would drive the
ESP32's RX pin at 5V logic levels — this can damage the ESP32 over time. Before wiring:
verify the specific module either (a) is built around a 3.3V-tolerant transceiver (e.g.
MAX3485/SP3485-based), or (b) is confirmed by its datasheet to run correctly off a 3.3V
supply with 3.3V logic thresholds. If neither is true, add a bidirectional logic-level
shifter between the ESP32 UART pins and the module's `RO`/`DI` pins rather than powering
the module directly from `3V3`.

### 2.2 ESP32 ↔ RS-485 transceiver module (e.g. MAX485 breakout)

Uses ESP32 hardware UART2 so UART0 stays free for USB/Serial Monitor logging.

| ESP32 pin | Module pin | Color (convention) | Function |
|---|---|---|---|
| GPIO16 (RX2) | `RO` | Yellow | Data from VFD → ESP32 |
| GPIO17 (TX2) | `DI` | Orange | Data from ESP32 → VFD |
| GPIO4 | `DE` **and** `RE` tied together | Green | Half-duplex direction control (driven HIGH to transmit, LOW to receive) |
| `3V3` | `VCC` | Red | See electrical note above |
| `GND` | `GND` | Black | |

### 2.3 RS-485 transceiver module ↔ VFD

**TBD** — see `docs/open-items.md`, item 3. The GS20-series control terminal block's
RS-485 labels (commonly `SG-`/`SG+`, or an RJ12 jack requiring an adapter cable) are not
yet confirmed from the drive's Chapter 5 (Serial Communications) register/wiring table.
Placeholder pending that:

| Module pin | VFD terminal | Color (convention) | Notes |
|---|---|---|---|
| `A` / `Y` | TBD (likely `SG+`) | White | Twisted pair with B/Z line below |
| `B` / `Z` | TBD (likely `SG-`) | Blue | |
| `GND` | VFD signal common (if a separate terminal exists) | Black | Only bond if the drive provides an isolated signal-ground terminal — don't tie to earth ground at both ends of a long run (ground loop) |

If the run is long or noisy, terminate the RS-485 bus with a 120Ω resistor across A/B at
each end (module end and drive end) — confirm against the manual whether the GS20 has
switchable internal termination before adding an external resistor.

### 2.4 Shaft RPM input (Lucas Nülle SERVO Machine Test System)

**TBD** — see `docs/open-items.md`, item 4. Interface type not yet confirmed. Candidate
wiring, to be finalized once the interface is known:

| If the SERVO system outputs... | ESP32 side | Color (convention) | Notes |
|---|---|---|---|
| Analog tachometer voltage (e.g. 0–10V) | Voltage divider into an ADC-capable GPIO (e.g. GPIO34) | White (signal), Black (return) | ESP32 ADC pins max ~3.3V — a divider or scaling op-amp is required, do not feed 0–10V directly |
| Digital pulse / encoder output | GPIO interrupt pin (e.g. GPIO27) | White (signal), Black (return) | Check output logic level; if 5V/12V TTL, level-shift or opto-isolate before the GPIO |
| Serial (RS-232/RS-485/USB) | Second UART or a second RS-485 channel | per §2.2 convention | Would reuse the same electrical-note caution as §2.1 if RS-485 |

Whichever it turns out to be, favor opto-isolation between the SERVO system and the
ESP32 if practical — it sits electrically close to the motor/drive and isolation limits
noise coupling into the logging electronics.

### 2.5 Load reference (QWORK crane scale)

**No wiring.** Treated as a standalone display with manual data entry — see
`docs/open-items.md`, item 5. The known/applied test weight is typed over USB serial
(ESP32's native UART0/USB connection to the logging PC) before each test point; see
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
| ESP32 dev board | USB to logging PC (power + serial) | New, this project |
| RS-485 transceiver module | Dupont jumpers to ESP32 UART2 | New, this project |
| VFD Modbus RTU (frequency/current) | RS-485 transceiver ↔ VFD control terminal block | Wiring TBD (item 3), protocol confirmed |
| Lucas Nülle SERVO system (shaft RPM) | TBD | Open item 4 |
| QWORK crane scale (load) | Manual entry, no wiring | Open item 5 |

---

## 4. Grounding & EMI notes

- Keep the RS-485 twisted pair (A/B) physically separated from VFD output power leads
  (motor cable) — VFD output switching is a strong EMI source. Cross power cables at 90°
  only if a crossing is unavoidable; don't run signal and motor-power cable in parallel
  runs.
- Use a single-point ground reference between ESP32 `GND`, the RS-485 module `GND`, and
  the VFD's control common — avoid creating a second ground path elsewhere that could
  form a ground loop.
- The control station cable (§1) and the RS-485/data wiring (§2) should be run as
  physically separate cables, not bundled together, since one carries switched pushbutton
  contacts, and the RS-485 differential pair is comparatively low-signal.
