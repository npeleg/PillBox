package com.example.bluetooth.le;
import java.util.HashMap;
/**
 * This class includes a small subset of standard GATT attributes for demonstration purposes.
 */
public class SampleGattAttributes {
    private static HashMap<String, String> attributes = new HashMap();
    public static String HEART_RATE_MEASUREMENT = "00002a37-0000-1000-8000-00805f9b34fb";
    public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
    static {
        // Sample Services.
        attributes.put("0000180d-0000-1000-8000-00805f9b34fb", "Heart Rate Service");
        attributes.put("0000fff0-0000-1000-8000-00805f9b34fb", "Fields");
        attributes.put("0000180a-0000-1000-8000-00805f9b34fb", "Device Information Service");
        // Sample Characteristics.
        attributes.put(HEART_RATE_MEASUREMENT, "Heart Rate Measurement");
        attributes.put("00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String");
        attributes.put("0000fff1-0000-1000-8000-00805f9b34fb", "Current Hour");
        attributes.put("0000fff2-0000-1000-8000-00805f9b34fb", "Current Minutes");
        attributes.put("0000fff3-0000-1000-8000-00805f9b34fb", "Morning Time");
        attributes.put("0000fff4-0000-1000-8000-00805f9b34fb", "Afternoon Time");
        attributes.put("0000fff5-0000-1000-8000-00805f9b34fb", "Evening time");
    }
    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }
}