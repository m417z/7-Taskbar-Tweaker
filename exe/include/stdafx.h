#pragma once

#define NTDDI_VERSION NTDDI_WIN7
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <shobjidl.h>
#include <wininet.h>
#include <stddef.h>
#include <uxtheme.h>
#include <shlwapi.h>
#include <dbghelp.h>

#include "buffer.h"
