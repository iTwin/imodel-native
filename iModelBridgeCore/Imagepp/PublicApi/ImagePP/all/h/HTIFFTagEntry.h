//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFTagEntry.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFTagEntry
//-----------------------------------------------------------------------------

#pragma once

#include "HTagDefinition.h"
#include "HTIFFUtils.h"


BEGIN_IMAGEPP_NAMESPACE
class HTagDefinition;
class HTIFFByteOrdering;
class HTIFFDirectory;

class HTIFFTagEntry
    {
public:
    friend  class HTIFFDirectory;     // Share the protected methods


    // Primary methods

    HTIFFTagEntry(const HTagInfo&           pi_rTagInfo,
                  const HTIFFByteOrdering*  pi_pByteOrder);

    HTIFFTagEntry(const HTagInfo&           pi_rTagInfo,
                  HTagID                    pi_Tag,
                  const HTIFFByteOrdering*  pi_pByteOrder,
                  bool                     pi_IsTiff64=false);
    virtual             ~HTIFFTagEntry();

    bool               IsValid         (HTIFFError**  po_ppError) const;
    HTagID              GetTagID        () const;
    HTagInfo::DataType  GetTagType      () const;
    const char*        GetTagNameString() const;

    void                ValidateTagDataAddress(const HTagInfo& pi_rTagInfo, HTIFFStream* pi_pFile);

    bool               ReadTagEntry (const HTagInfo&   pi_rTagInfo,
                                      HTIFFStream*      pio_pFile);
    bool               ReadData     (const HTagInfo&   pi_rTagInfo,
                                      HTIFFStream*      pio_pFile);    // ReadTagEntry must call before
    bool               WriteTagEntry(const HTagInfo&   pi_rTagInfo,
                                      HTIFFStream*      pio_pFile);    // WriteData must be call before
    bool               WriteData    (const HTagInfo&   pi_rTagInfo,
                                      HTIFFStream*      pio_pFile);
    uint64_t           GetTagOffSet ();
    uint64_t           GetTagSize   ();

    void                SetTagOffSet (uint64_t p_NewOffSet);
    // Others Methods

    void                GetConvertedValues (vector<double>& po_rValues);

    void                GetValues (Byte* po_pVal);
    void                GetValues (unsigned short* po_pVal);
    void                GetValues (uint32_t* po_pVal);
    void                GetValues (double* po_pVal);
    void                GetValues (uint64_t* po_pVal);
    void                GetValues (char** po_ppVal);
    void                GetValues (WChar** po_ppVal);
    void                GetValues (unsigned short* po_pVal1, unsigned short* po_pVal2);
    void                GetValues (uint32_t* po_pVal1, uint32_t* po_pVal2);                     // RATIONAL
    void                GetValues (uint32_t* po_pCount, unsigned short** po_ppVal);
    void                GetValues (uint32_t* po_pCount, uint32_t** po_ppVal);
    void                GetValues (uint32_t* po_pCount, uint64_t** po_ppVal);
    void                GetValues (uint32_t* po_pCount, double** po_ppVal);
    void                GetValues (const HTagInfo& pi_rTagInfo, uint32_t* po_pCount, Byte** po_ppVal);

    bool               SetValues (unsigned short pi_Val);
    bool               SetValues (uint32_t pi_Val);
    bool               SetValues (double pi_Val);
    bool               SetValues (uint64_t pi_Val);
    bool               SetValuesA (const char*  pi_pVal);
    bool               SetValuesW (const WChar*  pi_pVal);
    bool               SetValues (unsigned short pi_Val1,  unsigned short pi_Val2);
    bool               SetValues (uint32_t pi_pVal1, uint32_t pi_pVal2);     // RATIONAL
    bool               SetValues (uint32_t pi_Count, const unsigned short* pi_pVal);
    bool               SetValues (uint32_t pi_Count, const uint32_t* pi_pVal);
    bool               SetValues (uint32_t pi_Count, const uint64_t* pi_pVal);
    bool               SetValues (uint32_t pi_Count, const double* pi_pVal);
    bool               SetValues (const HTagInfo& pi_rTagInfo, uint32_t pi_Count, const Byte* pi_pVal);

    void                Touched   ();

protected:

private:

    struct EntryStatus
        {
        unsigned DataInOffset : 1;      // Data in the Offset Field
        unsigned NeedSwap : 1;          // The value must be Swap

        unsigned Dirty  : 1;            // The tag is modified
        unsigned Resize : 1;            // The size of the Tag has been changed
        };

    typedef struct {
        uint32_t    DirCount;        // Number of items; length in spec
        uint64_t     Offset64;        // Byte offset to field data
        uint32_t    SizeInFile;     // Original size from file.
        Byte*        pData;          // Data if len > 4, else Data is in Offset

        EntryStatus Status;
        } TagEntry64;


    // Members

    HTIFFByteOrdering*      m_pByteOrder;

    HTagDefinition*         m_pTagDef;
    TagEntry64*             m_pEntry;

    HTIFFError*             m_pError;      // Error message

    // Methods

    // Not implemented
    HTIFFTagEntry   (const HTIFFTagEntry& pi_rObj);
    HTIFFTagEntry&          operator=(const HTIFFTagEntry& pi_rObj);


    bool                   ValidateDataLen (uint64_t pi_Count, bool pi_Read=true);

    void                    SwapData        (void* pio_pData);
    void                    SetArray        (uint32_t pi_DirCount, const Byte* pi_pData);


    };
END_IMAGEPP_NAMESPACE

#include "HTIFFTagEntry.hpp"

