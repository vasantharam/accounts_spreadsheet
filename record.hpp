#ifndef __RECORD__HPP__
#define __RECORD__HPP__

/* Field len definitions */
#define MAX_FIELDS	10
#define MAX_LEN 		64
#include "errcodes.h"
#include "common.h"
#include "filebase.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class RecordClass
{

protected:
	Int recordType;
	Int numOfFields;
	Int changed;
	Int currentField;
	Int currentX;
	Int userNotify;
	/* For analytical records, only these two fields
	are displayed by default.*/
        double calculatedCredit;
        double calculatedDebit;
	char fields[MAX_FIELDS][MAX_LEN];
        class FileClass *file;

        Int autoLoad(char *headName, Int headCode);
	virtual Int fieldValidate();
        
public:
        virtual Int fieldChangeNotify(){return OK;};
	Int fieldSizes[MAX_FIELDS];
	RecordClass(FileClass *filearg)
	{
		recordType = PHYS_REC;
		changed = FALSE;
		currentField = 0;
		currentX = 0;
		file = filearg;
		userNotify = FALSE;
		calculatedCredit = 0.0;
		calculatedDebit = 0.0;
                memset(fields, 0, MAX_FIELDS*MAX_LEN);
	}
	virtual Int otherKey(Int keyCode, Int insert);
	virtual Int throwRecord();
        virtual Int sortRecord();
	virtual Int deleteRecord();
	virtual Int clearRecord();
	virtual Int moveLeft();
	virtual Int moveRight();
        virtual Int getFields(char (**fields)[MAX_FIELDS][MAX_FIELD_LEN]);
	virtual Int getCurrentX(Int& currentX);
	virtual Int setCurrentX(Int currentX);
	virtual Int getCurrentField(Int& currentField);
	virtual Int setCurrentField(Int currentField);
	virtual Int getPrevRecord(RecordClass **record);
	virtual Int getNextRecord(RecordClass **record);
	virtual Int getNumOfFields(Int& numOfFields);
	virtual Int getRecordType(Int& recordType);
	virtual Int setRecordType(Int recType);
        virtual void setCalculatedCredit(double arg){calculatedCredit=arg;sprintf(fields[3],"%9.2lf", arg);};
        virtual void setCalculatedDebit(double arg){calculatedDebit=arg;sprintf(fields[4], "%9.2lf", arg);};
        virtual void setItemDescription(char *str){sprintf(fields[2], "%s", str);}
        virtual void getCalculatedCredit(double& credit) {credit = calculatedCredit;};
        virtual void getCalculatedDebit(double& debit) {debit = calculatedDebit;};
	virtual Int setUserNotify()
	{
		userNotify = TRUE;
		return OK;
	}
        virtual Int clearUserNotify()
        {
                userNotify = FALSE;
                return OK;
        }
        virtual Int getUserNotify(Int &arg)
        {
                arg = userNotify;
                return OK;
        }
};
#endif

