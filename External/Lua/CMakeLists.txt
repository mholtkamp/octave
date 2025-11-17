set(TARGET lua)

file(GLOB SRC *.c *.h)
add_library(${TARGET} STATIC
    ${SRC}
)
target_include_directories(${TARGET}
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
