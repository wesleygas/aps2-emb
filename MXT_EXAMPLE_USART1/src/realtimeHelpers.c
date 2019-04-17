#include <string.h>


/**
* \brief Interrupt handler for the RTC. Refresh the display.
*/
void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
		
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR); //Avisa q foi handled
			
	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}

/*
*    Fills a string with time data in the form hh:mm:ss
*/
void timeToString(char *str, Horario tempo){
	if(tempo.hora < 10){
		str[0] = '0';
		str[1] = tempo.hora + 48;
		}else{
		str[0] = tempo.hora/10 + 48;
		str[1] = tempo.hora%10 + 48;
	}
	str[2] = ':';
	if(tempo.minuto < 10){
		str[3] = '0';
		str[4] = tempo.minuto + 48;
		}else{
		str[3] = tempo.minuto/10 + 48;
		str[4] = tempo.minuto%10 + 48;
	}
	str[5] = ':';
	if(tempo.segundo < 10){
		str[6] = '0';
		str[7] = tempo.segundo + 48;
		}else{
		str[6] = tempo.segundo/10 + 48;
		str[7] = tempo.segundo%10 + 48;
	}
	str[8] = 0;
	
}

/*
* Calculate time difference: est_finish - curr_time = eta
*/
void calcTimeDiff(Horario curr_time, Horario est_finish, Horario *eta){
	if(curr_time.segundo > est_finish.segundo){
		est_finish.minuto --;
		est_finish.segundo += 60;
	}
	eta->segundo = est_finish.segundo - curr_time.segundo;
	
	if(curr_time.minuto > est_finish.minuto){
		est_finish.hora --;
		est_finish.minuto += 60;
	}
	eta->minuto = est_finish.minuto - curr_time.minuto;
	eta->hora = est_finish.hora - curr_time.hora;
}


/**
* Configura o RTC para funcionar com interrupcao de alarme
*/
void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MONTH, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 3);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_SECEN);

}