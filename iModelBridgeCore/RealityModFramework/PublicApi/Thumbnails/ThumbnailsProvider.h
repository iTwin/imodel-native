/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Thumbnails/ThumbnailsProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*  WIP
    * CreateFootprint method should return a status and the double pointer should be pass in parameters.
    * Saving footprint and thumbnails to file should be done here instead of the caller (?).
    * Add possibility to run on more than one file at a time.
*/

typedef uint32_t PtHandle;

enum class PointCloudView
{
    Top,
    Front,
    Right,
    Iso
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
class IPropertiesProvider
{
public:
    THUMBNAILS_EXPORT virtual double*    GetFootprint() = 0;
    THUMBNAILS_EXPORT virtual StatusInt  GetThumbnail(HBITMAP *pThumbnailBmp) = 0;

    THUMBNAILS_EXPORT static IPropertiesProvider* Create(WCharCP inFilename, WCharCP view);
};


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
class RasterProperties : public RefCounted<IPropertiesProvider>
{
public:
    THUMBNAILS_EXPORT virtual double*     GetFootprint();
    THUMBNAILS_EXPORT virtual StatusInt   GetThumbnail(HBITMAP *pThumbnailBmp);

    RasterProperties(WCharCP filename);
    virtual ~RasterProperties();

private:
    double*                 ExtractFootprint();
    StatusInt               ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height);

    HFCPtr<HRFRasterFile>   GetFile(WCharCP inFilename);
    bool                    Initialize();
    void                    Terminate();

    HFCPtr<HRFRasterFile> mRasterFile;
};


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
class PointCloudProperties : public RefCounted<IPropertiesProvider>
{
public:
    THUMBNAILS_EXPORT virtual double*     GetFootprint();
    THUMBNAILS_EXPORT virtual StatusInt   GetThumbnail(HBITMAP *pThumbnailBmp);

    PointCloudProperties(WCharCP filename, WCharCP view);
    virtual ~PointCloudProperties();

private:
    double*     ExtractFootprint();
    StatusInt   ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height);

    void        GetFile(WCharCP inFilename);
    void        CloseFile();
    bool        Initialize();
    void        Terminate();

    PointCloudView  mView;
    PtHandle        mCloudFileHandle;
    PtHandle        mCloudHandle;
};







