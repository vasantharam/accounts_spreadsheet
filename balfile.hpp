#ifndef __BALFILE__HPP__
#define __BALFILE__HPP__
#include "filebase.hpp"
#include "common.h"



class BalanceBookFile :public FileClass
{
private:
        BalanceEntry *entryHead;
        Int getBlankEntry(BalanceEntry **entry);
        Int getTotCredit(double& totCredit);
        Int getTotDebit(double& totDebit);
        Int addEntry(BalanceEntry *entry);
        Int freeAll();
public:
	BalanceBookFile();
        ~BalanceBookFile();
        Int openNewYear();
	Int getFirst(RecordClass **record);
	Int getNext(RecordClass **record);
	Int getPrev(RecordClass **record);
        Int getBlankRecord(RecordClass **record);
	Int fileSearch();
	Int isScreenReadOnly(Int& readOnly);
        Int print();
        Int checkTotals();
        Int setActive(){checkTotals(); return OK;}
};
#endif

