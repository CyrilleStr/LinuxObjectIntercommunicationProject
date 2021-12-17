
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
