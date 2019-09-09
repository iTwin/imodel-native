/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnV8OpenRoadsDesigner/Aspects.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    Utf8String m_description;
    Utf8String m_corridorName, m_horizontalName, m_profileName;

protected:
    CorridorSurfaceAspect() {}
    CorridorSurfaceAspect(bool isTopMesh, bool isBottomMesh, Utf8CP description, Utf8CP corridorName, Utf8CP horizontalName, Utf8CP profileName): 
        m_isTopMesh(isTopMesh), m_isBottomMesh(isBottomMesh), m_description(description), 
        m_corridorName(corridorName), m_horizontalName(horizontalName), m_profileName(profileName) {}

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
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorSurfaceAspectPtr Create(bool isTopMesh, bool isBottomMesh, Utf8CP description,
                                                                         Utf8CP corridorName, Utf8CP horizontalName, Utf8CP profileName);

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
    // Get the description
    Utf8CP GetDescription() const { return m_description.c_str(); }
    //! @private
    void SetDescription(Utf8CP val) { m_description = val; }
    // Get the Corridor's name
    Utf8CP GetCorridorName() const { return m_corridorName.c_str(); }
    //! @private
    void SetCorridorName(Utf8CP val) { m_corridorName = val; }
    // Get the Horizontal's name
    Utf8CP GetHorizontalName() const { return m_horizontalName.c_str(); }
    //! @private
    void SetHorizontalName(Utf8CP val) { m_horizontalName = val; }
    // Get the Horizontal's name
    Utf8CP GetProfileName() const { return m_profileName.c_str(); }
    //! @private
    void SetProfileName(Utf8CP val) { m_profileName = val; }
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
    Utf8String m_name, m_definitionName, m_description;

protected:
    FeatureAspect() {}
    FeatureAspect(Utf8CP name, Utf8CP definitionName, Utf8CP description): m_name(name), m_definitionName(definitionName), m_description(description) {}

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
    DGNV8OPENROADSDESIGNER_EXPORT static FeatureAspectPtr Create(Utf8CP name, Utf8CP definitionName, Utf8CP description);

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
    // Get the feature description
    Utf8CP GetDescription() const { return m_description.c_str(); }
    //! @private
    void SetDescription(Utf8CP val) { m_description = val; }

}; // FeatureAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying Template Drop data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct TemplateDropAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct TemplateDropAspectHandler;

private:
    double m_interval;
    Utf8String m_templateName, m_description;

protected:
    TemplateDropAspect() {}
    TemplateDropAspect(double interval, Utf8CP templateName, Utf8CP description): m_interval(interval), m_templateName(templateName), m_description(description) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_TemplateDropAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(TemplateDropAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static TemplateDropAspectPtr Create(double interval, Utf8CP templateName, Utf8CP description);

    //! Retrieve the TemplateDropAspect instance related to an element, if any
    //! @return An instance of TemplateDropAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static TemplateDropAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static TemplateDropAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, TemplateDropAspectR aspect);

    // Get the Interval
    double GetInterval() const { return m_interval; }
    //! @private
    void SetInterval(double val) { m_interval = val; }
    // Get the Template name
    Utf8CP GetTemplateName() const { return m_templateName.c_str(); }
    //! @private
    void SetTemplateName(Utf8CP val) { m_templateName = val; }
    // Get the TemplateDrop definition name
    Utf8CP GetDescription() const { return m_description.c_str(); }
    //! @private
    void SetDescription(Utf8CP val) { m_description = val; }

}; // TemplateDropAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying StationRange data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct StationRangeAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct StationRangeAspectHandler;

private:
    double m_startStation, m_endStation;

protected:
    StationRangeAspect() {}
    StationRangeAspect(double startStation, double endStation): m_startStation(startStation), m_endStation(endStation) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_StationRangeAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(StationRangeAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static StationRangeAspectPtr Create(double startStation, double endStation);

    //! Retrieve the StationRangeAspect instance related to an element, if any
    //! @return An instance of StationRangeAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static StationRangeAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static StationRangeAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, StationRangeAspectR aspect);

    // Get Start Station
    double GetStartStation() const { return m_startStation; }
    //! @private
    void SetStartStation(double val) { m_startStation = val; }
    // Get End Station
    double GetEndStation() const { return m_endStation; }
    //! @private
    void SetEndStation(double val) { m_endStation = val; }

}; // StationRangeAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying Superelevation data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct SuperelevationAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct SuperelevationAspectHandler;

private:
    Utf8String m_name;
    double m_normalCrossSlope;

protected:
    SuperelevationAspect() {}
    SuperelevationAspect(Utf8CP name, double normalCrossSlope): m_name(name), m_normalCrossSlope(normalCrossSlope) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_SuperelevationAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(SuperelevationAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static SuperelevationAspectPtr Create(Utf8CP name, double normalCrossSlope);

    //! Retrieve the SuperelevationAspect instance related to an element, if any
    //! @return An instance of SuperelevationAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static SuperelevationAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static SuperelevationAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, SuperelevationAspectR aspect);

    // Get Name
    Utf8CP GetName() const { return m_name.c_str(); }
    //! @private
    void SetName(Utf8CP val) { m_name = val; }
    // Get Normal Cross Section
    double GetNormalCrossSection() const { return m_normalCrossSlope; }
    //! @private
    void SetNormalCrossSlope(double val) { m_normalCrossSlope = val; }

}; // SuperelevationAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying Corridor data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct CorridorAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct CorridorAspectHandler;

private:
    Utf8String m_name, m_activeProfileName, m_horizontalName;

protected:
    CorridorAspect() {}
    CorridorAspect(Utf8CP name, Utf8CP horizontalName, Utf8CP activeProfileName): 
        m_name(name), m_horizontalName(horizontalName), m_activeProfileName(activeProfileName) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_CorridorAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(CorridorAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorAspectPtr Create(Utf8CP name, Utf8CP horizontalName, Utf8CP activeProfileName);

    //! Retrieve the CorridorAspect instance related to an element, if any
    //! @return An instance of CorridorAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static CorridorAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, CorridorAspectR aspect);

    // Get Name
    Utf8CP GetName() const { return m_name.c_str(); }
    //! @private
    void SetName(Utf8CP val) { m_name = val; }
    // Get Active Profile Name
    Utf8CP GetActiveProfileName() const { return m_activeProfileName.c_str(); }
    //! @private
    void SetActiveProfileName(Utf8CP val) { m_activeProfileName = val; }
    // Get Horizontal Name
    Utf8CP GetHorizontalName() const { return m_horizontalName.c_str(); }
    //! @private
    void SetHorizontalName(Utf8CP val) { m_horizontalName = val; }

}; // CorridorAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying VolumetricQuantity data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct VolumetricQuantityAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct VolumetricQuantityAspectHandler;

private:
    double m_volume, m_slopedArea;

protected:
    VolumetricQuantityAspect() {}
    VolumetricQuantityAspect(double volume, double slopedArea): m_volume(volume), m_slopedArea(slopedArea) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_VolumetricQuantityAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(VolumetricQuantityAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static VolumetricQuantityAspectPtr Create(double volume, double surfaceArea);

    //! Retrieve the VolumetricQuantityAspect instance related to an element, if any
    //! @return An instance of VolumetricQuantityAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static VolumetricQuantityAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static VolumetricQuantityAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, VolumetricQuantityAspectR aspect);

    // Get Volume
    double GetVolume() const { return m_volume; }
    //! @private
    void SetVolume(double val) { m_volume = val; }
    // Get SurfaceArea
    double GetSlopedArea() const { return m_slopedArea; }
    //! @private
    void SetSlopedArea(double val) { m_slopedArea = val; }

}; // VolumetricQuantityAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying LinearQuantity data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct LinearQuantityAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct LinearQuantityAspectHandler;

private:
    double m_length;

protected:
    LinearQuantityAspect() {}
    LinearQuantityAspect(double length): m_length(length) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_LinearQuantityAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(LinearQuantityAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static LinearQuantityAspectPtr Create(double length);

    //! Retrieve the LinearQuantityAspect instance related to an element, if any
    //! @return An instance of LinearQuantityAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static LinearQuantityAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static LinearQuantityAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, LinearQuantityAspectR aspect);

    // Get Length
    double GetLength() const { return m_length; }
    //! @private
    void SetLength(double val) { m_length = val; }

}; // LinearQuantityAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying DiscreteQuantity data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct DiscreteQuantityAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct DiscreteQuantityAspectHandler;

private:
    int32_t m_count;

protected:
    DiscreteQuantityAspect() {}
    DiscreteQuantityAspect(int32_t count): m_count(count) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_DiscreteQuantityAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(DiscreteQuantityAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static DiscreteQuantityAspectPtr Create(int32_t count);

    //! Retrieve the DiscreteQuantityAspect instance related to an element, if any
    //! @return An instance of DiscreteQuantityAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static DiscreteQuantityAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static DiscreteQuantityAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, DiscreteQuantityAspectR aspect);

    // Get Count
    int32_t GetCount() const { return m_count; }
    //! @private
    void SetCount(int32_t val) { m_count = val; }

}; // DiscreteQuantityAspect

//=======================================================================================
//! UniqueAspect to be applied to elements carrying Alignment data from ORD.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct AlignmentAspect : Dgn::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(Dgn::DgnElement::UniqueAspect);
    friend struct AlignmentAspectHandler;

private:
    DPoint2d m_startPoint, m_endPoint;
    Utf8String m_activeProfileName;

protected:
    AlignmentAspect() {}
    AlignmentAspect(DPoint2d const& startPoint, DPoint2d const& endPoint, Utf8CP activeProfileName): 
        m_startPoint(startPoint), m_endPoint(endPoint), m_activeProfileName(activeProfileName) {}

    //! @private
    virtual Utf8CP _GetECSchemaName() const { return V8ORD_SCHEMA_NAME; }
    //! @private
    virtual Utf8CP _GetECClassName() const { return V8ORD_CLASS_AlignmentAspect; }
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
    DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(AlignmentAspect)
    DGNV8OPENROADSDESIGNER_EXPORT static AlignmentAspectPtr Create(DPoint2d const& startPoint, DPoint2d const& endPoint, Utf8CP activeProfileName);

    //! Retrieve the AlignmentAspect instance related to an element, if any
    //! @return An instance of AlignmentAspect, or nullptr
    DGNV8OPENROADSDESIGNER_EXPORT static AlignmentAspectCP Get(Dgn::DgnElementCR el);

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static AlignmentAspectP GetP(Dgn::DgnElementR el);
    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static void Set(Dgn::DgnElementR el, AlignmentAspectR aspect);

    // Get ActiveProfileName
    Utf8CP GetActiveProfileName() const { return m_activeProfileName.c_str(); }
    //! @private
    void SetActiveProfileName(Utf8CP activeProfileName) { m_activeProfileName = activeProfileName; }
    // Get StartPoint
    DPoint2d GetStartPoint() const { return m_startPoint; }
    //! @private
    void SetStartPoint(DPoint2d startPoint) { m_startPoint = startPoint; }
    // Get EndPoint
    DPoint2d GetEndPoint() const { return m_endPoint; }
    //! @private
    void SetEndPoint(DPoint2d endPoint) { m_endPoint = endPoint; }

}; // AlignmentAspect

END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
