set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB ON)
add_subdirectory(assimp)

set(BUILD_CPU_DEMOS OFF)
set(BUILD_OPENGL3_DEMOS OFF)
set(BUILD_BULLET2_DEMOS OFF)
set(BUILD_EXTRAS OFF)
set(BUILD_UNIT_TESTS OFF)
set(INSTALL_LIBS OFF)
set(INSTALL_CMAKE_FILES OFF)
add_subdirectory(bullet3)

add_subdirectory(Lua)

add_subdirectory(Imgui)

add_subdirectory(IrrXML)
