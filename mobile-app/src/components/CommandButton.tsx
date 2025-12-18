import React from 'react';
import { TouchableOpacity, Text, StyleSheet, ActivityIndicator } from 'react-native';
import { CommandCode } from '../types';

interface CommandButtonProps {
  command: CommandCode;
  label: string;
  onPress: () => void;
  disabled?: boolean;
  loading?: boolean;
  variant?: 'primary' | 'secondary' | 'danger' | 'success';
}

export const CommandButton: React.FC<CommandButtonProps> = ({
  command,
  label,
  onPress,
  disabled = false,
  loading = false,
  variant = 'primary',
}) => {
  const getButtonStyle = () => {
    const baseStyle = [styles.button];
    
    if (disabled) {
      baseStyle.push(styles.buttonDisabled);
    } else {
      switch (variant) {
        case 'primary':
          baseStyle.push(styles.buttonPrimary);
          break;
        case 'secondary':
          baseStyle.push(styles.buttonSecondary);
          break;
        case 'danger':
          baseStyle.push(styles.buttonDanger);
          break;
        case 'success':
          baseStyle.push(styles.buttonSuccess);
          break;
      }
    }
    
    return baseStyle;
  };

  const getTextStyle = () => {
    return [styles.buttonText, disabled && styles.buttonTextDisabled];
  };

  return (
    <TouchableOpacity
      style={getButtonStyle()}
      onPress={onPress}
      disabled={disabled || loading}
      activeOpacity={0.7}
    >
      {loading ? (
        <ActivityIndicator color="#FFFFFF" />
      ) : (
        <Text style={getTextStyle()}>{label}</Text>
      )}
    </TouchableOpacity>
  );
};

const styles = StyleSheet.create({
  button: {
    paddingVertical: 16,
    paddingHorizontal: 24,
    borderRadius: 12,
    alignItems: 'center',
    justifyContent: 'center',
    minHeight: 56,
    marginVertical: 8,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  buttonPrimary: {
    backgroundColor: '#2196F3',
  },
  buttonSecondary: {
    backgroundColor: '#9E9E9E',
  },
  buttonDanger: {
    backgroundColor: '#F44336',
  },
  buttonSuccess: {
    backgroundColor: '#4CAF50',
  },
  buttonDisabled: {
    backgroundColor: '#E0E0E0',
  },
  buttonText: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#FFFFFF',
  },
  buttonTextDisabled: {
    color: '#BDBDBD',
  },
});
