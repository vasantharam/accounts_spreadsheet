#ifndef __GRAPHAB__H__
#define __GRAPHAB__H__
#define INBOX_MINX 80
#define INBOX_MINY 225
#define INBOX_MAXX 560
#define INBOX_MAXY 255
#define INBOX_TITLEY 10
#define INBOX_TITLEOFF 80
#define INBOX_TEXTOFFSET 2

#define NUM_HELPLINES   29

#define HELPBOX_MINX 100
#define HELPBOX_MINY 90
#define HELPBOX_MAXX 640
#define HELPBOX_MAXY 410
#define HELPBOX_TITLEY 10
#define HELPBOX_TITLEOFF 100
#define HELPBOX_TEXTOFFSET 2


Int inputBox(char *title, char *resultStr);
Int messageBox(char *title);
Int helpScreen();
#endif
