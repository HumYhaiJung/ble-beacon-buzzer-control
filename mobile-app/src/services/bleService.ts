import { BleManager, Device, Subscription } from 'react-native-ble-plx';
import { BleDevice, BeaconPayload, BEACON_SERVICE_UUID } from '../types';
import { Platform, PermissionsAndroid } from 'react-native';

class BleService {
  private manager: BleManager;
  private scanSubscription: Subscription | null = null;

  constructor() {
    this.manager = new BleManager();
  }

  /**
   * Request Bluetooth permissions (Android)
   */
  async requestPermissions(): Promise<boolean> {
    if (Platform.OS === 'android') {
      if (Platform.Version >= 31) {
        // Android 12+
        const granted = await PermissionsAndroid.requestMultiple([
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        ]);
        return (
          granted['android.permission.BLUETOOTH_SCAN'] === PermissionsAndroid.RESULTS.GRANTED &&
          granted['android.permission.BLUETOOTH_CONNECT'] === PermissionsAndroid.RESULTS.GRANTED &&
          granted['android.permission.ACCESS_FINE_LOCATION'] === PermissionsAndroid.RESULTS.GRANTED
        );
      } else {
        // Android 11 and below
        const granted = await PermissionsAndroid.requestMultiple([
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_ADMIN,
        ]);
        return (
          granted['android.permission.ACCESS_FINE_LOCATION'] === PermissionsAndroid.RESULTS.GRANTED
        );
      }
    }
    return true; // iOS permissions handled via Info.plist
  }

  /**
   * Check if Bluetooth is enabled
   */
  async checkBluetoothState(): Promise<boolean> {
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
      const deviceName = Buffer.from(deviceNameHex, 'hex').toString('ascii');
      
      const command = parseInt(serviceData.substring(8, 10), 16);
      
      const timestampHex = serviceData.substring(10, 18);
      const timestamp = parseInt(timestampHex, 16);

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
   * Start scanning for BLE beacon devices
   */
  async startScanning(
    onDeviceFound: (device: BleDevice) => void,
    onError?: (error: Error) => void
  ): Promise<void> {
    try {
      // Stop any existing scan
      await this.stopScanning();

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
      this.scanSubscription = this.manager.startDeviceScan(
        null, // Scan for all devices
        { allowDuplicates: true }, // Allow duplicates to get RSSI updates
        (error, device) => {
          if (error) {
            console.error('Scan error:', error);
            if (onError) onError(error);
            return;
          }

          if (device && device.serviceData) {
            // Check if device has our beacon service UUID
            const serviceUUID = BEACON_SERVICE_UUID.toLowerCase();
            const serviceData = device.serviceData[serviceUUID];

            if (serviceData) {
              // Parse beacon payload
              const payload = this.parseBeaconPayload(serviceData);

              const bleDevice: BleDevice = {
                id: device.id,
                name: device.name || device.localName || 'Unknown',
                rssi: device.rssi || -100,
                serviceData: payload || undefined,
                lastSeen: new Date(),
              };

              onDeviceFound(bleDevice);
            }
          }
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
  async stopScanning(): Promise<void> {
    if (this.scanSubscription) {
      this.scanSubscription.remove();
      this.scanSubscription = null;
    }
    this.manager.stopDeviceScan();
  }

  /**
   * Connect to a device (for future use - current implementation is beacon-only)
   */
  async connectToDevice(deviceId: string): Promise<Device> {
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
    this.manager.destroy();
  }
}

export const bleService = new BleService();
