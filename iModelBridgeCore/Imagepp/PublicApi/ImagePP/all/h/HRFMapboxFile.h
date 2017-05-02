//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMapboxFile.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCBinStream.h"
#include "HFCAccessMode.h"

#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

#include <mutex>


BEGIN_IMAGEPP_NAMESPACE


struct WorkerPool;
struct MapBoxTileQuery;
struct ThreadLocalHttp;
struct HttpSession;

class HRFMapBoxCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFMapBoxCapabilities();
    };

class HRFMapBoxFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_MapBox, HRFRasterFile)

    friend class    HRFMapBoxTileEditor;        
    friend struct   HRFMapBoxCreator;

    // allow to Open an image file
    HRFMapBoxFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                   HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                    uint64_t        pi_Offset = 0);

    virtual     ~HRFMapBoxFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage) {return false;}

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save() {}

    virtual uint64_t                        GetFileCurrentSize() const;

    //Look ahead method.
    virtual bool                         CanPerformLookAhead(uint32_t pi_Page) const;

    
protected:

    // Methods
    // Constructor use only to create a child
    //    
    virtual bool                       Open                ();
    virtual void                        CreateDescriptors   ();
    HFCBinStream*               GetFilePtr          ();


    //--------------------------------------
    // LookAhead Methods
    //--------------------------------------
    //This method is used to determine if the file format supports look ahead
    //by block.
    virtual bool HasLookAheadByBlock(uint32_t pi_Page) const;

    //This method is used in SetLookAhead to give the list of needed tiles
    //to a derived class, since it knows how to obtain the tiles.
    virtual void RequestLookAhead(uint32_t             pi_Page,
                                 const HGFTileIDList& pi_rBlocks,
                                 bool                pi_Async);

    //This method is used in SetLookAhead to indicate to a derived class that
    //the current LookAhead has been cancelled.
    virtual void CancelLookAhead(uint32_t              pi_Page);

private:
    

    WorkerPool&         GetWorkerPool();
    std::unique_ptr<WorkerPool> m_pWorkerPool;

    HttpSession&        GetThreadLocalHttpSession();
    std::unique_ptr<ThreadLocalHttp> m_threadLocalHttp;

    std::map<uint64_t, RefCountedPtr<MapBoxTileQuery>> m_tileQueryMap;
    std::mutex                                         m_tileQueryMapMutex;
                  
    // Methods Disabled
    HRFMapBoxFile(const HRFMapBoxFile& pi_rObj);
    HRFMapBoxFile&             operator= (const HRFMapBoxFile& pi_rObj);
    };


// MapBox Creator.
struct HRFMapBoxCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFMapBoxCreator)


    // Disabled methodes
    HRFMapBoxCreator();
    };

END_IMAGEPP_NAMESPACE

