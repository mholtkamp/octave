set(TARGET imgui)

file(GLOB SRC *.cpp *.h)
file(GLOB MISC
    misc/cpp/*.cpp
    misc/cpp/*.h
)

add_library(${TARGET} STATIC
    ${SRC}
    ${MISC}
)
target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
