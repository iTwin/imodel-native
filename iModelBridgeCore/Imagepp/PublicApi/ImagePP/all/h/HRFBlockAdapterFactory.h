//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFBlockAdapterFactory.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFBlockAdapterFactory
//-----------------------------------------------------------------------------
// This class describes the stretcher implementation
//-----------------------------------------------------------------------------
#pragma once

#include "HRFRasterFileBlockAdapter.h"
#include "HRFBlockAdapter.h"
#include "HFCMacros.h"

//-----------------------------------------------------------------------------
// This is a helper class to instantiate an implementation object
// without knowing the different implementations.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFBlockAdapterFactory
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFBlockAdapterFactory)

public:
    // This methods allow to find the best adapter type for the specified raster file.
    virtual bool FindBestAdapterTypeFor(
        HFCPtr<HRFRasterFile>  pi_rpForRasterFile,
        uint32_t               pi_AtPage,
        unsigned short        pi_AtResolution,
        HRFBlockType*          po_ToBlockType,
        uint32_t*                po_ToBlockWidth,
        uint32_t*                po_ToBlockHeight) const;

    // This methods allow to if it is possible to adapt this raster file.
    virtual bool CanAdapt(
        HFCPtr<HRFRasterFile>  pi_rpFromRasterFile,
        uint32_t               pi_AtPage,
        unsigned short        pi_AtResolution,
        HRFBlockType           pi_ToBlockType,
        uint32_t               pi_ToWidth,
        uint32_t               pi_ToHeight) const;

    // This factory methods allow to instantiate the adapter for the specified resolution.
    virtual HRFBlockAdapter* New(
        HFCPtr<HRFRasterFile> pi_rpForRasterFile,
        uint32_t              pi_AtPage,
        unsigned short       pi_AtResolution,
        HFCAccessMode         pi_WithAccessMode) const;

    // Add the creators to the registry
    void Register(const HRFBlockAdapterCreator* pi_pCreator);

    // Destructor
    virtual ~HRFBlockAdapterFactory();

protected:
    // Constructor
    HRFBlockAdapterFactory();

    // Search the appropriate creator
    virtual const HRFBlockAdapterCreator* FindCreator(
        HFCPtr<HRFRasterFile>  pi_rpForRasterFile,
        uint32_t               pi_AtPage,
        unsigned short        pi_AtResolution) const;

    virtual const HRFBlockAdapterCreator* FindCreator(
        HRFBlockType           pi_FromBlockType,
        uint32_t               pi_FromWidth,
        uint32_t               pi_FromHeight,
        HRFBlockType           pi_ToBlockType,
        uint32_t               pi_ToWidth,
        uint32_t               pi_ToHeight) const;

private:
    // The registry of implementation creators
    typedef vector<HRFBlockAdapterCreator*, allocator<HRFBlockAdapterCreator* > >
    Creators;

    // Implementation Creators registry
    Creators    m_Creators;

    // Disabled methods
    HRFBlockAdapterFactory(const HRFBlockAdapterFactory&);
    HRFBlockAdapterFactory& operator=(const HRFBlockAdapterFactory&);
    };

//-----------------------------------------------------------------------------
// HRF_REGISTER_BLOCKADAPTER
// register the creator to the registry
//-----------------------------------------------------------------------------
#define HRF_REGISTER_BLOCKADAPTER(pi_ClassName) \
static struct pi_ClassName##CreatorRegister \
{ \
    pi_ClassName##CreatorRegister() \
    { \
        HRFBlockAdapterFactory::GetInstance()->Register(pi_ClassName::GetInstance()); \
    } \
} g_##pi_ClassName##CreatorRegister;
END_IMAGEPP_NAMESPACE


