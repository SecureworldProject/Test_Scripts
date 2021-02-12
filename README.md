# Test_Scripts
Various tests to validate or measure the performance of solutions

## Rendimiento_Prototipos
Esta prueba consiste en la copia de 1000 ficheros de cierto tamaño de una carpeta a otra, para calcular el rendimiento de los prototipos de VFS usados.
Antes de realizar la prueba, hay que preparar el entorno, para no tener carpetas de un gran tamaño permanentemente en el ordenador

1. Crea dos carpetas vacias con nombres: "Copia1" y "Copia2"
2. Ejecuta el script "preparacarpeta.bat". Este script copia el fichero que esta en la carpeta files, en la carpeta Copia1, para preparar la prueba.
3. Ejecuta el script "filecopia.bat". Este script ejecuta la copia de los 1000 ficheros de una ubicacion a otra.

Cuando termines de hacer pruebas, borra el contenido de Copia1, ya que tiene los 1000 ficheros preparados para ser copiados.
