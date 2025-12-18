# BLE Beacon Protocol Specification

This document describes the beacon protocol used for buzzer control communication.

## Overview

The system uses BLE (Bluetooth Low Energy) beacon advertising to transmit commands and status information. This allows for connectionless communication, making the system more power-efficient and simpler to implement.

## Service UUID

The beacon uses a custom 16-bit Service UUID:

```
0xAA28
```

This UUID identifies our custom beacon service in the BLE advertising data.

## Advertising Packet Format

The BLE advertising packet follows the standard format with our custom Service Data:

```
[Flags][Service Data]
```

### Flags (3 bytes)
```
02 01 06
```
- Length: 2 bytes
- Type: 0x01 (Flags)
- Value: 0x06 (General Discoverable, BR/EDR not supported)

### Service Data (12 bytes)
```
0E 16 [UUID] [Payload]
```
- Length: 14 bytes (0x0E)
- Type: 0x16 (Service Data - 16-bit UUID)
- UUID: 28 AA (0xAA28 in little-endian)
- Payload: 9 bytes (see below)

## Payload Structure

The beacon payload is 9 bytes and contains the following fields:

```
[Device Name: 4 bytes][Command: 1 byte][Timestamp: 4 bytes]
```

### Device Name (4 bytes)

ASCII characters identifying the device. Examples:
- `BZR1` - Buzzer device 1
- `BZR2` - Buzzer device 2
- `TEST` - Test device

The device name is used to address specific devices when multiple buzzers are present.

### Command (1 byte)

Command code indicating the action to perform:

| Code | Name    | Description                          |
|------|---------|--------------------------------------|
| 0x00 | STOP    | Stop the buzzer                      |
| 0x01 | PLAY    | Play buzzer pattern                  |
| 0x02 | ENABLE  | Enable buzzer (allow play commands)  |
| 0x03 | DISABLE | Disable buzzer (ignore play commands)|
| 0xFF | STATUS  | Request current status               |

### Timestamp (4 bytes)

32-bit unsigned integer representing a timestamp in milliseconds (little-endian format). This field serves two purposes:

1. **Command Identification**: Allows the receiver to distinguish between new and old commands
2. **Replay Attack Prevention**: Prevents malicious replay of old commands

The receiver should:
- Reject commands with timestamps older than the last processed command
- Optionally validate that the timestamp is within a reasonable time window

## Example Packets

### Example 1: Play Buzzer Command

```
Complete advertising packet:
02 01 06 0E 16 28 AA 42 5A 52 31 01 12 34 56 78

Breakdown:
02 01 06           - Flags
0E                 - Service Data length (14 bytes)
16                 - Service Data type
28 AA              - Service UUID (0xAA28)
42 5A 52 31        - Device name: "BZR1" (ASCII)
01                 - Command: PLAY
12 34 56 78        - Timestamp: 0x78563412 (little-endian)
```

### Example 2: Stop Buzzer Command

```
02 01 06 0E 16 28 AA 42 5A 52 31 00 34 56 78 9A

Breakdown:
02 01 06           - Flags
0E 16 28 AA        - Service Data header
42 5A 52 31        - Device name: "BZR1"
00                 - Command: STOP
34 56 78 9A        - Timestamp: 0x9A785634
```

### Example 3: Enable Buzzer Command

```
02 01 06 0E 16 28 AA 54 45 53 54 02 AB CD EF 01

Breakdown:
02 01 06           - Flags
0E 16 28 AA        - Service Data header
54 45 53 54        - Device name: "TEST"
02                 - Command: ENABLE
AB CD EF 01        - Timestamp: 0x01EFCDAB
```

### Example 4: Status Request

```
02 01 06 0E 16 28 AA 42 5A 52 32 FF 00 00 00 00

Breakdown:
02 01 06           - Flags
0E 16 28 AA        - Service Data header
42 5A 52 32        - Device name: "BZR2"
FF                 - Command: STATUS
00 00 00 00        - Timestamp: 0
```

## Implementation Notes

### Transmitter (Mobile App)

1. Build the advertising packet with the correct format
2. Set the Service UUID to 0xAA28
3. Include the device name, command, and current timestamp
4. Start advertising for a short duration (e.g., 1-2 seconds)
5. Stop advertising after command is sent

### Receiver (Firmware)

1. Scan for BLE advertising packets
2. Filter packets with Service UUID 0xAA28
3. Parse the payload to extract device name, command, and timestamp
4. Verify the device name matches the local device
5. Check timestamp to prevent replay attacks
6. Execute the command
7. Optionally respond with status via own beacon advertising

## Security Considerations

### Timestamp Validation

The timestamp field provides basic protection against replay attacks:

```c
// Pseudo-code for timestamp validation
if (received_timestamp <= last_processed_timestamp) {
    // Reject: old command
    return ERROR_OLD_COMMAND;
}

if (received_timestamp > (current_time + MAX_TIME_DRIFT)) {
    // Reject: timestamp too far in the future
    return ERROR_INVALID_TIMESTAMP;
}

// Accept command
last_processed_timestamp = received_timestamp;
```

### Recommendations

For enhanced security, consider:

1. **Authentication**: Add HMAC or signature field
2. **Encryption**: Encrypt the payload
3. **Nonce**: Add random nonce to prevent replay
4. **Pairing**: Implement device pairing mechanism
5. **Range Limiting**: Use RSSI to limit effective range

## Testing

### Using nRF Connect App

1. Open nRF Connect on mobile device
2. Enable advertising
3. Add custom advertising data:
   ```
   02 01 06 0E 16 28 AA 42 5A 52 31 01 00 00 00 00
   ```
4. Start advertising
5. Observe response from buzzer device

### Using Command Line (Linux/Mac)

```bash
# Using hcitool (requires root/sudo)
sudo hcitool -i hci0 cmd 0x08 0x0008 \
  1E 02 01 06 0E 16 28 AA 42 5A 52 31 01 00 00 00 00 \
  00 00 00 00 00 00 00 00 00 00 00 00 00
```

## Packet Size

Total advertising packet size:
- Flags: 3 bytes
- Service Data header: 4 bytes
- Payload: 9 bytes
- **Total: 16 bytes**

This fits well within the BLE advertising packet size limit of 31 bytes, leaving room for additional data if needed in future versions.

## Version History

- **v1.0** (2024): Initial protocol specification
  - Basic command structure
  - Timestamp-based replay prevention
  - 4-byte device name addressing

## Future Enhancements

Potential improvements for future versions:

1. Add payload version field
2. Support for extended advertising (BLE 5.0)
3. Encryption and authentication
4. Multi-command batching
5. Acknowledgment mechanism
6. Device grouping support
