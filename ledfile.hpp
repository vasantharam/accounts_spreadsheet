#ifndef __LEDFILE__HPP__
#define __LEDFILE__HPP__
#include "filebase.hpp"
#include "headfile.hpp"
#include "headrec.hpp"

class LedgerBookFile :public FileClass
{
        Int headCode;
        Int ledgerType;
        char date[9];
        double totCredit, totDebit;
        HeadListBookFile *headFile;
        
        double plTotals[MAX_HEADS];
        
        Int plHeadCodes[MAX_HEADS];
        char plItemDesc[MAX_HEADS][MAX_TITLE_LEN];
        
public:
        LedgerBookFile(HeadListBookFile *file);
        ~LedgerBookFile();
	Int getFirst(RecordClass **record);
	Int getNext(RecordClass **record);

        Int getFirstCPL(RecordClass **record);
        Int getNextCPL(RecordClass **record);
        Int getNextGPL(RecordClass **record);

	Int getPrev(RecordClass **record);

        Int getPrevCPL(RecordClass **record);
        Int getPrevGPL(RecordClass **record);

        Int getBlankRecord(RecordClass **record);
	Int createNewRecord(RecordClass **record);
	Int throwRecord(RecordClass *record);
        Int sortRecord(RecordClass *record);
	Int deleteRecord(RecordClass *record);
	Int fileSearch();
	Int isScreenReadOnly(Int& readOnly);
        Int print();
        Int getTitle(char *title);
	Int setActive();
        Int recalcCreditDebit(RecordClass *record);
        Int checkTotals();
        Int setHeadFile(HeadListBookFile *head)
        {
            headFile = head;
            return OK;
        }
        Int getHeadFile(HeadListBookFile **head)
        {
            *head = headFile;
            return OK;
        }
};
#endif

