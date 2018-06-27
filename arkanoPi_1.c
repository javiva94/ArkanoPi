
#include "arkanoPi_1.h"
enum fsm_state {
  START=0,
  PUSH=1,
  END=2,
  };
static volatile tipo_juego juego;
int columna = 0;
static tmr_t* timer;
static tmr_t* timer_ball;
volatile int flags = 0;
int array_f[]= {GPIO_ROW_0,GPIO_ROW_1,GPIO_ROW_2,GPIO_ROW_3,GPIO_ROW_4,GPIO_ROW_5,GPIO_ROW_6};
int array_c[]= {GPIO_COL_0,GPIO_COL_1,GPIO_COL_2,GPIO_COL_3};
int debounceTime = 0;
// espera hasta la próxima activación del reloj
void delay_until (unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay (next - now);
    }
}

//------------------------------------------------------
// FUNCIONES DE ACCION
//------------------------------------------------------

// FUNCIONES DE ENTRADA O COMPROBACIÓN DE LA MAQUINA DE ESTADOS
int compruebaTeclaPulsada (fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = ((flags & FLAG_TECLA)||(flags & FLAG_RAQUETA_IZQUIERDA)||(flags & FLAG_RAQUETA_DERECHA));
	piUnlock (FLAGS_KEY);

	return result;
}

int compruebaTeclaPelota (fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_PELOTA);
	piUnlock (FLAGS_KEY);

	return result;
}
void ActivaRaquetaDer(){

	if(millis()<debounceTime){
		debounceTime = millis() + DEBOUNCE_TIME;
		return;
	}

	piLock(FLAGS_KEY);
	flags |= FLAG_RAQUETA_DERECHA;
	piUnlock(FLAGS_KEY);

	while(digitalRead (GPIO_BUTTON_DER)== HIGH){
		delay(1);
	}
	debounceTime = millis () + DEBOUNCE_TIME ;

}
int compruebaTeclaRaquetaDerecha (fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_RAQUETA_DERECHA);
	piUnlock (FLAGS_KEY);

	return result;
}
void ActivaRaquetaIzq(){

	if (millis()< debounceTime){
		debounceTime= millis()+ DEBOUNCE_TIME;
		return;
	}

	piLock(FLAGS_KEY);
	flags |= FLAG_RAQUETA_IZQUIERDA;
	piUnlock(FLAGS_KEY);

	while (digitalRead(GPIO_BUTTON_IZQ) == HIGH){
		delay(1);
	}
	debounceTime = millis () + DEBOUNCE_TIME ;

}
int compruebaTeclaRaquetaIzquierda (fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_RAQUETA_IZQUIERDA);
	piUnlock (FLAGS_KEY);

	return result;
}

int compruebaFinalJuego (fsm_t* this) {
	int result;

	piLock (FLAGS_KEY);
	result = (flags & FLAG_FINAL_JUEGO);
	piUnlock (FLAGS_KEY);

	return result;
}

// FUNCIONES DE SALIDA O ACCION DE LA MAQUINA DE ESTADOS

void InicializaJuego (fsm_t* fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_TECLA;
	flags &= ~FLAG_PELOTA;
	flags &= ~FLAG_RAQUETA_IZQUIERDA;
	flags &= ~FLAG_RAQUETA_DERECHA;
	flags &= ~FLAG_FINAL_JUEGO;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	InicializaArkanoPi ((tipo_arkanoPi*)(&(juego.arkanoPi)));

	piUnlock (STD_IO_BUFFER_KEY);

}
void MovimientoPelota (fsm_t* fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_PELOTA;
	piUnlock (FLAGS_KEY);
	piLock (STD_IO_BUFFER_KEY);
	if(juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv >= MATRIZ_ALTO){
			FinalJuego((fsm_t*) (&(fsm)));
		}
		//Choca con Paredes
		if(juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv > MATRIZ_ANCHO-1){
			juego.arkanoPi.pelota.xv=-1;
		}
		else if(juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv < 0){
			juego.arkanoPi.pelota.xv =1;
		}
		if(juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.xv <= 0){
			juego.arkanoPi.pelota.yv=1;
		}

		//Choca ladrillos
		if(juego.arkanoPi.ladrillos.matriz[juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv][juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv]==1){
			juego.arkanoPi.ladrillos.matriz[juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv][juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv]=0;
			juego.arkanoPi.pelota.yv = -juego.arkanoPi.pelota.yv;

			if((CalculaLadrillosRestantes((tipo_pantalla*)(&(juego.arkanoPi.ladrillos))))== 0){
				FinalJuego((fsm_t*) (&(fsm)));
			}
		}
		//Choca Raqueta
		//IZQ
	     if(((juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv) == MATRIZ_ALTO-1) && ((juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv) == juego.arkanoPi.raqueta.x)){
	    	juego.arkanoPi.pelota.xv=-1;
	    	juego.arkanoPi.pelota.yv=-1;
	    }
	     //CENTRO
	     else if(((juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv) == MATRIZ_ALTO-1) && ((juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv) == juego.arkanoPi.raqueta.x+1)){
	     	juego.arkanoPi.pelota.xv=0;
	     	juego.arkanoPi.pelota.yv=-1;
	     }
	     //DER
	     else if(((juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv) == MATRIZ_ALTO-1) && ((juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv) == juego.arkanoPi.raqueta.x+2)){
	     	juego.arkanoPi.pelota.xv=1;
	     	juego.arkanoPi.pelota.yv=-1;
	     }
	     juego.arkanoPi.pelota.x = juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv;
	     juego.arkanoPi.pelota.y = juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv;
	     ActualizaPantalla ((tipo_arkanoPi*)(&(juego.arkanoPi)));
	piUnlock (STD_IO_BUFFER_KEY);

}

void MueveRaquetaDerecha (fsm_t* fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_DERECHA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	if (juego.arkanoPi.raqueta.x < 9){
		juego.arkanoPi.raqueta.x++;
	}
	digitalWrite(GPIO_BUTTON_DER, 1);
	ActualizaPantalla ((tipo_arkanoPi*)(&(juego.arkanoPi)));
	piUnlock (STD_IO_BUFFER_KEY);

}

void FinalJuego (fsm_t* fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_FINAL_JUEGO;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	ReseteaMatriz((tipo_pantalla*)(&(juego.arkanoPi.pantalla)));
	printf("\n");
	printf("FIN DEL JUEGO");
	printf("\n");
	ActualizaPantalla ((tipo_arkanoPi*)(&(juego.arkanoPi)));
	piUnlock (STD_IO_BUFFER_KEY);

}

void MueveRaquetaIzquierda (fsm_t* fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_RAQUETA_IZQUIERDA;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	if(juego.arkanoPi.raqueta.x > -2){
		juego.arkanoPi.raqueta.x--;
		}
	digitalWrite(GPIO_BUTTON_IZQ, 1);
	ActualizaPantalla ((tipo_arkanoPi*)(&(juego.arkanoPi)));
	piUnlock (STD_IO_BUFFER_KEY);

}

void ReseteaJuego (fsm_t* fsm) {
	piLock (FLAGS_KEY);
	flags &= ~FLAG_TECLA;
	flags &= ~FLAG_PELOTA;
	flags &= ~FLAG_RAQUETA_IZQUIERDA;
	flags &= ~FLAG_RAQUETA_DERECHA;
	flags &= ~FLAG_FINAL_JUEGO;
	piUnlock (FLAGS_KEY);

	piLock (STD_IO_BUFFER_KEY);
	InicializaArkanoPi ((tipo_arkanoPi*)(&(juego.arkanoPi)));
	piUnlock (STD_IO_BUFFER_KEY);

}

/*


// void InicializaJuego (void): funcion encargada de llevar a cabo
// la oportuna inicialización de toda variable o estructura de datos
// que resulte necesaria para el desarrollo del juego.
void InicializaJuego (void) {
	// A completar por el alumno...

	InicializaArkanoPi ((tipo_arkanoPi*)(&(juego.arkanoPi)));
}

// void MueveRaquetaIzquierda (void): funcion encargada de ejecutar
// el movimiento hacia la izquierda contemplado para la raqueta.
// Debe garantizar la viabilidad del mismo mediante la comprobación
// de que la nueva posición correspondiente a la raqueta no suponga
// que ésta rebase o exceda los límites definidos para el área de juego
// (i.e. al menos uno de los leds que componen la raqueta debe permanecer
// visible durante todo el transcurso de la partida).
void MueveRaquetaIzquierda (void) {
	// A completar por el alumno...

	int *x = juego.arkanoPi.raqueta.x;
	if (x==0){

	}
	else {
		*x=*x-1;
	}

	if(juego.arkanoPi.raqueta.x > -2){
		juego.arkanoPi.raqueta.x--;
	}
	//ActualizaPantalla ((tipo_arkanoPi*)(&(juego.arkanoPi)));

}

// void MueveRaquetaDerecha (void): función similar a la anterior
// encargada del movimiento hacia la derecha.
void MueveRaquetaDerecha (void) {
	// A completar por el alumno...
	int* x = juego.arkanoPi.raqueta.x;
		if (*x == MATRIZ_ANCHO){

		}
		else {
			*x=*x+1;
		}

		ActualizaPantalla ((tipo_arkanoPi*)(&(juego.arkanoPi)));

	if (juego.arkanoPi.raqueta.x < 9){
		juego.arkanoPi.raqueta.x++;
	}
}

// void MovimientoPelota (void): función encargada de actualizar la
// posición de la pelota conforme a la trayectoria definida para ésta.
// Para ello deberá identificar los posibles rebotes de la pelota para,
// en ese caso, modificar su correspondiente trayectoria (los rebotes
// detectados contra alguno de los ladrillos implicarán adicionalmente
// la eliminación del ladrillo). Del mismo modo, deberá también
// identificar las situaciones en las que se dé por finalizada la partida:
// bien porque el jugador no consiga devolver la pelota, y por tanto ésta
// rebase el límite inferior del área de juego, bien porque se agoten
// los ladrillos visibles en el área de juego.
void MovimientoPelota (void) {
	// A completar por el alumno...
	//Pierdes
	if(juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv >= MATRIZ_ALTO){
		FinalJuego();
	}
	//Choca con Paredes
	if(juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv > MATRIZ_ANCHO-1){
		juego.arkanoPi.pelota.xv=-1;
	}
	else if(juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv < 0){
		juego.arkanoPi.pelota.xv =1;
	}
	if(juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.xv < 0){
		juego.arkanoPi.pelota.yv=1;
	}
	//Choca ladrillos
	if(juego.arkanoPi.ladrillos.matriz[juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv][juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv]==1){
		juego.arkanoPi.ladrillos.matriz[juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv][juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv]=0;
		juego.arkanoPi.pelota.yv = -juego.arkanoPi.pelota.yv;

		if((CalculaLadrillosRestantes((tipo_pantalla*)(&(juego.arkanoPi.ladrillos))))== 0){
			FinalJuego();
		}
	}
	//Choca Raqueta
	//IZQ
     if(((juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv) == MATRIZ_ALTO-1) && ((juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv) == juego.arkanoPi.raqueta.x)){
    	juego.arkanoPi.pelota.xv=-1;
    	juego.arkanoPi.pelota.yv=-1;
    }
     //CENTRO
     else if(((juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv) == MATRIZ_ALTO-1) && ((juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv) == juego.arkanoPi.raqueta.x+1)){
     	juego.arkanoPi.pelota.xv=0;
     	juego.arkanoPi.pelota.yv=-1;
     }
     //DER
     else if(((juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv) == MATRIZ_ALTO-1) && ((juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv) == juego.arkanoPi.raqueta.x+2)){
     	juego.arkanoPi.pelota.xv=1;
     	juego.arkanoPi.pelota.yv=-1;
     }
     juego.arkanoPi.pelota.x = juego.arkanoPi.pelota.x + juego.arkanoPi.pelota.xv;
     juego.arkanoPi.pelota.y = juego.arkanoPi.pelota.y + juego.arkanoPi.pelota.yv;

}
// void FinalJuego (void): función encargada de mostrar en la ventana de
// terminal los mensajes necesarios para informar acerca del resultado del juego.
void FinalJuego (void) {
	// A completar por el alumno...
	ReseteaMatriz((tipo_pantalla*)(&(juego.arkanoPi.pantalla)));
	printf("FIN DEL JUEGO");
}

//void ReseteaJuego (void): función encargada de llevar a cabo la
// reinicialización de cuantas variables o estructuras resulten
// necesarias para dar comienzo a una nueva partida.
void ReseteaJuego (void) {
	// A completar por el alumno...
	InicializaJuego();
}
*/

//------------------------------------------------------
// FUNCIONES DE INICIALIZACION
//------------------------------------------------------

// int systemSetup (void): procedimiento de configuracion del sistema.
// Realizará, entra otras, todas las operaciones necesarias para:
// configurar el uso de posibles librerías (e.g. Wiring Pi),
// configurar las interrupciones externas asociadas a los pines GPIO,
// configurar las interrupciones periódicas y sus correspondientes temporizadores,
// crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
int systemSetup (void) {
	// A completar por el alumno...
	int x = 0;
	int i,j;

		piLock (STD_IO_BUFFER_KEY);
		wiringPiSetupGpio ();
		pinMode(GPIO_BUTTON_DER, INPUT);
		pinMode(GPIO_BUTTON_IZQ, INPUT);

		wiringPiISR(GPIO_BUTTON_DER, INT_EDGE_FALLING, ActivaRaquetaDer);
		wiringPiISR(GPIO_BUTTON_IZQ, INT_EDGE_FALLING, ActivaRaquetaIzq);
		for( i = 0; i<7 ;i++){
			pinMode(array_f[i],OUTPUT);
		}
		for ( j=0; j<4; j++){
			pinMode(array_c[j],OUTPUT);
		}

		//timer = tmr_new(timer_isr);

		// sets up the wiringPi library
		if (wiringPiSetupGpio () < 0) {
			printf ("Unable to setup wiringPi\n");
			piUnlock (STD_IO_BUFFER_KEY);
			return -1;
	    }

		// Lanzamos thread para exploracion del teclado convencional del PC
		x = piThreadCreate (thread_explora_teclado);

		if (x != 0) {
			printf ("it didn't start!!!\n");
			piUnlock (STD_IO_BUFFER_KEY);
			return -1;
	    }

		piUnlock (STD_IO_BUFFER_KEY);

		return 1;
}

void fsm_setup(fsm_t* arkano_fsm) {
	piLock (FLAGS_KEY);
	flags = 0;
	piUnlock (FLAGS_KEY);

	InicializaJuego(arkano_fsm);

	piLock (STD_IO_BUFFER_KEY);
	//printf("\Pulse una tecla\n");
	piUnlock (STD_IO_BUFFER_KEY);
}

//------------------------------------------------------
// PI_THREAD (thread_explora_teclado): Thread function for keystrokes detection and interpretation
//------------------------------------------------------

PI_THREAD (thread_explora_teclado) {
	int teclaPulsada;

	while(1) {
		delay(10); // Wiring Pi function: pauses program execution for at least 10 ms

		piLock (STD_IO_BUFFER_KEY);

		if(kbhit()) {
			teclaPulsada = kbread();
			//printf("Tecla %c\n", teclaPulsada);
			if(juego.estado== WAIT_START){
				piLock (FLAGS_KEY);
				flags |= FLAG_TECLA;
				piUnlock (FLAGS_KEY);
			}
			else if (juego.estado == WAIT_END ){
				piLock (FLAGS_KEY);
				flags |= FLAG_TECLA;
				piUnlock (FLAGS_KEY);
			}

			switch(teclaPulsada) {
				case 'o':
					piLock (FLAGS_KEY);
					flags |= FLAG_RAQUETA_DERECHA;
					piUnlock (FLAGS_KEY);
					break;

				case 'i':
					piLock (FLAGS_KEY);
					flags |= FLAG_RAQUETA_IZQUIERDA;
					piUnlock (FLAGS_KEY);
					break;

				case 'p':
				    piLock (FLAGS_KEY);
				    flags |= FLAG_PELOTA;
					piUnlock (FLAGS_KEY);
					break;
				case 'q':
					piLock (FLAGS_KEY);
					flags |= FLAG_FINAL_JUEGO;
					piUnlock (FLAGS_KEY);
					break;

				default:
					printf("INVALID KEY!!!\n");
					break;
			}
		}

		piUnlock (STD_IO_BUFFER_KEY);
	}
}
/*
 * TIMER QUE NO FUNCIONA
static void timer_start (int ms){
	int result = 2;
	timer_t timerid;
	struct itimerspec spec;
	struct sigevent se;

	se.sigev_notify = SIGEV_THREAD;
	se.sigev_value.sival_ptr = &timerid;
	se.sigev_notify_function = timer_isr;
	se.sigev_notify_attributes = NULL;

	spec.it_value.tv_sec = ms /1000;
	spec.it_value.tv_nsec = (ms % 1000)*1000000;
	spec.it_interval.tv_sec = ms / 1000;
	spec.it_interval.tv_nsec = (ms % 1000)* 1000000;

	timer = 0;
	result = timer_create (CLOCK_REALTIME, &se, &timerid);
	printf("result after create = %d\n", result);
	fflush(stdout);

	result = timer_settime (timerid, 0, &spec, NULL);
	printf("result after settime = %d\n", result);
	fflush(stdout);
}
void timer_isr (union sigval value){
	int columna=0;
	piLock(STD_IO_BUFFER_KEY);
	filas_a;
	switch(columna){
		case 0:
			digitalWrite(GPIO_COL_0,HIGH);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,LOW);
			for(int i=0;i<7;i++){
				if(juego.arkanoPi.pantalla.matriz[0][i]==1){
					filas_a[i]=0;
				}
			}
	}
}
*/

void timer_isr2(union sigval value){
	piLock(STD_PIN_KEY);
	flags |= FLAG_PELOTA;
	piUnlock(STD_PIN_KEY);
	tmr_startms (timer_ball,TIEMPO_PELOTA);
}


void timer_isr(union sigval value){


	piLock(STD_PIN_KEY);
	switch (columna){
		case 0:
			digitalWrite(GPIO_COL_0,LOW);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,LOW);

			break;
		case 1:
			digitalWrite(GPIO_COL_0,HIGH);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 2:
			digitalWrite(GPIO_COL_0,LOW);
			digitalWrite(GPIO_COL_1,HIGH);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 3:
			digitalWrite(GPIO_COL_0,HIGH);
			digitalWrite(GPIO_COL_1,HIGH);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 4:
			digitalWrite(GPIO_COL_0,LOW);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,HIGH);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 5:
			digitalWrite(GPIO_COL_0,HIGH);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,HIGH);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 6:
			digitalWrite(GPIO_COL_0,LOW);
			digitalWrite(GPIO_COL_1,HIGH);
			digitalWrite(GPIO_COL_2,HIGH);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 7:
			digitalWrite(GPIO_COL_0,HIGH);
			digitalWrite(GPIO_COL_1,HIGH);
			digitalWrite(GPIO_COL_2,HIGH);
			digitalWrite(GPIO_COL_3,LOW);

			break;

		case 8:
			digitalWrite(GPIO_COL_0,LOW);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,HIGH);

			break;

		case 9:
			digitalWrite(GPIO_COL_0,HIGH);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,HIGH);

			break;
		default:
			digitalWrite(GPIO_COL_0,LOW);
			digitalWrite(GPIO_COL_1,LOW);
			digitalWrite(GPIO_COL_2,LOW);
			digitalWrite(GPIO_COL_3,LOW);

			break;
	}
			int i = 0;
			for(i=0; i<7;i++){
				if(juego.arkanoPi.pantalla.matriz[columna][i]==1){
					digitalWrite(array_f[i],LOW);
				}else {
					digitalWrite(array_f[i],HIGH);
				}
			}
		piUnlock (STD_PIN_KEY);
		if (columna == 9){
			columna = 0;
		}else {
			columna++;
		}
		fflush(stdout);
		tmr_startms(timer,1);

}


int main(){
	timer = tmr_new(timer_isr);
	tmr_startms (timer,1);
	timer_ball = tmr_new(timer_isr2);
	tmr_startms (timer_ball,TIEMPO_PELOTA);
unsigned int next;

	// Maquina de estados: lista de transiciones
	// {EstadoOrigen,FunciónDeEntrada,EstadoDestino,FunciónDeSalida}
	fsm_trans_t tabla_transiciones[] = {
			{ START,   compruebaTeclaPulsada, PUSH, InicializaJuego },
			{ PUSH, compruebaTeclaPelota, PUSH,   MovimientoPelota },
			{ PUSH, compruebaTeclaRaquetaDerecha, PUSH, MueveRaquetaDerecha},
			{ PUSH, compruebaTeclaRaquetaIzquierda, PUSH, MueveRaquetaIzquierda},
			{ PUSH, compruebaFinalJuego, END, FinalJuego},
			{ END, compruebaTeclaPulsada, START, ReseteaJuego},
			{ -1, NULL, -1, NULL },
	};

	fsm_t* arkano_fsm = fsm_new (WAIT_START, tabla_transiciones, NULL);

	// Configuracion e inicializacion del sistema
	systemSetup();
	fsm_setup (arkano_fsm);

	next = millis();
	while (1) {
		fsm_fire (arkano_fsm);
		next += CLK_MS;
		delay_until (next);
	}

	fsm_destroy (arkano_fsm);
	tmr_destroy(timer);
}

/*
int main ()
{
	PintaMensajeInicialPantalla((tipo_pantalla*)(&(juego.arkanoPi.pantalla)),(tipo_pantalla*)(&(juego.arkanoPi.pantalla)));
	juego.estado = WAIT_START;
	// Configuracion e inicializacion del sistema
	// A completar por el alumno...

	while (1) {
		if(kbhit()) { // Funcion que detecta si se ha producido pulsacion de tecla alguna
			juego.teclaPulsada = kbread(); // Funcion que devuelve la tecla pulsada

			// Interpretacion de las pulsaciones para cada posible estado del sistema
			if( juego.estado == WAIT_START ) { // Cualquier pulsacion da comienzo al juego...
				// Descomente ambas lineas y sustituya cada etiqueta XXXXXXX por lo que corresponda en cada caso...
				InicializaJuego();
				juego.estado = WAIT_PUSH;
			}
			else if( juego.estado == WAIT_END ) { // Cualquier nos devuelve al estado inicial...
				// Descomente ambas lineas y sustituya cada etiqueta XXXXXXX por lo que corresponda en cada caso...
				ReseteaJuego();
				juego.estado = WAIT_START;
			}
			else { // Si estamos jugando...
				switch(juego.teclaPulsada) {
					case 'i':
						// A completar por el alumno...
						MueveRaquetaIzquierda();
						ActualizaPantalla((tipo_arkanoPi*)(&(juego.arkanoPi)));
						break;

					case 'o':
						// A completar por el alumno...
						MueveRaquetaDerecha();
						ActualizaPantalla((tipo_arkanoPi*)(&(juego.arkanoPi)));
						break;

					case 'p':
						// A completar por el alumno...
						MovimientoPelota();
						ActualizaPantalla((tipo_arkanoPi*)(&(juego.arkanoPi)));

						break;

					case 'q':
						exit(0);
						break;

					default:
						printf("INVALID KEY!!!\n");
						break;
				}
			}
		}
	}
}
*/
