import { BleDevice, Command, CommandCode, BEACON_SERVICE_UUID } from '../types';
import { bleService } from './bleService';

class CommandService {
  /**
   * Send a command to a beacon device via GATT write
   */
  async sendCommand(
    device: BleDevice,
    command: CommandCode
  ): Promise<Command> {
    try {
      // Validate command
      if (!this.isValidCommand(command)) {
        throw new Error('Invalid command code');
      }

      // Create command object with random ID
      const cmd: Command = {
        id: `${Date.now()}-${Math.random().toString(36).substring(2, 9)}`,
        deviceId: device.id,
        deviceName: device.name,
        command: command,
        timestamp: new Date(),
        success: false,
      };

      // Connect to device and send GATT write command
      await this.sendGattCommand(device.id, command);

      // Update command as successful
      cmd.success = true;
      cmd.response = `Command ${CommandCode[command]} sent successfully`;

      return cmd;
    } catch (error) {
      const cmd: Command = {
        id: `${Date.now()}-${Math.random().toString(36).substring(2, 9)}`,
        deviceId: device.id,
        deviceName: device.name,
        command: command,
        timestamp: new Date(),
        success: false,
        response: `Error: ${error instanceof Error ? error.message : 'Unknown error'}`,
      };
      throw cmd;
    }
  }

  /**
   * Validate command code
   */
  private isValidCommand(command: CommandCode): boolean {
    return [
      CommandCode.STOP,
      CommandCode.PLAY,
      CommandCode.ENABLE,
      CommandCode.DISABLE,
      CommandCode.STATUS,
    ].includes(command);
  }

  /**
   * Send GATT command to connected device
   */
  private async sendGattCommand(deviceId: string, command: CommandCode): Promise<void> {
    try {
      // Connect to the device
      const connectedDevice = await bleService.connectToDevice(deviceId);
      
      // Get SWT-T1 service and characteristic
      const services = await connectedDevice.services();
      console.log('Discovered services:', services.map(s => s.uuid));
      const swtService = services.find(s => 
        s.uuid.toLowerCase() === BEACON_SERVICE_UUID.toLowerCase() ||
        s.uuid.replace(/-/g, '').toLowerCase() === BEACON_SERVICE_UUID.replace(/-/g, '').toLowerCase()
      );
      
      if (!swtService) {
         bleService.disconnectDevice(deviceId);
        throw new Error('SWT-T1 service not found');
      }
      
      const characteristics = await swtService.characteristics();
      const writeChar = characteristics.find(c => c.isWritableWithResponse || c.isWritableWithoutResponse);
      
      if (!writeChar) {
        throw new Error('Write characteristic not found');
      }
      
      // Send command as single byte
      const commandByte = new Uint8Array([command]);
      const base64Command = btoa(String.fromCharCode(...commandByte));
      
      await writeChar.writeWithResponse(base64Command);
      
      // Disconnect after command
      setTimeout(() => {
        bleService.disconnectDevice(deviceId);
      }, 1000);
      
    } catch (error) {
      console.error('GATT command error:', error);
      throw error;
    }
  }

  /**
   * Build beacon payload for command
   */
  buildCommandPayload(
    deviceName: string,
    command: CommandCode,
    timestamp: number
  ): Uint8Array {
    const payload = new Uint8Array(9);
    
    // Device name (4 bytes)
    const nameBytes = new TextEncoder().encode(deviceName.padEnd(4, ' ').substring(0, 4));
    payload.set(nameBytes, 0);
    
    // Command (1 byte)
    payload[4] = command;
    
    // Timestamp (4 bytes, little-endian) - use unsigned right shift to handle large numbers
    payload[5] = (timestamp >>> 0) & 0xFF;
    payload[6] = (timestamp >>> 8) & 0xFF;
    payload[7] = (timestamp >>> 16) & 0xFF;
    payload[8] = (timestamp >>> 24) & 0xFF;
    
    return payload;
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

export const commandService = new CommandService();
