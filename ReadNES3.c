//3GenGames, 2011.
//Rewrite coming soon, probably, now that the code will be
//more accessible and can't be so terrible.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Version "3"
#define Revision "01"

char Buffer[16384];
unsigned char BaseROMFileName[256];
unsigned char OutputData[512];

int main(int argc, char *argv[])
{
//Files
    FILE *ROMFile;
    FILE *OutputROMFile;
    FILE *DetailsOutput;
//Pointers
    char *DeleteCharPointer;
//Main variables to run the program.
    int ProgramRepeat;
    int CharacterRepeat;
    int RepeatLoop;
    int DataTransferLoop;
//File details.
    unsigned char MapperNumber; //Mapper
    unsigned char Mirroring[11]; //Horizontal, Vertical, 4-Screen
    unsigned char System[32]; //NES,Playchoice 10,VS Unisystem
    unsigned char Region[5];
    unsigned char Trainer[4];
    unsigned char HeaderFormat;
//File header binary data.
    unsigned char HeaderTest[4];
    unsigned char ProgramBanks;
    unsigned char CharacterBanks;
    unsigned char Flags1;
    unsigned char Flags2;
    unsigned char ProgramRAM;
    unsigned char Flags3;
    unsigned char Flags4;
    int fwriteReturn;

    printf("ReadNES%s.%s\n",Version,Revision);
    if (argc<2)
    {
        printf("How to use:\n./ReadNES3 [File] [Program ROM repeat count] [CharacterROM repeat count]\n");
        return 0;
    }

    ROMFile=fopen(argv[1],"rb");
    if (ROMFile==NULL)
    {
        printf("\n\nFailed to open ROM file, aborting.\n\n");
        return 0;
    }

    switch(argc)
    {
    case 2:
        ProgramRepeat=1;
        CharacterRepeat=1;
        printf("\nNo data repetition.\n");
        break;
    case 3:
        ProgramRepeat=atoi(argv[2]);
        CharacterRepeat=1;
        printf("\nWriting program ROM data to chip %u time(s).\n",ProgramRepeat);
        break;
    case 4:
        ProgramRepeat=atoi(argv[2]);
        CharacterRepeat=atoi(argv[3]);
        printf("\nWriting program ROM data to chip %u time(s).\nWriting character ROM data to chip %u time(s).\n",ProgramRepeat,CharacterRepeat);
        break;
    default:
        printf("\nToo many arguments, aborting.\n\n");
        return 0;
    }

    fread(HeaderTest,1,4,ROMFile); //4
    if (HeaderTest[0]!='N' || HeaderTest[1]!='E' || HeaderTest[2]!='S' || HeaderTest[3]!=0x1A)
    {
        fclose(ROMFile);
        printf("\n\nBad header, aborting.\n\n");
        return 0;
    }
    printf("\n\tHeader:\t\tGood\n");
    ProgramBanks=fgetc(ROMFile); //5
    CharacterBanks=fgetc(ROMFile); //6
    Flags1=fgetc(ROMFile); //7
    Flags2=fgetc(ROMFile); //8
    ProgramRAM=fgetc(ROMFile); //9
    Flags3=fgetc(ROMFile); //10
    Flags4=fgetc(ROMFile);  //11
    fseek(ROMFile,5,SEEK_CUR);  //16

    strncpy(BaseROMFileName,argv[1],255);
    DeleteCharPointer=strrchr(BaseROMFileName,'.');
    if (DeleteCharPointer!=NULL)
    {
        *DeleteCharPointer='\0';
    }

    strcpy(OutputData,BaseROMFileName);
    strcat(OutputData,"Details.txt");
    DetailsOutput=fopen(OutputData,"w");

    printf("\tBase file name: %s\n",BaseROMFileName);
    fprintf(DetailsOutput,"Base file name: %s\n",BaseROMFileName);
    fprintf(DetailsOutput,"Program repetition: %u time(s)\n",ProgramRepeat);
    fprintf(DetailsOutput,"Character repetition: %u time(s)\n",CharacterRepeat);
    fprintf(DetailsOutput,"Program RAM size: %iKB\n",ProgramRAM<<3);

    if ((Flags2 & 0x0C) == 0x04)
    {
        HeaderFormat=2;
    }
    else
    {
        HeaderFormat=1;
    }
    printf("\tiNES format: \t%u.0\n",HeaderFormat);
    fprintf(DetailsOutput,"iNES format: %u.0\n",HeaderFormat);

    switch(Flags2 & 0x03)
    {
    case 1:
        strcpy(System,"VS Unisystem");
        break;
    case 2:
        strcpy(System,"Playchoice-10");
        break;
    default:
        strcpy(System,"NES");
        break;
    }
    printf("\tSystem:\t\t%s\n",System);
    fprintf(DetailsOutput,"System: %s\n",System);

    printf("\tProgram Banks:\t%u (16KB Each)\n",ProgramBanks);
    fprintf(DetailsOutput,"Program Banks: %u (16KB Each)\n",ProgramBanks);

    switch(CharacterBanks)
    {
    case 0:
        printf("\tCharacter Banks:%u (8KB CHR-RAM)\n",CharacterBanks);
        fprintf(DetailsOutput,"Character Banks: %u (8KB CHR-RAM)\n",CharacterBanks);
        break;
    default:
        printf("\tCharacter Banks:%u (8KB Each)\n",CharacterBanks);
        fprintf(DetailsOutput,"Character Banks: %u (8KB Each)\n",CharacterBanks);
        break;
    }

    MapperNumber=((Flags2 & 0xF0) | (Flags1 >> 4));
    printf("\tProgram RAM:\t%uKB\n\tMapper:\t\t%u\n",ProgramRAM<<3,MapperNumber);
    fprintf(DetailsOutput,"Mapper: %u\n",MapperNumber);

    switch(Flags1 & 9)
    {
    case 0:
        strcpy(Mirroring,"Horizontal");
        break;
    case 1:
        strcpy(Mirroring,"Vertical");
        break;
    default:
        strcpy(Mirroring,"4-Screen");
        break;
    }
    printf("\tMirroring:\t%s\n",Mirroring);
    fprintf(DetailsOutput,"Mirroring: %s\n",Mirroring);

    if (Flags1 & 4)
    {
        strcpy(Trainer,"Yes");
    }
    else
    {
        strcpy(Trainer,"No");
    }
    printf("\tTrainer:\t%s\n",Trainer);
    fprintf(DetailsOutput,"Trainer: %s\n",Trainer);

    if (Trainer[0]=='Y')
    {
        strcpy(OutputData,BaseROMFileName);
        strcat(OutputData,"Trainer.bin");
        OutputROMFile=fopen(OutputData,"wb");
        if (OutputROMFile==NULL)
        {
            printf("\nFailed to open trainer output file.\n");
            fclose(DetailsOutput);
            fclose(ROMFile);
            return 0;
        }
        fread(Buffer,1,512,ROMFile);
        fwrite(Buffer,1,512,OutputROMFile);
        fclose(OutputROMFile);
    }

    strcpy(OutputData,BaseROMFileName);
    strcat(OutputData,"Program.bin");
    OutputROMFile=fopen(OutputData,"wb");
    if (OutputROMFile==NULL)
    {
        printf("\nFailed to open program output file.\n");
        fclose(DetailsOutput);
        fclose(ROMFile);
        return 0;
    }

    for(RepeatLoop=0; RepeatLoop<ProgramRepeat; RepeatLoop++)
    {
        for(DataTransferLoop=0; DataTransferLoop<ProgramBanks; DataTransferLoop++)
        {
            fread(Buffer,1,16384,ROMFile);
            fwrite(Buffer,1,16384,OutputROMFile);
        }
        fseek(ROMFile,-ProgramBanks*16384,SEEK_CUR);
    }
    fclose(OutputROMFile);

    fseek(ROMFile,ProgramBanks*16384,SEEK_CUR);

    strcpy(OutputData,BaseROMFileName);
    strcat(OutputData,"Character.bin");
    OutputROMFile=fopen(OutputData,"wb");
    if (OutputROMFile==NULL)
    {
        printf("\nFailed to open character output file.\n");
        fclose(DetailsOutput);
        fclose(ROMFile);
        return 0;
    }

    for(RepeatLoop=0; RepeatLoop<CharacterRepeat; RepeatLoop++)
    {
        for(DataTransferLoop=0; DataTransferLoop<CharacterBanks; DataTransferLoop++)
        {
            fread(Buffer,1,8192,ROMFile);
            fwrite(Buffer,1,8192,OutputROMFile);
        }
        fseek(ROMFile,-CharacterBanks*8192,SEEK_CUR);
    }
    fclose(OutputROMFile);

    fseek(ROMFile,CharacterBanks*8192,SEEK_CUR);


    if (System[0]=='P')
    {
        strcpy(OutputData,BaseROMFileName);
        strcat(OutputData,"HintScreen.bin");
        OutputROMFile=fopen(OutputData,"wb");
        if (OutputROMFile==NULL)
        {
            printf("\nFailed to open character output file.\n");
            fclose(DetailsOutput);
            fclose(ROMFile);
            return 0;
        }
        fread(Buffer,1,8192,ROMFile);
        fwrite(Buffer,1,8192,OutputROMFile);
        fclose(OutputROMFile);
    }

    fclose(DetailsOutput);
    fclose(ROMFile);
    return 0;
}
