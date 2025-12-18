# BLE Beacon Buzzer Control

A complete IoT solution for controlling buzzer devices via BLE (Bluetooth Low Energy) beacon technology. This project includes firmware for the Dialog DA14531 SoC and a cross-platform React Native mobile application.

## 🎯 Features

### Firmware (Dialog DA14531)
- ✅ BLE beacon advertising with custom service data (UUID: 0xAA28)
- ✅ Beacon-based command processing (connectionless operation)
- ✅ Buzzer control with multiple commands (Play, Stop, Enable, Disable)
- ✅ Status reporting via beacon payload
- ✅ Power sensing and status LED indication
- ✅ Low power operation with extended sleep support
- ✅ Timestamp-based replay attack prevention

### Mobile App (React Native)
- ✅ BLE beacon scanning and device discovery
- ✅ Real-time RSSI (signal strength) monitoring
- ✅ Signal strength visualization with charts
- ✅ Command interface for buzzer control
- ✅ Command history with filtering
- ✅ Favorite devices management
- ✅ Cross-platform (iOS and Android)
- ✅ Persistent storage for history and settings

## 📋 Table of Contents

- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Protocol Specification](#protocol-specification)
- [Development](#development)
- [Testing](#testing)
- [Deployment](#deployment)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## 🚀 Quick Start

### Prerequisites
- Dialog DA14531 development board
- Smartphone (iOS or Android)
- Development computer with necessary tools installed

### Firmware
```bash
# 1. Setup Dialog SDK and toolchain (see docs/FIRMWARE_SETUP.md)
# 2. Build firmware
cd firmware
make

# 3. Flash to DA14531
# Use SmartSnippets Toolbox or J-Link to flash the .hex file
```

### Mobile App
```bash
# 1. Install dependencies
cd mobile-app
npm install

# 2. Run on device
npm run android  # For Android
npm run ios      # For iOS (macOS only)
```

## 📁 Project Structure

```
ble-beacon-buzzer-control/
├── firmware/                    # DA14531 firmware
│   ├── src/
│   │   ├── swt_t1_beacon.h     # Beacon header definitions
│   │   ├── swt_t1_beacon.c     # Beacon implementation
│   │   └── user_barebone_beacon.c  # Main application
│   └── README.md               # Firmware documentation
│
├── mobile-app/                  # React Native mobile app
│   ├── src/
│   │   ├── components/         # Reusable UI components
│   │   │   ├── DeviceCard.tsx
│   │   │   ├── CommandButton.tsx
│   │   │   └── StatusIndicator.tsx
│   │   ├── context/            # React context for state
│   │   │   └── AppContext.tsx
│   │   ├── screens/            # Screen components
│   │   │   ├── ScanScreen.tsx
│   │   │   ├── DeviceScreen.tsx
│   │   │   └── HistoryScreen.tsx
│   │   ├── services/           # Business logic services
│   │   │   ├── bleService.ts
│   │   │   ├── commandService.ts
│   │   │   └── storageService.ts
│   │   └── types/              # TypeScript type definitions
│   │       └── index.ts
│   ├── App.tsx                 # App entry point
│   ├── package.json            # Dependencies
│   ├── app.json                # App configuration
│   └── README.md               # App documentation
│
├── docs/                        # Documentation
│   ├── PROTOCOL.md             # Beacon protocol specification
│   ├── FIRMWARE_SETUP.md       # Firmware setup guide
│   ├── APP_SETUP.md            # Mobile app setup guide
│   └── BUILD_GUIDE.md          # Complete build instructions
│
├── .gitignore                  # Git ignore rules
├── LICENSE                     # Project license
└── README.md                   # This file
```

## 🔧 Hardware Requirements

### Firmware
- **Dialog DA14531** development board
- **Buzzer** (active or passive, connected to GPIO P0_6)
- **Status LED** (optional, GPIO P0_9)
- **Power sense** (optional, GPIO P0_7)
- **USB cable** for programming/debugging
- **J-Link debugger** (optional, for advanced debugging)

### Mobile App Testing
- **iOS device** (iPhone 6s or later, iOS 12.0+) or **iOS Simulator**
- **Android device** (Android 5.0+, API 21+) or **Android Emulator**
- Bluetooth Low Energy support required

## 💻 Software Requirements

### Firmware Development
- Dialog SmartSnippets Studio v5.0.14+
- Dialog SDK 6.0.14+
- ARM Keil MDK or GCC ARM toolchain
- J-Link software (for flashing)

### Mobile App Development
- Node.js v16+ and npm
- React Native development environment
- **For iOS**: Xcode 13+, CocoaPods (macOS only)
- **For Android**: Android Studio, JDK 11+, Android SDK

See [docs/FIRMWARE_SETUP.md](docs/FIRMWARE_SETUP.md) and [docs/APP_SETUP.md](docs/APP_SETUP.md) for detailed installation instructions.

## 📥 Installation

### 1. Clone Repository
```bash
git clone https://github.com/HumYhaiJung/ble-beacon-buzzer-control.git
cd ble-beacon-buzzer-control
```

### 2. Setup Firmware
```bash
cd firmware
# Follow setup instructions in docs/FIRMWARE_SETUP.md
make
```

### 3. Setup Mobile App
```bash
cd mobile-app
npm install

# For iOS (macOS only)
cd ios && pod install && cd ..
```

For complete setup instructions, see:
- [Firmware Setup Guide](docs/FIRMWARE_SETUP.md)
- [App Setup Guide](docs/APP_SETUP.md)
- [Build Guide](docs/BUILD_GUIDE.md)

## 🎮 Usage

### Firmware Operation

1. **Flash Firmware**
   - Build firmware using Keil or GCC
   - Flash to DA14531 using SmartSnippets Toolbox
   - Power on the device

2. **Device Initialization**
   - Status LED turns on when ready
   - Device starts advertising beacon with UUID 0xAA28
   - Buzzer is initially in IDLE state

3. **Command Reception**
   - Device receives commands via beacon service data
   - Status LED blinks on command reception
   - Buzzer responds according to command

### Mobile App Operation

1. **Scanning for Devices**
   - Open app and navigate to "Scan" tab
   - Tap "Start Scanning"
   - Devices with beacon service appear in list
   - RSSI signal strength shown in real-time

2. **Controlling Device**
   - Tap on a device to open details
   - View signal strength graph
   - Send commands:
     - **Play**: Activate buzzer
     - **Stop**: Stop buzzer
     - **Enable**: Enable buzzer functionality
     - **Disable**: Disable buzzer functionality
     - **Status**: Request device status

3. **Viewing History**
   - Navigate to "History" tab
   - View all sent commands
   - Filter by device
   - Clear history if needed

## 📡 Protocol Specification

### Beacon Service UUID
```
16-bit UUID: 0xAA28
```

### Payload Format
```
[Device Name: 4 bytes][Command: 1 byte][Timestamp: 4 bytes]
Total: 9 bytes
```

### Command Codes
| Code | Name    | Description                    |
|------|---------|--------------------------------|
| 0x00 | STOP    | Stop buzzer                    |
| 0x01 | PLAY    | Play buzzer pattern            |
| 0x02 | ENABLE  | Enable buzzer                  |
| 0x03 | DISABLE | Disable buzzer                 |
| 0xFF | STATUS  | Request status                 |

### Example Beacon Packet
```
02 01 06                    // Flags
0E 16 28 AA                 // Service Data (UUID 0xAA28)
42 5A 52 31                 // Device name: "BZR1"
01                          // Command: PLAY
12 34 56 78                 // Timestamp: 0x78563412
```

For complete protocol details, see [docs/PROTOCOL.md](docs/PROTOCOL.md).

## 🛠️ Development

### Building Firmware

**Using GCC:**
```bash
cd firmware
make clean
make -j4
```

**Using Keil:**
```
1. Open project in Keil µVision
2. Project -> Build Target (F7)
```

### Running Mobile App in Development

**iOS:**
```bash
cd mobile-app
npm start           # Start Metro bundler
npm run ios         # Run on iOS simulator
```

**Android:**
```bash
cd mobile-app
npm start           # Start Metro bundler
npm run android     # Run on Android emulator
```

### Code Style

- **Firmware**: Follow Dialog SDK coding conventions
- **Mobile App**: ESLint with TypeScript rules
- Use consistent indentation (2 spaces for JS/TS, 4 for C)

## 🧪 Testing

### Firmware Testing
```bash
# Using nRF Connect app
1. Install nRF Connect on smartphone
2. Scan for BLE devices
3. Look for Service UUID 0xAA28
4. Verify advertising data format
```

### Mobile App Testing
```bash
cd mobile-app
npm test              # Run unit tests
npm test -- --coverage  # Run with coverage
```

### Integration Testing
1. Flash firmware to DA14531
2. Power on device
3. Open mobile app
4. Scan for devices
5. Send commands and verify buzzer responds
6. Check command history is logged

## 📦 Deployment

### Firmware Deployment
- Flash via SmartSnippets Toolbox or J-Link
- For mass production, use Dialog programming tools
- Consider OTA (Over-The-Air) updates for field devices

### Mobile App Deployment

**iOS (App Store):**
```bash
# Build in Xcode
# Archive and upload to App Store Connect
# Submit for review
```

**Android (Google Play):**
```bash
cd mobile-app/android
./gradlew bundleRelease
# Upload AAB to Google Play Console
```

## ❓ Troubleshooting

### Common Issues

**Firmware doesn't advertise:**
- Check power supply (3.3V)
- Verify firmware flashed successfully
- Check BLE antenna connection

**Mobile app can't find devices:**
- Grant Bluetooth permissions
- Enable location services (Android requirement)
- Ensure firmware is advertising
- Check Service UUID matches

**Commands not working:**
- Verify device name matches
- Check timestamp is being set
- Review command processing in firmware

For more troubleshooting, see [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md).

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

Please ensure:
- Code follows project style guidelines
- All tests pass
- Documentation is updated
- Commit messages are clear

## 📄 License

This project is licensed under the Boost Software License - Version 1.0.

See the [LICENSE](LICENSE) file for details.

## 📞 Support

For issues, questions, or contributions:
- **Issues**: [GitHub Issues](https://github.com/HumYhaiJung/ble-beacon-buzzer-control/issues)
- **Discussions**: [GitHub Discussions](https://github.com/HumYhaiJung/ble-beacon-buzzer-control/discussions)

## 🙏 Acknowledgments

- Dialog Semiconductor for DA14531 SDK
- React Native community
- react-native-ble-plx library maintainers
- All contributors to this project

## 🗺️ Roadmap

### Planned Features
- [ ] Encryption and authentication for beacon protocol
- [ ] Multiple device control (groups)
- [ ] Custom buzzer patterns
- [ ] OTA firmware updates
- [ ] Cloud integration
- [ ] Advanced analytics and reporting

---

**Made with ❤️ for IoT enthusiasts**
