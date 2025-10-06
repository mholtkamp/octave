project(octave)
# Version 3.7 needed for find_vulkan
cmake_minimum_required(VERSION 3.7)

add_subdirectory(External)

if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(ANDROID)
    set(PLATFORM_NAME "Android")
elseif(CMAKE_SYSTEM_NAME MATCHES "NintendoWii" OR
       CMAKE_SYSTEM_NAME MATCHES "NintendoGameCube")
    set(PLATFORM_NAME "Dolphin")
elseif(CMAKE_SYSTEM_NAME MATCHES "Nintendo3DS")
    set(PLATFORM_NAME "3ds")
elseif(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
    set(PLATFORM_NAME "Linux")
endif()

string(TOUPPER ${PLATFORM_NAME} PLATFORM_DEF)
add_compile_definitions(
    -DPLATFORM_${PLATFORM_DEF}
)

add_subdirectory(Engine)

add_subdirectory(Standalone)
