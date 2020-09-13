#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>

#include "softmicro.h"

//Project status: en cours,

void PrintBinary2(uint8_t data)
{
int i=8;

    do
    {
        printf("%d ",(data >> (i-1)) & 1);
    } while (--i);
}

void DumpRegisters(void)
{
int i,j;
uint16_t sp;
    sp = getsp();

    /* Restore cursor position */
    printf("Size: %d, Address mode: %02X\n",app_size,adr_mode);
    printf("PC: %04X:",app_pc & 0xFFFF);
    for (i=0;i<12;i++)
    {
        printf(" %02X",app_memory[app_pc+i]);
    }

    printf("\nFlags: S T H I V N Z C               \tSP+03: %04X [%02X]\n       ",(sp+3)&0xFFFF,app_memory[(sp+3) & 0xffff]);
    PrintBinary2(app_flags);

    printf("               \tSP+02: %04X [%02X]\n",(sp+2)&0xFFFF,app_memory[(sp+2) & 0xffff]);
    printf("      f e d c b a 9 8 7 6 5 4 3 2 1 0\tSP+01: %04X [%02X]\n",(sp+1)&0xFFFF,app_memory[(sp+1) & 0xffff]);

    for (i=0;i<16;i++)
    {
        printf("R%02d: ",i);

        for (j=15;j>-1;j--)
        {
            printf("%02X",app_reg[i][j] & 0xFF);
        }
        printf("\tSP-%02X: %04X [%02X]\n",i,(sp-i)&0xFFFF,app_memory[(sp-i) & 0xffff]);
    }
    printf("\n");
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
char filename[128];
long fileSize;
int result;

    printf("File to open: ");
    result = scanf("%s",filename);
    printf("Result: %02X, filename: %s\n",result,filename);

    fp = fopen(filename,"rb");
    if (fp == NULL) printf("File %s not found\n",filename);
    else
    {
        fseek(fp, 0 , SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file

        if (fileSize > 65536) fileSize = 65536;

        fread(app_memory,1,fileSize,fp);
        printf("File %s loaded\n",filename);
        fclose(fp);
    }
}

void LoadMemory(void)
{
FILE *p;

    p = fopen("memory.bin","rb");
    if (p == NULL) printf("File memory.bin not found\n");
    else
    {
        fread(app_memory,1,65536,p);
        printf("File memory.bin loaded\n");
        fclose(p);
    }
}

void SaveMemory(void)
{
FILE *p;

    p = fopen("memory.bin","wb+");

    fwrite(app_memory,1,65536,p);
    printf("File memory.bin saved\n");
    fclose(p);
}

void Run(void)
{
}

int main()
{
//int i;
char c = 0x00;

    ClrRegisters();

    /* Clear screen, store cursor position */
    printf("\033c");

    do
    {
        printf("\033[f");

        DumpRegisters();

        printf("Soft micro C v1.0\n\n"
                "\tA\tReset\n"
                "\tB\tOp Step\n"
                "\tC\t\n"
                "\tD\tRun\n"
                "\tE\tErase memory\n"
                "\tF\tLoad binary file\n"
                "\tL\tLoad memory\n"
                "\tS\tSave memory\n"
                "\tQ\tExit\n\n> "
        );

        printf("\033[s\n\r");
        //c = getchar();
        //c = getch();

        switch(toupper(c))
        {
            case 'A':
                printf("\033[J");
                ClrRegisters();
                break;
            case ' ':
            case 'B':
                printf("\033[J");
                OpStep();
                break;
            case 'C':
                break;
            case 'D':
                Run();
                break;
            case 'E':
                printf("\033[J");
                ClearMemory();
                break;
            case 'F':
                printf("\033[J");
                LoadBinaryFile();
                break;
            case 'L':
                printf("\033[J");
                LoadMemory();
                break;
            case 'S':
                printf("\033[J");
                SaveMemory();
                break;

            case 'Q':
                printf("\033[J");
                exit(0);
            default:
            ;
        }
        printf("\033[u");
        c = getchar();

        //printf("\033[J");
/*        printf("\033[s");
        printf("\033[K                                                 ");
        printf("\033[K                                                 ");
        printf("\033[K                                                 ");
        printf("\033[K                                                 ");
        printf("\033[K                                                 ");
        printf("\033[u"); */

    } while (true);

    return 0;
}

