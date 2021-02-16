# Test_Scripts
Varios tests para validar o medir el rendimiento de los prototipos del Virtual File System

Los tests son los siguientes:

## Rendimiento_Prototipos
Esta prueba consiste en la copia de 1000 ficheros de cierto tamaño de una carpeta a otra, para calcular el rendimiento de los prototipos de VFS usados.
Antes de realizar la prueba, hay que preparar el entorno, para no tener carpetas de un gran tamaño permanentemente en el ordenador.

1. Crea dos carpetas vacías con nombres: `Copia1` y `Copia2`
2. Ejecuta el script `preparacarpeta.bat`. Este script copia el fichero que esta en la carpeta files, en la carpeta `Copia1`, para preparar la prueba.
3. Ejecuta el script `filecopia.bat`. Este script ejecuta la copia de los 1000 ficheros de una ubicación a otra.

Cuando termines de hacer pruebas, borra el contenido de `Copia1`, ya que tiene los 1000 ficheros preparados para ser copiados.

## Test de Memory Mapping
Esta prueba consiste en la lectura de un fichero mediante un programa que hace memory mapping. Permite comprobar si una solución Secureworld es capaz de capturar lecturas realizadas mediante memory mapping y, por tanto, insertar lógica como un crifrado o descifrado.

Para preparar el test:
1. Abre la solution `mmap_test.sln` desde Visual studio y compila en Release para la arquitectura de tu SO. Deberían aparecer dos ejecutables (.exe) en la ruta `./mmap_test/x64/Release/` o bien `./mmap_test/x86/Release/`.
2. Duplica el fichero `mmap_test_execute.exe` y cambia sus nombres para distinguirlos sin errores.
3. Configura la solución de Secureworld para que cifre al leer ficheros si la aplicación que lo hace es una (y sólo una) de las copias de `mmap_test_execute.exe`.
4. Ejecuta `mmap_test_file_creator.exe`.

Para llevar a cabo el test, ejecuta seguidamente (sin esperar a que acaben) las dos copias renombradas de `mmap_test_execute.exe`.

Si la solución Secureworld es capaz de detectar las lecturas del programa que hace memory mapping, la copia cuyo nombre se haya confgurado en Secureworld para cifrar al leer mostrará por consola un texto extraño, mientras que la otra mostrará sólo "CCCCCC...". Si no es capaz, ambas mostrarán "CCCCCC...".


