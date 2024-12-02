#include <stdio.h>
#include <string.h>

#include "struct.h"
#include "HAL.h"
#include "read_file.h"


int menu() 
{
    FATBootSector bootSector;
    while (1)
    {
        char imgpath[256];
        printf("Enter name File: ");
        scanf("%255s", imgpath);
        if (init_image(imgpath, &bootSector) !=0 )
        {
            printf("Failed to open floppy image\n");
        }
        else 
        {
            break;
        }
    }
    
    int FATType = detectFatType(&bootSector);
    if (FATType == 12) 
    {
        printf("Detected FAT12 filesystem.\n");
    } 
    else if (FATType == 16) 
    {
        printf("Detected FAT16 filesystem.\n");
    } 
    else if (FATType == 32) 
    {
        printf("Detected FAT32 filesystem.\n");
    } 
    else 
    {
        printf("Unknown FAT filesystem.\n");
        return -1; 
    }
    //PrintBootSectorInfo(&bootSector,FATType);
    PrintRootEntries(&bootSector, FATType);
    hal_close();
    
    return 0;
}


