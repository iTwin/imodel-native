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

/*&&ep d
class HRFRasterFile;
//template<class T> class HNOVTABLEINIT HFCPtr
//template<class T> class HFCPtr;
class HFCPtr<HRFRasterFile>;
*/

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterFileModelHandler;

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     06/2015
//=======================================================================================
struct RasterFileProperties
    {
    Utf8String m_url;               //! Raster url. 

    // Map window
    DRange2d m_boundingBox;         //! Bounding box corners (minx,miny,maxx,maxy) in 'CoordinateSystem' unit.
//&&ep need ?
    uint32_t m_metaWidth;           //! Width of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this width.
    uint32_t m_metaHeight;          //! Height of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this Height.

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
    RasterFilePtr           m_rasterFilePtr;    //&&ep - maybe not needed
 
    RasterFileProperties    m_properties;     //&&ep need a way to avoid duplication of information between RasterModel/RasterFileModel/RasterFileSource.

protected:
    friend struct RasterFileModelHandler;
    
    //! Create a new RasterFileModel object, in preparation for loading it from the DgnDb. Called by MODELHANDLER_DECLARE_MEMBERS. 
    RasterFileModel(CreateParams const& params);

    //! Create a new RasterFileModel object to be stored in the DgnDb.
    RasterFileModel(CreateParams const& params, RasterFileProperties const& properties);

    //! Destruct a RasterFileModel object.
    ~RasterFileModel();

/* &&ep o
    RASTERSCHEMA_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    RASTERSCHEMA_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    RASTERSCHEMA_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    RASTERSCHEMA_EXPORT virtual DgnPlatform::AxisAlignedBox3d _QueryModelRange() const override;
*/
    virtual void            _ToPropertiesJson(Json::Value&) const override;
    virtual void            _FromPropertiesJson(Json::Value const&) override;
    virtual BentleyStatus   _LoadQuadTree() override;

    //! Call this after creating a new model, in order to set up subclass-specific properties.
//&&ep needed ?
    BentleyStatus           SetProperties (BeFileNameCR fileName);

public:
    //! Provide a pointer to a HFCPtr<HRFRasterFile>. 
//&&ep needed ?
    RasterFilePtr           GetRasterFilePtr();

};

//=======================================================================================
// Model handler for RasterFile
// Instances of RasterFileModel must be able to assume that their handler is a RasterFileModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterFileModelHandler : RasterModelHandler
{
    RASTERMODELHANDLER_DECLARE_MEMBERS ("RasterFileModel", RasterFileModel, RasterFileModelHandler, RasterModelHandler, RASTERSCHEMA_EXPORT)

public:
    RASTERSCHEMA_EXPORT static DgnPlatform::DgnModelId CreateRasterFileModel(DgnDbR db, BeFileName fileName);
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE


