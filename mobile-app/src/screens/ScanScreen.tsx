import React, { useEffect, useState } from 'react';
import {
  View,
  Text,
  FlatList,
  StyleSheet,
  RefreshControl,
  Alert,
  TouchableOpacity,
} from 'react-native';
import { useNavigation } from '@react-navigation/native';
import { useAppContext } from '../context/AppContext';
import { bleService } from '../services/bleService';
import { DeviceCard } from '../components/DeviceCard';
import { StatusIndicator } from '../components/StatusIndicator';
import { BleDevice } from '../types';

export const ScanScreen: React.FC = () => {
  const navigation = useNavigation();
  const {
    devices,
    setDevices: _setDevices,
    isScanning,
    setIsScanning,
    setSelectedDevice,
    favoriteDevices,
    toggleFavorite,
  } = useAppContext();
  const setDevices = _setDevices as React.Dispatch<React.SetStateAction<BleDevice[]>>;
  const [refreshing, setRefreshing] = useState(false);
  const [showRaw, setShowRaw] = useState(false);

  useEffect(() => {
    startScan();
    return () => {
      bleService.stopScanning();
    };
  }, []);

  const startScan = async () => {
    try {
      setIsScanning(true);
      setDevices([]);

      await bleService.startScanning(
        (device) => {
          setDevices((prev: BleDevice[]) => {
            const existingIndex = prev.findIndex((d) => d.id === device.id);
            if (existingIndex >= 0) {
              const updated = [...prev];
              updated[existingIndex] = device;
              return updated;
            }
            return [...prev, device];
          });
        },
        (error) => {
          Alert.alert('Scan Error', error.message);
          setIsScanning(false);
        }
      );
    } catch (error) {
      Alert.alert('Error', error instanceof Error ? error.message : 'Failed to start scanning');
      setIsScanning(false);
    }
  };

  const stopScan = async () => {
    await bleService.stopScanning();
    setIsScanning(false);
  };

  const onRefresh = async () => {
    setRefreshing(true);
    await stopScan();
    await startScan();
    setRefreshing(false);
  };

  const handleDevicePress = (device: BleDevice) => {
    setSelectedDevice(device);
    navigation.navigate('Device' as never);
  };

  const sortedDevices = [...devices].sort((a, b) => {
    // Sort by favorite status first, then by RSSI
    const aFav = favoriteDevices.includes(a.id) ? 1 : 0;
    const bFav = favoriteDevices.includes(b.id) ? 1 : 0;
    if (aFav !== bFav) return bFav - aFav;
    return b.rssi - a.rssi;
  });

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>BLE Beacon Scanner</Text>
        <StatusIndicator status={isScanning ? 'scanning' : 'idle'} />
      </View>

      <View style={styles.controlsContainer}>
        <TouchableOpacity
          style={[styles.button, isScanning && styles.buttonDanger]}
          onPress={isScanning ? stopScan : startScan}
        >
          <Text style={styles.buttonText}>{isScanning ? 'Stop Scanning' : 'Start Scanning'}</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={[styles.button, styles.debugButton]}
          onPress={() => setShowRaw((s) => !s)}
        >
          <Text style={styles.buttonText}>{showRaw ? 'Hide Raw' : 'Show Raw'}</Text>
        </TouchableOpacity>
      </View>

      <View style={styles.deviceCountContainer}>
        <Text style={styles.deviceCountText}>
          Found {devices.length} device{devices.length !== 1 ? 's' : ''}
        </Text>
      </View>

      <FlatList
        data={sortedDevices}
        keyExtractor={(item) => item.id}
        renderItem={({ item }) => (
          <DeviceCard
            device={item}
            isFavorite={favoriteDevices.includes(item.id)}
            onPress={() => handleDevicePress(item)}
            onFavoriteToggle={() => toggleFavorite(item.id)}
          />
        )}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
        ListEmptyComponent={
          <View style={styles.emptyContainer}>
            <Text style={styles.emptyText}>
              {isScanning ? 'Scanning for devices...' : 'No devices found'}
            </Text>
            <Text style={styles.emptySubtext}>
              {isScanning
                ? 'Make sure your beacon device is nearby and powered on'
                : 'Pull down to refresh or press Start Scanning'}
            </Text>
          </View>
        }
        contentContainerStyle={devices.length === 0 && styles.emptyListContainer}
      />
      {showRaw && (
        <View style={styles.rawContainer}>
          <Text style={styles.rawTitle}>Raw Devices (debug)</Text>
          <FlatList
            data={devices}
            keyExtractor={(item) => `raw-${item.id}`}
            renderItem={({ item }) => (
              <View style={styles.rawItem}>
                <Text style={styles.rawId}>{item.id}</Text>
                <Text style={styles.rawJson}>{JSON.stringify(item.raw || {}, null, 2)}</Text>
              </View>
            )}
          />
        </View>
      )}
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#F5F5F5',
  },
  header: {
    padding: 16,
    backgroundColor: '#FFFFFF',
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#212121',
    marginBottom: 12,
  },
  controlsContainer: {
    padding: 16,
    backgroundColor: '#FFFFFF',
    borderBottomWidth: 1,
    borderBottomColor: '#E0E0E0',
  },
  button: {
    backgroundColor: '#2196F3',
    paddingVertical: 12,
    paddingHorizontal: 24,
    borderRadius: 8,
    alignItems: 'center',
  },
  buttonDanger: {
    backgroundColor: '#F44336',
  },
  buttonText: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: 'bold',
  },
  deviceCountContainer: {
    padding: 12,
    backgroundColor: '#FFFFFF',
    borderBottomWidth: 1,
    borderBottomColor: '#E0E0E0',
  },
  deviceCountText: {
    fontSize: 14,
    color: '#757575',
    textAlign: 'center',
  },
  emptyListContainer: {
    flex: 1,
  },
  emptyContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    padding: 32,
  },
  emptyText: {
    fontSize: 18,
    fontWeight: '600',
    color: '#757575',
    textAlign: 'center',
    marginBottom: 8,
  },
  emptySubtext: {
    fontSize: 14,
    color: '#9E9E9E',
    textAlign: 'center',
  },
  debugButton: {
    marginTop: 8,
    backgroundColor: '#607D8B',
  },
  rawContainer: {
    backgroundColor: '#FFFFFF',
    margin: 12,
    padding: 12,
    borderRadius: 8,
    maxHeight: 240,
  },
  rawTitle: {
    fontSize: 14,
    fontWeight: 'bold',
    marginBottom: 8,
  },
  rawItem: {
    borderTopWidth: 1,
    borderTopColor: '#EEE',
    paddingTop: 8,
    marginTop: 8,
  },
  rawId: {
    fontSize: 12,
    color: '#424242',
    marginBottom: 4,
  },
  rawJson: {
    fontSize: 11,
    color: '#616161',
    fontFamily: 'monospace',
  },
});
