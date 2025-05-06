cmake_minimum_required(VERSION 3.30)
project(os_project C)

set(CMAKE_C_STANDARD 11)

add_executable(os_project main_mutex.c
        main_semaphore.c
        main.c
)
