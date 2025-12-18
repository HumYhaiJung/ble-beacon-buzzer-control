import React from 'react';
import { View, Text, StyleSheet } from 'react-native';

interface StatusIndicatorProps {
  status: 'connected' | 'disconnected' | 'scanning' | 'idle';
  signalStrength?: number;
}

export const StatusIndicator: React.FC<StatusIndicatorProps> = ({
  status,
  signalStrength,
}) => {
  const getStatusColor = () => {
    switch (status) {
      case 'connected':
        return '#4CAF50';
      case 'scanning':
        return '#2196F3';
      case 'disconnected':
        return '#F44336';
      case 'idle':
        return '#9E9E9E';
      default:
        return '#9E9E9E';
    }
  };

  const getStatusText = () => {
    switch (status) {
      case 'connected':
        return 'Connected';
      case 'scanning':
        return 'Scanning...';
      case 'disconnected':
        return 'Disconnected';
      case 'idle':
        return 'Idle';
      default:
        return 'Unknown';
    }
  };

  const getSignalStrengthText = () => {
    if (signalStrength === undefined) return '';
    
    if (signalStrength >= -60) return 'Excellent';
    if (signalStrength >= -70) return 'Good';
    if (signalStrength >= -80) return 'Fair';
    return 'Weak';
  };

  return (
    <View style={styles.container}>
      <View style={[styles.statusDot, { backgroundColor: getStatusColor() }]} />
      <Text style={styles.statusText}>{getStatusText()}</Text>
      {signalStrength !== undefined && (
        <Text style={styles.signalText}>• {getSignalStrengthText()}</Text>
      )}
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 8,
    paddingHorizontal: 12,
    backgroundColor: '#F5F5F5',
    borderRadius: 20,
  },
  statusDot: {
    width: 10,
    height: 10,
    borderRadius: 5,
    marginRight: 8,
  },
  statusText: {
    fontSize: 14,
    fontWeight: '600',
    color: '#424242',
  },
  signalText: {
    fontSize: 12,
    color: '#757575',
    marginLeft: 8,
  },
});
