#include "record.hpp"

class LedgerRecord: public RecordClass
{
private:
	DayBookEntry far *entry;

public:
        Int fieldChangeNotify();
        LedgerRecord(FileClass *argfile, DayBookEntry far *arg);
	Int getEntry(DayBookEntry far ** entry);
        Int setRecordType(Int recType);
};

