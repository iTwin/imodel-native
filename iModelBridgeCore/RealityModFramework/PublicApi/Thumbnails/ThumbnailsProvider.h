/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Thumbnails/ThumbnailsProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

enum class PointCloudView
    {
    Top,
    Front,
    Right,
    Iso
    };

class IThumbnailsProvider
    {
    public:
        // IThumbnailProvider
        THUMBNAILS_EXPORT virtual int GetRasterThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height) = 0;
        THUMBNAILS_EXPORT virtual int GetPointCloudThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView) = 0;
        
        THUMBNAILS_EXPORT virtual double*   GetRasterFootprint(uint32_t& nbPts, WCharCP inputFilename) = 0;
        THUMBNAILS_EXPORT virtual double*   GetPointCloudFootprint(uint32_t& nbPts, WCharCP inputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView) = 0;
};

class ThumbnailsProvider : public IThumbnailsProvider
{
public:
    THUMBNAILS_EXPORT static IThumbnailsProvider* Get();

    // IThumbnailProvider
    THUMBNAILS_EXPORT virtual StatusInt GetRasterThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height);
    THUMBNAILS_EXPORT virtual StatusInt GetPointCloudThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView);

    THUMBNAILS_EXPORT virtual double*   GetRasterFootprint(uint32_t& nbPts, WCharCP inputFilename);
    THUMBNAILS_EXPORT virtual double*   GetPointCloudFootprint(uint32_t& nbPts, WCharCP inputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView);

private:
    ThumbnailsProvider();
    virtual ~ThumbnailsProvider();

    bool Initialize();
    void Terminate();

    StatusInt   ExtractRasterThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height);
    StatusInt   ExtractPointCloudThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView);
    
    double*     ExtractRasterFootprint(uint32_t& nbPts, WCharCP inputFilename);
    double*     ExtractPointCloudFootprint(uint32_t& nbPts, WCharCP inputFilename, uint32_t width, uint32_t height, PointCloudView pointCloudView);
};


class PointCloudThumbnailsProvider : public RefCounted<IThumbnailsProvider>
{
public:
    THUMBNAILS_EXPORT static IThumbnailsProvider* Get();

    // IThumbnailProvider
    THUMBNAILS_EXPORT virtual int GetThumbnail(HBITMAP *pThumbnailBmp, WCharCP filename, uint32_t width, uint32_t height);

private:
    PointCloudThumbnailsProvider() {};
    virtual ~PointCloudThumbnailsProvider() {};

    BentleyStatus Initialize();
    void Terminate();

    BentleyStatus           ExtractThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, uint32_t width, uint32_t height);

    static bool s_isInitialized;
};



