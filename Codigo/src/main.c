#include <linux/printk.h>  //Para imprimir mensajes en el kernel.
#include <linux/string.h>  //Para usar funciones de manipulación de cadenas, como memcpy.

#include "mypcb.h"  //Incluir el archivo de encabezado que define la estructura de control de procesos (PCB).

//Definición de un arreglo de tareas, y la tarea actualmente en ejecución.
myPCB task[MAX_TASK_NUM], *my_current_task = NULL;

//Variable que indica si se necesita una reprogramación de las tareas.
volatile int my_need_sched = 0;

//Declaración de las funciones externas.
extern void my_schedule(void); //Función de planificación de tareas.
extern enum SchedulingAlgorithm current_algorithm; //Algoritmo de planificación actual.

static void my_process(void); //Función que representa el cuerpo de las tareas.
static void run_simulation(int cycles); //Función que simula la ejecución del sistema durante ciertos ciclos.

void __init my_start_kernel(void)
{
    //INCIALIZANDO PROCESOS DE EJEMPLO

    int pid = 0; //Variable que representa el identificador del proceso (PID).

    /* Inicialización del proceso 0 */
    task[pid].pid = pid; // Asignar el PID.
    task[pid].state = S_runnable; // Establecer el estado del proceso como ejecutable.
    task[pid].task_entry = task[pid].thread.ip = (uintptr_t) my_process; // Establecer la dirección de entrada del proceso (función que ejecuta).
    task[pid].thread.sp = (uintptr_t) &task[pid].stack[KERNEL_STACK_SIZE - 1]; // Establecer la pila del proceso.
    task[pid].next = &task[pid]; // El siguiente proceso en la lista es el mismo (se circulariza).
    task[pid].priority = 5;  // Establecer una prioridad manual.
    task[pid].expected_burst = 10;  // Establecer el tiempo de ejecución esperado.
    task[pid].remaining_burst = task[pid].expected_burst; // Inicializar el burst restante con el valor esperado.
    task[pid].time_slice = 3;  // Establecer el time slice manual (usado en Round Robin).

    /* Inicialización del proceso 1 */
    pid = 1;  // Cambiar el PID al siguiente proceso.
    task[pid].pid = pid;
    task[pid].state = S_runnable; // Establecer el proceso como ejecutable.
    task[pid].task_entry = task[pid].thread.ip = (uintptr_t) my_process; // Asignar la función que ejecuta el proceso.
    task[pid].thread.sp = (uintptr_t) &task[pid].stack[KERNEL_STACK_SIZE - 1]; // Asignar la pila.
    task[pid].priority = 1;  // Establecer prioridad 1.
    task[pid].expected_burst = 5;  // Establecer tiempo de ejecución esperado.
    task[pid].remaining_burst = task[pid].expected_burst; // Inicializar el burst restante.
    task[pid].time_slice = 2;  // Establecer time slice de 2.
    task[pid].next = &task[0]; // Enlazar el proceso 1 al proceso 0.

    /* Inicialización del proceso 2 */
    pid = 2;
    task[pid].pid = pid;
    task[pid].state = S_runnable;
    task[pid].task_entry = task[pid].thread.ip = (uintptr_t) my_process;
    task[pid].thread.sp = (uintptr_t) &task[pid].stack[KERNEL_STACK_SIZE - 1];
    task[pid].priority = 3;
    task[pid].expected_burst = 8;
    task[pid].remaining_burst = task[pid].expected_burst;
    task[pid].time_slice = 4;
    task[pid].next = &task[1]; // Enlazar el proceso 2 al proceso 1.

    /* Inicialización del proceso 3 */
    pid = 3;
    task[pid].pid = pid;
    task[pid].state = S_runnable;
    task[pid].task_entry = task[pid].thread.ip = (uintptr_t) my_process;
    task[pid].thread.sp = (uintptr_t) &task[pid].stack[KERNEL_STACK_SIZE - 1];
    task[pid].priority = 7;
    task[pid].expected_burst = 12;
    task[pid].remaining_burst = task[pid].expected_burst;
    task[pid].time_slice = 3;
    task[pid].next = &task[2]; // Enlazar el proceso 3 al proceso 2.

    /* Enlazar todos los procesos en una lista circular */
    for (pid = 0; pid < MAX_TASK_NUM; pid++) {
        task[pid].next = &task[(pid + 1) % MAX_TASK_NUM]; // El último proceso se enlaza al primero.
    }

    /* Asegurarse de que el primer proceso se seleccione para ejecución */
    my_current_task = &task[0];

    /* Seleccionar el algoritmo de planificación */
    printk(KERN_NOTICE "Seleccion de algoritmo\n"); // Imprimir mensaje de cambio de algoritmo.
    current_algorithm = FCFS;  //Seleccionar el algoritmo FCFS por defecto (First-Come, First-Served).
    run_simulation(10); //Iniciar la simulación con 10 ciclos.

    printk(KERN_NOTICE "Simulacion completa\n"); // Imprimir mensaje indicando que la simulación ha terminado.
}

static void my_process(void)
{
    while (1) {
        if (my_need_sched == 1) { //Comprobar si se necesita reprogramación.
            my_need_sched = 0; //Restablecer la necesidad de reprogramación.
            my_schedule(); //Llamar a la función de planificación para cambiar de tarea.
        }
    }
}

static void run_simulation(int cycles)
{
    int i;
    for (i = 0; i < cycles; i++) { //Ejecutar la simulación durante el número de ciclos especificados.
        if (my_current_task) { //Comprobar si hay una tarea actualmente en ejecución.
            printk(KERN_NOTICE "Ciclo %d: PID %d, Estado: %d, Prioridad: %d, Burst Restante: %d\n",
                   i,
                   my_current_task->pid, // Imprimir el PID del proceso actual.
                   my_current_task->state, // Imprimir el estado del proceso.
                   my_current_task->priority, // Imprimir la prioridad del proceso.
                   my_current_task->remaining_burst); // Imprimir el burst restante.
        } else {
            printk(KERN_NOTICE "Ciclo %d: No hay proceso en ejecucion\n", i); //Si no hay proceso en ejecución, imprimir un mensaje.
        }

        if (my_need_sched == 0 && my_current_task) { //Comprobar si no se necesita una reprogramación y hay una tarea en ejecución.
            my_schedule(); //Llamar a la función de planificación para cambiar de tarea.
        } else {
            printk(KERN_ERR "Error: no hay proceso actual o necesidad de planificacion no marcada\n"); // Si no hay proceso o hay un error en la planificación, imprimir mensaje de error.
            break; //Salir del bucle de simulación.
        }
    }
}
