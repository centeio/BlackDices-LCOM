#include "test6.h"

void enable_irq();
void disable_irq();

unsigned long read_reg(unsigned long reg)
{
	unsigned long reg_value = 0;

	sys_outb(RTC_ADDR_REG, reg);
	sys_inb(RTC_DATA_REG, &reg_value);

	return reg_value;
}

int rtc_test_conf() {
	unsigned long reg = 0;

	reg = read_reg(RTC_REG_A);
	printf("REG A: 0x%x\n", reg);
	if((reg & BIT(7)) >> 7)
		printf("An update is in progress.\n\n");
	else
		printf("No update is in progress.\n\n");

	reg = read_reg(RTC_REG_B);
	printf("REG B: 0x%x\n", reg);
	if((reg & BIT(7)) >> 7)
		printf("Time/date register update inhibited.\n");
	else
		printf("Time/date register update activated.\n");
	if((reg & BIT(6)) >> 6)
		printf("Periodic interrupt enabled.\n");
	else
		printf("Periodic interrupt disabled.\n");
	if((reg & BIT(5)) >> 5)
		printf("Alarm interrupt enabled.\n");
	else
		printf("Alarm interrupt disabled.\n");
	if((reg & BIT(4)) >> 4)
		printf("Update interrupt enabled.\n");
	else
		printf("Update interrupt disabled.\n");
	if((reg & BIT(3)) >> 3)
		printf("Square-wave generation enabled.\n");
	else
		printf("Square-wave generation disabled.\n");
	if((reg & BIT(2)) >> 2)
		printf("Time, alarm and date registers in binary.\n");
	else
		printf("Time, alarm and date registers in BCD.\n");
	if((reg & BIT(1)) >> 1)
		printf("24 hours format.\n");
	else
		printf("12 hours format.\n");
	if((reg & BIT(0)) >> 0)
		printf("Daylight Savings Time enabled.\n\n");
	else
		printf("Daylight Savings Time disabled.\n\n");

	reg = read_reg(RTC_REG_C);
	printf("REG C: 0x%x\n", reg);
	if((reg & BIT(7)) >> 7)
		printf("IRQ line enabled.\n");
	else
		printf("IRQ line disabled.\n");
	if((reg & BIT(6)) >> 6)
		printf("Periodic interrupt pending.\n");
	else
		printf("No periodic interrupt pending.\n");
	if((reg & BIT(5)) >> 5)
		printf("Alarm interrupt pending.\n");
	else
		printf("No alarm interrupt pending.\n");
	if((reg & BIT(4)) >> 4)
		printf("Update interrupt pending.\n\n");
	else
		printf("No update interrupt pending.\n\n");

	reg = read_reg(RTC_REG_D);
	printf("REG D: 0x%x\n", reg);
	if((reg & BIT(7)) >> 7)
		printf("Valid RAM/time.\n\n");
	else
		printf("Invalid RAM/time.\n\n");

	return 0;
}

void wait_valid_rtc() {
	unsigned long regA = 0;
	do {
		disable_irq();
		regA = read_reg(RTC_REG_A);
		enable_irq();
	} while (regA & RTC_UIP);
}

int rtc_test_date() {
	wait_valid_rtc();
	unsigned long regB = read_reg(RTC_REG_B);

	if((regB & BIT(2)) >> 2) {	// DM = 1 - registers in binary
		printf("Second: %u\n", read_reg(SECOND));
		printf("Minute: %u\n", read_reg(MINUTE));
		printf("Hour: %u\n", read_reg(HOUR));
		printf("Week day: %u\n", read_reg(WEEK_DAY));
		printf("Month day: %u\n", read_reg(MONTH_DAY));
		printf("Month: %u\n", read_reg(MONTH));
		printf("Year: %u\n", read_reg(YEAR));
	}
	else {
		printf("Second: %x\n", read_reg(SECOND));
		printf("Minute: %x\n", read_reg(MINUTE));
		printf("Hour: %x\n", read_reg(HOUR));
		printf("Week day: %x\n", read_reg(WEEK_DAY));
		printf("Month day: %x\n", read_reg(MONTH_DAY));
		printf("Month: %x\n", read_reg(MONTH));
		printf("Year: %x\n", read_reg(YEAR));
	}

	return 0;
}

void rtc_ih(void) {
	int cause;
	unsigned long regA;
	sys_outb(RTC_ADDR_REG, RTC_REG_C);
	cause = sys_inb(RTC_DATA_REG, &regA);
	if( cause & RTC_UF );
	//handle_update_int();
	if( cause & RTC_AF );
	//handle_alarm_int();
	if( cause & RTC_PF );
	//handle_periodic_int();
}

int rtc_test_int() {

	return 0;
}

char* rtc_date()
{
	int num_date_elems = 6;
	// binary-BCD indication, year, month, day, hour, minute, second
	char *d = (char*)malloc((num_date_elems + 1)*sizeof(char));

	wait_valid_rtc();
	d[0] = (read_reg(RTC_REG_B) & BIT(2)) >> 2;	// 1 if binary mode or 0 if BCD mode
	d[1] = read_reg(YEAR);
	d[2] = read_reg(MONTH);
	d[3] = read_reg(MONTH_DAY);
	d[4] = read_reg(HOUR);
	d[5] = read_reg(MINUTE);
	d[6] = read_reg(SECOND);

	return d;
}
