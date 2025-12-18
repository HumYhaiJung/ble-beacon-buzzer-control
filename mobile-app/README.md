# BLE Beacon Buzzer Control - Mobile App

React Native mobile application for controlling buzzer devices via BLE beacons.

## Features

- 📡 BLE beacon scanning with real-time device discovery
- 📊 Signal strength monitoring and visualization
- 🎮 Command interface for buzzer control
- 📜 Command history with filtering
- ⭐ Favorite devices management
- 💾 Persistent storage
- 📱 Cross-platform (iOS and Android)

## Prerequisites

- Node.js v16 or later
- npm or yarn
- For iOS: macOS, Xcode 13+, CocoaPods
- For Android: Android Studio, JDK 11+

## Installation

```bash
# Install dependencies
npm install

# Install iOS dependencies (macOS only)
cd ios
pod install
cd ..
```

## Running the App

### Development Mode

**Start Metro Bundler:**
```bash
npm start
```

**iOS:**
```bash
# Simulator
npm run ios

# Specific simulator
npm run ios -- --simulator="iPhone 14"

# Physical device (via Xcode)
# Open ios/BleBeaconBuzzer.xcworkspace in Xcode
# Select your device and press Run
```

**Android:**
```bash
# Emulator
npm run android

# Physical device
# Enable USB debugging and connect device
npm run android
```

## Building for Production

### iOS

**Build with Xcode:**
```
1. Open ios/BleBeaconBuzzer.xcworkspace
2. Select "Any iOS Device"
3. Product -> Archive
4. Distribute App -> App Store Connect or Ad Hoc
```

### Android

**Build APK:**
```bash
cd android
./gradlew assembleRelease

# Output: android/app/build/outputs/apk/release/app-release.apk
```

**Build AAB:**
```bash
cd android
./gradlew bundleRelease

# Output: android/app/build/outputs/bundle/release/app-release.aab
```

## Project Structure

```
mobile-app/
├── src/
│   ├── components/          # Reusable UI components
│   │   ├── DeviceCard.tsx
│   │   ├── CommandButton.tsx
│   │   └── StatusIndicator.tsx
│   ├── context/             # React Context
│   │   └── AppContext.tsx
│   ├── screens/             # Screen components
│   │   ├── ScanScreen.tsx
│   │   ├── DeviceScreen.tsx
│   │   └── HistoryScreen.tsx
│   ├── services/            # Business logic
│   │   ├── bleService.ts
│   │   ├── commandService.ts
│   │   └── storageService.ts
│   └── types/               # TypeScript types
│       └── index.ts
├── App.tsx                  # App entry point
├── package.json
└── app.json
```

## Permissions

### iOS (Info.plist)
- `NSBluetoothAlwaysUsageDescription`
- `NSBluetoothPeripheralUsageDescription`
- `NSLocationWhenInUseUsageDescription`

### Android (AndroidManifest.xml)
- `BLUETOOTH`
- `BLUETOOTH_ADMIN`
- `BLUETOOTH_SCAN`
- `BLUETOOTH_CONNECT`
- `ACCESS_FINE_LOCATION`
- `ACCESS_COARSE_LOCATION`

## Testing

```bash
# Run tests
npm test

# Run with coverage
npm test -- --coverage

# Watch mode
npm test -- --watch
```

## Troubleshooting

### Metro Bundler Issues
```bash
# Clear cache
npm start -- --reset-cache
```

### Module Not Found
```bash
# Reinstall dependencies
rm -rf node_modules
npm install
```

### iOS Build Issues
```bash
# Clean build
cd ios
pod deintegrate
pod install
cd ..
```

### Android Build Issues
```bash
# Clean gradle
cd android
./gradlew clean
cd ..
```

### BLE Permissions
- Ensure all permissions are granted in device settings
- For Android 12+, location services must be enabled
- For iOS, check Settings -> Privacy -> Bluetooth

## Dependencies

### Core
- `react-native`: ^0.72.6
- `expo`: ~49.0.15
- `react-native-ble-plx`: ^3.1.2

### Navigation
- `@react-navigation/native`: ^6.1.9
- `@react-navigation/stack`: ^6.3.20
- `@react-navigation/bottom-tabs`: ^6.5.11

### UI & Charts
- `react-native-chart-kit`: ^6.12.0
- `react-native-svg`: 13.9.0

### Storage
- `@react-native-async-storage/async-storage`: ^1.19.5

### Utilities
- `axios`: ^1.6.2

## Scripts

```bash
npm start          # Start Metro bundler
npm run android    # Run on Android
npm run ios        # Run on iOS
npm run web        # Run on web (Expo)
npm run lint       # Run ESLint
npm test           # Run tests
```

## Configuration

### App Configuration (app.json)
- App name and slug
- Version
- iOS bundle identifier
- Android package name
- Permissions
- Icon and splash screen

### Environment Variables
Create `.env` file for environment-specific configuration:
```
API_URL=https://api.example.com
DEBUG_MODE=false
```

## Features in Detail

### BLE Scanning
- Continuous scanning for beacon devices
- Real-time RSSI updates
- Device filtering by Service UUID (0xAA28)
- Pull-to-refresh support

### Device Control
- Send Play/Stop commands
- Enable/Disable buzzer
- Request device status
- Visual feedback for command results

### Signal Strength
- Real-time RSSI monitoring
- Historical graph visualization
- Signal quality indicator

### Command History
- Persistent command log
- Success/failure tracking
- Filter by device
- Clear history option

## Contributing

1. Fork the repository
2. Create feature branch
3. Make changes
4. Run tests and linting
5. Submit pull request

## License

See main repository LICENSE file.

## Support

For issues and questions:
- GitHub Issues: [Report Issue](https://github.com/HumYhaiJung/ble-beacon-buzzer-control/issues)
- Documentation: See `docs/` folder in main repository

## Resources

- [React Native Docs](https://reactnative.dev/docs/getting-started)
- [Expo Docs](https://docs.expo.dev/)
- [react-native-ble-plx](https://github.com/dotintent/react-native-ble-plx)
- [React Navigation](https://reactnavigation.org/)
