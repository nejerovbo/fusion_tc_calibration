/*-----------------------------------------------------------------------------
 * atemsys.h
 * Copyright (c) 2009 - 2020 acontis technologies GmbH, Ravensburg, Germany
 * All rights reserved.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Response                  Paul Bussmann
 * Description               atemsys.ko headerfile
 * Note: This header is also included by userspace!

 *  Changes:
 *
 *  V1.0.00 - Inital, PCI/PCIe only.
 *  V1.1.00 - PowerPC tweaks.
 *            Support for SoC devices (no PCI, i.e. Freescale eTSEC).
 *            Support for current linux kernel's (3.0). Removed deprecated code.
 *  V1.2.00 - 64 bit support. Compat IOCTL's for 32-Bit usermode apps.
 *  V1.2.01 - request_irq() sometimes failed -> Map irq to virq under powerpc.
 *  V1.2.02 - Support for current Linux kernel (3.8.0)
 *  V1.2.03 - Support for current Linux kernel (3.8.13) on armv7l (beaglebone)
 *  V1.2.04 - Use dma_alloc_coherent for arm, because of DMA memory corruption on
 *            Xilinx Zynq.
 *  V1.2.05 - OF Device Tree support for Xilinx Zynq (VIRQ mapping)
 *  V1.2.06 - Wrong major version.
 *  V1.2.07 - Tolerate closing, e.g. due to system()-calls.
 *  V1.2.08 - Add VM_DONTCOPY to prevent crash on system()-calls
 *  V1.2.09 - Apply second controller name change in dts (standard GEM driver for Xilinx Zynq) to avoid default driver loading.
 *  V1.2.10 - Removed IO address alignment to support R6040
 *  V1.2.11 - Fix lockup in device_read (tLinkOsIst if NIC in interrupt mode) on dev_int_disconnect
 *  V1.2.12 - Fix underflow in dev_disable_irq() when more than one interrupts pending because of disable_irq_nosync usage
 *  V1.2.13 - Fix usage of x64 PCI physical addresses
 *  V1.2.14 - Changes for using with kernel beginnig from 2.6.18
 *  V1.2.15 - Add udev auto-loading support via DTB
 *  V1.2.16 - Add interrupt mode support for Xenomai 3 (Cobalt)
 *  V1.3.01 - Add IOCTL_MOD_GETVERSION
 *  V1.3.02 - Add support for kernel >= 4.11.00
 *  V1.3.03 - Fix IOCTL_MOD_GETVERSION
 *  V1.3.04 - Fix interrupt deadlock in Xenomai 2
 *  V1.3.05 - Use correct PCI domain
 *  V1.3.06 - Use rtdm_printk for Cobalt, add check if dev_int_disconnect was successful
 *  V1.3.07 - Remove IOCTL_PCI_RELEASE_DEVICE warnings due to untracked IOCTL_PCI_CONF_DEVICE
 *  V1.3.08 - Add support for kernel >= 4.13.00
 *  V1.3.09 - Add support for PRU ICSS in Device Tree
 *  V1.3.10 - Fix compilation on Ubuntu 18.04, Kernel 4.9.90, Xenomai 3.0.6 x64 Cobalt
 *  V1.3.11 - Add enable access to ARM cycle count register(CCNT)
 *  V1.3.12 - Add atemsys API version selection
 *  V1.3.13 - Add ARM64 support
 *  V1.3.14 - Fix edge type interrupt (enabled if Kernel >= 3.4.1, because exported irq_to_desc needed)
 *            Fix Xenomai Cobalt interrupt mode
 *  V1.3.15 - Fix crash while loading kernel module on ARM64
 *            Add support for kernel >= 5.0.00
 *  V1.3.16 - Handle API changes at kernel >= 4.18.00 
 *            Fix ARM DMA allocation for PCIe
 *  V1.4.01 - Register atemsys as Device Tree Ethernet driver "atemsys"
 *            and use Linux PHY and Mdio-Bus Handling
 *  V1.4.02 - Device Tree Ethernet driver improved robustness for unbind linux driver
 *            Fix for kernel >= 5.0.00  with device tree,
 *            Fix ARM/AARCH64 DMA configuration for PCIe and
 *            Fix occasional insmod Kernel Oops 
 *  V1.4.03 - Add log level (insmod atemsys loglevel=6) analog to kernel log level
 *  V1.4.04 - Fix Device Tree Ethernet driver robustness
 *            Add Device Tree Ethernet driver support for ICSS
 *  V1.4.05 - Add IOMMU/Vt-D support
 *  V1.4.06 - Fix IOMMU/Vt-D support for ARM
 *            Fix Mdio-Bus timeout for kernel >= 5.0.00
 *  V1.4.07 - Add support for imx8 / FslFec 64bit
 *  V1.4.08 - Fix Xilinx Ultrascale
 *            Fix cleanup of Device Tree Ethernet driver
 *  V1.4.09 - Add atemsys as PCI driver for Intel, Realtek and Beckhoff
 *            Add memory allocation and mapping on platform / PCI driver device
 *            Fix PHY driver for FslFec 64Bit
 *  V1.4.10 - Fix Device Tree Ethernet driver: Mdio/Phy sup-node, test 4.6.x kernel
 *            Add Device Tree Ethernet driver support for GEM
 *            Fix PCI driver: force DMA to 32 bit
 *  atemsys is shared across EC-Master V2.7+
 *----------------------------------------------------------------------------*/

#ifndef ATEMSYS_H
#define ATEMSYS_H

#include <linux/ioctl.h>
#include <linux/types.h>

#ifndef EC_MAKEVERSION
#define EC_MAKEVERSION(a,b,c,d) (((a)<<24)+((b)<<16)+((c)<<8))
#endif

#define ATEMSYS_VERSION_STR "1.4.10"
#define ATEMSYS_VERSION_NUM  1,4,10
#if (defined ATEMSYS_C)
#define USE_ATEMSYS_API_VERSION EC_MAKEVERSION(1,4,10,0)
#endif

/* support selection */
#if USE_ATEMSYS_API_VERSION >= EC_MAKEVERSION(1,3,4,0)
#define INCLUDE_ATEMSYS_PCI_DOMAIN
#endif

#define DRIVER_SUCCESS  0

/*
 * The major device number. We can't rely on dynamic
 * registration any more, because ioctls need to know
 * it.
 */
#define MAJOR_NUM 101

#if (defined INCLUDE_ATEMSYS_PCI_DOMAIN)
#define ATEMSYS_IOCTL_PCI_FIND_DEVICE           _IOWR(MAJOR_NUM,  0, ATEMSYS_T_PCI_SELECT_DESC)
#define ATEMSYS_IOCTL_PCI_CONF_DEVICE           _IOWR(MAJOR_NUM,  1, ATEMSYS_T_PCI_SELECT_DESC)
#endif
#define ATEMSYS_IOCTL_PCI_RELEASE_DEVICE        _IO(MAJOR_NUM,    2)
#define ATEMSYS_IOCTL_INT_CONNECT               _IOW(MAJOR_NUM,   3, __u32)
#define ATEMSYS_IOCTL_INT_DISCONNECT            _IOW(MAJOR_NUM,   4, __u32)
#define ATEMSYS_IOCTL_INT_INFO                  _IOR(MAJOR_NUM,   5, ATEMSYS_T_INT_INFO)
#define ATEMSYS_IOCTL_MOD_GETVERSION            _IOR(MAJOR_NUM,   6, __u32)
#define ATEMSYS_IOCTL_CPU_ENABLE_CYCLE_COUNT    _IOW(MAJOR_NUM,   7, __u32)
#define ATEMSYS_IOCTL_GET_MAC_INFO              _IOWR(MAJOR_NUM,  8, ATEMSYS_T_MAC_INFO)
#define ATEMSYS_IOCTL_PHY_START_STOP            _IOWR(MAJOR_NUM,  9, ATEMSYS_T_PHY_START_STOP_INFO)
#define ATEMSYS_IOCTL_GET_MDIO_ORDER            _IOWR(MAJOR_NUM, 10, ATEMSYS_T_MDIO_ORDER)
#define ATEMSYS_IOCTL_RETURN_MDIO_ORDER         _IOWR(MAJOR_NUM, 11, ATEMSYS_T_MDIO_ORDER)
#define ATEMSYS_IOCTL_GET_PHY_INFO              _IOWR(MAJOR_NUM, 12, ATEMSYS_T_PHY_INFO)

/* support legacy source code */
#define IOCTL_PCI_FIND_DEVICE           ATEMSYS_IOCTL_PCI_FIND_DEVICE
#define IOCTL_PCI_CONF_DEVICE           ATEMSYS_IOCTL_PCI_CONF_DEVICE
#define IOCTL_PCI_RELEASE_DEVICE        ATEMSYS_IOCTL_PCI_RELEASE_DEVICE
#define IOCTL_INT_CONNECT               ATEMSYS_IOCTL_INT_CONNECT
#define IOCTL_INT_DISCONNECT            ATEMSYS_IOCTL_INT_DISCONNECT
#define IOCTL_INT_INFO                  ATEMSYS_IOCTL_INT_INFO
#define IOCTL_MOD_GETVERSION            ATEMSYS_IOCTL_MOD_GETVERSION
#define IOCTL_CPU_ENABLE_CYCLE_COUNT    ATEMSYS_IOCTL_CPU_ENABLE_CYCLE_COUNT
#define IOCTL_PCI_FIND_DEVICE_v1_3_04   ATEMSYS_IOCTL_PCI_FIND_DEVICE_v1_3_04
#define IOCTL_PCI_CONF_DEVICE_v1_3_04   ATEMSYS_IOCTL_PCI_CONF_DEVICE_v1_3_04
#define USE_PCI_INT                     ATEMSYS_USE_PCI_INT
#define INT_INFO                        ATEMSYS_T_INT_INFO
#define PCI_SELECT_DESC                 ATEMSYS_T_PCI_SELECT_DESC


/*
 * The name of the device driver
 */
#define ATEMSYS_DEVICE_NAME "atemsys"

/* CONFIG_XENO_COBALT/CONFIG_XENO_MERCURY defined in xeno_config.h (may not be available when building atemsys.ko) */
#if (!defined CONFIG_XENO_COBALT) && (!defined CONFIG_XENO_MERCURY) && (defined CONFIG_XENO_VERSION_MAJOR) && (CONFIG_XENO_VERSION_MAJOR >= 3)
#define CONFIG_XENO_COBALT
#endif

/*
 * The name of the device file
 */
#ifdef CONFIG_XENO_COBALT
#define ATEMSYS_FILE_NAME "/dev/rtdm/" ATEMSYS_DEVICE_NAME
#else
#define ATEMSYS_FILE_NAME "/dev/" ATEMSYS_DEVICE_NAME
#endif /* CONFIG_XENO_COBALT */

#define ATEMSYS_PCI_MAXBAR (6)
#define ATEMSYS_USE_PCI_INT (0xFFFFFFFF) /* Query the selected PCI device for the assigned IRQ number */

typedef struct
{
   __u32  dwIOMem;           /* [out] IO Memory of PCI card (physical address) */
   __u32  dwIOLen;           /* [out] Length of the IO Memory area*/
} __attribute__((packed)) ATEMSYS_T_PCI_MEMBAR;

typedef struct
{
   __s32        nVendID;          /* [in] vendor ID */
   __s32        nDevID;           /* [in] device ID */
   __s32        nInstance;        /* [in] instance to look for (0 is the first instance) */
   __s32        nPciBus;          /* [in/out] bus */
   __s32        nPciDev;          /* [in/out] device */
   __s32        nPciFun;          /* [in/out] function */
   __s32        nBarCnt;          /* [out] Number of entries in aBar */
   __u32        dwIrq;            /* [out] IRQ or USE_PCI_INT */
   ATEMSYS_T_PCI_MEMBAR   aBar[ATEMSYS_PCI_MAXBAR]; /* [out] IO memory */
   __s32        nPciDomain;       /* [in/out] domain */
} __attribute__((packed)) ATEMSYS_T_PCI_SELECT_DESC;

typedef struct
{
   __u32        dwInterrupt;
} __attribute__((packed)) ATEMSYS_T_INT_INFO;


/* Defines and declarations for IO controls in v1_3_04 and earliear*/

#define ATEMSYS_IOCTL_PCI_FIND_DEVICE_v1_3_04    _IOWR(MAJOR_NUM, 0, ATEMSYS_T_PCI_SELECT_DESC_v1_3_04)
#define ATEMSYS_IOCTL_PCI_CONF_DEVICE_v1_3_04    _IOWR(MAJOR_NUM, 1, ATEMSYS_T_PCI_SELECT_DESC_v1_3_04)

typedef struct
{
   __s32        nVendID;          /* [in] vendor ID */
   __s32        nDevID;           /* [in] device ID */
   __s32        nInstance;        /* [in] instance to look for (0 is the first instance) */
   __s32        nPciBus;          /* [in/out] bus */
   __s32        nPciDev;          /* [in/out] device */
   __s32        nPciFun;          /* [in/out] function */
   __s32        nBarCnt;          /* [out] Number of entries in aBar */
   __u32        dwIrq;            /* [out] IRQ or USE_PCI_INT */
   ATEMSYS_T_PCI_MEMBAR   aBar[ATEMSYS_PCI_MAXBAR]; /* [out] IO memory */
} __attribute__((packed)) ATEMSYS_T_PCI_SELECT_DESC_v1_3_04;

#if (!defined INCLUDE_ATEMSYS_PCI_DOMAIN)
#define ATEMSYS_IOCTL_PCI_FIND_DEVICE           ATEMSYS_IOCTL_PCI_FIND_DEVICE_v1_3_04
#define ATEMSYS_IOCTL_PCI_CONF_DEVICE           ATEMSYS_IOCTL_PCI_CONF_DEVICE_v1_3_04
#endif


/* must match EC_T_PHYINTERFACE in EcLink.h */
typedef enum _EC_T_PHYINTERFACE_ATEMSYS
{
    eATEMSYS_PHY_FIXED_LINK = 1 << 0,
    eATEMSYS_PHY_MII        = 1 << 1,
    eATEMSYS_PHY_RMII       = 1 << 2,
    eATEMSYS_PHY_GMII       = 1 << 3,
    eATEMSYS_PHY_SGMII      = 1 << 4,
    eATEMSYS_PHY_RGMII      = 1 << 5,
    eATEMSYS_PHY_OSDRIVER   = 1 << 6,

    /* Borland C++ datatype alignment correction */
    eATEMSYS_PHY_BCppDummy  = 0xFFFFFFFF
} ATEMSYS_T_PHYINTERFACE;


#define EC_LINKOS_IDENT_MAX_LEN            0x20  /* must match EcLink.h */
#define PHY_AUTO_ADDR                (__u32) -1  /* must match EcPhy.h */
typedef struct
{
    char                        szIdent[EC_LINKOS_IDENT_MAX_LEN];   /* [out]    Name of Mac e.g. "FslFec" */
    __u32                       dwInstance;                         /* [out]    Number of used Mac (in official order!) */
    __u32                       dwIndex;                            /* [in]     Index of Mac in atemsys handling */
    __u64                       qwRegAddr;                          /* [in]     Hardware register address of mac */
    __u32                       dwRegSize;                          /* [in]     Hardware register size of mac */
    __u32                       dwStatus;                           /* [in]     Status of mac according to device tree */
    ATEMSYS_T_PHYINTERFACE      ePhyMode;                           /* [in]     Phy mac connection mode mii, rmii, rgmii, gmii, sgmii defined in SDK/INC/EcLink.h */
    __u32                       bNoMdioBus;                         /* [in]     Mac don't need to run own Mdio Bus */
    __u32                       dwPhyAddr;                          /* [in]     Address of PHY on mdio bus */
    __u32                       dwErrorCode;                        /* [in]     Error code defined in SDK/INC/EcError.h */
    __u32                       dwReserved[16];
} __attribute__((packed)) ATEMSYS_T_MAC_INFO;

typedef struct
{
    __u32                       dwIndex;                            /* [out]    Index of Mac in atemsys handling */
    __u32                       bInUse;                             /* [in]     Descriptor is in use */
    __u32                       bInUseByIoctl;                      /* [in]     Descriptor is in use by ATEMSYS_IOCTRLs */
    __u32                       bWriteOrder;                        /* [in/out] Mdio operation - write = 1, read = 0 */
    __u16                       wMdioAddr;                          /* [in/out] Current address */
    __u16                       wReg;                               /* [in/out] Current register */
    __u16                       wValue;                             /* [in/out] Current value read or write */
    __u32                       dwTimeoutMsec;                      /* [in]     Timeout in milli seconds */
    __u32                       dwErrorCode;                        /* [in]     Error code defined in SDK/INC/EcError.h */
    __u32                       dwReserved[4];
} __attribute__((packed)) ATEMSYS_T_MDIO_ORDER;

typedef struct
{
    __u32                       dwIndex;                            /* [out]    Index of Mac in atemsys handling */
    __u32                       dwLink;                             /* [in]     Link defined in /linux/phy.h */
    __u32                       dwDuplex;                           /* [in]     Duplex defined in /linux/phy.h (0x00: half, 0x01: full, 0xFF: unknown) */
    __u32                       dwSpeed;                            /* [in]     Speed defined in /linux/phy.h */
    __u32                       bPhyReady;                          /* [in]     Mdio Bus is currently not active */
    __u32                       dwErrorCode;                        /* [in]     Error code defined in SDK/INC/EcError.h */
    __u32                       dwReserved[4];
} __attribute__((packed)) ATEMSYS_T_PHY_INFO;

typedef struct
{
    __u32                       dwIndex;                            /* [out]    Index of Mac in atemsys handling */
    __u32                       bStart;                             /* [out]    Start = 1, stop = 0 */
    __u32                       dwErrorCode;                        /* [in]     Error code defined in SDK/INC/EcError.h */
    __u32                       dwReserved[4];
} __attribute__((packed)) ATEMSYS_T_PHY_START_STOP_INFO;


#endif  /* ATEMSYS_H */

