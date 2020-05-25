#include "record.hpp"

class BalanceRecord: public RecordClass
{
private:
        BalanceEntry far *entry;


public:
        Int fieldChangeNotify();
        BalanceRecord(FileClass *filearg, BalanceEntry *entry);
        Int getEntry(BalanceEntry far**entry);
        Int setRecordType(Int recType);
};

