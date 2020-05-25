#ifndef __ERRORCODES__H__
#define __ERRORCODES__H__

#define OK 1
#define INTERFACE_BASE			(0x100)
#define INTERFACE_START_FAIL	(INTERFACE_BASE + 0x1)


#define RECORD_BASE                     (0x200)
#define FIELD_OVERFLOW			(RECORD_BASE + 0x1)
#define FIELD_INVALID			(RECORD_BASE + 0x2)
#define REC_READONLY                    (RECORD_BASE + 0x3)
#define REC_PROTECTED			(RECORD_BASE + 0x4)
#define RELOAD_RECORDS                  (RECORD_BASE + 0x5)

#define FILE_BASE					(0x300)
#define FOPENERR					(FILE_BASE + 0x1)

#define MEMBASE					(0x400)
#define NOMEM						(MEMBASE + 0x1)
#define MEMINVALID				(MEMBASE + 0x2)

#define SEARCHBASE				(0x500)
#define NOTFOUND					(SEARCHBASE + 0x1)
#define NOTHING_TO_SEARCH			(SEARCHBASE + 0x2)
#define NEXTNOFIRST				(SEARCHBASE + 0x3)
#define PREVNOFIRST				(SEARCHBASE + 0x4)
#define SEARCH_COMPLETE			(SEARCHBASE + 0x5)

#define DATE_BASE                       (0x600)
#define DATE_INVALID                    (DATE_BASE+0x1)

#define BAL_BASE                        (0x700)
#define BALSHEET_ALREADY_MADE           (BAL_BASE+0x1)

#define SHEET_BASE                      (0x800)
#define READONLY                        (SHEET_BASE + 0x1)

#endif

