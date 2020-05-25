#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <mem.h>
#include "fileman.hpp"
#include "common.h"
#include "errcodes.h"

#define DATEINDEX(day, month, year) day+month*31+year*372

DayBookEntry far *dbMemHead = NULL;
LedgerHeadEntry far *hedMemHead = NULL;

DayBookEntry far *dbEntryAddr = NULL;
LedgerHeadEntry far *hedEntryAddr = NULL;

DayBookEntry far *dbFirstNewEntry = NULL;
LedgerHeadEntry far *hedFirstNewEntry = NULL;

Int datecmp(char *date1, char *date2)
{
        Int i;
        for (i=0;i<8; i++)        
                if (date1[i]!=date2[i]) return 1;
        return 0;
        
}
void getDMY(char far *date, Int& day, Int& month, Int& year)
{
        /* DO NOT USE VALIDATEDATE, since VALIDATEDATE USES THIS
        PROCEDURE. */
        if ((!date) || (!date[0]))
	{
                day=50; month=50; year=500;
                return;
	}
        /* subtract 1, Since day and month internally starts
         * from 0-30 and 0-11 resp.
         */
        day = (date[0]-'0')*10 + date[1]-'0'-1;
        month = (date[3]-'0')*10 + date[4]-'0'-1;
        year = (date[6]-'0')*10 + date[7]-'0';
	return;
}

double getDayBookTotal(Int headCode)
{
    DayBookEntry *dbptr;
    double totCredit = 0.0;
    double totDebit = 0.0;
    dbptr = NULL;
    findEntry(&dbptr, NULL, headCode, POSITIVE);
    while (dbptr)
    {
        totCredit  += dbptr->credit;
        totDebit += dbptr->debit;
        findEntry(&dbptr, NULL, headCode, POSITIVE);
    }
    return (totCredit - totDebit);
}

Int fileManInitialize()
{
	Int32 sizedb, sizehed;
	Int32 pagesdb, pageshed;
        Uint loop;
        DayBookEntry defDbEntry, *dbPtr;
        LedgerHeadEntry defHedEntry, *hedPtr;
	Int ret;
#ifdef PUREDOS
	unsigned segAddr=0;
#endif
        FILE *fpdb = fopen("accounts.db", "ab+");
        FILE *fphed = fopen("accounts.hed", "ab+");
        memset(&defDbEntry, 0, sizeof(defDbEntry));
        defDbEntry.date[0]=0;
        memset(&defHedEntry, 0, sizeof(defHedEntry));
	fseek(fpdb, 0, SEEK_END);
	sizedb = ftell(fpdb);
	fseek(fphed, 0, SEEK_END);
	sizehed = ftell(fphed);

        fseek(fpdb,0, SEEK_SET);
        fseek(fphed,0, SEEK_SET);

        if (sizedb)
        {
#ifdef PUREDOS
                pagesdb = sizedb/PARA_SIZE + 1;
                pageshed = sizehed/PARA_SIZE + 1;

                ret = allocmem((Int)pagesdb, (unsigned *)&segAddr);
                if (ret != -1)
                {
                        fclose(fpdb);
                        fclose(fphed);
                        return NOMEM;
                }
                dbMemHead = (DayBookEntry far *)segAddr;
#else
                dbMemHead = (DayBookEntry *)malloc(sizedb);
                dbEntryAddr = dbMemHead;
#ifdef DEBUGPRINT
                char *TESTMEM = (char *)malloc(2560000);
                if (!TESTMEM)
                        printf("MSG:101:Unable to alloc 2.56 MB\n");
                else
                {
                        printf("MSG:101:AMPLE MEMORY AVAILABLE.\n");
                        free(TESTMEM);
                }
#endif
                if (!dbMemHead)
                {
#ifdef DEBUGPRINT
                        printf("NOMEM\n");
#endif
                        fclose(fpdb);
                        fclose(fphed);
                        return NOMEM;
        
                }
#endif
                /* read the records now, allocation done. */
                fread(dbMemHead, 1, sizedb, fpdb);
                dbMemHead->prev=0;
                for (loop=0; loop<sizedb/sizeof(DayBookEntry); loop++)
                {
                        dbPtr = dbMemHead+loop;
                        if (loop) dbPtr->prev = (dbPtr-1);
                        dbPtr->next = (dbPtr+1);
                        dbPtr->entryIndex = EXISTING_ENTRY;
                }
                loop--;
                dbPtr = dbMemHead+loop;
                dbPtr->next = 0;        
        } /* End of if(sizedb) */

        if (sizehed)
        {
#ifdef PUREDOS
                ret = allocmem((Int)pageshed, (unsigned *)&segAddr);
                if (ret != -1)
                {
                        freemem((unsigned)dbMemHead);
                        fclose(fpdb);
                        fclose(fphed);
                        return NOMEM;
                }
                hedMemHead = (LedgerHeadEntry far *)segAddr;
#else
                hedMemHead = (LedgerHeadEntry far *)malloc(sizehed);
                hedEntryAddr = hedMemHead;
#ifdef DEBUGPRINT
                printf("headMemPtr at init=%x", hedMemHead);
#endif
                if (!hedMemHead)
                {
#ifdef DEBUGPRINT
                        printf("NOMEM\n");
#endif
                        free(dbMemHead);
                        fclose(fpdb);
                        fclose(fphed);
                        return NOMEM;
                }
#endif
#ifdef DEBUGPRINT
                printf("\nreading from file daybook");
#endif
                if (sizehed) fread(hedMemHead, 1, sizehed, fphed);
#ifdef DEBUGPRINT
                printf("MSG:15: sizehed=%d", sizehed);
#endif
                if (!sizehed) memset(hedMemHead, 0, sizeof(LedgerHeadEntry));
                else hedMemHead->prev = 0;
                for (loop=0; loop<sizehed/sizeof(LedgerHeadEntry); loop++)
                {
                        hedPtr = hedMemHead+loop;
                        if (loop) hedPtr->prev = (hedPtr-1);
                        hedPtr->next = (hedPtr+1);
                }
                loop--;
                hedPtr = hedMemHead+loop;
                hedPtr->next = 0;

        }        
	fclose(fpdb);
	fclose(fphed);
	return OK;
}
Int fileManCleanUp()
{
	unsigned freeAddr;
        DayBookEntry far *dbptr = dbFirstNewEntry;
        LedgerHeadEntry far *hedptr = hedFirstNewEntry;
        if (dbEntryAddr)
                freemem((unsigned)dbEntryAddr);
        if (hedEntryAddr)
                freemem((unsigned)hedEntryAddr);
	while (dbptr)
	{
		freeAddr = (unsigned)dbptr;
		dbptr = dbptr->next;
		freemem(freeAddr);
	};
	while (hedptr)
	{
		freeAddr = (unsigned)hedptr;
		hedptr = hedptr->next;
		freemem(freeAddr);
	}
	/* Everything done cleanly */
	return OK;
}
Int getFirstEntry(DayBookEntry far **entry)
{
        *entry = dbMemHead;

	return OK;
}
Int getFirstEntry(LedgerHeadEntry far **entry)
{
        *entry = hedMemHead;
	return OK;
}
Int getNextEntry(DayBookEntry far **entry)
{
#ifdef DEBUGPRINT
        printf("getNextEntry started\n");
#endif
        if (!*entry) return OK;
	*entry = (*entry)->next;
#ifdef DEBUGPRINT
        printf("MSG:9:getNextEntry PTR was OK\n");
#endif

	return OK;
}
Int getNextEntry(LedgerHeadEntry far **entry)
{
        if (!*entry) return OK;
	*entry = (*entry)->next;
	return OK;
}
Int getPrevEntry(DayBookEntry far **entry)
{
        if (!*entry) return OK;
	*entry = (*entry)->prev;
	return OK;
}
Int getPrevEntry(LedgerHeadEntry far **entry)
{
        if (!*entry) return OK;
	*entry = (*entry)->prev;
	return OK;
}
Int deleteEntry(DayBookEntry far *entry)
{
#ifdef DEBUGPRINT
        printf("delting entry = %x", entry);
#endif
        if (!entry) return OK;
        if (entry->prev)
        {
                entry->prev->next = entry->next;
        }
        else
        {
                /* Head is getting deleted. */
                dbMemHead = entry->next;
        }
        if (entry->next)
                entry->next->prev = entry->prev;
#ifdef DEBUGPRINT
        printf("delting done = %x", entry);
#endif

	return OK;
}

Int deleteEntry(LedgerHeadEntry far *entry)
{
#ifdef DEBUGPRINT
        if (!hedMemHead)
                printf("MSG:81: DELETED ALL\n");
#endif
        if (!entry) return OK;
        if (entry->prev)
        {
                entry->prev->next = entry->next;
        }
        else
        {
                hedMemHead = entry->next;
#ifdef DEBUGPRINT
                if (!hedMemHead)
                        printf("MSG:79: DELETED ALL entry->ledgerName=%s\n", entry->ledgerName);
#endif

        }
        if (entry->next)
                entry->next->prev = entry->prev;
	return OK;
}

Int insertEntry(DayBookEntry far *entry)
{
	DayBookEntry *ptr = dbMemHead;
	/* Let us search an entry which just greater than the
	to be inserted entry and then insert the new entry on the
	location previous to such an entry. */
	/* Hash tables might help, but this implementation, will not
	implement such a mechanism. */
	Int day, month, year, ledger;
        Int inDay, inMonth, inYear, inDateIndex, dateIndex;
        if (!ptr)
        {
                dbMemHead = entry;
                entry->prev = NULL;
                entry->next = NULL;
                return OK;
        }
	getDMY(entry->date, inDay, inMonth, inYear);
        inDateIndex = DATEINDEX(inDay, inMonth, inYear);
        entry->prev = ptr->prev;
	while(ptr)
	{

		getDMY(ptr->date, day, month, year);
                dateIndex = DATEINDEX(day, month, year); 
                if (dateIndex > inDateIndex) break;

                /* entry->prev is required to be stored, since we
                may not know the value when ptr becomes NULL. */
                entry->prev = ptr;

		ptr = ptr->next;
        }
#ifdef DEBUGPRINT
        if (!ptr)
                printf("MSG:53:inserting at the end\n");
        else printf("MSG:53:inserting somewhereelse\n");
#endif
        if (ptr && !ptr->prev)
                dbMemHead = entry;
        entry->next = ptr;
        /* entry->prev is already set in while loop. */
        if (ptr) ptr->prev = entry;
        if (entry->prev) entry->prev->next = entry;
#ifdef DEBUGPRINT
        printf("End of insert entry\n");
#endif
	return OK;
}

Int insertEntry(LedgerHeadEntry far *entry)
{
	/* pageOrder is the field used to sort the entries. */



	/* Let us search an entry which just greater than the
	to be inserted entry and then insert the new entry on the
	location previous to such an entry. */

	/* Hash tables might help, but this implementation, will not
	implement such a mechanism. */

        LedgerHeadEntry *ptr = hedMemHead;
	Int inPageOrder = entry->pageOrderCode;
	Int pageOrder;
#ifdef DEBUGPRINT
        LedgerHeadEntry *temp = ptr;

        while (temp)
        {
                printf("MSG:81: HEAD name=%s prev=%x next=%x\n", temp->ledgerName, temp->prev, temp->next);
                temp = temp->next;
        }
#endif

        if (!ptr)
        {
#ifdef DEBUGPRINT
                printf("MSG:74:insert entry:FIRSTENTRY:fileman: %s\n", entry->ledgerName);
#endif

                hedMemHead = entry;
                entry->prev = NULL;
                entry->next = NULL;
                return OK;
        }
#ifdef DEBUGPRINT
        printf("MSG:14: insertion of ledger head entry.\n");
#endif
        entry->prev = ptr->prev;
	while(ptr)
	{
#ifdef DEBUGPRINT
                printf("MSG:74:insert entry:fileman: %s\n", entry->ledgerName);
#endif

		pageOrder = ptr->pageOrderCode;
                if (inPageOrder && (pageOrder > inPageOrder)) break;
		/* This is just a temporary store, incase ptr becomes NULL. */
                entry->prev = ptr;

		ptr = ptr->next;
	}
#ifdef DEBUGPRINT
        if (!ptr)
                printf("MSG:76:inserting at the end\n");
        else printf("MSG:76:inserting somewhereelse\n");
#endif
        if (ptr && !ptr->prev)
                hedMemHead = entry;
        entry->next = ptr;
        /* entry->prev is already set in while loop. */
        if (ptr) ptr->prev = entry;
        if (entry->prev) entry->prev->next = entry;
	return OK;
}
Int saveAllEntries()
{
	FILE *fpdb;
	FILE *fphed;
	DayBookEntry dbEntry;
	LedgerHeadEntry hedEntry;
	DayBookEntry far *dbPtr = dbMemHead;
	LedgerHeadEntry far *hedPtr = hedMemHead;
#ifdef DEBUGPRINT
        static saveAllcount = 0;
        printf("MSG:55:saveAll hit count=%d\n", saveAllcount++);
#endif
	/* Session backup just incase something goes wrong. */
	fpdb = fopen("accounts.db", "wb");
	fphed = fopen("accounts.hed", "wb");
	while(dbPtr)
	{
#ifdef DEBUGPRINT
                printf("dbPtr=%x\n", dbPtr);
#endif
		_fmemcpy((void far *)&dbEntry, dbPtr, sizeof(DayBookEntry));
                if (validateDate(dbEntry.date) == OK) fwrite(&dbEntry, sizeof(DayBookEntry), 1, fpdb);
#ifdef DEBUGPRINT
                else printf("MSG:85:validate date fail, so not saving entry, with entry name = %s\n", dbEntry.itemDesc);
#endif
		dbPtr = dbPtr->next;
	}
	while (hedPtr)
	{
#ifdef DEBUGPRINT
                printf("hedPtr=%x\n", hedPtr);
#endif

		_fmemcpy((DayBookEntry far *)&hedEntry, hedPtr, sizeof(LedgerHeadEntry));
                if (hedEntry.headCode) fwrite(&hedEntry, sizeof(LedgerHeadEntry), 1, fphed);
		hedPtr = hedPtr->next;
	}
#ifdef DEBUGPRINT
                printf("MSG:56:end of saveAll\n", dbPtr);
#endif

	/* Lets hope everything went fine. */
	fclose(fphed);
	fclose(fpdb);
	return OK;
}

/* If a date is provided it is searched, else if a head it provided, it is
searched, else NOTHING_TO_SEARCH is returned. date search and head search
are not joint. That is date search only happens if date is provided.
ledger search happens only if a date is not provided. */
Int findEntry(DayBookEntry far **entry, char *date, Int head, Int direction)
{
        Int day, month, year, ledger, dateIndex;
        Int inDay, inMonth, inYear, inLedger, inDateIndex;

        if (!head && !date)
        {
                *entry = NULL;
                return NOTHING_TO_SEARCH;
        }

	if (!*entry)
		*entry = dbMemHead;
	else
	{
		if (direction == POSITIVE)
			(*entry) = (*entry)->next;
		else
			(*entry) = (*entry)->prev;
	}
        getDMY(date, inDay, inMonth, inYear);
        inDateIndex = DATEINDEX(inDay, inMonth, inYear);
	inLedger = head;
#ifdef DEBUGPRINT
        printf("MSG:69: findEntry before while loop\n" );
#endif
	while(*entry)
	{
#ifdef DEBUGPRINT
                printf("MSG:52:while(entry)curentry head=%d date=%s, item=%s\n", (*entry)->head, (*entry)->date, (*entry)->itemDesc);
#endif

                getDMY((*entry)->date, day, month, year);
                dateIndex = DATEINDEX(day, month, year); 
		ledger = (*entry)->head;
                if (date && (dateIndex>=inDateIndex)) break;
                if (!date && inLedger && (ledger == inLedger)) break;
		/* This is just a temporary store, incase ptr becomes NULL. */
		if (direction==POSITIVE) (*entry) = (*entry)->next;
		else (*entry)=(*entry)->prev;
	}
	if ((*entry) == NULL)
	{
		return NOTFOUND;
	}
	return OK;
}
Int findEntry(LedgerHeadEntry far **entry, Int head)
{
	Int curHead;

        if (!head) return NOTHING_TO_SEARCH;

        *entry = hedMemHead;

	while(*entry)
	{
		curHead = (*entry)->headCode;
		if (curHead >= head) break;
		/* This is just a temporary store, incase ptr becomes NULL. */
		(*entry) = (*entry)->next;
	}
	if ((*entry) == NULL)
	{
		return NOTFOUND;
	}
	return OK;
}

Int allocNewEntry(DayBookEntry far **entry)
{
        Int size = sizeof(DayBookEntry);
#ifdef PUREDOS
	Int ret;
	unsigned segAddr;
        size=size/16+1;
	ret = allocmem((Int)size, (unsigned *)&segAddr);
	if (ret != -1)
	{
		return NOMEM;
	}
        *entry = (DayBookEntry far *)segAddr;
#else
        *entry = (DayBookEntry far *)malloc(size);
        if (!*entry)
        {
#ifdef DEBUGPRINT
                printf("MSG:80:NO Memory for alloc\n");
#endif
                return NOMEM;
        }
#endif
        
        _fmemset(*entry, 0, sizeof(DayBookEntry));
        /* isOpening is set to FALSE by the memset*/
        (*entry)->entryIndex = NEW_ENTRY;
        if (!dbFirstNewEntry) dbFirstNewEntry = (*entry);
	return OK;
}
Int allocNewEntry(LedgerHeadEntry far **entry)
{
	Int ret;
	unsigned segAddr;
	Int size = sizeof(LedgerHeadEntry);

#ifdef DEBUGPRINT
        printf("MSG:76:Done with allocation of LeadegerHeadEntry\n");
#endif

#ifdef PUREDOS
        size=size/16+1;
	ret = allocmem((Int)size, &segAddr);
	if (ret != -1)
	{
		return NOMEM;
	}
        *entry = (LedgerHeadEntry far *)segAddr;
#else
        *entry = (LedgerHeadEntry far *)malloc(size);
        if (!*entry)
        {
#ifdef DEBUGPRINT
                printf("MSG:80:NO Memory for alloc\n");
#endif
                return NOMEM;
        }

#endif
        _fmemset(*entry, 0, sizeof(LedgerHeadEntry));
        (*entry)->entryIndex = NEW_ENTRY;
#ifdef DEBUGPRINT
        printf("MSG:76:Done with allocation of LeadegerHeadEntry\n");
#endif
        if (!hedFirstNewEntry) hedFirstNewEntry = (*entry);
        return OK;
}

Int deallocEntry(DayBookEntry far *entry)
{
	unsigned addr;
	addr = (unsigned)entry;
	freemem(addr);
#ifdef DEBUGPRINT
        printf("MSG:48:Entry freeing success\n");
#endif
	return OK;
}

Int deallocEntry(LedgerHeadEntry far *entry)
{
	unsigned addr;
	addr = (unsigned)entry;
	freemem(addr);
#ifdef DEBUGPRINT
        printf("MSG:49:Entry freeing success\n");
#endif

        return OK;
}

Int validateDate(char *date)
{
        Int day, month, year;
        if (!date) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date NOT null pointer\n");
#endif
        if ((date[2] != '-') || (date[5] != '-')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date has - at pos 2 & 5\n");
#endif

        if ((date[0]<'0') || (date[0]>'9')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date 0th entry is number\n");
#endif
        if ((date[1]<'0') || (date[1]>'9')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date 1th entry is number\n");
#endif
        if ((date[3]<'0') || (date[3]>'9')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date 3th entry is number\n");
#endif
        if ((date[4]<'0') || (date[4]>'9')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date 4th entry is number\n");
#endif
        if ((date[6]<'0') || (date[6]>'9')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date 5th entry is number\n");
#endif
        if ((date[7]<'0') || (date[7]>'9')) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date 6th entry is number\n");
#endif

        getDMY(date, day, month, year);
        if (day>30 || day<0) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:day valid\n");
#endif

        if (month>11 || month<0) return DATE_INVALID;
#ifdef DEBUGPRINT
        printf("MSG:85:date VALID thanks.\n");
#endif

        /* Now we have a valid date on the hand. */
        return OK;
}

deleteReadd(DayBookEntry *entry)
{
        DayBookEntry *origEntry = entry, *temp;
        char *date = entry->date;
        if (!entry) return OK;
        do
        {
                temp = entry;
                entry = entry->next;

                deleteEntry(temp);
                insertEntry(temp);

        } while (entry && (entry!=origEntry) && (datecmp(entry->date, date)==0));
        return OK;
}

deleteReadd(LedgerHeadEntry *entry)
{
        LedgerHeadEntry *origEntry = entry, *temp;
        Int pageOrder = entry->pageOrderCode;
#ifdef DEBUGLINKEDLIST
        void DisplayLinkedList();
        printf("\nBEFORE DELETE-READD\n");
        DisplayLinkedList();
#endif
        if (!entry) return OK;
        do
        {
                temp = entry;
                entry = entry->next;
                deleteEntry(temp);
                insertEntry(temp);
        } while (entry && (entry!=origEntry) && (entry->pageOrderCode==pageOrder));
#ifdef DEBUGLINKEDLIST

        printf("\nAFTER DELETE-READD\n");
        DisplayLinkedList();
#endif

        return OK;
}
/* This is findEntry, but EXACT match. */
Int getHeadListEntry(Int head, LedgerHeadEntry **entry)
{
        (*entry) = hedMemHead;

        while (*entry)
        {
                if ((*entry)->headCode == head) break;
                *entry = (*entry)->next;
        }
#ifdef DEBUGVERB1
        printf("verbMSG:1:(*entry)=%x\n", (*entry));
#endif
        return OK;
}

#ifdef DEBUGLINKEDLIST
void DisplayLinkedList()
{
        Int count=0;
        DayBookEntry *entry = dbMemHead;        
        while (entry)
        {
                if (entry->itemDesc)
                        printf("%d.%s next=%x prev=%x cur=%x\n", count, entry->itemDesc,
                         entry->next, entry->prev, entry);

                entry = entry->next; count++;
        }
}
#endif
