#include "errcodes.h"
#include "record.hpp"
#include "dayrec.hpp"
#include <ctype.h>
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
DayBookRecord::DayBookRecord(FileClass *filearg, DayBookEntry far *arg)
         :RecordClass(filearg)
{
        Int i;
        numOfFields = 5;
        for (i=0; i<numOfFields;i++)
        {
                fields[i][0] = '\0';
                fieldSizes[i] = dayBookFieldSizes[i];
        }
        entry = arg;
        strcpy(fields[0], entry->date);
        if (entry->head)
        {
                fields[1][0] = entry->head/10 + '0';
                fields[1][1] = entry->head%10 + '0';
        } else fields[1][0]=0;
        strcpy(fields[2], entry->itemDesc);
#ifdef DEBUGTOTAL
        printf("entry->credit=%lf, entry->debit=%lf\n", entry->credit, entry->debit);
#endif
        if (entry->credit)
                sprintf(fields[3],"%9.2lf", entry->credit);
        else fields[3][0] = 0;
        if (entry->debit)
                sprintf(fields[4],"%9.2lf", entry->debit);
        else fields[4][0] = 0;
}

Int DayBookRecord::fieldValidate()
{
#ifdef DEBUGVERB1
        printf("fieldValidation hit\n");
#endif
        
        Int head = atoi(fields[1]);
        if (fields[1][1] == 0) return OK;
        if (currentField == 1)
        {
#ifdef DEBUGVERB1
                printf("Current field is itemDesc field[2][0]=%c\n", fields[2][0]);
#endif

                autoLoad(fields[2], head);
                /* Item Description */

        }
        return RecordClass::fieldValidate();
}


Int DayBookRecord::getEntry(DayBookEntry far **pEntry)
{
	*pEntry = entry;
        return OK;

}
Int DayBookRecord::fieldChangeNotify()
{
        double flval;
	Int valueChange = FALSE;
#ifdef DEBUGPRINT
        printf("\nfield change notify.\n");
#endif
	strcpy(entry->date, fields[0]);
#ifdef DEBUGPRINT
        printf("\ndate at fieldChangeNotify=%s", entry->date);
#endif
        entry->head = atoi(fields[1]);
	strcpy(entry->itemDesc, fields[2]);
	flval = atof(fields[3]);
	if (entry->credit!=flval) valueChange = TRUE;
	entry->credit = flval;
	flval = atof(fields[4]);
	if (entry->debit!=flval) valueChange = TRUE;
	entry->debit = flval;
        if (valueChange)
        {
                file->checkTotals();
        }
	return OK;
}

Int DayBookRecord::setRecordType(Int recType)
{
        if (recType != PHYS_REC)
        {
                fields[0][0]=0;
                fields[1][0]=0;
                if (recType == ANAL_REC)
                        strcpy(fields[2],"Total:");
                else
                        strcpy(fields[2],"Net Total:");
        }
        recordType = recType;
        return OK;
}
