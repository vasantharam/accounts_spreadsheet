#ifndef __HEADREC__
#define __HEADREC__
#include "record.hpp"

class HeadListRecord: public RecordClass
{
private:
	Int fieldValidate();
	LedgerHeadEntry far *entry;

public:
        Int fieldChangeNotify(); 
        HeadListRecord(FileClass *filearg, LedgerHeadEntry *arg);
        Int getEntry(LedgerHeadEntry ** entry);
};
#endif
