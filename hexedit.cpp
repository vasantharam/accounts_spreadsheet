#include <common.h>
#include <stdio.h>
#include <stdlib.h>

main()
{
    DayBookEntry *dbMemHead, *dbPtr;
    Uint sizedb, loop;
    FILE *fpdb;
    char str[10];
    fpdb=fopen("accounts.db", "ab+");
    fseek(fpdb, 0, SEEK_END);
    sizedb = ftell(fpdb);
    fseek(fpdb,0, SEEK_SET);

    dbMemHead = (DayBookEntry *)malloc(sizedb);
    fread(dbMemHead, 1, sizedb, fpdb);
    dbMemHead->prev=0;
    for (loop=0; loop<sizedb/sizeof(DayBookEntry); loop++)
    {
        dbPtr = dbMemHead+loop;
        if (loop) dbPtr->prev = (dbPtr-1);
        dbPtr->next = (dbPtr+1);
        dbPtr->entryIndex = EXISTING_ENTRY;
        if (dbPtr->head == 0)
        {
            printf("Updating entry with Date = %s itemDesc = %s\n", dbPtr->date, dbPtr->itemDesc);
            dbPtr->head = 51;
        }
    }
    dbPtr->next = NULL;
    fclose(fpdb);
    fpdb = fopen("output.db", "wb");
    dbPtr = dbMemHead;
    while (dbPtr)
    {
	fwrite(dbPtr, sizeof(DayBookEntry), 1, fpdb);
	dbPtr = dbPtr->next;
    }
    fclose(fpdb);
}
