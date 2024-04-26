#
#
#

import serial
import serial.tools.list_ports
import time
import sys, os
import json

run = True
used_ports = []

# Set working directory
os.chdir(os.path.dirname(os.path.realpath(__file__)))

def flash_firmware():
    firmware = "../build/Frackstock.uf2"

    # Check if the firmware file exists
    if not os.path.exists(firmware):
        print("Firmware file not found")
        exit(1)

    # Check if drive called RPI-RP2 is mounted
    if sys.platform == "darwin":
        
        # Wait for the drive to be mounted
        while not os.path.exists("/Volumes/RPI-RP2"):
            print(".", end="")
            time.sleep(1)

        # Copy the firmware file to the drive
        os.system(f"cp {firmware} /Volumes/RPI-RP2")
        print("Firmware copied")



while run:
    # List available ports
    ports = serial.tools.list_ports.comports()
    pico_port = None
    for port, desc, hwid in sorted(ports):
        #print(f"{port} {desc} {hwid}")
        if("Pico" in str(desc)):
            pico_port = port
            break

    if pico_port is None:
        pass #print("[ERROR] Pico not found")

    elif pico_port in used_ports:
        pass #print("[ERROR] Pico at {pico_port} already handled")

    else:
        used_ports.append(pico_port)
        print("[INFO] Access Port: ", pico_port)

        ser = serial.Serial(pico_port, 115200, timeout=1, rtscts=True)

        try:
            time.sleep(1)

            for i in range(10):
                ser.reset_input_buffer()
                ser.flush()
                time.sleep(0.5)

            # get unique ID
            ser.write(b"unique\n")
            time.sleep(0.2)
            unique_id = ser.readline()
            print("[INFO] Unique ID: ", unique_id.decode("utf-8").strip())

            # get current configuration
            ser.write(b"status\n")
            time.sleep(0.2)
            status = ser.readline()
            status = status.decode("utf-8").strip()
            status = json.loads(status)
            print("[INFO] Current configuration: ", status)

            # Update firmware
            ser.write(b"bootloader\n")
            time.sleep(0.2)
            ser.close()

            flash_firmware()



        except Exception as e:
            print("[ERROR] ", e)

        except KeyboardInterrupt:
            print("[INFO] Exiting")
            run = False

        finally:
            if ser.is_open:
                ser.close()
            print("[INFO] Port closed")

