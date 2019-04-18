void maquina_callback(void);
void next_callback(void);
void back_callback(void);
void play_callback(void);
void pause_callback(void);
void configure_pins(int state);
void draw_screen(void);

#define CHOOSE_STATE 0
#define RUN_STATE 1

static void configure_lcd(void);
struct ili9488_opt_t g_ili9488_display_opt;

typedef struct {
  const uint8_t *data;
  uint16_t width;
  uint16_t height;
  uint8_t dataSize;
} tImage;

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

struct botao botaoRight = {
    .x = 415,
    .y = 240,
    .size_x = 64,
    .size_y = 64,
    .image = &right,
    .p_handler = next_callback};

struct botao botaoLeft = {
    .x = 0,
    .y = 240,
    .size_x = 64,
    .size_y = 64,
    .image = &left,
    .p_handler = back_callback};

struct botao botaoPlay = {
    .x = 210,
    .y = 230,
    .size_x = 64,
    .size_y = 64,
    .image = &play,
    .p_handler = play_callback};

struct botao botaoPause = {
    .x = 210,
    .y = 230,
    .size_x = 64,
    .size_y = 64,
    .image = &pause,
    .p_handler = pause_callback};

void draw_button(struct botao b[], uint N);
void draw(struct botao botoes[], int N);

volatile int draw_now = true;

//definindo leds
pino LED0 = {
    .id = 8,
    .mask = 1u << 8,
    .id_pio = ID_PIOC,
    .pio = PIOC,
};
pino LED1 = {
    .id = 0,
    .mask = 1u << 0,
    .id_pio = ID_PIOA,
    .pio = PIOA,
};
pino LED2 = {
    .id = 30,
    .mask = 1u << 30,
    .id_pio = ID_PIOC,
    .pio = PIOC,
};
pino LED3 = {
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

pino BOT0 = {
    .id = 11,
    .mask = 1u << 11,
    .id_pio = ID_PIOA,
    .pio = PIOA,
    .p_handler = BOT0_callback};

pino BOT1 = {
    .id = 28,
    .mask = 1u << 28,
    .id_pio = ID_PIOD,
    .pio = PIOD,
    .p_handler = BOT1_callback};

pino BOT2 = {
    .id = 31,
    .mask = 1u << 31,
    .id_pio = ID_PIOC,
    .pio = PIOC,
    .p_handler = BOT2_callback};

pino BOT3 = {
    .id = 19,
    .mask = 1u << 19,
    .id_pio = ID_PIOA,
    .pio = PIOA,
    .p_handler = BOT3_callback};

#define BUT_DEBOUNCING_VALUE 79