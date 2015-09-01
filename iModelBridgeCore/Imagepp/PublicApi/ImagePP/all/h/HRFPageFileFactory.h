//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPageFileFactory.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFPageFileFactory
//-----------------------------------------------------------------------------
// This class describes the stretcher implementation
//-----------------------------------------------------------------------------
#pragma once

#include "HRFPageFile.h"
#include "HFCMacros.h"

//-----------------------------------------------------------------------------
// This is a helper class to instantiate a page file object
// without knowing the different format.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFPageFileFactory
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPageFileFactory)

public:
    // Search the appropriate creator
    virtual const HRFPageFileCreator* FindCreatorFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                                                     bool pi_ApplyonAllFiles=true) const;

    // This methods allow to know if we have a page file for the specified raster file.
    virtual bool HasFor(const HFCPtr<HRFRasterFile>& pi_rpForRasterFile, bool pi_ApplyonAllFiles=false) const;

    // This method return the URL for the page decorator corresponding to the specified raster file.
    IMAGEPP_EXPORT HFCPtr<HFCURL> ComposeURLFor(const HFCPtr<HRFRasterFile>& pi_rpForRasterFile, bool pi_ApplyonAllFiles=true) const;

    // This method return the URL for the page decorator corresponding to the specified raster file.
    void GetRelatedPageFileURLs(const HFCPtr<HFCURL>& pi_rpForRasterFileURL,
                                ListOfRelatedURLs&    po_rRelatedURLs);

    // Add the creators to the registry
    IMAGEPP_EXPORT void Register(const HRFPageFileCreator* pi_pCreator);

    // Destructor
    virtual ~HRFPageFileFactory();

protected:
    // Constructor
    HRFPageFileFactory();

private:
    // The registry of implementation creators
    typedef vector<HRFPageFileCreator*, allocator<HRFPageFileCreator* > >
    Creators;

    // Implementation Creators registry
    Creators    m_Creators;

    // Disabled methods
    HRFPageFileFactory(const HRFPageFileFactory&);
    HRFPageFileFactory& operator=(const HRFPageFileFactory&);
    };
END_IMAGEPP_NAMESPACE




