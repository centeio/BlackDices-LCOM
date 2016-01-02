#define NDEBUG

#include "test7.h"

int get_ser_conf(unsigned short base_addr, unsigned long *LCR, unsigned long *IER, unsigned long *DLM, unsigned long *DLL) {
	// get LCR
	if(sys_inb(base_addr + LINE_CTRL_REG, LCR) != OK)
		return 1;

	// get IER
	if(sys_inb(base_addr + INT_EN_REG, IER) != OK)
		return 2;

	// set DLAB, get DLM and DLL and reset DLAB
	unsigned long LCR_with_DLAB = ((*LCR) | DLAB);

	if(((*LCR) & DLAB) == 0){	// DLAB not initially set
		if(sys_outb(base_addr + LINE_CTRL_REG, LCR_with_DLAB) != OK)
			return 3;
	}

	if(sys_inb(base_addr + DIVISOR_LATCH_MSB, DLM) != OK)
		return 4;

	if(sys_inb(base_addr + DIVISOR_LATCH_LSB, DLL) != OK)
		return 5;

	if(((*LCR) & DLAB) == 0){	// DLAB not initially set
		if(sys_outb(base_addr + LINE_CTRL_REG, *LCR) != OK)
			return 6;
	}

	return 0;
}

int ser_test_conf(unsigned short base_addr) {
	unsigned long LCR = 0, IER = 0, DLM = 0, DLL = 0;

#if defined(DEBUG)
	base_addr = COM1;
#endif

	if(get_ser_conf(base_addr, &LCR, &IER, &DLM, &DLL) != 0)
		return 1;

	// Line Control Register
	printf("	LCR = 0x%X: ", LCR);
	if((LCR & (BIT(1) | BIT(0))) == 0x00)
		printf("5 bits per char	");
	else
		if((LCR & (BIT(1) | BIT(0))) == BIT(0))
			printf("6 bits per char	");
		else
			if((LCR & (BIT(1) | BIT(0))) == BIT(1))
				printf("7 bits per char	");
			else
				if((LCR & (BIT(1) | BIT(0))) == (BIT(1) | BIT(0)))
					printf("8 bits per char	");

	if ((LCR & BIT(2)) == 0)
		printf("1 stop bit	");
	else	// if bit 2 == 1
		if ((LCR & (BIT(1) | BIT(0))) == 0x00)	// if 5 bits per char
			printf("1 and 1/2 stop bits	");
		else
			printf("2 stop bits	");

	if((LCR & BIT(3)) == 0)
		printf("No parity\n");
	else
		if((LCR & (BIT(5) | BIT(4) | BIT(3))) == BIT(3))
			printf("Odd parity\n");
		else
			if((LCR &  (BIT(5) | BIT(4) | BIT(3))) == (BIT(4) | BIT(3)))
				printf("Even parity\n");
			else
				if((LCR &  (BIT(5) | BIT(4) | BIT(3))) == (BIT(5) | BIT(3)))
					printf("Parity bit is 1 (always)\n");
				else
					if((LCR &  (BIT(5) | BIT(4) | BIT(3))) == (BIT(5) | BIT(4) | BIT(3)))
						printf("Parity bit is 0 (always)\n");

	if((LCR & BIT(6)) == BIT(6))
		printf("Break control on: if on sets serial output to 0 (low)\n");
	else
		printf("Break control off: if on sets serial output to 0 (low)\n");

	if((LCR & BIT(7)) == BIT(7))
		printf("DLAB Mode: Divisor Latch Access\n");
	else
		printf("DLAB Mode: RBR (read) or THR (write)\n");

	// Divisor Latch Registers
	printf("	DLM = 0x%X DLL = 0x%X: bitrate = %d bps\n", DLM, DLL, 115200/((DLM << 8) | DLL));

	// Interrupt Enable Register
	printf("	IER = 0x%X:	\n", IER);
	if((IER & BIT(0)) == BIT(0))
		printf("Received Data Available Interrupt (and timeout interrupts in the FIFO mode) enabled.\n");
	else
		printf("Received Data Available Interrupt (and timeout interrupts in the FIFO mode) disabled.\n");

	if((IER & BIT(1)) == BIT(1))
		printf("Transmitter Holding Register Empty Interrupt enabled.\n");
	else
		printf("Transmitter Holding Register Empty Interrupt disabled.\n");

	if((IER & BIT(2)) == BIT(2))
		printf("Receiver Line Status Interrupt enabled.\n");
	else
		printf("Receiver Line Status Interrupt disabled.\n");

	if((IER & BIT(3)) == BIT(3))
		printf("MODEM Status Interrupt enabled.\n");
	else
		printf("MODEM Status Interrupt disabled.\n");

	if((IER & (BIT(4) | BIT(5) | BIT(6) | BIT(7))) != 0)	// Bits 4 through 7 are always logic 0.
		printf("Probable error getting IER.\n");

	return 0;
}

int ser_conf_set(unsigned short base_addr, unsigned long bits, unsigned long stop,
		long parity, unsigned long rate){

	if(bits < 5 || bits > 8 || stop < 1 || stop > 2 || parity < -1 || parity > 1 || rate > 115200)
	{
		printf("Wrong UART configuration arguments.\n");
		return 1;
	}

	unsigned long lcr = 0, original_lcr = 0;

	if(sys_inb(base_addr + LINE_CTRL_REG, &original_lcr) != OK)
		return 2;

	lcr = (lcr | BIT(7)); // enable Divisor Latch Access

	// maintain bit 6
	if((original_lcr & BIT(6)) == BIT(6))	// if bit 6 == 0 it is already correct
		lcr = (lcr | BIT(6));

	// if parity == -1 (no parity) bit 3 is already 0
	if(parity == 0)	// even parity
		lcr = (lcr | BIT(4) | BIT(3));	// bit 5 is already 0
	else
		if(parity == 1)	// odd parity
			lcr = (lcr | BIT(3));	// bits 5,4 are already 0

	// if stop == 1 bit 2 is already 0
	if(stop == 2)
		lcr = (lcr | BIT(2));

	// if bits == 5 bits 1,0 are already 0
	if(bits == 6)
		lcr = (lcr | BIT(0));	// bit 1 is already 0
	else
		if(bits == 7)
			lcr = (lcr | BIT(1));	// bit 0 is already 0
		else
			if(bits == 8)
				lcr = lcr | BIT(1) | BIT(0);

	// send new LCR with Divisor Latch Access enabled
	if(sys_outb(base_addr + LINE_CTRL_REG, lcr) != OK)
		return 3;

	unsigned long DLL, DLM;
	DLL = (rate & 0xFF);
	DLM = ((rate >> 8) & 0xFF);

	if(sys_outb(base_addr + DIVISOR_LATCH_LSB, DLL) != OK)
		return 4;

	if(sys_outb(base_addr + DIVISOR_LATCH_MSB, DLM) != OK)
		return 5;

	lcr = (lcr & 0x7f);	// 0x7F has all bits set and bit 7 not set

	// send new LCR with Divisor Latch Access disabled
	if(sys_outb(base_addr + LINE_CTRL_REG, lcr) != OK)
		return 6;

	return 0;
}

int ser_test_set(unsigned short base_addr, unsigned long bits, unsigned long stop,
		long parity, unsigned long rate) {
#if defined(DEBUG)
	base_addr = COM1;
	printf("Original configuration:\n");
	ser_test_conf(base_addr);
#endif

	if(ser_conf_set(base_addr, bits, stop, parity, rate) != 0)
		return 1;

#if defined(DEBUG)
	printf("\nNew configuration:\n");
	ser_test_conf(base_addr);
#endif

	return 0;
}

int send_data(unsigned short base_addr, char *string, unsigned int length){
	unsigned long i, LSR;

	do{
		if(sys_inb(base_addr + LINE_STATUS_REG, &LSR) != OK)
			return 1;
	}while(!(LSR & BIT(5)));

	if(sys_outb(base_addr + TRANSMITTER_HOLD_REG, length) != OK)
		return 2;

	for(i=0;i<length && (*string) != 0;i++)
	{
		do{
			if(sys_inb(base_addr + LINE_STATUS_REG, &LSR) != OK)
				return 4;
		}while(!(LSR & BIT(5)));

		if(sys_outb(base_addr + TRANSMITTER_HOLD_REG, *string) != OK)
			return 3;
		string++;
	}

	return 0;
}

int receive_data(unsigned short base_addr, char *string, unsigned long *length){
	unsigned long i, ch, LSR;

	do{
		if(sys_inb(base_addr + LINE_STATUS_REG, &LSR) != OK)
			return 1;
	}while(!(LSR & BIT(0)));

	if(sys_inb(base_addr + RECEIVER_BUF_REG, length) != OK)
		return 2;

	for(i=0;i<*length;i++)
	{
		do{
			if(sys_inb(base_addr + LINE_STATUS_REG, &LSR) != OK)
				return 3;
		}while(!(LSR & BIT(0)));

		if(sys_inb(base_addr + RECEIVER_BUF_REG, &ch) != OK)
			return 4;
		*(string+i)=ch;
	}

	return 0;
}

int ser_test_poll(unsigned short base_addr, unsigned char tx, unsigned long bits,
		unsigned long stop, long parity, unsigned long rate,
		int stringc, char *strings[]) {

	if(ser_test_set(base_addr, bits, stop, parity, rate) != 0)
		return 1;

	if(tx == 0)
	{
		if(send_data(COM1, *strings, stringc) != 0)
			return 2;
		//send_data(COM1, "\n\n\n\n\n\n\n\n					HAPPY NEW YEAR\n\n\n\n\n\n\n\n\n\n\n.");
	}
	else
	{
		unsigned long i, length;
		char string[10000];
		if(receive_data(COM1, &string[0], &length) != 0)
			return 3;

		for(i=0;i<length;i++)
			printf("%c", string[i]);
	}

	return 0;
}

int ser_subscribe_int(unsigned int *hook_id) {
	int ret = 0;

	if (sys_irqsetpolicy(COM1_IRQ_LINE, IRQ_REENABLE | IRQ_EXCLUSIVE, hook_id) != OK)
		ret = 1;

	if (sys_irqenable(hook_id) != OK)
		ret = 1;

	return ret;
}

int ser_unsubscribe_int(unsigned int *hook_id) {
	int ret = 0;

	if (sys_irqdisable(hook_id) != OK)
		ret = 1;

	if (sys_irqrmpolicy(hook_id) != OK)
		ret = 1;

	return ret;
}

unsigned long ser_ih() {
	unsigned long IIR = 0, ch = 0;

	sys_inb(COM1 + INT_ID_REG, &IIR);
	if(IIR & INT_PEND) {
		switch(IIR & INT_ID) {
		case RECEIVER_LINE_STATUS:
			printf("Error detected\n");
		case RECEIVED_DATA_AVAILABLE:
			sys_inb(COM1+RECEIVER_BUF_REG, &ch);
			printf("%c", ch);
		case CHAR_TIMEOUT:
			printf("Char timeout\n");
		case TRANSMITTER_HOLD_REG_EMPTY:
			printf("Transmitter ready\n");
		default:
			break;
		}
	}

	return ch;
}

int ser_test_int() {
	unsigned int hook_id = 7;
	int irq_set = BIT(hook_id);
	int ipc_status;
	message msg;
	int ret = 0;
	unsigned long ch = 1;

	sys_outb(COM1 + INT_EN_REG, 0x0F);	// enable all interrupts

	if (ser_subscribe_int(&hook_id) != 0)
		ret = 1;

	while (ch != 0) { /* You may want to use a different condition */
		/* Get a request message. */printf("1");
		int r = driver_receive(ANY, &msg, &ipc_status);printf("2");
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) {/* subscribed interrupt */
					ch = ser_ih();
				}
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		}
		else { /* received a standard message, not a notification */
			/* no standard messages expected: do nothing */
		}
	}

	if (ser_unsubscribe_int(&hook_id) != 0)
		ret = 1;

	return ret;
}

int ser_test_fifo() {
	/* To be completed */
}
