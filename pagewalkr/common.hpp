#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "ia32.hpp"

// levels are only used to easily comment out chunks of prints to make debugging easier
#define log(...) DbgPrintEx(0, 0, __VA_ARGS__); // only used to print startup info
#define log_info(...) DbgPrintEx(0, 0, __VA_ARGS__);
#define kd_break if (KD_DEBUGGER_ENABLED) __debugbreak();

#include "imports.hpp"
#include "scanner.hpp"
#include "utils.hpp"