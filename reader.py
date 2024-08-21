import serial
import time
import re
import matplotlib.pyplot as plt
SERIAL_PORT = '/dev/ttyACM0'  # Typical for Linux
BAUD_RATE = 115200
BASE_TEMP = 23.0

def extract_temperature_history(line):
    match = re.search(r'Temperature history:?\s*([-\d\s]+)', line)
    if match:
        values = [int(x) for x in match.group(1).split()]
        return values
    return None

def convert_to_real_temperatures(differences):
    return [BASE_TEMP + diff for diff in differences if diff >= 0]

def read_temperature_data():
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            print("Connected to Raspberry Pi Pico. Waiting for data...")
            
            while True:
                if ser.in_waiting > 0:
                    line_data = ser.readline().decode('utf-8').strip()
                    print(line_data)
                
                    temp_differences = extract_temperature_history(line_data)
                    if temp_differences is not None:
                        real_temps = convert_to_real_temperatures(temp_differences)
                        print(f"Actual temperatures: {real_temps}")
                        print(f"Number of valid temperatures: {len(real_temps)}")
                
                        # Plot the data
                        plt.figure(figsize=(10, 6))
                        plt.plot(range(len(real_temps)), real_temps)
                        plt.xlabel('Time')
                        plt.ylabel('Temperature (°C)')
                        plt.title('Temperature Plot')
                        plt.grid(True)
                        plt.show()
                        
                        return  # Exit the function after plotting
                
                time.sleep(0.1)
                
    except serial.SerialException as e:
        print(f"Error: Could not open serial port {SERIAL_PORT}")
        print(e)
    except KeyboardInterrupt:
        print("\nScript terminated by user.")

if __name__ == "__main__":
    read_temperature_data()
