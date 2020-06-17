rem post build: ${ProjectDir}\versione\copia.bat ${OutputDir}\${ProjectShortName} ${ProjectDir}\bin\${ProjectShortName}
copy /Y %1.hex %2.hex
copy /Y %1.cyacd %2.cyacd
rem python .\versione\copia.py %1.cyacd %2.cyacd
