#pragma once

extern int nWinVersion;
extern WORD nExplorerBuild, nExplorerQFE;

#define WIN_VERSION_UNSUPPORTED    (-1)
#define WIN_VERSION_7              0
#define WIN_VERSION_8              1
#define WIN_VERSION_81             2
#define WIN_VERSION_811            3
#define WIN_VERSION_10_T1          4
#define WIN_VERSION_10_T2          5
#define WIN_VERSION_10_R1          6
#define WIN_VERSION_10_R2          7
#define WIN_VERSION_10_R3          8
#define WIN_VERSION_10_R4          9
#define WIN_VERSION_10_R5          10
#define WIN_VERSION_10_19H1        11

// helper macros
#define FIRST_NONEMPTY_ARG_2(a, b) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : (b) )
#define FIRST_NONEMPTY_ARG_3(a, b, c) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : (c) ))
#define FIRST_NONEMPTY_ARG_4(a, b, c, d) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : (d) )))
#define FIRST_NONEMPTY_ARG_5(a, b, c, d, e) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : (e) ))))
#define FIRST_NONEMPTY_ARG_6(a, b, c, d, e, f) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : \
                                   ( (sizeof(#e) > sizeof("")) ? (e+0) : (f) )))))
#define FIRST_NONEMPTY_ARG_7(a, b, c, d, e, f, g) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : \
                                   ( (sizeof(#e) > sizeof("")) ? (e+0) : \
                                   ( (sizeof(#f) > sizeof("")) ? (f+0) : (g) ))))))
#define FIRST_NONEMPTY_ARG_8(a, b, c, d, e, f, g, h) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : \
                                   ( (sizeof(#e) > sizeof("")) ? (e+0) : \
                                   ( (sizeof(#f) > sizeof("")) ? (f+0) : \
                                   ( (sizeof(#g) > sizeof("")) ? (g+0) : (h) )))))))
#define FIRST_NONEMPTY_ARG_9(a, b, c, d, e, f, g, h, i) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : \
                                   ( (sizeof(#e) > sizeof("")) ? (e+0) : \
                                   ( (sizeof(#f) > sizeof("")) ? (f+0) : \
                                   ( (sizeof(#g) > sizeof("")) ? (g+0) : \
                                   ( (sizeof(#h) > sizeof("")) ? (h+0) : (i) ))))))))
#define FIRST_NONEMPTY_ARG_10(a, b, c, d, e, f, g, h, i, j) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : \
                                   ( (sizeof(#e) > sizeof("")) ? (e+0) : \
                                   ( (sizeof(#f) > sizeof("")) ? (f+0) : \
                                   ( (sizeof(#g) > sizeof("")) ? (g+0) : \
                                   ( (sizeof(#h) > sizeof("")) ? (h+0) : \
                                   ( (sizeof(#i) > sizeof("")) ? (i+0) : (j) )))))))))
#define FIRST_NONEMPTY_ARG_11(a, b, c, d, e, f, g, h, i, j, k) \
                                   ( (sizeof(#a) > sizeof("")) ? (a+0) : \
                                   ( (sizeof(#b) > sizeof("")) ? (b+0) : \
                                   ( (sizeof(#c) > sizeof("")) ? (c+0) : \
                                   ( (sizeof(#d) > sizeof("")) ? (d+0) : \
                                   ( (sizeof(#e) > sizeof("")) ? (e+0) : \
                                   ( (sizeof(#f) > sizeof("")) ? (f+0) : \
                                   ( (sizeof(#g) > sizeof("")) ? (g+0) : \
                                   ( (sizeof(#h) > sizeof("")) ? (h+0) : \
                                   ( (sizeof(#i) > sizeof("")) ? (i+0) : \
                                   ( (sizeof(#j) > sizeof("")) ? (j+0) : (k) ))))))))))

#define DO2(d7, dx)                ( (nWinVersion == WIN_VERSION_7) ? (d7) : (dx) )
#define DO3(d7, d8, dx)            ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : (dx) ))
#define DO4(d7, d8, d81, dx)       ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : (dx) )))
#define DO5(d7, d8, d81, d811, dx) ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : (dx) ))))
#define DO6(d7, d8, d81, d811, d10_t1, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : (dx) )))))
#define DO7(d7, d8, d81, d811, d10_t1, d10_t2, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T2) ? FIRST_NONEMPTY_ARG_6(d10_t2, d10_t1, d811, d81, d8, d7) : (dx) ))))))
#define DO8(d7, d8, d81, d811, d10_t1, d10_t2, d10_r1, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T2) ? FIRST_NONEMPTY_ARG_6(d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R1) ? FIRST_NONEMPTY_ARG_7(d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : (dx) )))))))
#define DO9(d7, d8, d81, d811, d10_t1, d10_t2, d10_r1, d10_r2, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T2) ? FIRST_NONEMPTY_ARG_6(d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R1) ? FIRST_NONEMPTY_ARG_7(d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R2) ? FIRST_NONEMPTY_ARG_8(d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : (dx) ))))))))
#define DO10(d7, d8, d81, d811, d10_t1, d10_t2, d10_r1, d10_r2, d10_r3, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T2) ? FIRST_NONEMPTY_ARG_6(d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R1) ? FIRST_NONEMPTY_ARG_7(d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R2) ? FIRST_NONEMPTY_ARG_8(d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R3) ? FIRST_NONEMPTY_ARG_9(d10_r3, d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : (dx) )))))))))
#define DO11(d7, d8, d81, d811, d10_t1, d10_t2, d10_r1, d10_r2, d10_r3, d10_r4, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T2) ? FIRST_NONEMPTY_ARG_6(d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R1) ? FIRST_NONEMPTY_ARG_7(d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R2) ? FIRST_NONEMPTY_ARG_8(d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R3) ? FIRST_NONEMPTY_ARG_9(d10_r3, d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R4) ? FIRST_NONEMPTY_ARG_10(d10_r4, d10_r3, d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : (dx) ))))))))))
#define DO12(d7, d8, d81, d811, d10_t1, d10_t2, d10_r1, d10_r2, d10_r3, d10_r4, d10_r5, dx) \
                                   ( (nWinVersion == WIN_VERSION_7) ? (d7) : \
                                   ( (nWinVersion == WIN_VERSION_8) ? FIRST_NONEMPTY_ARG_2(d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_81) ? FIRST_NONEMPTY_ARG_3(d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_811) ? FIRST_NONEMPTY_ARG_4(d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T1) ? FIRST_NONEMPTY_ARG_5(d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_T2) ? FIRST_NONEMPTY_ARG_6(d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R1) ? FIRST_NONEMPTY_ARG_7(d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R2) ? FIRST_NONEMPTY_ARG_8(d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R3) ? FIRST_NONEMPTY_ARG_9(d10_r3, d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R4) ? FIRST_NONEMPTY_ARG_10(d10_r4, d10_r3, d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : \
                                   ( (nWinVersion == WIN_VERSION_10_R5) ? FIRST_NONEMPTY_ARG_11(d10_r5, d10_r4, d10_r3, d10_r2, d10_r1, d10_t2, d10_t1, d811, d81, d8, d7) : (dx) )))))))))))

#ifdef _WIN64
#define DEF3264(d32, d64)          (d64)
#else
#define DEF3264(d32, d64)          (d32)
#endif

#define DO2_3264(d7_32, d7_64, dx_32, dx_64) \
                                   DEF3264(DO2(d7_32, dx_32), DO2(d7_64, dx_64))

#define DO3_3264(d7_32, d7_64, d8_32, d8_64, dx_32, dx_64) \
                                   DEF3264(DO3(d7_32, d8_32, dx_32), DO3(d7_64, d8_64, dx_64))

#define DO4_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, dx_32, dx_64) \
                                   DEF3264(DO4(d7_32, d8_32, d81_32, dx_32), DO4(d7_64, d8_64, d81_64, dx_64))

#define DO5_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, dx_32, dx_64) \
                                   DEF3264(DO5(d7_32, d8_32, d81_32, d811_32, dx_32), DO5(d7_64, d8_64, d81_64, d811_64, dx_64))

#define DO6_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, dx_32, dx_64) \
                                   DEF3264(DO6(d7_32, d8_32, d81_32, d811_32, d10_t1_32, dx_32), DO6(d7_64, d8_64, d81_64, d811_64, d10_t1_64, dx_64))

#define DO7_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, d10_t2_32, d10_t2_64, dx_32, dx_64) \
                                   DEF3264(DO7(d7_32, d8_32, d81_32, d811_32, d10_t1_32, d10_t2_32, dx_32), DO7(d7_64, d8_64, d81_64, d811_64, d10_t1_64, d10_t2_64, dx_64))

#define DO8_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, d10_t2_32, d10_t2_64, d10_r1_32, d10_r1_64, dx_32, dx_64) \
                                   DEF3264(DO8(d7_32, d8_32, d81_32, d811_32, d10_t1_32, d10_t2_32, d10_r1_32, dx_32), DO8(d7_64, d8_64, d81_64, d811_64, d10_t1_64, d10_t2_64, d10_r1_64, dx_64))

#define DO9_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, d10_t2_32, d10_t2_64, d10_r1_32, d10_r1_64, d10_r2_32, d10_r2_64, dx_32, dx_64) \
                                   DEF3264(DO9(d7_32, d8_32, d81_32, d811_32, d10_t1_32, d10_t2_32, d10_r1_32, d10_r2_32, dx_32), DO9(d7_64, d8_64, d81_64, d811_64, d10_t1_64, d10_t2_64, d10_r1_64, d10_r2_64, dx_64))

#define DO10_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, d10_t2_32, d10_t2_64, d10_r1_32, d10_r1_64, d10_r2_32, d10_r2_64, d10_r3_32, d10_r3_64, dx_32, dx_64) \
                                   DEF3264(DO10(d7_32, d8_32, d81_32, d811_32, d10_t1_32, d10_t2_32, d10_r1_32, d10_r2_32, d10_r3_32, dx_32), DO10(d7_64, d8_64, d81_64, d811_64, d10_t1_64, d10_t2_64, d10_r1_64, d10_r2_64, d10_r3_64, dx_64))

#define DO11_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, d10_t2_32, d10_t2_64, d10_r1_32, d10_r1_64, d10_r2_32, d10_r2_64, d10_r3_32, d10_r3_64, d10_r4_32, d10_r4_64, dx_32, dx_64) \
                                   DEF3264(DO11(d7_32, d8_32, d81_32, d811_32, d10_t1_32, d10_t2_32, d10_r1_32, d10_r2_32, d10_r3_32, d10_r4_32, dx_32), DO11(d7_64, d8_64, d81_64, d811_64, d10_t1_64, d10_t2_64, d10_r1_64, d10_r2_64, d10_r3_64, d10_r4_64, dx_64))

#define DO12_3264(d7_32, d7_64, d8_32, d8_64, d81_32, d81_64, d811_32, d811_64, d10_t1_32, d10_t1_64, d10_t2_32, d10_t2_64, d10_r1_32, d10_r1_64, d10_r2_32, d10_r2_64, d10_r3_32, d10_r3_64, d10_r4_32, d10_r4_64, d10_r5_32, d10_r5_64, dx_32, dx_64) \
                                   DEF3264(DO12(d7_32, d8_32, d81_32, d811_32, d10_t1_32, d10_t2_32, d10_r1_32, d10_r2_32, d10_r3_32, d10_r4_32, d10_r5_32, dx_32), DO12(d7_64, d8_64, d81_64, d811_64, d10_t1_64, d10_t2_64, d10_r1_64, d10_r2_64, d10_r3_64, d10_r4_64, d10_r5_64, dx_64))

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)       ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)       ((int)(short)HIWORD(lParam))
#endif

// This macros allow you to declare a __thiscall function in C
// The hack is done in x86 and is not required in x64
#ifdef _WIN64
#define THISCALL_C
#else
#define THISCALL_C                      __fastcall
#endif

#ifdef _WIN64
#define THISCALL_C_THIS_ARG(this_arg)   this_arg
#else
#define THISCALL_C_THIS_ARG(this_arg)   this_arg, LONG_PTR _edx_var
#endif

#ifdef _WIN64
#define THISCALL_C_THIS_TYPE(this_type) this_type
#else
#define THISCALL_C_THIS_TYPE(this_type) this_type, LONG_PTR
#endif

#ifdef _WIN64
#define THISCALL_C_THIS_VAL(this_val)   this_val
#else
#define THISCALL_C_THIS_VAL(this_val)   this_val, _edx_var
#endif

// Structs
typedef struct _move_button_in_group {
	LONG_PTR *button_group;
	int index_from;
	int index_to;
} MOVE_BUTTON_IN_GROUP;

typedef struct _secondary_task_list_get {
	int count;
	LONG_PTR *dpa_ptr;
} SECONDARY_TASK_LIST_GET;

typedef struct _animation_manager_item {
	LONG_PTR lpSecondaryTaskListLongPtr;
	LONG_PTR *pAnimationManager;
} ANIMATION_MANAGER_ITEM;

// Enums
enum
{
	TASKBAR_WINDOW_UNKNOWN,

	TASKBAR_WINDOW_THUMBNAIL,
	TASKBAR_WINDOW_TASKLIST,
	TASKBAR_WINDOW_TASKSW,
	TASKBAR_WINDOW_TASKBAND,
	TASKBAR_WINDOW_NOTIFY,
	TASKBAR_WINDOW_TASKBAR,

	TASKBAR_SECONDARY_THUMBNAIL,
	TASKBAR_SECONDARY_TASKLIST,
	TASKBAR_SECONDARY_TASKBAND,
	TASKBAR_SECONDARY_TASKBAR,
};

// General functions
VS_FIXEDFILEINFO *GetModuleVersionInfo(HMODULE hModule, UINT *puPtrLen);
void **FindImportPtr(HMODULE hFindInModule, char *pModuleName, char *pImportName);
void PatchPtr(void **ppAddress, void *pPtr);
void PatchMemory(void *pDest, void *pSrc, size_t nSize);
BOOL StringBeginsWith(WCHAR *pString, WCHAR *pBeginStr);
void StripAmpersand(WCHAR *pDst, WCHAR *pSrc);
WCHAR *LoadStrFromRsrc(UINT uStrId);
LRESULT SendMessageBlock(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT TweakerSendErrorMsg(WCHAR *pText);

// Undocumented WinAPI functions
HWND WINAPI HungWindowFromGhostWindow_(HWND hwndGhost);
HWND WINAPI GhostWindowFromHungWindow_(HWND hwndHung);
BOOL WINAPI MirrorIcon_(HICON *phIconSmall, HICON *phIconLarge);

// Window and task item functions
BOOL IsGhostWindowClass(HWND hWnd);
HWND TryGetWndFromGhostIfHung(HWND hWnd);
HWND TryGetGhostFromWndIfHung(HWND hWnd);
BOOL WndGetAppId(HWND hWnd, WCHAR pAppId[MAX_PATH]);
BOOL WndSetAppId(HWND hWnd, WCHAR *pAppId);
void PostWindowScroll(HWND hWnd, int nBar, int nPages);
BOOL IsWindows811ImmersiveTaskItem(LONG_PTR *task_item);
BOOL IsMinimizedTaskItem(LONG_PTR *task_item);
BOOL CanMinimizeTaskItem(LONG_PTR *task_item);
BOOL CanMinimizeWindow(HWND hWnd);
BOOL IsMaximizedTaskItem(LONG_PTR *task_item);
BOOL CanMaximizeTaskItem(LONG_PTR *task_item);
BOOL CanMaximizeWindow(HWND hWnd);
BOOL CanRestoreTaskItem(LONG_PTR *task_item);
BOOL CanRestoreWindow(HWND hWnd);
BOOL CanCloseTaskItem(LONG_PTR *task_item);
BOOL CanCloseWindow(HWND hWnd);
void SwitchToTaskItem(LONG_PTR *task_item);
void SwitchToWindow(HWND hWnd);
void MinimizeTaskItem(LONG_PTR *task_item);
void MinimizeThumbTaskItem(LONG_PTR *task_item);
void CloseTaskItem(LONG_PTR *task_item, BOOL bSwitchOnTimeout);
BOOL TerminateProcessOfTaskItem(LONG_PTR *task_item);

// Taskbar functions
BOOL IsAppIdARandomGroup(WCHAR *pAppId);
BOOL TaskbarMoveButtonInGroup(MOVE_BUTTON_IN_GROUP *p_move_button);
BOOL TaskbarMoveThumbInGroup(LONG_PTR lpMMThumbnailLongPtr, int index_from, int index_to);
BOOL TaskbarMoveTaskInGroup(LONG_PTR *task_group, LONG_PTR *task_item_from, LONG_PTR *task_item_to);
LONG_PTR *ButtonGroupFromTaskGroup(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *task_group);
BOOL TaskbarMoveGroup(LONG_PTR lpMMTaskListLongPtr, int index_from, int index_to);
LONG_PTR *TaskbarScroll(LONG_PTR lpMMTaskListLongPtr, int nRotates, BOOL bSkipMinimized, BOOL bWarpAround, LONG_PTR *src_task_item);
LONG_PTR *TaskbarGetTrackedButton(LONG_PTR lpMMTaskListLongPtr);
LONG_PTR *TaskbarGetTrackedTaskItem(LONG_PTR lpMMTaskListLongPtr);
LONG_PTR *ThumbnailGetTrackedTaskItem(LONG_PTR lpMMThumbnailLongPtr, LONG_PTR **p_container_task_item);
LONG_PTR *TaskbarGetTrackedButtonGroup(LONG_PTR lpMMTaskListLongPtr);
LONG_PTR *TaskbarGetActiveButtonGroup(LONG_PTR lpMMTaskListLongPtr);
LONG_PTR *TaskbarGetActiveButton(LONG_PTR lpMMTaskListLongPtr);
LONG_PTR *TaskbarGetActiveTaskItem(LONG_PTR lpMMTaskListLongPtr);
void SortButtonGroupItems(LONG_PTR *button_group);
BOOL ButtonGroupValidate(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);
void TaskListRecomputeLayout(LONG_PTR lpMMTaskListLongPtr);
void MMTaskListRecomputeLayout(void);
DWORD TaskbarGetPreference(LONG_PTR lpMMTaskListLongPtr);
void ShowLivePreview(LONG_PTR lpMMThumbnailLongPtr, HWND hWnd);
void TaskbarToggleAutoHide(void);
int GetSecondaryTaskListCount(void);
LONG_PTR SecondaryTaskListGetFirstLongPtr(SECONDARY_TASK_LIST_GET *p_secondary_task_list_get);
LONG_PTR SecondaryTaskListGetNextLongPtr(SECONDARY_TASK_LIST_GET *p_secondary_task_list_get);
BOOL WillExtendedUIGlom(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);
LONG_PTR MMTaskListLongPtrFromMonitor(HMONITOR hTargetMonitor);
HWND GetTaskItemWnd(LONG_PTR *task_item);
HWND GetButtonWnd(LONG_PTR *button);
void OpenThumbnailPreview(LONG_PTR lpMMTaskListLongPtr);
void CreateNewInstance(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);
void DismissHoverUI(LONG_PTR lpMMTaskListLongPtr, BOOL bHideImmediately);
int GetTaskbarMinWidth(void);
int GetTaskbarMinHeight(void);
void DisableTaskbarTopmost(BOOL bDisable);
void EnableTaskbarBlurBehindWindow(BOOL bEnable);
void ButtonGroupExecMenuCommand(LONG_PTR *button_group, WPARAM wCommand);
BOOL IsMMTaskListLongPtr(LONG_PTR lp);
BOOL IsTaskbarWindow(HWND hWnd);
int IdentifyTaskbarWindow(HWND hWnd);
void DisableTaskbarsAnimation(LONG_PTR **ppMainTaskListAnimationManager, ANIMATION_MANAGER_ITEM **plpSeconadryTaskListAnimationManagers);
void RestoreTaskbarsAnimation(LONG_PTR *pMainTaskListAnimationManager, ANIMATION_MANAGER_ITEM *lpSeconadryTaskListAnimationManagers);
void Win10ShowStartMenu(LONG_PTR lpMMTaskbarLongPtr);
void Win10ShowWinXPowerMenu(LONG_PTR lpMMTaskbarLongPtr);
