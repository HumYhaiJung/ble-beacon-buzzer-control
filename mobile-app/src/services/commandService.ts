import { BleDevice, Command, CommandCode, BEACON_SERVICE_UUID } from '../types';
import { bleService } from './bleService';

class CommandService {
  /**
   * Simple base64 encoding for React Native
   */
  private toBase64(bytes: Uint8Array): string {
    const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
    let result = '';
    let i = 0;
    
    while (i < bytes.length) {
      const a = bytes[i++];
      const b = i < bytes.length ? bytes[i++] : 0;
      const c = i < bytes.length ? bytes[i++] : 0;
      
      const bitmap = (a << 16) | (b << 8) | c;
      
      result += chars.charAt((bitmap >> 18) & 63) +
                chars.charAt((bitmap >> 12) & 63) +
                (i - 2 < bytes.length ? chars.charAt((bitmap >> 6) & 63) : '=') +
                (i - 1 < bytes.length ? chars.charAt(bitmap & 63) : '=');
    }
    
    return result;
  }
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
      console.log(`[CommandService] Connecting to device ${deviceId}`);
      
      // Connect to the device
      const connectedDevice = await bleService.connectToDevice(deviceId);
      console.log(`[CommandService] Connected successfully`);
      
      // Get SWT-T1 service and characteristic
      const services = await connectedDevice.services();
      console.log(`[CommandService] Discovered ${services.length} services:`);
      services.forEach(service => {
        console.log(`  - Service UUID: ${service.uuid}`);
      });
      
      console.log(`[CommandService] Looking for UUID: ${BEACON_SERVICE_UUID} (both 16-bit and full format)`);
      
      // Find SWT-T1 service with more flexible matching
      let swtService = services.find(s => {
        const serviceUuid = s.uuid.toLowerCase().replace(/-/g, '');
        const targetUuid = BEACON_SERVICE_UUID.toLowerCase();
        
        console.log(`[CommandService] Comparing: ${serviceUuid} vs ${targetUuid}`);
        
        // Direct match for 16-bit UUID
        if (serviceUuid === targetUuid || 
            serviceUuid === `0000${targetUuid}00001000800000805f9b34fb` || // Full 16-bit UUID format
            s.uuid.toLowerCase() === `0000${targetUuid}-0000-1000-8000-00805f9b34fb`) {
          console.log(`[CommandService] Found direct UUID match!`);
          return true;
        }
        
        // Match custom FFF0 service
        if (serviceUuid.includes('fff0')) {
          console.log(`[CommandService] Found FFF0 service!`);
          return true;
        }
        
        return false;
      });
      
      // Skip GAP service (contains device name/appearance characteristics)
      if (swtService && swtService.uuid.toLowerCase().includes('1800')) {
        swtService = undefined;
      }
      
      // If still not found, look for any non-standard service (skip GAP, GATT, etc.)
      if (!swtService) {
        console.log(`[CommandService] Looking for custom services (non-standard UUIDs)`);
        swtService = services.find(s => {
          const uuid = s.uuid.toLowerCase();
          // Skip standard services
          if (uuid.includes('1800') || // GAP
              uuid.includes('1801') || // GATT 
              uuid.includes('180a') || // Device Information
              uuid.includes('180f') || // Battery
              uuid.endsWith('-0000-1000-8000-00805f9b34fb')) { // Standard Bluetooth UUIDs
            return false;
          }
          return true;
        });
      }
      
      // If still not found, try writing to ANY writable characteristic
      if (!swtService) {
        console.log(`[CommandService] No custom service found. Checking ALL services for writable characteristics.`);
        
        for (const service of services) {
          console.log(`[CommandService] Thoroughly checking service ${service.uuid}`);
          try {
            const characteristics = await service.characteristics();
            console.log(`[CommandService] Service ${service.uuid} has ${characteristics.length} characteristics:`);
            
            let hasWritable = false;
            characteristics.forEach(char => {
              const isWritable = char.isWritableWithResponse || char.isWritableWithoutResponse;
              console.log(`  - ${char.uuid}: R:${char.isReadable}, W:${char.isWritableWithResponse}, WNR:${char.isWritableWithoutResponse}, N:${char.isNotifiable} ${isWritable ? '✓ WRITABLE' : '✗ READ-only'}`);
              if (isWritable) hasWritable = true;
            });
            
            if (hasWritable) {
              console.log(`[CommandService] Found writable characteristics in service: ${service.uuid}`);
              swtService = service;
              break;
            } else {
              console.log(`[CommandService] No writable characteristics in ${service.uuid}`);
            }
          } catch (e) {
            console.log(`[CommandService] Error checking characteristics for ${service.uuid}:`, e);
          }
        }
        
      // If still not found, try the GATT service (sometimes has writable characteristics)
      if (!swtService) {
        console.log(`[CommandService] Trying GATT service (0x1801) which may have writable Service Changed characteristic`);
        
        const gattService = services.find(s => s.uuid.toLowerCase().includes('1801'));
        if (gattService) {
          try {
            const characteristics = await gattService.characteristics();
            console.log(`[CommandService] GATT service characteristics:`);
            
            for (const char of characteristics) {
              console.log(`  - ${char.uuid}: R:${char.isReadable}, W:${char.isWritableWithResponse}, WNR:${char.isWritableWithoutResponse}, N:${char.isNotifiable}, I:${char.isIndicatable}`);
              
              // Try the Service Changed characteristic (0x2A05) - sometimes supports indications
              if (char.uuid.toLowerCase().includes('2a05')) {
                console.log(`[CommandService] Found Service Changed characteristic - trying to use it`);
                swtService = gattService;
                break;
              }
            }
          } catch (e) {
            console.log(`[CommandService] Error checking GATT service:`, e);
          }
        }
      }
      
      // If still not found, check all services for ANY characteristic with notification/indication
      if (!swtService) {
        console.log(`[CommandService] Looking for characteristics with notification/indication capabilities`);
        
        for (const service of services) {
          try {
            const characteristics = await service.characteristics();
            for (const char of characteristics) {
              if (char.isNotifiable || char.isIndicatable) {
                console.log(`[CommandService] Found notifiable/indicatable characteristic: ${char.uuid} in service ${service.uuid}`);
                swtService = service;
                break;
              }
            }
            if (swtService) break;
          } catch (e) {
            console.log(`[CommandService] Error checking service ${service.uuid}:`, e);
          }
        }
      }
      }
      
      if (!swtService) {
        bleService.disconnectDevice(deviceId);
        throw new Error(`FIRMWARE ISSUE: No writable characteristics found in any service. The custom service (0xFFF0) was not created by firmware. Available services: ${services.map(s => s.uuid).join(', ')}`);
      }
      
      console.log(`[CommandService] Using service: ${swtService.uuid}`);
      
      // Get characteristics and find truly writable ones
      const characteristics = await swtService.characteristics();
      console.log(`[CommandService] Found ${characteristics.length} characteristics in selected service:`);
      characteristics.forEach(char => {
        console.log(`  - Char UUID: ${char.uuid}, Properties: Write:${char.isWritableWithResponse}, WriteNoResp:${char.isWritableWithoutResponse}, Read:${char.isReadable}, Notify:${char.isNotifiable}`);
      });
      
      // Prefer write-with-response (firmware handles that reliably), fallback to without-response
      let writeChar = characteristics.find(c => c.isWritableWithResponse);
      if (!writeChar) {
        writeChar = characteristics.find(c => c.isWritableWithoutResponse);
      }
      
      if (!writeChar) {
        throw new Error(`FIRMWARE ISSUE: Selected service has no writable characteristics. This should not happen if custom service was created properly. Characteristics: ${characteristics.map(c => `${c.uuid} (R:${c.isReadable},W:${c.isWritableWithResponse},WNR:${c.isWritableWithoutResponse})`).join(', ')}`);
      }
      
      console.log(`[CommandService] Using writable characteristic: ${writeChar.uuid}`);
      
      // Send command as single byte - use custom base64 encoding
      const commandByte = new Uint8Array([command]);
      const base64Command = this.toBase64(commandByte);
      
      console.log(`[CommandService] Sending command ${command} (0x${command.toString(16)}) as base64: ${base64Command}`);
      
      // Use the appropriate write method based on characteristic properties
      if (writeChar.isWritableWithoutResponse) {
        await writeChar.writeWithoutResponse(base64Command);
        console.log(`[CommandService] Write without response completed successfully`);
      } else if (writeChar.isWritableWithResponse) {
        await writeChar.writeWithResponse(base64Command);
        console.log(`[CommandService] Write with response completed successfully`);
      }
      
      // Disconnect after command
      setTimeout(() => {
        bleService.disconnectDevice(deviceId);
        console.log(`[CommandService] Disconnected from device`);
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
