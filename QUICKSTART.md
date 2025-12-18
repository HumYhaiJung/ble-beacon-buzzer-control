# Quick Start Guide

Get started with BLE Beacon Buzzer Control in 10 minutes!

## What You'll Need

- Dialog DA14531 development board with buzzer
- Smartphone (iOS or Android)
- Computer for development

## Step 1: Flash Firmware (5 minutes)

### Option A: Pre-built Binary (Recommended for Quick Start)
Unfortunately, you'll need to build the firmware yourself as it requires Dialog SDK.

### Option B: Build from Source

1. **Install Dialog SDK**
   ```bash
   # Download from dialog-semiconductor.com
   # Extract to ~/Dialog/SDK_6.0.14
   ```

2. **Install ARM GCC Toolchain**
   ```bash
   # macOS
   brew install gcc-arm-embedded
   
   # Linux
   sudo apt-get install gcc-arm-none-eabi
   ```

3. **Build Firmware**
   ```bash
   cd firmware
   export SDK_ROOT=~/Dialog/SDK_6.0.14
   make
   ```

4. **Flash to Device**
   - Connect DA14531 via USB
   - Open SmartSnippets Toolbox
   - Flash `build/ble_beacon_buzzer_control.hex`

**Note:** For detailed firmware setup, see [docs/FIRMWARE_SETUP.md](docs/FIRMWARE_SETUP.md)

## Step 2: Install Mobile App (3 minutes)

### For Testing (Development Mode)

1. **Install Dependencies**
   ```bash
   cd mobile-app
   npm install
   
   # For iOS (macOS only)
   cd ios && pod install && cd ..
   ```

2. **Run on Device**
   ```bash
   # Android
   npm run android
   
   # iOS (macOS only)
   npm run ios
   ```

### For Production (Release Build)

See [docs/APP_SETUP.md](docs/APP_SETUP.md) for building APK/IPA files.

## Step 3: Test the System (2 minutes)

1. **Power on DA14531**
   - Status LED should light up
   - Device starts advertising

2. **Open Mobile App**
   - Grant Bluetooth permissions when prompted
   - Grant location permissions (Android only)

3. **Scan for Devices**
   - Tap "Start Scanning" button
   - Your device should appear in the list
   - Note the signal strength (RSSI)

4. **Control the Buzzer**
   - Tap on your device
   - Tap "Play Buzzer" button
   - Buzzer should activate!
   - Try other commands (Stop, Enable, Disable)

5. **View History**
   - Navigate to "History" tab
   - See all commands you sent
   - Check success/failure status

## Troubleshooting

### Device Not Found
- ✅ Check DA14531 is powered on
- ✅ Verify firmware is flashed correctly
- ✅ Ensure Bluetooth is enabled on phone
- ✅ Grant all required permissions

### Buzzer Not Working
- ✅ Check buzzer connections (GPIO P0_6)
- ✅ Verify buzzer polarity (active high)
- ✅ Try "Enable Buzzer" command first
- ✅ Check status LED blinks on command

### App Crashes
- ✅ Reinstall app dependencies: `rm -rf node_modules && npm install`
- ✅ Clear Metro cache: `npm start -- --reset-cache`
- ✅ Check app logs for errors

### Build Errors
- ✅ Verify SDK_ROOT path is correct
- ✅ Check toolchain is installed: `arm-none-eabi-gcc --version`
- ✅ Ensure all dependencies are installed
- ✅ See detailed troubleshooting in [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md)

## What's Next?

### Customize Your Project
- Change device name in firmware (default: "BZR1")
- Adjust buzzer patterns in `user_barebone_beacon.c`
- Customize app theme and colors
- Add custom commands

### Learn More
- 📚 [Protocol Specification](docs/PROTOCOL.md) - Understand the beacon protocol
- 🔧 [Firmware Setup](docs/FIRMWARE_SETUP.md) - Detailed firmware guide
- 📱 [App Setup](docs/APP_SETUP.md) - Detailed app development guide
- 🏗️ [Build Guide](docs/BUILD_GUIDE.md) - Complete build instructions

### Contribute
- Report bugs or request features via [GitHub Issues](https://github.com/HumYhaiJung/ble-beacon-buzzer-control/issues)
- Submit improvements via Pull Requests
- Check [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines

## Hardware Setup Details

### GPIO Connections
| Function     | Port | Pin | Notes              |
|--------------|------|-----|-------------------|
| Buzzer       | P0   | 6   | Active high       |
| Power Sense  | P0   | 7   | Pull-up, optional |
| Status LED   | P0   | 9   | Active high       |

### Buzzer Connection
```
DA14531 P0_6 --> Buzzer (+)
GND          --> Buzzer (-)
```

Or use a transistor for higher current buzzers:
```
DA14531 P0_6 --> 1kΩ resistor --> Transistor Base
                  Transistor Collector --> Buzzer (+)
                  Transistor Emitter --> GND
                  Buzzer (-) --> VCC
```

## Commands Reference

| Command | Code | Description                |
|---------|------|----------------------------|
| PLAY    | 0x01 | Activate buzzer           |
| STOP    | 0x00 | Stop buzzer               |
| ENABLE  | 0x02 | Enable buzzer control     |
| DISABLE | 0x03 | Disable buzzer control    |
| STATUS  | 0xFF | Request device status     |

## Support

- 📖 Documentation: See `docs/` folder
- 🐛 Issues: [GitHub Issues](https://github.com/HumYhaiJung/ble-beacon-buzzer-control/issues)
- 💬 Discussions: [GitHub Discussions](https://github.com/HumYhaiJung/ble-beacon-buzzer-control/discussions)

---

**Happy Building! 🎉**

If you found this project helpful, please consider giving it a ⭐ on GitHub!
