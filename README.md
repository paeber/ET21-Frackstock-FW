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
At the moment, setup in windows is the easiest. Mac OS and Linux need more manual steps to setup. [Pico C SDK](https://www.raspberrypi.com/documentation/pico-sdk/) is used within VS-Code.

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

### Debug

 - CLion:
 
  1. Download special build of OpenOCD that supports Picoprobe: https://github.com/shabaz123/rp2040tools
	 This is a pre-built version for Windows x64. For different operating system you can build ist yourself: https://github.com/majbthrd/pico-debug/blob/master/howto/openocd.md
  2. Place software at a convenient location. E.g. C:\OpenOCD\RastberryPi -> C:\OpenOCD\RastberryPi\openocd_rp2040\bin\openocd.exe
  3. In CLion under Settings->Build, Executions, Deployment->Embedded Development set your OpenOCD Location
  4. In the opened Frackstock project click on Run->Edit Configuration and add OpenOCD Download & Run
  5. Select your executable, and select under Board config file board/rp2040_cmsis_dap_clion.cfg
  6. Now debuging should be possible
  7. Full tutorial: https://community.element14.com/products/raspberry-pi/b/blog/posts/using-clion-for-easier-coding-with-pi-pico-and-c-c
  
 - VS Code:
 
  1. Full tutorial: https://techniccontroller.com/debugging-of-pico-in-vs-code-on-windows/
  2. Should also work with the pre-built OpenOCD version. Follow steps 1 & 2 of CLion
