if(PLATFORM_NAME MATCHES "Linux")
    set(NEEDS_VULKAN TRUE)

    find_package(PkgConfig)
    pkg_check_modules(PKG_CONFIG_DEPS REQUIRED IMPORTED_TARGET
        alsa
        ogg
        vorbis
        vorbisenc
        xcb
    )
    list(APPEND PLATFORM_DEPS PkgConfig::PKG_CONFIG_DEPS)
    # The Vulkan SDK for Linux is missing a module for shaderc, so we have to
    # add it manually. It's a dirty hack, indeed.
    find_package(Vulkan)
    string(REPLACE include lib VULKAN_PATH ${Vulkan_INCLUDE_DIR})
    link_directories(${VULKAN_PATH})
    list(APPEND PLATFORM_DEPS ${VULKAN_PATH}/libshaderc_combined.a)
    list(APPEND PLATFORM_DEPS ${VULKAN_PATH}/libspirv-cross-core.a)
elseif(PLATFORM_NAME MATCHES "Windows")
    set(NEEDS_VULKAN TRUE)
endif()

if(NEEDS_VULKAN)
    list(APPEND PLATFORM_DEFINITIONS -DAPI_VULKAN=1)
    find_package(Vulkan REQUIRED)
    list(APPEND PLATFORM_DEPS Vulkan::Vulkan)

    set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Shaders/GLSL/src)
    set(SHADER_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Shaders/GLSL/bin)
    file(GLOB SHADER_SRC
        ${SHADER_SRC_DIR}/*.comp
        ${SHADER_SRC_DIR}/*.frag
        ${SHADER_SRC_DIR}/*.vert
    )
    file(MAKE_DIRECTORY "${SHADER_OUT_DIR}")
    foreach(SHADER_SOURCE IN LISTS SHADER_SRC)
        get_filename_component(SHADER_NAME "${SHADER_SOURCE}" NAME)

        add_custom_command(OUTPUT "${SHADER_OUT_DIR}/${SHADER_NAME}"
            COMMAND Vulkan::glslc "${SHADER_SOURCE}" "-o" "${SHADER_NAME}"
            WORKING_DIRECTORY "${SHADER_OUT_DIR}"
            VERBATIM
            DEPENDS "${SHADER_SOURCE}"
        )

        list(APPEND SHADER_PRODUCTS "${SHADER_OUT_DIR}/${SHADER_NAME}")
    endforeach()

    add_custom_target(EngineShaders ALL
        COMMENT "Compiling Shaders"
        DEPENDS ${SHADER_PRODUCTS}
    )
endif()

file(GLOB_RECURSE SRC *.cpp *.c *.h)
set(COMMON_DEPS
    ${PLATFORM_DEPS}
    imgui
    irrxml
    lua
    BulletCollision
    BulletDynamics
)

set(TARGET EngineEditorLib)

add_library(${TARGET} STATIC
    ${SRC}
)
target_include_directories(${TARGET} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    ${BULLET_PHYSICS_SOURCE_DIR}/src
)
target_link_libraries(${TARGET} PUBLIC
    ${COMMON_DEPS}
    assimp::assimp
)
target_compile_definitions(${TARGET} PUBLIC
    ${PLATFORM_DEFINITIONS}
    -DEDITOR=1
)


set(TARGET EngineGameLib)

file(GLOB_RECURSE SRC *.cpp *.c *.h)
add_library(${TARGET} STATIC
    ${SRC}
)
target_include_directories(${TARGET} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    ${BULLET_PHYSICS_SOURCE_DIR}/src
)
target_link_libraries(${TARGET} PUBLIC
    ${COMMON_DEPS}
)
target_compile_definitions(${TARGET} PRIVATE
    ${PLATFORM_DEFINITIONS}
    -DEDITOR=0
)
