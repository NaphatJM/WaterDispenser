import serial
import requests
import time
import sys
import json

# Configure UART
uart_port = '/dev/ttyS1'  # Adjust this to your UART device
baud_rate = 9600
data_file = 'dispensed_data.json'  # File to store dispensed water data

def save_data(dispenser):
    # Get current time
    current_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())
    
    # Create a data object
    data = {"volume": dispenser, "time": current_time}
    
    # Load existing data if available
    try:
        with open(data_file, 'r') as f:
            existing_data = json.load(f)
    except FileNotFoundError:
        existing_data = []
    
    # Add the new data
    existing_data.append(data)
    
    # Save updated data back to the file
    with open(data_file, 'w') as f:
        json.dump(existing_data, f)
    
    return data

def main():
    print('Initializing UART connection')

    try:
        ser = serial.Serial(uart_port, baud_rate, timeout=1)
        print(f'UART connection established on port {uart_port} with baud_rate {baud_rate}')

        while True:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()

                if data.isdigit():
                    dispenser = int(data)
                    print(f"received : {dispenser}")
                    
                    # Save data locally
                    saved_data = save_data(dispenser)
                    print(f"Saved data: {saved_data}")

                else:
                    print(f'error : {data}')

    except serial.SerialException as e:
        print(f'Serial exception occurred : {e}')

    except KeyboardInterrupt:
        print('Interrupt by user')

    finally:
        if 'ser' in locals():
            ser.close()
            print("UART connection closed")
        print("exiting")

if __name__ == '__main__':
    main()
