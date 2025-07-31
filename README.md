# STM32F411 Discovery USB HID Keyboard

![STM32F411](https://img.shields.io/badge/STM32F411-Discovery-03234B?logo=stmicroelectronics&logoColor=white)
![USB](https://img.shields.io/badge/USB-HID_Keyboard-2496ED?logo=usb&logoColor=white)
![HAL](https://img.shields.io/badge/STM32-HAL_Library-03234B?logo=stmicroelectronics)

<img src="https://github.com/user-attachments/assets/77085677-584e-405d-8764-d9902ea89b98" width="750" alt="BFEE5AF6-5B92-4060-BFC3-6C82D1C010E6_1_105_c">


## Key Features
- ⌨️ **Full USB HID Keyboard** compliance with boot protocol
- 🔵 **Blue USER button** triggers Page Down key presses
- ⚡ **96MHz system clock** via 8MHz HSE + PLL
- 📝 **8-byte HID report** format (modifiers + keycodes)
- 🔄 **Debounced input** with 50ms delay

## Hardware Setup
| Function       | Pin  | Board Location | Badge |
|----------------|------|----------------|-------|
| USER Button    | PA0  | Blue button    | ![GPIO](https://img.shields.io/badge/GPIO-PA0-yellow) |
| USB FS         | PA11/PA12 | CN5 connector | ![USB](https://img.shields.io/badge/USB-Full_Speed-blue) |

## Key Press Implementation
```c
// Press Page Down
keys_buffer[3] = 0x4E;  // HID keycode for Page Down
USBD_HID_SendReport(&hUsbDeviceFS, keys_buffer, 8);

// Release all keys
keys_buffer[3] = 0x00;
USBD_HID_SendReport(&hUsbDeviceFS, keys_buffer, 8);
