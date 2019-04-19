#include <stdint.h>
#include <string.h>
#include "asf.h"

//RTC constants
//Date
#define YEAR	2019
#define MONTH	03
#define DAY		30
#define WEEK	12
//Time
#define HOUR	11
#define MINUTE	59
#define SECOND	50


typedef struct Horario{
	int hora;
	int minuto;
	int segundo;
} Horario;

void RTC_Handler(void);
void timeToString(char *str, Horario tempo);
void calcTimeDiff(Horario curr_time, Horario est_finish, Horario *eta);
void incTime(Horario *c_time);
void time_reset(Horario *c_time);
void RTC_init(void);

Horario c_time;
