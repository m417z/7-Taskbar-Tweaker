#pragma once

#define OPT_EX_DRAG_TOWARDS_DESKTOP               0
#define OPT_EX_NOCHECK_MINIMIZE                   1
#define OPT_EX_NOCHECK_MAXIMIZE                   2
#define OPT_EX_NOCHECK_CLOSE                      3
#define OPT_EX_PINNED_UNGROUPED_ANIMATE_LAUNCH    4
#define OPT_EX_SNDVOL_TOOLTIP                     5
#define OPT_EX_TRAY_CLOCK_FIX_WIDTH               6
#define OPT_EX_FIX_HANG_REPOSITION                7
#define OPT_EX_W7_TASKLIST_HTCLIENT               8
#define OPT_EX_ALWAYS_SHOW_THUMB_LABELS           9
#define OPT_EX_SCROLL_REVERSE_CYCLE               10
#define OPT_EX_SCROLL_REVERSE_MINIMIZE            11
#define OPT_EX_MULTIPAGE_WHEEL_SCROLL             12
#define OPT_EX_SHOW_DESKTOP_BUTTON_SIZE           13
#define OPT_EX_TRAY_ICONS_PADDING                 14
#define OPT_EX_NO_WIDTH_LIMIT                     15
#define OPT_EX_W7_SHOW_DESKTOP_CLASSIC_CORNER     16
#define OPT_EX_LIST_REVERSE_ORDER                 17
#define OPT_EX_DISABLE_TOPMOST                    18
#define OPT_EX_MULTIROW_EQUAL_WIDTH               19
#define OPT_EX_SCROLL_MAXIMIZE_RESTORE            20
#define OPT_EX_ALWAYS_SHOW_TOOLTIP                21
#define OPT_EX_DISABLE_TASKBAR_TRANSPARENCY       22
#define OPT_EX_NO_START_BTN_SPACING               23
#define OPT_EX_RIGHT_DRAG_TOGGLE_LABELS           24
#define OPT_EX_SHOW_DESKTOP_ON_HOVER              25
#define OPT_EX_DISABLE_ITEMS_DRAG                 26
#define OPT_EX_DISABLE_TRAY_ICONS_DRAG            27
#define OPT_EX_W10_LARGE_ICONS                    28
#define OPT_EX_CYCLE_SAME_VIRTUAL_DESKTOP         29
#define OPT_EX_VIRTUAL_DESKTOP_ORDER_FIX          30
#define OPT_EX_SCROLL_NO_WRAP                     31
#define OPT_EX_SHOW_LABELS                        32
#define OPT_EX_SNDVOL_CLASSIC                     33

#define OPTS_EX_COUNT                             34
#define OPTS_EX_BUFF                              (OPTS_EX_COUNT*sizeof(int))

BOOL LoadOptionsEx(int pOptionsEx[OPTS_EX_COUNT]);
