#ifndef _LCOM_8042_H_
#define _LCOM_8042_H_

// GENERAL DEFINES
#define BIT(n) (0x01<<(n))
#define DELAY_US 20000

// KBC BUFFERS AND REGISTERS
#define STAT_REG 0x64
#define KBC_CMD_REG 0x64
#define IN_BUF 0x60
#define OUT_BUF 0x60

// KBC COMMANDS AND RETURNS
#define ACK 0xFA
#define ERROR_CODE 0xFC
#define RESEND_CODE 0xFE
#define IBF BIT(1)
#define OBF BIT(0)

// PS/2 MOUSE
#define MOUSE_IRQ_LINE 12
#define WRITE_BYTE_TO_MOUSE 0xD4
#define ENABLE_MOUSE 0xF4
#define DISABLE_MOUSE 0xF5
#define SET_DEFAULT 0xF6
#define STATUS_REQUEST 0xE9

#endif
