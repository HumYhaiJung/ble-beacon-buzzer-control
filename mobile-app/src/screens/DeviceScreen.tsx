import React, { useState, useEffect } from 'react';
import { View, Text, StyleSheet, ScrollView, Alert, Dimensions } from 'react-native';
import { useAppContext } from '../context/AppContext';
import { CommandButton } from '../components/CommandButton';
import { StatusIndicator } from '../components/StatusIndicator';
import { connectionService } from '../services/connectionService';
import { CommandCode, SignalStrengthPoint } from '../types';
import { LineChart } from 'react-native-chart-kit';

export const DeviceScreen: React.FC = () => {
  const { selectedDevice, addCommandToHistory } = useAppContext();
  const [loadingCommand, setLoadingCommand] = useState<CommandCode | null>(null);
  const [signalHistory, setSignalHistory] = useState<SignalStrengthPoint[]>([]);

  // Helper function to get command label
  const getCommandLabel = (command: CommandCode): string => {
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
  };

  useEffect(() => {
    if (selectedDevice) {
      // Add current RSSI to history
      const point: SignalStrengthPoint = {
        timestamp: Date.now(),
        rssi: selectedDevice.rssi,
      };

      setSignalHistory((prev) => {
        const updated = [...prev, point];
        // Keep last 20 points
        return updated.slice(-20);
      });
    }
  }, [selectedDevice?.rssi]);

  if (!selectedDevice) {
    return (
      <View style={styles.container}>
        <Text style={styles.errorText}>No device selected</Text>
      </View>
    );
  }

  const handleCommandPress = async (command: CommandCode) => {
    try {
      setLoadingCommand(command);

      // Use connection service instead of GATT command service
      await connectionService.sendCommand(selectedDevice.id, command);

      // Create a successful command for history
      const result = {
        id: Date.now().toString(),
        deviceId: selectedDevice.id,
        deviceName: selectedDevice.name,
        command,
        timestamp: new Date(),
        success: true,
        response: 'Command sent via connection trigger',
      };

      addCommandToHistory(result);

      Alert.alert(
        'Success',
        `Command "${getCommandLabel(command)}" sent successfully via connection`
      );
    } catch (error: any) {
      // Create a failed command for history
      const result = {
        id: Date.now().toString(),
        deviceId: selectedDevice.id,
        deviceName: selectedDevice.name,
        command,
        timestamp: new Date(),
        success: false,
        response: error.message || 'Connection command failed',
      };

      addCommandToHistory(result);
      Alert.alert('Error', error.message || 'Failed to send command');
    } finally {
      setLoadingCommand(null);
    }
  };

  const chartData = {
    labels:
      signalHistory.length > 0
        ? signalHistory.map((_, idx) => (idx % 5 === 0 ? idx.toString() : ''))
        : [''],
    datasets: [
      {
        data: signalHistory.length > 0 ? signalHistory.map((point) => point.rssi) : [-100],
      },
    ],
  };

  const screenWidth = Dimensions.get('window').width;

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.contentContainer}>
      <View style={styles.header}>
        <Text style={styles.deviceName}>{selectedDevice.name}</Text>
        <StatusIndicator status="connected" signalStrength={selectedDevice.rssi} />
      </View>

      <View style={styles.infoCard}>
        <View style={styles.infoRow}>
          <Text style={styles.infoLabel}>Device ID:</Text>
          <Text style={styles.infoValue}>{selectedDevice.id}</Text>
        </View>
        <View style={styles.infoRow}>
          <Text style={styles.infoLabel}>Signal Strength:</Text>
          <Text style={styles.infoValue}>{selectedDevice.rssi} dBm</Text>
        </View>
        {selectedDevice.serviceData && (
          <>
            <View style={styles.infoRow}>
              <Text style={styles.infoLabel}>Device Name:</Text>
              <Text style={styles.infoValue}>{selectedDevice.serviceData.deviceName}</Text>
            </View>
            <View style={styles.infoRow}>
              <Text style={styles.infoLabel}>Last Command:</Text>
              <Text style={styles.infoValue}>
                {getCommandLabel(selectedDevice.serviceData.command)}
              </Text>
            </View>
          </>
        )}
      </View>

      {signalHistory.length > 0 && (
        <View style={styles.chartCard}>
          <Text style={styles.chartTitle}>Signal Strength History</Text>
          <LineChart
            data={chartData}
            width={screenWidth - 48}
            height={200}
            chartConfig={{
              backgroundColor: '#FFFFFF',
              backgroundGradientFrom: '#FFFFFF',
              backgroundGradientTo: '#FFFFFF',
              decimalPlaces: 0,
              color: (opacity = 1) => `rgba(33, 150, 243, ${opacity})`,
              labelColor: (opacity = 1) => `rgba(117, 117, 117, ${opacity})`,
              style: {
                borderRadius: 16,
              },
              propsForDots: {
                r: '4',
                strokeWidth: '2',
                stroke: '#2196F3',
              },
            }}
            bezier
            style={styles.chart}
          />
        </View>
      )}

      <View style={styles.commandsCard}>
        <Text style={styles.commandsTitle}>Commands</Text>

        <CommandButton
          command={CommandCode.PLAY}
          label="Play Buzzer"
          onPress={() => handleCommandPress(CommandCode.PLAY)}
          loading={loadingCommand === CommandCode.PLAY}
          disabled={loadingCommand !== null}
          variant="success"
        />

        {/* <CommandButton
          command={CommandCode.STOP}
          label="Stop Buzzer"
          onPress={() => handleCommandPress(CommandCode.STOP)}
          loading={loadingCommand === CommandCode.STOP}
          disabled={loadingCommand !== null}
          variant="danger"
        />

        <CommandButton
          command={CommandCode.ENABLE}
          label="Enable Buzzer"
          onPress={() => handleCommandPress(CommandCode.ENABLE)}
          loading={loadingCommand === CommandCode.ENABLE}
          disabled={loadingCommand !== null}
          variant="primary"
        />

        <CommandButton
          command={CommandCode.DISABLE}
          label="Disable Buzzer"
          onPress={() => handleCommandPress(CommandCode.DISABLE)}
          loading={loadingCommand === CommandCode.DISABLE}
          disabled={loadingCommand !== null}
          variant="secondary"
        /> */}

        {/* <CommandButton
          command={CommandCode.STATUS}
          label="Request Status"
          onPress={() => handleCommandPress(CommandCode.STATUS)}
          loading={loadingCommand === CommandCode.STATUS}
          disabled={loadingCommand !== null}
          variant="primary"
        /> */}
      </View>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#F5F5F5',
  },
  contentContainer: {
    padding: 16,
  },
  header: {
    backgroundColor: '#FFFFFF',
    borderRadius: 12,
    padding: 16,
    marginBottom: 16,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  deviceName: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#212121',
    marginBottom: 12,
  },
  infoCard: {
    backgroundColor: '#FFFFFF',
    borderRadius: 12,
    padding: 16,
    marginBottom: 16,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  infoRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 12,
  },
  infoLabel: {
    fontSize: 14,
    color: '#757575',
    fontWeight: '600',
  },
  infoValue: {
    fontSize: 14,
    color: '#212121',
    flex: 1,
    textAlign: 'right',
  },
  chartCard: {
    backgroundColor: '#FFFFFF',
    borderRadius: 12,
    padding: 16,
    marginBottom: 16,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  chartTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#212121',
    marginBottom: 12,
  },
  chart: {
    borderRadius: 12,
  },
  commandsCard: {
    backgroundColor: '#FFFFFF',
    borderRadius: 12,
    padding: 16,
    marginBottom: 16,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  commandsTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#212121',
    marginBottom: 12,
  },
  errorText: {
    fontSize: 16,
    color: '#757575',
    textAlign: 'center',
    marginTop: 32,
  },
});
