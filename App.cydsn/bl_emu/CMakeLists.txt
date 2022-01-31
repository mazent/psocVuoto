set(CODICE
	bl_emu.c
	bl_tel.c
	BootloaderEmulator.c
	Encryption.c
	ExternalMemoryInterfaceXMEM.c
)

add_library(bl_emu OBJECT ${CODICE})
