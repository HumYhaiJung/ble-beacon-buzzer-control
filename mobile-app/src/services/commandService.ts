import { BleDevice, Command, CommandCode, BEACON_SERVICE_UUID } from '../types';

class CommandService {
  /**
   * Send a command to a beacon device
   * Note: In beacon mode, we don't actually "send" commands in the traditional sense.
   * Instead, we would advertise our own beacon with the command.
   * This is a simplified implementation.
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

      // In a real implementation, we would:
      // 1. Start advertising our own beacon with the command
      // 2. Wait for the device to respond with updated status
      // 3. Parse the response and update command.success
      
      // For now, simulate sending the command
      await this.simulateCommandSend(device, command);

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
   * Simulate command send (placeholder for actual beacon advertising)
   */
  private async simulateCommandSend(
    device: BleDevice,
    command: CommandCode
  ): Promise<void> {
    // In a real implementation, this would:
    // 1. Configure our device to advertise as a beacon
    // 2. Set service data with our command payload
    // 3. Start advertising for a short period
    // 4. Stop advertising
    
    // Simulate network delay
    await new Promise((resolve) => setTimeout(resolve, 500));

    // For now, just log the command
    console.log(`Sending command ${CommandCode[command]} to device ${device.name}`);
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
    
    // Timestamp (4 bytes, little-endian)
    payload[5] = timestamp & 0xFF;
    payload[6] = (timestamp >> 8) & 0xFF;
    payload[7] = (timestamp >> 16) & 0xFF;
    payload[8] = (timestamp >> 24) & 0xFF;
    
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
