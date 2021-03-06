/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del scheduler
*/

#include "sched.h"
#include "defines.h"

// vector con tareas registradas (12 entradas, )
int16_t tareas[12];
// int tarea actual
uint8_t iTareaActual = 0;

int16_t ballsTSS[6] = {
	GDT_IDX_TSS_PA_T1 << 3,
	GDT_IDX_TSS_PA_T2 << 3,
	GDT_IDX_TSS_PA_T3 << 3,
	GDT_IDX_TSS_PB_T1 << 3,
	GDT_IDX_TSS_PB_T2 << 3,
	GDT_IDX_TSS_PB_T3 << 3,
};

int16_t handlersTSS[6] = {
	GDT_IDX_TSS_PA_T1_H << 3,
	GDT_IDX_TSS_PA_T2_H << 3,
	GDT_IDX_TSS_PA_T3_H << 3,
	GDT_IDX_TSS_PB_T1_H << 3,
	GDT_IDX_TSS_PB_T2_H << 3,
	GDT_IDX_TSS_PB_T3_H << 3,
};

f_handler_t *handlers[6];

extern void saltarAIdle();

int sched_isHandler() {
	return iTareaActual % 2 == 0;
}

void task_inc() {
	iTareaActual = (iTareaActual + 1) % 12;
}

void sched_init() {
	for (int i = 0; i < 12; ++i) {
		tareas[i] = 0;
	}
}

int16_t sched_nextTask() {
	task_inc();

	// Si debería ejecutar un handler y no lo tengo declarado, voy a la task directo
	if (sched_isHandler()) {
		if (tareas[iTareaActual] == 0){
			task_inc();
		} else {
			// Resetteo el handler antes de ejecutarlo!
			PLAYER p = sched_getTareaActual();
			tss_ball_handler_reset(p, handlers[p]);
		}
	}

	// Si no hay tarea definida, simplemente ejecuto la idle
	if (tareas[iTareaActual] == NULL) {
		return GDT_IDX_TSS_IDLE << 3;
	}

  	return tareas[iTareaActual];
}

void sched_newBall(PLAYER ballType) {
	/*	
		1. Obtenemos la TSS de la pelota
		2. La ponemos en la posición correcta del vector
		3. Resetteamos la TSS!
	*/
	tareas[ballType * 2 + 1] = ballsTSS[ballType];
	tss_ball_reset(ballType);
}

uint32_t sched_getTareaActual() {
	// Obtiene el número de tarea actual (agrupando handler y tarea)
	// Nota: la división entera trunca la parte decimal, por lo que
	//		 hace lo que queremos :D
	return iTareaActual / 2;
}

void sched_registerHandler(f_handler_t* f) {
	sched_killIfHandler();
	uint32_t player = sched_getTareaActual();
	// El handler siempre está en el índice anterior a la tarea:
	uint8_t handlerIndex = iTareaActual - 1;
	if (handlers[player] != NULL) {
		// Entonces ya estaba setteado el handler!
		sched_makeItLookLikeAnAccident();
	} else {
		tareas[handlerIndex] = handlersTSS[player];
		handlers[player] = f;
	}
}

void sched_killIfNotHandler() {
	if (!sched_isHandler()) {
		// Entonces no es un handler!
		sched_makeItLookLikeAnAccident();
	}
}

void sched_killIfHandler() {
	if (sched_isHandler()) {
		// Entonces es un handler!
		task_inc(); // Normalizamos el estado del scheduler
		sched_makeItLookLikeAnAccident();
	}
}

void sched_makeItLookLikeAnAccident() {
	sched_killCurrent();
	saltarAIdle();
}

void sched_killCurrent() {
	uint32_t player = sched_getTareaActual();
	sched_killBall(player);
}

void sched_killBall(int iBall) {
	tareas[iBall * 2] = NULL;
	tareas[iBall * 2 + 1] = NULL;
	handlers[iBall] = NULL;
}
