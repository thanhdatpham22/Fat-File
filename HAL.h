#ifndef HAL_H
    #define HAL_H
    #include "struct.h"

    /*******************************************************************************
     * Function: init_image
     * Description: Initializes the image file and reads the boot sector into
     *              the specified FATBootSector structure.
     * Parameters:
     *   const char *file_path - The path to the image file to be opened.
     *   FATBootSector *bootSector - Pointer to the FATBootSector structure
     *                                where the boot sector information will
     *                                be stored.
     ******************************************************************************/
    int init_image(const char *file_path, FATBootSector *bootSector);


    /*******************************************************************************
     * Function: readSector
     * Description: Reads a single sector from the image file.
     * Parameters:
     *   uint32_t index - The sector index to read from.
     *   uint8_t *buff - Pointer to the buffer where the sector data will be
     *                   stored.
     ******************************************************************************/
    int32_t readSector(uint32_t index, uint8_t *buff);

    /*******************************************************************************
     * Function: readMultiSector
     * Description: Reads multiple sectors from the image file.
     * Parameters:
     *   uint32_t index - The starting sector index to read from.
     *   uint32_t num - The number of sectors to read.
     *   uint8_t *buff - Pointer to the buffer where the sector data will be
     *                   stored.
     ******************************************************************************/
    int32_t readMultiSector(uint32_t index, uint32_t num, uint8_t *buff);

    /*******************************************************************************
     * Function: hal_close
     * Description: Closes the image file and frees any associated resources.
     * Parameters: None.
     ******************************************************************************/
    void hal_close();

#endif /* HAL_H */