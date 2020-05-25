#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "errcodes.h"
#include "inrec.hpp"
#include "wingraph.h"
#include "graphab.hpp"

char *helpText[NUM_HELPLINES] =
{
        "F1 : DayBook sheet",
        "F2 : Ledger sheet",
        "F3 : Balance sheet",
        "F4 : Header List sheet",
        "F9 : Create New year",
        "Up Arrow : To move one row above.",
        "Down Arrow : To move one row down.",
        "Left Arrow : Move cursor within current column left.",
        "Right Arrow : Move cursor within current column right.",
        "Page up : To move to multiple rows above current row",
        "Page down : To move to multiple rows below current row",
        "Home : To move the the first row, first column in current screen",
        "End : To move the the last row, first column in current screen",
        "Ctrl+End: To move to last character in current column",
        "Insert: Text overwrite or append toggle",
        "CTRL+Insert: COPY",
        "SHIFT+Insert: PASTE",
        "Delete: Delete current row PERMENANTLY",
        "CTRL + Delete :Delete current field",
        "CTRL + I: Insert a blank row",
        "CTRL + F: Search record interface",
        "tab: move to next column",
        "Shift + tab: move to prev column",
        "CTRL + S: Save Current Sheet",
        "CTRL + P: Print out to PRINTER or FILE current sheet",
        "Backspace : delete character before cursor",
        "CTRL + H: Display this help screen.",
        "daybook.ttl: Edit this file to change Daybook title",
        "Led<num>.ttl: Edit file to change Ledger <num> title"
};

Int inputBox(char *title, char *result)
{
        char (*fields)[MAX_FIELDS][MAX_LEN];
        InputBoxRecord record;
        Int key;
        Int done = FALSE;
        Int memsize;
        /* Eat the key strokes on buffer and ask
        fresh. */
        record.getFields(&fields);
        while(kbhit()) getch();
        memsize = imagesize(INBOX_MINX, INBOX_MINY, INBOX_MAXX, INBOX_MAXY);
        char *graphbuffer = (char *)malloc(memsize);
        if (!graphbuffer)
        {
#ifdef DEBUGPRINT
                printf("\nNot enough memory to display input BOX");
#endif
                return NOMEM;
        }
        getimage(INBOX_MINX, INBOX_MINY, INBOX_MAXX, INBOX_MAXY, graphbuffer);

        setcolor(BLUE);
        setfillstyle(SOLID_FILL, LIGHTBLUE);
        bar(INBOX_MINX, INBOX_MINY, INBOX_MAXX, INBOX_MAXY);

        setcolor(BLUE);
        outtextxy(INBOX_MINX+INBOX_TEXTOFFSET, INBOX_MINY+INBOX_TEXTOFFSET, "INPUT BOX:");
        outtextxy(INBOX_MINX+INBOX_TITLEOFF+INBOX_TEXTOFFSET, INBOX_MINY+INBOX_TEXTOFFSET, title);
        bar(INBOX_MINX+1, INBOX_MINY+INBOX_TITLEY, INBOX_MAXX-1, INBOX_MAXY-1);

        while(!done)
        {
                while (!kbhit());
                key = getch();
                if (key == ENTER) done = TRUE;
                if (key)
                        record.otherKey(key, INSERTON);
                else key = getch();
                if (key == LEFTKEY) record.moveLeft();
                if (key == RIGHTKEY) record.moveRight();
                outtextxy(INBOX_MINX+1, INBOX_MINY+INBOX_TITLEY, (*fields)[0]);

        }
        strcpy(result, (*fields)[0]);
        /* restore the contents of the screen back. */
    
        putimage(INBOX_MINX, INBOX_MINY, graphbuffer, COPY_PUT);
        free(graphbuffer);
	return OK;
}
Int messageBox(char *title)
{
        Int key;
        Int memsize;
        /* Eat the key strokes on buffer and ask
        fresh. */
        while(kbhit()) getch();
        memsize = imagesize(INBOX_MINX, INBOX_MINY, INBOX_MAXX, INBOX_MAXY);
        char *graphbuffer = (char *)malloc(memsize);
        if (!graphbuffer)
        {
#ifdef DEBUGPRINT
                printf("\nNot enough memory to display input BOX");
#endif
                return NOMEM;
        }
        getimage(INBOX_MINX, INBOX_MINY, INBOX_MAXX, INBOX_MAXY, graphbuffer);

        setcolor(BLUE);
        setfillstyle(SOLID_FILL, LIGHTBLUE);
        bar(INBOX_MINX, INBOX_MINY, INBOX_MAXX, INBOX_MAXY);

        outtextxy(INBOX_MINX, INBOX_MINY, "Message Box:");
        bar(INBOX_MINX+1, INBOX_MINY+INBOX_TITLEY, INBOX_MAXX-1, INBOX_MAXY-1);
        outtextxy(INBOX_MINX+1, INBOX_MINY+INBOX_TITLEY, title);
        key = 0;
        while (!isalnum(key) && key!=13 && key!=' ')
        {
                while (!kbhit());
                while(kbhit()) key = getch();
        }
        /* restore the contents of the screen back. */

        putimage(INBOX_MINX, INBOX_MINY, graphbuffer, COPY_PUT);
        free(graphbuffer);
	return OK;
        
}

Int helpScreen()
{
        Int key, i;
        Int memsize;
        /* Eat the key strokes on buffer and ask
        fresh. */
        while(kbhit()) getch();
        memsize = imagesize(HELPBOX_MINX, HELPBOX_MINY, HELPBOX_MAXX, HELPBOX_MAXY);
        char *graphbuffer = (char *)malloc(memsize);
        if (!graphbuffer)
        {
#ifdef DEBUGPRINT
                printf("\nNot enough memory to display input BOX");
#endif
                return NOMEM;
        }
        getimage(HELPBOX_MINX, HELPBOX_MINY, HELPBOX_MAXX, HELPBOX_MAXY, graphbuffer);

        setcolor(BLUE);
        setfillstyle(SOLID_FILL, LIGHTBLUE);
        bar(HELPBOX_MINX, HELPBOX_MINY, HELPBOX_MAXX, HELPBOX_MAXY);

        outtextxy(HELPBOX_MINX, HELPBOX_MINY, "Help Screen:");
        bar(HELPBOX_MINX+1, HELPBOX_MINY+HELPBOX_TITLEY, HELPBOX_MAXX-1, HELPBOX_MAXY-1);
        for (i=0; i< NUM_HELPLINES; i++)
                outtextxy(HELPBOX_MINX+1, HELPBOX_MINY+HELPBOX_TITLEY*(i+2), helpText[i]);

        key = 0;
        while (!isalnum(key) && key!=13 && key!=' ')
        {
                while (!kbhit());
                while(kbhit()) key = getch();
        }


        putimage(HELPBOX_MINX, HELPBOX_MINY, graphbuffer, COPY_PUT);
#ifdef DEBUGPRINT
        printf("MSG:70:After putimage\n");
#endif
        free(graphbuffer);
	return OK;        
}

