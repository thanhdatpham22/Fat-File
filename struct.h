#ifndef STRUCT
    #define STRUCT
    #include <stdint.h>
    #pragma pack(push, 1)
    /*******************************************************************************
     * Definitions
    ******************************************************************************/
    typedef union
    {
        struct 
        {
            uint8_t drive_number;                  /* Drive number (1 byte) */
            uint8_t reserved1;                     /* Reserved (1 byte) */
            uint8_t extend_boot_signature;         /* Extended boot signature (1 byte) */
            uint8_t volume_id[4];                  /* Volume ID (4 bytes) */
            uint8_t volume_label[11];              /* Volume label (11 bytes) */
            uint8_t fat_type[8];                   /* FAT type (8 bytes) */
            uint8_t boot_strap[448];               /* Boot strap (448 bytes) */
            uint8_t signature[2];                  /* Signature (2 bytes) */

        } fat12_16; /* Structure for FAT12 and FAT16 */

        struct 
        {
            uint8_t sector_per_fat[4];             /* Sectors per FAT (4 bytes) */
            uint8_t mirror_flag[2];                 /* Mirror flag (2 bytes) */
            uint8_t fat32_version[2];               /* FAT32 version (2 bytes) */
            uint8_t start_clus_index[4];           /* Start cluster index (4 bytes) */
            uint8_t fsinfor_sector_count[2];        /* FS Information sector count (2 bytes) */
            uint8_t boot_sector_coppy_index[2];     /* Boot sector copy index (2 bytes) */
            uint8_t reserved[12];                   /* Reserved (12 bytes) */

            uint8_t drive_number;                   /* Drive number (1 byte) */
            uint8_t reserved1;                      /* Reserved (1 byte) */
            uint8_t extend_boot_signature;          /* Extended boot signature (1 byte) */
            uint8_t volume_id[4];                   /* Volume ID (4 bytes) */
            uint8_t volume_label[11];               /* Volume label (11 bytes) */
            uint8_t fat_type[8];                    /* FAT type (8 bytes) */
            uint8_t boot_strap[420];                /* Boot strap (420 bytes) */
            uint8_t signature[2];                   /* Signature (2 bytes) */
        } fat32; /* Structure for FAT32 */
    } FatBiosParameterExtend; /* Union for BIOS Parameter Block extension */

    typedef struct
    {
        uint8_t jmp_boot[3];                     /* Boot jump instruction (3 bytes) */
        uint8_t oem_name[8];                     /* OEM Name (8 bytes) */
        uint8_t byte_per_sector[2];              /* Bytes per sector (2 bytes) */
        uint8_t sector_per_clus;                 /* Sectors per cluster (1 byte) */
        uint8_t reserved_sector_count[2];        /* Reserved sectors (2 bytes) */
        uint8_t fat_count;                        /* Number of FATs (1 byte) */
        uint8_t root_entries_count[2];           /* Maximum root directory entries (2 bytes) */
        uint8_t total_sector[2];                  /* Total sectors (16-bit) (2 bytes) */
        uint8_t volume_type;                      /* Media descriptor (1 byte) */
        uint8_t sector_per_fat[2];               /* Sectors per FAT (2 bytes) */
        uint8_t sector_per_track[2];             /* Sectors per track (2 bytes) */
        uint8_t disk_head_count[2];              /* Number of heads (2 bytes) */
        uint8_t hidden_sector_count[4];          /* Hidden sectors (4 bytes) */
        uint8_t total_sector_32[4];              /* Total sectors (32-bit) (4 bytes) */
        FatBiosParameterExtend ebpd;             /* Extended BIOS Parameter Block */

    } FATBootSector; /* Structure for FAT Boot Sector */

    typedef struct 
    {
        uint8_t name[8];                         /* File name (8 bytes) */
        uint8_t extend[3];                       /* File extension (3 bytes) */
        uint8_t attr;                            /* File attributes (1 byte) */
        uint8_t reserved;                        /* Reserved (1 byte) */
        uint8_t create_time_tenth;              /* Creation time tenths (1 byte) */
        uint16_t create_time;                   /* Creation time (2 bytes) */
        uint16_t create_date;                   /* Creation date (2 bytes) */
        uint16_t last_access_date;              /* Last access date (2 bytes) */
        uint16_t start_cluster_high;            /* High 16 bits of the start cluster (2 bytes) */
        uint16_t time;                          /* Last write time (2 bytes) */
        uint16_t date;                          /* Last write date (2 bytes) */
        uint16_t start_cluster;                 /* Start cluster (2 bytes) */
        uint32_t file_size;                     /* File size (4 bytes) */

    } DirectoryEntry; /* Structure for directory entry */

    #pragma pack(pop)


#endif /*STRUCT*/