#include "headfile.hpp"
#include "headrec.hpp"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "errcodes.h"
#include "fileman.hpp"
#include "graphab.hpp"

HeadListBookFile::HeadListBookFile()
{
        strcpy(titleFile, "headlist.ttl");
        strcpy(defaultTitle, "Header List");
}
Int HeadListBookFile::getBlankRecord(RecordClass **record)
{
        LedgerHeadEntry *entry;
        allocNewEntry(&entry);
        insertEntry(entry);
        *record = new HeadListRecord(this, entry);
        return OK;
}
Int HeadListBookFile::getFirst(RecordClass **record)
{
        LedgerHeadEntry *entry = NULL;
#ifdef DEBUGPRINT
        printf("MSG:71:before getFirst entry\n");
#endif
        getFirstEntry(&entry);
        if (entry == NULL)
        {
#ifdef DEBUGPRINT
                printf("MSG:72: NO ENTRY in HEADLIST.\n");
#endif

                allocNewEntry(&entry);
#ifdef DEBUGPRINT
                printf("MSG:74: LedgerHeadListEntry alloc done\n");
#endif

                insertEntry(entry);
#ifdef DEBUGPRINT
                printf("MSG:75: LedgerHeadListEntry inserted.\n");
#endif

        }
#ifdef DEBUGPRINT
                printf("MSG:73: Ready to create HeadListRecord.n");
#endif

        *record = new HeadListRecord(this, entry);        
	return OK;
}
Int HeadListBookFile::getNext(RecordClass **record)
{
        LedgerHeadEntry *entry = NULL;
        ((HeadListRecord *)(*record))->getEntry(&entry);
        getNextEntry(&entry);
        if (entry == NULL)
        {
                allocNewEntry(&entry);
                insertEntry(entry);
        }
        *record = new HeadListRecord(this, entry);

	return OK;
}
Int HeadListBookFile::getPrev(RecordClass **record)
{
        LedgerHeadEntry *entry = NULL;
        ((HeadListRecord *)(*record))->getEntry(&entry);
        getPrevEntry(&entry);
        if (entry == NULL)
        {
                return NOTHING_TO_SEARCH;
        }
        *record = new HeadListRecord(this, entry);
	return OK;
}
Int HeadListBookFile::throwRecord(RecordClass *record)
{
        LedgerHeadEntry *entry = NULL;
        ((HeadListRecord *)record)->getEntry(&entry);
        if (!entry->headCode)
        {
#ifdef DEBUGPRINT
                printf("MSG:71:not writing record %s\n", entry->ledgerName);
#endif
                deleteEntry(entry);
                if (entry->entryIndex == NEW_ENTRY)
                        deallocEntry(entry);
        }
        else if (entry->entryIndex == NEW_ENTRY)
        {
#ifdef DEBUGPRINT
                printf("MSG:71:sorting entry in throw %s\n", entry->ledgerName);
#endif

                /* Deletion and readdition sorts the entry*/
                deleteReadd(entry);
        }
	return OK;
}
Int HeadListBookFile::sortRecord(RecordClass *record)
{
        LedgerHeadEntry *entry = NULL;
        ((HeadListRecord *)record)->getEntry(&entry);
        if (entry->headCode && (entry->entryIndex == NEW_ENTRY))
        {
#ifdef DEBUGPRINT
                printf("MSG:71:sorting entry in sort %s\n", entry->ledgerName);
#endif

                /* Deletion and readdition sorts the entry*/
                deleteReadd(entry);
        }

	return OK;
}

Int HeadListBookFile::deleteRecord(RecordClass *record)
{
        LedgerHeadEntry *entry = NULL;
        ((HeadListRecord *)record)->getEntry(&entry);
        deleteEntry(entry);
        if (entry->entryIndex == NEW_ENTRY)
            deallocEntry(entry);
	return OK;
}
Int HeadListBookFile::fileSearch()
{
        messageBox("No search in head list in this version.\n");
	return OK;
}

Int HeadListBookFile::isScreenReadOnly(Int& readOnly)
{
	readOnly = FALSE;
	return OK;
}
Int HeadListBookFile::print()
{
        messageBox("No PRINT option for head list sheet.\n");
	return OK;
}

