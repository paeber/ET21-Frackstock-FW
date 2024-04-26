
import os, sys

firmware = "../build/Frackstock.uf2"

# Set working directory
os.chdir(os.path.dirname(os.path.realpath(__file__)))

# Check if the firmware file exists
if not os.path.exists(firmware):
    print("Firmware file not found")
    exit(1)

# Check if drive called RPI-RP2 is mounted
if sys.platform == "darwin":
    
    # Check if drive is mounted
    if not os.path.exists("/Volumes/RPI-RP2"):
        print("Drive not mounted")
        exit(1)

    # Copy the firmware file to the drive
    os.system(f"cp {firmware} /Volumes/RPI-RP2")
    print("Firmware copied")