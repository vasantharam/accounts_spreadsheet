#include "screen.hpp"
#include "balfile.hpp"
#include "errcodes.h"
#include "common.h"
#include "wingraph.h"
#include "graphab.hpp"
#include "generic.hpp"

#define DRAWUNDRAW(color) \
{\
	Int width; \
	Int startX, endX, startY, endY;\
	char curChar;\
	char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];\
	Int currentField;\
	Int currentX;\
	records[currentRecord]->getCurrentField(currentField);\
	records[currentRecord]->getCurrentX(currentX);\
        records[currentRecord]->getFields(&fields);     \
	curChar = (*fields)[currentField][currentX];\
	(*fields)[currentField][currentX]=0;\
	width = textwidth((*fields)[currentField]);\
	(*fields)[currentField][currentX]=curChar;\
	getFieldStartEndXPos(currentRecord, currentField, startX, endX);\
        getFieldStartEndYPos(currentRecord, startY, endY);\
	setcolor(color);\
        line(startX+width+CURSOROFFSET, startY+1,\
		startX+width+CURSOROFFSET,\
                startY+ROWHEIGHT-2);\
        if (insertStatus)\
        {\
                setfillstyle(SOLID_FILL, color);\
                bar(startX+width+CURSOROFFSET, startY+1,\
                        startX+width+CURSOROFFSET+HORIZPROP,\
                        startY+ROWHEIGHT-2);\
        }\
	return OK;\
}

ScreenClass::ScreenClass(FileClass *arg)
{
	file=arg;
#ifdef DEBUGPRINT
        printf("MSG:6:Trying record instantiation\n");
#endif
        file->getTitle(title);
	file->instantiateScreenRecords(records);
#ifdef DEBUGPRINT
        printf("After instantiation in ScreenClass\n");
#endif
	currentRecord=0;
	insertStatus = INSERTOFF;
	file->isScreenReadOnly(readOnly);

#ifdef DEBUGPRINT
        printf("MSG:601: %s is the title\n", title);
#endif
        copyValid = FALSE;
	return;
}
Int ScreenClass::drawFullScreen()
{
	int loop;
	/* Later check if we can live away with doing cleardevice. */

        setcolor(WHITE);
	setfillstyle(SOLID_FILL, WHITE);
        bar(5, SHEETMINY, SHEETMAXX, SHEETMAXY);
        

        setbkcolor(WHITE);
        setcolor(DARKGRAY);
        bar(SHEETMINX-5, TITLEY, SHEETMAXX, TITLEY+20);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(SHEETMINX-5, TITLEY,title);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);

        setcolor(YELLOW);
        line(5,5,5,5);
	for (loop=0; loop<ARCRAD; loop++)
        {

                arc(6, SHEETMINY, STARTANG, ENDANG, loop);
        }
	for (loop=0; loop<MAX_ROWS; loop++)
	{
		drawRow(loop);
	}
        drawField(0,0);
	return OK;
}
Int ScreenClass::getFieldStartEndXPos(Int row, Int field, Int& startX, Int &endX)
{
	Int totSize=0, loop;
	for (loop=0; loop<field; loop++)
	{
                totSize+=records[row]->fieldSizes[loop]+1;
	}
        startX = SHEETMINX + totSize *HORIZPROP;
        endX = startX + (records[row]->fieldSizes[field]+1) * HORIZPROP;
	return OK;
}
Int ScreenClass::getFieldStartEndYPos(Int row, Int& startY, Int &endY)
{
        startY = (row)*ROWHEIGHT + STARTOFFSET;
        endY = (row+1)*ROWHEIGHT + STARTOFFSET;
	return OK;
}
Int ScreenClass::drawField(Int row, Int field)
{
	Int startX, endX, startY, endY;
	Int currentField;
        Int recType, color, notify;
	char (*fieldEntry)[MAX_FIELDS][MAX_FIELD_LEN];
        records[row]->getRecordType(recType);
        records[row]->getUserNotify(notify);
#ifdef DEBUGPRINT
	printf("Draw Field start\n");
#endif
	getFieldStartEndXPos(row, field, startX, endX);
        getFieldStartEndYPos(row, startY, endY);
#ifdef DEBUGPRINT
        printf("startX=%d, startY=%d, endX=%d, endY=%d\n", startX, startY, endX, endY);
#endif

	if (row == currentRecord)
	{
		records[row]->getCurrentField(currentField);
		if (currentField == field) setcolor(MAGENTA);
                else setcolor(LIGHTGRAY);
        } else setcolor(LIGHTGRAY);
	rectangle(startX, startY, endX, endY);
        setbkcolor(WHITE);
        setcolor(WHITE);
	setfillstyle(SOLID_FILL, WHITE);
        rectangle(startX+1, startY+1, endX-2, endY-2);
        bar(startX+1, startY+1, endX-2, endY-2);
#ifdef DEBUGPRINT
	printf("records[%d]=%x", row, records[row]);
#endif
        if ((recType == PHYS_REC) || (recType >= PL_REC1))
                color = DARKGRAY;
        else if (!notify)
                color = RED;
        else color = GREEN;

        setfillstyle(SOLID_FILL, color);
        setcolor(color);

	records[row]->getFields(&fieldEntry);
        outtextxy(startX+2, startY+2, (*fieldEntry)[field]);


	return OK;
}

Int ScreenClass::drawRow(Int row)
{
	Int currentField, loop;
	Int numFields;
	records[row]->getNumOfFields(numFields);
        setcolor(DARKGRAY);
/*        rectangle(SHEETMINX,
			(row) * ROWHEIGHT + STARTOFFSET,
			 SHEETMAXX,
                        (row+1) * ROWHEIGHT + STARTOFFSET);*/
	for (loop=0; loop<numFields; loop++)
	{
		drawField(row, loop);
	}
	return OK;
}

Int ScreenClass::scrollDown(Int count, Int fromRow)
{
        Int i, ret;
        Int curField;
        records[currentRecord]->getCurrentField(curField);
	RecordClass *outGoingRecord;
	RecordClass *newRecord;
	unDrawCursor();
	do
	{
		if (fromRow==0)
		{                        
                        ret = records[0]->getPrevRecord(&newRecord);
                        if (ret!=OK) break;
                        newRecord->setCurrentField(curField);
		}
                outGoingRecord = records[MAX_ROWS-1];
                for (i=MAX_ROWS-1; i>fromRow; i--)
		{
			records[i] = records[i-1];
			drawRow(i);
		}
		if (fromRow==0)
		{

#ifdef DEBUGPRINT
                        printf("MSG:47:updating records[0] with newRecord.\n");
#endif
			records[0] = newRecord;
                        drawRow(0);
			/* Point to the record on cursor, correctly,
                        since it has scrolled.
                        currentRecord++;*/
		}
		outGoingRecord->throwRecord();
		delete outGoingRecord;
        } while(--count);
#ifdef DEBUGPRINT
        printf("scrollDown just before drawCursor()\n");
#endif
	drawCursor();
	return OK;
}

Int ScreenClass::scrollUp(Int count, Int fromRow, Int toRow)
{
        int i, ret;
	RecordClass *outGoingRecord;
	RecordClass *newRecord;
        Int currentField;
	unDrawCursor();
        while(count--)
	{
		if (fromRow == MAX_ROWS-1)
		{
#ifdef DEBUGPRINT
                        printf("MSG:18:scrollUp:before getNextRecord\n");
#endif
                        ret = records[fromRow]->getNextRecord(&newRecord);
                        records[currentRecord]->getCurrentField(currentField);
                        newRecord->setCurrentField(currentField);

                        if (ret!=OK) break;
		}

                outGoingRecord = records[toRow];
                for (i=toRow; i<fromRow; i++)
		{
			records[i] = records[i+1];
			drawRow(i);
		}
		if (fromRow == MAX_ROWS-1)
		{
                        
                        records[MAX_ROWS-1] = newRecord;
                        drawRow(MAX_ROWS-1);
		}
                outGoingRecord->throwRecord();
                delete outGoingRecord;
        };
	drawCursor();
#ifdef DEBUGPRINT
        if (!newRecord)
                printf("MSG:12:nULL newRecord");
        else
                printf("MSG:12:newRecord OK");
#endif
	return OK;
}

Int ScreenClass::scrollAllDown(Int count)
{
	scrollDown(count, 0);
	return OK;
}

Int ScreenClass::scrollAllUp(Int count)
{
#ifdef DEBUGPRINT
        printf("\nMSG:17:calling scrollUP.\n");
#endif
        scrollUp(count, MAX_ROWS-1, 0);
        return OK;
}

Int ScreenClass::drawCursor()
{
        Int currentField;
        records[currentRecord]->getCurrentField(currentField);
        drawField(currentRecord, currentField);
        DRAWUNDRAW(MAGENTA);
}

Int ScreenClass::unDrawCursor()
{
        Int currentField;
        records[currentRecord]->getCurrentField(currentField);
        currentRecord++;
        drawField(currentRecord-1, currentField);
        currentRecord--;
//        DRAWUNDRAW(WHITE);
        return OK;
}

Int ScreenClass::upArrow()
{
	Int currentField;
	Int currentX;
	records[currentRecord]->getCurrentField(currentField);
	records[currentRecord]->getCurrentX(currentX);
	unDrawCursor();
#ifdef DEBUGPRINT
        printf("MSG:31:cursor undrawn");
#endif
	if (currentRecord == 0)
        {
            scrollAllDown(1);
        }
	else currentRecord--;
#ifdef DEBUGPRINT
        printf("MSG:31:after if cur scrollAlldown\n");
#endif

	records[currentRecord]->setCurrentField(currentField);
	records[currentRecord]->setCurrentX(currentX);
	drawCursor();
	return OK;
}

Int ScreenClass::downArrow()
{
	Int currentField;
	records[currentRecord]->getCurrentField(currentField);
	unDrawCursor();
#ifdef DEBUGPRINT
        printf("MSG:16: downArrow");
#endif
	if (currentRecord == MAX_ROWS-1) scrollAllUp(1);
	else currentRecord++;
	records[currentRecord]->setCurrentField(currentField);
#ifdef DEBUGPRINT
        printf("MSG:17: downArrow: start of drawCur");
#endif

	drawCursor();
	return OK;
}

Int ScreenClass::leftArrow()
{
	unDrawCursor();
	records[currentRecord]->moveLeft();
	drawCursor();
	return OK;
}

Int ScreenClass::rightArrow()
{
	unDrawCursor();
	records[currentRecord]->moveRight();
	drawCursor();
	return OK;
}

Int ScreenClass::pageUp()
{
	Int i;
	for (i=0; i< MAX_ROWS/3; i++)
		upArrow();
	return OK;
}

Int ScreenClass::pageDown()
{
	Int i;
	for (i=0; i< MAX_ROWS/3; i++)
		downArrow();
	return OK;
}
Int ScreenClass::openNewYear()
{
        ((BalanceBookFile *)file)->openNewYear();
        return OK;
}
Int ScreenClass::home()
{
	unDrawCursor();
	currentRecord = 0;
	records[0]->setCurrentField(0);
	records[0]->setCurrentX(0);
	drawCursor();
	return OK;
}

Int ScreenClass::end()
{
	unDrawCursor();
	currentRecord = MAX_ROWS-1;
	records[0]->setCurrentField(0);
	records[0]->setCurrentX(0);
	drawCursor();
	return OK;
}

Int ScreenClass::ctrlend()
{
        char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];
        Int pos, currentField;
        records[currentRecord]->getFields(&fields);
        records[currentRecord]->getCurrentField(currentField);

        pos = strlen((*fields)[currentField]);

	unDrawCursor();
        records[currentRecord]->setCurrentX(pos);
	drawCursor();
	return OK;
}
Int ScreenClass::ctrlC()
{
        Int i;
        char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];
#ifdef DEBUGPRINT
        printf("MSG:101:ctrlC hit\n");
#endif
        copyValid = TRUE;
        records[currentRecord]->getFields(&fields);
        for (i=0; i<MAX_FIELDS; i++)
        {
                strcpy(copyFields[i], (*fields)[i]);
        }
        return OK;
}                
Int ScreenClass::ctrlV()
{
        Int i, recType;
        if (!copyValid) return OK;
        records[currentRecord]->getRecordType(recType);
        if ((recType != PHYS_REC) || readOnly)
        {
                messageBox("Unable to paste. readonly record or page.");
                return READONLY;
        }

#ifdef DEBUGPRINT
        printf("MSG:100:Pasting\n");
#endif
        unDrawCursor();

        char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];
        copyValid = TRUE;
        records[currentRecord]->getFields(&fields);
        for (i=0; i<MAX_FIELDS; i++)
        {
                strcpy((*fields)[i], copyFields[i]);
        }
        drawRow(currentRecord);
        records[currentRecord]->setCurrentField(0);
        records[currentRecord]->setCurrentX(0);
        records[currentRecord]->fieldChangeNotify();
        drawCursor();
        return OK;
}

Int ScreenClass::insert()
{
        unDrawCursor();

	if (insertStatus == INSERTON)
		insertStatus = INSERTOFF;
	else
		insertStatus = INSERTON;
        drawCursor();
        return OK;
}
Int ScreenClass::deleteField()
{
        Int currentField;
        char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];
        if (readOnly)
        {
                messageBox("Cannot delete READONLY");
                return OK;
        }
        records[currentRecord]->getCurrentField(currentField);
        records[currentRecord]->getFields(&fields);
        (*fields)[currentField][0] = 0;
        records[currentRecord]->fieldChangeNotify();
        drawField(currentRecord, currentField);
        return OK;
}
Int ScreenClass::deleteRow()
{
        char input[MAX_TITLE_LEN];
        Int count = 1;
        Int tempCurrent;
        Int recType, i;
        if (currentRecord == MAX_ROWS-1) return OK;
        inputBox("Are you sure about DELETING current Record", input);
        if ( (input[0] != 'y' && input[0] != 'Y')
                ||
             (input[1] != 'e' && input[1] != 'E')
                ||
             (input[2] != 's' && input[2] != 'S')
            )
        {
                return OK;
        }
        if (readOnly)
        {
                messageBox("Unable to delete. Screen readonly.");
                return READONLY;
        }
        records[currentRecord]->getRecordType(recType);
        if (recType != PHYS_REC)
        {
                messageBox("Unable to delete. Record readonly.");
                return READONLY;
        }

	unDrawCursor();
        tempCurrent=currentRecord;
        records[tempCurrent++]->clearRecord();
#ifdef DEBUGPRINT
        printf("MSG115:clear record\n");
#endif

        records[tempCurrent]->getRecordType(recType);
        if ((recType == ANAL_REC) || (recType == ANAL_REC2))
        {

                records[tempCurrent++]->clearRecord();
                count++;
                records[tempCurrent]->getRecordType(recType);
                if ((recType == ANAL_REC) || (recType == ANAL_REC2))
                {
                        records[tempCurrent++]->clearRecord();
                        count++;
                }
        }
#ifdef DEBUGPRINT
        printf("MSG:111: just before scroll in delete.\n");
#endif
        scrollUp(count, MAX_ROWS-1, currentRecord);

	drawCursor();
	return OK;
}

Int ScreenClass::insertRow()
{
	Int ret;
        if (readOnly)
        {
                messageBox("Unable to insert row. Screen readonly.");
                return READONLY;
        }
	unDrawCursor();
        if (currentRecord == 0) downArrow();
	scrollDown(1, currentRecord);
        file->getBlankRecord(&(records[currentRecord]));
	drawRow(currentRecord);
	drawCursor();
	return OK;
}

Int ScreenClass::search()
{
	Int ret, i;
	ret = file->fileSearch();
	if (ret==OK)
	{
		for (i=0; i< MAX_ROWS; i++)
                {
			records[i]->throwRecord();
			delete records[i];
                }
	}
#ifdef DEBUGPRINT
        printf("MSG:90:out of deletion of old records.\n");
#endif
        file->instantiateScreenRecords(records);
	currentRecord=0;
	drawFullScreen();
	return OK;
}

Int ScreenClass::tab()
{
	Int currentField;
	Int numFields;
        unDrawCursor();
#ifdef DEBUGPRINT
        printf("tab\n");
#endif
	records[currentRecord]->getCurrentField(currentField);
	records[currentRecord]->getNumOfFields(numFields);
        if (currentField==(numFields-1))
	{
                records[currentRecord]->setCurrentField(0);
                currentField = 0;
                if (currentRecord<(MAX_ROWS-1))
		{
			currentRecord++;
		}
		else downArrow();
                
	}
	else currentField++;
	records[currentRecord]->setCurrentField(currentField);
#ifdef DEBUGPRINT
        printf("tab end\n");
#endif

        drawCursor();
	return OK;
}
Int ScreenClass::shifttab()
{
	Int currentField;
	Int numFields;
        unDrawCursor();
#ifdef DEBUGPRINT
        printf("currentRecord = %d", currentRecord);
#endif
	records[currentRecord]->getCurrentField(currentField);
	records[currentRecord]->getNumOfFields(numFields);
	if (currentField==0)
	{
		currentField=numFields-1;
                records[currentRecord]->setCurrentField(currentField);
                if (currentRecord>=1)
		{
			currentRecord--;
		}
		else upArrow();
	}
	else currentField--;
	records[currentRecord]->setCurrentField(currentField);
        drawCursor();
	return OK;
}
Int ScreenClass::save()
{
        Int i;
#ifdef DEBUGPRINT
        printf("MSG:61:start of save.\n");
#endif
        for (i=0; i<MAX_ROWS; i++)
        {
                if (records[i])
                {
#ifdef DEBUGPRINT
#endif
                        records[i]->sortRecord();
                }
        }
        file->saveRecords();
#ifdef DEBUGPRINT
        printf("MSG:58:End of save\n");
#endif
	return OK;
}
Int ScreenClass::print()
{
#ifdef DEBUGPRINT
        printf("MSG:71: print hit\n");
#endif
	file->print();
	return OK;
}
Int ScreenClass::backSpace()
{
	Int currentX;
	Int currentField;
        Int recordType;
	char temp[MAX_FIELD_LEN];
	char (*fields)[MAX_FIELDS][MAX_FIELD_LEN];
        /* This is bad programming, backSpace is implemented
        on screen, but otherKey or other functons on the record.*/
        records[currentRecord]->getRecordType(recordType);
        if (recordType != PHYS_REC)
        {
            return OK;    
        }
        if (readOnly)
        {
                messageBox("Unable to delete. screen readonly. ");
                return READONLY;
        }

#ifdef DEBUGPRINT
        printf("MSG:69:Back space hit\n");
#endif
	records[currentRecord]->getCurrentX(currentX);
	records[currentRecord]->getCurrentField(currentField);
        records[currentRecord]->getFields(&fields);
	if (currentX>0)
		currentX--;
	else
                return OK;
        unDrawCursor();
	records[currentRecord]->setCurrentX(currentX);
        strcpy(&((*fields)[currentField][currentX]),  &((*fields)[currentField][currentX+1]));

        records[currentRecord]->fieldChangeNotify();

        drawField(currentRecord, currentField);
        
        drawCursor();
	return OK;
}
Int ScreenClass::setActive()
{
#ifdef DEBUGPRINT
        printf("setActive()\n");
#endif
#ifdef DEBUGLINKEDLIST
        void DisplayLinkedList();
        DisplayLinkedList();
#endif
        copyValid = FALSE;
#ifdef DEBUGPRINT
        printf("debug before setActive file is %x ", file);
#endif
        if (file) file->setActive();

        /* We are reloading irrespective of whether,
        RELOAD_REC is the ret type, since deactivate throws records
        away.*/
        {
#ifdef DEBUGPRINT
                printf("MSG:91: before instantiate again in setAct\n");
#endif

                /* Reinstantiation */
                file->instantiateScreenRecords(records);
                currentRecord=0;
                file->getTitle(title);
        }
	drawFullScreen();
	return OK;
}
Int ScreenClass::deactive()
{
        Int i;
        copyValid = FALSE;        
        for (i=0; i<MAX_ROWS ; i++)
        {
                if (records[i])
                        records[i]->throwRecord();
                delete records[i];
                records[i]=NULL;
        }
	return OK;
}
Int ScreenClass::otherKey(Int key)
{
#ifdef DEBUGPRINT
        printf("Calling record other key.");
#endif
        Int currentField, recordType;
        Int i;

        records[currentRecord]->getRecordType(recordType);
        if (recordType != PHYS_REC)
            return OK;
        records[currentRecord]->getCurrentField(currentField);
        unDrawCursor();
	records[currentRecord]->otherKey(key, insertStatus);
        if (currentField == 1) drawField(currentRecord, currentField+1);
                for (i=0; i<MAX_ROWS; i++)
                {
                        records[i]->getRecordType(recordType);
#ifdef DEBUGDYNAMIC
                        printf("\nbefore if PHYS check\n");
#endif

                        if (recordType != PHYS_REC)
                        {
#ifdef DEBUGDYNAMIC
                                printf("\nbefore recalc\n");
#endif
                                file->recalcCreditDebit(records[i]);
                                drawRow(i);
                        }
                }
        drawCursor();
	return OK;
}

