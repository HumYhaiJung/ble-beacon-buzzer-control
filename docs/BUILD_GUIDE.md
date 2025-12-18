# Complete Build Guide

This guide provides step-by-step instructions for building both the firmware and mobile app for the BLE Beacon Buzzer Control project.

## Overview

The project consists of two main components:
1. **Firmware**: C code for Dialog DA14531 SoC
2. **Mobile App**: React Native application for iOS and Android

## Prerequisites Checklist

Before starting, ensure you have:

- [ ] Development computer (Windows, macOS, or Linux)
- [ ] Dialog DA14531 development board
- [ ] USB cable for DA14531 (programming/debugging)
- [ ] iOS device (for iOS app testing) or Android device
- [ ] Buzzer component connected to GPIO
- [ ] Internet connection for downloading dependencies

## Quick Start

For users who want to get started quickly:

```bash
# 1. Build firmware (requires Dialog SDK and toolchain)
cd firmware
make

# 2. Build mobile app
cd ../mobile-app
npm install
npm run android  # or npm run ios
```

## Part 1: Firmware Build

### Step 1: Setup Development Environment

Follow the [Firmware Setup Guide](FIRMWARE_SETUP.md) to install:
- Dialog SmartSnippets Studio
- Dialog SDK 6.0.14
- ARM toolchain (Keil or GCC)
- J-Link software

**Verification:**
```bash
# Check GCC installation
arm-none-eabi-gcc --version

# Check SDK
ls $SDK_ROOT
```

### Step 2: Prepare Project

1. **Create project directory**
   ```bash
   mkdir -p ~/workspace/ble-beacon-buzzer-control/firmware
   cd ~/workspace/ble-beacon-buzzer-control/firmware
   ```

2. **Copy source files**
   ```bash
   # Copy from repository
   cp -r src/ .
   ```

3. **Create/update Makefile** (for GCC build)
   ```makefile
   # See firmware/Makefile for complete example
   SDK_ROOT = /path/to/SDK_6.0.14
   TARGET = DA14531
   ```

### Step 3: Build Firmware

**Option A: Using GCC**
```bash
cd firmware

# Clean previous build
make clean

# Build
make -j4

# Output: build/ble_beacon_buzzer_control.hex
```

**Option B: Using Keil**
```
1. Open Keil µVision
2. File -> Open Project -> ble_beacon_buzzer_control.uvprojx
3. Project -> Build Target (F7)
4. Output: Objects/ble_beacon_buzzer_control.hex
```

**Build Time:** ~2-5 minutes (first build), ~30 seconds (incremental)

### Step 4: Verify Build

```bash
# Check output file exists
ls -lh build/ble_beacon_buzzer_control.hex

# Check file size (should be < 48KB for DA14531)
du -h build/ble_beacon_buzzer_control.hex

# Verify hex file format
head -n 5 build/ble_beacon_buzzer_control.hex
```

Expected output:
```
:020000040000FA
:10000000...
```

### Step 5: Flash Firmware

**Option A: Using SmartSnippets Toolbox**
```
1. Connect DA14531 board via USB
2. Open SmartSnippets Toolbox
3. Select "Flash Programmer"
4. Device: DA14531
5. Load HEX file: build/ble_beacon_buzzer_control.hex
6. Click "Program"
7. Wait for success message
```

**Option B: Using J-Link Commander**
```bash
# Start J-Link
JLinkExe -device DA14531 -if SWD -speed 4000

# Inside J-Link prompt:
connect
loadfile build/ble_beacon_buzzer_control.hex
r
g
q
```

### Step 6: Test Firmware

1. **Power on device**
   - Connect power to DA14531 board
   - Status LED should illuminate

2. **Scan for beacon**
   - Use nRF Connect app on smartphone
   - Scan for BLE devices
   - Look for device with Service UUID 0xAA28
   - Verify advertising data contains beacon payload

3. **Check serial output** (if available)
   ```bash
   # Connect to serial console (if UART connected)
   screen /dev/ttyUSB0 115200
   ```

## Part 2: Mobile App Build

### Step 1: Setup Development Environment

Follow the [App Setup Guide](APP_SETUP.md) to install:
- Node.js and npm
- React Native dependencies
- iOS tools (macOS only)
- Android Studio and SDK

**Verification:**
```bash
# Check Node.js
node --version  # Should be v16+

# Check npm
npm --version

# Check React Native
npx react-native --version

# Check Android (if doing Android development)
adb --version
```

### Step 2: Install Dependencies

```bash
cd mobile-app

# Install JavaScript dependencies
npm install

# This may take 5-10 minutes for first install
```

**Expected packages:**
- react-native-ble-plx (BLE support)
- @react-navigation (navigation)
- @react-native-async-storage (storage)
- react-native-chart-kit (charts)

### Step 3: iOS Setup (macOS only)

```bash
# Install CocoaPods dependencies
cd ios
pod install
cd ..

# This may take 5-10 minutes
```

**Verify:**
```bash
ls ios/*.xcworkspace  # Should show workspace file
```

### Step 4: Build for Development

**iOS (Simulator):**
```bash
# Start Metro bundler in one terminal
npm start

# In another terminal, run on iOS
npm run ios

# Or specify simulator
npm run ios -- --simulator="iPhone 14"
```

**iOS (Device):**
```
1. Open ios/BleBeaconBuzzer.xcworkspace in Xcode
2. Connect iPhone via USB
3. Select your device from device list
4. Product -> Run (Cmd+R)
5. Trust developer certificate on device if prompted
```

**Android (Emulator):**
```bash
# Start emulator (or use AVD Manager)
emulator -avd Pixel_5_API_33 &

# Start Metro bundler
npm start

# In another terminal, run on Android
npm run android
```

**Android (Device):**
```bash
# Enable USB debugging on Android device
# Connect device via USB
# Verify connection
adb devices

# Run app
npm run android
```

**Build Time:** 
- First build: 10-15 minutes
- Incremental: 30-60 seconds

### Step 5: Build for Production

**iOS (App Store / TestFlight):**
```
1. Open Xcode workspace
2. Select "Any iOS Device" (not simulator)
3. Product -> Archive
4. Wait for archive to complete (5-10 minutes)
5. Organizer window opens automatically
6. Click "Distribute App"
7. Choose distribution method:
   - App Store Connect (for TestFlight/App Store)
   - Ad Hoc (for testing on registered devices)
   - Enterprise (if you have enterprise account)
8. Follow signing and provisioning steps
9. Upload to App Store Connect or export IPA
```

**Android (APK):**
```bash
cd mobile-app/android

# Build release APK
./gradlew assembleRelease

# Output: android/app/build/outputs/apk/release/app-release.apk

# Build time: 3-5 minutes
```

**Android (AAB for Play Store):**
```bash
cd mobile-app/android

# Build release bundle
./gradlew bundleRelease

# Output: android/app/build/outputs/bundle/release/app-release.aab
```

### Step 6: Test Mobile App

1. **Grant Permissions**
   - iOS: Settings -> Privacy -> Bluetooth (enable for app)
   - Android: App will request permissions on first launch

2. **Test BLE Scanning**
   - Open app
   - Navigate to "Scan" tab
   - Tap "Start Scanning"
   - Verify beacon device appears in list
   - Check RSSI signal strength

3. **Test Commands**
   - Tap on a discovered device
   - Try "Play Buzzer" command
   - Verify buzzer activates on hardware
   - Check command appears in History tab

## Part 3: Integration Testing

### End-to-End Test

1. **Setup**
   - Flash firmware to DA14531
   - Power on DA14531 board
   - Ensure buzzer is connected
   - Install mobile app on smartphone

2. **Scanning Test**
   ```
   1. Open mobile app
   2. Start scanning
   3. Verify device appears within 5 seconds
   4. Check RSSI updates in real-time
   5. Verify device name matches firmware (e.g., "BZR1")
   ```

3. **Command Test**
   ```
   1. Tap on device to open details
   2. Send "Play" command
   3. Verify buzzer sounds
   4. Send "Stop" command
   5. Verify buzzer stops
   6. Check status LED on DA14531 blinks on command
   ```

4. **Range Test**
   ```
   1. Move smartphone away from DA14531
   2. Observe RSSI value decrease
   3. Test maximum range (typically 10-30 meters)
   4. Verify commands still work at edge of range
   ```

## Troubleshooting

### Firmware Build Issues

**Problem: "SDK_ROOT not found"**
```bash
# Solution: Set environment variable
export SDK_ROOT=/path/to/SDK_6.0.14
```

**Problem: "Compiler not found"**
```bash
# Solution: Install ARM GCC toolchain
# See FIRMWARE_SETUP.md for instructions
```

**Problem: "Region ROM overflowed"**
```
Solution: Enable optimization in build settings
- Keil: -O2 optimization level
- GCC: Add -Os to CFLAGS
```

### Flash Issues

**Problem: "Device not detected"**
```
Solution:
1. Check USB connection
2. Try different USB port
3. Install/update J-Link drivers
4. Power cycle the board
```

**Problem: "Flash verification failed"**
```
Solution:
1. Erase flash: JLink -> erase
2. Try slower programming speed
3. Check power supply voltage (3.3V)
```

### Mobile App Build Issues

**Problem: Metro bundler fails**
```bash
# Solution: Clear cache
npm start -- --reset-cache
```

**Problem: "Module not found"**
```bash
# Solution: Reinstall dependencies
rm -rf node_modules
npm install
```

**Problem: iOS pod install fails**
```bash
# Solution: Update CocoaPods and retry
cd ios
rm -rf Pods Podfile.lock
pod repo update
pod install
```

**Problem: Android build fails**
```bash
# Solution: Clean gradle
cd android
./gradlew clean
./gradlew build --refresh-dependencies
```

### Runtime Issues

**Problem: App can't find devices**
```
Solution:
1. Check Bluetooth permissions granted
2. Enable location services (Android requirement)
3. Verify firmware is advertising
4. Check Service UUID matches (0xAA28)
```

**Problem: Commands not working**
```
Solution:
1. Verify beacon payload format
2. Check device name matches
3. Ensure timestamp is being set
4. Review firmware command processing logic
```

## Performance Tips

### Firmware
- Enable compiler optimizations (-O2 or -Os)
- Use extended sleep mode when idle
- Minimize advertising data size
- Optimize advertising interval (100-1000ms)

### Mobile App
- Implement efficient device list updates
- Debounce UI updates for RSSI
- Cache discovered devices
- Batch command operations

## Build Time Summary

| Component | First Build | Incremental | Flash/Deploy |
|-----------|-------------|-------------|--------------|
| Firmware  | 2-5 min     | 30 sec      | 30-60 sec    |
| iOS App   | 10-15 min   | 30-60 sec   | 2-3 min      |
| Android App | 3-5 min   | 30-60 sec   | 1-2 min      |

## Next Steps

After successful build and testing:
1. Read [PROTOCOL.md](PROTOCOL.md) for beacon protocol details
2. Customize firmware for your specific hardware
3. Add additional features to mobile app
4. Implement security enhancements
5. Deploy to production

## Resources

- [Firmware Setup](FIRMWARE_SETUP.md)
- [App Setup](APP_SETUP.md)
- [Protocol Specification](PROTOCOL.md)
- [Dialog DA14531 Documentation](https://www.dialog-semiconductor.com/products/da14531)
- [React Native Documentation](https://reactnative.dev)
