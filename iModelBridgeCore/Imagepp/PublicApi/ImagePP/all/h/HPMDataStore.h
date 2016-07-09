//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMDataStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once


#ifdef NOT_NOW_ZLIB
#error zlib should not be included or redefined in a header file.  How about HCDCodecZlib?

#if 0
#include "ZLib.h"
#else

#define Z_OK            0
typedef unsigned char  Byte;  /* 8 bits */
typedef unsigned long  uLong; /* 32 bits or more */

#if defined (_HDLL_SUPPORT)

inline int compress2 (Byte* dest,   uLong* destLen,
                      const Byte* source, uLong sourceLen, int level)
    {
    HASSERT(!"Fix me");
    return 1;
    }

inline int uncompress (Byte* dest,   uLong* destLen,
                       const Byte* source, uLong sourceLen)
    {
    HASSERT(!"Fix me");
    return 1;
    }


#else

// #include "ZConf.h"

#        define ZEXTERN extern __declspec(dllexport)
//#        define ZEXTERN extern
# define ZEXPORT
#  define compress2             z_compress2
#  define uncompress            z_uncompress

extern "C" {

    ZEXTERN int ZEXPORT compress2 (Byte* dest,   uLong* destLen,
                                   const Byte* source, uLong sourceLen,
                                   int level);
    ZEXTERN int ZEXPORT uncompress (Byte* dest,   uLong* destLen,
                                    const Byte* source, uLong sourceLen);
    }

#endif // _HDLL_SUPPORT

#endif
#endif


BEGIN_IMAGEPP_NAMESPACE
/** --------------------------------------------------------------------------------------------------------
 The present class is the ancester from which must inherit any generic block Identifier that cannot
 be represented as an integer. The class must implement all of the 5 methods below.
 The IsValid() override is optional sicne the default implementation assumes that if a generic ID exist
 it is valid.
 The generic block ID will be copied (Clone()) quite often so it should be the lightest weight possible
 and many copies of the same generic block ID must refer to the same data block.
    --------------------------------------------------------------------------------------------------------
*/
class HPMGenericBlockID
    {
public:
    HPMGenericBlockID() {}
    virtual ~HPMGenericBlockID() {}
    virtual HPMGenericBlockID* Clone() const = 0;
    virtual HPMGenericBlockID& operator=(const HPMGenericBlockID& blockID) = 0;
    virtual bool operator==(const HPMGenericBlockID& blockID) const = 0;
    virtual bool IsValid() const {
        return true;
        }
    };

/** --------------------------------------------------------------------------------------------------------
 This class serves as an identifier of data blocks. It provides a long long integer
 for storage of the implementation specific ID or provides an additional pointer to a generic block id
 To use, if the data store can identify data blocks using an integer then the m_integerID should be used
 If the datablock cannot be identified using an integer, then a custom made class deriving from IHPMDataStore::GenericBlockID
 must be implemented by the specific store and provided at the creation of the BlockID. The created blockID
 retains full ownership of this generic block ID and will delete it whenever the block ID is destroyed. It follows
 that when a block ID is copied (which may happen often enough) then the generic ID will be cloned using the
 Clone() method.

 The way the blockID must be used, is typically to encapsulate an integer which interpretation of is store
 implementation specific. For a generic store, the block ID would be based on an integer representing
 the file offset from the start. For a TIFF based store, the integer identifier would represent the
 identifier of the tile or TIFF block to access. For other storage, the interpretation of the integer, if
 one is used or of the generic block identifier if pertinent is up to the store it is used in conjunction with.
 Usually, the store mechansim will be in charge of creation of Block IDs which will be transported by data
 containers till given to store for access, replacement or deletion of related data.
    --------------------------------------------------------------------------------------------------------
*/
class HPMBlockID
    {
public:
    /**----------------------------------------------------------------------------
     Default constructor.
     It creates a uninitialised block ID. It is an appropriate default value
     for most usage of block IDs
    -----------------------------------------------------------------------------*/
    HPMBlockID()
        {
        m_integerInitialized = false;
        m_alternateID = NULL;
        m_integerID = 0;
        }
    /**----------------------------------------------------------------------------
     Constructor.
     It creates an initialised integer based block ID.
    -----------------------------------------------------------------------------*/
    HPMBlockID(int64_t ID)
        {
        m_integerID = ID;
        m_integerInitialized = true;
        m_alternateID = NULL;
        }
    /**----------------------------------------------------------------------------
     Copy Constructor.
     The copy refers to the same storage location.
    -----------------------------------------------------------------------------*/
    HPMBlockID(const HPMBlockID& blockID)
        {
        m_integerID = blockID.m_integerID;
        m_integerInitialized = blockID.m_integerInitialized;
        m_alternateID = NULL;
        if (blockID.m_alternateID != NULL)
            m_alternateID = blockID.m_alternateID->Clone();
        }
    /**----------------------------------------------------------------------------
     Destroyer
     If a generic identifier object is used it will be deleted.
    -----------------------------------------------------------------------------*/
    ~HPMBlockID()
        {
        if (m_alternateID != NULL)
            delete m_alternateID;
        }

    /**----------------------------------------------------------------------------
     Assignment operator
     The assignee then identifies the same storage block as given block
    -----------------------------------------------------------------------------*/
    HPMBlockID& operator=(const HPMBlockID& blockID)
        {
        if (m_alternateID != NULL)
            delete m_alternateID;
        m_alternateID = NULL;

        m_integerID = blockID.m_integerID;
        m_integerInitialized = blockID.m_integerInitialized;
        if (blockID.m_alternateID != NULL)
            m_alternateID = blockID.m_alternateID->Clone();

        return *this;
        }

    /**----------------------------------------------------------------------------
     Integer based assignement operator
     For integer based block IDs
    -----------------------------------------------------------------------------*/
    HPMBlockID& operator=(int64_t integerID)
        {
        if (m_alternateID != NULL)
            delete m_alternateID;
        m_alternateID = NULL;

        m_integerID = integerID;
        m_integerInitialized = true;
        return *this;
        }

    /**----------------------------------------------------------------------------
     equality operator
     Returns true if the block IDs represent the same block. Since the
     block ID does not know the store it is related to, the compare operation
     must be performed for pertinent block ID refering to the same store.

    If either block ID is uninitialised then false is returned.
    -----------------------------------------------------------------------------*/
    bool operator==(const HPMBlockID& blockID)
        {
        // If any of the ID uses an alternate generic ID then both should
        // and both should be equal
        if (m_alternateID != NULL)
            {
            if (blockID.m_alternateID != NULL)
                return ((*m_alternateID) == (*(blockID.m_alternateID)));
            else
                return false;
            }
        else
            {
            // If no generic ID used then both ID must be initialized
            if (!m_integerInitialized || !blockID.m_integerInitialized)
                return false;

            return (m_integerID == blockID.m_integerID);
            }
        }
    bool operator!=(const HPMBlockID& blockID)
        {
        return !operator==(blockID);
        }
    /**----------------------------------------------------------------------------
     Indicates if the block ID is valid/initialized
    -----------------------------------------------------------------------------*/
    bool IsValid() const
        {
        if (m_alternateID != NULL)
            return m_alternateID->IsValid();

        return m_integerInitialized;
        }

    // The members are public and users may set them at will given they
    // understand how the block ID behaves.
    bool              m_integerInitialized;
    int64_t           m_integerID;
    HPMGenericBlockID*   m_alternateID;
    };


/** --------------------------------------------------------------------------------------------------------
 This important base class is introduced to  provide a way for multiple inherited data store of different argument type to
 share the same ancester implementation of the shareable object
    --------------------------------------------------------------------------------------------------------
*/

class IHPMBaseDataStore: public HFCShareableObject<IHPMBaseDataStore>
    {
public:
    IHPMBaseDataStore() {};
    virtual ~IHPMBaseDataStore () {};
    };

/** --------------------------------------------------------------------------------------------------------
 This interface defines the methods required for a base data store. In short, a base data store uses block
 IDs (HPMBlockID) to identify blocks of data of type DataType. Blocks of data can be stored, loaded
 or removed from store.
    --------------------------------------------------------------------------------------------------------
*/
template <typename DataType> class IHPMDataStore: virtual public IHPMBaseDataStore
    {


public:

    IHPMDataStore() {};
    virtual ~IHPMDataStore () {};

    /**----------------------------------------------------------------------------
     Closes the store. After closure it is not possible to load or store data.
    -----------------------------------------------------------------------------*/
    virtual void Close () = 0;

    /**----------------------------------------------------------------------------
     Stores a new block ... The block ID is returned
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreNewBlock (DataType* DataTypeArray, size_t countData) = 0;

    /**----------------------------------------------------------------------------
     Stores an existing block ... if the current block size is not sufficient to hold the data, then
     a new block can be allocated elsewhere. The new or previous block ID is returned
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreBlock (DataType* DataTypeArray, size_t countData, HPMBlockID blockID) = 0;

    /**----------------------------------------------------------------------------
     Returns the current block size ... this size can be 0
    -----------------------------------------------------------------------------*/
    virtual size_t GetBlockDataCount (HPMBlockID blockID) const = 0;

    /**----------------------------------------------------------------------------
     Loads data type block designated. The method will not load more data
     than can be held in provided buffer. To know the required buffer size prior
     to loading the block, the method GetBlockDataCount() can be used.
    -----------------------------------------------------------------------------*/
    virtual size_t LoadBlock (DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID) = 0;

    /**----------------------------------------------------------------------------
     Removes the block from the store.
    -----------------------------------------------------------------------------*/
    virtual bool DestroyBlock (HPMBlockID blockID) = 0;
    };







/** --------------------------------------------------------------------------------------------------------
 The IPermanentDataStore defines an interface that inherits from the generic data store but
 recognises the fact some control information must also be stored along the data as well as information concerning
 the whole dataset. The interface provides the means to store a generic header along the data block and a master
 header applicable to the whole dataset of file. The result interface provide the basis for
 a permanent storage implementation.
    --------------------------------------------------------------------------------------------------------
*/
template <typename DataType, typename MasterHeaderType, typename TileHeaderType> class IHPMPermanentStore: public IHPMDataStore<DataType>
    {
public:
    // Constructor ...
    // Either creates or opens specified file according to the fact it already exists or not.
    IHPMPermanentStore() {};
    ~IHPMPermanentStore () {};


    /**----------------------------------------------------------------------------
     Stores the master header in the store. The master header is of an undefined type
     but should contain all information pertinent to the whole store.
     The header size provided may be useful for some store implementation but is usually
     not used by most store. The interpretation of the headerSize is implementation
     specific but initially was meant to indicate the number of bytes contained in the
     header.
    -----------------------------------------------------------------------------*/
    virtual bool StoreMasterHeader (MasterHeaderType* header, size_t headerSize) = 0;

    /**----------------------------------------------------------------------------
     Loads the master header from the store. The master header is of an undefined type
     but should contain all information pertinent to the whole store.
     The maximum header size provided may be useful for some store implementation but is usually
     not used. The max header size is used in some generic store to indicate the number
     of bytes available in the header structure. The interpretation of this parameter is implementation
     specific but initially was meant to indicate the number of bytes contained in the
     header.
    -----------------------------------------------------------------------------*/
    virtual size_t LoadMasterHeader (MasterHeaderType* header, size_t maxHeaderSize) = 0;

       
    /**----------------------------------------------------------------------------
     Stores the block header in the store. The block header is of an undefined type
     but should contain all information pertinent to the designated block except the
     block of data of type DataType.
    -----------------------------------------------------------------------------*/
    virtual size_t StoreHeader (TileHeaderType* header, HPMBlockID blockID) = 0;

    /**----------------------------------------------------------------------------
     Loads the block header in the store. The block header is of an undefined type
     but should contain all information pertinent to the designated block except the
     block of data of type DataType.
    -----------------------------------------------------------------------------*/
    virtual size_t LoadHeader (TileHeaderType* header, HPMBlockID blockID) = 0;

    // IHPMDataStore implementation
    virtual void Close () = 0;

    virtual HPMBlockID StoreNewBlock (DataType* DataTypeArray, size_t countData) = 0;

    virtual HPMBlockID StoreBlock (DataType* DataTypeArray, size_t countData, HPMBlockID blockID) = 0;

    virtual size_t GetBlockDataCount (HPMBlockID blockID) const = 0;

    virtual size_t LoadBlock (DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID) = 0;

    virtual bool DestroyBlock (HPMBlockID blockID) = 0;

    };



/** --------------------------------------------------------------------------------------------------------
 The generic data store is provided as a quick and dirty implementation of a permanent data store file format
 that will store blocks of atomic fixed size data types and headers of fixed sizes. block IDs are simple
 file offsets and no specific file format is assigned. The implementation stores the master header
 at the start of the file, so the master header MUST be stored once immediately after the creation and
 prior to storing any blocks of data. The implementation does not managed unused regions of the file but will
 reuse block location if rewritten/modified given the block location is of sufficient size.
    --------------------------------------------------------------------------------------------------------
*/
template <class DataType> class HPMGenericDataStore: public IHPMPermanentStore<DataType, Byte, Byte>
    {
public:
    /**----------------------------------------------------------------------------
     Creates a generic data store. If the file already exist then it will be opened.
     If the file does not exist then it will be created. If it is a new file then
     the user of the generic data store should immediately store the master header.

     Since the size of the block header cannot be determined, it is provided at the
     time of the construction.
    -----------------------------------------------------------------------------*/
    HPMGenericDataStore(const char* fileName, size_t tileHeaderSize)
        {
        // Open
        m_File = fopen (fileName, "r+b");
        if (m_File == NULL)
            {
            // File does not exist ... create it
            m_File = fopen (fileName, "w+b");
            if (m_File == NULL)
                {
                // Unknown error
                throw;
                }

            // Write magic number
            size_t toto = 213255;
            fwrite (&toto, sizeof(size_t), 1, m_File);
            }

        m_open = true;
        m_tileHeaderSize = tileHeaderSize;

        }


    /**----------------------------------------------------------------------------
     Destroyer
     If the file is opened then it will be closed automatically.
    -----------------------------------------------------------------------------*/
    ~HPMGenericDataStore ()
        {
        if (m_open)
            Close();
        }


    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    void Close ()
        {
        fclose (m_File);
        m_open = false;
        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual bool StoreMasterHeader (Byte* header, size_t headerSize)
        {
        if (!m_open)
            return false;

        // header size cannot be 0
        if (0 == headerSize)
            return false;

        // Go to start of file
        _fseeki64 (m_File, (int64_t)8, SEEK_SET);

        // Store header size
        if (1 != fwrite (&headerSize, sizeof (headerSize), 1, m_File))
            return false;

        // Store header
        if (1 != fwrite (header, headerSize, 1, m_File))
            return false;

        return true;

        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual size_t LoadMasterHeader (Byte* header, size_t maxHeaderSize)
        {
        if (!m_open)
            return 0;

        // Go to current block position
        if (0 !=  _fseeki64 (m_File, 8, SEEK_SET))
            return 0;

        // Load header size
        size_t headerSize;
        if (1 != fread(&headerSize , sizeof(size_t), 1, m_File))
            return 0;

        if (headerSize > maxHeaderSize)
            headerSize = maxHeaderSize;

        if (headerSize == 0)
            return 0;

        // Load header
        if (1 != fread(header, headerSize, 1, m_File))
            return 0;


        return headerSize;
        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreNewBlock (DataType* DataTypeArray, size_t countData)
        {
        if (!m_open)
            return HPMBlockID();

        // Go to end of file
        _fseeki64 (m_File, (int64_t)0, SEEK_END);

        return StoreBlockAtCurrentLocation (DataTypeArray, countData);


        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreBlock (DataType* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        if (!m_open)
            return HPMBlockID();

        // If block ID is 0 then we store new
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            {
            return StoreNewBlock (DataTypeArray, countData);
            }

        // Load current block size
        size_t currentBlockCount = GetBlockDataCount(blockID);

        // Make sure that the current block is large enough
        // if it is not large enough ... call StoreNew()
        if (currentBlockCount < countData)
            return StoreNewBlock (DataTypeArray, countData);
        else
            {
            // Go to current block position
            if (0 !=  _fseeki64 (m_File, blockID.m_integerID, SEEK_SET))
                return HPMBlockID();


            return StoreBlockAtCurrentLocation (DataTypeArray, countData);

            }

        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual size_t GetBlockDataCount (HPMBlockID blockID) const
        {
        if (!m_open)
            return 0;

        // Go to file location
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            return 0;

        // Go to current block position
        if (0 !=  _fseeki64 (m_File, blockID.m_integerID, SEEK_SET))
            return 0;

        size_t tileHeaderSize;
        if (1 != fread(&tileHeaderSize , sizeof(size_t), 1, m_File))
            return 0;

        // Go to data position
        if (0 !=  _fseeki64 (m_File, blockID.m_integerID + tileHeaderSize + sizeof(size_t), SEEK_SET))
            return 0;

        size_t currentCount;
        if (1 != fread(&currentCount , sizeof(size_t), 1, m_File))
            return 0;

        return currentCount;

        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual size_t StoreNodeHeader (Byte* header, HPMBlockID blockID)
        {
        if (!m_open)
            return 0;

        // Go to location of block
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            return 0;

        // Go to current block position
        if (0 !=  _fseeki64 (m_File, blockID.m_integerID, SEEK_SET))
            return 0;

        // validate tile header size

        // Store header size
        if (1 != fwrite(&m_tileHeaderSize , sizeof(m_tileHeaderSize), 1, m_File))
            return 0;

        if (m_tileHeaderSize > 0)
            {
            // Load header
            if (1 != fwrite(header, m_tileHeaderSize, 1, m_File))
                return 0;
            }

        return m_tileHeaderSize;
        }


    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual size_t LoadNodeHeader (Byte* header, HPMBlockID blockID)
        {
        if (!m_open)
            return 0;

        // Go to location of block
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            return 0;

        // Go to current block position
        if (0 !=  _fseeki64 (m_File, blockID.m_integerID, SEEK_SET))
            return 0;

        // Load header size
        size_t tileHeaderSize;
        if (1 != fread(&tileHeaderSize , sizeof(size_t), 1, m_File))
            return 0;

        HASSERT (tileHeaderSize == m_tileHeaderSize);

        if (tileHeaderSize > 0)
            {
            // Load header
            if (1 != fread(header, tileHeaderSize, 1, m_File))
                return 0;
            }

        return tileHeaderSize;
        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual size_t LoadBlock (DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        if (!m_open)
            return 0;

        // Go to location of block
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            return 0;

        // Load size (THIS POSITIONS UPON DATA LOCATION)
        size_t currentCount = GetBlockDataCount(blockID);

        // Verify that the maxCount is bigger
        if (currentCount > maxCountData)
            currentCount = maxCountData;

        // Load the data
        if (currentCount != fread(DataTypeArray, sizeof(DataType), currentCount, m_File))
            return 0;

        return currentCount;
        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual bool DestroyBlock (HPMBlockID blockID)
        {
        // For the generic store the way to destory a block is simply to abandon it
        // Nothing is done
        return true;
        }

protected:

    /**----------------------------------------------------------------------------
     PROTECTED METHOD
     Creates a generic data store. This constructor is usefull to inherit from present
     class yet prevent the automatic creation of the file if it does not exist.
     Since the size of the block header cannot be determined, it is provided at the
     time of the construction.
    -----------------------------------------------------------------------------*/
    HPMGenericDataStore(size_t tileHeaderSize)
        {
        m_tileHeaderSize = tileHeaderSize;
        }
    HPMGenericDataStore() {};

    /**----------------------------------------------------------------------------
     PROTECTED METHOD

     Provides a unique location for implementation of both StoreNewBlock() and
     StoreBlock(). Both will call this method after block location has been validated
     as sufficient in size or a new block has been created.
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreBlockAtCurrentLocation (DataType* DataTypeArray, size_t countData)
        {
        if (!m_open)
            return HPMBlockID();

        HPMBlockID blockID = _ftelli64 (m_File);

        // Store header size
        if (1 != fwrite (&m_tileHeaderSize, sizeof (m_tileHeaderSize), 1, m_File))
            return HPMBlockID();

        if (m_tileHeaderSize > 0)
            {
            Byte* tileHeaderPadding = (Byte*)malloc(m_tileHeaderSize * sizeof (Byte) + 1);

            // Store header padding
            if (1 != fwrite (tileHeaderPadding, m_tileHeaderSize, 1, m_File))
                {
                free(tileHeaderPadding);
                return HPMBlockID();
                }

            free(tileHeaderPadding);

            }

        // Store data count
        if (1 != fwrite (&countData, sizeof (countData), 1, m_File))
            return HPMBlockID();

        // Store new tile
        if (countData != fwrite (DataTypeArray, sizeof(DataType), countData, m_File))
            return HPMBlockID();

        // Return block ID (offset)
        return blockID;
        }


    // The file
    FILE* m_File;

    // Boolean indicating if the file is already opened.
    bool m_open;

    // The size of the block header
    size_t m_tileHeaderSize;
    };





/** --------------------------------------------------------------------------------------------------------
 This generic data store compressed data blocks using the deflate compression. No compression is applied
 to master header or to block header
    --------------------------------------------------------------------------------------------------------
*/
template <class DataType> class HPMGenericCompressedDataStore : public HPMGenericDataStore<DataType>
    {
public:
    HPMGenericCompressedDataStore(const char* fileName, size_t tileHeaderSize, int compressionFactor)
        : HPMGenericDataStore (tileHeaderSize)
        {
        // Open
        m_File = fopen (fileName, "r+b");
        if (m_File == NULL)
            {
            // File does not exist ... create it
            m_File = fopen (fileName, "w+b");
            if (m_File == NULL)
                {
                // Unknown error
                throw;
                }

            // Write magic number
            size_t toto = 213256;
            fwrite (&toto, sizeof(size_t), 1, m_File);
            }

        m_open = true;
        m_compressionFactor = compressionFactor;

        }
    ~HPMGenericCompressedDataStore ()
        {
        if (m_open)
            fclose (m_File);
        }
    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    void Close ()
        {
        fclose (m_File);
        m_open = false;
        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreBlockAtCurrentLocation (DataType* DataTypeArray, size_t countData)
        {
        if (!m_open)
            return HPMBlockID();

        HPMBlockID blockID = _ftelli64 (m_File);

        // Store header size
        if (1 != fwrite (&m_tileHeaderSize, sizeof (m_tileHeaderSize), 1, m_File))
            return HPMBlockID();

        if (m_tileHeaderSize > 0)
            {
            Byte* tileHeaderPadding = (Byte*)malloc(m_tileHeaderSize * sizeof (Byte) + 1);

            // Store header padding
            if (1 != fwrite (tileHeaderPadding, m_tileHeaderSize, 1, m_File))
                {
                free(tileHeaderPadding);
                return HPMBlockID();
                }

            free(tileHeaderPadding);
            }

        // Store data count
        if (1 != fwrite (&countData, sizeof (countData), 1, m_File))
            return HPMBlockID();


        // Compress memory
        Byte* compressedMemory = (Byte*)malloc (sizeof(DataType)*countData + 100);
        if (compressedMemory == NULL)
            return HPMBlockID();

        uint32_t OutLen = (uint32_t)sizeof(DataType)*countData + 100;
        int err;


        // Use level 5 compression. Varies from 1 to 9, 6 being the default.
        // There seems to be a big difference between 5 and 6 on execution time for
        // big images, with only a small size penalty.
        err = compress2(compressedMemory, &OutLen, (Byte*)DataTypeArray, (uint32_t)(countData*sizeof(DataType)), m_compressionFactor);

        if(err != Z_OK)
            {
            free(compressedMemory);
            return HPMBlockID();
            }

        // Store compressed data size
        size_t sizedOutLen = OutLen;
        if (1 != fwrite (&sizedOutLen, sizeof (sizedOutLen), 1, m_File))
            return HPMBlockID();

        // Store new tile
        if (OutLen != fwrite (compressedMemory, sizeof(Byte), OutLen, m_File))
            return HPMBlockID();

        // free compressed memory
        free (compressedMemory);

        // Return block ID (offset)
        return blockID;

        }

    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual HPMBlockID StoreBlock (DataType* DataTypeArray, size_t countData,  HPMBlockID blockID)
        {
        if (!m_open)
            return HPMBlockID();

        // If block ID is 0 then we store new
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            {
            return StoreNewBlock (DataTypeArray, countData);
            }

        // Load current block size
        size_t currentBlockCount = GetBlockDataCount(blockID);

        // Make sure that the current block is large enough
        // if it is not large enough ... call StoreNew()
        if (currentBlockCount < countData)
            return StoreNewBlock (DataTypeArray, countData);
        else
            {

            // Compress memory
            Byte* compressedMemory = (Byte*)malloc (sizeof(DataType)*countData + 100);
            if (compressedMemory == NULL)
                return HPMBlockID();

            uint32_t OutLen = (uint32_t)sizeof(DataType)*countData + 100;
            int err;


            // Use level 5 compression. Varies from 1 to 9, 6 being the default.
            // There seems to be a big difference between 5 and 6 on execution time for
            // big images, with only a small size penalty.
            err = compress2(compressedMemory, &OutLen, (Byte*)DataTypeArray, (uint32_t)(countData*sizeof(DataType)), m_compressionFactor);

            if(err != Z_OK)
                {
                free(compressedMemory);
                return HPMBlockID();
                }

            // Load the compressed size
            // Load compressed size
            size_t sizedOutLen;
            if (1 != fread (&sizedOutLen, sizeof(size_t), 1, m_File))
                {
                free(compressedMemory);
                return HPMBlockID();
                }

            // Check if current length is sufficient for the new length
            if (sizedOutLen < OutLen)
                {
                free(compressedMemory);
                return StoreNewBlock (DataTypeArray, countData);
                }
            else
                {
                // Go to current block position
                if (0 !=  _fseeki64 (m_File, blockID.m_integerID, SEEK_SET))
                    return HPMBlockID();

                // Store header size
                if (1 != fwrite (&m_tileHeaderSize, sizeof (m_tileHeaderSize), 1, m_File))
                    return HPMBlockID();

                if (m_tileHeaderSize > 0)
                    {
                    Byte* tileHeaderPadding = (Byte*)malloc(m_tileHeaderSize * sizeof (Byte) + 1);

                    // Store header padding
                    if (1 != fwrite (tileHeaderPadding, m_tileHeaderSize, 1, m_File))
                        {
                        free(tileHeaderPadding);
                        return HPMBlockID();
                        }

                    free(tileHeaderPadding);
                    }

                // Store data count
                if (1 != fwrite (&countData, sizeof (size_t), 1, m_File))
                    return HPMBlockID();

                // Store compressed data size
                sizedOutLen = OutLen;
                if (1 != fwrite (&sizedOutLen, sizeof (sizedOutLen), 1, m_File))
                    return HPMBlockID();

                // Store new tile
                if (OutLen != fwrite (compressedMemory, sizeof(Byte), OutLen, m_File))
                    return HPMBlockID();

                // free compressed memory
                free (compressedMemory);

                // Return block ID (offset)
                return blockID;
                }
            }

        }



    /**----------------------------------------------------------------------------
     @bsimethod                                          Alain.Robert 2010/10
    -----------------------------------------------------------------------------*/
    virtual size_t LoadBlock (DataType* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        if (!m_open)
            return 0;

        // Go to location of block
        if (!blockID.IsValid() || 0 == blockID.m_integerID)
            return 0;


        // Load size
        size_t currentCount = GetBlockDataCount(blockID);
        // Verify that the maxCount is bigger
        if (currentCount > maxCountData)
            currentCount = maxCountData;

        // Load compressed size
        size_t sizedOutLen;
        if (1 != fread (&sizedOutLen, sizeof(size_t), 1, m_File))
            return 0;

        // Allocated Compressmemory
        Byte* compressedMemory = (Byte*)malloc ((long)sizedOutLen + 100);
        if (compressedMemory == NULL)
            return 0;

        // Load the compressed data
        if ((long)sizedOutLen != fread(compressedMemory, sizeof(Byte), (long)sizedOutLen, m_File))
            return 0;

        // Uncompress
        uint32_t OutLen = (uint32_t)currentCount*sizeof(DataType);
        int err = uncompress((Byte*)DataTypeArray, &OutLen, (Byte*)compressedMemory, (uint32_t)sizedOutLen);


        // free compressed memory
        free (compressedMemory);


        if(err != Z_OK)
            return 0;

        return currentCount;
        }

    int m_compressionFactor;
    };

END_IMAGEPP_NAMESPACE