/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RasterSchema/RasterFileHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <RasterSchema/RasterSchemaApi.h>
#include <RasterSchema/RasterHandler.h>
#include <RasterSchema/RasterSchemaTypes.h>

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct RasterFileModelHandler;
struct RasterRoot;

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     06/2015
//=======================================================================================
struct RasterFileProperties
    {
    Utf8String  m_fileId;           //! File id provided by the application. Used to resolve the local file name.  
    DMatrix4d   m_sourceToWorld;    //! Transformation from source(lower-left origin) to BIM coordinate. 
                                    //! This transform may contains a reprojection approximation(source Gcs to BIM Gcs) if one was required.

                                RasterFileProperties();
    void                        ToJson(Json::Value&) const;
    void                        FromJson(Json::Value const&);                
    };

//=======================================================================================
// A DgnModel to reference a raster file. This holds the name of the file and a transform 
// to position the raster in the BIM.
// Note that the a rasterfile may also have a its own geo-reference(matrix, GCS or both) stored in it,
// so the location can be calculated by geo-referncing it to the one in the BIM. But, since not all raster files are geo-referenced,
// and sometimes users may want to "tweak" the location relative to their BIM, we store it in the model and use that. If reprojection
// is required it is approximated and include in the source to world transformation.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterFileModel : RasterModel
{
    DGNMODEL_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterFileModel, RasterModel)

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RasterModel::CreateParams);

        Utf8String m_fileId;
        DMatrix4dCP m_sourceToWorldP;   //! Optional transformation from source(lower-left origin) to BIM coordinate. If not provide we use source georeference.

        public:
            //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
            CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

            //! Parameters to create a new instance of a RasterFileModel.
            //! @param[in] dgndb The DgnDb for the new DgnModel
            //! @param[in] code The Code for the DgnModel
            //! @param[in] fileId File Id of the raster file.
            //! @param[in] transformP Transform of the raster file. This parameter can be null.
            CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnCode code, Utf8StringCR fileId, DMatrix4dCP sourceToWorld) :
                T_Super(dgndb, RasterFileModel::QueryClassId(dgndb), Dgn::DgnElementId() /* WIP: Which element? */, code), m_fileId(fileId), m_sourceToWorldP(sourceToWorld)
                {}
        };


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

    virtual void            _WriteJsonProperties(Json::Value&) const override;
    virtual void            _ReadJsonProperties(Json::Value const&) override;
    virtual BentleyStatus   _Load(Dgn::Render::SystemP renderSys) const override;
    
    virtual DMatrix4dCR  _GetSourceToWorld() const override { return m_fileProperties.m_sourceToWorld;}

public:

    //! Query the DgnClassId of the RasterFileModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the RasterFileModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(RASTER_SCHEMA_NAME, RASTER_CLASSNAME_RasterFileModel)); }
};

//=======================================================================================
// Model handler for RasterFile
// Instances of RasterFileModel must be able to assume that their handler is a RasterFileModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterFileModelHandler : RasterModelHandler
{
    RASTERMODELHANDLER_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterFileModel, RasterFileModel, RasterFileModelHandler, RasterModelHandler, RASTERSCHEMA_EXPORT)
private:

    static StatusInt ComputeGeoLocationFromFile(DMatrix4dR sourceToWorld, RasterFileR raster, Dgn::DgnDbR dgndb);

public:
    RASTERSCHEMA_EXPORT static  RasterFileModelPtr CreateRasterFileModel(RasterFileModel::CreateParams const& params);
};

END_BENTLEY_RASTERSCHEMA_NAMESPACE


