
typedef struct ciclo t_ciclo;
typedef struct ciclo {
  int enxagueTempo;       // tempo que fica em cada enxague
  int enxagueQnt;         // quantidade de enxagues
  int centrifugacaoRPM;   // velocidade da centrifugacao
  int centrifugacaoTempo; // tempo que centrifuga
  char nome[32];          // nome do ciclo, para ser exibido
  char heavy;             // modo pesado de lavagem
  char bubblesOn;         // smart bubbles on (???),
  char temp;
  tImage *image;
  t_ciclo *previous;
  t_ciclo *next;
};

#include "icones/centrifuge.h"
#include "icones/daily.h"
#include "icones/pesado.h"
#include "icones/rapido.h"
#include "icones/water.h"
#include "icones/custom.h"

t_ciclo c_rapido = {
	.nome = "Rapido",
    .enxagueTempo = 1,
    .enxagueQnt = 1,
    .centrifugacaoRPM = 900,
    .centrifugacaoTempo = 0,
    .heavy = 0,
    .bubblesOn = 1,
    .temp = 20,
    .image = &rapido_icon};

t_ciclo c_diario = {
    .nome = "Diario",
    .enxagueTempo = 15,
    .enxagueQnt = 2,
    .centrifugacaoRPM = 1200,
    .centrifugacaoTempo = 8,
    .heavy = 0,
    .bubblesOn = 1,
    .temp = 20,
    .image = &daily_icon};

t_ciclo c_pesado = {
    .nome = "Pesado",
    .enxagueTempo = 10,
    .enxagueQnt = 3,
    .centrifugacaoRPM = 1200,
    .centrifugacaoTempo = 10,
    .heavy = 1,
    .bubblesOn = 1,
    .temp = 20,
    .image = &pesado_icon};

t_ciclo c_enxague = {
    .nome = "Enxague",
    .enxagueTempo = 10,
    .enxagueQnt = 1,
    .centrifugacaoRPM = 0,
    .centrifugacaoTempo = 0,
    .heavy = 0,
    .bubblesOn = 0,
    .temp = 20,
    .image = &water_icon};

t_ciclo c_centrifuga = {
    .nome = "Centrifuga",
    .enxagueTempo = 0,
    .enxagueQnt = 0,
    .centrifugacaoRPM = 1200,
    .centrifugacaoTempo = 10,
    .heavy = 0,
    .bubblesOn = 0,
    .temp = 20,
    .image = &centrifuge_icon};

t_ciclo c_custom = {
	.nome = "Customize",
	.enxagueTempo = 5,
	.enxagueQnt = 2,
	.centrifugacaoRPM = 1200,
	.centrifugacaoTempo = 10,
	.heavy = 0,
	.bubblesOn = 0,
	.temp = 27,
.image = &custom};


t_ciclo *actual_cycle = &c_rapido;