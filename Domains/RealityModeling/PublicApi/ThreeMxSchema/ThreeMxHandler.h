/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMxSchema/ThreeMxHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

THREEMXSCHEMA_TYPEDEFS (ThreeMxModel)
THREEMXSCHEMA_TYPEDEFS (S3SceneInfo)

USING_NAMESPACE_BENTLEY_DGNPLATFORM
BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ThreeMxScene : RefCountedBase
{

    Transform       m_transform;

    ThreeMxScene () : m_transform (Transform::FromIdentity()) { }
    void    SetTransform (TransformCR transform) { m_transform = transform; }

    virtual void            _Draw (bool& childrenScheduled, ViewContextR viewContext, struct MRMeshContext const& meshContext) = 0;
    virtual BentleyStatus   _GetRange (DRange3dR range, TransformCR transform)  const = 0;;

};  // ThreeMxScene

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct ThreeMxGCS
{
    static BentleyStatus GetProjectionTransform (TransformR transform, S3SceneInfoCR sceneInfo, DgnDbR db, DRange3dCR range);
    
};


typedef RefCountedPtr <struct ThreeMxScene>      ThreeMxScenePtr;

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxModel : PhysicalModel
{
    DGNMODEL_DECLARE_MEMBERS("ThreeMxModel", PhysicalModel);

private:
    ThreeMxScenePtr         m_scene;
    DRange3d                GetSceneRange();
    static ThreeMxScenePtr  ReadScene (BeFileNameR fileName, DgnDbR db, Utf8StringCR fileId);

    ~ThreeMxModel() { }

public:
    ThreeMxModel(CreateParams const& params) : T_Super (params), m_scene (NULL) { }

    THREEMX_SCHEMA_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    THREEMX_SCHEMA_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    THREEMX_SCHEMA_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    THREEMX_SCHEMA_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
    THREEMX_SCHEMA_EXPORT static DgnModelId  CreateThreeMxModel (DgnDbR dgnDb, Utf8StringCR fileId);

    ThreeMxScenePtr  GetScene ();
    void             SetScene (ThreeMxScenePtr& scene)    { m_scene = scene; }
    void             SetFileId (Utf8StringCR fileId)      { m_properties.m_fileId = fileId; }

    struct Properties
        {
        Utf8String          m_fileId;    

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

protected:
    Properties      m_properties;

    friend struct ThreeMxModelHandler;
    friend struct ThreeMxProgressiveDisplay;


};  // ThreeMxScene;


//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxModelHandler :  Dgn::dgn_ModelHandler::Physical
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ThreeMxModelHandler, Dgn::dgn_ModelHandler::Physical, THREEMX_SCHEMA_EXPORT)

};

typedef RefCountedPtr<ThreeMxModel>                          ThreeMxModelPtr;

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
