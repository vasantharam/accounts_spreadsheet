#include "errcodes.h"
#include "dayfile.hpp"
#include "dayrec.hpp"
#include "common.h"
#include "fileman.hpp"
#include "graphab.hpp"
#include <stdio.h>
#include <math.h>
#include <string.h>

DayBookFile::DayBookFile()
{
	/* Totals are not stored in physical file. They
	need to be recaluculated when some changes happen.*/
        strcpy(titleFile, "daybook.ttl");
        strcpy(defaultTitle, "Day Book");

#ifdef DEBUGPRINT
	printf("DayBookFile\n");
#endif
        memset(date, 0, 8);
	checkTotals();
}


DayBookFile::~DayBookFile()
{
}

Int DayBookFile::getBlankRecord(RecordClass **record)
{
        DayBookEntry *entry;
        allocNewEntry(&entry);
        insertEntry(entry);
        *record = new DayBookRecord(this, entry);
        return OK;
}

Int DayBookFile::getFirst(RecordClass **record)
{
	DayBookEntry *entry=NULL;
        DayBookRecord *dRec;
#ifdef DEBUGPRINT
        printf("MSG:7:getFirst record\n");
#endif
        if (validateDate(date) != OK)
	{
#ifdef DEBUGPRINT
                printf("MSG:108:Search string validity fail....%s\n", date);
#endif
		getFirstEntry(&entry);
	}
        else
        {
#ifdef DEBUGPRINT
                printf("MSG:108:Search string validity pass....%s\n", date);
#endif

                findEntry(&entry, date, 0, POSITIVE);
        }
        if (entry==NULL)
        {
                allocNewEntry(&entry);
                insertEntry(entry);
        }
        dRec = new DayBookRecord(this, entry);
        dRec->setRecordType(PHYS_REC);
        
	*record = (RecordClass *)dRec;
	return OK;
}
Int DayBookFile::getNext(RecordClass **record)
{
	DayBookEntry far *temp, *entry;
	Int day, month, year, dateIndex;
	Int recType, dateDiff;
        double netTotal, credit, debit;
	/* Get the record type */
	((DayBookRecord *)*record)->getEntry(&entry);
	temp = entry;
#ifdef DEBUGPRINT
        static int getNextHITCOUNTER=0;
        printf("MSG:25:getNext hit count=%d\n", getNextHITCOUNTER++);
        extern DayBookEntry dbMemHead;
        printf("MSG:24:entry=%x dbMemHead=%x\n",entry, dbMemHead);
#endif

	(*record)->getRecordType(recType);
	/* Get the current date from string. */
	getDMY(entry->date, day, month, year);
        dateIndex = month*31 + day;
#ifdef DEBUGPRINT
        printf("MSG:21:dateIndex:%d\n", dateIndex);
#endif
	/* Get the next record. */
	getNextEntry(&temp);
        if (temp == NULL)
        {
                /* */
#ifdef DEBUGPRINT
                printf("MSG:22:nextEntry NULL, should be last entry.\n");
#endif
                if (validateDate(entry->date) == OK) dateDiff = 1;
                else dateDiff = 0;
#ifdef DEBUGPRINT
                printf("MSG:25:dateDiff for NULL entry calc done..\n");
#endif

        }
        else dateDiff = datecmp(temp->date, entry->date);

#ifdef DEBUGPRINT
        if (temp)
        printf("MSG:23:temp->date=%s, entry->date=%s, entry->next=%x, temp=%x, dateDiff=%d\n",
                temp->date, entry->date, entry->next, temp, 
                dateDiff);
        extern DayBookEntry dbMemHead;
        printf("MSG:10:rec=%s\n",(recType==PHYS_REC)?"PHYS":(recType==ANAL_REC?"ANAL_REC":"ANAL_REC2"));
#endif

	if (recType == ANAL_REC)
	{
#ifdef DEBUGPRINT
                printf("MSG:10: Inserting ANAL_REC2\n");
#endif
		/* Return a ANAL_REC2 record, with the net total. */
		(*record) = new DayBookRecord(this, entry);
                getCreditDebit(dateIndex, credit, debit);
                netTotal = credit - debit;
		(*record)->setCalculatedCredit(netTotal);
		(*record)->setCalculatedDebit(0);
                (*record)->setRecordType(ANAL_REC2);
		if (netTotal<0) (*record)->setUserNotify();
                else (*record)->clearUserNotify();
	}
	else if ((recType == ANAL_REC2) || (dateDiff==0))
	{
#ifdef DEBUGPRINT
                        printf("MSG:8: physical record\n");
#endif
			entry = temp;
			if (entry == NULL)
			{
#ifdef DEBUGPRINT
                                printf("MSG:9: NULL, so allocating\n");
#endif

				/* Allocate entry */
				allocNewEntry(&entry);
#ifdef DEBUGPRINT
                                printf("MSG:11: out of, allocNewEntry\n");
#endif

				insertEntry(entry);
#ifdef DEBUGPRINT
                                printf("MSG:12: out of, insertEntry\n");
                                if (entry==NULL) printf("MSG:27:allocation failed. \n");
#endif
			}
#ifdef DEBUGPRINT
                        printf("MSG:23:Physical record created\n");
#endif
			(*record) = new DayBookRecord(this, entry);
			(*record)->setRecordType(PHYS_REC);
	}
	else
	{
#ifdef DEBUGPRINT
                printf("MSG:10: Inserting ANAL_REC\n");
#endif

		/* We come here, if prev record is physical and the date
		on next record is different*/
		/* Return a ANAL_REC*/
		(*record) = new DayBookRecord(this, entry);
                getCreditDebit(dateIndex, credit, debit);
                (*record)->setCalculatedCredit(credit);
                (*record)->setCalculatedDebit(debit);
                (*record)->setRecordType(ANAL_REC);
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

Int DayBookFile::getPrev(RecordClass **record)
{
	DayBookEntry far *temp, *entry;
	Int day, month, year, dateIndex;
	Int recType, dateDiff;
        double netTotal, credit, debit;
	/* Get the record type */
	((DayBookRecord *)*record)->getEntry(&entry);
        if (entry==NULL)
        {
                return NOTHING_TO_SEARCH;
        }
	temp = entry;
#ifdef DEBUGPRINT
        static int getPrevHITCOUNTER=0;
        printf("MSG:33:getPrev hit count=%d\n", getPrevHITCOUNTER++);
        extern DayBookEntry dbMemHead;
        printf("MSG:34:entry=%x dbMemHead=%x\n",entry, dbMemHead);
#endif

	(*record)->getRecordType(recType);
	/* Get the current date from string. */
	getDMY(entry->date, day, month, year);
        dateIndex = month*31 + day;
#ifdef DEBUGPRINT
        printf("MSG:35:dateIndex:%d\n", dateIndex);
#endif
	/* Get the next record. */
        getPrevEntry(&temp);
        if ((temp == NULL))
        {
                /* */
#ifdef DEBUGPRINT
                printf("MSG:36:nextEntry NULL, should be FIRST entry.\n");
#endif
                if (recType == PHYS_REC)
                        return NOTHING_TO_SEARCH;
        }
        else dateDiff = datecmp(temp->date, entry->date);

#ifdef DEBUGPRINT
        if (temp)
        printf("MSG:37:temp->date=%s, entry->date=%s, entry->next=%x, temp=%x, dateDiff=%d\n",
                temp->date, entry->date, entry->next, temp, 
                dateDiff);
        extern DayBookEntry dbMemHead;
        printf("MSG:38:rec=%s\n",(recType==PHYS_REC)?"PHYS":(recType==ANAL_REC?"ANAL_REC":"ANAL_REC2"));
#endif
        if ((recType == ANAL_REC) || ((dateDiff==0) && (recType!=ANAL_REC2)))
	{
#ifdef DEBUGPRINT
                printf("MSG:40: physical record\n");
#endif
                if (recType !=ANAL_REC) entry = temp;
                (*record) = new DayBookRecord(this, entry);
                (*record)->setRecordType(PHYS_REC);
	}
        else if (recType == ANAL_REC2)
	{
                /* insert analytical record. */
		(*record) = new DayBookRecord(this, entry);
                getCreditDebit(dateIndex, credit, debit);
                (*record)->setCalculatedCredit(credit);
                (*record)->setCalculatedDebit(debit);
                (*record)->setRecordType(ANAL_REC);
	}
	else
	{
#ifdef DEBUGPRINT
                printf("MSG:39: Inserting ANAL_REC2\n");
#endif
		/* Return a ANAL_REC2 record, with the net total. */
                entry = temp;
                getDMY(entry->date, day, month, year);
                dateIndex = month*31 + day;
		(*record) = new DayBookRecord(this, entry);
                getCreditDebit(dateIndex, credit, debit);

                netTotal = credit - debit;
		(*record)->setCalculatedCredit(netTotal);
		(*record)->setCalculatedDebit(0);
                (*record)->setRecordType(ANAL_REC2);
		if (netTotal<0) (*record)->setUserNotify();
                else (*record)->clearUserNotify();
	}
#ifdef DEBUGPRINT
        if (*record)
                (*record)->getRecordType(recType);
        else printf("MSG:46:record invalid\n");
        printf("MSG:47:Done getting Next%d\n", recType);
#endif
        return OK;
}

Int DayBookFile::createNewRecord(RecordClass **record)
{
	DayBookEntry *entry;
	allocNewEntry(&entry);
        insertEntry(entry);
	*record = new DayBookRecord(this, entry);
        (*record)->setRecordType(PHYS_REC);
        return OK;
}

Int DayBookFile::throwRecord(RecordClass* record)
{
	DayBookEntry *entry;
#ifdef DEBUGPRINT
        printf("MSG:13:throwing record\n");
#endif
	((DayBookRecord *)record)->getEntry(&entry);
#ifdef DEBUGPRINT
        printf("MSG:18:entry at throw=%x\n", entry);
#endif
        if (validateDate(entry->date)!=OK)
	{
#ifdef DEBUGPRINT
                printf("MSG:19:Deleting and deallocating temporary entry =%s date=%s\n", entry->itemDesc, entry->date);
#endif

		deleteEntry(entry);
#ifdef DEBUGPRINT
                printf("MSG:112:After delete entry\n");
#endif

                if (entry->entryIndex == NEW_ENTRY)
                {
#ifdef DEBUGPRINT
                        printf("MSG:114: NEW_ENTRY DEALLOCING.\n");
#endif
                        deallocEntry(entry);
                }
#ifdef DEBUGPRINT
                printf("MSG:113:After dealloc\n");
#endif

	}
        else /*if (entry->entryIndex == NEW_ENTRY)*/
	{                
#ifdef DEBUGPRINT
                printf("MSG:20: Inserting new entry permenantly, by delete re-add\n", entry);
#endif
		/* Delete and re-add, so that the entry goes to the
		appropriate location.*/
                deleteReadd(entry);
#ifdef DEBUGPRINT
                printf("MSG:54:delete re-add done\n");
#endif
	}
        return OK;
}

Int DayBookFile::sortRecord(RecordClass* record)
{
	DayBookEntry *entry;
#ifdef DEBUGPRINT
        printf("MSG:13:throwing record\n");
#endif
	((DayBookRecord *)record)->getEntry(&entry);
#ifdef DEBUGPRINT
        printf("MSG:18:entry at throw=%x\n", entry);
#endif
        if (validateDate(entry->date)==OK /*&& (entry->entryIndex == NEW_ENTRY)*/)
	{
#ifdef DEBUGPRINT
                printf("MSG:20: Inserting new entry permenantly, by delete re-add\n", entry);
#endif

		/* Delete and re-add, so that the entry goes to the
		appropriate location.*/
                deleteReadd(entry);
#ifdef DEBUGPRINT
                printf("MSG:54:delete re-add done\n");
#endif
	}
        return OK;
}

Int DayBookFile::deleteRecord(RecordClass* record)
{
	DayBookEntry *entry;
	((DayBookRecord *)(record))->getEntry(&entry);
        deleteEntry(entry);
        if (entry->entryIndex == NEW_ENTRY)
                deallocEntry(entry);

        return OK;
}

Int DayBookFile::fileSearch()
{
#ifdef DEBUGPRINT
        printf("MSG:109:fileSearch daybook\n");
#endif
	inputBox("Enter the date to search", date);
        validateDate(date);
        return OK;
}


Int DayBookFile::isScreenReadOnly(Int& readOnly)
{
	readOnly = FALSE;
	return OK;
}
Int DayBookFile::print()
{
	FILE *fp;
        DayBookEntry *entry, *ptr;
        RecordClass *rec = NULL, *outRec;
        char crout[6], dbout[6];
        char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];
        char tempdate[9];
	char title[MAX_TITLE_LEN];
        char filename[FILENAME_LEN];
        Int len, recType, prevRecType;
        Int day, month, year;
        double openingCredit = 0.0, openingDebit = 0.0;
        Int cashOnHandDone = False;
        getFirstEntry(&entry);
        getDMY(entry->date, day, month, year);

        strcpy(filename, "DAYBOOK.TXT");

        fp = fopen(filename, "wb");
	if (!fp)
		return FOPENERR;

	strcpy(tempdate, date);
        memset(date, 0, 8);

        getFirst(&rec);
        rec->getRecordType(recType); 
	getTitle(title);
        fprintf(fp, "ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ\n");
        fprintf(fp, "%15s", " ");
        fprintf(fp, "%s 20%02d - 20%02d\n", title, year, year+1);
        fprintf(fp, "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
#ifdef DEBUGPRINTER
                printf("MSG:120: before while loop\n");
#endif

        while (rec)
	{
#ifdef DEBUGPRINTER
                printf("MSG:119: printer test loop before getEntry.\n");
#endif

                ((DayBookRecord *)rec)->getEntry(&entry);
                ptr = entry;
                
#ifdef DEBUGPRINTER
                printf("MSG:118: printer test loop after getEntry.%s\n", entry->itemDesc);
#endif

                if (validateDate(entry->date)!=OK)
                {
#ifdef DEBUGPRINTER
                        printf("MSG:121: Date invalid, ignore cur-rec.\n");
#endif

                        delete rec;
                        break;
                }
                if ((entry->isOpening) && (recType == PHYS_REC))
                {

                    printf("printing %s %lf %lf\n", entry->itemDesc, entry->credit, entry->debit); 

                    openingCredit += entry->credit;
                    openingDebit += entry->debit;
                    printf("Opening credit %lf opening debit %lf\n", openingCredit, openingDebit);
                    
                }
                else
                {
                    printf("COH done = %d\n", cashOnHandDone);
                    if (cashOnHandDone == False)
                    {
                        printf("adding COH entry\n");
                        DayBookEntry newEntry;
                        _fmemset(&newEntry, 0, sizeof(DayBookEntry));
                        strcpy(newEntry.date, entry->date);
                        newEntry.date[0]='0';
                        newEntry.date[1]='1';
                        newEntry.date[3]='0';
                        newEntry.date[4]='4';
                        strcpy(newEntry.itemDesc, "Cash on Hand");
                        newEntry.credit = openingCredit - openingDebit;
                        newEntry.debit = 0.0;
                        printHelper(fp, &newEntry, NULL);
                        cashOnHandDone = True;
                    }
                }
                if (!ptr->isOpening) printHelper(fp, ptr, rec);
#ifdef DEBUGPRINTER
                printf("MSG:123:. Done printing credit debit\n");
#endif

                rec->getRecordType(prevRecType);
#ifdef DEBUGPRINTER
                printf("MSG:124: after getRecordType\n");
#endif

                outRec = rec;
#ifdef DEBUGPRINTER
                printf("MSG:117: getNext start in print.\n");
#endif

                printf("getting next record\n");
                getNext(&rec);

                delete outRec;
                if (!rec) continue;
#ifdef DEBUGPRINTER
                printf("MSG:116: getNext in print pass.\n");
#endif
                rec->getRecordType(recType);
                if (cashOnHandDone)
                {
                    if ( (recType == ANAL_REC) ||
                             ((prevRecType == ANAL_REC2) && (recType == PHYS_REC)) )
                    {
                            fprintf(fp, "ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅ\n");
                    }
                }
	}
	strcpy(date, tempdate);
	fclose(fp);
        system("prin.py DAY\n");
        messageBox("Print complete");
	return OK;
}

Int DayBookFile::printHelper(FILE *fp, DayBookEntry *ptr, RecordClass *rec)
{
                Int len;
                char crout[6], dbout[6];
                double credit, debit;
                Int recType;
                static numLines=0;
                numLines++;
                if (numLines >=56)
                {
                    fprintf(fp, "\n\r%c", 0x0c);
                    numLines = 0;
                }
                if (rec!=NULL)
                    rec->getRecordType(recType);
                else recType = PHYS_REC;
                len = strlen(ptr->itemDesc);
                
#ifdef DEBUGPRINTER
                 printf("MSG:122: Length calculated.\n");
#endif
                if (recType == PHYS_REC)
                {
                        char tempchar;
                        if (len>40)
                        {
                                tempchar = ptr->itemDesc[40];
                                ptr->itemDesc[40] = 0;
                        }
                        fprintf(fp, "%-8s³%-40s³", ptr->date,
                                 ptr->itemDesc);

                        if (len>40)
                        {
                                ptr->itemDesc[40] = tempchar;
                                fprintf(fp, "%-9s³%c%c³%-9s³%c%c³\n", 
                                        "         ", ' ',' ', "         ",' ',' ');

                                fprintf(fp, "%-8s³%-40s³", ptr->date,
                                         ptr->itemDesc+40);
                        }

                }
                else
                {
                        fprintf(fp, "%-8s³%-40s³", " ",
                                 " ");

                }
#ifdef DEBUGPRINTER
                printf("MSG:123:. Ready to print credit debit\n");
#endif
                if (recType == PHYS_REC)
                {
                        credit = ptr->credit;
                        debit = ptr->debit;
                }
                else
                {
                        rec->getCalculatedCredit(credit);
                        rec->getCalculatedDebit(debit);
                }
                sprintf(crout, "%-0.2lf", credit-floor(credit));
                sprintf(dbout, "%-0.2lf", debit-floor(debit));

                if (credit > 0.01 && debit > 0.01)
                {
                        fprintf(fp, "%9.0lf³%c%c³%9.0lf³%c%c³\n", 
                                 floor(credit), crout[2], crout[3], floor(debit), dbout[2], dbout[3]);
                }
                else if ( (credit < 0.01) && (debit > 0.01) )
                {
                        fprintf(fp, "%-9s³%c%c³%9.0lf³%c%c³\n", 
                                 "         ", ' ', ' ', floor(debit), dbout[2], dbout[3]);
                }
                else if ( (credit > 0.01) && (debit < 0.01) )
                {
                        fprintf(fp, "%9.0lf³%c%c³%-9s³%c%c³\n", 
                                 floor(credit), crout[2], crout[3], "         ", ' ', ' ');
                }
                else
                {
                        fprintf(fp, "%-9s³%c%c³%-9s³%c%c³\n", 
                                 "         ", ' ', ' ', "         ", ' ', ' ');
                }
                return OK;
}

Int DayBookFile::checkTotals()
{
	/* It would have been great if we could do this
	only to the perticular date under consideration. Hope the traversal
	does not take too much time. */
	Int day, month, year;
	DayBookEntry far *entry;
	Int i;
        memset(date, 0, 8);
	for (i=0; i<372; i++)
	{
                totCredit[i] = 0.0;
                totDebit[i] = 0.0;
	}
#ifdef DEBUGPRINT
        printf("before getFirstEntry()\n");
#endif

	getFirstEntry(&entry);
	while (entry)
	{
                if (validateDate(entry->date)!=OK)
                {
#ifdef DEBUGPRINT
                        printf("MSG:1:continuing to next entry in getFirst\n");
#endif
                        getNextEntry(&entry);
                        continue;
                }
		getDMY(entry->date, day, month, year);
		i = month*31 + day;
		totCredit[i] += entry->credit;
		totDebit[i] += entry->debit;

#ifdef DEBUGTOTAL
                printf("entry->credit=%lf, entry->debit=%lf\n", entry->credit, entry->debit);
                printf("TOTAL:entry-item=%s %lf %lf\n", entry->itemDesc, totCredit[i], totDebit[i]);
#endif

#ifdef DEBUGPRINT
		printf("before getNextEntry()\n");
#endif
		getNextEntry(&entry);
#ifdef DEBUGPRINT
		printf("after getNextEntry()\n");
#endif
	}
#ifdef DEBUGPRINT
        printf("out of while loop()\n");
#endif
        return OK;
}

Int DayBookFile::recalcCreditDebit(RecordClass *record)
{
#ifdef DEBUGDYNAMIC
        printf("\nRecalculating credit debit.\n");
#endif
        Int day, month, year;
        Int recType;
        double credit, debit;
        DayBookRecord *drec = (DayBookRecord *)record;
        DayBookEntry *entry;
        drec->getEntry(&entry);
        getDMY(entry->date, day, month, year);
        getCreditDebit(day+month*31, credit, debit);
        record->getRecordType(recType);
        if (recType == ANAL_REC)
        {
                record->setCalculatedCredit(credit);
                record->setCalculatedDebit(debit);
        }
        if (recType == ANAL_REC2)
        {
                record->setCalculatedCredit(credit-debit);
                if (credit >= debit)
                {
                        record->clearUserNotify();
                }
                else
                {
                        record->setUserNotify();
                }
        }
        return OK;
}
Int DayBookFile::getCreditDebit(Int dateIndex, double& credit, double& debit)
{
        int i, endIndex;
        /* Year starts at date Index 3*31 = 93, because first
        date in financial year is 01/04/xx. This means dateIndex
        for 01/04/xx should be the first entry for operations. The
        01/01/xx, is not the first financially, but it is on our array. */
        credit = 0.0;
        debit = 0.0;
        if (dateIndex<93)
        {
                for (i=0; i<=dateIndex; i++)
                        credit+=totCredit[i] - totDebit[i];
                endIndex = 371;
        }
        else
                endIndex = dateIndex;
        for (i=93; i<=endIndex; i++)
                credit+=totCredit[i] - totDebit[i];

        debit = totDebit[dateIndex];

        credit = credit+debit;
        return OK;
}

DayBookFile::saveRecords()
{
        saveAllEntries();
        return OK;
}


