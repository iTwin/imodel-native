/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


//&&JFC - Add a begin/end namespace. Everything struct/class must in our own namespace. 
//      - provide XXXX_TYPEDEFS and XXXX_REF_COUNTED_PTR
//      - Do use add USING_ to one of the header file. Use the type defined by XXX_TYPEDEFS

/*  WIP
    * CreateFootprint method should return a status and the double pointer should be pass in parameters.
    * Saving footprint and thumbnails to file should be done here instead of the caller (?).
    * Add possibility to run on more than one file at a time.
    * Initialize/GetFile/etc only one time.
*/

typedef uint32_t                PtHandle;

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
struct PropertiesProvider : public RefCountedBase
{
public:
    //&&JFC we have a pointcloud specific param here maybe this create doesnot belong here.
    static RefCountedPtr<PropertiesProvider> Create(WCharCP inFilename, PointCloudView view);

    //! &&JFC DOC Footprint format? Can we reuse one of the Bentley Geom type?
    REALITYDATAPLATFORM_EXPORT double* GetFootprint() const;

    //&&JFC remove HBITMAP and windows header dependency
    REALITYDATAPLATFORM_EXPORT virtual StatusInt GetThumbnail(HBITMAP *pThumbnailBmp) const;

protected:
   
    virtual ~PropertiesProvider(){};
    virtual double*    _GetFootprint() const = 0;
    virtual StatusInt  _GetThumbnail(HBITMAP *pThumbnailBmp) const = 0;
};


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterProperties : public PropertiesProvider
{
protected:

    virtual double*  _GetFootprint() const override;
    virtual StatusInt _GetThumbnail(HBITMAP *pThumbnailBmp) const override;

    RasterProperties(WCharCP filename);
    virtual ~RasterProperties();

private:
    double* ExtractFootprint() const;

    StatusInt ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height);

    bool Initialize();
    void Terminate();

    WCharCP m_filename;
};


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudProperties : public PropertiesProvider
{
protected:
    virtual double*  _GetFootprint() const override;
    virtual StatusInt _GetThumbnail(HBITMAP *pThumbnailBmp) const override;

    PointCloudProperties(WCharCP filename, PointCloudView view);
    virtual ~PointCloudProperties();

private:
    double*     ExtractFootprint();
    StatusInt   ExtractThumbnail(HBITMAP *pThumbnailBmp, uint32_t width, uint32_t height);

    void        GetFile(WCharCP inFilename);
    void        CloseFile();
    bool        Initialize();
    void        Terminate();

    PointCloudView  m_view;
    PtHandle        m_cloudFileHandle;
    PtHandle        m_cloudHandle;
};







