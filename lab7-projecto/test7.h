//#include <minix/syslib.h>
#include <minix/drivers.h>

// GENERAL DEFINES
#define BIT(n) (0x01<<(n))

// UARTs ADDRESSES
#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

// UARTs IRQ LINES
#define COM1_IRQ_LINE 4
#define COM2_IRQ_LINE 3
#define COM3_IRQ_LINE 4
#define COM4_IRQ_LINE 3

// UART BUFFERS AND REGISTERS
#define RECEIVER_BUF_REG 0
#define TRANSMITTER_HOLD_REG 0
#define DIVISOR_LATCH_LSB 0
#define INT_EN_REG 1
#define DIVISOR_LATCH_MSB 1
#define INT_ID_REG 2
#define FIFO_CTRL_REG 2
#define LINE_CTRL_REG 3
#define MODEM_CTRL_REG 4
#define LINE_STATUS_REG 5
#define MODEM_STATUS_REG 6
#define SCRATCHPAD_REG 7

// UART SPECIFIC BITS
#define DLAB BIT(7)

// INTERRUPT INDICATORS
#define INT_PEND 0	// bit 0 = 0
#define INT_ID BIT(3) | BIT(2) | BIT(1)
#define RECEIVER_LINE_STATUS BIT(2) | BIT(1)
#define RECEIVED_DATA_AVAILABLE BIT(2)
#define CHAR_TIMEOUT BIT(3) | BIT(2)
#define TRANSMITTER_HOLD_REG_EMPTY BIT(1)

int ser_test_conf(unsigned short base_addr);

int ser_test_set(unsigned short base_addr, unsigned long bits, unsigned long stop,
		long parity, /* -1: none, 0: even, 1: odd */	unsigned long rate);

int ser_test_poll(unsigned short base_addr, unsigned char tx, unsigned long bits,
		unsigned long stop, long parity, unsigned long rate,
		int stringc, char *strings[]);

int ser_test_int() ;

int ser_test_fifo();
