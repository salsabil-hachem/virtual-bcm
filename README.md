# Virtual Body Control Module (BCM)

[![CI](https://github.com/salsabil-hachem/virtual-bcm/actions/workflows/ci.yml/badge.svg)](https://github.com/salsabil-hachem/virtual-bcm/actions/workflows/ci.yml)

Automotive **Body Control Module** firmware in **C on FreeRTOS**, communicating over **CAN**, fully tested and checked against **MISRA C:2012**.
It runs **without any hardware**: FreeRTOS runs on its Linux (POSIX) port and the CAN bus is a Linux virtual CAN interface (`vcan0`).

The BCM is the ECU that handles the "body" functions of a car: turn signals, hazard lights, door locking, automatic headlights and the anti-theft alarm.

## Architecture

~~~
 +----------- Test bench (Python) -----------+                +------------ BCM firmware (C) -------------+
 | pytest + python-can + cantools            |                | FreeRTOS tasks                            |
 |  - simulated dashboard (every 100 ms)     |  DASH_Inputs   |  CanRx    1 ms   prio 3  receive frames   |
 |  - speed, light sensor, doors, bulbs  ----+---- vcan0 ---->|  BcmLogic 10 ms  prio 4  bcm_update()     |
 |  - checks lamps, doors, alarm, faults <---+---- vcan0 -----|  CanTx    20 ms  prio 2  send status      |
 |                                           |  BCM_Lamps     |  Monitor  100 ms prio 1  timeout -> safe  |
 +-------------------------------------------+  BCM_Status    |                                           |
                                                              | Hardware-independent core (MISRA scope):  |
                                                              |  turn_signal, door_lock, auto_lights,     |
                                                              |  alarm, bcm (arbitration), bcm_comm (CAN) |
                                                              +-------------------------------------------+
~~~

The functional core (`src/`) has **no dependency on FreeRTOS or Linux**: each function takes its inputs and the current time and computes its outputs.
This makes it unit-testable with a simulated clock, and only `app/can_socket.c` would change on a real microcontroller.

## Functions

| Function | Rules |
|---|---|
| Turn signals | 1.5 Hz (333 ms on / 333 ms off), first flash immediate, **double speed on bulb failure** |
| Hazard lights | both sides in sync, **priority over the lever** |
| Door locking | key lock/unlock (edge detection), **auto-lock above 15 km/h once per trip**, unlock on ignition off |
| Automatic headlights | on below 1000 lux for 2 s, off above 3000 lux for 5 s (**hysteresis + debounce**, a bridge does not switch them on) |
| Anti-theft alarm | state machine DISARMED, ARMING (20 s), ARMED, TRIGGERED (siren + hazard lights for 30 s), only with ignition off |
| Safe mode | no dashboard message for 500 ms: **headlights forced on + fault flag** |

## CAN protocol

Defined in [`can/bcm.dbc`](can/bcm.dbc) (one signal per byte, little endian).

| ID | Message | Direction | Content |
|---|---|---|---|
| 0x100 | DASH_Inputs | Dashboard to BCM | lever, hazard, key lock, key unlock, ignition (every 100 ms) |
| 0x101 | VEH_Speed | to BCM | speed (km/h) |
| 0x102 | ENV_Light | to BCM | ambient light (lux) |
| 0x103 | DOOR_Status | to BCM | door open |
| 0x104 | LAMP_Diag | to BCM | left / right bulb failure |
| 0x200 | BCM_Lamps | from BCM | left lamp, right lamp, headlights, siren |
| 0x201 | BCM_Status | from BCM | doors locked, alarm state, faults |

## Verification

| Level | Tool | Content | Result |
|---|---|---|---|
| Unit tests | Unity (C) | 46 tests: every rule of every function, CAN decoding/encoding, simulated time | all pass |
| Integration tests | pytest + python-can + cantools | 11 driver scenarios on the running firmware over CAN (blink rhythm measured, alarm triggered by a door opening, tunnel vs bridge, safe mode...) | all pass |
| Static analysis | cppcheck + MISRA C:2012 addon | `src/` checked, 0 violations, 1 documented deviation ([`misra_suppressions.txt`](misra_suppressions.txt)) | clean |
| CI | GitHub Actions | build, unit tests and MISRA on every push; CAN integration tests when the runner kernel provides `vcan` | green |

The integration tests encode and decode frames **with the DBC file**, so they also check that the DBC and the C code agree.

**Finding from integration testing:** a "lock" key press arriving in the same CAN frame as "ignition off" was cancelled by the "unlock on ignition off" rule. Unit tests did not catch it because each event was tested separately.

## How to run (Ubuntu / WSL2)

~~~bash
sudo apt install build-essential cmake cppcheck can-utils python3-pip
pip3 install -r tests/integration/requirements.txt

cmake -B build && cmake --build build        # FreeRTOS and Unity are downloaded by CMake
cd build && ctest && cd ..                    # unit tests

./scripts/setup_vcan.sh                       # create vcan0
python3 -m pytest -v                          # integration tests
./scripts/run_misra.sh                        # MISRA check
~~~

Run the BCM by hand:

~~~bash
./build/bcm                                                        # terminal 1: the BCM
candump vcan0                                                      # terminal 2: watch the bus
while true; do cansend vcan0 100#0100000001; sleep 0.1; done       # terminal 3: left turn signal
~~~

## Repository layout

~~~
include/, src/        hardware-independent BCM core (MISRA scope)
app/                  FreeRTOS tasks, SocketCAN driver, FreeRTOSConfig.h
can/bcm.dbc           CAN database
tests/unit/           Unity tests
tests/integration/    pytest scenarios
scripts/              vcan setup, MISRA check
.github/workflows/    CI
~~~

## Possible next steps

- Port to an STM32 board (FDCAN driver instead of SocketCAN, GPIO for the lamps)
- Give the lock key priority when it arrives in the same cycle as ignition off
- Yocto image for a Linux-side CAN gateway
