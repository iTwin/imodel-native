/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
// Defines the traversal observer interface as an abstract class.
// The traversal observer that must inherit from the present one must implement
// a behavior for the OnFileListed, OnFileDownloaded and OnDataExtracted method.
//=====================================================================================
struct ISpatialEntityTraversalObserver
{
public:
    //! OnFileListed is called whenever a file was listed for download. It is up to
    //! the effective traversal observer to do whatever process is required on the data.
    virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file) = 0;

    //! OnFileDownloaded is called whenever a download is completed. It is up to
    //! the effective traversal observer to do whatever process is required on the data.
    virtual void OnFileDownloaded(Utf8CP file) = 0;

    //! OnDataExtracted is called whenever an extraction is completed by the 
    //! data handler. The data object given defines the data discovered. It is up to
    //! the effective traversal observer to do whatever process is required on the data.
    virtual void OnDataExtracted(SpatialEntityCR data) = 0;
};



END_BENTLEY_REALITYPLATFORM_NAMESPACE