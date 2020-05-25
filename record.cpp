#include "record.hpp"
#include "common.h"
#include <string.h>
#include <math.h>
#include "errcodes.h"
#include <stdio.h>
#include <ctype.h>
Int RecordClass::fieldValidate()
{
	Int len = strlen(fields[currentField]);
	Int maxSize = fieldSizes[currentField];
	Int val;
        double flval;
        if (len > maxSize) return FIELD_OVERFLOW;
        len = currentX-1;
#ifdef DEBUGPRINT
        printf("MSG:93:len=%d", len);
#endif
	switch(currentField)
	{
		case 0:
			/* Date*/
			if ( (len==2) || (len==5) ) fields[currentField][len]='-';
                        else if ((fields[0][len]<'0') || (fields[0][len]>'9'))
				return FIELD_INVALID;
			if (len==1)
			{
				val = fields[0][len]-'0' + (fields[0][len-1]-'0')*10;
				if (val>31) return FIELD_INVALID;
			}
			else if (len==4)
			{
				val = fields[0][len]-'0' + (fields[0][len-1]-'0')*10;
				if (val>12) return FIELD_INVALID;
			}

			break;
		case 1:
			/* HeadCode*/
                        if ((fields[1][len]<'0') || (fields[1][len]>'9')) return FIELD_INVALID;
			break;
		case 2:
			/* Item Description */
			break;
		case 3:
			/* Varavu: (english: Credit) */
                        if (fields[3][len]!='.' && (fields[3][len]<'0') || (fields[3][len]>'9')) return FIELD_INVALID;
			break;
		case 4:
			/* Selavu (english: Debit)*/
                        if (fields[4][len]!='.' && (fields[4][len]<'0') || (fields[4][len]>'9')) return FIELD_INVALID;
			break;
	}

	return OK;
}

Int RecordClass::autoLoad(char *field, Int head)
{
        LedgerHeadEntry *entry;
        
        getHeadListEntry(head, &entry);
        if (entry)
                strcpy(field, entry->ledgerName);
        return OK;
}

Int RecordClass::otherKey(Int keyCode, Int insert)
{
        char temp[MAX_FIELD_LEN], tempchar, temp2[MAX_FIELD_LEN];
        Int maxSize = fieldSizes[currentField];
	if (recordType != PHYS_REC) return REC_READONLY;
	strcpy(temp, fields[currentField]);
        if (currentX>=maxSize)
                return OK;
#ifdef DEBUGPRINT
        printf("\nother key in Record class.key = %c\n", (char)keyCode);
#endif
        if (isalnum(keyCode) || keyCode=='.' || keyCode=='-' || keyCode ==' ' || keyCode =='\\' || keyCode =='/' || keyCode ==',' || keyCode == '&' || keyCode == '!' || keyCode == '@' || keyCode == '%' || keyCode == ';' || keyCode == ',' || keyCode == '\"' || keyCode == ':' || keyCode == '?' || keyCode == '<' || keyCode == '>' || keyCode == '+' || keyCode == '=' || keyCode == '&' || keyCode == '*' || keyCode == '$' || keyCode == '#')
	{
		currentX++;
		if (insert == INSERTON)
		{
                        if (fields[currentField][currentX-1] == '0')
                               fields[currentField][currentX] = '\0';
			fields[currentField][currentX-1] = (char)keyCode;
		}
		else
		{
#ifdef DEBUGPRINT
                        printf("\ninsert OFF.");
#endif
			tempchar = fields[currentField][currentX-1];
			fields[currentField][currentX-1] = '\0';
#ifdef DEBUGPRINT
                        printf("\nbefore sprintf in RecordClass::otherKey.");
#endif
                        sprintf(temp2, "%s",
                                 fields[currentField]);
                        sprintf(temp2+strlen(temp2), "%c", (char)keyCode);
                        sprintf(temp2+strlen(temp2), "%c", tempchar);
                        if (tempchar) sprintf(temp2+strlen(temp2), "%s", &(temp[currentX]));
                        strcpy(fields[currentField], temp2);
		}

                if (fieldValidate()!=OK)
                {
                        strcpy(fields[currentField], temp);
                        currentX--;
                        return FIELD_INVALID;
                }
#ifdef DEBUGPRINT
                printf("\nfieldValidate OK..");
#endif
                changed = TRUE;
                fieldChangeNotify();
        }
#ifdef DEBUGPRINT
        printf("\nfieldChangeNotify.curF=%s\n", fields[currentField]);
#endif

	return OK;
}
Int RecordClass::throwRecord()
{
        if (recordType==PHYS_REC) return file->throwRecord(this);
        return OK;
}
Int RecordClass::sortRecord()
{
        if (recordType==PHYS_REC) return file->sortRecord(this);
        return OK;
}
Int RecordClass::deleteRecord()
{
	if (recordType==PHYS_REC)
	{
		file->deleteRecord(this);
		return OK;
	}
        else return REC_READONLY;
}
Int RecordClass::clearRecord()
{
	Int i;
	for (i=0; i< numOfFields; i++)
	{
		fields[i][0]=0;
	}
        fieldChangeNotify();
        changed = FALSE; /* Since its a freash record now. */
	return OK;
}
Int RecordClass::moveLeft()
{
        if (currentX>=1) currentX--;
        return OK;
}
Int RecordClass::moveRight()
{
	if ((currentX < fieldSizes[currentField])
		&& (fields[currentField][currentX]))
			currentX++;
	return OK;
}
Int RecordClass::getFields(char (**argfields)[MAX_FIELDS][MAX_FIELD_LEN])
{
#ifdef DEBUGPRINT
        printf("\ngetFields start\n");
#endif
        *argfields = &fields;
	return OK;
}
Int RecordClass::getCurrentX(Int& argcurrentX)
{
	argcurrentX = currentX;
	return OK;
}
Int RecordClass::setCurrentX(Int argcurrentX)
{
	currentX = argcurrentX;
	return OK;
}
Int RecordClass::getCurrentField(Int& argcurrentField)
{
	argcurrentField = currentField;
	return OK;
}
Int RecordClass::setCurrentField(Int argcurrentField)
{
        if (currentField!=argcurrentField)
                currentX=0;
	currentField = argcurrentField;
        return OK;
}
Int RecordClass::getNumOfFields(Int& argnumOfFields)
{
	argnumOfFields = numOfFields;
	return OK;
}
Int RecordClass::getRecordType(Int& argRecordType)
{
#ifdef DEBUGPRINTER
        printf("MSG:126: getRecordType\n");
#endif
	argRecordType = recordType;
        return OK;
}

Int RecordClass::setRecordType(Int argRecType)
{
	recordType = argRecType;
        return OK;
}

Int RecordClass::getPrevRecord(RecordClass **record)
{
        *record = this;
        return file->getPrev(record);
}

Int RecordClass::getNextRecord(RecordClass **record)
{
        *record = this;
        return file->getNext(record);
}

