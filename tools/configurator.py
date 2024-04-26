import serial
import serial.tools.list_ports
import time

# Set configuration
color = (0, 255, 0)
abrev = "FRK_ET21"

# Limit abrev to 8 characters
if len(abrev) > 8:
    abrev = abrev[:8]

# List available ports
ports = serial.tools.list_ports.comports()
idx = 0
for port, desc, hwid in sorted(ports):
    print(f"{idx}: {port} {desc} {hwid}")
    idx += 1

port = input("Select port: ")
port = ports[int(port)].device
print(f"Selected port: {port}")

ser = serial.Serial(port, 115200, timeout=1, rtscts=True)
try: 
    ser.flush()
    time.sleep(0.5)

    ser.write(b"status\n")
    time.sleep(0.2)

    # read all lines
    lines = []
    while ser.in_waiting:
        line = ser.readline()
        if not line:
            break
        lines.append(line.decode("utf-8").strip())
    
    print("[INFO] Current configuration:")
    for line in lines:
        if "ID" in line:
            print("[INFO] Current ID: ", line.split(":")[1].strip())
        if "Color" in line:
            print("[INFO] Current color: ", line.split(":")[1].strip())
        if "Abrev" in line:
            print("[INFO] Current abrev: ", line.split(":")[1].strip())

    #print("\n".join(lines))

    print("\n")

    time.sleep(0.5)

    color_cmd = f"set color ({color[0]},{color[1]},{color[2]})\n"
    ser.write(color_cmd.encode("utf-8"))

    time.sleep(0.5)
    abrev_cmd = f"set abrev {abrev}\n"
    ser.write(abrev_cmd.encode("utf-8"))
    
    while ser.in_waiting:
        line = ser.readline()
        if not line:
            break
        #print(line.decode("utf-8").strip())

    ser.flush()
    time.sleep(0.5)

    ser.write(b"status\n")
    time.sleep(0.2)

    # read all lines
    lines = []
    while ser.in_waiting:
        line = ser.readline()
        if not line:
            break
        lines.append(line.decode("utf-8").strip())
    
    print("[INFO] New configuration:")
    for line in lines:
        if "ID" in line:
            print("[INFO] New ID: ", line.split(":")[1].strip())
        if "Color" in line:
            print("[INFO] New color: ", line.split(":")[1].strip())
        if "Abrev" in line:
            print("[INFO] New abrev: ", line.split(":")[1].strip())
    #print("\n".join(lines))

    arg = input("Apply changes? [y/n]: ")
    if arg.lower() == "y":
        ser.write(b"apply\n")
        time.sleep(0.5)
        ser.write(b"reset\n")
        time.sleep(0.5)
    else:
        print("Changes not applied")
    
    







except KeyboardInterrupt:
    print("Exiting")
finally:
    ser.close()
