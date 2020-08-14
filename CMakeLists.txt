cmake_minimum_required(VERSION 3.7)

project(Base 
	VERSION 1.2.3.4
        LANGUAGES C)

set(CMAKE_C_STANDARD 99)
	
#add_executable(target source ...)


#target_include_directories(target PUBLIC dir)

add_subdirectory(Lib)
