#ifndef MYPCB_H
#define MYPCB_H

//inclusión de la cabecera necesaria para tipos de datos definidos por el sistema Linux.
#include <linux/types.h>

//definición del número máximo de tareas.
#define MAX_TASK_NUM        4

//tamaño de la pila del kernel para cada tarea.
#define KERNEL_STACK_SIZE   (1024 * 8)

//definición del quantum de tiempo para el algoritmo Round Robin (RR).
#define TIME_QUANTUM        10

//estructura que representa el estado específico de la CPU para esta tarea 
struct myThread {
    uintptr_t ip; // Dirección de la instrucción (registro IP) que se ejecutará a continuación.
    uintptr_t sp; // Dirección de la pila (registro SP) donde se almacenan los datos de contexto.
};

//Estados de un proceso
enum myState {
    S_unrunnable = -1,  // El proceso no es ejecutable.
    S_runnable = 0,     // El proceso es ejecutable.
    S_stopped = 1,      // El proceso está detenido.
    S_terminated = 2    // El proceso ha terminado.
};

// Definición de la estructura myPCB (Control de Bloque de Proceso) que almacena los datos de un proceso
typedef struct _myPCB {
    int pid;                      //id del proceso (PID).
    volatile enum myState state;   //estado actual del proceso
    char stack[KERNEL_STACK_SIZE]; //espacio para la pila del proceso.
    
    //Estado específico de la CPU para este proceso (información de registros)
    struct myThread thread;        

    uintptr_t task_entry;          //dirección de inicio de la tarea función que ejecuta el proceso.
    struct _myPCB *next;           //puntero al siguiente PCB en la lista de tareas enlazado de forma circular.
    
    int priority;                  //Prioridad de la tarea (usado en la planificación por prioridades).
    int expected_burst;            //Tiempo de ejecución esperado (usado en SJF, Shortest Job First).
    int remaining_burst;           //Tiempo de ejecución restante (usado en SRT, Shortest Remaining Time).
    int time_slice;                //Tiempo asignado para ejecutar la tarea (usado en Round Robin).
} myPCB;

//tipos de algoritmos del scheduler
enum SchedulingAlgorithm {
    FCFS,     //First-Come, First-Served
    SJF,      //Shortest Job First
    SRT,      //Shortest Remaining Time
    PRIORITY, //Planificación basada en prioridades
    RR        //Round Robin
};

//Declaración del algoritmo de planificación actual en uso.
extern enum SchedulingAlgorithm current_algorithm;

//Declaración de la función de planificación que se llama para cambiar de tarea.
void my_schedule(void);

#endif // MYPCB_H
