/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/PointCloudHandle.h>
#include <BePointCloud/PointCloudScene.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct PointCloudModelHandler;

//=======================================================================================
// Obtain and display point cloud data from POD files. 
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModel : Dgn::SpatialModel
    {
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(Dgn::SpatialModel::CreateParams);

        Utf8String m_fileId;

        public:
            //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
            CreateParams(Dgn::DgnModel::CreateParams const& params) : T_Super(params) {}

            //! Parameters to create a new instance of a PointCloudModel.
            //! @param[in] dgndb The DgnDb for the new DgnModel
            //! @param[in] code The Code for the DgnModel
            //! @param[in] fileId File Id of the PointCloud file.
            CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnCode code, Utf8StringCR fileId) :
                T_Super(dgndb, PointCloudModel::QueryClassId(dgndb), code), m_fileId(fileId)
                {}
        };

    DGNMODEL_DECLARE_MEMBERS(POINTCLOUD_CLASSNAME_PointCloudModel, Dgn::SpatialModel)

    private:
        mutable BePointCloud::PointCloudScenePtr    m_pointCloudScenePtr;

        DRange3d                            GetSceneRange();

    public:
        // POINTCLOUD_WIP_GR06_Json - To remove this, we could move JsonUtils.h to PublicApi and then delete this struct and associated methods.
        struct JsonUtils
            {
            static void DPoint3dToJson (JsonValueR outValue, DPoint3dCR point);
            static void DPoint3dFromJson (DPoint3dR point, Json::Value const& inValue);
            };

        struct Properties
            {
            DRange3d            m_range;        //! Point Cloud range
            Utf8String          m_fileId;       //! File id provided by the application. Used to resolve the local file name.

            void ToJson(Json::Value&) const;
            void FromJson(Json::Value const&);
            };

    protected:
        friend struct PointCloudModelHandler;
        friend struct PointCloudProgressiveDisplay;

        Properties m_properties;

        //! Destruct a PointCloudModel object.
        ~PointCloudModel();

    public:
        //! Create a new PointCloudModel object, in preparation for loading it from the DgnDb.
        PointCloudModel(CreateParams const& params);
        PointCloudModel(CreateParams const& params, PointCloudModel::Properties const& properties) ;

        virtual void _AddSceneGraphics(Dgn::SceneContextR) const override;
        virtual void _WriteJsonProperties(Json::Value&) const override;
        virtual void _ReadJsonProperties(Json::Value const&) override;
        virtual Dgn::AxisAlignedBox3d _QueryModelRange() const override;
        BePointCloud::PointCloudSceneP GetPointCloudSceneP () const;
        DRange3dCR GetRange() const {return m_properties.m_range;}
        DRange3dR GetRangeR() {return m_properties.m_range;}

        //! Query the DgnClassId of the PointCloudModel ECClass in the specified DgnDb.
        //! @note This is a static method that always returns the DgnClassId of the PointCloudModel class - it does @em not return the class of a specific instance.
        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(POINTCLOUD_SCHEMA_NAME, POINTCLOUD_CLASSNAME_PointCloudModel)); }
    };

//=======================================================================================
// Model handler for point clouds.
// Instances of PointCloudModel must be able to assume that their handler is a PointCloudModelHandler.
// @bsiclass                                                    Eric.Paquet     04/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PointCloudModelHandler : Dgn::dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS(POINTCLOUD_CLASSNAME_PointCloudModel, PointCloudModel, PointCloudModelHandler, Dgn::dgn_ModelHandler::Spatial, POINTCLOUDSCHEMA_EXPORT)

public:
    POINTCLOUDSCHEMA_EXPORT static PointCloudModelPtr CreatePointCloudModel(PointCloudModel::CreateParams const& params);
};

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
