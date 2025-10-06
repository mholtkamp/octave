set(TARGET EngineEditor)

file(GLOB SRC
    Source/*.cpp
)

add_executable(${TARGET}
    ${SRC}
)
target_link_libraries(${TARGET}
    EngineEditorLib
)
