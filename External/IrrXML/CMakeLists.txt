set(TARGET irrxml)

file(GLOB SRC *.cpp *.h)

add_library(${TARGET} STATIC
    ${SRC}
)
target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
