/**
  ******************************************************************************
  * @file    usbd_hid.c
  * @author  MCD Application Team
  * @brief   This file provides the HID core functions.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  * @verbatim
  *
  *          ===================================================================
  *                                HID Class  Description
  *          ===================================================================
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */
/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_ClassTypeDef USBD_HID =
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL,              /* EP0_TxSent */
  NULL,              /* EP0_RxReady */
  USBD_HID_DataIn,   /* DataIn */
  NULL,              /* DataOut */
  NULL,              /* SOF */
  NULL,
  NULL,
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_HID_GetHSCfgDesc,
  USBD_HID_GetFSCfgDesc,
  USBD_HID_GetOtherSpeedCfgDesc,
  USBD_HID_GetDeviceQualifierDesc,
#endif /* USE_USBD_COMPOSITE  */
};

#ifndef USE_USBD_COMPOSITE
/* USB HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09,                                               /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                        /* bDescriptorType: Configuration */
  USB_HID_CONFIG_DESC_SIZ,                            /* wTotalLength: Bytes returned */
  0x00,
  0x01,                                               /* bNumInterfaces: 1 interface */
  0x01,                                               /* bConfigurationValue: Configuration value */
  0x00,                                               /* iConfiguration: Index of string descriptor
                                                         describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xE0,                                               /* bmAttributes: Bus Powered according to user configuration */
#else
  0xA0,                                               /* bmAttributes: Bus Powered according to user configuration */
#endif /* USBD_SELF_POWERED */
  USBD_MAX_POWER,                                     /* MaxPower (mA) */

  /************** Descriptor of Joystick Mouse interface ****************/
  /* 09 */
  0x09,                                               /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                            /* bDescriptorType: Interface descriptor type */
  0x00,                                               /* bInterfaceNumber: Number of Interface */
  0x00,                                               /* bAlternateSetting: Alternate setting */
  0x01,                                               /* bNumEndpoints */
  0x03,                                               /* bInterfaceClass: HID */
  0x01,                                               /* bInterfaceSubClass : 1=BOOT, 0=no boot */
  0x01,                                               /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
  0,                                                  /* iInterface: Index of string descriptor */
  /******************** Descriptor of Joystick Mouse HID ********************/
  /* 18 */
  0x09,                                               /* bLength: HID Descriptor size */
  HID_DESCRIPTOR_TYPE,                                /* bDescriptorType: HID */
  0x11,                                               /* bcdHID: HID Class Spec release number */
  0x01,
  0x00,                                               /* bCountryCode: Hardware target country */
  0x01,                                               /* bNumDescriptors: Number of HID class descriptors to follow */
  0x22,                                               /* bDescriptorType */
  HID_MOUSE_REPORT_DESC_SIZE,                         /* wItemLength: Total length of Report descriptor */
  0x00,
  /******************** Descriptor of Mouse endpoint ********************/
  /* 27 */
  0x07,                                               /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                             /* bDescriptorType:*/

  HID_EPIN_ADDR,                                      /* bEndpointAddress: Endpoint Address (IN) */
  0x03,                                               /* bmAttributes: Interrupt endpoint */
  HID_EPIN_SIZE,                                      /* wMaxPacketSize: 4 Bytes max */
  0x00,
  HID_FS_BINTERVAL,                                   /* bInterval: Polling Interval */
  /* 34 */
};
#endif /* USE_USBD_COMPOSITE  */

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_Desc[USB_HID_DESC_SIZ] __ALIGN_END =
{
  /* 18 */
  0x09,                                               /* bLength: HID Descriptor size */
  HID_DESCRIPTOR_TYPE,                                /* bDescriptorType: HID */
  0x11,                                               /* bcdHID: HID Class Spec release number */
  0x01,
  0x00,                                               /* bCountryCode: Hardware target country */
  0x01,                                               /* bNumDescriptors: Number of HID class descriptors to follow */
  0x22,                                               /* bDescriptorType */
  HID_MOUSE_REPORT_DESC_SIZE,                         /* wItemLength: Total length of Report descriptor */
  0x00,
};

#ifndef USE_USBD_COMPOSITE
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};
#endif /* USE_USBD_COMPOSITE  */

/*  HID keyboard report descriptor */
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =
{
	     0x05    ,//bSize: 0x01, bType: Global, bTag: Usage Page
	     0x01    ,//Usage Page(Generic Desktop Controls )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x06    ,//Usage(Keyboard)
	     0xA1    ,//bSize: 0x01, bType: Main, bTag: Collection
	     0x01    ,//Collection(Application )
	     0x85    ,//bSize: 0x01, bType: Global, bTag: Report ID
	     0x01    ,//Report ID(0x1 )
	     0x05    ,//bSize: 0x01, bType: Global, bTag: Usage Page
	     0x07    ,//Usage Page(Keyboard/Keypad )
	     0x19    ,//bSize: 0x01, bType: Local, bTag: Usage Minimum
	     0xE0    ,//Usage Minimum(0xE0 )
	     0x29    ,//bSize: 0x01, bType: Local, bTag: Usage Maximum
	     0xE7    ,//Usage Maximum(0xE7 )
	     0x15    ,//bSize: 0x01, bType: Global, bTag: Logical Minimum
	     0x00    ,//Logical Minimum(0x0 )
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x01    ,//Logical Maximum(0x1 )
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x01    ,//Report Size(0x1 )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x08    ,//Report Count(0x8 )
	     0x81    ,//bSize: 0x01, bType: Main, bTag: Input
	     0x02    ,//Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x08    ,//Report Size(0x8 )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x01    ,//Report Count(0x1 )
	     0x81    ,//bSize: 0x01, bType: Main, bTag: Input
	     0x01    ,//Input(Constant, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
	     0x05    ,//bSize: 0x01, bType: Global, bTag: Usage Page
	     0x07    ,//Usage Page(Keyboard/Keypad )
	     0x19    ,//bSize: 0x01, bType: Local, bTag: Usage Minimum
	     0x00    ,//Usage Minimum(0x0 )
	     0x29    ,//bSize: 0x01, bType: Local, bTag: Usage Maximum
	     0x65    ,//Usage Maximum(0x65 )
	     0x15    ,//bSize: 0x01, bType: Global, bTag: Logical Minimum
	     0x00    ,//Logical Minimum(0x0 )
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x65    ,//Logical Maximum(0x65 )
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x08    ,//Report Size(0x8 )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x05    ,//Report Count(0x5 )
	     0x81    ,//bSize: 0x01, bType: Main, bTag: Input
	     0x00    ,//Input(Data, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
	     0xC0    ,//bSize: 0x00, bType: Main, bTag: End Collection
	     0x05    ,//bSize: 0x01, bType: Global, bTag: Usage Page
	     0x0C    ,//Usage Page(Consumer )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x01    ,//Usage(Consumer Control)
	     0xA1    ,//bSize: 0x01, bType: Main, bTag: Collection
	     0x01    ,//Collection(Application )
	     0x85    ,//bSize: 0x01, bType: Global, bTag: Report ID
	     0x02    ,//Report ID(0x2 )
	     0x19    ,//bSize: 0x01, bType: Local, bTag: Usage Minimum
	     0x00    ,//Usage Minimum(0x0 )
	     0x2A    ,//bSize: 0x02, bType: Local, bTag: Usage Maximum
	     0x3C,
	     0x02 ,//Usage Maximum(0x23C )
	     0x15    ,//bSize: 0x01, bType: Global, bTag: Logical Minimum
	     0x00    ,//Logical Minimum(0x0 )
	     0x26    ,//bSize: 0x02, bType: Global, bTag: Logical Maximum
	     0x3C,
	     0x02 ,//Logical Maximum(0x23C )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x01    ,//Report Count(0x1 )
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x10    ,//Report Size(0x10 )
	     0x81    ,//bSize: 0x01, bType: Main, bTag: Input
	     0x00    ,//Input(Data, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
	     0xC0    ,//bSize: 0x00, bType: Main, bTag: End Collection
	     0x05    ,//bSize: 0x01, bType: Global, bTag: Usage Page
	     0x01    ,//Usage Page(Generic Desktop Controls )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x80    ,//Usage(System Control)
	     0xA1    ,//bSize: 0x01, bType: Main, bTag: Collection
	     0x01    ,//Collection(Application )
	     0x85    ,//bSize: 0x01, bType: Global, bTag: Report ID
	     0x03    ,//Report ID(0x3 )
	     0x19    ,//bSize: 0x01, bType: Local, bTag: Usage Minimum
	     0x81    ,//Usage Minimum(0x81 )
	     0x29    ,//bSize: 0x01, bType: Local, bTag: Usage Maximum
	     0x83    ,//Usage Maximum(0x83 )
	     0x15    ,//bSize: 0x01, bType: Global, bTag: Logical Minimum
	     0x00    ,//Logical Minimum(0x0 )
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x01    ,//Logical Maximum(0x1 )
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x01    ,//Report Size(0x1 )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x03    ,//Report Count(0x3 )
	     0x81    ,//bSize: 0x01, bType: Main, bTag: Input
	     0x02    ,//Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x05    ,//Report Count(0x5 )
	     0x81    ,//bSize: 0x01, bType: Main, bTag: Input
	     0x01    ,//Input(Constant, Array, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
	     0xC0    ,//bSize: 0x00, bType: Main, bTag: End Collection
	     0x06    ,//bSize: 0x02, bType: Global, bTag: Usage Page
	     0x01,
	     0xFF ,//Usage Page(Undefined )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x01    ,//Usage(1)
	     0xA1    ,//bSize: 0x01, bType: Main, bTag: Collection
	     0x01    ,//Collection(Application )
	     0x85    ,//bSize: 0x01, bType: Global, bTag: Report ID
	     0x04    ,//Report ID(0x4 )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x01    ,//Report Count(0x1 )
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x08    ,//Report Size(0x8 )
	     0x15    ,//bSize: 0x01, bType: Global, bTag: Logical Minimum
	     0x01    ,//Logical Minimum(0x1 )
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x0A    ,//Logical Maximum(0xA )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x20    ,//Usage(32)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x23    ,//Usage(35)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x4F    ,//Logical Maximum(0x4F )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x21    ,//Usage(33)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x30    ,//Logical Maximum(0x30 )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x22    ,//Usage(34)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x03    ,//Report Count(0x3 )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x24    ,//Usage(36)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0xC0    ,//bSize: 0x00, bType: Main, bTag: End Collection
	     0x06    ,//bSize: 0x02, bType: Global, bTag: Usage Page
	     0x01,
	     0xFF ,//Usage Page(Undefined )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x01    ,//Usage(1)
	     0xA1    ,//bSize: 0x01, bType: Main, bTag: Collection
	     0x01    ,//Collection(Application )
	     0x85    ,//bSize: 0x01, bType: Global, bTag: Report ID
	     0x05    ,//Report ID(0x5 )
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x01    ,//Report Count(0x1 )
	     0x75    ,//bSize: 0x01, bType: Global, bTag: Report Size
	     0x08    ,//Report Size(0x8 )
	     0x15    ,//bSize: 0x01, bType: Global, bTag: Logical Minimum
	     0x01    ,//Logical Minimum(0x1 )
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x0A    ,//Logical Maximum(0xA )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x20    ,//Usage(32)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x23    ,//Usage(35)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x4F    ,//Logical Maximum(0x4F )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x21    ,//Usage(33)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x25    ,//bSize: 0x01, bType: Global, bTag: Logical Maximum
	     0x30    ,//Logical Maximum(0x30 )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x22    ,//Usage(34)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0x95    ,//bSize: 0x01, bType: Global, bTag: Report Count
	     0x03    ,//Report Count(0x3 )
	     0x09    ,//bSize: 0x01, bType: Local, bTag: Usage
	     0x24    ,//Usage(36)
	     0xB1    ,//bSize: 0x01, bType: Main, bTag: Feature
	     0x03    ,//Feature(Constant, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Non VolatileBit Field)
	     0xC0    //bSize: 0x00, bType: Main, bTag: End Collection
};
//End Change the HID report descriptor

static uint8_t HIDInEpAdd = HID_EPIN_ADDR;

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

  USBD_HID_HandleTypeDef *hhid;

  hhid = (USBD_HID_HandleTypeDef *)USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

  if (hhid == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)hhid;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  HIDInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    pdev->ep_in[HIDInEpAdd & 0xFU].bInterval = HID_HS_BINTERVAL;
  }
  else   /* LOW and FULL-speed endpoints */
  {
    pdev->ep_in[HIDInEpAdd & 0xFU].bInterval = HID_FS_BINTERVAL;
  }

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, HIDInEpAdd, USBD_EP_TYPE_INTR, HID_EPIN_SIZE);
  pdev->ep_in[HIDInEpAdd & 0xFU].is_used = 1U;

  hhid->state = USBD_HID_IDLE;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_HID_DeInit
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  HIDInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close HID EPs */
  (void)USBD_LL_CloseEP(pdev, HIDInEpAdd);
  pdev->ep_in[HIDInEpAdd & 0xFU].is_used = 0U;
  pdev->ep_in[HIDInEpAdd & 0xFU].bInterval = 0U;

  /* Free allocated memory */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_StatusTypeDef ret = USBD_OK;
  uint16_t len;
  uint8_t *pbuf;
  uint16_t status_info = 0U;

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case USBD_HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case USBD_HID_REQ_GET_PROTOCOL:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->Protocol, 1U);
          break;

        case USBD_HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case USBD_HID_REQ_GET_IDLE:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->IdleState, 1U);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == HID_REPORT_DESC)
          {
            len = MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength);
            pbuf = HID_MOUSE_ReportDesc;
          }
          else if ((req->wValue >> 8) == HID_DESCRIPTOR_TYPE)
          {
            pbuf = USBD_HID_Desc;
            len = MIN(USB_HID_DESC_SIZ, req->wLength);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
          }
          (void)USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}


/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @param  ClassId: The Class ID
  * @retval status
  */
#ifdef USE_USBD_COMPOSITE
uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len, uint8_t ClassId)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassDataCmsit[ClassId];
#else
uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#endif /* USE_USBD_COMPOSITE */

  if (hhid == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  HIDInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, ClassId);
#endif /* USE_USBD_COMPOSITE */

  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == USBD_HID_IDLE)
    {
      hhid->state = USBD_HID_BUSY;
      (void)USBD_LL_Transmit(pdev, HIDInEpAdd, report, len);
    }
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_HID_GetPollingInterval
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval;

  /* HIGH-speed endpoints */
  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Sets the data transfer polling interval for high speed transfers.
     Values between 1..16 are allowed. Values correspond to interval
     of 2 ^ (bInterval-1). This option (8 ms, corresponds to HID_HS_BINTERVAL */
    polling_interval = (((1U << (HID_HS_BINTERVAL - 1U))) / 8U);
  }
  else   /* LOW and FULL-speed endpoints */
  {
    /* Sets the data transfer polling interval for low and full
    speed transfers */
    polling_interval =  HID_FS_BINTERVAL;
  }

  return ((uint32_t)(polling_interval));
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_HID_GetCfgFSDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpDesc = USBD_GetEpDesc(USBD_HID_CfgDesc, HID_EPIN_ADDR);

  if (pEpDesc != NULL)
  {
    pEpDesc->bInterval = HID_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_GetCfgHSDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetHSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpDesc = USBD_GetEpDesc(USBD_HID_CfgDesc, HID_EPIN_ADDR);

  if (pEpDesc != NULL)
  {
    pEpDesc->bInterval = HID_HS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetOtherSpeedCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpDesc = USBD_GetEpDesc(USBD_HID_CfgDesc, HID_EPIN_ADDR);

  if (pEpDesc != NULL)
  {
    pEpDesc->bInterval = HID_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);
  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_HID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId])->state = USBD_HID_IDLE;

  return (uint8_t)USBD_OK;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_HID_DeviceQualifierDesc);

  return USBD_HID_DeviceQualifierDesc;
}
#endif /* USE_USBD_COMPOSITE  */
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

