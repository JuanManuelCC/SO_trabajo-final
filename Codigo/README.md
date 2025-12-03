# Mi Proyecto - Diseño, Implementación y Evaluación de un Sistema Operativo Simplificado en Entornos Virtualizados

Este proyecto se puede clonar y compilar en sistemas operativos basados en Linux, como Ubuntu, utilizando el compilador `gcc-5`. Este README sirve como guia de instalacion para poder hacer correr el proyecto 

## Requisitos

Asegúrate de tener instalados los siguientes componentes en tu sistema:

1. **VIM**: Editor de texto utilizado para modificar archivos de configuración, como sources.list, que permitió agregar los repositorios necesarios para instalar gcc-5.

2. **gcc-5**: El compilador requerido para este proyecto.

3. **quemu**: Herramienta de emulación y virtualización para probar el kernel generado en un entorno simulado con soporte para arquitecturas x86.

### Instalación de VIM

1. Actualizamos la lista de paquetes disponibles:

```bash
sudo apt update
```

2. Instala Vim:

```bash
sudo apt install vim
```

3. Verifica la instalación:

```bash
vim -- version
```

### Instalación de gcc-5
Luego de haber instalado VIM hacemos los siguiente:

1. Editar el archivo sources.list con Vim:
 ```bash
 sudo vim /etc/apt/sources.list
 ```
 Dentro del archivo, añade las siguientes líneas al final del documento:

 deb http://dk.archive.ubuntu.com/ubuntu/ xenial main
 deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe

 Para guardar los cambios en Vim:

Presiona ESC para salir del modo de inserción.
Escribe :wq y presiona Enter para guardar y salir.

2. Actualizar los repositorios e instalar gcc-5 y g++-5:
Ejecuta los siguientes comandos:
 ```bash
 sudo apt update
 sudo apt install g++-5 gcc-5
 ```
3. Configurar gcc-5 como la versión predeterminada:

 ```bash
 sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 5
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 5
 ```

### Instalación de quemu
1. Actualizar la lista de paquetes disponibles:
```bash
sudo apt update
 ```
2. Instalar QEMU con soporte para x86:
```bash
sudo apt-get install qemu-system-x86
 ```

3. Verificar la instalación de QEMU:
```bash
qemu-system-x86_64 --version
 ```

### Compilar y Ejecutar el Proyecto
Una vez instalados todos los requisitos, siguemos estos pasos para compilar y ejecutar el proyecto:

1. Accedemos al directorio del proyecto:
```bash
cd /ruta/a/tu/proyecto
 ```

2. Limpiamos compilaciones anteriores (si las hay):
```bash
make clean
 ```

3. Compilamos el proyecto:
```bash:
make 
 ```

4. Ejecutamos el proyecto con QEMU:
```bash
make run
 ```
 
###Para cambiar el tipo de algoritmo a usar cambiar la variable : current_algorithm en main.c 
###Por alguno de los siguientes:
###   FCFS,     //First-Come, First-Served
###    SJF,      //Shortest Job First
###    SRT,      //Shortest Remaining Time
###    PRIORITY, //Planificación basada en prioridades
###    RR        //Round Robin
