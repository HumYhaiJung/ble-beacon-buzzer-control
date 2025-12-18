import { BleManager, Device, Subscription } from 'react-native-ble-plx';
import { BleDevice, BeaconPayload, BEACON_SERVICE_UUID, CommandCode } from '../types';
import { Platform, PermissionsAndroid, NativeModules } from 'react-native';

class BleService {
  private manager: BleManager | null = null;
  private scanSubscription: Subscription | null = null;

  constructor() {
    // Check if BLE native module is available (not in Expo Go or web environment)
    if (NativeModules.BlePlx) {
      try {
        this.manager = new BleManager();
      } catch (error) {
        console.error('Failed to initialize BLE Manager:', error);
        throw new Error(
          'BLE module is not available. Make sure you are running on a device or emulator with native support. ' +
          'Do NOT use Expo Go. Use "npx expo run:android" or "npx expo run:ios" instead.'
        );
      }
    } else {
      throw new Error(
        'BLE native module not available. This app requires a native build. ' +
        'Run "npx expo run:android" or "npx expo run:ios" instead of "expo start"'
      );
    }
  }

  /**
   * Request Bluetooth permissions (Android)
   */
  async requestPermissions(): Promise<boolean> {
    if (Platform.OS === 'android') {
      if (Platform.Version >= 31) {
        // Android 12+
        const permissionsRaw = [
          (PermissionsAndroid as any).PERMISSIONS.BLUETOOTH_SCAN,
          (PermissionsAndroid as any).PERMISSIONS.BLUETOOTH_CONNECT,
          (PermissionsAndroid as any).PERMISSIONS.ACCESS_FINE_LOCATION,
        ];
        const permissions = permissionsRaw.filter(
          (perm) => perm !== undefined && perm !== null
        );
        console.log('Requesting permissions (Android 12+):', permissions);
        if (permissions.length !== permissionsRaw.length) {
          console.error('One or more permissions are null or undefined:', permissionsRaw);
        }
        const granted = await (PermissionsAndroid as any).requestMultiple(permissions);
        return (
          granted['android.permission.BLUETOOTH_SCAN'] === (PermissionsAndroid as any).RESULTS.GRANTED &&
          granted['android.permission.BLUETOOTH_CONNECT'] === (PermissionsAndroid as any).RESULTS.GRANTED &&
          granted['android.permission.ACCESS_FINE_LOCATION'] === (PermissionsAndroid as any).RESULTS.GRANTED
        );
      } else {
        // Android 11 and below
        const perms = (PermissionsAndroid as any).PERMISSIONS;
        const permissions: string[] = [];
        if (perms.ACCESS_FINE_LOCATION) permissions.push(perms.ACCESS_FINE_LOCATION);
        if (perms.BLUETOOTH) permissions.push(perms.BLUETOOTH);
        if (perms.BLUETOOTH_ADMIN) permissions.push(perms.BLUETOOTH_ADMIN);
        console.log('Requesting permissions (Android <=11):', permissions);
        if (permissions.length === 0) {
          console.error('No permissions available to request', perms);
        }
        const granted = await (PermissionsAndroid as any).requestMultiple(permissions);
        return (
          granted['android.permission.ACCESS_FINE_LOCATION'] === (PermissionsAndroid as any).RESULTS.GRANTED
        );
      }
    }
    return true; // iOS permissions handled via Info.plist
  }

  /**
   * Check if Bluetooth is enabled
   */
  async checkBluetoothState(): Promise<boolean> {
    if (!this.manager) {
      throw new Error('BLE Manager not initialized');
    }
    const state = await this.manager.state();
    return state === 'PoweredOn';
  }

  /**
   * Parse beacon payload from service data
   */
  private parseBeaconPayload(serviceData: string): BeaconPayload | null {
    try {
      // Service data format: hex string
      // Expected: [device_name:4bytes][command:1byte][timestamp:4bytes] = 9 bytes = 18 hex chars
      if (serviceData.length < 18) {
        return null;
      }

      const deviceNameHex = serviceData.substring(0, 8);
      // Convert hex to ASCII without using Buffer
      let deviceName = '';
      for (let i = 0; i < deviceNameHex.length; i += 2) {
        deviceName += String.fromCharCode(parseInt(deviceNameHex.substring(i, i + 2), 16));
      }
      
      const command = parseInt(serviceData.substring(8, 10), 16);
      
      const timestampHex = serviceData.substring(10, 18);
      // Use unsigned right shift to handle large numbers
      const timestamp = parseInt(timestampHex, 16) >>> 0;

      return {
        deviceName,
        command,
        timestamp,
      };
    } catch (error) {
      console.error('Error parsing beacon payload:', error);
      return null;
    }
  }

  /**
   * Simple base64 to bytes decoder (no Buffer/atob needed for React Native)
   */
  private base64ToBytes(base64: string): number[] {
    const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const bytes: number[] = [];
    let i = 0;
    
    while (i < base64.length) {
      const a = chars.indexOf(base64[i++]);
      const b = chars.indexOf(base64[i++]);
      const c = chars.indexOf(base64[i++]);
      const d = chars.indexOf(base64[i++]);
      
      if (a >= 0) bytes.push((a << 2) | (b >> 4));
      if (c >= 0) bytes.push(((b & 15) << 4) | (c >> 2));
      if (d >= 0) bytes.push(((c & 3) << 6) | d);
    }
    
    return bytes;
  }

  /**
   * Parse raw advertisement bytes (base64 input) into AD structures
   * Returns map of AD types to array of byte arrays
   */
  private parseRawScanRecord(base64Record: string): Record<number, number[][]> {
    try {
      const bytes = this.base64ToBytes(base64Record);
      const result: Record<number, number[][]> = {};
      let i = 0;
      while (i < bytes.length) {
        const len = bytes[i++];
        if (!len || i + len > bytes.length) break;
        const type = bytes[i++];
        const dataLen = len - 1;
        const data: number[] = [];
        for (let j = 0; j < dataLen; j++) {
          data.push(bytes[i + j]);
        }
        i += dataLen;
        if (!result[type]) result[type] = [];
        result[type].push(data);
      }
      return result;
    } catch (e) {
      console.warn('Failed to parse rawScanRecord:', e);
      return {};
    }
  }

  /**
   * Decode manufacturerData to extract beacon name
   * Expects base64 string, decodes to bytes and extracts first 4 bytes as device name
   */
  private decodeManufacturerData(manufacturerData: string): { deviceName: string } | null {
    try {
      // Decode base64 to bytes manually (no Buffer needed)
      const bytes = this.base64ToBytes(manufacturerData);
      let deviceName = '';
      
      // Extract first 4 bytes as device name (ASCII)
      for (let i = 0; i < Math.min(4, bytes.length); i++) {
        const char = bytes[i];
        if (char >= 32 && char <= 126) {
          // Only include printable ASCII
          deviceName += String.fromCharCode(char);
        }
      }
      
      if (deviceName.length > 0) {
        console.log('Decoded manufacturerData name:', deviceName);
        return { deviceName };
      }
      return null;
    } catch (error) {
      console.error('Error decoding manufacturerData:', error);
      return null;
    }
  }

  /**
   * Start scanning for BLE beacon devices
   */
  async startScanning(
    onDeviceFound: (device: BleDevice) => void,
    onError?: (error: Error) => void
  ): Promise<void> {
    try {
      // Stop any existing scan
      this.stopScanning();

      // Check permissions
      const hasPermission = await this.requestPermissions();
      if (!hasPermission) {
        throw new Error('Bluetooth permissions not granted');
      }

      // Check Bluetooth state
      const isEnabled = await this.checkBluetoothState();
      if (!isEnabled) {
        throw new Error('Bluetooth is not enabled');
      }

      // Start scanning
      if (!this.manager) {
        throw new Error('BLE Manager not initialized');
      }

      console.log('Starting BLE scan with BEACON_SERVICE_UUID =', BEACON_SERVICE_UUID);
      try {
        const mgrState = await this.manager.state();
        console.log('BLE Manager state before scan:', mgrState);
      } catch (e) {
        console.warn('Failed to read BLE manager state:', e);
      }

      this.manager.startDeviceScan(
        null, // Scan for all devices
        { allowDuplicates: true }, // Allow duplicates to get RSSI updates
        (error, device) => {
          if (error) {
            console.error('Scan error:', error);
            if (onError) onError(error);
            return;
          }

          if (!device) return;

          // Log discovered device summary for debugging
          try {
            const sdKeys = device.serviceData ? Object.keys(device.serviceData) : null;
            // console.log('Discovered device:', {
            //   id: device.id,
            //   name:  device.localName || device.name,
            //   rssi: device.rssi,
            //   serviceDataKeys: sdKeys,
            // });
            if (device.serviceData && sdKeys && sdKeys.length > 0) {
              sdKeys.forEach((k) => console.log(`serviceData[${k}] =`, device.serviceData ? device.serviceData[k] : null,device.localName || device.name));
            }
          } catch (e) {
            console.warn('Error logging device info:', e);
          }

          // Try to extract beacon data from service data UUID 28151 (0x6DF7)
          let payload: BeaconPayload | null = null;
          let extractedName: string | undefined;
          let powerStatus: number | undefined;

          if (device.serviceData) {
            // Check for service data with UUID 28151 (0x6DF7) - power status
            const powerServiceData = device.serviceData['6df7'] || device.serviceData['6DF7'];
            if (powerServiceData) {
              // Service data format: power status (1 byte)
              const bytes = this.base64ToBytes(powerServiceData);
              if (bytes.length > 0) {
                powerStatus = bytes[0]; // 0 = power off, 1 = power on
              }
            }

            // Check for main service UUID (128-bit swt-t1 service)
            const serviceUUID = BEACON_SERVICE_UUID.toLowerCase();
            const serviceData = device.serviceData[serviceUUID] || device.serviceData[BEACON_SERVICE_UUID] || device.serviceData[serviceUUID.toUpperCase()];

            if (serviceData) {
              payload = this.parseBeaconPayload(serviceData);
              if (payload?.deviceName) {
                extractedName = payload.deviceName;
              }
            }
          }

          // If still no name, try parsing rawScanRecord AD structures (like nRF Connect)
          try {
            const raw = (device as any).rawScanRecord || (device as any).rawScanRecordBase64 || (device as any).rawScanRecordString;
            if (!extractedName && raw) {
              const adMap = this.parseRawScanRecord(raw);
              // AD type 0x09 = Complete Local Name, 0x08 = Shortened Local Name
              const nameTypes = [0x09, 0x08];
              for (const t of nameTypes) {
                const entries = adMap[t];
                if (entries && entries.length > 0) {
                  // take first occurrence
                  const bytes = entries[0];
                  try {
                    const name = String.fromCharCode(...bytes.filter((b) => b > 0));
                    if (name && name.length) {
                      extractedName = name;
                      break;
                    }
                  } catch (e) {
                    // ignore
                  }
                }
              }

              // AD type 0xFF = Manufacturer Specific Data, may contain name bytes
              if (!extractedName && adMap[0xFF] && adMap[0xFF].length > 0) {
                const mbytes = adMap[0xFF][0];
                // Try to extract ASCII from first 4 bytes
                const possible = mbytes.slice(0, 4).filter((b) => b >= 32 && b <= 126);
                if (possible.length > 0) {
                  extractedName = String.fromCharCode(...possible);
                }
              }
            }
          } catch (e) {
            // ignore parsing errors
          }

          // If no name yet, try to extract from manufacturerData fallback
          if (!extractedName && (device as any).manufacturerData) {
            const mfgData = this.decodeManufacturerData((device as any).manufacturerData);
            if (mfgData?.deviceName) {
              extractedName = mfgData.deviceName;
            }
          }

          // Create device with best available name and power status
          const deviceName = device.localName || device.name || extractedName || 'Unknown';
          const bleDevice: BleDevice = {
            id: device.id,
            name: deviceName,
            rssi: device.rssi || -100,
            serviceData: payload || (powerStatus !== undefined ? { 
              deviceName: deviceName,
              command: powerStatus as CommandCode,
              timestamp: Date.now()
            } : undefined),
            raw: {
              serviceData: device.serviceData,
              manufacturerData: (device as any).manufacturerData,
              localName: device.localName,
              name: device.name,
              rawScanRecord: (device as any).rawScanRecord,
              powerStatus: powerStatus,
            },
            lastSeen: new Date(),
          };
          onDeviceFound(bleDevice);
        }
      );
    } catch (error) {
      console.error('Error starting scan:', error);
      if (onError) onError(error as Error);
      throw error;
    }
  }

  /**
   * Stop scanning
   */
  stopScanning(): void {
    if (this.manager) {
      this.manager.stopDeviceScan();
    }
  }

  /**
   * Connect to a device (for future use - current implementation is beacon-only)
   */
  async connectToDevice(deviceId: string): Promise<Device> {
    if (!this.manager) {
      throw new Error('BLE Manager not initialized');
    }
    try {
      const device = await this.manager.connectToDevice(deviceId);
      await device.discoverAllServicesAndCharacteristics();
      return device;
    } catch (error) {
      console.error('Error connecting to device:', error);
      throw error;
    }
  }

  /**
   * Disconnect from a device
   */
  async disconnectDevice(deviceId: string): Promise<void> {
    try {
      if (!this.manager) {
        throw new Error('BLE Manager not initialized');
      }
      await this.manager.cancelDeviceConnection(deviceId);
    } catch (error) {
      console.error('Error disconnecting device:', error);
      throw error;
    }
  }

  /**
   * Destroy BLE manager
   */
  destroy(): void {
    this.stopScanning();
    if (this.manager) {
      this.manager.destroy();
    }
  }
}

export const bleService = new BleService();
