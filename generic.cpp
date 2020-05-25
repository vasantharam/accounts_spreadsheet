#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wingraph.h"
#include "generic.hpp"
#include "errcodes.h"
#include "screen.hpp"
#include "dayfile.hpp"
#include "ledfile.hpp"
#include "balfile.hpp"
#include "headfile.hpp"
#include "graphab.hpp"

Int dayBookFieldSizes[]={11, 3, 80, 13, 13};
Int HeadListFieldSizes[]={30, 3, 7, 11};
extern int ctrlOn;
extern Int shiftOn;
main()
{
	char *bgiPath;
        FILE *fp;
        HeadListBookFile *headFile;
        LedgerBookFile *ledFile;
        DayBookFile *dayFile;
        int gd=DETECT, gm=DETECT;
        char input[MAX_TITLE_LEN], cmd[MAX_TITLE_LEN];
	bgiPath=getenv("BGIPATH");
	if (!bgiPath) bgiPath=BGIPATH;
	initgraph(&gd, &gm, bgiPath);

#ifdef DEBUGPRINT
        printf("gd=%d\n", gd);
#endif
        fileManInitialize();
	ScreenClass *screenObj[4];
        dayFile = new DayBookFile;
#ifdef DEBUGPRINT
        printf("dayFile = %x", dayFile);
#endif
        screenObj[DAYBOOK]=new ScreenClass(dayFile);
#ifdef DEBUGPRINT
        printf("MSG:601:DayBook success\n");
#endif
        headFile = new HeadListBookFile;
        screenObj[HEADLIST] =new ScreenClass(headFile);

        ledFile = new LedgerBookFile(headFile);
        screenObj[LEDGER] =new ScreenClass(ledFile);
#ifdef DEBUGPRINT
        printf("MSG:601:Ledger success\n");
#endif
	screenObj[BALANCESHEET] =new ScreenClass(new BalanceBookFile);
#ifdef DEBUGPRINT
        printf("MSG:601:Balance open success\n");
#endif

      

#ifdef DEBUGPRINT
        printf("MSG:601:Head List open success\n");
#endif

#ifdef DEBUGPRINT
        printf("\nMSG:601:Application start\n");
#endif

	runGenericInterface(screenObj, 4);
	return 0;
}

Int runGenericInterface(ScreenClass* screenObj[], Int numOfScreens)
{
        Int key, keyStatus, control;
        char ascii;
	Int exitApp = FALSE;
	Int currentScreen = DAYBOOK;
	Int i;
#ifdef DEBUGPRINT
	printf("Before setActive\n");
#endif
	screenObj[currentScreen]->setActive();
#ifdef DEBUGPRINT
        printf("After set Active\n");
#endif
        /* Flush keys on buffer before start.*/
        while(kbhit()) getch();
	while(!exitApp)
	{
#ifdef DEBUGKEY
                printf("before kbhit\n");
#endif
              //  while(!kbhit());
                ((char *)(&key))[0] = getch();
                if (((char *)(&key))[0]==0) ((char *)(&key))[1] = getch();
                else ((char *)(&key))[1]=0;
#ifdef DEBUGKEY
                printf("key[0]=%d\n", ((char *)(&key))[0]);
                printf("key[1]=%d\n", ((char *)(&key))[1]);
                continue;
#endif

                ascii = ((char *)(&key))[0];
		control = ((char *)(&key))[1];
		if (ascii)
		{
			/*Alphanumeric, or special character, not a
			control character. */
			switch(ascii)
			{
				case TABASCII:
                                        screenObj[currentScreen]->tab();
					break;
				case ESCASCII:
					for (i=0; i<numOfScreens; i++)
                                        {
						screenObj[i]->save();
#ifdef DEBUGPRINT
                                                printf("MSG:60:free on screen%d done\n", i);
#endif

                                        }

#ifdef DEBUGPRINT
                                        printf("MSG:59:save on all screens done\n");
#endif
					exitApp = TRUE;
					continue;
                                case BACKSPACEKEY:
                                        screenObj[currentScreen]->backSpace();
                                        break;
				default:
					screenObj[currentScreen]->otherKey(ascii);
					continue;
			}
		}
                else
                switch(control)
		{
			case F1KEY:
				if (currentScreen == DAYBOOK) continue;
				screenObj[currentScreen]->deactive();
				currentScreen = DAYBOOK;
				screenObj[currentScreen]->setActive();
				break;
			case F2KEY:                                
				screenObj[currentScreen]->deactive();
                                currentScreen = LEDGER;
				screenObj[currentScreen]->setActive();
				break;
			case F3KEY:
				if (currentScreen == BALANCESHEET) continue;
				screenObj[currentScreen]->deactive();
				currentScreen = BALANCESHEET;
				screenObj[currentScreen]->setActive();
				break;
			case F4KEY:
				if (currentScreen == HEADLIST) continue;
				screenObj[currentScreen]->deactive();
				currentScreen = HEADLIST;
				screenObj[currentScreen]->setActive();
				break;
                        case F9KEY:
                                screenObj[BALANCESHEET]->openNewYear();
                                exitApp = TRUE;
                                break;
                        case TABKEY:
				/* Normal tab is handled in ascii... */
                                if (!shiftOn) continue;
				screenObj[currentScreen]->shifttab();
				break;
			case LEFTKEY:
				screenObj[currentScreen]->leftArrow();
				break;
			case RIGHTKEY:
				screenObj[currentScreen]->rightArrow();
				break;
			case UPKEY:
				screenObj[currentScreen]->upArrow();
				break;
			case DOWNKEY:
				screenObj[currentScreen]->downArrow();
				break;
			case PAGEDOWNKEY:
				screenObj[currentScreen]->pageDown();
				break;
			case PAGEUPKEY:
				screenObj[currentScreen]->pageUp();
				break;
			case HOMEKEY:
				screenObj[currentScreen]->home();
				break;
			case ENDKEY:
                                if (ctrlOn)
                                        screenObj[currentScreen]->ctrlend();
                                else
                                        screenObj[currentScreen]->end();
				break;
			case INSERTKEY:
                                if (ctrlOn && !shiftOn)
                                        screenObj[currentScreen]->ctrlC();
                                else if (!ctrlOn && shiftOn)
                                        screenObj[currentScreen]->ctrlV();
                                else
                                        screenObj[currentScreen]->insert();
				break;
			case DELETEKEY:
#ifdef DEBUGPRINT
                                printf("Delete Key hit");
#endif
                                if (ctrlOn)
                                        screenObj[currentScreen]->deleteField();
                                else
                                        screenObj[currentScreen]->deleteRow();
				break;
                        case CTRL_INSERT:
                                if (!ctrlOn) continue;
                                screenObj[currentScreen]->insertRow();
                                break;
                        case CTRL_P:
                                if (!ctrlOn) continue;
                                screenObj[currentScreen]->print();
                                break;
                        case CTRL_F:
                                if (!ctrlOn) continue;
                                screenObj[currentScreen]->search();
                                break;
                        case CTRL_S:
                                if (!ctrlOn) continue;
                                screenObj[currentScreen]->save();
                                break;
                        case CTRL_H:
                                if (!ctrlOn) continue;
                                helpScreen();
                                break;
                        case CTRL_C:
                                if (!ctrlOn) continue;
                                screenObj[currentScreen]->ctrlC();
                                break;
                        case CTRL_V:
                                if (!ctrlOn) continue;
                                screenObj[currentScreen]->ctrlV();
                                break;
		}
	}
#ifdef DEBUGPRINT
        printf("MSG:57:probable end of application\n");
#endif
	return OK;
}

