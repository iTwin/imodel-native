#ifndef   __readdbghH__
#define   __readdbghH__

#include <Bentley/Bentley.h>

// (reverse engineered) delay import
typedef struct _IMAGE_DELAY_IMPORT_DESCRIPTOR
    {
    DWORD Characteristics;
    DWORD Name;
    DWORD Hmodule;
    DWORD ImportAddressTable;
    DWORD Array;
    DWORD Reserved1;
    DWORD Reserved2;
    DWORD Reserved3;
    } IMAGE_DELAY_IMPORT_DESCRIPTOR;

typedef struct readDbgHandle
{
    char filename[512];
    intptr_t hFileMapping;
    Byte *fileBaseP;
    uintptr_t  imageBase;
    uint32_t sizeOfImage;
    uint32_t numberOfSections;
    IMAGE_SECTION_HEADER *imageSectionHeaderP;
    uint32_t debugDirectoryCount;
    IMAGE_DEBUG_DIRECTORY *imageDebugDirectoryP;
    IMAGE_COFF_SYMBOLS_HEADER *imageCoffSymbolsHeaderP;
    Byte *codeViewBaseP;
    FPO_DATA *fpoDataP;
    uint32_t fpoCount;
    IMAGE_DEBUG_MISC *imageDebugMiscP;
    IMAGE_NT_HEADERS *imageNTHeadersP;
    IMAGE_RUNTIME_FUNCTION_ENTRY *imageRuntimeFunctionEntryP;
    uint32_t imageRuntimeFunctionCount;
    Byte *exportBaseP;
    IMAGE_EXPORT_DIRECTORY *imageExportDirectoryP;
    uint32_t *imodBaseA;
    uint32_t checkSum;
    uint32_t imodCount;
    Byte *importBaseP;
    IMAGE_IMPORT_DESCRIPTOR * imageImportDescriptorP;
    Byte *delayImportBaseP;
    IMAGE_DELAY_IMPORT_DESCRIPTOR * imageDelayImportDescriptorP;
    uint32_t timeDateStamp;
    Byte *resourceBaseP;
    IMAGE_RESOURCE_DIRECTORY *imageResourceDirectoryP;
} ReadDbgHandle;

#endif /* __readdbghH__ */
