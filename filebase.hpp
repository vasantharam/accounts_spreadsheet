#ifndef __FILEBASE__HPP__
#define __FILEBASE__HPP__
#include "record.hpp"
#include "common.h"
#include <string.h>
#include "fileman.hpp"
#include <stdio.h>
class RecordClass;
class FileClass
{
protected:
        /* Lets not allow saveRecords to be called by 'others',
        since it should be called only from the destructor and only
        once. */
        char titleFile[FILENAME_LEN];
        char defaultTitle[MAX_TITLE_LEN];

public:
        FileClass()
        {
                titleFile[0] = 0;
                strcpy(defaultTitle, "No title");
        }
	~FileClass()
	{
                fileManCleanUp();
	}
	virtual Int getFirst(RecordClass **record) = 0;
	virtual Int getNext(RecordClass **record) = 0;
	virtual Int getPrev(RecordClass **record) = 0;
        virtual Int getBlankRecord(RecordClass **record) = 0;
        virtual Int createNewRecord(RecordClass **record){return OK;};
        virtual Int saveRecords() {return OK;};
        virtual Int throwRecord(RecordClass *record){return OK;};
        virtual Int sortRecord(RecordClass *record){return OK;};
        virtual Int deleteRecord(RecordClass *record){return OK;};
	virtual Int fileSearch()=0;
        virtual Int setCurrentRecord(RecordClass *record){return OK;};
	/* Should load respective to current screen */
	virtual Int instantiateScreenRecords(RecordClass *records[]);
	virtual Int isScreenReadOnly(Int& readOnly)=0;
        virtual Int getTitle(char *title);
        virtual Int setActive()
        {
#ifdef DEBUGPRINT
            printf("set active in base\n");
#endif
            return RELOAD_RECORDS;
        };
        virtual Int recalcCreditDebit(RecordClass *record){return OK;};
	virtual Int print()=0;
	virtual Int checkTotals(){return OK;};
};
#endif


