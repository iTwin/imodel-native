/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Raster/RasterApi.h>
#include <Raster/RasterHandler.h>
#include <Raster/RasterTypes.h>

BEGIN_BENTLEY_RASTER_NAMESPACE

struct RasterFileModelHandler;
struct RasterRoot;

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

    BE_JSON_NAME(srcToDb)

public:
    uint32_t _GetExcessiveRefCountThreshold() const override { return 0x7fffffff; } // tile publisher makes lots of referrents...

    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RasterModel::CreateParams);

        DMatrix4dCP m_sourceToWorldP;   //! Optional transformation from source(lower-left origin) to BIM coordinate. If not provided we use the source georeference.
        Dgn::RepositoryLinkCP m_link;   //! Link to the raster file.
        public:
            //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
            CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) { m_link = nullptr; m_sourceToWorldP = nullptr;  }

            //! Parameters to create a new instance of a RasterFileModel.
            //! @param[in] dgndb The DgnDb for the new DgnModel
            //! @param[in] link The file link.
            //! @param[in] sourceToWorld Transformation from source(lower-left origin) to BIM coordinate. This parameter can be null.
            CreateParams(Dgn::DgnDbR dgndb, Dgn::RepositoryLinkCR link, DMatrix4dCP sourceToWorld) :
                T_Super(dgndb, RasterFileModel::QueryClassId(dgndb), link.GetElementId()), m_sourceToWorldP(sourceToWorld),
                m_link(&link)
                {
                BeAssert(link.GetElementId().IsValid());
                }
        };

private:

    DMatrix4d               m_sourceToWorld;    //! Transformation from source(lower-left origin) to BIM coordinate. 
                                                //! This transform may contains a reprojection approximation(source Gcs to BIM Gcs) if one was required.
    mutable bool            m_loadFileFailed = false;       //! if we failed to open the file, we won't try again again...

protected:
    friend struct RasterFileModelHandler;
    
    //! Create a new RasterFileModel object, in preparation for loading it from the DgnDb. Called by MODELHANDLER_DECLARE_MEMBERS. 
    RasterFileModel(CreateParams const& params);

    //! Create a new RasterFileModel object to be stored in the DgnDb.
    RasterFileModel(CreateParams const& params, DMatrix4dCR sourceToWorld);

    //! Destruct a RasterFileModel object.
    ~RasterFileModel();

    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;
    Dgn::TileTree::RootPtr _CreateTileTree(Dgn::Render::SystemP renderSys) override;
    Dgn::TileTree::RootPtr _GetTileTree(Dgn::RenderContextR) override;

    bool _IsParallelToGround() const override;
    
public:

    DMatrix4dCR  GetSourceToWorld() const { return m_sourceToWorld; }
    void SetSourceToWorld(DMatrix4dCR stw) { m_sourceToWorld = stw; }

    //! Query the DgnClassId of the RasterFileModel ECClass in the specified DgnDb.
    //! @note This is a static method that always returns the DgnClassId of the RasterFileModel class - it does @em not return the class of a specific instance.
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetClassId(RASTER_SCHEMA_NAME, RASTER_CLASSNAME_RasterFileModel)); }
};

//=======================================================================================
// Model handler for RasterFile
// Instances of RasterFileModel must be able to assume that their handler is a RasterFileModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RasterFileModelHandler : RasterModelHandler
{
    RASTERMODELHANDLER_DECLARE_MEMBERS(RASTER_CLASSNAME_RasterFileModel, RasterFileModel, RasterFileModelHandler, RasterModelHandler, RASTER_EXPORT)
private:

    static StatusInt ComputeGeoLocationFromFile(DMatrix4dR sourceToWorld, RasterFileR raster, Dgn::DgnDbR dgndb);

public:
    RASTER_EXPORT static  RasterFileModelPtr CreateRasterFileModel(RasterFileModel::CreateParams const& params);
};

END_BENTLEY_RASTER_NAMESPACE


