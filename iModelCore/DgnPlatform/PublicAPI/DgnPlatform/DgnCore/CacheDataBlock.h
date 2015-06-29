/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/CacheDataBlock.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

enum
{
    K                       = (1024),
    MEG                     = (K*K),
    GIG                     = (K*MEG),
    DEF_PAGESIZE            = (64*K),
    ELEMENT_DATABLOCK_SIZE  = (3*DEF_PAGESIZE),
};

inline void     makeMultipleOf (uint32_t& num, int base) {num = ((num+base-1)/base) * base;}

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#ifdef DGNV10FORMAT_CHANGES_WIP
typedef class  CacheDataBlock*      CacheDataBlockP;
typedef class  CacheDataBlock**     CacheDataBlockH;
typedef struct DgnV8AttrWriter*     DgnV8AttrWriterP;
typedef struct DgnV8AttrReader*     DgnV8AttrReaderP;

struct DgnElemDataRef;

//=======================================================================================
// @bsiclass                                                     Bern.McCarty      06/04
//=======================================================================================
class CacheDataBlock
{
private:
    friend struct DgnElemDataRef;

    static int32_t      s_pageSize;             // MT: safely initialized in CacheDataBlock::StaticInitialize

    Byte*       m_data;
    DgnModelR   m_dgnModel;
    uint32_t    m_numBytes;             // total number of bytes in this block
    uint32_t    m_nextAvailable;        // one past valid data
    bool        m_dirty;

protected:
    void                AllocateMemory();
    void                FreeMemory ();

public:
    void                SetDirty()    {m_dirty = true;}
    void                CloseBlockForWrites();
    DgnModelR           GetModel()                      {return m_dgnModel;}
    uint32_t            GetNumBytes() const             {return m_numBytes;}
    Byte*               GetDataP()                      {return m_data;} // for debugger
    Byte*               GetDataP(int offset)            {return m_data + offset;}
    Byte const*         GetConstPtr (int32_t offset)      {return GetDataP (offset);}
    Byte*               GetWriteablePtr (int32_t offset)  {SetDirty(); return GetDataP (offset);}

    StatusInt ReturnMemory (DgnElemDataRef& elem);

    CacheDataBlock (DgnModelR dgnModel, int size);
    ~CacheDataBlock ();

    static void StaticInitialize();
    static uint32_t GetPageSize () {return s_pageSize;}
    static int numBlocks (int size, int blockSize) {return (size+blockSize-1) / blockSize; }
    static int blockMemSize (int size, int blockSize) {return numBlocks(size,blockSize) * blockSize; }

    //! allocate memory for an element.
    //! @return ERROR if there's not enough memory to fit element in this block.
    StatusInt AllocElemMemory (DgnElemDataRef& elem, uint32_t sizeBytes);
 };

//=======================================================================================
// @bsiclass                                                     Keith.Bentley   02/04
//=======================================================================================
struct DgnElemDataRef
{
    friend class CacheDataBlock;

private:
    DgnElementP      m_ptr;
    uint32_t        m_allocSize;

public:
    DgnElemDataRef () {m_ptr=0; m_allocSize=0;}

    uint32_t    GetAllocSize() {return m_allocSize;}
    void        Allocate (uint32_t size);
    void        Deallocate ();
    DgnElementCP GetPtrC () const {return m_ptr;}
    DgnElementP  GetPtr () {return m_ptr;}
};

#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE
