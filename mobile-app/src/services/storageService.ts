import AsyncStorage from '@react-native-async-storage/async-storage';
import { Command, AppSettings } from '../types';

const KEYS = {
  COMMAND_HISTORY: '@command_history',
  FAVORITE_DEVICES: '@favorite_devices',
  SETTINGS: '@settings',
};

class StorageService {
  /**
   * Save command history
   */
  async saveCommandHistory(history: Command[]): Promise<void> {
    try {
      const jsonValue = JSON.stringify(history);
      await AsyncStorage.setItem(KEYS.COMMAND_HISTORY, jsonValue);
    } catch (error) {
      console.error('Error saving command history:', error);
      throw error;
    }
  }

  /**
   * Load command history
   */
  async loadCommandHistory(): Promise<Command[] | null> {
    try {
      const jsonValue = await AsyncStorage.getItem(KEYS.COMMAND_HISTORY);
      if (jsonValue != null) {
        const history = JSON.parse(jsonValue);
        // Convert timestamp strings back to Date objects
        return history.map((cmd: any) => ({
          ...cmd,
          timestamp: new Date(cmd.timestamp),
        }));
      }
      return null;
    } catch (error) {
      console.error('Error loading command history:', error);
      return null;
    }
  }

  /**
   * Clear command history
   */
  async clearCommandHistory(): Promise<void> {
    try {
      await AsyncStorage.removeItem(KEYS.COMMAND_HISTORY);
    } catch (error) {
      console.error('Error clearing command history:', error);
      throw error;
    }
  }

  /**
   * Save favorite devices
   */
  async saveFavoriteDevices(deviceIds: string[]): Promise<void> {
    try {
      const jsonValue = JSON.stringify(deviceIds);
      await AsyncStorage.setItem(KEYS.FAVORITE_DEVICES, jsonValue);
    } catch (error) {
      console.error('Error saving favorite devices:', error);
      throw error;
    }
  }

  /**
   * Load favorite devices
   */
  async loadFavoriteDevices(): Promise<string[] | null> {
    try {
      const jsonValue = await AsyncStorage.getItem(KEYS.FAVORITE_DEVICES);
      return jsonValue != null ? JSON.parse(jsonValue) : null;
    } catch (error) {
      console.error('Error loading favorite devices:', error);
      return null;
    }
  }

  /**
   * Save app settings
   */
  async saveSettings(settings: AppSettings): Promise<void> {
    try {
      const jsonValue = JSON.stringify(settings);
      await AsyncStorage.setItem(KEYS.SETTINGS, jsonValue);
    } catch (error) {
      console.error('Error saving settings:', error);
      throw error;
    }
  }

  /**
   * Load app settings
   */
  async loadSettings(): Promise<AppSettings | null> {
    try {
      const jsonValue = await AsyncStorage.getItem(KEYS.SETTINGS);
      return jsonValue != null ? JSON.parse(jsonValue) : null;
    } catch (error) {
      console.error('Error loading settings:', error);
      return null;
    }
  }

  /**
   * Clear all stored data
   */
  async clearAll(): Promise<void> {
    try {
      await AsyncStorage.multiRemove([
        KEYS.COMMAND_HISTORY,
        KEYS.FAVORITE_DEVICES,
        KEYS.SETTINGS,
      ]);
    } catch (error) {
      console.error('Error clearing all data:', error);
      throw error;
    }
  }
}

export const storageService = new StorageService();
