# React Native Mobile App Setup Guide

This guide will help you set up the development environment for the BLE Beacon Buzzer Control mobile app.

## Prerequisites

- macOS (for iOS development), Windows, or Linux
- 8GB RAM minimum (16GB recommended)
- 20GB free disk space
- Basic knowledge of React Native and TypeScript

## System Requirements

### Common (All Platforms)

- **Node.js**: v16.x or later (LTS recommended)
- **npm**: v8.x or later (or **yarn** v1.22.x)
- **Watchman**: v4.9.0 or later (macOS/Linux)

### iOS Development

- **macOS**: 10.15 (Catalina) or later
- **Xcode**: 13.0 or later
- **CocoaPods**: 1.11.0 or later
- **iOS Simulator** or physical device (iOS 12.0+)

### Android Development

- **Android Studio**: 2021.3.1 or later
- **JDK**: 11 or 17
- **Android SDK**: API 21+ (Android 5.0+)
- **Android NDK**: r21 or later

## Installation

### 1. Install Node.js

**macOS:**
```bash
# Using Homebrew
brew install node

# Or download from nodejs.org
# Verify installation
node --version
npm --version
```

**Windows:**
```
1. Download installer from https://nodejs.org
2. Run installer and follow wizard
3. Verify in Command Prompt:
   node --version
   npm --version
```

**Linux (Ubuntu/Debian):**
```bash
# Using NodeSource repository
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt-get install -y nodejs

# Verify installation
node --version
npm --version
```

### 2. Install Watchman (macOS/Linux)

**macOS:**
```bash
brew install watchman
```

**Linux:**
```bash
# Build from source
git clone https://github.com/facebook/watchman.git
cd watchman
./autogen.sh
./configure
make
sudo make install
```

### 3. Install Expo CLI

```bash
# Install globally
npm install -g expo-cli

# Verify installation
expo --version
```

## iOS Setup (macOS only)

### 1. Install Xcode

```
1. Open Mac App Store
2. Search for "Xcode"
3. Download and install (may take 30+ minutes)
4. Open Xcode and accept license agreement
5. Install additional components when prompted
```

### 2. Install Xcode Command Line Tools

```bash
xcode-select --install
```

### 3. Install CocoaPods

```bash
# Using Ruby gem
sudo gem install cocoapods

# Or using Homebrew
brew install cocoapods

# Verify installation
pod --version
```

### 4. Configure iOS Simulator

```
1. Open Xcode
2. Xcode -> Preferences -> Components
3. Download desired iOS simulators
4. Test with: open -a Simulator
```

## Android Setup

### 1. Install Android Studio

**Download:**
- Visit: https://developer.android.com/studio
- Download Android Studio for your platform

**Installation:**

**macOS:**
```
1. Open downloaded .dmg file
2. Drag Android Studio to Applications
3. Launch Android Studio
4. Follow setup wizard
5. Install Android SDK, SDK Platform, and Virtual Device
```

**Windows:**
```
1. Run the installer (.exe)
2. Follow installation wizard
3. Select standard installation
4. Install all recommended components
```

**Linux:**
```bash
# Extract archive
tar -xzf android-studio-*.tar.gz
cd android-studio/bin

# Launch
./studio.sh
```

### 2. Configure Android SDK

```
1. Open Android Studio
2. Tools -> SDK Manager
3. SDK Platforms tab:
   - Select Android 13.0 (API 33) or later
   - Select Android 5.0 (API 21) for minimum support
4. SDK Tools tab:
   - Android SDK Build-Tools
   - Android SDK Platform-Tools
   - Android Emulator
   - Intel x86 Emulator Accelerator (HAXM)
5. Click Apply to install
```

### 3. Set Environment Variables

**macOS/Linux:**
```bash
# Add to ~/.bash_profile, ~/.zshrc, or ~/.bashrc
export ANDROID_HOME=$HOME/Library/Android/sdk  # macOS
# export ANDROID_HOME=$HOME/Android/Sdk        # Linux

export PATH=$PATH:$ANDROID_HOME/emulator
export PATH=$PATH:$ANDROID_HOME/platform-tools
export PATH=$PATH:$ANDROID_HOME/tools
export PATH=$PATH:$ANDROID_HOME/tools/bin

# Reload shell configuration
source ~/.bash_profile  # or ~/.zshrc
```

**Windows:**
```
1. Open System Properties -> Advanced -> Environment Variables
2. Add new user variable:
   Variable name: ANDROID_HOME
   Variable value: C:\Users\<username>\AppData\Local\Android\Sdk
3. Edit Path variable, add:
   %ANDROID_HOME%\platform-tools
   %ANDROID_HOME%\emulator
   %ANDROID_HOME%\tools
   %ANDROID_HOME%\tools\bin
```

### 4. Create Android Virtual Device (AVD)

```
1. Open Android Studio
2. Tools -> AVD Manager
3. Click "Create Virtual Device"
4. Select device (e.g., Pixel 5)
5. Select system image (e.g., API 33)
6. Configure AVD settings
7. Click Finish
8. Launch emulator to test
```

## Project Setup

### 1. Clone Repository

```bash
git clone https://github.com/yourusername/ble-beacon-buzzer-control.git
cd ble-beacon-buzzer-control/mobile-app
```

### 2. Install Dependencies

```bash
# Using npm
npm install

# Or using yarn
yarn install
```

### 3. Install iOS Dependencies (macOS only)

```bash
cd ios
pod install
cd ..
```

## Running the App

### Development Mode

**Start Metro Bundler:**
```bash
npm start
# Or
expo start
```

### Run on iOS (macOS only)

**Simulator:**
```bash
npm run ios
# Or
expo start --ios
```

**Physical Device:**
```bash
# Connect device via USB
# Enable developer mode on device
# Trust computer when prompted
npm run ios --device
```

### Run on Android

**Emulator:**
```bash
# Start emulator first (AVD Manager or command line)
npm run android
# Or
expo start --android
```

**Physical Device:**
```bash
# Enable Developer Options and USB Debugging on device
# Connect device via USB
# Verify connection: adb devices
npm run android
```

## Building for Production

### iOS

**Build with Xcode:**
```
1. Open ios/BleBeaconBuzzer.xcworkspace in Xcode
2. Select target device or "Any iOS Device"
3. Product -> Archive
4. Distribute App -> App Store Connect or Ad Hoc
5. Follow signing and provisioning steps
```

**Build with Expo:**
```bash
expo build:ios
```

### Android

**Build APK:**
```bash
cd android
./gradlew assembleRelease

# APK output:
# android/app/build/outputs/apk/release/app-release.apk
```

**Build AAB (for Play Store):**
```bash
cd android
./gradlew bundleRelease

# AAB output:
# android/app/build/outputs/bundle/release/app-release.aab
```

**Build with Expo:**
```bash
expo build:android
```

## Permissions Configuration

### iOS (Info.plist)

Already configured in `app.json`, but verify:
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app needs Bluetooth to control buzzer devices</string>
<key>NSBluetoothPeripheralUsageDescription</key>
<string>This app needs Bluetooth to scan for devices</string>
<key>NSLocationWhenInUseUsageDescription</key>
<string>Location is required for Bluetooth scanning</string>
```

### Android (AndroidManifest.xml)

Already configured in `app.json`, but verify:
```xml
<uses-permission android:name="android.permission.BLUETOOTH"/>
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN"/>
<uses-permission android:name="android.permission.BLUETOOTH_SCAN"/>
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT"/>
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
<uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION"/>
```

## Troubleshooting

### Common Issues

**Metro Bundler fails to start:**
```bash
# Clear cache
npm start -- --reset-cache

# Or with Expo
expo start -c
```

**Module not found errors:**
```bash
# Clear node modules and reinstall
rm -rf node_modules
npm install

# Clear watchman cache (macOS/Linux)
watchman watch-del-all
```

**iOS build fails:**
```bash
# Clear derived data
rm -rf ~/Library/Developer/Xcode/DerivedData

# Clean build
cd ios
xcodebuild clean
pod deintegrate
pod install
cd ..
```

**Android build fails:**
```bash
# Clean gradle
cd android
./gradlew clean

# Clear gradle cache
rm -rf ~/.gradle/caches/
./gradlew build --refresh-dependencies
```

### BLE Permissions Issues

**iOS:**
- Ensure Info.plist has all required descriptions
- Request permissions at runtime
- Check Settings -> Privacy -> Bluetooth

**Android:**
- For Android 12+, BLUETOOTH_SCAN and BLUETOOTH_CONNECT required
- Request location permission for BLE scanning
- Enable location services in device settings

### Device Not Detected

**iOS:**
- Verify device is in developer mode
- Trust computer in device settings
- Check cable connection
- Use `ios-deploy -c` to list devices

**Android:**
```bash
# List connected devices
adb devices

# If unauthorized, check device for prompt
# If not listed, check drivers (Windows) or udev rules (Linux)

# Restart adb server
adb kill-server
adb start-server
```

## Testing

### Run Tests

```bash
# Run all tests
npm test

# Run with coverage
npm test -- --coverage

# Watch mode
npm test -- --watch
```

### E2E Testing (Optional)

Install Detox for end-to-end testing:
```bash
npm install -g detox-cli
npm install --save-dev detox
```

## Development Tips

1. **Hot Reloading**: Enable fast refresh for instant updates
2. **React DevTools**: Install browser extension for debugging
3. **Flipper**: Use for advanced React Native debugging
4. **Remote Debugging**: Use Chrome DevTools for JS debugging

## Resources

- [React Native Documentation](https://reactnative.dev/docs/getting-started)
- [Expo Documentation](https://docs.expo.dev/)
- [react-native-ble-plx](https://github.com/dotintent/react-native-ble-plx)
- [React Navigation](https://reactnavigation.org/docs/getting-started)

## Next Steps

After successful setup:
1. Configure BLE permissions on your test device
2. Ensure firmware is running on DA14531
3. Test scanning and command sending
4. Review logs for any BLE connection issues
