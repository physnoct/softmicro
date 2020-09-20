#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <signal.h>

#include "softmicro.h"

char filename[128] = "memory.bin";

static void finish(int sig);

void PrintBinary2(uint8_t data)
{
int i=8;

    do
    {
        printw("%d ",(data >> (i-1)) & 1);
    } while (--i);
}

void DumpRegisters(void)
{
int i,j;
uint16_t sp;
    sp = getsp();

    /* Restore cursor position */
    printw("Size: %d, Address mode: %02X\n",app_size,adr_mode);
    printw("PC: %04X:",app_pc & 0xFFFF);
    for (i=0;i<12;i++)
    {
        printw(" %02X",app_memory[app_pc+i]);
    }

    printw("\nFlags: S T H I V N Z C               \tSP+03: %04X [%02X]\n       ",(sp+3)&0xFFFF,app_memory[(sp+3) & 0xffff]);
    PrintBinary2(app_flags);

    printw("               \tSP+02: %04X [%02X]\n",(sp+2)&0xFFFF,app_memory[(sp+2) & 0xffff]);
    printw("      f e d c b a 9 8 7 6 5 4 3 2 1 0\tSP+01: %04X [%02X]\n",(sp+1)&0xFFFF,app_memory[(sp+1) & 0xffff]);

    for (i=0;i<16;i++)
    {
        printw("R%02d: ",i);

        for (j=15;j>-1;j--)
        {
            printw("%02X",app_reg[i][j] & 0xFF);
        }
        printw("\tSP-%02X: %04X [%02X]\n",i,(sp-i)&0xFFFF,app_memory[(sp-i) & 0xffff]);
    }
    printw("\n");
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

    printw("File to open: ");
    result = scanw("%s",filename);
    printw("Result: %02X, filename: %s\n",result,filename);

    fp = fopen(filename,"rb");
    if (fp == NULL) printw("File %s not found\n",filename);
    else
    {
        fseek(fp, 0 , SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file

        if (fileSize > 65536) fileSize = 65536;

        fread(app_memory,1,fileSize,fp);
        printw("File %s loaded\n",filename);
        fclose(fp);
    }
}

void LoadMemory(void)
{
FILE *p;

    p = fopen(filename,"rb");
    if (p == NULL) printw("File %s not found\n",filename);
    else
    {
        fread(app_memory,1,65536,p);
        printw("File %s loaded\n",filename);
        fclose(p);
    }
}

void SaveMemory(void)
{
FILE *p;

    p = fopen(filename,"wb+");

    fwrite(app_memory,1,65536,p);
    printw("File %s saved\n",filename);
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
//int i;
char c = 0x00;
bool quit = false;

/* initialize your non-curses data structures here */

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    (void) initscr();      /* initialize the curses library */
//    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) echo();         /* echo input - in color */

    ClrRegisters();

    clear();

    do
    {
        move(0,0);

        DumpRegisters();

        printw("Soft micro C v1.0, Inst set: %d\n\n"
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

        c = getch();
        printw("\n");
        clrtobot();

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

        refresh();

    } while (!quit);

    finish(0);
}

static void finish(int sig)
{
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}
