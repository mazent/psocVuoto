cmake_minimum_required(VERSION 3.7)

project(Base 
	VERSION 1.2.3.4
        LANGUAGES C)

set(CMAKE_C_STANDARD 99)
	
find_program(CLANG_TIDY_COMMAND NAMES clang-tidy-12)
if(CLANG_TIDY_COMMAND)
    # --fix modifica i file
    #set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--header-filter=.;--checks=*")
    # analisi statica
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_COMMAND};--header-filter=.;--checks=-*,clang-analyzer-*")
endif()



#add_executable(target source ...)


#target_include_directories(target PUBLIC dir)

add_subdirectory(Lib)
