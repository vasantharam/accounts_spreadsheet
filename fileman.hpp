#ifndef __FILEMAN__H__
#define __FILEMAN__H__
#include "common.h"

#define PARA_SIZE	16
#define DBFILE 1
#define HEDFILE 2

Int fileManInitialize();
Int fileManCleanUp();
void getDMY(char far *date, Int& day, Int& month, Int& year);
Int datecmp(char *date1, char * date2);
Int getFirstEntry(DayBookEntry far **entry);
Int getFirstEntry(LedgerHeadEntry far **entry);
Int getNextEntry(DayBookEntry far **entry);
Int getNextEntry(LedgerHeadEntry far **entry);
Int getPrevEntry(DayBookEntry far **entry);
Int getPrevEntry(LedgerHeadEntry far **entry);
Int deleteEntry(DayBookEntry far *entry);
Int deleteEntry(LedgerHeadEntry far *entry);
Int findEntry(DayBookEntry far **entry, char *date, Int head, Int direction);
Int findEntry(LedgerHeadEntry far **entry, Int head);
Int insertEntry(DayBookEntry far *entry);
Int insertEntry(LedgerHeadEntry far *entry);
Int allocNewEntry(DayBookEntry far **entry);
Int allocNewEntry(LedgerHeadEntry far **entry);
Int deallocEntry(DayBookEntry far *entry);
Int deallocEntry(LedgerHeadEntry far *entry);
Int deleteReadd(DayBookEntry far *entry);
Int deleteReadd(LedgerHeadEntry far *entry);
Int validateDate(char *date);
Int saveAllEntries();
Int getHeadListEntry(Int head, LedgerHeadEntry **entry);
double getDayBookTotal(Int headCode);
#endif

