#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "HAL.h"
#include "read_file.h"
#include "struct.h"

#define SECTOR_SIZE 512
#define MAX_ENTRIES 100
#define READ16(x) (x[0] | (x[1] << 8))
#define READ32(x) (x[0] | (x[1] << 8) | (x[2] << 16) | (x[3] << 24))
DirectoryEntry validEntries[MAX_ENTRIES];
uint32_t validCount = 0;
typedef struct 
{
    DirectoryEntry entries[MAX_ENTRIES];
    uint32_t count;
} DirectoryStack;

DirectoryStack dirStack[MAX_ENTRIES];  
int dirStackIndex = -1;                  

void pushToStack(DirectoryEntry *entries, uint32_t count) 
{
    if (dirStackIndex < MAX_ENTRIES - 1) {
        dirStackIndex++;
        memcpy(dirStack[dirStackIndex].entries, entries, sizeof(DirectoryEntry) * count);
        dirStack[dirStackIndex].count = count;
    } else 
    {
        printf("Stack is full, cannot push more directories!\n");
    }
}

bool popFromStack(DirectoryEntry **entries, uint32_t *count) 
{
    if (dirStackIndex >= 0) 
    {
        *entries = dirStack[dirStackIndex].entries;
        *count = dirStack[dirStackIndex].count;
        dirStackIndex--;
        return true;
    }
    return false;
}

void PrintBootSectorInfo(FATBootSector *bst)
 {
    readSector(0, (uint8_t*)(bst));
    printf("Jump Boot: %02X %02X %02X\n", bst->jmp_boot[0], bst->jmp_boot[1], bst->jmp_boot[2]);

    printf("OEM Name: ");
    for (int i = 0; i < 8; i++) {
        printf("%c", bst->oem_name[i]);
    }
    printf("\n");

    printf("Bytes Per Sector: %u\n", READ16(bst->byte_per_sector));
    printf("Sectors Per Cluster: %u\n", bst->sector_per_clus);
    printf("Reserved Sector Count: %u\n", READ16(bst->reserved_sector_count));
    printf("Number of FATs: %u\n", bst->fat_count);
    printf("Max Root Directory Entries: %u\n", READ32(bst->root_entries_count));
    printf("Total Sectors (32-bit): %u\n", READ32(bst->total_sector_32));
    printf("Sectors Per FAT: %u\n", READ32(bst->sector_per_fat));
    printf("Hidden Sector Count: %u\n", READ32(bst->hidden_sector_count));

    printf("Drive Number: %02X\n", bst->ebpd.fat32.drive_number);
    printf("Extended Boot Signature: %02X\n", bst->ebpd.fat32.extend_boot_signature);
    printf("Volume ID: ");
    for (int i = 0; i < 4; i++) {
        printf("%02X ", bst->ebpd.fat32.volume_id[i]);
    }
    printf("\n");

    printf("Volume Label: ");
    for (int i = 0; i < 11; i++) {
        printf("%c", bst->ebpd.fat32.volume_label[i]);
    }
    printf("\n");

    printf("File System Type: ");
    for (int i = 0; i < 8; i++) {
        printf("%c", bst->ebpd.fat32.fat_type[i]);
    }
    printf("\n");
}

void printSub(FATBootSector *bst, DirectoryEntry dirEntry) 
{
    uint32_t start_cluster = (dirEntry.start_cluster_high << 16) | dirEntry.start_cluster;
    uint32_t sub_dir_start = ((start_cluster - 2) * bst->sector_per_clus) + (READ16(bst->reserved_sector_count) + (READ32(bst->sector_per_fat) * bst->fat_count));
    uint8_t sub_buffer[SECTOR_SIZE * 14];
    DirectoryEntry validEntries[MAX_ENTRIES]; 
    uint32_t validCount = 0;

    if (readMultiSector(sub_dir_start, 1, sub_buffer) <= 0) { 
        printf("\nError reading subdirectory\n");
        return;
    }

    printf("\nSubdirectory of %s:\n", dirEntry.name);
    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------+\n");
    printf("|STT | Name                  | Size (B) | Attr   | Time      | Date               | Cluster    |\n");
    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------+\n");

    for (uint32_t sub_entry_index = 0; sub_entry_index < 16; ++sub_entry_index) { 
        DirectoryEntry *sub_dir_entry = (DirectoryEntry *)&sub_buffer[sub_entry_index * 32];

        if (sub_dir_entry->name[0] == 0x00) {
            break;  
        }
        if (sub_dir_entry->name[0] == 0xE5) {
            continue;  
        }
        if (validCount < MAX_ENTRIES) {
            validEntries[validCount++] = *sub_dir_entry;
        }
    }

    for (uint32_t i = 0; i < validCount; ++i) {
        DirectoryEntry sub_dir_entry = validEntries[i];
        printf("|%2d  | %-11.11s           | %-8u | 0x%-3X  | %02u:%02u     | %02u/%02u/%04u         | %-6u     |\n", 
            i + 1, 
            sub_dir_entry.name, 
            sub_dir_entry.file_size, 
            sub_dir_entry.attr, 
            (sub_dir_entry.time >> 11) & 0x1F, 
            (sub_dir_entry.time >> 5) & 0x3F, 
            (sub_dir_entry.date & 0x1F), 
            (sub_dir_entry.date >> 5) & 0x0F, 
            ((sub_dir_entry.date >> 9) & 0x7F) + 1980, 
            start_cluster);
    }
    chooseEntry(bst, validEntries, validCount);
}

void printFileContent(FATBootSector *bst, DirectoryEntry dirEntry) 
{
    uint32_t start_cluster = (dirEntry.start_cluster_high << 16) | dirEntry.start_cluster;
    uint32_t content_dir_start = ((start_cluster - 2) * bst->sector_per_clus) + (READ16(bst->reserved_sector_count) + (READ32(bst->sector_per_fat) * bst->fat_count));
    uint8_t buffer[SECTOR_SIZE];
    printf("Noi dung cua file:\n");
    uint32_t bytes_read = 0;

    while (bytes_read < dirEntry.file_size) {
        if (readMultiSector(content_dir_start, 1, buffer) != SECTOR_SIZE) {
            printf("Error reading file content\n");
            return;
        }

        for (uint32_t i = 0; i < SECTOR_SIZE && bytes_read < dirEntry.file_size; i++) {
            if (buffer[i] >= 32 && buffer[i] <= 126) {
                printf("%c", buffer[i]);
            } else {
                printf("."); 
            }
            bytes_read++;
        }
    }

    printf("\n");
}

void chooseEntry(FATBootSector *bst, DirectoryEntry *validEntries, uint32_t validCount) {
    DirectoryEntry *currentEntries = validEntries; 
    uint32_t currentValidCount = validCount;
    while (1) {
        printf("\nNhap vao so thu tu cua file/thu muc (0 de thoat): ");
        int choice;
        scanf("%d", &choice);
        if (choice == -1) {
            return; 
        } else if (choice == 0) {
            if (dirStackIndex == -1) {
                printf("Ban dang o thu muc goc, khong the quay lai!\n");
            } else {
                printf("\nQuay lai thu muc truoc...\n");
                if (popFromStack(&currentEntries, &currentValidCount)) {
                    printf("\nDanh sach cac thu muc va file trong thu muc hien tai:\n");
                    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------+\n");
                    printf("|STT | Name                  | Size (B) | Attr   | Time      | Date               | Cluster    |\n");
                    printf("+----+-----------------------+----------+--------+-----------+--------------------+------------+\n");

                    for (uint32_t i = 0; i < currentValidCount; ++i) {
                        DirectoryEntry dirEntry = currentEntries[i];
                        printf("|%2d  | %-11.11s           | %-8u | 0x%-3X  | %02u:%02u     | %02u/%02u/%04u         | %-6u     |\n", 
                            i + 1, 
                            dirEntry.name, 
                            dirEntry.file_size, 
                            dirEntry.attr, 
                            (dirEntry.time >> 11) & 0x1F, 
                            (dirEntry.time >> 5) & 0x3F, 
                            (dirEntry.date & 0x1F), 
                            (dirEntry.date >> 5) & 0x0F, 
                            ((dirEntry.date >> 9) & 0x7F) + 1980, 
                            dirEntry.start_cluster);
                    }
                } else {
                    printf("Khong co thu muc de quay lai!\n");
                }
                continue;
            }
        } else if (choice > currentValidCount) {
            printf("Lua chon khong hop le!\n");
            continue;
        }

        DirectoryEntry dirEntry = currentEntries[choice - 1];
        if (dirEntry.attr & 0x10) { 
            pushToStack(currentEntries, currentValidCount); 
            printSub(bst, dirEntry);
        } else { 
            printFileContent(bst, dirEntry);
        }
    }
}

int main() 
{
    FATBootSector boot_sector;
    PrintBootSectorInfo(&boot_sector);
    DirectoryEntry dirEntry;

    // Read root directory
    printSub(&boot_sector,dirEntry);
    
    return 0;
}
