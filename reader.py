import serial
import time

# Adjust the port name as needed (e.g., 'COM3' on Windows)
SERIAL_PORT = '/dev/ttyACM0'  # Typical for Linux
BAUD_RATE = 115200

def read_temperature_data():
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            print("Connected to Raspberry Pi Pico. Waiting for data...")
            
            while True:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8').strip()
                    print(line)
                
                time.sleep(0.1)
                
    except serial.SerialException as e:
        print(f"Error: Could not open serial port {SERIAL_PORT}")
        print(e)
    except KeyboardInterrupt:
        print("\nScript terminated by user.")

if __name__ == "__main__":
    read_temperature_data()
