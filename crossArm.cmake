set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(gcc_dir
	"/home/massimo/gnu.arm/gcc-arm-none-eabi-9-2020-q2-update")
set(CMAKE_C_COMPILER ${gcc_dir}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${gcc_dir}/bin/arm-none-eabi-g++)

set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")

#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
