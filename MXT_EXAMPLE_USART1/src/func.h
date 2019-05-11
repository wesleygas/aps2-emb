void maquina_callback(void);
void next_callback(void);
void back_callback(void);
void play_callback(void);
void cancel_callback(void);
void blank_callback(void);

void temperature_plus_callback(void);
void temperature_minus_callback(void);
void bubbles_plus_callback(void);
void bubbles_minus_callback(void);
void centr_plus_callback(void);
void centr_minus_callback(void);
void enx_plus_callback(void);
void enx_minus_callback(void);
void tempo_enx_plus_callback(void);
void tempo_enx_minus_callback(void);
void rpm_plus_callback(void);
void rpm_minus_callback(void);

void toggle_pause_callback(void);
void configure_pins(int state);
void draw_screen(void);
void padlock_callback(void);

enum State {
	CHOOSE_STATE = 0,
	RUN_STATE,
	FINISH_STATE,
	CUSTOM_STATE,
	};

static void configure_lcd(void);
struct ili9488_opt_t g_ili9488_display_opt;


typedef struct {
  uint32_t id;
  uint32_t mask;
  uint32_t id_pio;
  Pio *pio;
  void (*p_handler)(uint32_t id, uint32_t mask);
} pino;

struct botao {
  uint16_t x;
  uint16_t y;
  uint16_t size_x;
  uint16_t size_y;
  tImage *image;
  void (*p_handler)(void);
};

#include "icones/arrows.h"
#include "icones/pause.h"
#include "icones/stop.h"
#include "icones/padlock.h"
#include "icones/unlocked.h"
#include "icones/washComplete.h"
#include "icones/backHome.h"
#include "icones/plus.h"
#include "icones/minus.h"

static struct botao botaoRight = {
    .x = 415,
    .y = 240,
    .size_x = 64,
    .size_y = 64,
    .image = &right,
    .p_handler = next_callback};

static struct botao botaoLeft = {
    .x = 0,
    .y = 240,
    .size_x = 64,
    .size_y = 64,
    .image = &left,
    .p_handler = back_callback};

static struct botao botaoPlay = {
    .x = 210,
    .y = 230,
    .size_x = 64,
    .size_y = 64,
    .image = &play,
    .p_handler = play_callback};
	
static struct botao botaoCancel = {
	.x = 176,
	.y = 191,
	.size_x = 128,
	.size_y = 128,
	.image = &stop,
.p_handler = cancel_callback};

struct botao botaoDPlayPause = {
	.x = 405,
	.y = 10,
	.size_x = 64,
	.size_y = 64,
	.image = &pause,
.p_handler = toggle_pause_callback};

struct botao botaoPadlock = {
	.x = 85,
	.y = 240,
	.size_x = 64,
	.size_y = 64,
	.image = &padlock,
.p_handler = padlock_callback};

static struct botao botaoWashComplete = {
	.x = 180,
	.y = 181,
	.size_x = 118,
	.size_y = 134,
	.image = &washComplete,
.p_handler = cancel_callback};

static struct botao botaoBackHome = {
	.x = 10,
	.y = 10,
	.size_x = 64,
	.size_y = 64,
	.image = &backHome,
.p_handler = cancel_callback};



static struct botao botaoPlusTemperature = {
	.x = 446,
	.y = 10,
	.size_x = 24,
	.size_y = 24,
	.image = &plus,
.p_handler = temperature_plus_callback};

static struct botao botaoMinusTemperature = {
	.x = 417,
	.y = 10,
	.size_x = 24,
	.size_y = 24,
	.image = &minus,
.p_handler = temperature_minus_callback};

static struct botao botaoPlusBubbles = {
	.x = 446,
	.y = 40,
	.size_x = 24,
	.size_y = 24,
	.image = &plus,
.p_handler = bubbles_plus_callback};

static struct botao botaoMinusBubbles = {
	.x = 417,
	.y = 40,
	.size_x = 24,
	.size_y = 24,
	.image = &minus,
.p_handler = bubbles_minus_callback};

static struct botao botaoPlusCentr = {
	.x = 446,
	.y = 70,
	.size_x = 24,
	.size_y = 24,
	.image = &plus,
.p_handler = centr_plus_callback};

static struct botao botaoMinusCentr = {
	.x = 417,
	.y = 70,
	.size_x = 24,
	.size_y = 24,
	.image = &minus,
.p_handler = centr_minus_callback};

static struct botao botaoPlusEnx = {
	.x = 446,
	.y = 100,
	.size_x = 24,
	.size_y = 24,
	.image = &plus,
.p_handler = enx_plus_callback};

static struct botao botaoMinusEnx = {
	.x = 417,
	.y = 100,
	.size_x = 24,
	.size_y = 24,
	.image = &minus,
.p_handler = enx_minus_callback};

static struct botao botaoPlusTempoEnx = {
	.x = 446,
	.y = 130,
	.size_x = 24,
	.size_y = 24,
	.image = &plus,
.p_handler = tempo_enx_plus_callback};

static struct botao botaoMinusTempoEnx = {
	.x = 417,
	.y = 130,
	.size_x = 24,
	.size_y = 24,
	.image = &minus,
.p_handler = tempo_enx_minus_callback};

static struct botao botaoPlusRPM = {
	.x = 446,
	.y = 160,
	.size_x = 24,
	.size_y = 24,
	.image = &plus,
.p_handler = rpm_plus_callback};

static struct botao botaoMinusRPM = {
	.x = 417,
	.y = 160,
	.size_x = 24,
	.size_y = 24,
	.image = &minus,
.p_handler = rpm_minus_callback};



void draw_button(struct botao b[], uint N);
void draw(struct botao botoes[], int N);

volatile int draw_now = true;

//definindo leds
const pino LED0 = {
    .id = 8,
    .mask = 1u << 8,
    .id_pio = ID_PIOC,
    .pio = PIOC,
};
const pino LED1 = {
    .id = 0,
    .mask = 1u << 0,
    .id_pio = ID_PIOA,
    .pio = PIOA,
};
const pino LED2 = {
    .id = 30,
    .mask = 1u << 30,
    .id_pio = ID_PIOC,
    .pio = PIOC,
};
const pino LED3 = {
    .id = 2,
    .mask = 1u << 2,
    .id_pio = ID_PIOB,
    .pio = PIOB,
};

// definindo botões
static void BOT0_callback(uint32_t id, uint32_t mask);
static void BOT1_callback(uint32_t id, uint32_t mask);
static void BOT2_callback(uint32_t id, uint32_t mask);
static void BOT3_callback(uint32_t id, uint32_t mask);

#define BUT1_PIO		  PIOD
#define BUT1_MASK		  (1u << 28u)

#define BUT2_PIO		  PIOC
#define BUT2_MASK		  (1u << 31)

#define BUT3_PIO		  PIOA
#define BUT3_MASK		  (1u << 19)

const pino BOT0 = {
    .id = 11,
    .mask = 1u << 11,
    .id_pio = ID_PIOA,
    .pio = PIOA,
    .p_handler = BOT0_callback};

const pino BOT1 = {
    .id = 28,
    .mask = 1u << 28,
    .id_pio = ID_PIOD,
    .pio = PIOD,
    .p_handler = BOT1_callback};

const pino BOT2 = {
    .id = 31,
    .mask = 1u << 31,
    .id_pio = ID_PIOC,
    .pio = PIOC,
    .p_handler = BOT2_callback};

const pino BOT3 = {
    .id = 19,
    .mask = 1u << 19,
    .id_pio = ID_PIOA,
    .pio = PIOA,
    .p_handler = BOT3_callback};

#define BUT_DEBOUNCING_VALUE 79