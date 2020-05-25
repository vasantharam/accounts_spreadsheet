#ifndef __COMMON__H__
#define __COMMON__H__


#define INSERTOFF	0
#define INSERTON	1

/* Actual 64 + overflow bandwidth of 2*/
/* Definitions for record types. */
#define PHYS_REC 1
#define ANAL_REC 2
#define ANAL_REC2 3

#define PL_REC1 4
#define PL_REC2 5
#define PL_REC3 6
#define PL_REC4 7
#define PL_REC5 8
#define PL_REC6 9
#define PL_REC7 10

#define EXISTING_ENTRY 1
#define NEW_ENTRY 2

#define POSITIVE 1
#define NEGATIVE -1

#define MAX_TITLE_LEN 66
#define MAX_ROWS 26
#define FILENAME_LEN 64
#define MAX_FIELD_LEN 64
#define MAX_HEADS 120

#define TRUE 	1
#define FALSE 	0
#define True TRUE
#define False FALSE

#define Int 	int
#define Int32  long
#define Uint unsigned int

#define CAPS	0x40
#define NUML   0x20
#define SCRL	0x10
#define ALTK	0x08
#define CTRL	0x04
#define LSHIFT	0x02
#define RSHIFT	0x01

#define TABASCII	9
#define ESCASCII	27
#define BACKSPACEKEY    8
#define ENTER 13

#define F1KEY	59
#define F2KEY  60
#define F3KEY  61
#define F4KEY  62
#define F9KEY  67
#define TABKEY  9
#define LEFTKEY 	75
#define RIGHTKEY  77
#define UPKEY     72
#define DOWNKEY   80
#define PAGEDOWNKEY	81
#define PAGEUPKEY    73
#define HOMEKEY      71
#define ENDKEY       79
#define INSERTKEY    82
#define DELETEKEY    83
#define CTRL_INSERT  23
#define CTRL_F       33
#define CTRL_P       25
#define CTRL_S       31
#define CTRL_H       35
#define CTRL_C       49
#define CTRL_V       47

#define far
#ifndef PUREDOS
#define _fmemcpy memcpy
#define _fmemset memset
#define freemem(x) free((void *)x)
#endif
#ifdef gcc
#define far
#define allocmem(x,y) 1;
#define freemem(x) 1;
#define _fmemcpy(x,y,z)
#define _fmemset(x,y,z)
#endif
/* Structures */
/* Day book record */
typedef struct sDayBookEntry
{
        Int entryIndex; /* NEW_ENTRY or EXISTING_ENTRY. */
        Int isOpening;
        char date[9];
	Int head;
	char itemDesc[64];
        double credit;
        double debit;
	struct sDayBookEntry far *next;
	struct sDayBookEntry far *prev;
} DayBookEntry;

/* Ledger Head Names Record */
typedef struct sLedgerHeadEntry
{
	char ledgerName[20];
	Int headCode;
	Int pageOrderCode;
	Int ledgerType;
	Int entryIndex;
	struct sLedgerHeadEntry far *next;
	struct sLedgerHeadEntry far *prev;
} LedgerHeadEntry;

typedef struct sBalanceEntry
{
        char date[9];
	Int head;
        Int page;
	char itemDesc[64];
        Int entryIndex;
        double credit;
        double debit;
        struct sBalanceEntry far *next;
        struct sBalanceEntry far *prev;
} BalanceEntry;


int getDayBookTitle(char *title);
int getLedgerHeadTitle(int ledgerCode, char *title);

extern Int dayBookFieldSizes[];
extern Int HeadListFieldSizes[];
#define MAX_INBOX_FIELD_SIZE 100

#define STATIC_HEAD 1
#define PL_HEAD 2
#define COH 98
#define CPL_HEAD 4
#define GPL_HEAD 5

#define PLC_HEADCODE 99


#endif

