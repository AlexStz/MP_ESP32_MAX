# MicroPython for ESP32

# with support up to 16MB of psRAM
[Original **loboris** README (MicroPython_ESP32_psRAM_LoBo)](https://github.com/AlexStz/MP_ESP32_MAX/blob/MicroPython/README.LoBo_original.md)<br>
[Original **m5stack** README](https://github.com/AlexStz/MP_ESP32_MAX/blob/MicroPython/README.m5stack.md)<br>
[Original **ciniml** README (chinese)](https://github.com/AlexStz/MP_ESP32_MAX/blob/MicroPython/README.ciniml.md)<br>
All Wiki links - to **loboris** wiki project.

<br>

> This repository can be used to build MicroPython for ESP32 boards/modules with **psRAM** as well as for ESP32 boards/modules **without psRAM.**<br><br>
> *Building on* **Linux**, **MacOS** *and* **Windows** (including **Linux Subsystem on Windows 10**) *is supported*.<br><br>
> MicroPython works great on ESP32, but the most serious issue is still (as on most other MicroPython boards) limited amount of free memory.<br>
> This repository contains all the tools and sources necessary to **build working MicroPython firmware** which can fully use the advantages of **4MB** (or more) of **psRAM**.<br>
> It is **huge difference** between MicroPython running with **less than 100KB** of free memory and running with **4MB** of free memory.

<br>

ESP32 can use external **SPIRAM** (psRAM) to expand available RAM up to 16MB.

---

[Wiki pages](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo/wiki) with detailed documentation specific to this **MicroPython** port are available.

---

This repository contains all the tools and sources necessary to **build working MicroPython firmware** which can fully use the advantages of **4MB** (or more) of **psRAM**

It is **huge difference** between MicroPython running with **less than 100KB** of free memory and running with **4MB** of free memory.

---

## **The MicroPython firmware is built as esp-idf component**

This means the regular esp-idf **menuconfig** system can be used for configuration. Besides the ESP32 configuration itself, many MicroPython options can also be configured via **menuconfig**.

This way many features not available in standard ESP32 MicroPython are enabled, like unicore/dualcore, all Flash speed/mode options etc. No manual *sdkconfig.h* editing and tweaking is necessary.

---

### Features

* MicroPython core based on latest build from [main Micropython repository](https://github.com/micropython/micropython)
* added changes needed to build for ESP32 with psRAM
* Default configuration has **2MB** of MicroPython heap, **20KB** of MicroPython stack, **~200KB** of free DRAM heap for C modules and functions
* MicroPython can be built in **unicore** (FreeRTOS & MicroPython task running only on the first ESP32 core, or **dualcore** configuration (MicroPython task running on ESP32 **App** core)
* ESP32 Flash can be configured in any mode, **QIO**, **QOUT**, **DIO**, **DOUT**
* **BUILD.sh** script is provided to make **building** MicroPython firmware as **easy** as possible
* Internal Fat filesystem is built with esp-idf **wear leveling** driver, so there is less danger of damaging the flash with frequent writes.
* **SPIFFS** filesystem is supported and can be used instead of FatFS in SPI Flash. Configurable via **menuconfig**
* Flexible automatic and/or manual filesystem configuration
* **sdcard** support is included which uses esp-idf **sdmmc** driver and can work in **SD mode** (*1-bit* and *4-bit*) or in **SPI mode** (sd card can be connected to any pins). For imformation on how to connect sdcard see the documentation.
* Files **timestamp** is correctly set to system time both on internal fat filesysten and on sdcard
* **Native ESP32 VFS** support for spi Flash & sdcard filesystems.
* **RTC Class** is added to machine module, including methods for synchronization of system time to **ntp** server, **deepsleep**, **wakeup** from deepsleep **on external pin** level, ...
* **Time zone** can be configured via **menuconfig** and is used when syncronizing time from NTP server
* Built-in **ymodem module** for fast transfer of text/binary files to/from host
* Some additional frozen modules are added, like **pye** editor, **urequests**, **functools**, **logging**, ...
* **Btree** module included, can be Enabled/Disabled via **menuconfig**
* **_threads** module greatly improved, inter-thread **notifications** and **messaging** included
* **Neopixel** module using ESP32 **RMT** peripheral with many new features
* **DHT** module implemented using ESP32 RMT peripheral
* **1-wire** module implemented using ESP32 RMT peripheral
* **i2c** module uses ESP32 hardware i2c driver - now changed to software i2c driver, maybe revert later...
* **spi** module uses ESP32 hardware spi driver
* **adc** module improved, new functions added
* **pwm** module, ESP32 hardware based
* **timer** module improved, new timer types and features
* **curl** module added, many client protocols including FTP and eMAIL
* **ssh** module added with sftp/scp support and _exec_ function to execute program on server
* **display** module added with full support for spi TFT displays
* **mqtt** module added, implemented in C, runs in separate task
* **mDNS** module added, implemented in C, runs in separate task
* **telnet** module added, connect to **REPL via WiFi** using telnet protocol
* **ftp** server module added, runs as separate ESP32 task
* **GSM/PPPoS** support, connect to the Internet via GSM module
* **OTA Update** supported, various partitions layouts
* **Eclipse** project files included. To include it into Eclipse goto File->Import->Existing Projects into Workspace->Select root directory->[select *MicroPython_BUILD* directory]->Finish. **Rebuild index**.
* **and more...**
---


### How to Build

---

Detailed instructions on **MicroPython** building process are available in the [Wiki](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo/wiki/build).

---


#### Using file systems

Detailed information about using MicroPython file systems are available in the [Wiki](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo/wiki/filesystems).
