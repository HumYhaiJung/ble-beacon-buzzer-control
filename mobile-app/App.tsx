import React from 'react';
import { Text } from 'react-native';
import { NavigationContainer } from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import { createStackNavigator } from '@react-navigation/stack';
import { AppProvider } from './src/context/AppContext';
import { ScanScreen } from './src/screens/ScanScreen';
import { DeviceScreen } from './src/screens/DeviceScreen';
import { HistoryScreen } from './src/screens/HistoryScreen';
import { StatusBar } from 'expo-status-bar';

const Tab = createBottomTabNavigator();
const Stack = createStackNavigator();

function ScanStack() {
  return (
    <Stack.Navigator>
      <Stack.Screen
        name="ScanList"
        component={ScanScreen}
        options={{ headerShown: false }}
      />
      <Stack.Screen
        name="Device"
        component={DeviceScreen}
        options={{ title: 'Device Details' }}
      />
    </Stack.Navigator>
  );
}

// Simple icon component using emoji
function TabIcon({ emoji }: { emoji: string }) {
  return <Text style={{ fontSize: 24 }}>{emoji}</Text>;
}

function MainTabs() {
  return (
    <Tab.Navigator
      screenOptions={{
        tabBarActiveTintColor: '#2196F3',
        tabBarInactiveTintColor: '#9E9E9E',
        tabBarStyle: {
          backgroundColor: '#FFFFFF',
          borderTopWidth: 1,
          borderTopColor: '#E0E0E0',
        },
      }}
    >
      <Tab.Screen
        name="Scan"
        component={ScanStack}
        options={{
          headerShown: false,
          tabBarLabel: 'Scan',
          tabBarIcon: ({ color }) => <TabIcon emoji="🔍" />,
        }}
      />
      <Tab.Screen
        name="History"
        component={HistoryScreen}
        options={{
          headerShown: false,
          tabBarLabel: 'History',
          tabBarIcon: ({ color }) => <TabIcon emoji="📜" />,
        }}
      />
    </Tab.Navigator>
  );
}

export default function App() {
  return (
    <AppProvider>
      <NavigationContainer>
        <StatusBar style="auto" />
        <MainTabs />
      </NavigationContainer>
    </AppProvider>
  );
}
