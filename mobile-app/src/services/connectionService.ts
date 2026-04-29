/**
 * Connection-based command service for SWT-T1 beacons
 * Uses connection events to trigger buzzer instead of GATT writes
 */

import { bleService } from './bleService';
import { CommandCode } from '../types';

class ConnectionService {
  /**
   * Send command by connecting to device (connection event triggers buzzer)
   */
  async sendCommand(deviceId: string, command: CommandCode): Promise<void> {
    console.log(`[ConnectionService] Sending ${this.getCommandLabel(command)} command via connection`);
    
    try {
      // Connect to the device - this triggers buzzer in firmware
      console.log(`[ConnectionService] Connecting to device ${deviceId}`);
      const connectedDevice = await bleService.connectToDevice(deviceId);
      console.log(`[ConnectionService] Connected successfully - buzzer triggered`);
      
      // Wait a moment for the buzzer to play
      await new Promise(resolve => setTimeout(resolve, 1000));
      
      // Disconnect
      await bleService.disconnectDevice(deviceId);
      console.log(`[ConnectionService] Disconnected - command completed`);
      
    } catch (error) {
      console.error(`[ConnectionService] Command failed:`, error);
      const message = error instanceof Error ? error.message : String(error);
      throw new Error(`Failed to send ${this.getCommandLabel(command)} command: ${message}`);
    }
  }

  /**
   * Get command label
   */
  getCommandLabel(command: CommandCode): string {
    switch (command) {
      case CommandCode.STOP:
        return 'Stop';
      case CommandCode.PLAY:
        return 'Play';
      case CommandCode.ENABLE:
        return 'Enable';
      case CommandCode.DISABLE:
        return 'Disable';
      case CommandCode.STATUS:
        return 'Status';
      default:
        return 'Unknown';
    }
  }
}

export const connectionService = new ConnectionService();
// Backwards-compatible alias for existing imports
export const commandService = connectionService;