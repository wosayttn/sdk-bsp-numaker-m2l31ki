# NuMaker-M2L31KI

## 1. Introduction

The NuMicro M2L31 series is based on Arm Cortex-M23 core at Armv8-M architecture with a single cycle hardware multiplier/divider. It runs up to 72 MHz and features 64 to 512 Kbytes ReRAM, 40 to 168 Kbytes SRAM, 1.71V to 3.6V operating voltage, -40°C to 105°C wide operating temperature, a variety of packages choices, and excellent high immunity characteristics by 4 kV ESD HBM and 4.4 kV EFT.

ReRAM (Resistive Random-Access Memory) is a type of non-volatile memory that achieves digital data storage by altering the resistance state of its components through the application of an external voltage. It boasts three major characteristics: fast read/write speeds, low power consumption, and superior durability, making it hailed as the next-generation embedded universal memory. Unlike embedded flash memory, ReRAM doesn't require an page erase operation before writing, resulting in faster write speeds and a more straightforward and speedy operation similar to EEPROM. Furthermore, ReRAM consumes less energy for storing each bit compared to what is needed in flash memory. Additionally, because each storage unit can be individually set or reset, it offers greater endurance than flash memory.

### 1.1 MCU specification

|  | Features |
| -- | -- |
| MCU | M2L31KIDAE |
| Operation frequency | 72MHz |
| Embedded Flash size | 512KB, Dual Bank |
| SRAM size | 168kB |
| Crypto engine | TRNG, AES, CRC cipher accelerator |

### 1.2 Interface

| Interface |
| -- |
| Arduino UNO |
| USB 1.1 Type-C ports |

### 1.3 On-board devices

| Device | Description | Driver supporting status |
| -- | -- | -- |
| N/A | N/A | N/A |

## 2. Supported compiler

Support GCC, MDK5 IDE/compilers. More information of these compiler version as following:

| IDE/Compiler  | Tested version                    |
| ---------- | ------------------------------------ |
| MDK5       | 5.29                                 |
| GCC        | Arm Embedded Toolchain 10.3-2021.10 (Env 1.3.5 embedded version)|

Notice:
(1) Please install Nu-Link_Keil_Driver for development.

## 3. Program firmware

### Step 1

At first, you need to configure ICESW2 switch on the NuMaker-M2L31KI board. Set the four switches to ‘ON’ position. After the configuration is done, connect the NuMaker-M2L31KI board and your computer using the USB Type-C cable. After that, window manager will show a ‘NuMicro MCU’ virtual disk. Finally, you will use this virtual disk to burn firmware.

### Step 2

A simple firmware burning method is that you can drag and drop the binary image file to NuMicro MCU virtual disk or copy the binary file to NuMicro MCU disk to burn firmware.

## 4. Test

You can use Tera Term terminate emulator (or other software) to type commands of RTT. All parameters of serial communication are shown in below image. Here, you can find out the corresponding port number of Nuvoton Virtual Com Port in window device manager.

## 5. Purchase

* [Where to buy][1]

## 6. Resources

* [M2L31 Series Introduction][2]

  [1]: https://www.nuvoton.com.cn/products/microcontrollers/arm-cortex-m23-mcus/m2l31-series/?tab=5
  [2]: https://www.nuvoton.com.cn/products/microcontrollers/arm-cortex-m23-mcus/m2l31-series/?tab=1
