// GENERAL DEFINES
#define BIT(n) (0x01<<(n))

// RTC IRQ LINE
#define RTC_IRQ 8

// RTC BUFFERS AND REGISTERS
#define RTC_ADDR_REG 0x70
#define RTC_DATA_REG 0x71

#define SECOND 0
#define SECONDS_ALARM 1
#define MINUTE 2
#define MINUTES_ALARM 3
#define HOUR 4
#define HOURS_ALARM 5
#define WEEK_DAY 6
#define MONTH_DAY 7
#define MONTH 8
#define YEAR 9
#define RTC_REG_A 10
#define RTC_REG_B 11
#define RTC_REG_C 12
#define RTC_REG_D 13

// RTC FLAGS
#define RTC_UIP BIT(7)	// of REGISTER A
#define RTC_PF BIT(6)	// of REGISTER C
#define RTC_AF BIT(5)	// of REGISTER C
#define RTC_UF BIT(4)	// of REGISTER C
