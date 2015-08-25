/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterFileHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnCore/RasterBaseModel.h>
#include <RasterSchema/RasterHandler.h>
#include <RasterSchema/RasterSchemaTypes.h>

// POINTCLOUD_WIP_GR06 - Temporary location for FileMoniker (until we decide how to handle local file names)
RASTERSCHEMA_TYPEDEFS(FileMoniker);
RASTERSCHEMA_REF_COUNTED_PTR(FileMoniker);

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterFileModelHandler;

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     06/2015
//=======================================================================================
struct RasterFileProperties
    {
    FileMonikerPtr              m_fileMonikerPtr;
    DRange2d                    m_boundingBox;   //! Bounding box corners (minx,miny,maxx,maxy) in 'CoordinateSystem' unit.

                                RasterFileProperties();
    void                        ToJson(Json::Value&) const;
    void                        FromJson(Json::Value const&);                
    };

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterFileModel : RasterModel
{
    DEFINE_T_SUPER(RasterModel)

private:
    RasterFileProperties    m_fileProperties;

protected:
    friend struct RasterFileModelHandler;
    
    //! Create a new RasterFileModel object, in preparation for loading it from the DgnDb. Called by MODELHANDLER_DECLARE_MEMBERS. 
    RasterFileModel(CreateParams const& params);

    //! Create a new RasterFileModel object to be stored in the DgnDb.
    RasterFileModel(CreateParams const& params, RasterFileProperties const& properties);

    //! Destruct a RasterFileModel object.
    ~RasterFileModel();

    virtual void            _ToPropertiesJson(Json::Value&) const override;
    virtual void            _FromPropertiesJson(Json::Value const&) override;
    virtual BentleyStatus   _LoadQuadTree() override;
    virtual Dgn::AxisAlignedBox3d _QueryModelRange() const override;

public:

};

//=======================================================================================
// Model handler for RasterFile
// Instances of RasterFileModel must be able to assume that their handler is a RasterFileModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterFileModelHandler : RasterModelHandler
{
    RASTERMODELHANDLER_DECLARE_MEMBERS (RASTER_CLASSNAME_RasterFileModel, RasterFileModel, RasterFileModelHandler, RasterModelHandler, RASTERSCHEMA_EXPORT)

private:
                        static  ReprojectStatus GetRasterExtentInUors(DRange2d &range, RasterFileCR rasterFile, Dgn::DgnDbCR db);

public:
    RASTERSCHEMA_EXPORT static  Dgn::DgnModelId CreateRasterFileModel(Dgn::DgnDbR db, FileMonikerPtr fileMoniker);
};

//=======================================================================================
// POINTCLOUD_WIP_GR06 - Temporary location for FileMoniker (until we decide how to handle local file names)
// @bsiclass                                                    Eric.Paquet     08/2015
//=======================================================================================
struct FileMoniker : RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

protected:
    Utf8String      m_fullPath;
    Utf8String      m_basePath;

    FileMoniker (Utf8StringCR fullPath, Utf8StringCR basePath);
    ~FileMoniker () {};

public:
    RASTERSCHEMA_EXPORT static  FileMonikerPtr      Create (Utf8StringCR fullPath, Utf8StringCR basePath);
    RASTERSCHEMA_EXPORT         BentleyStatus       ResolveFileName (Utf8StringR resolvedName, Utf8StringCR basePath) const;
    RASTERSCHEMA_EXPORT         void                ToJson(JsonValueR outValue) const;
    RASTERSCHEMA_EXPORT         void                FromJson(JsonValueCR inValue);
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE


