#include "errcodes.h"
#include "common.h"
#include <ctype.h>
#include "filebase.hpp"

Int FileClass::getTitle(char *title)
{
        Int fSize, i;
        FILE *fBal_ttl = fopen(titleFile, "rb");
	if (!fBal_ttl)
	{
                strcpy(title, defaultTitle);
		return OK;
	}
	fseek(fBal_ttl, 0, SEEK_END);
	fSize = ftell(fBal_ttl);
        fseek(fBal_ttl, 0, SEEK_SET);
        memset(title, 0, MAX_TITLE_LEN);
	fread(title, 1, (unsigned)fSize, fBal_ttl);
        for  (i=0; i<fSize; i++)
        {
                if ((!isalnum(title[i])) && (title[i]!=' ') && (title[i]!='-')
                   && (title[i]!='+') && (title[i]!='.'))
                   {
#ifdef DEBUGPRINT
                        printf("MSG:110title 0ed %d titleFile=%s title=%s", i, titleFile, title);
#endif
                        title[i] = 0;
                        break;
                   }
        }
	fclose(fBal_ttl);
	return OK;
}


Int FileClass::instantiateScreenRecords(RecordClass *records[])
{
	Int i;
        char str[MAX_TITLE_LEN];
        getTitle(str);
   	getFirst(&(records[0]));
#ifdef DEBUGPRINT
        printf("MSG:63:instd new record = %x\n", records[0]);
#endif
	for (i=1; i< MAX_ROWS; i++)
	{
		records[i] = records[i-1];
		getNext(&(records[i]));
#ifdef DEBUGPRINT
                printf("MSG:63:instd new record = %x\n", records[i]);
#endif
	}
        return OK;
}
