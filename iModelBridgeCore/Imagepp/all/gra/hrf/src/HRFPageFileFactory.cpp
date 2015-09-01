//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPageFileFactory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFPageFileFactory
//-----------------------------------------------------------------------------
// This class describes the PageFile implementation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>

HFC_IMPLEMENT_SINGLETON(HRFPageFileFactory)

//-----------------------------------------------------------------------------
// This is a helper class to instantiate an implementation object
// without knowing the different implementations.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFPageFileFactory::HRFPageFileFactory()
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFPageFileFactory::~HRFPageFileFactory()
    {
    // Empty the registry
    while (m_Creators.size() > 0)
        {
        m_Creators.erase(m_Creators.begin());
        }
    }

//-----------------------------------------------------------------------------
// Search the appropriate creator
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 @return A pointer on a PageFile Creator if the file sister file associate with
         the Raster file is found.

 @param pi_rpForRasterFile Raster file source.
 @param pi_ApplyonAllFiles true(default) : Try to find an associate the sister file with
                     all types of files(ITiff, GeoTiff, etc)
                     false : Try to find an associate sister file only with
                     the files don't support Georeference.
-----------------------------------------------------------------------------*/
const HRFPageFileCreator* HRFPageFileFactory::FindCreatorFor(
    const HFCPtr<HRFRasterFile>& pi_rpForRasterFile,
    bool pi_ApplyonAllFiles) const
    {
    HRFPageFileCreator* pCreator = 0;

    // Iterator is used to loop through the vector.
    Creators::const_iterator CreatorIterator;

    // Search the best creator with the capabilities
    CreatorIterator  = m_Creators.begin();
    while (CreatorIterator != m_Creators.end())
        {
        // Test if this is the good creator
        if (((HRFPageFileCreator*)(*CreatorIterator))->HasFor(pi_rpForRasterFile, pi_ApplyonAllFiles))
            {
            // Found the creator
            pCreator = (*CreatorIterator);
            // Stop the research
            CreatorIterator = m_Creators.end();
            }
        else
            CreatorIterator++;
        }

    return pCreator;
    }

/**----------------------------------------------------------------------------
 @return true if the file sister file associate with the Raster file
         is found.

 @param pi_rpForRasterFile Raster file source.
 @param pi_ApplyonAllFiles true : Try to find an associate the sister file with
                     all types of files(ITiff, GeoTiff, etc)
                     false : Try to find an associate sister file only with
                     the files don't support Georeference.
-----------------------------------------------------------------------------*/
bool HRFPageFileFactory::HasFor(const HFCPtr<HRFRasterFile>& pi_rpForRasterFile,
                                 bool pi_ApplyonAllFiles) const
    {
    const HRFPageFileCreator* pCreator  = 0;

    // Instantiate the raster file
    pCreator = FindCreatorFor(pi_rpForRasterFile, pi_ApplyonAllFiles);

    return (pCreator != 0);
    }

/** -----------------------------------------------------------------------------
    This method return the URL for the page decorator corresponding to the
    specified raster file.

    @param pi_rpForRasterFile  Reference to a smart pointer on the raster file on witch we
                               want to get the page decorator's URL.
    @param pi_ApplyonAllFiles true : Try to find an associate the sister file with
                            all types of files(ITiff, GeoTiff, etc)
                            false : Try to find an associate sister file only with
                            the files don't support Georeference.

    @return The raster file page decorator's URL.
   ------------------------------------------------------------------------------
*/
HFCPtr<HFCURL> HRFPageFileFactory::ComposeURLFor(const HFCPtr<HRFRasterFile>& pi_rpForRasterFile,
                                                 bool pi_ApplyonAllFiles) const
    {
    HPRECONDITION(HasFor(pi_rpForRasterFile, pi_ApplyonAllFiles));

    const HRFPageFileCreator* pCreator  = 0;

    // Instantiate the raster file
    pCreator = FindCreatorFor(pi_rpForRasterFile, pi_ApplyonAllFiles);

    return (pCreator->FoundFileFor(pi_rpForRasterFile->GetURL()));
    }


//-----------------------------------------------------------------------------
// Add the creators to the registry
//-----------------------------------------------------------------------------
void HRFPageFileFactory::Register(const HRFPageFileCreator* pi_pCreator)
    {
    // Register the Raster File Creators
    m_Creators.push_back((HRFPageFileCreator*)pi_pCreator);
    }


//-----------------------------------------------------------------------------
// Get all the related page file URLs.
//-----------------------------------------------------------------------------
void HRFPageFileFactory::GetRelatedPageFileURLs(const HFCPtr<HFCURL>& pi_rpForRasterFileURL,
                                                ListOfRelatedURLs&    po_rRelatedURLs)
    {
    // Iterator is used to loop through the vector.
    Creators::const_iterator CreatorIter(m_Creators.begin());

    // Search the best creator with the capabilities
    while (CreatorIter != m_Creators.end())
        {
        // Test if this is the good creator
        HFCPtr<HFCURL> pSisterFileURL = ((HRFPageFileCreator*)(*CreatorIter))->
                                        FoundFileFor(pi_rpForRasterFileURL);

        if (pSisterFileURL != 0)
            {
            po_rRelatedURLs.push_back(pSisterFileURL);
            }

        CreatorIter++;
        }
    }
