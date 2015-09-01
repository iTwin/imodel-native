//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFBlockAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFBlockAdapter
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

//#include "HRFRasterFile.h"
#include "HFCPtr.h"
//#include "HRFResolutionDescriptor.h"
//#include "HFCAccessMode.h"
#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRFRasterFile;

//-----------------------------------------------------------------------------
// This ancestor class define the standard
// interface to query about the supported thing.
// There will be an object that derives from
// this one for each specific Implementation object.
// Each specific implementation of this object add
// only their supported thing to the list.
//-----------------------------------------------------------------------------
class HRFBlockAdapterCapabilities
    {
public:
    // enumeration of Capabilities
    enum Capability
        {
        // Bits
        FROM_LINE  = 1,
        FROM_STRIP = 2,
        FROM_TILE  = 3,
        FROM_IMAGE = 4,

        TO_LINE    = 11,
        TO_STRIP   = 12,
        TO_TILE    = 13,
        TO_IMAGE   = 14,

        TO_N_BLOCK = 21,
        };

    HRFBlockAdapterCapabilities();
    virtual ~HRFBlockAdapterCapabilities();

    // Query about the supported thing of that specific implementation
    virtual bool Supports(HRFBlockAdapterCapabilities::Capability pi_Capability) const;

protected:
    // List of capabilities
    typedef vector<Capability>
    ListOfCapabilities;

    ListOfCapabilities    m_ListOfCapabilities;
    };

//-----------------------------------------------------------------------------
// This ancestor class define the standard interface.
// There will be an object that derives from this one
// for each specific adapter object.
//-----------------------------------------------------------------------------
class HRFBlockAdapter : public HRFResolutionEditor, public HRFBlockAdapterCapabilities
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    // friend class HRFRasterFile;
    HRFBlockAdapter (
        HRFBlockAdapterCapabilities* pi_pCapabilities,
        HFCPtr<HRFRasterFile>        pi_rpRasterFile,
        uint32_t                     pi_Page,
        unsigned short              pi_Resolution,
        HFCAccessMode                pi_AccessMode);

    virtual                            ~HRFBlockAdapter();

    // This ancestor implement the Wrapper on the Capabilities
    // Query about the supported thing of that specific implementation
    virtual bool Supports(HRFBlockAdapterCapabilities::Capability pi_Capability) const;

    virtual const HRFResolutionEditor* GetAdaptedResolutionEditor() const;

    IMAGEPP_EXPORT virtual void             SetPalette    (const HRPPixelPalette& pi_rPalette);

    // Used by HRSObjectStore to synchronize TileDataFlag before the save.
    virtual void                    SaveDataFlag();

protected:
    HAutoPtr<HRFResolutionEditor> m_pAdaptedResolutionEditor;
    HFCPtr<HRFRasterFile>         m_pTheTrueOriginalFile;

    HRFBlockAdapterCapabilities*  m_pCapabilities; // no responsible for the destruction of this pointer

private:
    // Methods Disabled
    HRFBlockAdapter(const HRFBlockAdapter& pi_rObj);
    HRFBlockAdapter& operator=(const HRFBlockAdapter& pi_rObj);
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific object.
// There will be an object that derives from this one for each specific object.
// It is used by the Block Adapter factory.
//-----------------------------------------------------------------------------
class HRFBlockAdapterCreator
    {
public:
    // Obtain the capabilities of stretcher
    virtual HRFBlockAdapterCapabilities* GetCapabilities() const = 0;

    // Creation of this specific instance
    virtual HRFBlockAdapter*             Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                uint32_t              pi_Page,
                                                unsigned short       pi_Resolution,
                                                HFCAccessMode         pi_AccessMode) const = 0;
    };
END_IMAGEPP_NAMESPACE


