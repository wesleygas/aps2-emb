#include <asf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf_board.h"

#include "conf_example.h"

#include "conf_uart_serial.h"

#include "tfont.h"

#include "sans36.h"

#include "realtimeHelpers.h"

#include "func.h"

#include "ciclo.h"

// #define pino_pio PIOA

#define MAX_ENTRIES 3
#define STRING_LENGTH 70
#define STRING_MENU_LENGTH 35

#define USART_TX_MAX_LENGTH 0xff

volatile unsigned char state = CHOOSE_STATE;
//inicia o estado previo com um inexistente
volatile unsigned char prev_state = -1;
volatile int paused = 0;
volatile int p_paused = 0;
volatile int lock_counter = -1;
volatile int locked = 0;
volatile int p_locked = 1;
volatile int update = 1;

#define TEST

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);
static void rtt_reconfigure(void);

void temperature_plus_callback(void) {
  char temperatura = actual_cycle->temp;
  if (temperatura < 100) {
    temperatura++;
  } else {
    temperatura = 0;
  }
  actual_cycle->temp = temperatura;
  draw_now = true;
}
void temperature_minus_callback(void) {
  char temperatura = actual_cycle->temp;

  if (temperatura >= 0) {
    temperatura--;
  } else {
    temperatura = 100;
  }

  actual_cycle->temp = temperatura;
  draw_now = true;
}

void bubbles_plus_callback(void) {
  actual_cycle->bubblesOn = 1;
  draw_now = true;
}
void bubbles_minus_callback(void) {
  actual_cycle->bubblesOn = 0;
  draw_now = true;
}

void centr_plus_callback(void) {
  int centrifuga = actual_cycle->centrifugacaoTempo;
  if (centrifuga >= 30) {
    centrifuga = 0;
  } else {
    centrifuga++;
  }
  actual_cycle->centrifugacaoTempo = centrifuga;
  draw_now = true;
}
void centr_minus_callback(void) {
  int centrifuga = actual_cycle->centrifugacaoTempo;
  if (centrifuga <= 0) {
    centrifuga = 30;
  } else {
    centrifuga--;
  }
  actual_cycle->centrifugacaoTempo = centrifuga;
  draw_now = true;
}

void enx_plus_callback(void) {
  int quantEnx = actual_cycle->enxagueQnt;
  if (quantEnx >= 10) {
    quantEnx = 0;
  } else {
    quantEnx++;
  }
  actual_cycle->enxagueQnt = quantEnx;
  draw_now = true;
}
void enx_minus_callback(void) {
  int quantEnx = actual_cycle->enxagueQnt;
  if (quantEnx <= 0) {
    quantEnx = 10;
  } else {
    quantEnx--;
  }
  actual_cycle->enxagueQnt = quantEnx;
  draw_now = true;
}

void tempo_enx_plus_callback(void) {
  int tempEnx = actual_cycle->enxagueTempo;
  if (tempEnx >= 10) {
    tempEnx = 0;
  } else {
    tempEnx++;
  }
  actual_cycle->enxagueTempo = tempEnx;
  draw_now = true;
}
void tempo_enx_minus_callback(void) {
  int tempEnx = actual_cycle->enxagueTempo;
  if (tempEnx <= 0) {
    tempEnx = 10;
  } else {
    tempEnx--;
  }
  actual_cycle->enxagueTempo = tempEnx;
  draw_now = true;
}

void rpm_plus_callback(void) {
  int rpm = actual_cycle->centrifugacaoRPM;
  if (rpm >= 3000) {
    rpm = 100;
  } else {
    rpm += 100;
  }
  actual_cycle->centrifugacaoRPM = rpm;
  draw_now = true;
}
void rpm_minus_callback(void) {
  int rpm = actual_cycle->centrifugacaoRPM;
  if (rpm <= 100) {
    rpm = 3000;
  } else {
    rpm -= 100;
  }
  actual_cycle->centrifugacaoRPM = rpm;
  draw_now = true;
}

void RTT_Handler(void) {
  uint32_t ul_status;

  /* Get RTT status */
  ul_status = rtt_get_status(RTT);

  /* IRQ due to Time has changed */
  if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
  }

  /* IRQ due to Alarm */
  if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
    update = 1;
  }
}

void initMenuOrder() {
  c_rapido.previous = &c_custom;
  c_rapido.next = &c_diario;

  c_diario.previous = &c_rapido;
  c_diario.next = &c_pesado;

  c_pesado.previous = &c_diario;
  c_pesado.next = &c_enxague;

  c_enxague.previous = &c_pesado;
  c_enxague.next = &c_centrifuga;

  c_centrifuga.previous = &c_enxague;
  c_centrifuga.next = &c_custom;

  c_custom.previous = &c_centrifuga;
  c_custom.next = &c_rapido;
}

void next_callback(void) {
  if (!locked) {
    actual_cycle = actual_cycle->next;
    draw_now = true;
    if (!strcmp(actual_cycle->nome, "Customize")) {
      state = CUSTOM_STATE;
    } else {
      state = CHOOSE_STATE;
    }
  }
}

void back_callback(void) {
  if (!locked) {
    actual_cycle = actual_cycle->previous;
    draw_now = true;
    if (!strcmp(actual_cycle->nome, "Customize")) {
      state = CUSTOM_STATE;
    } else {
      state = CHOOSE_STATE;
    }
  }
}

void play_callback(void) {
  if (!locked && pio_get(BOT2.pio, PIO_INPUT, BOT2.mask)) {
    state = RUN_STATE;
    draw_now = true;
    paused = false;
    pio_clear(LED1.pio, LED1.mask);
  }
}

void toggle_pause_callback(void) {
  if (!locked) {
    if (pio_get(BOT2.pio, PIO_INPUT, BOT2.mask)) {
      paused = !paused;
    }
    if (paused) {
      pio_set(LED1.pio, LED1.mask);
    } else {
      pio_clear(LED1.pio, LED1.mask);
    }
    draw_now = true;
  }
}

void cancel_callback(void) {
  if (!locked) {
    state = CHOOSE_STATE;
    pio_set(LED1.pio, LED1.mask);
    draw_now = true;
    if (!strcmp(actual_cycle->nome, "Customize")) {
      state = CUSTOM_STATE;
    } else {
      state = CHOOSE_STATE;
    }
  }
}

void blank_callback(void) {
}

void padlock_callback(void) {
  if (locked) {
    ili9488_set_foreground_color(COLOR_CONVERT(COLOR_RED));
    ili9488_draw_filled_rectangle(145, 45, 359, 80);
    ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
    ili9488_draw_string(150, 50, "Bloqueio Ativado!");
    delay_s(1);
    draw_now = true;
  }
}

void draw_lock(void) {
  if (locked) {
    botaoPadlock.image = &padlock;
  } else {
    botaoPadlock.image = &unlocked;
  }
  ili9488_draw_pixmap(botaoPadlock.x,
                      botaoPadlock.y,
                      botaoPadlock.image->width,
                      botaoPadlock.image->height,
                      botaoPadlock.image->data);
}

static void BOT0_callback(uint32_t id, uint32_t mask) {
  locked = true;
  lock_counter = 5;
}
static void BOT1_callback(uint32_t id, uint32_t mask) {
  if (lock_counter < 0) {
    lock_counter = 5;
  }
}

void configure_pins(int state_pin) {
  pmc_enable_periph_clk(ID_PIOA);
  pmc_enable_periph_clk(ID_PIOB);
  pmc_enable_periph_clk(ID_PIOC);
  pmc_enable_periph_clk(ID_PIOD);

  pio_set_output(LED0.pio, LED0.mask, state_pin, 0, 0);
  pio_set_output(LED1.pio, LED1.mask, state_pin, 0, 0);
  pio_set_output(LED2.pio, LED2.mask, state_pin, 0, 0);
  pio_set_output(LED3.pio, LED3.mask, state_pin, 0, 0);

  pio_set_input(BOT0.pio, BOT0.mask, PIO_PULLUP | PIO_DEBOUNCE);
  pio_set_input(BOT1.pio, BOT1.mask, PIO_PULLUP | PIO_DEBOUNCE);
  pio_set_input(BOT2.pio, BOT2.mask, PIO_PULLUP | PIO_DEBOUNCE);
  pio_set_input(BOT3.pio, BOT3.mask, PIO_PULLUP | PIO_DEBOUNCE);

  pio_enable_interrupt(BOT0.pio, BOT0.mask);
  pio_enable_interrupt(BOT1.pio, BOT1.mask);
  pio_enable_interrupt(BOT2.pio, BOT2.mask);
  pio_enable_interrupt(BOT3.pio, BOT3.mask);

  pio_handler_set(BOT0.pio, BOT0.id_pio, BOT0.mask, PIO_IT_FALL_EDGE, BOT0.p_handler); //BOT0.p_handler);
  pio_handler_set(BOT1.pio, BOT1.id_pio, BOT1.mask, PIO_IT_FALL_EDGE, BOT1.p_handler); //BOT1.p_handler);
  pio_handler_set(BOT2.pio, BOT2.id_pio, BOT2.mask, PIO_IT_FALL_EDGE, BOT2.p_handler); //BOT2.p_handler);
  pio_handler_set(BOT3.pio, BOT3.id_pio, BOT3.mask, PIO_IT_FALL_EDGE, BOT3.p_handler); //BOT3.p_handler);

  NVIC_EnableIRQ(BOT0.id_pio);
  NVIC_EnableIRQ(BOT1.id_pio);
  NVIC_EnableIRQ(BOT2.id_pio);
  NVIC_EnableIRQ(BOT3.id_pio);

  NVIC_SetPriority(BOT0.id_pio, 4);
  NVIC_SetPriority(BOT1.id_pio, 4);
  NVIC_SetPriority(BOT2.id_pio, 4);
  NVIC_SetPriority(BOT3.id_pio, 4);
}

void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
  char *p = text;
  while (*p != NULL) {
    char letter = *p;
    int letter_offset = letter - font->start_char;
    if (letter <= font->end_char) {
      tChar *current_char = font->chars + letter_offset;
      ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
      x += current_char->image->width + spacing;
    }
    p++;
  }
}

void draw_button(struct botao b[], uint N) {
  for (unsigned int i = 0; i < N; i++) {
    ili9488_draw_pixmap(b[i].x,
                        b[i].y,
                        b[i].image->width,
                        b[i].image->height,
                        b[i].image->data);
  }
}

void draw_menu(t_ciclo *c) {
  char buf[STRING_MENU_LENGTH];

  ili9488_draw_pixmap(10,
                      10,
                      (c->image)->width,
                      (c->image)->height,
                      (c->image)->data);

  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(10, 138, 138, 168);
  ili9488_draw_filled_rectangle(145, 10, 480, 220);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  ili9488_draw_string(10,
                      145,
                      c->nome);

  sprintf(buf, "Temperatura %d C", (c->temp));
  ili9488_draw_string(155,
                      10,
                      buf);
  sprintf(buf, "Smart Bubbles %s", c->bubblesOn ? "On" : "Off");
  ili9488_draw_string(155,
                      35,
                      buf);

  sprintf(buf, "Heavy mode %s", c->heavy ? "On" : "Off");
  ili9488_draw_string(155,
                      60,
                      buf);
  sprintf(buf, "Tempo centr. %d minutos", c->centrifugacaoTempo);
  ili9488_draw_string(155,
                      85,
                      buf);
  sprintf(buf, "Qnt enx. %d", c->enxagueQnt);
  ili9488_draw_string(155,
                      110,
                      buf);

  sprintf(buf, "Tempo enx. %d minutos", c->enxagueTempo);
  ili9488_draw_string(155,
                      135,
                      buf);

  sprintf(buf, "Centr RMP %d", c->centrifugacaoRPM);
  ili9488_draw_string(155,
                      160,
                      buf);
  sprintf(buf, "Tempo total %d minutos", (c->enxagueTempo * c->enxagueQnt) + (c->centrifugacaoTempo));
  ili9488_draw_string(155,
                      185,
                      buf);
}

void draw_custom_menu(t_ciclo *c) {
  char buf[STRING_MENU_LENGTH];

  ili9488_draw_pixmap(10,
                      10,
                      (c->image)->width,
                      (c->image)->height,
                      (c->image)->data);

  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(10, 138, 138, 168);
  ili9488_draw_filled_rectangle(145, 10, 480, 220);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  ili9488_draw_string(10,
                      145,
                      c->nome);

  sprintf(buf, "Temperatura %d C", (c->temp));
  ili9488_draw_string(155,
                      10,
                      buf);
  sprintf(buf, "Smart Bubbles %s", c->bubblesOn ? "On" : "Off");
  ili9488_draw_string(155,
                      40,
                      buf);

  sprintf(buf, "Tempo centr. %d min", c->centrifugacaoTempo);
  ili9488_draw_string(155,
                      70,
                      buf);
  sprintf(buf, "Qnt enx. %d", c->enxagueQnt);
  ili9488_draw_string(155,
                      100,
                      buf);

  sprintf(buf, "Tempo enx. %d min", c->enxagueTempo);
  ili9488_draw_string(155,
                      135,
                      buf);

  sprintf(buf, "Centr RMP %d", c->centrifugacaoRPM);
  ili9488_draw_string(155,
                      160,
                      buf);
  sprintf(buf, "Tempo total %d min", (c->enxagueTempo * c->enxagueQnt) + (c->centrifugacaoTempo));
  ili9488_draw_string(155,
                      185,
                      buf);
}

void draw_dashboard(int draw_from_scratch, t_ciclo *c) {
  char buf[STRING_MENU_LENGTH];
  Horario c_duration;
  Horario eta;
  time_reset(&c_duration);
  //time_reset(&eta);
  c_duration.minuto = (c->enxagueTempo * c->enxagueQnt) + (c->centrifugacaoTempo);
  //Cycle name
  if (draw_from_scratch) {
    ili9488_draw_rectangle(120, 10, 360, 100);
    ili9488_draw_string(130, 15, c->nome);
  }
  calcTimeDiff(c_time, c_duration, &eta);
  timeToString(buf, eta);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(140, 45, 358, 80);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
  font_draw_text(&sans36, buf, 160, 45, 2);
  if (eta.hora == 0 && eta.minuto == 0 && eta.segundo == 0) {
    state = FINISH_STATE;
    draw_now = true;
  }
}

void draw_finish_metrics() {
  char buf[STRING_MENU_LENGTH];
  ili9488_draw_rectangle(120, 10, 360, 100);
  ili9488_draw_string(130, 15, actual_cycle->nome);
  sprintf(buf, "00:00:00");
  font_draw_text(&sans36, buf, 160, 45, 2);
  ili9488_draw_string(150, 150, "CICLO COMPLETO!");
}

void draw(struct botao botoes[], int N) {

  if (draw_now) {

    switch (state) {

    case CHOOSE_STATE:
      if (state != prev_state) {
        draw_screen();
        draw_button(botoes, N);
        prev_state = state;
      }
      draw_menu(actual_cycle);

      draw_now = false;
      break;

    case RUN_STATE:
      if (state != prev_state) {
        draw_screen();
        time_reset(&c_time);
        draw_button(botoes, N);
        draw_dashboard((state != prev_state), actual_cycle);
        prev_state = state;
      }
      draw_now = false;
      if (!paused) {
        incTime(&c_time);
        draw_dashboard((state != prev_state), actual_cycle);
      }
      if (paused != p_paused) {
        if (paused) {
          botoes[1].image = &play;
        } else {
          botoes[1].image = &pause;
        }
        draw_button(botoes, N);
        p_paused = paused;
      }

      break;

    case FINISH_STATE:
      if (state != prev_state) {
        draw_screen();
        draw_button(botoes, N);
        if (locked)
          draw_lock();
        prev_state = state;
        draw_finish_metrics();
      }
      draw_now = false;

      break;
    case CUSTOM_STATE:
      if (state != prev_state) {
        draw_screen();
        prev_state = state;
      }
      draw_custom_menu(actual_cycle);
      draw_button(botoes, N);
      draw_now = false;
      break;
    default:
      break;
    }
  }
}

int processa_touch(struct botao b[], struct botao *rtn, uint N, uint x, uint y) {
  for (unsigned int i = 0; i < N; i++) {
    if ((x >= b[i].x) && (x <= (b[i].x + b[i].size_x)) && (y >= b[i].y) && (y <= (b[i].y + b[i].size_y))) {

      //("Encontrou botao %d \n\r", i);
      *rtn = b[i];
      return true;
    }
  }
  return false;
}

static void configure_lcd(void) {
  /* Initialize display parameter */
  g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
  g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
  g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
  g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

  /* Initialize LCD */
  ili9488_init(&g_ili9488_display_opt);
}

/**
 * \brief Set maXTouch configuration
 *
 * This function writes a set of predefined, optimal maXTouch configuration data
 * to the maXTouch Xplained Pro.
 *
 * \param device Pointer to mxt_device struct
 */
static void mxt_init(struct mxt_device *device) {
  enum status_code status;

  /* T8 configuration object data */
  uint8_t t8_object[] = {
      0x0d, 0x00, 0x05, 0x0a, 0x4b, 0x00, 0x00,
      0x00, 0x32, 0x19};

  /* T9 configuration object data */
  uint8_t t9_object[] = {
      0x8B, 0x00, 0x00, 0x0E, 0x08, 0x00, 0x80,
      0x32, 0x05, 0x02, 0x0A, 0x03, 0x03, 0x20,
      0x02, 0x0F, 0x0F, 0x0A, 0x00, 0x00, 0x00,
      0x00, 0x18, 0x18, 0x20, 0x20, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x02,
      0x02};

  /* T46 configuration object data */
  uint8_t t46_object[] = {
      0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x03,
      0x00, 0x00};

  /* T56 configuration object data */
  uint8_t t56_object[] = {
      0x02, 0x00, 0x01, 0x18, 0x1E, 0x1E, 0x1E,
      0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E,
      0x1E, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00};

  /* TWI configuration */
  twihs_master_options_t twi_opt = {
      .speed = MXT_TWI_SPEED,
      .chip = MAXTOUCH_TWI_ADDRESS,
  };

  status = (enum status_code)twihs_master_setup(MAXTOUCH_TWI_INTERFACE, &twi_opt);
  Assert(status == STATUS_OK);

  /* Initialize the maXTouch device */
  status = mxt_init_device(device, MAXTOUCH_TWI_INTERFACE,
                           MAXTOUCH_TWI_ADDRESS, MAXTOUCH_XPRO_CHG_PIO);
  Assert(status == STATUS_OK);

  /* Issue soft reset of maXTouch device by writing a non-zero value to
	 * the reset register */
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_COMMANDPROCESSOR_T6, 0) + MXT_GEN_COMMANDPROCESSOR_RESET, 0x01);

  /* Wait for the reset of the device to complete */
  delay_ms(MXT_RESET_TIME);

  /* Write data to configuration registers in T7 configuration object */
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 0, 0x20);
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 1, 0x10);
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 2, 0x4b);
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_POWERCONFIG_T7, 0) + 3, 0x84);

  /* Write predefined configuration data to configuration objects */
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_GEN_ACQUISITIONCONFIG_T8, 0), &t8_object);
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_TOUCH_MULTITOUCHSCREEN_T9, 0), &t9_object);
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_SPT_CTE_CONFIGURATION_T46, 0), &t46_object);
  mxt_write_config_object(device, mxt_get_object_address(device, MXT_PROCI_SHIELDLESS_T56, 0), &t56_object);

  /* Issue recalibration command to maXTouch device by writing a non-zero
	 * value to the calibrate register */
  mxt_write_config_reg(device, mxt_get_object_address(device, MXT_GEN_COMMANDPROCESSOR_T6, 0) + MXT_GEN_COMMANDPROCESSOR_CALIBRATE, 0x01);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses) {
  uint32_t ul_previous_time;

  /* Configure RTT for a 1 second tick interrupt */
  rtt_sel_source(RTT, false);
  rtt_init(RTT, pllPreScale);

  ul_previous_time = rtt_read_timer_value(RTT);
  while (ul_previous_time == rtt_read_timer_value(RTT))
    ;

  rtt_write_alarm_time(RTT, IrqNPulses + ul_previous_time);

  /* Enable RTT interrupt */
  NVIC_DisableIRQ(RTT_IRQn);
  NVIC_ClearPendingIRQ(RTT_IRQn);
  NVIC_SetPriority(RTT_IRQn, 3);
  NVIC_EnableIRQ(RTT_IRQn);
  rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}

static void rtt_reconfigure() {
  uint16_t pllPreScale = (int)(((float)32768) / 100.0);
  uint32_t irqRTTvalue = 100;

  // reinicia RTT para gerar um novo IRQ
  RTT_init(pllPreScale, irqRTTvalue);
}

void draw_screen(void) {
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
  ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH - 1, ILI9488_LCD_HEIGHT - 1);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
}

uint32_t convert_axis_system_x(uint32_t touch_x) {
  // entrada: 4096 - 0 (sistema de coordenadas atual)
  // saida: 0 - 320
  return ILI9488_LCD_WIDTH - (ILI9488_LCD_WIDTH * touch_x / 4096);
}

uint32_t convert_axis_system_y(uint32_t touch_y) {
  // entrada: 0 - 4096 (sistema de coordenadas atual)
  // saida: 0 - 320
  return ILI9488_LCD_HEIGHT - (ILI9488_LCD_HEIGHT * touch_y / 4096);
}

void mxt_handler(struct mxt_device *device, struct botao botoes[], uint Nbotoes) {
  /* USART tx buffer initialized to 0 */
  char tx_buf[STRING_LENGTH * MAX_ENTRIES] = {0};
  uint8_t i = 0; /* Iterator */

  /* Temporary touch event data struct */
  struct mxt_touch_event touch_event;

  /* Collect touch events and put the data in a string,
	 * maximum 2 events at the time */
  do {
    /* Temporary buffer for each new touch event line */
    char buf[STRING_LENGTH];

    /* Read next next touch event in the queue, discard if read fails */
    if (mxt_read_touch_event(device, &touch_event) != STATUS_OK) {
      continue;
    }

    // eixos trocados (quando na vertical LCD)
    uint32_t conv_x = convert_axis_system_x(touch_event.x);
    uint32_t conv_y = convert_axis_system_y(touch_event.y);

    /* Format a new entry in the data string that will be sent over USART */
    sprintf(buf, "X:%3d Y:%3d \n", conv_x, conv_y);

    /* -----------------------------------------------------*/
    struct botao bAtual;
    //contribuicao de status
    if (processa_touch(botoes, &bAtual, Nbotoes, conv_x, conv_y) && touch_event.status < 60) {
      bAtual.p_handler();
    }
    //update_screen(conv_x, conv_y);
    /* -----------------------------------------------------*/

    /* Add the new string to the string buffer */
    strcat(tx_buf, buf);
    i++;

    /* Check if there is still messages in the queue and
		 * if we have reached the maximum numbers of events */
  } while ((mxt_is_message_pending(device)) & (i < MAX_ENTRIES));

/* If there is any entries in the buffer, send them over USART */
#ifdef TEST

  if (i > 0) {
    usart_serial_write_packet(USART_SERIAL_EXAMPLE, (uint8_t *)tx_buf, strlen(tx_buf));
  }
#endif
}

int main(void) {
  struct mxt_device device; /* Device data container */

  /* Initialize the USART configuration struct */
  const usart_serial_options_t usart_serial_options = {
      .baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
      .charlength = USART_SERIAL_CHAR_LENGTH,
      .paritytype = USART_SERIAL_PARITY,
      .stopbits = USART_SERIAL_STOP_BIT};

  sysclk_init(); /* Initialize system clocks */
  board_init();  /* Initialize board */
  configure_lcd();
  draw_screen();
  initMenuOrder();
  configure_pins(1);

  struct botao botoes[][15] = {
      {
          botaoLeft,
          botaoPlay,
          botaoRight,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
      },
      {
          botaoCancel,
          botaoDPlayPause,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
      },
      {
          botaoWashComplete,
          botaoBackHome,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
      },
      {
          botaoLeft,
          botaoPlay,
          botaoRight,
          botaoPlusTemperature,
          botaoMinusTemperature,
          botaoPlusBubbles,
          botaoMinusBubbles,
          botaoPlusCentr,
          botaoMinusCentr,
          botaoPlusEnx,
          botaoMinusEnx,
          botaoPlusTempoEnx,
          botaoMinusTempoEnx,
          botaoPlusRPM,
          botaoMinusRPM,

      }};
  //Numero de botoes em cada estado
  int botoes_num[] = {3, 2, 2, 15};

  /* Initialize the mXT touch device */
  mxt_init(&device);

  /* Initialize stdio on USART */
  stdio_serial_init(USART_SERIAL_EXAMPLE, &usart_serial_options);

  printf("\n\rTesting uart connection\n\r");

  //pino led = {.id = 4, .id_pio = ID_PIOA, .mask = 1 << 4, .pio = PIOA};

  ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH, ILI9488_LCD_HEIGHT);
  ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));

  /* -----------------------------------------------------*/
  pio_set(LED0.pio, LED0.mask);
  pio_set(LED1.pio, LED1.mask);
  pio_set(LED0.pio, LED2.mask);
  pio_set(LED3.pio, LED3.mask);
  lock_counter = -1;
  locked = 0;
  while (true) {
    if (update) {
      rtt_reconfigure();
      update = 0;
      if (pio_get(LED0.pio, PIO_INPUT, LED0.mask)) {
        pio_clear(LED0.pio, LED0.mask);
      } else {
        pio_set(LED0.pio, LED0.mask);
      }
      if (locked) {
        pio_clear(LED3.pio, LED3.mask);
        if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_MASK)) {
          if (lock_counter > 0) {
            lock_counter--;
          } else {
            locked = false;
          }
        }
      } else {
        pio_set(LED3.pio, LED3.mask);
      }
      if (p_locked != locked) {
        draw_lock();
        p_locked = locked;
      }
      if (state == RUN_STATE) {
        draw_now = true;
      }
    }
    draw(botoes[state], botoes_num[state]);
    /* Check for any pending messages and run message handler if any
		 * message is found in the queue */

    if (mxt_is_message_pending(&device)) {
      mxt_handler(&device, botoes[state], botoes_num[state]);
    }
  }

  return 0;
}
