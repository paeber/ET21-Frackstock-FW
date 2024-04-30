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
update_enable = False

id = 93
enumerate_ids = True

# Set working directory
os.chdir(os.path.dirname(os.path.realpath(__file__)))

deviceListFile = "device_list.csv"
firmware = "../build/Frackstock.uf2"

def get_serial(ser: serial.Serial, cmd: str):
    cmd = (cmd + "\n").encode("utf-8")
    ser.reset_input_buffer()
    ser.flush()
    ser.write(cmd)
    time.sleep(0.2)
    ret = ser.readline().decode("utf-8").strip()
    ret = ret.replace("\n", "").replace("\r", "")
    return ret

def update_csv(data, unique_id, status):
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
            if unique_id in row:
                if(row[3] != status["id"] and not enumerate_ids):
                    print("[WARNING] Device ID mismatch for: {0}, is: {1} should: {2}".format(unique_id, status["id"], row[3]))
                    arg = input("Update device ID? (y/n): ")
                    if arg == "y":
                        row[3] = status["id"]
                row[2] = status["v"]
                row[4] = status["color"]
                row[5] = status["abrev"]
                break
    
    # Write the updated data to the file
    with open(deviceListFile, mode='w') as file:
        writer = csv.writer(file)
        writer.writerows(data)


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
previous_ports = []
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

        if previous_ports != ports:
            # remove changed ports from used_ports
            for port in used_ports:
                if port not in [p[0] for p in ports]:
                    used_ports.remove(port)
        previous_ports = ports
        print(".", end="")
        time.sleep(1)

    if pico_port is not None:
        print("\n[INFO] Pico found at: ", pico_port)
        used_ports.append(pico_port)

        ser = serial.Serial(pico_port, 115200, timeout=1, rtscts=True)

        try:
            time.sleep(0.2)

            # Disable serial ouput without request
            ret = get_serial(ser, "serial 0")

            # get unique ID
            unique_id = get_serial(ser, "unique")
            print("[INFO] Unique ID: ", unique_id)

            # get current configuration
            status = get_serial(ser, "status")
            status = json.loads(status)
            print("[INFO] Current configuration: ", status)

            if status["id"] == 0 or enumerate_ids:
                get_serial(ser, "set id {0}".format(id))
                id += 1
                print("[INFO]", get_serial(ser, "apply"))
            
            status = get_serial(ser, "status")
            status = json.loads(status)
            print("[INFO] New configuration: ", status)

            # Update the device list
            update_csv(data, unique_id, status)

            if update_enable:
                # Update firmware
                ser.write(b"bootloader\n")
                time.sleep(0.2)
                ser.close()

                if(flash_firmware(firmware)):
                    print("[ERROR] Flashing failed")
                else:
                    print("[INFO] Flashing successful")

                # Wait for the device to reboot
                print("[INFO] Waiting for the device to reboot")
                while not pico_port in serial.tools.list_ports.comports():
                    print(".", end="")
                    time.sleep(1)
                print("\n[INFO] Pico rebooted")

                ser = serial.Serial(pico_port, 115200, timeout=1, rtscts=True)
                time.sleep(0.2)
                ret = get_serial(ser, "serial 0")
                unique_id = get_serial(ser, "unique")
                status = get_serial(ser, "status")
                status = json.loads(status)
                print("[INFO] New configuration: ", status)
                ser.close()

                # Update the device list
                update_csv(data, unique_id, status)

            pico_port = None

        except Exception as e:
            print("[ERROR] ", e)

        except KeyboardInterrupt:
            print("[INFO] Exiting")
            run = False

    time.sleep(1)

