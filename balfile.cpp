#include <windows.h>
#include <math.h>
#include <stdio.h>
#include "errcodes.h"
#include "balfile.hpp"
#include "wingraph.h"
#include "common.h"
#include <string.h>
#include "balrec.hpp"
#include "ledrec.hpp"
#include "dayrec.hpp"
#include "fileman.hpp"
#include "graphab.hpp"

BalanceBookFile::BalanceBookFile()
{
        entryHead = NULL;
        strcpy(titleFile, "balance.ttl");
        strcpy(defaultTitle, "Balance Sheet");
        /* check totals is responsible for loading
        the entryHead list with proper data. */
        checkTotals();
}
BalanceBookFile::~BalanceBookFile()
{
        /* free the entire linked list of entries.. */
        freeAll();
}
BalanceBookFile::freeAll()
{
        BalanceEntry *ptr=entryHead, *freeAddr;
        while (ptr)
        {
                /* There is no harm in using just
                one variable, but anyway...*/
                freeAddr = ptr;
                ptr = ptr->next;
                free(freeAddr);
        }
        entryHead = NULL;
        return OK;
}
Int BalanceBookFile::getBlankRecord(RecordClass **record)
{
        BalanceEntry *entry;
        getBlankEntry(&entry);
        *record = new BalanceRecord(this, entry);
        return OK;
}
Int BalanceBookFile::getBlankEntry(BalanceEntry **entry)
{
        BalanceEntry *ptr = entryHead;
        while (ptr->next)
        {
                ptr=ptr->next;
        }
        *entry = (BalanceEntry *)malloc(sizeof(BalanceEntry));
        memset(*entry, 0, sizeof(BalanceEntry));
        (*entry)->entryIndex = NEW_ENTRY;
        (*entry)->prev = ptr;
        ptr->next = *entry;
        return OK;
}

Int BalanceBookFile::getTotCredit(double& totCredit)
{
        BalanceEntry *ptr = entryHead;
        totCredit = 0.0;
        while (ptr)
        {
                totCredit += ptr->credit;
                ptr = ptr->next;
        }
        return OK;
}

Int BalanceBookFile::getTotDebit(double& totDebit)
{
        BalanceEntry *ptr = entryHead;
        totDebit = 0;
        while (ptr)
        {
                totDebit += ptr->debit;
                ptr = ptr->next;
        }
        return OK;

}

Int BalanceBookFile::getFirst(RecordClass **record)
{
        *record = (RecordClass *)(new BalanceRecord(this, entryHead));
	return OK;
}
Int BalanceBookFile::getNext(RecordClass **record)
{
        BalanceEntry far *temp, *entry;
        Int recType;
        double netTotal, totCredit, totDebit;
	/* Get the record type */
        ((BalanceRecord *)*record)->getEntry(&entry);

	(*record)->getRecordType(recType);
	/* Get the current date from string. */
        temp = entry->next;
        if ((recType ==PHYS_REC)
                && !temp && (entry->entryIndex != ANAL_REC))
	{
        /* Current=Physical, existing.
         * Next = NULL => LAST ENTRY
         */
#ifdef DEBUGPRINT
                printf("MSG:10: BALRED: Inserting ANAL_REC\n");
#endif
		/* Return a ANAL_REC*/
                (*record) = new BalanceRecord(this, entry);
                getTotCredit(totCredit);
                (*record)->setCalculatedCredit(totCredit);
                getTotDebit(totDebit);
                (*record)->setCalculatedDebit(totDebit);
                (*record)->setRecordType(ANAL_REC);
	}
        else
	{
#ifdef DEBUGPRINT
                printf("MSG:89:BALANCEBOOK: PHYS_REC\n");
#endif
                entry = temp;
                if (entry == NULL)
                {
                        getBlankEntry(&entry);
                }
                (*record) = new BalanceRecord(this, entry);
                (*record)->setRecordType(PHYS_REC);
	}
        if (entry == NULL)
        {
                delete (*record);
                *record = NULL;
        }
#ifdef DEBUGPRINT
        if (*record)
                (*record)->getRecordType(recType);
        else printf("MSG:26:record invalid\n");
        printf("MSG:11:Done getting Next%d\n", recType);
#endif
        return OK;
}

Int BalanceBookFile::getPrev(RecordClass **record)
{
        BalanceEntry far *temp, *entry;
        Int recType;
        double netTotal, totCredit, totDebit;
	/* Get the record type */
        ((BalanceRecord *)*record)->getEntry(&entry);

	(*record)->getRecordType(recType);
	/* Get the current date from string. */
        temp = entry->prev;
#ifdef DEBUGLEVEL2
        printf("MSG:10: previous entry check\n");
#endif

        if (temp == NULL)
        {
                return NOTHING_TO_SEARCH;
        }
#ifdef DEBUGLEVEL2
        printf("MSG:10: previous entry exists\n");
#endif

        if ((temp->entryIndex == NEW_ENTRY) &&
                (entry->entryIndex == EXISTING_ENTRY))
	{
        /* Current=Physical, existing.
         * Next = NULL => LAST ENTRY
         */
#ifdef DEBUGLEVEL2
                printf("MSG:10: Inserting ANAL_REC\n");
#endif
		/* Return a ANAL_REC*/
                (*record) = new BalanceRecord(this, temp);
                getTotCredit(totCredit);
                (*record)->setCalculatedCredit(totCredit);
                getTotDebit(totDebit);
                (*record)->setCalculatedDebit(totDebit);
                (*record)->setRecordType(ANAL_REC);
	}
        else
	{
                entry = temp;
                (*record) = new BalanceRecord(this, entry);
                (*record)->setRecordType(PHYS_REC);
	}
        if (entry == NULL)
        {
                delete (*record);
                *record = NULL;
                return NOTFOUND;
        }
#ifdef DEBUGLEVEL2
        if (*record)
                (*record)->getRecordType(recType);
        else printf("MSG:26:record invalid\n");
        printf("MSG:11:Done getting Next%d\n", recType);
#endif
        return OK;
}

Int BalanceBookFile::fileSearch()
{
	return OK;
}

Int BalanceBookFile::isScreenReadOnly(Int& readOnly)
{
	readOnly = TRUE;
	return OK;
}
Int BalanceBookFile::print()
{
        FILE *fp;
        BalanceEntry *ptr = entryHead;
        LedgerHeadEntry *entry;
        DayBookEntry *dentry;
        Int pageOrderCode;
        char title[MAX_TITLE_LEN];
        char filename[FILENAME_LEN];
        Int decimalPart, len, day, month, year;
        char crout[6], dbout[6];
        double totCredit=0.0, totDebit=0.0;
        
        strcpy(filename, "BAL.TXT");
        fp = fopen(filename, "w");

        getFirstEntry(&dentry);
        getDMY(dentry->date, day, month, year);

        if (!fp)
        {
                messageBox("Unable to open print file");
                return FOPENERR;
        }
#ifdef DEBUGPRINT
        printf("File opened\n", ptr->credit, ptr->debit);
                
#endif

        getTitle(title);
        fprintf(fp, "อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ\n");
        fprintf(fp, "                 %s 20%02d - 20%02d\n", title, year, year+1);
        fprintf(fp, "อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ\n");
        while(ptr)
        {
                char tempchar;
                if (ptr->entryIndex == NEW_ENTRY)
                {
                        ptr = ptr->next;
                        continue;
                }
                totCredit += ptr->credit;
                totDebit += ptr->debit;

                len = strlen(ptr->itemDesc);
                getHeadListEntry(ptr->head, &entry);
                if (entry) pageOrderCode = entry->pageOrderCode;
                else pageOrderCode = 0;
                if (len>40)
                {
                        tempchar = ptr->itemDesc[40];
                        ptr->itemDesc[40] = 0;
                }
                if ((pageOrderCode != 0) )
                    fprintf(fp, "%-8sณ%-2dณ%-40sณ", ptr->date,
                            pageOrderCode, ptr->itemDesc);
                else
                    fprintf(fp, "%-8sณ%-2sณ%-40sณ", ptr->date,
                            " ", ptr->itemDesc);

                if (len>40)
                {
                        ptr->itemDesc[40] = tempchar;
                        fprintf(fp, "%9.0lfณ%c%cณ%9.0lfณ%c%cณ\n", 
                                0, '0','0', 0,'0','0');

                        if ((pageOrderCode !=0))
                            fprintf(fp, "%-8sณ%-2dณ%-40sณ", ptr->date,
                                     pageOrderCode, ptr->itemDesc+40);
                        else
                            fprintf(fp, "%-8sณ%-2sณ%-40sณ", ptr->date,
                                     " ", ptr->itemDesc+40);

                }

                sprintf(crout, "%-0.2lf", ptr->credit-(Int)ptr->credit);
                sprintf(dbout, "%-0.2lf", ptr->debit-(Int)ptr->debit);

                fprintf(fp, "%9.0lfณ%c%cณ%9.0lfณ%c%cณ\n", 
                         floor(ptr->credit), crout[2], crout[3], floor(ptr->debit), dbout[2], dbout[3]);
#ifdef DEBUGPRINT
                printf("credit = %lf debit = %lf totCredit=%lf, totDebit=%lf\n", ptr->credit, ptr->debit, totCredit, totDebit);                
#endif
                ptr = ptr->next;
        }
#ifdef DEBUGPRINT
        printf("tot %lf %lf\n", totCredit, totDebit);
#endif

        fprintf(fp, "ฤฤฤฤฤฤฤฤลฤฤลฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤลฤฤฤฤฤฤฤฤฤลฤฤลฤฤฤฤฤฤฤฤฤลฤฤล\n");
                fprintf(fp, "%-8sณ%-2sณ%-40sณ", " ",
                        " ", " ");
                sprintf(crout, "%-0.2lf", totCredit-floor(totCredit));
                sprintf(dbout, "%-0.2lf", totDebit-floor(totDebit));

                fprintf(fp, "%9.0lfณ%c%cณ%9.0lfณ%c%cณ\n", 
                         floor(totCredit), crout[2], crout[3], floor(totDebit), dbout[2], dbout[3]);
        fprintf(fp, "อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ\n");

        fclose(fp);
        system("prin.py BAL\n");
        messageBox("Print complete");
	return OK;
}

Int BalanceBookFile::addEntry(BalanceEntry *entry)
{
        BalanceEntry *balptr = entryHead;
        if (!entryHead)
        {
                entryHead = entry;
                return OK;
        }
        
        while(balptr->next) balptr = balptr->next;
        balptr->next = entry;
        entry->prev = balptr;

        return OK;        
}


Int BalanceBookFile::checkTotals()
{
        /* Get the right list on entryHead, rember to make
        an entry for the cash on hand. Calculate it separately
        to ensure, closeness to physical meaning of the entry. */
        BalanceEntry *newEntry;
        LedgerHeadEntry *hedptr;
        DayBookEntry *dbptr=NULL;
        double totCredit, totDebit, netTotal;
        double plTotDebit=0.0, plTotCredit=0.0;
        if (entryHead)
        {
                freeAll();
        }
        getFirstEntry(&hedptr);
        while(hedptr)
        {
                totCredit = 0.0;
                totDebit = 0.0;
                dbptr = NULL;
                findEntry(&dbptr, NULL, hedptr->headCode, POSITIVE);
                while (dbptr)
                {
                        totCredit  += dbptr->credit;
                        totDebit += dbptr->debit;
                        findEntry(&dbptr, NULL, hedptr->headCode, POSITIVE);
                }
                if (hedptr->ledgerType == PL_HEAD)
                {
#ifdef DEBUGTOTALS
                        printf("plTotC=%lf plTotD=%lf\n", plTotCredit, plTotDebit);
#endif
                        plTotCredit += totCredit;
                        plTotDebit += totDebit;
                }
                else if (hedptr->ledgerType == STATIC_HEAD)
                {
                        newEntry = (BalanceEntry *)malloc(sizeof(BalanceEntry));
                        memset(newEntry, 0, sizeof(BalanceEntry));
                        newEntry->entryIndex = EXISTING_ENTRY;
                        strcpy(newEntry->itemDesc, hedptr->ledgerName);
                        newEntry->page = hedptr->pageOrderCode;
                        newEntry->head = hedptr->headCode;
                        if (totCredit>=totDebit)
                        {
                                totCredit = totCredit - totDebit;
                                totDebit = 0.0;
                        }
                        else
                        {
                                totDebit = totDebit - totCredit;
                                totCredit = 0.0;
                        }
                        newEntry->credit = totCredit;
                        newEntry->debit = totDebit;
                        addEntry(newEntry);
                } else if (hedptr->ledgerType == GPL_HEAD)
                {
                    /* GPL_HEAD is the last entry */
                        newEntry = (BalanceEntry *)malloc(sizeof(BalanceEntry));
                        memset(newEntry, 0, sizeof(BalanceEntry));
                        newEntry->entryIndex = EXISTING_ENTRY;
                        strcpy(newEntry->itemDesc, hedptr->ledgerName);
                        newEntry->page = hedptr->pageOrderCode;
                        newEntry->head = hedptr->headCode;
                        plTotCredit += totCredit;
                        plTotDebit += totDebit;
                        if (plTotCredit>=plTotDebit)
                        {
                                plTotCredit = plTotCredit - plTotDebit;
                                plTotDebit = 0.0;
                        }
                        else
                        {
                                plTotDebit = plTotDebit - plTotCredit;
                                plTotCredit = 0.0;
                        }
                        newEntry->credit = plTotCredit;
                        newEntry->debit = plTotDebit;
                        addEntry(newEntry);

                }
                getNextEntry(&hedptr);
        }

        /* Add cash on hand entry */
        newEntry = (BalanceEntry *)malloc(sizeof(BalanceEntry));
        memset(newEntry, 0, sizeof(BalanceEntry));
        newEntry->entryIndex = EXISTING_ENTRY;
        newEntry->page = COH;
        newEntry->head = COH;
        strcpy(newEntry->itemDesc, "Cash on hand:");
        dbptr = NULL;
        getFirstEntry(&dbptr);
        totCredit = 0.0;
        totDebit = 0.0;
        while(dbptr)
        {
                totCredit += dbptr->credit;
                totDebit += dbptr->debit;
#ifdef DEBUGPRINT
                printf("\nMSG:83:getting next daybook entry for COH calc...\n");
#endif
                getNextEntry(&dbptr);
        }
        newEntry->credit = 0.0;
        newEntry->debit = totCredit - totDebit;
#ifdef DEBUGPRINT
        printf("\nMSG:84:Before addEntry\n");
#endif

        addEntry(newEntry);
        /* Balance sheet created END OF PROJECT AND START OF
        A NEW ERA.....!!!!!*/
	return OK;
}

Int BalanceBookFile::openNewYear()
{
        DayBookEntry dbEntry, *ptr;
        BalanceEntry *balEntry = entryHead;
        Int day, month, year, count=0;
        FILE *fp;
        char cmd[MAX_FIELD_LEN];
        char backfile[MAX_FIELD_LEN];
        memset(&dbEntry, 0, sizeof(DayBookEntry));
        getFirstEntry(&ptr);
        getDMY(ptr->date, day, month, year);

        sprintf((char *)&(dbEntry.date), "%02d-%02d-%02d", 01, 04, year+1);

        while (validateDate(dbEntry.date)!=OK)
        {
                inputBox("Enter the first date in financial year", (char *)&dbEntry.date);
                count++;
                if (count >= 3)
                {
                        messageBox("Unable to validate date.");
                        return OK;
                }
        }
        inputBox("File name for current year backup:-", backfile);
        sprintf(cmd, "copy accounts.db %s", backfile);
        system(cmd);
        /* Default double back up on file bak<year>.db */
        sprintf(cmd, "copy accounts.db bak%d.db", year);
        system(cmd);
        fp = fopen("accounts.db", "wb");
        if (!fp)
        {
                messageBox("Unable to open file.");
                return OK;
        }
        while (balEntry)
        {
                if (balEntry->entryIndex == NEW_ENTRY)
                {
                        /* First entry NULL. */
                        break;
                }
                if ((balEntry->page == COH) &&
                        (balEntry->head == COH))
                {
                        balEntry = balEntry->next;
                        continue;
                }

                dbEntry.isOpening = True;
                dbEntry.head = balEntry->head;
                strcpy(dbEntry.itemDesc, balEntry->itemDesc);
                dbEntry.credit = balEntry->credit;
                dbEntry.debit = balEntry->debit;
                
                fwrite(&dbEntry, 1, sizeof(DayBookEntry), fp);
                do
                {
                        balEntry = balEntry->next;
                } while (balEntry && (balEntry->entryIndex == NEW_ENTRY));
        }
        fclose(fp);
        messageBox("New Year Creation Complete.");
        return OK;
}

