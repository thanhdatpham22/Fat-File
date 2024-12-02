#include <stdio.h>
#include <stdint.h>
#include "HAL.h"

static uint16_t sectorSize = 512;  
static FILE *file = NULL;


int init_image(const char *file_path, FATBootSector *bootSector) 
{
    file = fopen(file_path, "rb");
    if (file == NULL) 
    {
        perror("---Error opening file---");
        return -1;
    }

    if (fread(bootSector, 1, sizeof(FATBootSector), file) != sizeof(FATBootSector)) 
    {
        printf("---Error: Unable to read boot sector.---\n");
        return -1;
    }

    sectorSize = (bootSector->byte_per_sector[1] << 8) | bootSector->byte_per_sector[0];
    //printf("Detected sector size: %u bytes\n", sectorSize);

    return 0;
}


int32_t readSector(uint32_t index, uint8_t *buff) 
{
    if (file == NULL) return -1; 

    fseek(file, index * sectorSize, SEEK_SET);  
    return fread(buff, 1, sectorSize, file);    
}


int32_t readMultiSector(uint32_t index, uint32_t num, uint8_t *buff) 
{
    if (file == NULL) 
    {
        printf("---Error: File not opened.---\n");
        return -1; 
    }


    long offset = index * sectorSize;

    if (fseek(file, offset, SEEK_SET) != 0) 
    {
        perror("---Error seeking in file---"); 
        return -1;
    }

    
    size_t bytes_read = fread(buff, sectorSize, num , file);
    
   
    if (bytes_read != num) 
    {
        printf("Error: Only read %zu sectors, expected %u sectors.\n", bytes_read, num);
        return -1;
    }

    return bytes_read * sectorSize; 
}


void hal_close() 
{
    if (file != NULL) 
    {
        fclose(file);
        file = NULL;  
    }
}
