#include "record.hpp"
#include "headrec.hpp"
#include "common.h"
#include "errcodes.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

HeadListRecord::HeadListRecord(FileClass *filearg, LedgerHeadEntry *arg)
        :RecordClass(filearg)
{
        Int i;
        numOfFields = 4;
        for (i=0; i<numOfFields;i++)
        {
                fields[i][0] = '\0';
                fieldSizes[i] = HeadListFieldSizes[i];
        }
        recordType = PHYS_REC;
        entry = arg;
        strcpy(fields[0], entry->ledgerName);
        if (entry->headCode) sprintf(fields[1], "%d", entry->headCode);
        if (entry->pageOrderCode) sprintf(fields[2], "%d", entry->pageOrderCode);
        if (entry->ledgerType) sprintf(fields[3], "%d", entry->ledgerType);
}

Int HeadListRecord::fieldValidate()
{
	Int len = strlen(fields[currentField]);
	Int maxSize = fieldSizes[currentField];
	Int val;
        double flval;
        if (len > maxSize) return FIELD_OVERFLOW;
	len=currentX-1;
	switch(currentField)
	{
		case 0:
			/* Name */
			break;
		case 1:
			/* HeadCode*/
                        break;
		case 2:
			/* PageOrder */
                        break;
		case 3:
			/* ledType */
			if ((fields[currentField][len]<'0') && (fields[currentField][len]>'9'))
				return FIELD_INVALID;
			break;
	}
	return OK;
}
Int HeadListRecord::getEntry(LedgerHeadEntry far **pEntry)
{
	*pEntry = entry;
        return OK;
}

Int HeadListRecord::fieldChangeNotify()
{
	 strcpy(entry->ledgerName, fields[0]);
         entry->headCode = atoi(fields[1]);
         entry->pageOrderCode = atoi(fields[2]);
         entry->ledgerType = atoi(fields[3]);
         return OK;
}

