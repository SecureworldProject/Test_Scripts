@echo off
echo Bienvenido al programa de copia
echo Proceso iniciado...
for /l %%x in (1,1,1000) do (
	rem echo %%x
	copy "Files\imagetest.jpg" "Copia1\imagetest%%x.jpg" > nul
	)
pause