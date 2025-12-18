import React, { createContext, useContext, useState, useEffect, ReactNode } from 'react';
import { BleDevice, Command, AppSettings, CommandCode } from '../types';
import { storageService } from '../services/storageService';

interface AppContextType {
  devices: BleDevice[];
  setDevices: (devices: BleDevice[]) => void;
  selectedDevice: BleDevice | null;
  setSelectedDevice: (device: BleDevice | null) => void;
  commandHistory: Command[];
  addCommandToHistory: (command: Command) => void;
  clearHistory: () => void;
  isScanning: boolean;
  setIsScanning: (scanning: boolean) => void;
  settings: AppSettings;
  updateSettings: (settings: Partial<AppSettings>) => void;
  favoriteDevices: string[];
  toggleFavorite: (deviceId: string) => void;
}

const AppContext = createContext<AppContextType | undefined>(undefined);

export const useAppContext = () => {
  const context = useContext(AppContext);
  if (!context) {
    throw new Error('useAppContext must be used within AppProvider');
  }
  return context;
};

interface AppProviderProps {
  children: ReactNode;
}

export const AppProvider: React.FC<AppProviderProps> = ({ children }) => {
  const [devices, setDevices] = useState<BleDevice[]>([]);
  const [selectedDevice, setSelectedDevice] = useState<BleDevice | null>(null);
  const [commandHistory, setCommandHistory] = useState<Command[]>([]);
  const [isScanning, setIsScanning] = useState(false);
  const [settings, setSettings] = useState<AppSettings>({
    scanInterval: 1000,
    autoConnect: false,
    theme: 'light',
  });
  const [favoriteDevices, setFavoriteDevices] = useState<string[]>([]);

  // Load saved data on mount
  useEffect(() => {
    loadStoredData();
  }, []);

  // Save command history when it changes
  useEffect(() => {
    if (commandHistory.length > 0) {
      storageService.saveCommandHistory(commandHistory);
    }
  }, [commandHistory]);

  // Save favorite devices when they change
  useEffect(() => {
    if (favoriteDevices.length >= 0) {
      storageService.saveFavoriteDevices(favoriteDevices);
    }
  }, [favoriteDevices]);

  // Save settings when they change
  useEffect(() => {
    storageService.saveSettings(settings);
  }, [settings]);

  const loadStoredData = async () => {
    try {
      const [savedHistory, savedFavorites, savedSettings] = await Promise.all([
        storageService.loadCommandHistory(),
        storageService.loadFavoriteDevices(),
        storageService.loadSettings(),
      ]);

      if (savedHistory) setCommandHistory(savedHistory);
      if (savedFavorites) setFavoriteDevices(savedFavorites);
      if (savedSettings) setSettings(savedSettings);
    } catch (error) {
      console.error('Error loading stored data:', error);
    }
  };

  const addCommandToHistory = (command: Command) => {
    setCommandHistory((prev) => [command, ...prev].slice(0, 100)); // Keep last 100 commands
  };

  const clearHistory = () => {
    setCommandHistory([]);
    storageService.clearCommandHistory();
  };

  const updateSettings = (newSettings: Partial<AppSettings>) => {
    setSettings((prev) => ({ ...prev, ...newSettings }));
  };

  const toggleFavorite = (deviceId: string) => {
    setFavoriteDevices((prev) => {
      if (prev.includes(deviceId)) {
        return prev.filter((id) => id !== deviceId);
      } else {
        return [...prev, deviceId];
      }
    });
  };

  const value: AppContextType = {
    devices,
    setDevices,
    selectedDevice,
    setSelectedDevice,
    commandHistory,
    addCommandToHistory,
    clearHistory,
    isScanning,
    setIsScanning,
    settings,
    updateSettings,
    favoriteDevices,
    toggleFavorite,
  };

  return <AppContext.Provider value={value}>{children}</AppContext.Provider>;
};
