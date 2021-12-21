
project(sy40)

set(SRCS
    src/main.c
    src/vehicule.c
    src/portique.c
    src/quai.c
    )
    
set(HEADERS
    inc/portique.h
    inc/quai.h
    inc/vehicule.h
    )

add_executable(sy40 ${SRCS} ${HEADERS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(sy40 PRIVATE Threads::Threads)
