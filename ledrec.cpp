#include "record.hpp"
#include "ledfile.hpp"
#include "ledrec.hpp"
#include "headfile.hpp"
#include "headrec.hpp"
#include "common.h"
#include "errcodes.h"
#include <string.h>
#include <math.h>

LedgerRecord::LedgerRecord(FileClass *argfile, DayBookEntry far *arg)
	:RecordClass(argfile)
{
	Int i;
	numOfFields = 5;
	for (i=0; i<numOfFields;i++)
	{
		fields[i][0] = '\0';
		fieldSizes[i]=dayBookFieldSizes[i];
	}
	entry = arg;
        strcpy(fields[0], entry->date);
        if (entry->head)
        {
                fields[1][0] = entry->head/10 + '0';
                fields[1][1] = entry->head%10 + '0';
        } else fields[1][0]=0;

        strcpy(fields[2], entry->itemDesc);
        if (entry->credit)
                sprintf(fields[3],"%9.2f", entry->credit);
        else fields[3][0] = 0;
        if (entry->debit)
                sprintf(fields[4],"%9.2f", entry->debit);
        else fields[4][0] = 0;
}


Int LedgerRecord::getEntry(DayBookEntry far **argentry)
{
	*argentry = entry;
	return OK;
}

Int LedgerRecord::fieldChangeNotify()
{
        double flval;
	Int valueChange = FALSE;
	strcpy(entry->date, fields[0]);
        entry->head = atoi(fields[1]);
	strcpy(entry->itemDesc, fields[2]);
	flval = atof(fields[3]);
	if (entry->credit!=flval) valueChange = TRUE;
	entry->credit = flval;
	flval = atof(fields[4]);
	if (entry->debit!=flval) valueChange = TRUE;
	entry->debit = flval;
        if (valueChange) file->checkTotals();
	return OK;
}

Int LedgerRecord::setRecordType(Int recType)
{
        DayBookEntry *ptr;
        Int day, month, year, match = False;
        LedgerHeadEntry *headEntry;
        RecordClass *headrec;

        HeadListBookFile *headFile;
        ((LedgerBookFile *)file)->getHeadFile(&headFile);

        headFile->getFirst(&headrec);
        ((HeadListRecord *)headrec)->getEntry(&headEntry);

        while(headEntry)
        {
            if (headEntry->headCode == 0) break;
            if (headEntry->headCode == entry->head)
            {
                if (headEntry->ledgerType == PL_HEAD) match = True;
                break;
            }
            headFile->getNext(&headrec);
            ((HeadListRecord *)headrec)->getEntry(&headEntry);
        }

        getFirstEntry(&ptr);
        getDMY(ptr->date, day, month, year);
        if (recType != PHYS_REC)
        {
                fields[0][0]=0;
                fields[1][0]=0;
                if (recType == ANAL_REC)
                {
                        if (match)
                        {
                            sprintf(fields[2],"Transfered to P&L A/C for year %02d-%02d:", year, year+1);
                            if (calculatedCredit >= calculatedDebit)
                            {
                                setCalculatedDebit(calculatedCredit-calculatedDebit);
                                setCalculatedCredit(0);
                            }
                            else
                            {
                                setCalculatedCredit(calculatedDebit - calculatedCredit);
                                setCalculatedDebit(0);
                            }
                        }
                        else
                            strcpy(fields[2],"Total:");
                }
                else
                {
                        strcpy(fields[2],"Net Total:");
                        if (match)
                        {
                            setCalculatedCredit(0);
                            setCalculatedDebit(0);
                        }
                }
        }
        recordType = recType;
        return OK;
        
}
