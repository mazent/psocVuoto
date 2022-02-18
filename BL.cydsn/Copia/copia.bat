rem post build: ${ProjectDir}\copia\copia.bat ${OutputDir}\${ProjectShortName} ${ProjectDir}\bin\${ProjectShortName}
copy /Y %1.hex %2.hex
copy /Y %1.elf %2.elf
copy /Y %1.map %2.map
