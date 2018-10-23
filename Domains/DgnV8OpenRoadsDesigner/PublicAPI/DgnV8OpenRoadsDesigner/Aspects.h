/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnV8OpenRoadsDesigner/Aspects.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DgnV8OpenRoadsDesigner.h"

BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

//=======================================================================================
//! UniqueAspect to be applied to elements carrying CorridorSurface data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct CorridorSurfaceAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct CorridorSurfaceAspectHandler;

private:
    bool m_isTopMesh, m_isBottomMesh;

protected:
    CorridorSurfaceAspect() {}
    CorridorSurfaceAspect(bool isTopMesh, bool isBottomMesh): m_isTopMesh(isTopMesh), m_isBottomMesh(isBottomMesh) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_CorridorSurfaceAspect; }
    //! @private
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }
    //! @private
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    //! @private
    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    //! @private
    virtual Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    //! @private
    virtual Dgn::DgnDbStatus _SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(CorridorSurfaceAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorSurfaceAspectPtr Create(bool isTopMesh, bool isBottomMesh);

    //! Retrieve the CorridorSurfaceAspect instance related to an element, if any
    //! @return An instance of CorridorSurfaceAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorSurfaceAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorSurfaceAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, CorridorSurfaceAspectR aspect);

    // Get whether the associated element's mesh is at the top
    bool GetIsTopMesh() const { return m_isTopMesh; }
    //! @private
    void SetIsTopMesh(bool val) { m_isTopMesh = val; }
    // Get whether the associated element's mesh is at the bottom
    bool GetIsBottomMesh() const { return m_isBottomMesh; }
    //! @private
    void SetIsBottomMesh(bool val) { m_isBottomMesh = val; }
}; // CorridorSurfaceAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying Featurized data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct FeatureAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct FeatureAspectHandler;

private:
    Utf8String m_name, m_definitionName;

protected:
    FeatureAspect() {}
    FeatureAspect(Utf8CP name, Utf8CP definitionName): m_name(name), m_definitionName(definitionName) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_FeatureAspect; }
    //! @private
    virtual Utf8CP _GetSuperECClassName() const override { return T_Super::_GetECClassName(); }
    //! @private
    virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR el) override;
    //! @private
    virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken) override;
    //! @private
    virtual Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, Dgn::PropertyArrayIndex const& arrayIndex) const override;
    //! @private
    virtual Dgn::DgnDbStatus _SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex) override;

public:
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(FeatureAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static FeatureAspectPtr Create(Utf8CP name, Utf8CP definitionName);

    //! Retrieve the FeatureAspect instance related to an element, if any
    //! @return An instance of FeatureAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static FeatureAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static FeatureAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, FeatureAspectR aspect);

    // Get the feature name
    Utf8CP GetName() const { return m_name.c_str(); }
    //! @private
    void SetName(Utf8CP val) { m_name = val; }
    // Get the feature definition name
    Utf8CP GetDefinitionName() const { return m_definitionName.c_str(); }
    //! @private
    void SetDefinitionName(Utf8CP val) { m_definitionName = val; }

}; // FeatureAspect

END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
