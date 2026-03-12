#pragma once

/**
 * @file OctaveAPI.h
 * @brief Export macros for Octave Engine symbols.
 *
 * This header defines OCTAVE_API which is used to export symbols from the
 * engine executable so that native addon DLLs can link against them.
 *
 * Build Configuration:
 * - When building Octave.exe: Define OCTAVE_ENGINE_EXPORT
 * - When building native addons: Don't define OCTAVE_ENGINE_EXPORT (symbols imported)
 */

#ifdef _WIN32
    #ifdef OCTAVE_ENGINE_EXPORT
        // Building the engine - export symbols
        #define OCTAVE_API __declspec(dllexport)
    #else
        // Building a plugin/addon - import symbols from engine
        #define OCTAVE_API __declspec(dllimport)
    #endif
#else
    // On Linux/other platforms, use visibility attribute
    #define OCTAVE_API __attribute__((visibility("default")))
#endif

// Convenience macro for classes that should be fully exported
// Use: class OCTAVE_API MyClass { ... };
// This exports the class's vtable, RTTI, and all members

// For template instantiations, use explicit instantiation:
// extern template class OCTAVE_API SmartPointer<MyClass>;
