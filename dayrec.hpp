#include "record.hpp"
#include <string.h>

class DayBookRecord: public RecordClass
{
private:
        char date[9];
	DayBookEntry far *entry;
        Int fieldValidate();

public:
        Int fieldChangeNotify();
        DayBookRecord(FileClass *filearg, DayBookEntry far *arg);
	Int getEntry(DayBookEntry far**entry);
        Int setRecordType(Int recType);
};

