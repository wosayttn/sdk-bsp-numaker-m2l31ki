__STACK_SIZE = 0x000001000;
__HEAP_SIZE  = 0x000000000;

/* System memory brief */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x80000    /* 512k */
  SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x2A000    /* 168k */
  LPSRAM (rwx): ORIGIN = 0x28000000, LENGTH = 0x2000     /* 8k   */
}


/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ITCM and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */

ENTRY(Reset_Handler)

SECTIONS
{
  .text :
  {
    KEEP(*(.vectors))
    KEEP(*(CLINFO))
    *custom_loader.o(.text* .rodata*)
    *startup_M2L31.o( .text* .rodata*)
    *(.text*)

    KEEP(*(.init))
    KEEP(*(.fini))

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)

    /* section information for finsh shell */
    . = ALIGN(4);
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;
    . = ALIGN(4);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    . = ALIGN(4);

    /* section information for initial. */
    . = ALIGN(4);
    __rt_init_start = .;
    KEEP(*(SORT(.rti_fn*)))
    __rt_init_end = .;
    . = ALIGN(4);

    /* section information for utest */
    . = ALIGN(4);
    __rt_utest_tc_tab_start = .;
    KEEP(*(UtestTcTab))
    __rt_utest_tc_tab_end = .;

    /* section information for cherryusb usb host class */
    . = ALIGN(4);
    __usbh_class_info_start__ = .;
    KEEP(*(.usbh_class_info))
    __usbh_class_info_end__ = .;

    KEEP(*(.eh_frame*))
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } > FLASH

  .zero.table :
  {
    . = ALIGN(4);
    __zero_table_start__ = .;
    LONG (__bss_start__)
    LONG ((__bss_end__ - __bss_start__) / 4)
    LONG (__lpsram_zi_start__)
    LONG ((__lpsram_zi_end__ - __lpsram_zi_start__) / 4)
    __zero_table_end__ = .;
  } > FLASH

  .copy.table :
  {
    . = ALIGN(4);
    __copy_table_start__ = .;

    /* Section to be copied - part 1: RW data for SRAM */
    LONG (__eitcm)
    LONG (__data_start__)
    LONG ((__data_end__ - __data_start__) / 4)

    __copy_table_end__ = .;
  } > FLASH

  /**
   * Location counter can end up 2 byte aligned with narrow Thumb code but
   * __etext is assumed by startup code to be the LMA of a section
   * which must be 4byte aligned
   */
  __etext = ALIGN (4);
  __eitcm = __etext;

  .stack (ORIGIN(SRAM)) (NOLOAD) :
  {
    . = ALIGN(8);
    __StackLimit = .;
    . = . + __STACK_SIZE;
    . = ALIGN(8);
    __StackTop = .;
  } > SRAM
  PROVIDE(__stack = __StackTop);

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    . = . + __HEAP_SIZE;
    . = ALIGN(8);
    __HeapLimit = .;
  } > SRAM

  .data : AT(__eitcm)
  {
    __data_start__ = .;
    *(vtable)
    *(.data)
    *(.data.*)

    . = ALIGN(4);
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.jcr*))
    . = ALIGN(4);
    /* All data end */
    __data_end__ = .;
  } > SRAM

  .sram :
  {
    . = ALIGN(16);
    *(.bss.tensor_arena)
  } > SRAM

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > SRAM AT > SRAM

  /* __esram_data = __eitcm + (__data_end__ - __data_start__); */

  .lpsram (NOLOAD) :
  {
    . = ALIGN(4);
    __lpsram_zi_start__ = .;
    *(.lpSram)
    __lpsram_zi_end__ = .;
  } > LPSRAM AT > LPSRAM

  /* Because stack placed in DTCM, cannot check if data + heap + stack exceeds SRAM limit */
  /*ASSERT(__StackLimit >= __HeapLimit, "region SRAM overflowed with stack")*/
}
