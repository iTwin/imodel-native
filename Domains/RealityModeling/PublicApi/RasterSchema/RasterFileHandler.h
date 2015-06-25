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

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterFileModelHandler;

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     06/2015
//=======================================================================================
struct RasterFileProperties
    {
    Utf8String m_url;               //! Raster url. 

    DRange2d m_boundingBox;         //! Bounding box corners (minx,miny,maxx,maxy) in 'CoordinateSystem' unit.

    void ToJson(Json::Value&) const;
    void FromJson(Json::Value const&);                
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
    virtual DgnPlatform::AxisAlignedBox3d _QueryModelRange() const override;

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

public:
    RASTERSCHEMA_EXPORT static DgnPlatform::DgnModelId CreateRasterFileModel(DgnDbR db, BeFileName fileName);
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE


