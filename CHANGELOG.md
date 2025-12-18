# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2024-12-18

### Added

#### Firmware (Dialog DA14531)
- Initial beacon-based buzzer control implementation
- Custom BLE Service UUID 0xAA28 for beacon advertising
- Beacon payload format with device name, command, and timestamp
- Command processing: Play, Stop, Enable, Disable, Status
- Power sensing via GPIO
- Status LED indication
- Low power beacon advertising mode
- Timestamp-based replay attack prevention
- GPIO configuration for buzzer, power sense, and status LED

#### Mobile App (React Native)
- Cross-platform mobile app for iOS and Android
- BLE beacon scanning and device discovery
- Real-time RSSI (signal strength) monitoring
- Signal strength visualization with line charts
- Command interface with five control buttons
- Command history with persistent storage
- Favorite devices management
- Device list with signal strength bars
- Pull-to-refresh on scan screen
- Filter command history by device
- TypeScript implementation with full type safety
- React Context for global state management
- AsyncStorage integration for data persistence
- Navigation with React Navigation (Stack and Bottom Tabs)

#### Documentation
- Comprehensive README with quick start guide
- Protocol specification (PROTOCOL.md)
  - Beacon packet format
  - Command codes reference
  - Example packets
  - Security considerations
- Firmware setup guide (FIRMWARE_SETUP.md)
  - Dialog SDK installation
  - Toolchain setup (Keil and GCC)
  - Project configuration
  - Build instructions
- Mobile app setup guide (APP_SETUP.md)
  - Node.js and React Native setup
  - iOS development setup (Xcode, CocoaPods)
  - Android development setup (Android Studio)
  - Build and deployment instructions
- Complete build guide (BUILD_GUIDE.md)
  - Step-by-step build process
  - Integration testing
  - Troubleshooting
- Contributing guidelines (CONTRIBUTING.md)
- Changelog (this file)

#### Build System
- Makefile for firmware compilation (GCC)
- J-Link flashing script
- TypeScript configuration
- ESLint and Prettier setup
- Jest testing configuration
- Metro bundler configuration
- Babel configuration for React Native

#### Project Infrastructure
- Git repository structure
- .gitignore for Node.js, React Native, and firmware artifacts
- MIT/Boost Software License
- Organized directory structure

### Features

#### Firmware Features
- Connectionless BLE beacon operation (no pairing required)
- 9-byte payload with device addressing
- Command validation and processing
- Buzzer control with multiple states
- Status reporting via beacon
- Power-efficient advertising

#### Mobile App Features
- Real-time device scanning
- RSSI-based device sorting
- Visual signal strength indicators
- Interactive command buttons with loading states
- Persistent command history (up to 100 commands)
- Device favorites system
- Cross-platform compatibility (iOS 12+, Android 5.0+)
- Clean, modern UI with Material Design principles

### Technical Details

- **Firmware**: C for Dialog DA14531, Dialog SDK 6.0.14+
- **Mobile App**: React Native 0.72.6, TypeScript 5.1.3, Expo 49.0.15
- **BLE Library**: react-native-ble-plx 3.1.2
- **Navigation**: React Navigation 6.x
- **Storage**: AsyncStorage
- **Charts**: react-native-chart-kit

### Known Limitations

- Firmware requires Dialog DA14531 hardware
- Mobile app requires BLE 4.0+ support
- iOS requires location permission for BLE scanning
- Android 12+ requires specific BLE permissions
- Command acknowledgment is passive (via status beacon)
- No encryption in current version (planned for future)

### Security

- Timestamp-based replay attack prevention
- Device name addressing for multi-device scenarios
- No sensitive data in beacon payload

---

## [Unreleased]

### Planned Features

- Encryption and authentication for beacon protocol
- OTA firmware updates
- Multiple device control (groups/scenes)
- Custom buzzer patterns
- Cloud integration
- Advanced analytics dashboard
- Device pairing mechanism
- Extended advertising (BLE 5.0)
- iOS widget support
- Android widget support

---

**Legend:**
- `Added`: New features
- `Changed`: Changes in existing functionality
- `Deprecated`: Soon-to-be removed features
- `Removed`: Removed features
- `Fixed`: Bug fixes
- `Security`: Security improvements
