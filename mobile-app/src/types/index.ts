/**
 * Type definitions for BLE Beacon Buzzer Control app
 */

export interface BleDevice {
  id: string;
  name: string;
  rssi: number;
  serviceData?: BeaconPayload;
  lastSeen: Date;
  isConnected?: boolean;
}

export interface BeaconPayload {
  deviceName: string;
  command: CommandCode;
  timestamp: number;
}

export enum CommandCode {
  STOP = 0x00,
  PLAY = 0x01,
  ENABLE = 0x02,
  DISABLE = 0x03,
  STATUS = 0xFF,
}

export interface Command {
  id: string;
  deviceId: string;
  deviceName: string;
  command: CommandCode;
  timestamp: Date;
  response?: string;
  success?: boolean;
}

export interface DeviceStatus {
  isPlaying: boolean;
  isEnabled: boolean;
  lastUpdate: Date;
}

export interface AppSettings {
  scanInterval: number;
  autoConnect: boolean;
  theme: 'light' | 'dark';
}

export interface SignalStrengthPoint {
  timestamp: number;
  rssi: number;
}

export const BEACON_SERVICE_UUID = 'AA28';

export const CommandLabels: Record<CommandCode, string> = {
  [CommandCode.STOP]: 'Stop',
  [CommandCode.PLAY]: 'Play',
  [CommandCode.ENABLE]: 'Enable',
  [CommandCode.DISABLE]: 'Disable',
  [CommandCode.STATUS]: 'Status',
};
