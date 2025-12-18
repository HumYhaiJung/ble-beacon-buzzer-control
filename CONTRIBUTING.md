# Contributing to BLE Beacon Buzzer Control

Thank you for your interest in contributing to this project! We welcome contributions from the community.

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue on GitHub with:
- A clear, descriptive title
- Steps to reproduce the bug
- Expected behavior
- Actual behavior
- Your environment (OS, SDK version, device, etc.)
- Screenshots or logs if applicable

### Suggesting Enhancements

We welcome feature requests! Please create an issue with:
- A clear description of the feature
- Use cases and benefits
- Possible implementation approach (optional)

### Pull Requests

1. **Fork the repository**
   ```bash
   git clone https://github.com/HumYhaiJung/ble-beacon-buzzer-control.git
   cd ble-beacon-buzzer-control
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow the existing code style
   - Add comments where necessary
   - Update documentation if needed

4. **Test your changes**
   - For firmware: Build and test on actual hardware
   - For mobile app: Test on both iOS and Android
   - Ensure all existing functionality still works

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add feature: description of your changes"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Provide a clear description of changes
   - Reference any related issues
   - Include screenshots for UI changes

## Code Style Guidelines

### Firmware (C)
- Use 4-space indentation
- Follow Dialog SDK coding conventions
- Add Doxygen-style comments for functions
- Keep functions small and focused
- Use meaningful variable names

Example:
```c
/**
 * @brief Initialize the beacon module
 * @param[in] device_name  Device name (4 bytes)
 * @return void
 */
void beacon_init(const uint8_t *device_name) {
    // Implementation
}
```

### Mobile App (TypeScript)
- Use 2-space indentation
- Follow ESLint and Prettier rules
- Use TypeScript types consistently
- Use functional components with hooks
- Add JSDoc comments for complex functions

Example:
```typescript
/**
 * Send a command to the beacon device
 * @param device - Target device
 * @param command - Command code to send
 * @returns Promise with command result
 */
async sendCommand(device: BleDevice, command: CommandCode): Promise<Command> {
  // Implementation
}
```

## Documentation

When adding new features, please update:
- Code comments
- README.md (if applicable)
- Relevant documentation files in `docs/`
- CHANGELOG.md

## Testing

### Firmware Testing
- Build successfully without warnings
- Test on actual DA14531 hardware
- Verify all commands work correctly
- Check power consumption if relevant

### Mobile App Testing
- Test on both iOS and Android
- Test on different device sizes
- Verify BLE functionality
- Check UI responsiveness
- Test error handling

## Commit Message Guidelines

Use clear, descriptive commit messages:
- Start with a verb (Add, Fix, Update, Remove, etc.)
- Keep the first line under 50 characters
- Add detailed description if needed

Good examples:
```
Add replay attack prevention using timestamps
Fix BLE scanning on Android 12+
Update documentation for firmware setup
Remove unused dependencies
```

## Code Review Process

1. A maintainer will review your pull request
2. They may request changes or ask questions
3. Make requested changes and push to your branch
4. Once approved, your PR will be merged

## Questions?

If you have questions, feel free to:
- Open an issue for discussion
- Contact the maintainers
- Check existing issues and documentation

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (Boost Software License).

Thank you for contributing! 🎉
