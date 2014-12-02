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
    byte *fileBaseP;
    uintptr_t  imageBase;
    UInt32  sizeOfImage;
    UInt32  numberOfSections;
    IMAGE_SECTION_HEADER *imageSectionHeaderP;
    UInt32 debugDirectoryCount;
    IMAGE_DEBUG_DIRECTORY *imageDebugDirectoryP;
    IMAGE_COFF_SYMBOLS_HEADER *imageCoffSymbolsHeaderP;
    byte *codeViewBaseP;
    FPO_DATA *fpoDataP;
    UInt32 fpoCount;
    IMAGE_DEBUG_MISC *imageDebugMiscP;
    IMAGE_NT_HEADERS *imageNTHeadersP;
    IMAGE_RUNTIME_FUNCTION_ENTRY *imageRuntimeFunctionEntryP;
    UInt32 imageRuntimeFunctionCount;
    byte *exportBaseP;
    IMAGE_EXPORT_DIRECTORY *imageExportDirectoryP;
    UInt32 *imodBaseA;
    UInt32 checkSum;
    UInt32 imodCount;
    byte *importBaseP;
    IMAGE_IMPORT_DESCRIPTOR * imageImportDescriptorP;
    byte *delayImportBaseP;
    IMAGE_DELAY_IMPORT_DESCRIPTOR * imageDelayImportDescriptorP;
    UInt32 timeDateStamp;
    byte *resourceBaseP;
    IMAGE_RESOURCE_DIRECTORY *imageResourceDirectoryP;
} ReadDbgHandle;

#endif /* __readdbghH__ */
