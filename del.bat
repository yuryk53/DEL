@goto start
--------------------------
Весия 2.0
Даный пакетный файл удаляет все ярлыки и файл autorun.inf
потенциально созданный вирусным исполняемым файлом.
Если используется система выше Win Xp, требуется запуск
от имени Администратора.
--------------------------
Пакетный файл написан 22/04/2012
Copyright Yura Bilyk
--------------------------
:start
@echo made by progr@2mer{}
@echo Yuryk53@yandex.ru
@echo version 2.0
@echo NOW WORKS WITH WINDOWS 7
@echo Please, if you're using Win 7 run this under Administrator!!!
@ping localhost -n 5 > nul
:: Задержка на 5 секунд
@cd %~d0\
@%~d0
@echo.
@echo                             ----Now working----
@echo.
@IF EXIST "*.lnk" del *.lnk
@IF NOT EXIST *.lnk echo All link files have been deleted...
@echo.
@ATTRIB -s -h -r -a "*"  /S /D
@echo Attributes have been changed...
@echo.
@IF EXIST "autorun.inf" del /F /S /Q autorun.inf 
@IF NOT EXIST "autorun.inf" echo The "autorun.inf" file has been deleted...
@echo.
@IF NOT EXIST "autorun.inf" mkdir "autorun.inf"
@ATTRIB +s +r "autorun.inf"  /S /D
@echo The "Autorun.inf" dir has been created...
@IF EXIST "RECYCLER" RD /S /Q "RECYCLER"
@echo.
@echo Thank you for using this...
@ping localhost -n 5 > nul  
exit