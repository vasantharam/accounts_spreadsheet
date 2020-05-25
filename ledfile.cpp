#include "ledfile.hpp"
#include "ledrec.hpp"
#include "headfile.hpp"
#include "headrec.hpp"

#include "fileman.hpp"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "graphab.hpp"
#include "errcodes.h"


LedgerBookFile::LedgerBookFile(HeadListBookFile *file)
{
        /* Initializing to 1 to test before inputBox is
                implemented. */
        headFile = file;
        strcpy(titleFile, "led.ttl");
        strcpy(defaultTitle, "Ledger Sheet");

        headCode = 0;
	checkTotals();
}
LedgerBookFile::~LedgerBookFile()
{
}

Int LedgerBookFile::getBlankRecord(RecordClass **record)
{
        DayBookEntry *entry;
        allocNewEntry(&entry);
        entry->head = headCode;
#ifdef DEBUGPRINT
        printf("MSG:604: insert entry before\n");
#endif
        insertEntry(entry);
        *record = new LedgerRecord(this, entry);
        return OK;
}

Int LedgerBookFile::getFirstCPL(RecordClass **record)
{
        HeadListRecord *headRec;
        LedgerHeadEntry *headEntry = NULL;
        Int loop;
        Int headIndex = 0;
        char str[MAX_TITLE_LEN];

        for (loop = 0; loop< MAX_HEADS; loop++)
        {
            plTotals[loop] = 0.0;
            plHeadCodes[loop] = 0;
        }


        headFile->getFirst((RecordClass **)&headRec);
        (headRec)->getEntry(&headEntry);
        while(headEntry && headEntry->headCode)
        {
            if (headEntry->ledgerType == PL_HEAD)
            {
                plTotals[headEntry->headCode] = getDayBookTotal(headEntry->headCode);
                plHeadCodes[headIndex] = headEntry->headCode;
                strcpy(plItemDesc[headIndex], headEntry->ledgerName);
                headIndex++;
            }
            headFile->getNext((RecordClass **)&headRec);
            (headRec)->getEntry(&headEntry);
        }
        createNewRecord(record);
        (*record)->setRecordType(PL_REC1);

        sprintf(str, "Transfered from %s a/c", plItemDesc[0]);
        (*record)->setItemDescription(str);
        if (plTotals[plHeadCodes[0]] < 0)
        {
            (*record)->setCalculatedCredit(0);
            (*record)->setCalculatedDebit(-1*plTotals[plHeadCodes[0]]);
        }
        else
        {
            (*record)->setCalculatedCredit(plTotals[plHeadCodes[0]]);
            (*record)->setCalculatedDebit(0);
        }

        return OK;

}

Int LedgerBookFile::getNextCPL(RecordClass **record)
{
    Int headIndex=0, recType;
    DayBookEntry *entry;
    double credit=0.0, debit=0.0;
    (*record)->getRecordType(recType);
    if (recType >= PL_REC1)
    {
        headIndex = recType - PL_REC1 + 1;

    }
    char str[MAX_TITLE_LEN];

    allocNewEntry(&entry);
    insertEntry(entry);
    *record = new LedgerRecord(this, entry);
    if ((recType >= PL_REC1) && (plHeadCodes[headIndex] == 0))
    {
        printf("ANAL_REC this should print \n");
        totCredit = 0.0;
        totDebit = 0.0;
        for (Int loop=0; loop< headIndex; loop++)
        {
            if (plTotals[plHeadCodes[loop]] < 0)
            {
                totDebit += -1 *plTotals[plHeadCodes[loop]];
            }
            else 
                totCredit += plTotals[plHeadCodes[loop]];
        }
        if (totCredit > totDebit)
        {
            credit = totCredit - totDebit;
        }
        else
            debit = totDebit - totCredit;

        (*record)->setRecordType(ANAL_REC);
        (*record)->setCalculatedCredit(debit);
        (*record)->setCalculatedDebit(credit);
        (*record)->setItemDescription("Transfered to Profit and Loss A/C");

        return OK;

    }

#ifdef DEBUGPRINT
    else printf("ANAL_REC why was there a mismatch %d %d\n", recType, plHeadCodes[headIndex]);
#endif
    if (recType == ANAL_REC)
    {
#ifdef DEBUGPRINT
        printf("ANAL_REC2\n");
#endif
        totCredit = 0.0;
        totDebit = 0.0;
        for (Int loop=0; loop<MAX_HEADS; loop++)
        {
            if (plTotals[plHeadCodes[loop]] < 0)
            {
                totDebit += -1 *plTotals[plHeadCodes[loop]];
#ifdef DEBUGPRINT
                printf("ANAL_REC totDebig = %lf" , totDebit);
#endif

            }
            else 
                totCredit += plTotals[plHeadCodes[loop]];
        }

        if (totCredit > totDebit)
        {
            (*record)->setCalculatedCredit(totCredit);
            (*record)->setCalculatedDebit(totCredit);
        }
        else
        {
            (*record)->setCalculatedCredit(totDebit);
            (*record)->setCalculatedDebit(totDebit);
        }

        /* insert ANAL_REC2 */
        (*record)->setRecordType(ANAL_REC2);


        return OK;
    }
    if ((recType == ANAL_REC2) || (recType == PHYS_REC))
    {
        /* insert empty physical record */
        (*record)->setRecordType(PHYS_REC);
        return OK;
    }
    (*record)->setRecordType(recType+1);
    sprintf(str, "Transfered from %s a/c", plItemDesc[headIndex]);
    (*record)->setItemDescription(str);
    if (plTotals[plHeadCodes[headIndex]] < 0)
    {
        (*record)->setCalculatedCredit(0);
        (*record)->setCalculatedDebit(-plTotals[plHeadCodes[headIndex]]);
        
    }
    else
    {
        (*record)->setCalculatedCredit(plTotals[plHeadCodes[headIndex]]);
        (*record)->setCalculatedDebit(0);
    }
    return OK;

}

Int LedgerBookFile::getNextGPL(RecordClass **record)
{
    /* pseudo code
     * if (recType == PHYS_REC) 
     *     create and return PL_REC1 for transfered entry
     * if (recType == PL_REC1)
     *     create and return ANAL_REC
     * if (recType == ANAL_REC)
     *     create and return ANAL_REC2 for last entry
     * if (recType == ANAL_REC2)
     *     create and return empty PHYS_REC, difference between
     *     first entry PHYS_REC is validity of date.
     */

    Int recType;
    DayBookEntry *entry;
    HeadListRecord *headRec;
    LedgerHeadEntry *headEntry = NULL;
    Int loop;
    Int headIndex = 0;
    char str[MAX_TITLE_LEN];
    double totalTransfer = 0.0, credit, debit;

    for (loop = 0; loop< MAX_HEADS; loop++)
    {
        plTotals[loop] = 0.0;
        plHeadCodes[loop] = 0;
    }


    headFile->getFirst((RecordClass **)&headRec);
    (headRec)->getEntry(&headEntry);
    while(headEntry && headEntry->headCode)
    {
        if (headEntry->ledgerType == PL_HEAD)
        {
            plTotals[headEntry->headCode] = getDayBookTotal(headEntry->headCode);
            totalTransfer += plTotals[headEntry->headCode];
            plHeadCodes[headIndex] = headEntry->headCode;
            strcpy(plItemDesc[headIndex], headEntry->ledgerName);
            headIndex++;
        }
        headFile->getNext((RecordClass **)&headRec);
        (headRec)->getEntry(&headEntry);
    }
    (*record)->getRecordType(recType);

    if (recType == PHYS_REC)
    {
        ((LedgerRecord *)*record)->getEntry(&entry);
        if (validateDate(entry->date) != OK)
        {
            recType = ANAL_REC2;
        }
    }

    allocNewEntry(&entry);
    insertEntry(entry);

    
    switch(recType)
    {
        case PHYS_REC:
            entry->head = headCode;
            *record = new LedgerRecord(this, entry);
            (*record)->setRecordType(PL_REC1);
            if (totalTransfer <0)
            {
                (*record)->setCalculatedDebit(-1*totalTransfer);
                (*record)->setCalculatedCredit(0);
            }
            else
            {
                (*record)->setCalculatedCredit(totalTransfer);
                (*record)->setCalculatedDebit(0);
            }
            (*record)->setItemDescription("Transfered from P&L current Yr");
            break;
        case PL_REC1:
            (*record) = new LedgerRecord(this, entry);
            entry = NULL;
            findEntry(&entry, NULL, headCode, POSITIVE);
            
            (*record)->setRecordType(ANAL_REC);
            if (totalTransfer < 0)
            {
                debit = entry->debit - totalTransfer;
                credit = entry->credit;
            }
            else
            {
                credit = entry->credit + totalTransfer;
                debit = entry->debit;
            }
            (*record)->setCalculatedCredit(credit);
            (*record)->setCalculatedDebit(debit);
            break;
        case ANAL_REC:
            (*record) = new LedgerRecord(this, entry);
            entry = NULL;
            findEntry(&entry, NULL, headCode, POSITIVE);

            (*record)->setRecordType(ANAL_REC2 );
            if (totalTransfer < 0)
            {
                debit = entry->debit - totalTransfer;
                credit = entry->credit;
            }
            else
            {
                credit = entry->credit + totalTransfer;
                debit = entry->debit;
            }
            if (credit > debit)
            {
                (*record)->setCalculatedCredit(credit-debit);
                (*record)->setCalculatedDebit(0);
            }
            else
            {
                (*record)->setCalculatedCredit(0);
                (*record)->setCalculatedDebit(debit - credit);
            }
            break;
        case ANAL_REC2:
            (*record) = new LedgerRecord(this, entry);
            (*record)->setRecordType(PHYS_REC);
            break;
    }
    return OK;
}

Int LedgerBookFile::getFirst(RecordClass **record)
{
	DayBookEntry *entry=NULL;
#ifdef DEBUGPRINT
        printf("ledgerType in getFirst is %s", ledgerType==CPL_HEAD?"CPL_HEAD":"NON_CPL");
#endif
        if (ledgerType == CPL_HEAD)
        {
            getFirstCPL(record);
            return OK;
        }
	findEntry(&entry, NULL, headCode, POSITIVE);
        if (entry == NULL) createNewRecord(record);
        else *record = (RecordClass *)new LedgerRecord(this, entry);
#ifdef DEBUGPRINT
        printf("MSG:606: End of getFirst\n");
#endif
        (*record)->setRecordType(PHYS_REC);
	return OK;
}
Int LedgerBookFile::getNext(RecordClass **record)
{
	DayBookEntry far *temp, *entry;
	Int recType, dateDiff;
        double netTotal;
#ifdef DEBUGPRINT
        printf("ledgerType in getFirst is %s", ledgerType==CPL_HEAD?"CPL_HEAD":"NON_CPL");
#endif

        if (ledgerType == CPL_HEAD)
        {
            return getNextCPL(record);
            
        }
        else if (ledgerType == GPL_HEAD)
        {
            return getNextGPL(record);
            
        }
	/* Get the record type */
        ((LedgerRecord *)*record)->getEntry(&entry);
	temp = entry;
#ifdef DEBUGPRINT
        static int getNextHITCOUNTER=0;
        printf("MSG:25:getNext hit count=%d\n", getNextHITCOUNTER++);
        extern DayBookEntry dbMemHead;
        printf("MSG:24:entry=%x dbMemHead=%x\n",entry, dbMemHead);
#endif

	(*record)->getRecordType(recType);

	/* Get the next record. */
        findEntry(&temp, NULL, headCode, POSITIVE);
#ifdef DEBUGPRINT
        printf("MSG:68: findEntry done.\n");
#endif
        dateDiff = 0;
        if (temp == NULL)
        {
                /* */
#ifdef DEBUGPRINT
                printf("MSG:22:nextEntry NULL, should be last entry.\n");
#endif
                if (validateDate(entry->date) == OK) dateDiff = 1;
#ifdef DEBUGPRINT
                printf("MSG:25:dateDiff for NULL entry calc done..\n");
#endif
        }


	if (recType == ANAL_REC)
	{
#ifdef DEBUGPRINT
                printf("MSG:10: Inserting ANAL_REC2\n");
#endif
		/* Return a ANAL_REC2 record, with the net total. */
                (*record) = new LedgerRecord(this, entry);                
                (*record)->setRecordType(ANAL_REC2);
                if (ledgerType == PL_HEAD)
                {
                    if (totCredit > totDebit)
                    {
                        (*record)->setCalculatedCredit(totCredit);
                        (*record)->setCalculatedDebit(totCredit);
                    }
                    else
                    {
                        (*record)->setCalculatedCredit(totDebit);
                        (*record)->setCalculatedDebit(totDebit);

                    }
                }
                else
                {
                    if (totCredit > totDebit)
                    {
                        (*record)->setCalculatedCredit(totCredit - totDebit);
                        (*record)->setCalculatedDebit(0);
                    }
                    else
                    {
                        (*record)->setCalculatedCredit(0);
                        (*record)->setCalculatedDebit(totDebit - totCredit);
                    }
                }
               
                if (totDebit > totCredit) (*record)->setUserNotify();
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
                        (*record) = new LedgerRecord(this, entry);
			(*record)->setRecordType(PHYS_REC);
	}
	else
	{
#ifdef DEBUGPRINT
                printf("MSG:10: Inserting ANAL_REC\n");
#endif

		/* Return a ANAL_REC*/
                (*record) = new LedgerRecord(this, entry);

                (*record)->setCalculatedCredit(totCredit);
                (*record)->setCalculatedDebit(totDebit);
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

Int LedgerBookFile::getPrevCPL(RecordClass **record)
{
    /* pseudo code
     *
     * getRecordType
     * if ANAL_REC2
     *   create and return ANAL_REC
     * if ANAL_REC
     *   create and return last PL_REC?
     * if PHYS_REC
     *   create and return ANAL_REC2
     * if PL_REC?
     *   create and return previosu PL_REC
     * if PL_REC1
     *   return NOTHING_TO_SEARCH
     *
     */
     Int recType, loop;
     DayBookEntry *entry;
     double credit=0.0, debit=0.0;
     char str[MAX_TITLE_LEN];
     (*record)->getRecordType(recType);
     if (recType == PL_REC1)
         return NOTHING_TO_SEARCH;
     allocNewEntry(&entry);
     insertEntry(entry);
     *record = new LedgerRecord(this, entry);


     switch(recType)
     {
         case ANAL_REC2:
             totCredit = 0.0;
             totDebit = 0.0;
             for (Int loop=0; loop< MAX_HEADS; loop++)
             {
                 if (plTotals[plHeadCodes[loop]] < 0)
                 {
                     totDebit += -1 *plTotals[plHeadCodes[loop]];
                 }
                 else 
                     totCredit += plTotals[plHeadCodes[loop]];
             }
             if (totCredit > totDebit)
             {
                 credit = totCredit - totDebit;
             }
             else
                 debit = totDebit - totCredit;

             (*record)->setRecordType(ANAL_REC);
             (*record)->setCalculatedCredit(debit);
             (*record)->setCalculatedDebit(credit);
             (*record)->setItemDescription("Transfered to Profit and Loss A/C");

             break;
         case ANAL_REC:
             for (loop=0; loop<MAX_HEADS; loop++)
             {
                 if (plHeadCodes[loop] == 0) break;
             }
             if (loop == MAX_HEADS) return NOTHING_TO_SEARCH;
             loop--;

             (*record)->setRecordType(loop + PL_REC1);
             sprintf(str, "Transfered from %s a/c", plItemDesc[loop]);
             (*record)->setItemDescription(str);
             if (plTotals[plHeadCodes[loop]] < 0)
             {
                 (*record)->setCalculatedCredit(0);
                 (*record)->setCalculatedDebit(-plTotals[plHeadCodes[loop]]);
        
             }
             else
             {
                 (*record)->setCalculatedCredit(plTotals[plHeadCodes[loop]]);
                 (*record)->setCalculatedDebit(0);
             }
             break;
         case PHYS_REC:
             totCredit = 0.0;
             totDebit = 0.0;
             for (Int loop=0; loop< MAX_HEADS; loop++)
             {
                 if (plTotals[plHeadCodes[loop]] < 0)
                 {
                     totDebit += -1 *plTotals[plHeadCodes[loop]];
                 }
                 else 
                     totCredit += plTotals[plHeadCodes[loop]];
             }

             (*record)->setRecordType(ANAL_REC2);
             if (totCredit > totDebit)
             {
                 (*record)->setCalculatedCredit(totCredit);
                 (*record)->setCalculatedDebit(totCredit);
             }
             else
             {
                 (*record)->setCalculatedCredit(totDebit);
                 (*record)->setCalculatedDebit(totDebit);
             }
        
             break;
         case PL_REC7:
         case PL_REC6:
         case PL_REC5:
         case PL_REC4:
         case PL_REC3:
         case PL_REC2:
             loop = recType-1-PL_REC1;

             (*record)->setRecordType(loop + PL_REC1);
             sprintf(str, "Transfered from %s a/c", plItemDesc[loop]);
             (*record)->setItemDescription(str);
             if (plTotals[plHeadCodes[loop]] < 0)
             {
                 (*record)->setCalculatedCredit(0);
                 (*record)->setCalculatedDebit(-plTotals[plHeadCodes[loop]]);
        
             }
             else
             {
                 (*record)->setCalculatedCredit(plTotals[plHeadCodes[loop]]);
                 (*record)->setCalculatedDebit(0);
             }
             break;

             
     }
     return OK;
}

Int LedgerBookFile::getPrevGPL(RecordClass **record)
{
    /* pseudo code
     *
     * if (recType == PHYS_REC and date invalid)
     *    create and return ANAL_REC2
     * if (recType == ANAL_REC2)
     *    create and return ANAL_REC
     * if (recType == ANAL_REC)
     *    create and return PL_REC1
     * if (recType == PL_REC1)
     *    create and return PHYS_REC, first entry
     * if (recType == PHYS_REC and date valid) return NOTHING_TO_SEARCH
     *
     */
    Int recType;
    DayBookEntry *entry;
    HeadListRecord *headRec;
    LedgerHeadEntry *headEntry = NULL;
    Int loop;
    Int headIndex = 0;
    char str[MAX_TITLE_LEN];
    double totalTransfer = 0.0, credit, debit;
    Int dateValid;
    for (loop = 0; loop< MAX_HEADS; loop++)
    {
        plTotals[loop] = 0.0;
        plHeadCodes[loop] = 0;
    }


    headFile->getFirst((RecordClass **)&headRec);
    (headRec)->getEntry(&headEntry);
    while(headEntry && headEntry->headCode)
    {
        if (headEntry->ledgerType == PL_HEAD)
        {
            plTotals[headEntry->headCode] = getDayBookTotal(headEntry->headCode);
            totalTransfer += plTotals[headEntry->headCode];
            plHeadCodes[headIndex] = headEntry->headCode;
            strcpy(plItemDesc[headIndex], headEntry->ledgerName);
            headIndex++;
        }
        headFile->getNext((RecordClass **)&headRec);
        (headRec)->getEntry(&headEntry);
    }
    (*record)->getRecordType(recType);

    ((LedgerRecord *)(*record))->getEntry(&entry);
    dateValid = (validateDate(entry->date) == OK);

    allocNewEntry(&entry);
    insertEntry(entry);
    switch(recType)
    {
        case PHYS_REC:
            if (dateValid) return NOTHING_TO_SEARCH;
            else
            {
                *record = new LedgerRecord(this, entry);
                (*record)->setRecordType(ANAL_REC2);
                (*record) = new LedgerRecord(this, entry);
                entry = NULL;
                findEntry(&entry, NULL, headCode, POSITIVE);

                (*record)->setRecordType(ANAL_REC2 );
                if (totalTransfer < 0)
                {
                    debit = entry->debit - totalTransfer;
                    credit = entry->credit;
                }
                else
                {
                    credit = entry->credit + totalTransfer;
                    debit = entry->debit;
                }
                if (credit > debit)
                {
                    (*record)->setCalculatedCredit(credit-debit);
                    (*record)->setCalculatedDebit(0);
                }
                else
                {
                    (*record)->setCalculatedCredit(0);
                    (*record)->setCalculatedDebit(debit - credit);
                }
                break;

            }
        case ANAL_REC2:
            (*record) = new LedgerRecord(this, entry);
            entry = NULL;
            findEntry(&entry, NULL, headCode, POSITIVE);
            
            (*record)->setRecordType(ANAL_REC);
            if (totalTransfer < 0)
            {
                debit = entry->debit - totalTransfer;
                credit = entry->credit;
            }
            else
            {
                credit = entry->credit + totalTransfer;
                debit = entry->debit;
            }
            (*record)->setCalculatedCredit(credit);
            (*record)->setCalculatedDebit(debit);

            break;
        case ANAL_REC:
            entry->head = headCode;
            *record = new LedgerRecord(this, entry);
            (*record)->setRecordType(PL_REC1);
            if (totalTransfer <0)
            {
                (*record)->setCalculatedDebit(-1*totalTransfer);
                (*record)->setCalculatedCredit(0);
            }
            else
            {
                (*record)->setCalculatedCredit(totalTransfer);
                (*record)->setCalculatedDebit(0);
            }
            (*record)->setItemDescription("Transfered from P&L current Yr");
            break;
        case PL_REC1:
            getFirst(record);
            break;

    }
    return OK;
}

Int LedgerBookFile::getPrev(RecordClass **record)
{
	DayBookEntry far *temp, *entry;
	Int recType, dateDiff;
        double netTotal;
        if (ledgerType == CPL_HEAD)
        {
            return getPrevCPL(record);
            
        }
        else if (ledgerType == GPL_HEAD)
        {
            return getPrevGPL(record);
        }

	/* Get the record type */
        ((LedgerRecord *)*record)->getEntry(&entry);
        if (entry==NULL)
        {
                return NOTHING_TO_SEARCH;
        }
	temp = entry;
#ifdef DEBUGPRINT
        static int getPrevHITCOUNTER=0;
        printf("MSG:33:getPrev hit count=%d\n", getPrevHITCOUNTER++);
        extern DayBookEntry dbMemHead;
        printf("MSG:34:entry=%x dbMemHead=%x\n", entry, dbMemHead);
#endif

	(*record)->getRecordType(recType);

	/* Get the next record. */
        findEntry(&temp, NULL, headCode, NEGATIVE);
        if (temp == NULL)
        {
                /* */
#ifdef DEBUGPRINT
                printf("MSG:36:nextEntry NULL, should be FIRST entry.\n");
#endif
                return NOTHING_TO_SEARCH;
        }

#ifdef DEBUGPRINT
        if (temp)
        printf("MSG:37:temp->date=%s, entry->date=%s, entry->next=%x, temp=%x, dateDiff=%d\n",
                temp->date, entry->date, entry->next, temp, 
                dateDiff);
        extern DayBookEntry dbMemHead;
        printf("MSG:38:rec=%s\n",(recType==PHYS_REC)?"PHYS":(recType==ANAL_REC?"ANAL_REC":"ANAL_REC2"));
#endif
        if ((recType == ANAL_REC) || (recType!=ANAL_REC2 && validateDate(entry->date)==OK))
	{
#ifdef DEBUGPRINT
                printf("MSG:40: physical record\n");
#endif
                if (recType !=ANAL_REC) entry = temp;
                (*record) = new LedgerRecord(this, entry);
                (*record)->setRecordType(PHYS_REC);
	}
        else if (recType == ANAL_REC2)
	{
                /* insert analytical record. */
                (*record) = new LedgerRecord(this, entry);

                (*record)->setCalculatedCredit(totCredit);
                (*record)->setCalculatedDebit(totDebit);
                (*record)->setRecordType(ANAL_REC);
	}
        else if ( validateDate(entry->date) != OK)
	{
#ifdef DEBUGPRINT
                printf("MSG:39: Inserting ANAL_REC2\n");
#endif
		/* Return a ANAL_REC2 record, with the net total. */
                entry = temp;
                (*record) = new LedgerRecord(this, entry);


                netTotal = totCredit - totDebit;
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

Int LedgerBookFile::createNewRecord(RecordClass **record)
{
	DayBookEntry *entry;
	allocNewEntry(&entry);
        entry->head = headCode;
#ifdef DEBUGPRINT
        printf("MSG:604:insertEntry from createNewRecord\n");
#endif
	insertEntry(entry);
#ifdef DEBUGPRINT
        printf("MSG:605:insertEntry from createNewRecord\n");
#endif

	*record = new LedgerRecord(this, entry);
#ifdef DEBUGPRINT
        printf("MSG:606: after new record\n");
#endif
	return OK;
}

Int LedgerBookFile::throwRecord(RecordClass* record)
{
	DayBookEntry *entry;
	((LedgerRecord *)record)->getEntry(&entry);
        if (validateDate(entry->date)!=OK)
	{
#ifdef DEBUGPRINT
                printf("MSG:91:Deleting entry %s\n", entry->itemDesc);
#endif
		deleteEntry(entry);
                if (entry->entryIndex == NEW_ENTRY) deallocEntry(entry);
	}
        else /*if (entry->entryIndex == NEW_ENTRY)*/
	{
		/* Delete and re-add, so that the entry goes to the
                appropriate location. deleteReadd, tries to prevent,
                rotation of screen records*/
                deleteReadd(entry);
		entry->entryIndex = EXISTING_ENTRY;
	}
	return OK;
}

Int LedgerBookFile::sortRecord(RecordClass* record)
{
	DayBookEntry *entry;
	((LedgerRecord *)record)->getEntry(&entry);
        if ((validateDate(entry->date)==OK) /*&& (entry->entryIndex == NEW_ENTRY)*/)
	{
		/* Delete and re-add, so that the entry goes to the
		appropriate location.*/
                deleteReadd(entry);
		entry->entryIndex = EXISTING_ENTRY;
	}
	return OK;
}

Int LedgerBookFile::deleteRecord(RecordClass* record)
{
	DayBookEntry *entry;
	((LedgerRecord *)(record))->getEntry(&entry);
	/* If date is NULL, entry will be deleted during
	constructor destruction. */
        memset(entry->date, 0, 8);
	return OK;
}
Int LedgerBookFile::fileSearch()
{
	/* Currently useless. Make it useful. */
	inputBox("Enter the date to search", date);
        return OK;
}

Int LedgerBookFile::isScreenReadOnly(Int& readOnly)
{
        
	readOnly = FALSE;
        return OK;
}

Int LedgerBookFile::print()
{
	FILE *fp;
        DayBookEntry *entry, *ptr;
        LedgerHeadEntry *headEntry;
        RecordClass *rec, *outRec, *headRec;
        char crout[6], dbout[6], pageStr[6];
        char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];

	char title[MAX_TITLE_LEN];
        char filename[FILENAME_LEN];
        Int len,  recType, prevRecType, numLines, page, pl_page=0;
        Int prevHeadCode = headCode;
        Int prevLedgerType = ledgerType;
        double credit, debit, totCredit=0.0, totDebit=0.0;

        headFile->getFirst(&headRec);
        ((HeadListRecord *)headRec)->getEntry(&headEntry);

        while (headEntry->headCode)
        {
            rec = NULL;
            headCode = headEntry->headCode;
            ledgerType = headEntry->ledgerType;
            printf("Head Code = %d ledgerType = %d\n", headCode, headEntry->ledgerType);
            checkTotals();
            if ((
                (headEntry -> ledgerType >= PL_HEAD)
                )
                &&
                (pl_page == 0)
                )
            {
#ifdef DEBUGPRINT
                printf("MSG:603: PL_HEAD and PL_PAGE==0\n");
#endif
                pl_page = headEntry->pageOrderCode;
                page = pl_page;
            }
            else if (pl_page == 0)
                page = headEntry->pageOrderCode;
            else page = pl_page;
            sprintf(filename, "LED%d.TXT", headCode);
    
            fp = fopen(filename, "wb");
            if (!fp)
    		return FOPENERR;
    
    
            getFirst(&rec);
        
            getTitle(title);
            fprintf(fp, "ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍPageÍ%dÍÍ\n", page++);
            fprintf(fp, "%25s%s\n", " ", title);
            fprintf(fp, "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
            numLines = 3;
#ifdef DEBUGPRINTER
                printf("MSG:120: before while loop\n");
#endif

            while (rec)
            {
#ifdef DEBUGPRINTER
                printf("MSG:119: printer test loop before getEntry.\n");
#endif

                ((LedgerRecord *)rec)->getEntry(&entry);
                ptr = entry;
                
#ifdef DEBUGPRINTER
                printf("MSG:118: printer test loop after getEntry.%s\n", entry->itemDesc);
#endif

                rec->getRecordType(recType);
#ifdef DEBUGPRINTER
                printf("recType %d\n", recType);
#endif
                if ((recType == PHYS_REC) && (validateDate(entry->date)!=OK))
                {
#ifdef DEBUGPRINTER
                        printf("MSG:121: Date invalid, ignore cur-rec.\n");
#endif
                        delete rec;
                        break;

                }
                len = strlen(ptr->itemDesc);
#ifdef DEBUGPRINTER
                 printf("MSG:122: Length calculated.\n");
#endif
                
                if (recType == PHYS_REC)
                {
                        char tempchar;
#ifdef DEBUGPRINTER
                         printf("PRNMSG:122: itemDesc %s.\n", ptr->itemDesc);
#endif
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
                                numLines++;
                                fprintf(fp, "%-8s³%-40s³", ptr->date,
                                         ptr->itemDesc+40);
                        }
                }
                else
                {
                        rec->getFields(&fields);
                        fprintf(fp, "%-8s³%-40s³", " ",
                                 (*fields)[2]);


                }
#ifdef DEBUGPRINTER
                printf("MSG:123:. Ready to print credit debit recType = %d\n", recType);
#endif
                if ( (recType == PHYS_REC))
                {
                        credit = ptr->credit;
                        debit = ptr->debit;
                        totCredit +=credit;
                        totDebit +=debit;
                }
                else
                {
                        rec->getCalculatedCredit(credit);
                        rec->getCalculatedDebit(debit);
                }
                sprintf(crout, "%-0.2f", credit-floor(credit));
                sprintf(dbout, "%-0.2f", debit-floor(debit));

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
                if ((recType != PHYS_REC) && (recType <PL_REC1))
                    fprintf(fp, "ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅ\n");
                numLines++;
                if (numLines >= 57)
                {
                        fprintf(fp, "\n\r%c", 0xc);
                        numLines = 0;
                        fprintf(fp, "ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅ\n");
                        fprintf(fp, "%-8s³%-40s³", " ",
                                 " ");

                        sprintf(crout, "%-0.2f", totCredit-floor(totCredit));
                        sprintf(dbout, "%-0.2f", totDebit-floor(totDebit));

                        if (totCredit > 0.01 && totDebit > 0.01)
                        {
                                fprintf(fp, "%9.0lf³%c%c³%9.0lf³%c%c³\n", 
                                         floor(totCredit), crout[2], crout[3], floor(totDebit), dbout[2], dbout[3]);
                        }
                        else if ( (totCredit < 0.01) && (totDebit > 0.01) )
                        {
                                fprintf(fp, "%-9s³%c%c³%9.0lf³%c%c³\n", 
                                         "         ", ' ', ' ', floor(totDebit), dbout[2], dbout[3]);
                        }
                        else if ( (totCredit > 0.01) && (totDebit < 0.01) )
                        {
                                fprintf(fp, "%9.0lf³%c%c³%-9s³%c%c³\n", 
                                         floor(totCredit), crout[2], crout[3], "         ", ' ', ' ');
                        }
                        else
                        {
                                fprintf(fp, "%-9s³%c%c³%-9s³%c%c³\n", 
                                         "         ", ' ', ' ', "         ", ' ', ' ');
                        }

                        fprintf(fp, "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄPageÄ%dÄÄÄ\n", page++);
                        
                }
#ifdef DEBUGPRINTER
                printf("MSG:123:. Done printing credit debit %d\n", headCode);
#endif

                rec->getRecordType(prevRecType);
#ifdef DEBUGPRINTER
                printf("MSG:124: after getRecordType %d\n", headCode);
#endif

                outRec = rec;
#ifdef DEBUGPRINTER
                printf("MSG:117: getNext start in print. %d\n", headCode);
#endif

                getNext(&rec);

                delete outRec;
                if (!rec) continue;
#ifdef DEBUGPRINTER
                printf("MSG:116: getNext in print pass. %d\n", headCode);
#endif
                rec->getRecordType(recType);
                
                if ( (recType == ANAL_REC) ||
                         ((prevRecType == ANAL_REC2) && (recType == PHYS_REC)) )
                {
                        fprintf(fp, "ÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÅ\n");
                }
	    }
            fclose(fp);
            if (headEntry->ledgerType >= PL_HEAD)
            {
                pl_page = page;
            }
            headFile->getNext(&headRec);
            ((HeadListRecord *)headRec)->getEntry(&headEntry);
        }
        headCode = prevHeadCode;
        ledgerType = prevLedgerType;
        system("prin.py LED\n");
        messageBox("Print complete");
	return OK;

}

Int LedgerBookFile::recalcCreditDebit(RecordClass *record)
{
#ifdef DEBUGDYNAMIC
        printf("\nRecalculating credit debit.\n");
#endif
        Int recType;


        record->getRecordType(recType);
        if (recType == ANAL_REC)
        {
                record->setCalculatedCredit(totCredit);
                record->setCalculatedDebit(totDebit);
        }
        if (recType == ANAL_REC2)
        {
                record->setCalculatedCredit(totCredit-totDebit);
                if (totCredit >= totDebit)
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

Int  LedgerBookFile::setActive()
{
	char result[MAX_FIELD_LEN];
	inputBox("Enter the Ledger Number:", (char *)result);
	headCode = atoi(result);
        checkTotals();
#ifdef DEBUGPRINT
        printf("MSG:61:headCode = %d", headCode);
#endif
        return RELOAD_RECORDS;
}

Int LedgerBookFile::checkTotals()
{
	DayBookEntry *entry = NULL;
#ifdef DEBUGPRINT
        printf("MSG:2:checkTotals in Ledger headCode = %d\n", headCode);
#endif

	findEntry(&entry, NULL, headCode, POSITIVE);
        totCredit = 0.0;
        totDebit = 0.0;
	while (entry)
	{
#ifdef DEBUGPRINT
                printf("MSG:51:matching ledger entry found date=%s, item=%s\n", entry->date, entry->itemDesc);
#endif

                        
                totCredit += entry->credit;
                totDebit +=  entry->debit;
                findEntry(&entry, NULL, headCode, POSITIVE);            
	}
        return OK;
}

Int LedgerBookFile::getTitle(char *title)
{
        HeadListRecord *headRec;
        LedgerHeadEntry *headEntry = NULL;

        headFile->getFirst((RecordClass **)&headRec);
        (headRec)->getEntry(&headEntry);
        while(headEntry && headEntry->headCode)
        {
            if (headEntry->headCode == headCode) break;
            headFile->getNext((RecordClass **)&headRec);
            (headRec)->getEntry(&headEntry);
        }

        if (headEntry && headEntry->headCode)
        {
            ledgerType = headEntry->ledgerType;
            sprintf(defaultTitle, "%s", headEntry->ledgerName);
#ifdef DEBUGPRINT
            printf("setting ledgerType\n");
#endif
        }
        else
            sprintf(defaultTitle, "UNKNOWN HEAD");
#ifdef DEBUGPRINT
        printf("MSG:602:End of getTitle Title: %s\n", defaultTitle);
#endif
        strcpy(title, defaultTitle);
        return OK;
}
