[<img src="docs/images/logo-quic-on%40h68.png" height="68px" width="393px" alt="Qualcomm Innovation Center" align="right"/>](https://github.com/quic)

# EUD Library:

Embedded USB Debug (EUD) is a powerful feature integrated into Qualcomm chipsets, designed to facilitate efficient debugging and development processes. 

This technology leverages the USB interface to provide developers with a robust and versatile tool for diagnosing and resolving issues within embedded systems.

EUD Supports range of peripherals, including: 
1. CTRL: Control interface for managing and configuring the embedded system.
2. SWD (Serial Wire Debug): A two-pin interface for debugging and programming ARM-based microcontrollers.
3. JTAG (Joint Test Action Group): A standard interface for debugging and testing integrated circuits.
4. TRACE: A high-speed interface for capturing real-time trace data from the embedded system.
5. COM: Communication interface for serial communication with the embedded system.

EUD enables seamless communication between the host system and the embedded device allowing debugging. 

By utilizing the USB interface, EUD offers a standardized and widely supported method for interfacing with embedded systems, 

making it an invaluable asset for engineers working on complex projects.

## EUD Dependencies:
1. libusb-1.0.dll - Libusb library required for USB Driver.
2. libgcc_s_seh-1.dll - GCC runtime library, providing support for exception handling and other runtime features.
3. libwinpthread-1.dll - pthread library for POSIX thread support.
4. libstdc++-6.dll - GNU Standard C++ library providing standard C++ library functions. 

On Windows you'll want to grab:
```
choco install meson ninja pkgconfiglite
```

As well as a version of Visual Studio build tools, which are installed as part of the VS installer workflow.

## Building EUD:
```
# Windows prerequisite
mkdir subprojects
meson wrap install libusb

# On all platforms
meson build
ninja -C build
```

## License:
EUD is licensed on the GPL-2.0 OR BSD 3-clause "New" or "Revised" License.  Check out the [LICENSE](LICENSE.txt) for more details.
