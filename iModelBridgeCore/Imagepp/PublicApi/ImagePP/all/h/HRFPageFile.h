//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPageFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class : HRFPageFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HRFPageDescriptor.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"
#include "HGF2DWorld.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFPageFile : public HFCShareableObject<HRFPageFile>
    {
public:
    // Class ID for this class.
    HDECLARE_BASECLASS_ID(HRFPageFileId_Base)

    virtual                               ~HRFPageFile ();

    // File name
    const HFCPtr<HFCURL>&         GetURL () const;

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const = 0;

    uint32_t                      CountPages () const;
    HFCPtr<HRFPageDescriptor>     GetPageDescriptor (uint32_t pi_Page) const;

    // Access mode management
    HFCAccessMode                 GetAccessMode () const;

    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const = 0;

    virtual void                          WriteToDisk() = 0;

    IMAGEPP_EXPORT virtual void                   SetDefaultRatioToMeter(double pi_RatioToMeter);
    virtual double                       GetDefaultRatioToMeter() const;

protected:
    // List of pages descriptor
    typedef vector<HFCPtr<HRFPageDescriptor>, allocator<HFCPtr<HRFPageDescriptor> > >
    ListOfPageDescriptor;

    // File information
    HFCPtr<HFCURL>                      m_pURL;
    ListOfPageDescriptor                m_ListOfPageDescriptor;

    // Access mode management
    HFCAccessMode                       m_FileAccessMode;

    double                             m_DefaultRatioToMeter;

    // Methods
    // allow to Open an image file
    HRFPageFile         (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode = HFC_READ_ONLY);
private:
    // Methods Disabled
    HRFPageFile(const HRFPageFile& pi_rObj);
    HRFPageFile& operator=(const HRFPageFile& pi_rObj);
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific object.
// There will be an object that derives from this one for each specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------
class HRFPageFileCreator
    {
public:

    // Creation of this specific instance
    virtual bool                HasFor          (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile, bool pi_ApplyonAllFiles=false) const = 0;
    virtual HFCPtr<HRFPageFile>  CreateFor       (const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const = 0;
    virtual HFCPtr<HFCURL>       ComposeURLFor   (const HFCPtr<HFCURL>&   pi_rpURLFileName) const = 0;
    virtual HFCPtr<HFCURL>       FoundFileFor    (const HFCPtr<HFCURL>& pi_rpURL) const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities() = 0;

    virtual ~HRFPageFileCreator()    {}

protected:

    // capabilities instance member definition
    HFCPtr<HRFRasterFileCapabilities> m_pCapabilities;
    };
END_IMAGEPP_NAMESPACE

#include "HRFPageFile.hpp"

