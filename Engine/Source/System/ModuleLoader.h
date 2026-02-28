#pragma once

/**
 * @file ModuleLoader.h
 * @brief Cross-platform dynamic library loading utility.
 *
 * Only available in EDITOR builds for hot-loading native addons.
 */

#if EDITOR

/**
 * @brief Load a dynamic library.
 *
 * @param path Path to the library file (.dll on Windows, .so on Linux)
 * @return Handle to loaded library, or nullptr on failure
 */
void* MOD_Load(const char* path);

/**
 * @brief Get a symbol (function/variable) from a loaded library.
 *
 * @param handle Library handle from MOD_Load
 * @param name Symbol name to find
 * @return Pointer to the symbol, or nullptr if not found
 */
void* MOD_Symbol(void* handle, const char* name);

/**
 * @brief Unload a dynamic library.
 *
 * @param handle Library handle from MOD_Load
 */
void MOD_Unload(void* handle);

/**
 * @brief Get the last error message.
 *
 * @return Error string, or empty string if no error
 */
const char* MOD_GetError();

#endif // EDITOR
