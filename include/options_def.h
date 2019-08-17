#pragma once

#define OPT_RCLICK                     0
#define OPT_MCLICK                     1
#define OPT_DROP                       2
#define OPT_HOVER                      3
#define OPT_THUMB_REORDER			   4
#define OPT_THUMB_ACTIVEMIN			   5
#define OPT_PINNED_REMOVEGAP           6
#define OPT_PINNED_DBLCLICK            7
#define OPT_GROUPING                   8
#define OPT_GROUPING_OPENNEAR          9
#define OPT_GROUPING_NOPINNED          10
#define OPT_GROUPING_RIGHTDRAG         11
#define OPT_COMBINING                  12
#define OPT_COMBINING_DEACTIVE         13
#define OPT_COMBINING_DEONHOVER        14
#define OPT_COMBINING_DE_LABELS        15
#define OPT_COMBINED_LCLICK            16
#define OPT_WHEEL_CYCLE                17
#define OPT_WHEEL_CYCLE_SKIPMIN        18
#define OPT_WHEEL_MINTASKBAR           19
#define OPT_WHEEL_MINTHUMB             20
#define OPT_WHEEL_VOLTASKBAR           21
#define OPT_WHEEL_VOLNOTIFY            22
#define OPT_EMPTYDBLCLICK              23
#define OPT_EMPTYMCLICK                24
#define OPT_OTHER_NOSTARTBTN           25
#define OPT_OTHER_NOSHOWDESK           26
#define OPT_OTHER_CLOCKSHOWSEC         27
#define OPT_OTHER_EXTRAEMPTY           28

#define OPTS_COUNT                     29
#define OPTS_BUFF                      (OPTS_COUNT*sizeof(int))

extern UINT opts_configuration_values[OPTS_COUNT];
extern int opts_max_values[OPTS_COUNT];

typedef struct {
	int nOptIndex;
	int nOptValue;
} OPTS_STRUCT_RULES;

OPTS_STRUCT_RULES *opts_dependences_rules(int pOptions[OPTS_COUNT], int nOption);
