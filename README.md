# ET21-Frackstock-FW

## Features

- [x] LED Ring color effects
- [x] Segment show HEX and DEC numbers
- [x] 868 MHz Tx and Rx
- [x] IMU Tap detection
- [x] Persistent flash storage for custom settings
- [x] IMU Interrupts
- [x] Touch Button 

### To do

- [ ] IMU any_motion and no_motion detection
- [ ] Sleep mode for power improvements

## Flash instruction
1. Press <kbd>BOOTSEL</kbd> button on Pico board
2. Plug in the device to PC (or press reset if possible)
3. Release <kbd>BOOTSEL</kbd>
4. Copy `*.uf2` file to the Pico mounted as a drive. The script will run on complete

## Development
At the moment, setup in windows is the easiest. Mac OS and Linux need more manual steps to setup

### Windows Setup
Follow the guide and installer
 - Github: [pico-setup-windows](https://github.com/raspberrypi/pico-setup-windows/)
 - Guide: [Raspberry Pi Pico Windows Installer](https://www.raspberrypi.com/news/raspberry-pi-pico-windows-installer/)
 - Use the VS Code version tagged with "Pico"

### Mac OS [Guide](https://blog.smittytone.net/2021/02/02/program-raspberry-pi-pico-c-mac/) (not complete)

 - Install the SDK:

   1. In Terminal, go to your projects directory, eg. `~/Developer/pico`
   2. Run `git clone -b master --recurse-submodules``https://github.com/raspberrypi/pico-sdk.git`
   3. Edit your `.zsh_profile` or `.zshrc` and add: 
      `export PICO_SDK_PATH="$HOME/Developer/pico/pico-sdk"`

- Install the toolchain:

  1. `brew install cmake`
  2. `brew tap ArmMbed/homebrew-formulae`
  3. `brew install arm-none-eabi-gcc`

- Configure the IDE:

  1. Run — or [install and then run](https://code.visualstudio.com/download) — Microsoft Visual Studio Code.
  2. Click on the Extensions icon.
  3. Enter `CMake Tools` in the search field.
  4. Locate CMake Tools by Microsoft and click **Install**.

### CMakeLists
New sources must be added in file `CMakeLists.txt`

```makefile
add_executable(Frackstock
    source/main.cpp
    source/led_ring.cpp
    #... Add new source here
)
```

### Building

Press **Build All Projects** in cmake sidebar.

Executable is located in `build/Frackstock.uf2`

<img src="./assets/screenshot-build-all.png" alt="Build-All" style="zoom: 67%;" />
