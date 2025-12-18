import React from 'react';
import { View, Text, TouchableOpacity, StyleSheet } from 'react-native';
import { BleDevice } from '../types';

interface DeviceCardProps {
  device: BleDevice;
  isFavorite?: boolean;
  onPress: () => void;
  onFavoriteToggle?: () => void;
}

export const DeviceCard: React.FC<DeviceCardProps> = ({
  device,
  isFavorite = false,
  onPress,
  onFavoriteToggle,
}) => {
  const getSignalStrengthBars = (rssi: number): number => {
    if (rssi >= -50) return 5;
    if (rssi >= -60) return 4;
    if (rssi >= -70) return 3;
    if (rssi >= -80) return 2;
    if (rssi >= -90) return 1;
    return 0;
  };

  const getSignalColor = (rssi: number): string => {
    if (rssi >= -60) return '#4CAF50';
    if (rssi >= -75) return '#FFC107';
    return '#F44336';
  };

  const bars = getSignalStrengthBars(device.rssi);
  const signalColor = getSignalColor(device.rssi);

  return (
    <TouchableOpacity style={styles.card} onPress={onPress}>
      <View style={styles.cardContent}>
        <View style={styles.leftContent}>
          <Text style={styles.deviceName}>{device.name}</Text>
          <Text style={styles.deviceId}>{device.id.substring(0, 17)}...</Text>
          {device.serviceData && (
            <Text style={styles.deviceInfo}>
              Device: {device.serviceData.deviceName}
            </Text>
          )}
        </View>
        
        <View style={styles.rightContent}>
          {onFavoriteToggle && (
            <TouchableOpacity onPress={onFavoriteToggle} style={styles.favoriteButton}>
              <Text style={styles.favoriteIcon}>{isFavorite ? '★' : '☆'}</Text>
            </TouchableOpacity>
          )}
          
          <View style={styles.signalContainer}>
            <View style={styles.signalBars}>
              {[1, 2, 3, 4, 5].map((bar) => (
                <View
                  key={bar}
                  style={[
                    styles.signalBar,
                    {
                      height: bar * 4,
                      backgroundColor: bar <= bars ? signalColor : '#E0E0E0',
                    },
                  ]}
                />
              ))}
            </View>
            <Text style={styles.rssiText}>{device.rssi} dBm</Text>
          </View>
        </View>
      </View>
      
      {device.isConnected && (
        <View style={styles.connectedBadge}>
          <Text style={styles.connectedText}>Connected</Text>
        </View>
      )}
    </TouchableOpacity>
  );
};

const styles = StyleSheet.create({
  card: {
    backgroundColor: '#FFFFFF',
    borderRadius: 12,
    padding: 16,
    marginVertical: 8,
    marginHorizontal: 16,
    elevation: 3,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  cardContent: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
  },
  leftContent: {
    flex: 1,
  },
  rightContent: {
    alignItems: 'flex-end',
  },
  deviceName: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#212121',
    marginBottom: 4,
  },
  deviceId: {
    fontSize: 12,
    color: '#757575',
    marginBottom: 4,
  },
  deviceInfo: {
    fontSize: 12,
    color: '#424242',
  },
  favoriteButton: {
    padding: 4,
    marginBottom: 8,
  },
  favoriteIcon: {
    fontSize: 24,
    color: '#FFC107',
  },
  signalContainer: {
    alignItems: 'center',
  },
  signalBars: {
    flexDirection: 'row',
    alignItems: 'flex-end',
    height: 20,
    marginBottom: 4,
  },
  signalBar: {
    width: 4,
    marginHorizontal: 1,
    borderRadius: 2,
  },
  rssiText: {
    fontSize: 10,
    color: '#757575',
  },
  connectedBadge: {
    marginTop: 12,
    paddingVertical: 4,
    paddingHorizontal: 12,
    backgroundColor: '#4CAF50',
    borderRadius: 12,
    alignSelf: 'flex-start',
  },
  connectedText: {
    fontSize: 12,
    fontWeight: 'bold',
    color: '#FFFFFF',
  },
});
