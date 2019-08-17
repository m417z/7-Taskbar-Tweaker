#pragma once

/*
Error ranges:

1-100
	Injected code
101-1000
	DLL loading issues
1001-2000
	EXE loading issues
>2000
	Reserved (we have only 12 bits - max 4095)
*/

#define INJ_ERR_BEFORE_RUN                   1
#define INJ_ERR_BEFORE_GETMODULEHANDLE       2
#define INJ_ERR_BEFORE_LOADLIBRARY           3
#define INJ_ERR_BEFORE_GETPROCADDR           4
#define INJ_ERR_BEFORE_LIBINIT               5
#define INJ_ERR_GETMODULEHANDLE              6
#define INJ_ERR_LOADLIBRARY                  7
#define INJ_ERR_GETPROCADDR                  8

#define LIB_ERR_INIT_ALREADY_CALLED          101
#define LIB_ERR_LIB_VER_MISMATCH             102
#define LIB_ERR_WIN_VER_MISMATCH             103
#define LIB_ERR_FIND_IMPORT_1                104
#define LIB_ERR_FIND_IMPORT_2                105
#define LIB_ERR_WND_TASKBAR                  106
#define LIB_ERR_WND_TASKBAND                 107
#define LIB_ERR_WND_TASKSW                   108
#define LIB_ERR_WND_TRAYNOTIFY               109
#define LIB_ERR_WND_TASKLIST                 110
#define LIB_ERR_WND_THUMB                    111
#define LIB_ERR_WND_TRAYOVERFLOWTOOLBAR      112
#define LIB_ERR_WND_TRAYTEMPORARYTOOLBAR     113
#define LIB_ERR_WND_TRAYTOOLBAR              114
#define LIB_ERR_WND_TRAYCLOCK                115
#define LIB_ERR_WND_SHOWDESKTOP              116
#define LIB_ERR_WND_W7STARTBTN               117
#define LIB_ERR_MSG_DLL_INIT                 118
#define LIB_ERR_WAITTHREAD                   119

#define LIB_ERR_EXTHREAD_MINHOOK             201
#define LIB_ERR_EXTHREAD_MINHOOK_PRELOADED   202
#define LIB_ERR_EXTHREAD_MOUSECTRL           203
#define LIB_ERR_EXTHREAD_KEYBDHOTKEYS        204
#define LIB_ERR_EXTHREAD_APPIDLISTS          205
#define LIB_ERR_EXTHREAD_COMFUNCHOOK         206
#define LIB_ERR_EXTHREAD_DPAHOOK             207
#define LIB_ERR_EXTHREAD_REFRESHTASKBAR      208
#define LIB_ERR_EXTHREAD_MINHOOK_APPLY       209

#define EXE_ERR_VIRTUAL_ALLOC                1001
#define EXE_ERR_WRITE_PROC_MEM               1002
#define EXE_ERR_CREATE_REMOTE_THREAD         1003
#define EXE_ERR_READ_PROC_MEM                1004
