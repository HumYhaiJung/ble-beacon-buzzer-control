import React, { useState } from 'react';
import {
  View,
  Text,
  FlatList,
  StyleSheet,
  TouchableOpacity,
  Alert,
} from 'react-native';
import { useAppContext } from '../context/AppContext';
import { Command, CommandCode } from '../types';
import { commandService } from '../services/commandService';

export const HistoryScreen: React.FC = () => {
  const { commandHistory, clearHistory, devices } = useAppContext();
  const [filterDeviceId, setFilterDeviceId] = useState<string | null>(null);

  const handleClearHistory = () => {
    Alert.alert(
      'Clear History',
      'Are you sure you want to clear all command history?',
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Clear',
          style: 'destructive',
          onPress: clearHistory,
        },
      ]
    );
  };

  const filteredHistory = filterDeviceId
    ? commandHistory.filter((cmd) => cmd.deviceId === filterDeviceId)
    : commandHistory;

  const renderCommandItem = ({ item }: { item: Command }) => {
    const timestamp = new Date(item.timestamp).toLocaleString();
    const commandLabel = commandService.getCommandLabel(item.command);

    return (
      <View style={styles.commandCard}>
        <View style={styles.commandHeader}>
          <Text style={styles.deviceName}>{item.deviceName}</Text>
          <View
            style={[
              styles.statusBadge,
              { backgroundColor: item.success ? '#4CAF50' : '#F44336' },
            ]}
          >
            <Text style={styles.statusText}>
              {item.success ? 'Success' : 'Failed'}
            </Text>
          </View>
        </View>
        
        <View style={styles.commandDetails}>
          <View style={styles.detailRow}>
            <Text style={styles.detailLabel}>Command:</Text>
            <Text style={styles.detailValue}>{commandLabel}</Text>
          </View>
          
          <View style={styles.detailRow}>
            <Text style={styles.detailLabel}>Time:</Text>
            <Text style={styles.detailValue}>{timestamp}</Text>
          </View>
          
          {item.response && (
            <View style={styles.detailRow}>
              <Text style={styles.detailLabel}>Response:</Text>
              <Text style={styles.detailValue}>{item.response}</Text>
            </View>
          )}
        </View>
      </View>
    );
  };

  const uniqueDevices = Array.from(
    new Set(commandHistory.map((cmd) => cmd.deviceId))
  ).map((deviceId) => {
    const cmd = commandHistory.find((c) => c.deviceId === deviceId);
    return {
      id: deviceId,
      name: cmd?.deviceName || 'Unknown',
    };
  });

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>Command History</Text>
        <Text style={styles.subtitle}>
          {filteredHistory.length} command{filteredHistory.length !== 1 ? 's' : ''}
        </Text>
      </View>

      {uniqueDevices.length > 1 && (
        <View style={styles.filterContainer}>
          <Text style={styles.filterLabel}>Filter by device:</Text>
          <View style={styles.filterButtons}>
            <TouchableOpacity
              style={[
                styles.filterButton,
                filterDeviceId === null && styles.filterButtonActive,
              ]}
              onPress={() => setFilterDeviceId(null)}
            >
              <Text
                style={[
                  styles.filterButtonText,
                  filterDeviceId === null && styles.filterButtonTextActive,
                ]}
              >
                All
              </Text>
            </TouchableOpacity>
            
            {uniqueDevices.map((device) => (
              <TouchableOpacity
                key={device.id}
                style={[
                  styles.filterButton,
                  filterDeviceId === device.id && styles.filterButtonActive,
                ]}
                onPress={() => setFilterDeviceId(device.id)}
              >
                <Text
                  style={[
                    styles.filterButtonText,
                    filterDeviceId === device.id && styles.filterButtonTextActive,
                  ]}
                >
                  {device.name}
                </Text>
              </TouchableOpacity>
            ))}
          </View>
        </View>
      )}

      <FlatList
        data={filteredHistory}
        keyExtractor={(item) => item.id}
        renderItem={renderCommandItem}
        ListEmptyComponent={
          <View style={styles.emptyContainer}>
            <Text style={styles.emptyText}>No command history</Text>
            <Text style={styles.emptySubtext}>
              Commands you send will appear here
            </Text>
          </View>
        }
        contentContainerStyle={
          filteredHistory.length === 0 && styles.emptyListContainer
        }
      />

      {commandHistory.length > 0 && (
        <View style={styles.footer}>
          <TouchableOpacity
            style={styles.clearButton}
            onPress={handleClearHistory}
          >
            <Text style={styles.clearButtonText}>Clear History</Text>
          </TouchableOpacity>
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
    marginBottom: 4,
  },
  subtitle: {
    fontSize: 14,
    color: '#757575',
  },
  filterContainer: {
    padding: 16,
    backgroundColor: '#FFFFFF',
    borderBottomWidth: 1,
    borderBottomColor: '#E0E0E0',
  },
  filterLabel: {
    fontSize: 14,
    fontWeight: '600',
    color: '#424242',
    marginBottom: 8,
  },
  filterButtons: {
    flexDirection: 'row',
    flexWrap: 'wrap',
  },
  filterButton: {
    paddingVertical: 8,
    paddingHorizontal: 16,
    borderRadius: 20,
    backgroundColor: '#F5F5F5',
    marginRight: 8,
    marginBottom: 8,
  },
  filterButtonActive: {
    backgroundColor: '#2196F3',
  },
  filterButtonText: {
    fontSize: 14,
    color: '#424242',
  },
  filterButtonTextActive: {
    color: '#FFFFFF',
    fontWeight: 'bold',
  },
  commandCard: {
    backgroundColor: '#FFFFFF',
    borderRadius: 12,
    padding: 16,
    marginVertical: 8,
    marginHorizontal: 16,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  commandHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 12,
  },
  deviceName: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#212121',
  },
  statusBadge: {
    paddingVertical: 4,
    paddingHorizontal: 12,
    borderRadius: 12,
  },
  statusText: {
    fontSize: 12,
    fontWeight: 'bold',
    color: '#FFFFFF',
  },
  commandDetails: {
    borderTopWidth: 1,
    borderTopColor: '#E0E0E0',
    paddingTop: 12,
  },
  detailRow: {
    flexDirection: 'row',
    marginBottom: 8,
  },
  detailLabel: {
    fontSize: 14,
    color: '#757575',
    fontWeight: '600',
    width: 100,
  },
  detailValue: {
    fontSize: 14,
    color: '#212121',
    flex: 1,
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
    marginBottom: 8,
  },
  emptySubtext: {
    fontSize: 14,
    color: '#9E9E9E',
    textAlign: 'center',
  },
  footer: {
    padding: 16,
    backgroundColor: '#FFFFFF',
    borderTopWidth: 1,
    borderTopColor: '#E0E0E0',
  },
  clearButton: {
    backgroundColor: '#F44336',
    paddingVertical: 12,
    paddingHorizontal: 24,
    borderRadius: 8,
    alignItems: 'center',
  },
  clearButtonText: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: 'bold',
  },
});
