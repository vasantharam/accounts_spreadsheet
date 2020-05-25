#ifndef __HEADFILE__HPP__
#define __HEADFILE__HPP__
#include "filebase.hpp"
class HeadListBookFile :public FileClass
{
public:
	HeadListBookFile();
	Int getFirst(RecordClass **record);
	Int getNext(RecordClass **record);
	Int getPrev(RecordClass **record);
        Int getBlankRecord(RecordClass **record);
	Int throwRecord(RecordClass *record);
        Int sortRecord(RecordClass *record);
	Int deleteRecord(RecordClass *record);
	Int fileSearch();
	Int isScreenReadOnly(Int& readOnly);
        Int print();
};
#endif

