#ifndef __INREC__HPP__
#define __INREC__HPP__

#include "errcodes.h"
#include "common.h"
#include "filebase.hpp"
#include "record.hpp"
#include <stdio.h>

class InputBoxRecord: public RecordClass
{
public:
        InputBoxRecord()
                :RecordClass(NULL)
	{
                Int i;
                numOfFields = 1;
                fieldSizes[0] = MAX_INBOX_FIELD_SIZE;
	}
        Int fieldValidate() {return OK;}
};
#endif

