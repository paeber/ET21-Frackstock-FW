
import os, sys, time


def flash_firmware(firmware):
    # Set working directory
    os.chdir(os.path.dirname(os.path.realpath(__file__)))

    # Check if the firmware file exists
    if not os.path.exists(firmware):
        print("Firmware file not found")
        return 1

    print("Looking for drive...")

    # Check if drive called RPI-RP2 is mounted
    if sys.platform == "darwin":
        
        # Check if drive is mounted
        if not os.path.exists("/Volumes/RPI-RP2"):
            print("Drive not mounted")
            return 1

        # Copy the firmware file to the drive
        os.system(f"cp {firmware} /Volumes/RPI-RP2")
        print("Firmware copied")
        return 0
    
    elif sys.platform == "win32":
        pi_drive = None

        # try 5 times to mount the drive
        for i in range(5):
            # List all drives on Windows with letters and names
            drives = os.popen("wmic logicaldisk get caption, volumename").read()
            drives = drives.split("\n")
            drives = [drive.split() for drive in drives]
            drives = [(drive[0], drive[1]) for drive in drives if len(drive) > 1]

            # Check if drive is mounted
            for (letter, name) in drives:
                if name == "RPI-RP2":
                    pi_drive = letter
                    break
            if pi_drive is not None:
                break
            time.sleep(1)

        if pi_drive is None:
            print("Drive not mounted")
            return 1

        print(f"Drive mounted at {pi_drive}")

        win_fw = firmware.replace("/", "\\")

        # Copy the firmware file to the drive
        os.system(f"copy {win_fw} {pi_drive}")
        print("Firmware copied")
        return 0

    else:
        print("Platform not supported")
        return 1

    
if __name__ == "__main__":
    
    firmware = "../build/Frackstock.uf2"
    try:
        while True:
            flash_firmware(firmware)
            time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")
        sys.exit(0)


