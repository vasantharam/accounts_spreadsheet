#include "errcodes.h"
#include "record.hpp"
#include "balrec.hpp"
#include "common.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

BalanceRecord::BalanceRecord(FileClass *filearg, BalanceEntry *arg)
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
        if (entry->page) sprintf(fields[1], "%-2d", entry->page);
        if (entry->itemDesc) sprintf(fields[2], "%s", entry->itemDesc);
        if (entry->credit) sprintf(fields[3], "%9.2lf", entry->credit);
        if (entry->debit) sprintf(fields[4], "%9.2lf", entry->debit);
}

Int BalanceRecord::getEntry(BalanceEntry far**pEntry)
{
	*pEntry = entry;
	return OK;
}

Int BalanceRecord::fieldChangeNotify()
{
        double flval;
	Int valueChange = FALSE;
	strcpy(entry->date, fields[0]);
        entry->head = (fields[1][0]-'0')*10 + fields[1][1]-'0';
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

Int BalanceRecord::setRecordType(Int recType)
{
        if (recType!=PHYS_REC)
        {
                fields[2][0]=0;
        }
        recordType = recType;
        return OK;
}
