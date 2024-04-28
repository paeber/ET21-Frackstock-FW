#
#
#

import serial
import serial.tools.list_ports
import time
import sys, os
import json
import csv

from flashtool import flash_firmware

run = True
used_ports = []

# Set working directory
os.chdir(os.path.dirname(os.path.realpath(__file__)))

deviceListFile = "device_list.csv"
firmware = "../build/Frackstock.uf2"


# check if device list file exists
if not os.path.exists(deviceListFile):
    print("Device list file not found")
    # Create a new file
    with open(deviceListFile, mode='w') as file:
        writer = csv.writer(file)
        writer.writerow(["Pos", "Unique ID", "Version", "ID", "Color", "Abreviation"])

# Read the device list file
with open(deviceListFile, mode='r') as file:
    reader = csv.reader(file)
    data = list(reader)


if sys.platform == "win32":
    input("Unplug all Pico devices and press Enter to continue...")
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
        print(f"Forget: {port} {desc} {hwid}")
        used_ports.append(port)

print("[INFO] Waiting for pico's...")
while run:
    pico_port = None

    while pico_port is None:
        # List available ports
        ports = serial.tools.list_ports.comports()
        
        for port, desc, hwid in sorted(ports):
            print(f"{port} {desc} {hwid}")
            if("Pico" in str(desc)):
                pico_port = port
                break
            if(sys.platform == "win32"):
                if port not in used_ports:
                    pico_port = port
                    break

        if pico_port in used_ports:
            pico_port = None #print("[ERROR] Pico at {pico_port} already handled")

        print(".", end="")
        time.sleep(1)

    if pico_port is not None:
        print("\n[INFO] Pico found at: ", pico_port)
        used_ports.append(pico_port)

        ser = serial.Serial(pico_port, 115200, timeout=1, rtscts=True)

        try:
            time.sleep(0.2)
            ser.write(b"serial 0\n") # Disable serial output
            time.sleep(0.2)
            ser.reset_input_buffer()
            ser.flush()
            time.sleep(0.2)

            # get unique ID
            ser.write(b"unique\n")
            time.sleep(0.2)
            unique_id = ser.readline()
            uinque_id = unique_id.decode("utf-8").strip()
            print("[INFO] Unique ID: ", unique_id)

            # get current configuration
            ser.write(b"status\n")
            time.sleep(0.2)
            status = ser.readline()
            status = status.decode("utf-8").strip()
            status = json.loads(status)
            print("[INFO] Current configuration: ", status)

            # Check if the device is already in the list
            found = False
            for row in data:
                if unique_id in row:
                    found = True
                    break

            if not found:
                data.append([len(data), unique_id, status["v"], status["id"], status["color"], status["abrev"]])
            else:
                # Update the information
                for row in data:
                    if unique_id.decode("utf-8").strip() in row:
                        row[2] = status["v"]
                        row[3] = status["id"]
                        row[4] = status["color"]
                        row[5] = status["abrev"]
                        break
            
            # Write the updated data to the file
            with open(deviceListFile, mode='w') as file:
                writer = csv.writer(file)
                writer.writerows(data)
                

            # Update firmware
            ser.write(b"bootloader\n")
            time.sleep(0.2)
            ser.close()

            if(flash_firmware(firmware)):
                print("[ERROR] Flashing failed")
            else:
                print("[INFO] Flashing successful")

            pico_port = None

        except Exception as e:
            print("[ERROR] ", e)

        except KeyboardInterrupt:
            print("[INFO] Exiting")
            run = False

    time.sleep(1)

