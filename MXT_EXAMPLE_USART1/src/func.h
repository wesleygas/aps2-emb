void maquina_callback(void);
void next_callback(void);
void back_callback(void);
void lavagem_callback(void);
void secagem_callback(void);

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
#include "icones/icone1.h"
#include "icones/maquina.h"

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
    .p_handler = next_callback};

struct botao botaoPlay = {
    .x = 200,
    .y = 230,
    .size_x = 64,
    .size_y = 64,
    .image = &play,
    .p_handler = next_callback};

struct botao botaoSecagem = {
    .x = 0,
    .y = 0,
    .size_x = 100,
    .size_y = 100,
    .image = &icone1,
    .p_handler = secagem_callback};

void draw_button(struct botao b[], uint N);
volatile int main_menu = true;