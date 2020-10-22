#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <signal.h>

#define USE_COLOR
#include "softmicro.h"

WINDOW *wBMain, *wBRegs, *wBMem, *wBStack, *wBPC, *wBConsole;
WINDOW *wMain, *wRegs, *wMem, *wStack, *wPC, *wConsole;
SCREEN *sMem = NULL;

uint16_t Current_address = 0;

char filename[128] = "memory.bin";

static void finish(int sig);

void PrintBinary2(WINDOW *w, uint8_t data)
{
int i=8;

    do
    {
        wprintw(w,"%d ",(data >> (i-1)) & 1);
    } while (--i);
}

void DisplayStack(WINDOW *w)
{
int i;
uint16_t sp;

    sp = getsp();

    wbkgd(w,COLOR_PAIR(2));
    wclear(w);

    for (i=12;i>=0;i--)
    {
        wprintw(w,"SP+%02X: %04X [%02X]\n",i,(sp+i)&0xFFFF,app_memory[(sp+i) & 0xffff]);
    }
    wattrset(w,COLOR_PAIR(1));
    for (i=1;i<16;i++)
    {
        wprintw(w,"SP-%02X: %04X [%02X]\n",i,(sp-i)&0xFFFF,app_memory[(sp-i) & 0xffff]);
    }
    wrefresh(w);
}

void DisplayPC(WINDOW *w)
{
int i;
    wbkgd(w,COLOR_PAIR(5));
    wclear(w);

    wprintw(w,"Size: %d, Address mode: %02X\n",app_size,adr_mode);
    wprintw(w,"PC: %04X:",app_pc & 0xFFFF);
    for (i=0;i<16;i++)
    {
        wprintw(w," %02X",app_memory[app_pc+i]);
    }
    wrefresh(w);
}

void DisplayRegisters(WINDOW *w)
{
int i,j;
    wbkgd(w,COLOR_PAIR(4));
    wclear(w);

    wprintw(w,"Flags: S T H I V N Z C\n       ");
    PrintBinary2(w,app_flags);

    wprintw(w,"\n      f e d c b a 9 8 7 6 5 4 3 2 1 0\n");

    for (i=0;i<16;i++)
    {
        wprintw(w,"R%02d: ",i);

        for (j=15;j>-1;j--)
        {
            wprintw(w,"%02X",app_reg[i][j] & 0xFF);
        }
        wprintw(w,"\n");
    }
    wprintw(w,"\n");
    wrefresh(w);
}

void DisplayMemory(WINDOW *w, uint16_t addr)
{
int i,j;

    wbkgd(w,COLOR_PAIR(3));
    wclear(w);

    for (i=0;i<16;i++)
    {
        wprintw(w,"%04X: ",addr & 0xFF00 + i*16);

        for (j=0;j<16;j++)
        {
            wprintw(w,"%02X ",app_memory[(addr & 0xFF00) + i*16 + j] & 0xFF);
        }
    }
    wprintw(w,"\n");
    wrefresh(w);
}

void ClrRegisters(void)
{
int i,j;

    app_size = 1;
    adr_mode = 0;
    app_flags = 0;
    app_pc = 0;

    for (i=0;i<16;i++)
    {
        for (j=0;j<16;j++)
        {
            app_reg[i][j] = 0;
        }
    }
}

void ClearMemory(void)
{
int i;

    for (i=0;i<65536;i++)
    {
        app_memory[i] = 0;
    }
}

void LoadBinaryFile(void)
{
FILE *fp;
long fileSize;
int result;

    wprintw(wConsole,"File to open: ");
    result = scanw("%s",filename);
    wprintw(wConsole,"Result: %02X, filename: %s\n",result,filename);

    fp = fopen(filename,"rb");
    if (fp == NULL) wprintw(wConsole,"File %s not found\n",filename);
    else
    {
        fseek(fp, 0 , SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file

        if (fileSize > 65536) fileSize = 65536;

        fread(app_memory,1,fileSize,fp);
        wprintw(wConsole,"File %s loaded\n",filename);
        fclose(fp);
    }
}

void LoadMemory(void)
{
FILE *p;

    p = fopen(filename,"rb");
    if (p == NULL) wprintw(wConsole,"File %s not found\n",filename);
    else
    {
        fread(app_memory,1,65536,p);
        wprintw(wConsole,"File %s loaded\n",filename);
        fclose(p);
    }
}

void SaveMemory(void)
{
FILE *p;

    p = fopen(filename,"wb+");

    fwrite(app_memory,1,65536,p);
    wprintw(wConsole,"File %s saved\n",filename);
    fclose(p);
}

void RunToNextReturn(void)
{
    run_until_ret = true;
    step_mode = false;
    while (run_until_ret) OpStep();
    step_mode = true;
}

void Run(void)
{
    step_mode = false;
    while (!step_mode) OpStep();
}

int main()
{
	int ch;

char c = 0x00;
bool quit = false;

/* initialize your non-curses data structures here */

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    slk_init(3);    /* Soft key setup */
    initscr();      /* initialize the curses library */

//    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) echo();         /* echo input - in color */

    //sMem = newterm(NULL,stdin,stdout);
    //wprintw(wConsole,"Screen memory\n");
    if (has_colors())
    {
        start_color();

        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
	 * be redefined.  This example uses the same value for the color
	 * pair as for the foreground color, though of course that is not
	 * necessary:
         */
/*
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
*/
        init_pair(1, COLOR_BLACK, COLOR_RED);
        init_pair(2, COLOR_BLACK, COLOR_GREEN);
        init_pair(3, COLOR_BLACK, COLOR_YELLOW);
        init_pair(4, COLOR_BLACK, COLOR_BLUE);
        init_pair(5, COLOR_BLACK, COLOR_CYAN);
        init_pair(6, COLOR_BLACK, COLOR_MAGENTA);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
    }
    refresh();

    /* PC */
    wBPC = newwin(4,60,0,0);
    box(wBPC,0,0);
    mvwaddstr(wBPC,0,2,"PC");
    wrefresh(wBPC);

    wPC = subwin(wBPC,2,58,1,1);
    DisplayPC(wPC);

    /* Stack */
    wBStack = newwin(30,19,4,0);
    box(wBStack,0,0);
    mvwaddstr(wBStack,0,2,"Stack");
    wrefresh(wBStack);

    wStack = subwin(wBStack,28,17,5,1);
    DisplayStack(wStack);

    /* Regs */
    wBRegs = newwin(21,40,4,20);
    box(wBRegs,0,0);
    mvwaddstr(wBRegs,0,2,"Regs");
    wrefresh(wBRegs);

    wRegs = subwin(wBRegs,19,38,5,21);
    DisplayRegisters(wRegs);

    /* Memory */
    wBMem = newwin(18,56,0,60);
    box(wBMem,0,0);
    mvwaddstr(wBMem,0,2,"Memory");
    wrefresh(wBMem);

    wMem = subwin(wBMem,16,54,1,61);
    DisplayMemory(wMem,0);

    /* Console */
    wBConsole = newwin(33,132,34,0);
    box(wBConsole,0,0);
    mvwaddstr(wBConsole,0,2,"Console");
    wrefresh(wBConsole);

    wConsole = subwin(wBConsole,31,130,35,1);
    scrollok(wConsole,TRUE);
    refresh();

    wattrset(wConsole,COLOR_PAIR(7));
	//wprintw(wConsole,"Soft micro C v1.0, Inst set: %d\n\n",SOFT_MICRO_INST_SET_VERSION);
    wrefresh(wConsole);


    ClrRegisters();
    refresh();

    //c = getch();

    do
    {
        //wclear(wConsole);
        wmove(wConsole,0,0);

//        attrset(COLOR_PAIR(5));
        DisplayPC(wPC);
        DisplayStack(wStack);
        DisplayRegisters(wRegs);
        DisplayMemory(wMem,Current_address);

//        attrset(COLOR_PAIR(7));
        wprintw(wConsole,"Soft micro C v1.0, Inst set: %d\n\n"
                "\tA\tReset\n"
                "\tB\tStep into\n"
                "\tC\tRun to next return\n"
                "\tD\tRun\n"
                "\tE\tErase memory\n"
                "\tF\tLoad binary file\n"
                "\tL\tLoad memory\n"
                "\tS\tSave memory\n"
                "\tQ\tExit\n\n> ",
                SOFT_MICRO_INST_SET_VERSION
        );

        c = wgetch(wConsole);
        wprintw(wConsole,"\n");
        wclrtobot(wConsole);

        switch(toupper(c))
        {
            case 'A':
                ClrRegisters();
                break;
            case ' ':
            case 'B':
                OpStep();
                break;
            case 'C':
                RunToNextReturn();
                break;
            case 'D':
                Run();
                break;
            case 'E':
                ClearMemory();
                break;
            case 'F':
                LoadBinaryFile();
                break;
            case 'L':
                LoadMemory();
                break;
            case 'S':
                SaveMemory();
                break;

            case 'Q':
                quit = true;
                break;
            default:
            ;
        }

        wrefresh(wConsole);

    } while (!quit);


    finish(0);
}

static void finish(int sig)
{
    if (sMem  != NULL) delscreen(sMem);
    if (wMem  != NULL) delwin(wMem);
    if (wStack != NULL) delwin(wStack);
    if (wPC   != NULL) delwin(wPC);
    if (wRegs != NULL) delwin(wRegs);
    if (wConsole != NULL) delwin(wConsole);

    if (wBMem  != NULL) delwin(wBMem);
    if (wBStack != NULL) delwin(wBStack);
    if (wBPC   != NULL) delwin(wBPC);
    if (wBRegs != NULL) delwin(wBRegs);
    if (wBConsole != NULL) delwin(wBConsole);
    if (wMain != NULL) delwin(wMain);
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}
