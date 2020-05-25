#ifndef __DAYFILE__HPP__
#define __DAYFILE__HPP__
#include "filebase.hpp"
class DayBookFile :public FileClass
{
private:
        double totCredit[372], totDebit[372];
	/* Current Entry. */
	DayBookEntry far *entry;
        char date[9];
        Int getCreditDebit(Int dateIndex, double& totCredit, double& totDebit);
        Int printHelper(FILE *fp, DayBookEntry *ptr, RecordClass *rec);
public:
	DayBookFile();
	~DayBookFile();
	Int getFirst(RecordClass **record);
	Int getNext(RecordClass **record);
	Int getPrev(RecordClass **record);
        Int getBlankRecord(RecordClass **record);
	Int createNewRecord(RecordClass **record);
	Int throwRecord(RecordClass *record);
        Int sortRecord(RecordClass *record);
	Int deleteRecord(RecordClass *record);
	Int fileSearch();
	Int isScreenReadOnly(Int& readOnly);
	Int print();
        Int saveRecords();
        Int recalcCreditDebit(RecordClass *record);
	Int checkTotals();
};
#endif

