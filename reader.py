import serial
import time
import re
import matplotlib.pyplot as plt
import numpy as np
from sklearn.linear_model import LinearRegression

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

def predict_next_temperature(temperatures):
    X = np.array(range(len(temperatures))).reshape(-1, 1)
    y = np.array(temperatures)
    model = LinearRegression()
    model.fit(X, y)
    next_point = np.array([[len(temperatures)]])
    predicted_temp = model.predict(next_point)[0]
    return predicted_temp, model

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
                
                        # Predict next temperature
                        next_temp, model = predict_next_temperature(real_temps)
                        print(f"Predicted next temperature: {next_temp:.2f}")
                        # Plot the data
                        plt.figure(figsize=(12, 6))
                        x = range(len(real_temps))
                        plt.plot(x, real_temps, 'b-', label='Actual Temperatures')
                        plt.scatter(x, real_temps, color='blue')
                        # Plot the regression line
                        x_reg = np.array(range(len(real_temps) + 1)).reshape(-1, 1)
                        y_reg = model.predict(x_reg)
                        plt.plot(x_reg, y_reg, 'r--', label='Regression Line')
                        
                        # Plot the predicted point
                        plt.scatter([len(real_temps)], [next_temp], color='red', s=100, label='Predicted Next Temperature')
                
                        plt.xlabel('Time')
                        plt.ylabel('Temperature (°C)')
                        plt.title('Temperature Plot with Prediction')
                        plt.legend()
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
