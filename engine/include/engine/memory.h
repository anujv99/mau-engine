#pragma once

#include <engine/profiler.h>

#define MAU_ALLOC(ptr, type, ...) { ptr = new type(__VA_ARGS__); TracyAlloc(ptr, sizeof(type)); }
#define MAU_ALLOC_BYTES(ptr, size) { ptr = reinterpret_cast<void*>(new char[size]); TracyAlloc(ptr, size); }
#define MAU_FREE(ptr) { TracyFree(ptr); delete ptr; ptr = nullptr; }
#define MAU_FREE_BYTES(ptr) { TracyFree(ptr); delete[] ptr; ptr = nullptr; }
