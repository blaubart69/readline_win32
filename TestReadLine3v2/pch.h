// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <Windows.h>

#include "CppUnitTest.h"

#include "InOutFile.h"

#include "../ReadLine3v2/src/buffered_reader.h"
#include "../ReadLine3v2/src/readline3.h"

#endif //PCH_H
