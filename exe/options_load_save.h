#ifndef _OPTIONS_LOAD_SAVE_H_
#define _OPTIONS_LOAD_SAVE_H_

#include "portable_settings.h"
#include "options_def.h"

BOOL LoadOptions(int pOptions[OPTS_COUNT]);
BOOL SaveOptions(int pOptions[OPTS_COUNT]);
static void CheckOptionDependences(int pOptions[OPTS_COUNT], int nOption);

#endif // _OPTIONS_LOAD_SAVE_H_
