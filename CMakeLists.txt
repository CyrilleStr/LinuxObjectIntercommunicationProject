
project(sy40)

cmake_minimum_required(VERSION 3.12)

set(SRCS
    src/main.c
    src/vehicule.c
    src/portique.c
    )
    
set(HEADERS
    inc/portique.h
    inc/vehicule.h
    inc/general.h
    )

add_executable(sy40 ${SRCS} ${HEADERS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(sy40 PRIVATE Threads::Threads)
