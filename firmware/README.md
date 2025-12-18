# Firmware for Dialog DA14531 - BLE Beacon Buzzer Control

This firmware implements a BLE beacon-based buzzer control system for the Dialog DA14531 SoC.

## Features

- BLE Beacon advertising with custom service data (UUID: 0xAA28)
- Command processing for buzzer control (Play, Stop, Enable, Disable)
- Status reporting via beacon payload
- Power sensing and status LED indication
- Low power operation

## Hardware Requirements

- Dialog DA14531 Development Kit
- Buzzer connected to GPIO P0_6
- Power sense on GPIO P0_7
- Status LED on GPIO P0_9

## Software Requirements

- Dialog SmartSnippets Studio (version 5.0.14 or later)
- Dialog SDK 6.0.14 or later
- Arm Keil MDK or GCC ARM toolchain

## Project Structure

```
firmware/
├── src/
│   ├── swt_t1_beacon.h         # Beacon header definitions
│   ├── swt_t1_beacon.c         # Beacon implementation
│   └── user_barebone_beacon.c  # Main application
└── README.md                    # This file
```

## Setup Instructions

### 1. Install Dialog SDK

1. Download Dialog SDK from the Dialog Semiconductor website
2. Extract to a convenient location (e.g., `C:\Dialog\SDK_6.0.14`)
3. Install SmartSnippets Studio

### 2. Create Project

1. Open SmartSnippets Studio
2. Create new SDK project based on `ble_app_barebone` template
3. Replace default files with the provided source files:
   - Copy `swt_t1_beacon.h` and `swt_t1_beacon.c` to project `src/` directory
   - Replace `user_barebone.c` with `user_barebone_beacon.c`

### 3. Configure Project

Add the following to your project configuration:

**Include Paths:**
- Add project `src/` directory to include paths

**Preprocessor Defines:**
- `CFG_BLE_BEACON`
- `__DA14531__`

**Linker Settings:**
- Use DA14531 linker script
- Configure RAM and ROM sizes appropriately

## Build Instructions

### Using Keil MDK

1. Open project in Keil µVision
2. Select target configuration (DA14531)
3. Build project: `Project -> Build Target` or press `F7`
4. Output binary will be in `out_<target>/` directory

### Using GCC ARM

```bash
# Set up environment
export SDK_ROOT=/path/to/dialog/sdk

# Clean previous build
make clean

# Build project
make

# Binary output: out_gcc/ble_app_barebone.hex
```

## Flash Instructions

### Using SmartSnippets Toolbox

1. Connect DA14531 board via USB
2. Open SmartSnippets Toolbox
3. Go to "Flash Programmer"
4. Select device: DA14531
5. Load binary file: `ble_app_barebone.hex`
6. Click "Program" to flash

### Using JTAG/SWD

1. Connect J-Link or compatible debugger
2. Use SmartSnippets Studio to program device
3. Or use command line:
```bash
JLinkExe -device DA14531 -if SWD -speed 4000
> loadfile ble_app_barebone.hex
> r
> g
> q
```

## Beacon Protocol

### Service UUID
- 16-bit UUID: `0xAA28`

### Payload Format
```
[Device Name: 4 bytes][Command: 1 byte][Timestamp: 4 bytes]
Total: 9 bytes
```

### Command Codes
- `0x00` - STOP: Stop buzzer
- `0x01` - PLAY: Play buzzer pattern
- `0x02` - ENABLE: Enable buzzer
- `0x03` - DISABLE: Disable buzzer
- `0xFF` - STATUS: Request status

### Example Advertising Packet

```
02 01 06                    // Flags
0E 16 28 AA                 // Service Data header (UUID 0xAA28)
42 5A 52 31                 // Device name: "BZR1"
01                          // Command: PLAY
12 34 56 78                 // Timestamp: 0x78563412
```

## GPIO Configuration

| Function     | Port | Pin | Direction | Notes              |
|--------------|------|-----|-----------|-------------------|
| Buzzer       | P0   | 6   | Output    | Active high       |
| Power Sense  | P0   | 7   | Input     | Pull-up           |
| Status LED   | P0   | 9   | Output    | Active high       |

## Testing

1. Power on the DA14531 board
2. Status LED should turn on (device ready)
3. Use a BLE scanner app to verify beacon advertising
4. Use the mobile app to send commands
5. Observe buzzer and LED behavior

## Troubleshooting

### Device Not Advertising
- Check power supply
- Verify firmware was flashed correctly
- Check advertising interval settings
- Ensure BLE antenna connection

### Buzzer Not Working
- Verify GPIO configuration
- Check buzzer connection and polarity
- Test GPIO with LED first
- Verify command reception via status LED

### Cannot Flash Device
- Check USB connection
- Verify debugger drivers installed
- Try different USB port
- Reset device before flashing

## Power Consumption

Typical current consumption:
- Active advertising: 4-6 mA
- Deep sleep: 1-3 µA
- Buzzer active: +20-50 mA

## Notes

- The firmware uses Dialog SDK BLE stack
- Advertising interval: 100ms (configurable)
- Connection not required - pure beacon mode
- Supports extended sleep for low power
- Timestamp prevents command replay attacks

## References

- [Dialog DA14531 Datasheet](https://www.dialog-semiconductor.com/products/da14531)
- [Dialog SDK Documentation](https://www.dialog-semiconductor.com/support)
- Dialog SmartSnippets Studio User Manual

## License

See project root LICENSE file.
