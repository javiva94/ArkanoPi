#ifndef _ARKANOPI_H_
#define _ARKANOPI_H_

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h> // A descomentar en posteriores sesiones

#define GPIO_ROW_0 0
#define GPIO_ROW_1 1
#define GPIO_ROW_2 2
#define GPIO_ROW_3 3
#define GPIO_ROW_4 4
#define GPIO_ROW_5 7
#define GPIO_ROW_6 23

#define GPIO_COL_0 14
#define GPIO_COL_1 17
#define GPIO_COL_2 18
#define GPIO_COL_3 22

#define GPIO_BUTTON_DER 16
#define GPIO_BUTTON_IZQ 19



#include "kbhit.h" // para poder detectar teclas pulsadas sin bloqueo y leer las teclas pulsadas
#include "arkanoPiLib.h"
#include "fsm.h"
#include "tmr.h"

//CONSTANTES
#define CLK_MS 10
#define DEBOUNCE_TIME 500
#define TIEMPO_PELOTA 1000

// FLAGS DEL SISTEMA
#define FLAG_TECLA 					0x01
#define FLAG_PELOTA 				0x02
#define FLAG_RAQUETA_DERECHA       	0x04
#define FLAG_RAQUETA_IZQUIERDA     	0x08
#define FLAG_FINAL_JUEGO			0x10

#define	FLAGS_KEY	1
#define	STD_IO_BUFFER_KEY	2
#define STD_PIN_KEY	 0

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
// Prototipos de funciones de entrada
int compruebaTeclaPulsada (fsm_t* this);
int compruebaTeclaPelota (fsm_t* this);
int compruebaTeclaRaquetaDerecha (fsm_t* this);
int compruebaTeclaRaquetaIzquierda (fsm_t* this);
int compruebaFinalJuego (fsm_t* this);

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
void InicializaJuego(fsm_t*);
void MovimientoPelota(fsm_t*);
void MovimientoRaquetaDerecha(fsm_t*);
void MovimientoRaquetaIzquierda(fsm_t*);
void ReseteaJuego(fsm_t*);
void FinalJuego(fsm_t*);

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------
PI_THREAD (thread_explora_teclado);

typedef enum {
	WAIT_START,
	WAIT_PUSH,
	WAIT_END
} tipo_estados_juego;

typedef struct {
	tipo_arkanoPi arkanoPi;
	tipo_estados_juego estado;
	char teclaPulsada;
} tipo_juego;

void timer_isr(union sigval value);

//------------------------------------------------------
// FUNCIONES DE ACCION
//------------------------------------------------------
/*
void InicializaJuego (void);
void MueveRaquetaIzquierda (void);
void MueveRaquetaDerecha (void);
void MovimientoPelota (void);
void FinalJuego (void);
void ReseteaJuego (void);
*/
//------------------------------------------------------
// FUNCIONES DE INICIALIZACION
//------------------------------------------------------
int systemSetup (void);

#endif /* ARKANOPI_H_ */
