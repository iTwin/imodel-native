/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Thumbnails/ThumbnailsProvider.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
        THUMBNAILS_EXPORT virtual int GetRasterThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height) = 0;
        THUMBNAILS_EXPORT virtual int GetPointCloudThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height, PointCloudView pointCloudView) = 0;
};

class ThumbnailsProvider : public IThumbnailsProvider
{
public:
    THUMBNAILS_EXPORT static IThumbnailsProvider* Get();

    // IThumbnailProvider
    THUMBNAILS_EXPORT virtual StatusInt GetRasterThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height);
    THUMBNAILS_EXPORT virtual StatusInt GetPointCloudThumbnail(HBITMAP *pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height, PointCloudView pointCloudView);

private:
    ThumbnailsProvider();
    virtual ~ThumbnailsProvider();

    bool Initialize();
    void Terminate();

    StatusInt             ExtractRasterThumbnail    (HBITMAP* pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height);
    StatusInt             ExtractPointCloudThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height, PointCloudView pointCloudView);
};


class PointCloudThumbnailsProvider : public RefCounted<IThumbnailsProvider>
{
public:
    THUMBNAILS_EXPORT static IThumbnailsProvider* Get();

    // IThumbnailProvider
    THUMBNAILS_EXPORT virtual int GetThumbnail(HBITMAP *pThumbnailBmp, WCharCP filename, UInt32 width, UInt32 height);

private:
    PointCloudThumbnailsProvider() {};
    virtual ~PointCloudThumbnailsProvider() {};

    BentleyStatus Initialize();
    void Terminate();

    BentleyStatus           ExtractThumbnail(HBITMAP* pThumbnailBmp, WCharCP InputFilename, UInt32 width, UInt32 height);

    static bool s_isInitialized;
};



