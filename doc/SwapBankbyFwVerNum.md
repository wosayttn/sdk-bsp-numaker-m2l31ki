# Custom Loader

This port provides a straightforward implementation of a custom loader. In the Reset_Handler, the program determines the version numbers on Bank0 and Bank1, and decides which bank to set as the booting bank based on the version number's magnitude.

Note: Since the MCU defaults to using Bank0 as the booting bank, it is essential to ensure that valid data is written to Bank0 during the firmware updating stage and power interruption should be avoided.

## Booting Log

FirmwareA Version Number: `0x20240326`

FirmwareB Version Number: `0x20240327`

The booting log shown as below after programming VerA into Bank#0 and VerB into Bank#1.

```bash
 \ | /
- RT -     Thread Operating System
 / | \     5.0.0 build Mar 26 2024 16:00:29
 2006 - 2022 Copyright by RT-Thread team
Set pdma0 SCATBA address to 0x20000000.
Set lppdma SCATBA address to 0x28000000.
Executed Bank# is 1.
[D/FAL] (fal_flash_init:47) Flash device |                    APROM | addr: 0x00000000 | len: 0x00080000 | blk_size: 0x00001000 |initialized finish.
[D/FAL] (fal_flash_init:47) Flash device |                    LDROM | addr: 0x0f100000 | len: 0x0f102000 | blk_size: 0x00001000 |initialized finish.
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name   | flash_dev |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | loader | LDROM     | 0x00000000 | 0x00001000 |
[I/FAL] | bank   | APROM     | 0x00040000 | 0x00040000 |
[I/FAL] =============================================================
[I/FAL] RT-Thread Flash Abstraction Layer initialize success.
msh >
```

The booting log shown as below after programming VerA into Bank#1 and VerB into Bank#0.

```bash
 \ | /
- RT -     Thread Operating System
 / | \     5.0.0 build Mar 26 2024 16:00:29
 2006 - 2022 Copyright by RT-Thread team
Set pdma0 SCATBA address to 0x20000000.
Set lppdma SCATBA address to 0x28000000.
Executed Bank# is 0.
[D/FAL] (fal_flash_init:47) Flash device |                    APROM | addr: 0x00000000 | len: 0x00080000 | blk_size: 0x00001000 |initialized finish.
[D/FAL] (fal_flash_init:47) Flash device |                    LDROM | addr: 0x0f100000 | len: 0x0f102000 | blk_size: 0x00001000 |initialized finish.
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name   | flash_dev |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | loader | LDROM     | 0x00000000 | 0x00001000 |
[I/FAL] | bank   | APROM     | 0x00040000 | 0x00040000 |
[I/FAL] =============================================================
[I/FAL] RT-Thread Flash Abstraction Layer initialize success.
msh >
```

## Define Firmware Version Number

You can maintain the version number in `<rt-thread>/bsp/nuvoton/numaker-m2l31ki/board/custom_loader.c` source

```c
/* Use an unsigned integer number as version number. */
#define DEF_FW_VER_NUM     0x20240327

typedef struct
{
#define DEF_MAGIC_NUM      0xA5A5A5A5
    uint32_t m_u32MagicNum1;
    uint32_t m_u32FwVer;
    uint32_t m_u32Reserved;
    uint32_t m_u32MagicNum2;
} customer_loader_info;
```
