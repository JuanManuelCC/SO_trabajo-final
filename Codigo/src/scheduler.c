#include <linux/printk.h>      //Cabecera para la impresión de mensajes en el kernel.
#include <linux/delay.h>       //Cabecera para las funciones de retardo.
#include "mypcb.h"             //Cabecera que incluye la definición de la estructura myPCB.

extern myPCB task[MAX_TASK_NUM], *my_current_task; //Declaración de las tareas y el puntero a la tarea actual.
extern volatile int my_need_sched; //Variable para indicar si se necesita hacer planificación.
volatile int time_count = 0;  //Contador de tiempo, usado para el manejo de temporizadores.

enum SchedulingAlgorithm current_algorithm = RR; // Algoritmo de planificación actual, inicialmente Round Robin (RR).
#define TIME_QUANTUM 3 // Quantum de tiempo para el algoritmo Round Robin.

 /*
  * Llamada por la interrupción del temporizador.
  * Se ejecuta en el contexto del proceso que está corriendo actualmente,
  * usando la pila del proceso actual.
  */
void my_timer_handler(void)
{
    if (current_algorithm == RR) { //Si el algoritmo de planificación es Round Robin (RR).
        if (my_current_task != NULL) {// Si hay un proceso en ejecución.
            // Decrementar el time slice de la tarea actual.
            if (my_current_task->time_slice > 0) {
                my_current_task->time_slice--;
            }

            //Si el time slice se agotó o el proceso terminó (remaining_burst == 0),
            //se debe hacer una nueva planificación.
            if (my_current_task->time_slice == 0 || my_current_task->remaining_burst == 0) {
                if (my_current_task->remaining_burst == 0) { //Si el proceso ha terminado, cambiar su estado.
                    my_current_task->state = S_stopped;
                }

                //Indicar que es necesario hacer una nueva planificación.
                my_need_sched = 1;
            }
        }
    } else { //Si no se usa Round Robin, se realiza planificación basada en tiempo.
        if (time_count % 1000 == 0 && my_need_sched != 1) {
            my_need_sched = 1; // Indicar que se necesita una nueva planificación.
        }
    }
    time_count++; // Incrementar el contador de tiempo.
}

//imprimir estado de los procesos
void print_all_tasks_info(void)
{
    myPCB *current = my_current_task; //Inicializar el puntero al proceso actual.

    if (current == NULL) { //Si no hay procesos disponibles.
        printk(KERN_NOTICE "No hay procesos.\n");
        return;
    }

    printk(KERN_NOTICE ">>> Listando todos los procesos <<<\n"); //Imprimir la cabecera para la lista de tareas.

    do { // Iterar sobre todas las tareas en la lista.
        printk(KERN_NOTICE "PID: %d, Estado: %d, Prioridad: %d, Burst Restante: %d, Time Slice: %d\n",
               current->pid, current->state, current->priority,
               current->remaining_burst, current->time_slice);
        current = current->next; //Avanzar al siguiente proceso.
    } while (current != my_current_task); // Terminar cuando se vuelva al proceso inicial.

    printk(KERN_NOTICE ">>> ////////// <<<\n"); // Imprimir el final de la lista de tareas.
}

//Planificación basada en el algoritmo FCFS (First-Come, First-Served).
void my_schedule_fcfs(void)
{
    myPCB *next, *prev; //Punteros a las tareas actual y siguiente.

    if (my_current_task == NULL || my_current_task->next == NULL) //Si no hay tareas o solo una tarea.
        return;

    prev = my_current_task; //Inicializar el proceso anterior.
    next = my_current_task->next; //Inicializar el siguiente proceso.

    while (next != my_current_task) { //Iterar sobre todos los procesos.
        if (next->state == S_runnable) { //Si el proceso siguiente es ejecutable.
            // Decrementar el remaining_burst de la tarea actual antes de cambiar de contexto.
            if (my_current_task->remaining_burst > 0) {
                my_current_task->remaining_burst--;
            }

            my_current_task = next; //Cambiar al siguiente proceso.
            printk(KERN_NOTICE ">>>cambiando  de %d a %d<<<\n", prev->pid, next->pid); //Imprimir la conmutación.
            printk(KERN_NOTICE "Proceso actual - PID: %d, Estado: %d, Prioridad: %d, Burst Restante: %d\n",
                   my_current_task->pid, my_current_task->state,
                   my_current_task->priority, my_current_task->remaining_burst);

            asm volatile( //Realizar el cambio de contexto entre procesos utilizando assembly.
                "movl    %%esp, %0\n\t"  /*guardar el esp (puntero de pila) */
                "movl    %2, %%esp\n\t"  /*restaurar el esp */
                "movl    $1f, %1\n\t"    /*guardar la dirección de la instrucción siguiente */
                "jmp    *%3\n"           /*saltar a la dirección de inicio del siguiente proceso */
                "1:\t"                   /*El siguiente proceso comienza aquí */
                : "=m" (prev->thread.sp), "=m" (prev->thread.ip)
                : "m" (next->thread.sp), "m" (next->thread.ip)
            );
            return; //Salir de la función una vez se haya hecho el cambio de contexto.
        }
        next = next->next; //Avanzar al siguiente proceso.
    }
    printk(KERN_NOTICE "No hay procesos ejecutables.\n"); //Si no hay tareas ejecutables
    print_all_tasks_info(); //Imprimir información de todas las tareas.
}

//Planificación basada en el algoritmo SJF (Shortest Job First).
void my_schedule_sjf(void)
{
    myPCB *next, *prev, *shortest = NULL; //Punteros a las tareas actual, siguiente y la más corta.

    if (my_current_task == NULL || my_current_task->next == NULL) //Si no hay tareas o solo una tarea.
        return;

    prev = my_current_task; //Inicializar el proceso anterior.
    next = my_current_task; //Inicializar el siguiente proceso.

    do {
        if (next->state == S_runnable && (shortest == NULL || next->remaining_burst < shortest->remaining_burst)) {
            shortest = next; //Si el proceso siguiente tiene el menor remaining_burst, seleccionarlo.
        }
        next = next->next; //Avanzar al siguiente proceso.
    } while (next != my_current_task); // Terminar cuando se vuelva al proceso inicial.

    if (shortest) { //Si se encontró el proceso más corto.
        //Decrementar el remaining_burst de la tarea actual antes de cambiar de contexto.
        if (my_current_task->remaining_burst > 0) {
            my_current_task->remaining_burst--;
        }

        my_current_task = shortest; //Cambiar al proceso más corto.
        printk(KERN_NOTICE ">>>cambiando de %d a %d<<<\n", prev->pid, shortest->pid); // Imprimir la conmutación.
        printk(KERN_NOTICE "Proceso actual - PID: %d, Estado: %d, Prioridad: %d, Burst Restante: %d\n",
               my_current_task->pid, my_current_task->state,
               my_current_task->priority, my_current_task->remaining_burst);

        asm volatile( //Realizar el cambio de contexto entre procesos utilizando assembly.
            "movl    %%esp, %0\n\t"  /* guardar el esp */
            "movl    %2, %%esp\n\t"  /* restaurar el esp */
            "movl    $1f, %1\n\t"    /* guardar la dirección de la instrucción siguiente */
            "jmp    *%3\n"
            "1:\t"                   /* El siguiente proceso comienza aquí */
            : "=m" (prev->thread.sp), "=m" (prev->thread.ip)
            : "m" (shortest->thread.sp), "m" (shortest->thread.ip)
        );
    } else {
        printk(KERN_NOTICE "No hay procesos ejecutables.\n"); //Si no se encontró ningún proceso ejecutable.
        print_all_tasks_info(); //Imprimir información de todas las tareas.
    }
}

//Planificación basada en el algoritmo de prioridad.
void my_schedule_priority(void)
{
    myPCB *next, *prev, *highest_priority = NULL; //Punteros a las tareas actual, siguiente y la de mayor prioridad.

    if (my_current_task == NULL || my_current_task->next == NULL) //Si no hay tareas o solo una tarea.
        return;

    prev = my_current_task; //Inicializar el proceso anterior.
    next = my_current_task; //Inicializar el siguiente proceso.

    do {
        if (next->state == S_runnable && (highest_priority == NULL || next->priority > highest_priority->priority)) {
            highest_priority = next; //Si el proceso siguiente tiene la mayor prioridad, seleccionarlo.
        }
        next = next->next; //Avanzar al siguiente proceso.
    } while (next != my_current_task); //Terminar cuando se vuelva al proceso inicial.

    if (highest_priority) { // Si se encontró el proceso con la mayor prioridad.
        // Decrementar el remaining_burst de la tarea actual antes de cambiar de contexto.
        if (my_current_task->remaining_burst > 0) {
            my_current_task->remaining_burst--;
        }

        my_current_task = highest_priority; //Cambiar al proceso de mayor prioridad.
        printk(KERN_NOTICE ">>>cambiando  de %d a %d<<<\n", prev->pid, highest_priority->pid); //Imprimir la conmutación.
        printk(KERN_NOTICE "Proceso actual - PID: %d, Estado: %d, Prioridad: %d, Burst Restante: %d\n",
               my_current_task->pid, my_current_task->state,
               my_current_task->priority, my_current_task->remaining_burst);

        asm volatile( //Realizar el cambio de contexto entre procesos utilizando assembly.
            "movl    %%esp, %0\n\t"  /* guardar el esp */
            "movl    %2, %%esp\n\t"  /* restaurar el esp */
            "movl    $1f, %1\n\t"    /* guardar la dirección de la instrucción siguiente */
            "jmp    *%3\n"
            "1:\t"                   /* El siguiente proceso comienza aquí */
            : "=m" (prev->thread.sp), "=m" (prev->thread.ip)
            : "m" (highest_priority->thread.sp), "m" (highest_priority->thread.ip)
        );
    } else {
        printk(KERN_NOTICE "No hay procesos ejecutables.\n"); //Si no se encontró ningún proceso ejecutable.
        print_all_tasks_info(); //Imprimir información de todas las tareas.
    }
}

// Planificación basada en el algoritmo Round Robin.
void my_schedule_rr(void)
{
    myPCB *next, *prev; //Punteros a las tareas actual y siguiente.

    if (my_current_task == NULL || my_current_task->next == NULL) {// Si no hay tareas o solo una tarea.
        printk(KERN_NOTICE "No tasks to schedule.\n");
        return;
    }

    prev = my_current_task; //Inicializar el proceso anterior.
    next = my_current_task->next; //Inicializar el siguiente proceso.

    //Buscar el siguiente proceso ejecutable.
    while (next != my_current_task) {
        if (next->state == S_runnable) { // Si el proceso siguiente es ejecutable.
            // Decrementar el remaining_burst de la tarea actual antes de cambiar de contexto.
            if (my_current_task->remaining_burst > 0) {
                my_current_task->remaining_burst--;
            }

            // Reiniciar el time slice para el siguiente proceso.
            next->time_slice = TIME_QUANTUM; 
            my_current_task = next; // Cambiar al siguiente proceso.

            printk(KERN_NOTICE ">>>cambiando de  %d a %d<<<\n", prev->pid, next->pid); // Imprimir la conmutación.
            printk(KERN_NOTICE "Proceso actual - PID: %d, Estado: %d, Prioridad: %d, Burst Restante: %d\n",
                   my_current_task->pid, my_current_task->state,
                   my_current_task->priority, my_current_task->remaining_burst);

            asm volatile( // Realizar el cambio de contexto entre procesos utilizando assembly.
                "movl    %%esp, %0\n\t"  /* guardar el esp */
                "movl    %2, %%esp\n\t"  /* restaurar el esp */
                "movl    $1f, %1\n\t"    /* guardar la dirección de la instrucción siguiente */
                "jmp    *%3\n"
                "1:\t"                   /* El siguiente proceso comienza aquí */
                : "=m" (prev->thread.sp), "=m" (prev->thread.ip)
                : "m" (next->thread.sp), "m" (next->thread.ip)
            );
            return; // Salir de la función una vez se haya hecho el cambio de contexto.
        }
        next = next->next; // Avanzar al siguiente proceso.
    }

    // Si no se encontró un proceso ejecutable.
    printk(KERN_NOTICE "No hay procesos ejecutables.\n");
    print_all_tasks_info(); // Imprimir información de todas las tareas.
}

//Función principal de planificación.
void my_schedule(void)
{
    switch (current_algorithm) { //Según el algoritmo de planificación seleccionado.
        case FCFS:
            my_schedule_fcfs(); //Llamar a la planificación FCFS.
            break;
        case SJF:
            my_schedule_sjf(); //Llamar a la planificación SJF.
            break;
        case PRIORITY:
            my_schedule_priority(); //Llamar a la planificación basada en prioridad.
            break;
        case RR:
            my_schedule_rr(); //Llamar a la planificación Round Robin.
            break;
        default:
            printk(KERN_WARNING "error\n"); // Si el algoritmo es desconocido.
    }

    //Si el proceso actual terminó, actualizar su estado y hacer nueva planificación.
    if (my_current_task && my_current_task->remaining_burst == 0) {
        my_current_task->state = S_stopped; //El proceso se detiene si su remaining_burst es 0.
        my_need_sched = 1; //Indicar que se necesita una nueva planificación.
    }
}
