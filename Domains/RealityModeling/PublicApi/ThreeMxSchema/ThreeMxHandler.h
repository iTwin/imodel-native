/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMxSchema/ThreeMxHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__


USING_NAMESPACE_BENTLEY_DGNPLATFORM
BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxModel : PhysicalModel
{
    DEFINE_T_SUPER(PhysicalModel)

private:
//  ThreeMxScenePtr    m_threeMxScenePtr;

    DRange3d                            GetSceneRange();


    ~ThreeMxModel() { }

public:
    ThreeMxModel(CreateParams const& params) : T_Super (params) { }

     virtual void _AddGraphicsToScene(ViewContextR) override;
    THREEMX_SCHEMA_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    THREEMX_SCHEMA_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    THREEMX_SCHEMA_EXPORT virtual AxisAlignedBox3d _QueryModelRange() const override;
//  THREEMX_SCHEMA_EXPORT ThreeMxScenePtr GetThreeMxScenePtr ();

    THREEMX_SCHEMA_EXPORT static DgnModelId  CreateThreeMxModel (DgnDbR dgnDb, BeFileNameCR fileName);

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


};


//=======================================================================================
// @bsiclass                                                    Ray.Bentley     09/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreeMxModelHandler :  Dgn::dgn_ModelHandler::Model
{
    MODELHANDLER_DECLARE_MEMBERS ("ThreeMxModel", ThreeMxModel, ThreeMxModelHandler, Dgn::dgn_ModelHandler::Model, THREEMX_SCHEMA_EXPORT)

};

typedef RefCountedPtr<ThreeMxModel>                          ThreeMxModelPtr;

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
