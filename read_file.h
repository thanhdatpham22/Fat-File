#ifndef READ_F
    #define READ_F
    #include"struct.h"
    /* Function: detectFatType
     * Detects the FAT type of the given boot sector
     * Parameters:
     *   - FATBootSector *bs: Pointer to the boot sector structure.
     */
    int detectFatType(FATBootSector *bs);

    /*
     * Function: PrintBootSectorInfo
     * Prints the information contained in the boot sector.
     * Parameters:
     *   - FATBootSector *bst: Pointer to the boot sector structure.
     *   - int FATType: The FAT type detected (12, 16, or 32).
     */
    int PrintBootSectorInfo(FATBootSector *bst, int FATType );

    /*
     * Function: PrintRootEntries
     * Prints the root directory entries for the specified FAT type.
     * Parameters:
     *   - FATBootSector *bootSector: Pointer to the boot sector structure.
     *   - int FATType: The FAT type detected (12, 16, or 32).
     */
    int PrintRootEntries(FATBootSector *bootSector,int FATType);

      /*
     * Function: printSub
     * Prints the entries of a subdirectory.
     * Parameters:
     *   - FATBootSector *bst: Pointer to the boot sector structure.
     *   - int FATType: The FAT type detected (12, 16, or 32).
     *   - DirectoryEntry dirEntry: The directory entry of the subdirectory to print.
     */
    int printSub (FATBootSector *bst,int FATType, DirectoryEntry dirEntry);

    /*
     * Function: printFileContent
     * Prints the content of a file specified by the directory entry.
     * Parameters:
     *   - FATBootSector *bst: Pointer to the boot sector structure.
     *   - DirectoryEntry dirEntry: The directory entry of the file to print.
     *   - int FATType: The FAT type detected (12, 16, or 32).
     */
    int printFileContent(FATBootSector *bst, DirectoryEntry dirEntry, int FATType);

    /*
     * Function: chooseEntry
     * Allows the user to choose an entry from the directory.
     *
     * Parameters:
     *   - FATBootSector *bst: Pointer to the boot sector structure.
     *   - int FATType: The FAT type detected (12, 16, or 32).
     *   - DirectoryEntry *validEntries: Array of valid directory entries to choose from.
     *   - uint32_t validCount: Number of valid entries.
     */
    int chooseEntry(FATBootSector *bst,int FATType, DirectoryEntry *validEntries, uint32_t validCount);


    /*
     * Function: clusterToSector
     * Converts a cluster number to a corresponding sector number.
     *
     * Parameters:
     *   - FATBootSector *bst: Pointer to the boot sector structure.
     *   - uint32_t currentCluster: The cluster number to convert.
     */
    uint32_t clusterToSector(FATBootSector *bst, uint32_t currentCluster);

      /*
     * Function: getNextCluster
     * Retrieves the next cluster in the chain for the specified cluster.
     *   
     * Parameters:
     *   - FATBootSector *bst: Pointer to the boot sector structure.
     *   - uint32_t currentCluster: The current cluster number.
     */
    uint32_t getNextCluster(FATBootSector *bst, uint32_t currentCluster);
    
    
#endif /* READ_F */