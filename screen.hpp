#ifndef __SCREEN__HPP__
#define __SCREEN__HPP__
#include "common.h"
#include "filebase.hpp"
#include "record.hpp"

#define SHEETMINX       10
#define SHEETMAXX       765
#define SHEETMINY	40
#define SHEETMAXY       590
#define STARTANG 270
#define ENDANG	  0
#define ARCRAD          14
#define HORIZPROP       6
#define HORIZOFF        2

#define ROWHEIGHT 18
#define TITLEX	220
#define TITLEY	10
#define STARTOFFSET 75
#define CURSOROFFSET 2
class ScreenClass
{
public:
	ScreenClass(FileClass* file);
	FileClass *file;
	Int currentRecord;
        
private:
        char copyFields[MAX_FIELDS][MAX_FIELD_LEN];
        Int copyValid;
	Int insertStatus;
	Int  readOnly;
	RecordClass *records[MAX_ROWS];
	char title[MAX_TITLE_LEN];
	Int drawFullScreen();
	Int scrollAllDown(Int count);
	Int scrollAllUp(Int count);
	Int scrollDown(Int count, Int fromRow);
        Int scrollUp(Int count, Int fromRow, Int toRow);
	Int drawCursor();
	Int unDrawCursor();
	Int drawField(Int row, Int field);
	Int drawRow(Int row);
	Int getFieldStartEndXPos(Int row, Int field, Int& startX, Int &EndX);
        Int getFieldStartEndYPos(Int row, Int& startY, Int &EndY);
public:
	Int upArrow();
	Int downArrow();
	Int leftArrow();
	Int rightArrow();
	Int pageUp();
	Int pageDown();
        Int openNewYear();
	Int home();
	Int end();
        Int ctrlend();
	Int insert();
        Int deleteRow(); /* Check*/
        Int deleteField();
        Int insertRow(); /* Check*/
        Int search(); /* Check*/
	Int tab();
        Int ctrlC();
        Int ctrlV();
	Int shifttab();
	Int save();
        Int print(); /* Check*/
        Int backSpace(); /* Check*/
	Int setActive();
	Int deactive();
	Int otherKey(Int key);
};
#endif

