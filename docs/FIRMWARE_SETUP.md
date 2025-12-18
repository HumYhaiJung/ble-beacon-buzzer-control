# Dialog DA14531 Firmware Setup Guide

This guide will help you set up the development environment for building and flashing the BLE Beacon Buzzer Control firmware for the Dialog DA14531 SoC.

## Prerequisites

- Windows 10/11, Linux (Ubuntu 20.04+), or macOS
- 4GB RAM minimum (8GB recommended)
- 10GB free disk space

## Software Requirements

### 1. Dialog SmartSnippets Studio

Dialog SmartSnippets Studio is the official IDE for Dialog Semiconductor devices.

**Download:**
- Visit: https://www.dialog-semiconductor.com/support
- Navigate to "Software and Tools"
- Download SmartSnippets Studio v5.0.14 or later

**Installation:**

**Windows:**
```
1. Run the installer: SmartSnippetsStudio-<version>.exe
2. Follow the installation wizard
3. Install to default location: C:\Dialog\SmartSnippetsStudio
4. Allow the installer to install drivers when prompted
```

**Linux:**
```bash
# Extract the archive
tar -xzf SmartSnippetsStudio-<version>.tar.gz
cd SmartSnippetsStudio

# Run the installer
./install.sh

# Add to PATH (optional)
echo 'export PATH=$PATH:/opt/Dialog/SmartSnippetsStudio' >> ~/.bashrc
source ~/.bashrc
```

**macOS:**
```bash
# Mount the DMG
open SmartSnippetsStudio-<version>.dmg

# Drag to Applications folder
# Launch from Applications
```

### 2. Dialog SDK 6.0.14

The SDK contains libraries, drivers, and example projects.

**Download:**
```
1. Visit Dialog support website
2. Navigate to DA14531 product page
3. Download "SDK 6.0.14 for DA14585/586 and DA14531"
```

**Installation:**

**Windows:**
```
1. Extract SDK_6.0.14.zip
2. Recommended location: C:\Dialog\SDK_6.0.14
3. Set environment variable:
   - Variable name: SDK_ROOT
   - Variable value: C:\Dialog\SDK_6.0.14
```

**Linux/macOS:**
```bash
# Extract SDK
unzip SDK_6.0.14.zip -d ~/Dialog/

# Set environment variable
echo 'export SDK_ROOT=~/Dialog/SDK_6.0.14' >> ~/.bashrc
source ~/.bashrc
```

### 3. ARM Toolchain

Choose one of the following:

#### Option A: Keil MDK (Windows only, Recommended)

**Download:**
- Visit: https://www.keil.com/download/product/
- Download MDK-Arm (MDK-Core or MDK-Plus)
- Version 5.34 or later

**Installation:**
```
1. Run the installer
2. Install to default location
3. Install device packs when prompted
4. Register for free license (code size limited) or purchase full license
```

**Configure:**
```
1. Open Keil µVision
2. Project -> Manage -> Pack Installer
3. Search for "Dialog"
4. Install "DA14531" device pack
```

#### Option B: ARM GCC (Cross-platform, Free)

**Download:**
- Visit: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
- Download: GNU Arm Embedded Toolchain (version 9-2020-q2-update or later)

**Installation:**

**Windows:**
```
1. Run the installer
2. Select "Add to PATH"
3. Install to: C:\Program Files (x86)\GNU Arm Embedded Toolchain
```

**Linux:**
```bash
# Download and extract
cd ~/Downloads
tar -xjf gcc-arm-none-eabi-<version>.tar.bz2

# Move to /opt
sudo mv gcc-arm-none-eabi-<version> /opt/

# Add to PATH
echo 'export PATH=$PATH:/opt/gcc-arm-none-eabi-<version>/bin' >> ~/.bashrc
source ~/.bashrc

# Verify installation
arm-none-eabi-gcc --version
```

**macOS:**
```bash
# Using Homebrew
brew install gcc-arm-embedded

# Verify
arm-none-eabi-gcc --version
```

### 4. J-Link Software (for debugging/flashing)

**Download:**
- Visit: https://www.segger.com/downloads/jlink/
- Download J-Link Software and Documentation Pack

**Installation:**
- Follow the installer for your platform
- Drivers will be installed automatically

## Project Setup

### Create New Project from Template

1. **Open SmartSnippets Studio**
   ```
   Launch SmartSnippets Studio from Start Menu or Applications
   ```

2. **Import SDK**
   ```
   File -> Import -> Existing Projects into Workspace
   Browse to: $SDK_ROOT/projects
   Select: ble_examples/ble_app_barebone
   ```

3. **Copy Template**
   ```
   Right-click on project -> Copy
   Right-click in workspace -> Paste
   Rename to: ble_beacon_buzzer_control
   ```

### Add Custom Source Files

1. **Copy firmware files**
   ```
   Copy the following files to project's src/ directory:
   - swt_t1_beacon.h
   - swt_t1_beacon.c
   - user_barebone_beacon.c
   ```

2. **Update project configuration**
   ```
   Right-click project -> Properties -> C/C++ Build -> Settings
   
   Include Paths:
   - Add: ../src
   
   Preprocessor Defines:
   - Add: CFG_BLE_BEACON
   - Add: __DA14531__
   ```

3. **Replace user_barebone.c**
   ```
   Delete or exclude: user_barebone.c
   Add to build: user_barebone_beacon.c
   ```

## Build Configuration

### Keil µVision

1. **Open project**
   ```
   File -> Open Project
   Navigate to: project directory
   Open: ble_beacon_buzzer_control.uvprojx
   ```

2. **Select target**
   ```
   Project -> Select Device for Target
   Choose: Dialog DA14531
   ```

3. **Configure build options**
   ```
   Project -> Options for Target
   
   Target:
   - ARM Compiler: Use default compiler version 5
   
   C/C++:
   - Optimization: -O2
   - Define: __DA14531__, CFG_BLE_BEACON
   - Include Paths: Add SDK paths
   
   Linker:
   - Use Memory Layout from Target Dialog
   ```

### GCC Makefile

1. **Navigate to project directory**
   ```bash
   cd /path/to/ble_beacon_buzzer_control
   ```

2. **Edit Makefile**
   ```makefile
   # Set SDK path
   SDK_ROOT = /path/to/SDK_6.0.14
   
   # Set target
   TARGET = DA14531
   
   # Add source files
   SOURCES += src/swt_t1_beacon.c
   SOURCES += src/user_barebone_beacon.c
   
   # Add include paths
   INCLUDES += -I./src
   
   # Add defines
   DEFINES += -D__DA14531__ -DCFG_BLE_BEACON
   ```

## Build Instructions

### Using Keil

```
1. Open project in Keil µVision
2. Select build target: DA14531
3. Project -> Build Target (F7)
4. Check output window for errors
5. Output: Objects\ble_beacon_buzzer_control.hex
```

### Using GCC

```bash
# Clean previous build
make clean

# Build project
make -j4

# Output: build/ble_beacon_buzzer_control.hex
```

### Using SmartSnippets Studio

```
1. Right-click project in workspace
2. Select "Build Project"
3. View build output in Console
4. Find binary in: Debug/ or Release/
```

## Verify Build

After successful build, you should have:
- `.hex` file (for flashing)
- `.elf` file (for debugging)
- `.bin` file (for OTA updates)

```bash
# Check file size (should be < 48KB for DA14531)
ls -lh *.hex

# Verify hex file integrity
arm-none-eabi-objdump -h *.elf
```

## Troubleshooting

### Build Errors

**Error: "SDK_ROOT not defined"**
```bash
# Set environment variable
export SDK_ROOT=/path/to/SDK_6.0.14
```

**Error: "arm-none-eabi-gcc not found"**
```bash
# Verify GCC installation
which arm-none-eabi-gcc

# Add to PATH if needed
export PATH=$PATH:/path/to/gcc-arm/bin
```

**Error: "Cannot open include file"**
- Check include paths in project settings
- Verify SDK_ROOT points to correct location
- Ensure all SDK files are extracted

### Linker Errors

**Error: "Region 'ROM' overflowed"**
- Code size exceeds flash capacity
- Enable optimization (-O2 or -Os)
- Remove unused features
- Check for large arrays or strings

**Error: "Undefined reference"**
- Missing source file in build
- Missing library in linker settings
- Check function declarations

### Keil-Specific Issues

**Error: "No ULINK device detected"**
- Not an error, just a warning if no debugger connected
- Ignore if only building, not debugging

**Error: "License error"**
- Keil free license has code size limit (32KB)
- Register for free license at keil.com
- Or purchase full license

## Next Steps

Once the firmware is built successfully:
1. Proceed to flashing instructions (see firmware/README.md)
2. Connect debugger for live debugging
3. Test with mobile app

## References

- [Dialog DA14531 Datasheet](https://www.dialog-semiconductor.com/products/da14531)
- [SDK User Manual](https://www.dialog-semiconductor.com/support)
- [SmartSnippets Studio Documentation](https://www.dialog-semiconductor.com/support)
- [ARM Keil MDK Documentation](https://www.keil.com/support)
