#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "HAL.h"
#include "read_file.h"
#include "struct.h"


uint16_t READ16(const uint8_t *buffer) 
{
    return (buffer[1] << 8) | buffer[0]; 
}

uint32_t READ32(const uint8_t *buffer) 
{
    return ((uint32_t)buffer[0]) |
           ((uint32_t)buffer[1] << 8) |
           ((uint32_t)buffer[2] << 16)|
           ((uint32_t)buffer[3] << 24);
}

int detectFatType(FATBootSector *bst) 
{
    int fat_type = 0;
    
    uint16_t size = READ16(bst->byte_per_sector); /* Tổng số sector */

    uint32_t totalSectors = (READ16(bst->total_sector) != 0) 
                                ? READ16(bst->total_sector) 
                                : READ32(bst->total_sector_32);

    uint32_t sectorsPerFAT = (READ16(bst->sector_per_fat) != 0) /* Sector mỗi FAT */
                                ? READ16(bst->sector_per_fat) 
                                : READ32(bst->ebpd.fat32.sector_per_fat);
    
    uint32_t rootDirSectors = ((READ16(bst->root_entries_count) * 32) + (size - 1)) / size;/* Tính số sector trong thư mục gốc */

    uint32_t dataSectors = totalSectors - (READ16(bst->reserved_sector_count) + (bst->fat_count * sectorsPerFAT) + rootDirSectors); /* Tính số sector của vùng dữ liệu */

    if (bst->sector_per_clus == 0) /* Kiểm tra sector trong mỗi Cluster */
    {
        printf("Error: sectors_per_clus is 0.\n");
        return -1; 
    }

    uint32_t totalClusters = dataSectors / bst->sector_per_clus;/* Tính tổng Cluster */

    if (totalClusters < 4085) /* Kiểm tra loại Fat */
    {        
        fat_type =  12; // FAT12
    } 
    else if (totalClusters < 65525) 
    {
        fat_type =  16; // FAT16
    } 
    else 
    {       
        fat_type =  32; // FAT32
    }
    return fat_type;
}
                  
int PrintBootSectorInfo(FATBootSector *bst,int FATType)
{
    int result = 1;
   
    printf("Jump Boot: %02X %02X %02X\n", bst->jmp_boot[0], bst->jmp_boot[1], bst->jmp_boot[2]); /*In thông tin cơ bản của boot sector */ 

    printf("OEM Name: ");
    for (int i = 0; i < 8; i++) 
    {
        printf("%c", bst->oem_name[i]);
    }
    printf("\n");
    if(FATType == 32)
    {
        printf("File system: FAT32\n");
        printf("Sectors Per FAT32: %u\n", READ32(bst->ebpd.fat32.sector_per_fat));
        printf("Root Cluster: %u\n", READ32(bst->ebpd.fat32.start_clus_index));
        printf("Fat32 Version: %u\n", READ16(bst->ebpd.fat32.fat32_version));   
    }
    else
    {
        printf("File system: FAT12/FAT16\n");
        printf("Sectors Per FAT: %u\n", READ16(bst->sector_per_fat));
        printf("Fat12/16_Type: %u\n",READ16(bst->ebpd.fat12_16.fat_type));
    }
    printf("Bytes Per Sector: %u\n", READ16(bst->byte_per_sector));
    printf("Sectors Per Cluster: %u\n", bst->sector_per_clus);
    printf("Reserved Sector Count: %u\n", READ16(bst->reserved_sector_count));
    printf("Number of FATs: %u\n", bst->fat_count);
    printf("Max Root Directory Entries: %u\n", READ16(bst->root_entries_count));
    printf("Total Sectors (16-bit): %u\n", READ16(bst->total_sector));
    printf("Media Descriptor: 0x%02X\n", bst->volume_type);
    printf("Sectors Per Track: %u\n", READ16(bst->sector_per_track));
    printf("Number of Heads: %u\n", READ16(bst->disk_head_count));
    printf("Hidden Sector Count: %u\n", READ32(bst->hidden_sector_count));
    printf("Total Sectors (32-bit): %u\n",READ32( bst->total_sector_32));
    printf("Drive Number: %02X\n", bst->ebpd.fat12_16.drive_number);
    printf("Extended Boot Signature: %02X\n", bst->ebpd.fat12_16.extend_boot_signature);
    return result;
}
int printSub(FATBootSector *bst, int FATType, DirectoryEntry dirEntry) 
{  
    int result = 1; 
    uint16_t start_cluster = dirEntry.start_cluster;
    uint32_t sub_dir_start = clusterToSector(bst, start_cluster);
    uint16_t size = READ16(bst->byte_per_sector);
    uint16_t sector_per_clus = bst->sector_per_clus;
    size_t size_sub = size * sector_per_clus;
    uint8_t sub_buffer[size_sub]; 
    uint32_t currentCluster = start_cluster;
    uint32_t validCount = 0;
    uint32_t sub_entry_index;
    DirectoryEntry *validEntries = (DirectoryEntry *)malloc(size_sub);/* Tạo mảng để lưu các entries hợp lệ*/
    if (validEntries == NULL) 
    {
        printf("\nMemory allocation failed during reallocation\n");
        result = -1;
    } 
    do 
    {
        sub_dir_start = clusterToSector(bst, currentCluster);
        if (readMultiSector(sub_dir_start, sector_per_clus, sub_buffer) <= 0) 
        {
            printf("\nError reading subdirectory sectors\n");
            result = -1;
            break; 
        }

        for (sub_entry_index = 0; sub_entry_index < size_sub / sizeof(DirectoryEntry); ++sub_entry_index) 
        {
            DirectoryEntry *sub_dir_entry = (DirectoryEntry *)&sub_buffer[sub_entry_index * sizeof(DirectoryEntry)];

            if (sub_dir_entry->name[0] == 0x00) 
            {
                break; /* Entry Empty */ 
            }
            if (sub_dir_entry->name[0] == 0xE5 || sub_dir_entry->attr == 0x0F) 
            {
                continue; /* Entry is deleted */
            }

            if (validCount % size_sub == 0) 
            {
                validEntries = (DirectoryEntry *)realloc(validEntries, (validCount + size_sub) * sizeof(DirectoryEntry));
                if (validEntries == NULL) 
                {
                    printf("\nMemory allocation failed during reallocation\n");
                    result = -1;
                    break; 
                }
            }

            validEntries[validCount++] = *sub_dir_entry;
        }

        currentCluster = getNextCluster(bst, currentCluster);
    } 
    while ((FATType == 32 && currentCluster < 0xFFFFFF8) || (FATType != 32 && currentCluster < 0xFFF));
    
    printf("\nSubdirectory of %s:\n", dirEntry.name);
    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------------+\n");
    printf("|STT | Name                  | Size     | Attr   | Time      | Date               | Creation Time    |\n");
    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------------+\n");

    for (sub_entry_index = 0; sub_entry_index < validCount; ++sub_entry_index) 
    {
        DirectoryEntry sub_dir_entry = validEntries[sub_entry_index];
        printf("|%2d  | %-11.11s           | %-8u | 0x%-3X  | %02u:%02u     | %02u/%02u/%04u         | %02u:%02u:%02u         |\n",
               sub_entry_index + 1,
               sub_dir_entry.name,
               sub_dir_entry.file_size,
               sub_dir_entry.attr,
               (sub_dir_entry.time >> 11) & 0x1F,
               (sub_dir_entry.time >> 5) & 0x3F,
               (sub_dir_entry.date & 0x1F),
               (sub_dir_entry.date >> 5) & 0x0F,
               ((sub_dir_entry.date >> 9) & 0x7F) + 1980,
               ((sub_dir_entry.last_access_date >> 11) & 0x1F), 
               ((sub_dir_entry.last_access_date >> 5) & 0x3F),  
               ((sub_dir_entry.last_access_date & 0x1F) * 2));
    }
    chooseEntry(bst, FATType, validEntries, validCount);
    free(validEntries);
    return result;
}


int printFileContent(FATBootSector *bst, DirectoryEntry dirEntry, int FATType) 
{
    
    int result = 1; 
    uint32_t start_cluster = dirEntry.start_cluster;
    uint32_t content_dir_start;
    uint16_t size = READ16(bst->byte_per_sector);
    size_t numberSector = (dirEntry.file_size + size - 1) / size;
    size_t x = size * numberSector;

    uint8_t *buffer = (uint8_t *)malloc(x);
    if (buffer == NULL) 
    {
        printf("Memory allocation failed\n");
        return -1;
    }
    size_t bytesRead = 0;
    uint32_t currentCluster = start_cluster;

    while (currentCluster < 0xFFFFFF8 && bytesRead < dirEntry.file_size) /* Đọc nội dung tệp trong khi có cluster hợp lệ*/
    {
        content_dir_start = clusterToSector(bst, currentCluster);

        size_t sectorCount = (dirEntry.file_size - bytesRead + size - 1) / size;
        if (readMultiSector(content_dir_start, sectorCount, &buffer[bytesRead]) != (size * sectorCount)) 
        {
            printf("Error reading file content\n");
            free(buffer);  
            return -1;
        }

        bytesRead += size * sectorCount;

        currentCluster = getNextCluster(bst, currentCluster);
    }

    for (uint32_t i = 0; i < dirEntry.file_size; i++) 
    {
        if (buffer[i] >= 32 && buffer[i] <= 126) 
        {
            printf("%c", buffer[i]);  
        } 
        else 
        {
            printf(".");  
        }
    }

    free(buffer);
    return result;
}

int chooseEntry(FATBootSector *bst, int FATType, DirectoryEntry *validEntries, uint32_t validCount) 
{
    int result = 1;
    uint32_t currentCluster;
    DirectoryEntry dirEntry; 
    while (1) 
    { 
        printf("\nEnter number of file/folder (0 to exit, 1 to show current folder, 2 to go back, 999 to root): ");
        int choice;
        scanf("%d", &choice);
        if (choice == 0) 
        {
            printf("Exit Program.\n");
            exit(0); 
        } 
        else if (choice > 0 && choice <= validCount) 
        {
            dirEntry = validEntries[choice - 1]; 

            if (dirEntry.attr == 0x10) 
            {  
                if (dirEntry.start_cluster == 0) 
                {
                    currentCluster = 0;
                    printf("\nReturn Directory Entries.\n");
                    PrintRootEntries(bst, FATType); 
                } 
                else 
                {
                    currentCluster = dirEntry.start_cluster; 
                    printSub(bst, FATType, dirEntry); 
                }
            } 
            else 
            { 
                printf("\nContent of File %s:\n", dirEntry.name);
                printFileContent(bst, dirEntry, FATType); 
            }
        } 
        else if (choice == 999)
        {
            PrintRootEntries(bst, FATType); 
        }
        else
        {
            printf("\nChoose invaid!\n");
        }
    }
    return result; 
}

int PrintRootEntries(FATBootSector *bst, int FATType) 
{   
    int result = 1;
    uint16_t reserved_sector_count = READ16(bst->reserved_sector_count);
    uint8_t number_of_FATs = bst->fat_count; 
    uint32_t root_dir_start;
    uint32_t root_dir_sector_count;
    DirectoryEntry *validEntries = NULL;
    uint32_t validCount = 0;
    uint32_t entry_index; 
    DirectoryEntry dirEntry;
    if (FATType == 12 || FATType == 16) 
    {
        uint16_t sectorsPerFAT = READ16(bst->sector_per_fat);
        uint16_t root_entries_count = READ16(bst->root_entries_count);
        uint16_t size = READ16(bst->byte_per_sector);

        root_dir_start = reserved_sector_count + (sectorsPerFAT * number_of_FATs);
        root_dir_sector_count = (root_entries_count * 32 + (size - 1)) / size;

        uint8_t buffer[size * root_dir_sector_count]; 
        if (readMultiSector(root_dir_start, root_dir_sector_count, buffer) != (root_dir_sector_count * size)) 
        {
            printf("Error: Can not read mutisector! ");
            result = -1;
        }

        validEntries = (DirectoryEntry *)malloc(root_entries_count * sizeof(DirectoryEntry));
        for (entry_index = 0; entry_index < root_entries_count; ++entry_index) 
        {
            memcpy(&dirEntry, &buffer[entry_index * 32], sizeof(DirectoryEntry));


            if (dirEntry.name[0] == 0x00) 
            {
                break; 
            }
            if (dirEntry.name[0] == 0xE5 || dirEntry.attr == 0x0F)
            {
                continue; 
            }

            validEntries[validCount++] = dirEntry;
     
        }
    } 
    else if (FATType == 32) 
    {        
        uint32_t rootCluster = READ32(bst->ebpd.fat32.start_clus_index);
        uint16_t sector_per_cluster = bst->sector_per_clus;
        uint32_t currentCluster = rootCluster;
        uint16_t size = READ16(bst->byte_per_sector);
        uint8_t buffer[size * sector_per_cluster]; 
        do 
        {
            uint32_t root_dir_start = clusterToSector(bst, currentCluster);
            if (readMultiSector(root_dir_start, bst->sector_per_clus, buffer) != (bst->sector_per_clus * size)) 
            {
                printf("Error: Can not read mutisector! ");
                result = -1;
            }
            
            uint32_t max_entries = (size * bst->sector_per_clus) / 32;
            validEntries = (DirectoryEntry *)realloc(validEntries, (validCount + max_entries) * sizeof(DirectoryEntry));

            for (entry_index = 0; entry_index < (size * bst->sector_per_clus / 32); ++entry_index) 
            {
                memcpy(&dirEntry, &buffer[entry_index * 32], sizeof(DirectoryEntry));

                if (dirEntry.name[0] == 0x00) 
                {
                    break;

                }
                if (dirEntry.name[0] == 0xE5 || dirEntry.attr == 0x0F) 
                {
                    continue; 
                }

                validEntries[validCount++] = dirEntry;
                
            }
            currentCluster = getNextCluster(bst, currentCluster);
        } while (currentCluster < 0xFFFFFF8); 
    }

    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------------+\n");
    printf("|STT | Name                  | Size     | Attr   | Time      | Date               | Creation Time    |\n");
    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------------+\n");

    for (uint32_t i = 0; i < validCount; ++i) 
    {
        dirEntry = validEntries[i]; 
        printf("|%2d  | %-11.11s           | %-8u | 0x%-3X  | %02u:%02u     | %02u/%02u/%04u         | %02u:%02u:%02u         |\n",
            i + 1, 
            dirEntry.name,
            dirEntry.file_size,
            dirEntry.attr,
            (dirEntry.time >> 11) & 0x1F,
            (dirEntry.time >> 5) & 0x3F,
            (dirEntry.date & 0x1F),
            (dirEntry.date >> 5) & 0x0F,
            ((dirEntry.date >> 9) & 0x7F) + 1980,
            ((dirEntry.last_access_date >> 11) & 0x1F), 
            ((dirEntry.last_access_date >> 5) & 0x3F),  
            ((dirEntry.last_access_date & 0x1F) * 2) );
    }
    chooseEntry(bst, FATType, validEntries, validCount);
    free(validEntries);
    return result;
}

uint32_t clusterToSector(FATBootSector *bst , uint32_t currentCluster) 
{
    uint32_t firstDataSector = READ16(bst->reserved_sector_count) 
    + (bst->fat_count * (READ16(bst->sector_per_fat) != 0 
    ? READ16(bst->sector_per_fat) 
    : READ32(bst->ebpd.fat32.sector_per_fat)))
    + ((READ16(bst->root_entries_count) * 32 + READ16(bst->byte_per_sector) - 1) 
    / READ16(bst->byte_per_sector));
    return firstDataSector + (currentCluster - 2) * bst->sector_per_clus;
}

uint32_t getNextCluster(FATBootSector *bst, uint32_t currentCluster) 
{
    uint16_t reservedSectors = READ16(bst->reserved_sector_count);
    uint8_t numberOfFATs = bst->fat_count;
    uint32_t fatOffset;
    uint16_t size = READ16(bst->byte_per_sector);
    uint8_t fatBuffer[size];
    int FATType = detectFatType(bst);    
	uint16_t val = 0;
	uint32_t nextCluster = 0;
	uint32_t fatSector;
	uint32_t entOffset; 
	int redVal;
    if (FATType == 12) 
	{
        fatOffset = (currentCluster *3) / 2;
    } 
	else if (FATType == 16) 
	{
        fatOffset = currentCluster * 2;
    } 
	else if (FATType == 32) 
	{
        fatOffset = currentCluster * 4;
    } 
	else 
	{
        printf("Unsupported FAT type.\n");
        redVal = 0;
    }
 
    fatSector = reservedSectors + (fatOffset / size);
    entOffset = fatOffset % size;
    if (readSector(fatSector, fatBuffer) != size) 
	{
        printf("Error reading FAT sector.\n");
        redVal = 0;
    }
    if (FATType == 12) 
	{
        val = READ16(&fatBuffer[entOffset]);
        if (currentCluster & 0x0001) 
		{
            nextCluster = val >> 4;
        } 
		else 
		{
            nextCluster = val & 0x0FFF;
        }
    } 
	else if (FATType == 16) 
	{
        nextCluster = READ16(&fatBuffer[entOffset]);
    } 
	else if (FATType == 32)
	{  
        nextCluster = READ32(&fatBuffer[entOffset]) & 0x0FFFFFFF;
    }
    redVal = nextCluster;
    return redVal;
}


