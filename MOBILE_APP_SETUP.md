# Mobile App Setup Guide

## ⚠️ Important: Do NOT use Expo Go

This app uses native modules (`react-native-ble-plx`) that are **NOT supported** in Expo Go. You must build the app with native code.

## Prerequisites

1. **Node.js**: Version 20.x or higher (recommended: 20.19.4+)

   - Check version: `node --version`
   - Download from: https://nodejs.org/

2. **Android Development** (for Android)

   - Android Studio or Android SDK
   - Emulator or real device with Android OS

3. **iOS Development** (for iOS)
   - macOS with Xcode
   - Real device or simulator

## Setup Steps

### 1. Install Dependencies

```bash
cd mobile-app
npm install
```

### 2. Build and Run on Android

```bash
npx expo run:android
```

This will:

- Build the native Android app
- Link react-native-ble-plx
- Deploy to emulator or connected device
- Start the Metro bundler

### 3. Build and Run on iOS

```bash
npx expo run:ios
```

This will:

- Build the native iOS app
- Link react-native-ble-plx
- Deploy to simulator or device
- Start the Metro bundler

## Troubleshooting

### Error: "Cannot read property 'createClient' of null"

**Cause**: App is running in Expo Go or web environment
**Solution**: Use `npx expo run:android` or `npx expo run:ios` instead

### Error: "BLE native module not available"

**Cause**: Native module was not properly linked
**Solution**:

1. Delete node_modules: `rm -r node_modules`
2. Delete build cache: `rm -r android/.gradle` (or iOS build folder)
3. Reinstall: `npm install`
4. Rebuild: `npx expo run:android` (or run:ios)

### Error: "Unsupported engine" warnings

**Cause**: Node.js version is too old
**Solution**: Upgrade Node.js to version 20+

```bash
# Check current version
node --version

# If old, upgrade from https://nodejs.org/
```

### Android: "Could not find Android SDK"

**Solution**: Install Android Studio from https://developer.android.com/studio

### iOS: Build fails with CocoaPods error

**Solution**: Update CocoaPods

```bash
sudo gem install cocoapods
pod repo update
```

## Development Workflow

### Hot Reload

When running the app with `npx expo run:android` or `npx expo run:ios`, the Metro bundler allows:

- **Fast Refresh**: Auto-reload on JS file changes
- **Manual reload**: Press 'R' in the Metro bundler console

### Running Metro Bundler Separately

```bash
npx expo start
```

Then select:

- Press 'a' for Android
- Press 'i' for iOS

### Testing

```bash
npm test
```

### Linting

```bash
npm run lint
```

## App Features

### Screens

1. **Scan** (🔍): Discover BLE beacon devices
2. **History** (📜): View command history

### Device Communication

- Uses BLE beacons (connectionless)
- Sends commands: PLAY, STOP, ENABLE, DISABLE, STATUS
- Parses beacon payload with device name and timestamp
- Anti-replay protection via timestamp validation

## Architecture

### Services

- **bleService.ts**: BLE scanning and device discovery
- **commandService.ts**: Command formatting and transmission
- **storageService.ts**: Local data persistence

### Context

- **AppContext.tsx**: Global state management

### Types

- **index.ts**: TypeScript interfaces and enums

## Dependencies

### Critical Native Modules

- `react-native-ble-plx`: BLE scanning and communication
- `react-native-permissions`: Runtime permissions

### UI/Navigation

- `@react-navigation/*`: Screen navigation
- `react-native-svg`: Icon rendering
- `react-native-chart-kit`: Data visualization

### Utilities

- `axios`: HTTP requests
- `@react-native-async-storage/async-storage`: Local storage

## Notes

- ✅ Uses Hermes JS engine for better performance
- ✅ Supports Android 11+ and iOS 13+
- ✅ BLE beacon protocol: Service UUID `0xAA28`
- ✅ Anti-replay: Timestamp validation prevents command replay attacks
- ⚠️ Requires native build (NOT Expo Go)
- ⚠️ Requires Node.js 20+

## Support

If issues persist:

1. Check error message carefully
2. Follow the troubleshooting section
3. Delete node_modules and reinstall
4. Ensure you're using the correct build command
5. Verify device/emulator is properly set up
