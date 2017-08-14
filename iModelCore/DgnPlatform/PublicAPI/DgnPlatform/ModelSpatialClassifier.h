/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ModelSpatialClassifier.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "ClipPrimitive.h"

BEGIN_BENTLEY_DGN_NAMESPACE

typedef bvector<ClipPrimitivePtr> T_ClipPrimitiveVector;

/*=================================================================================**//**
*  A model classifier provides a spatial classification of a model by filtering display 
*  of a model based on geometry.  These are currently only supported by reality meshes
*  but could apply to any model.
* @bsiclass                                                     Ray.Bentley     07/2017
+===============+===============+===============+===============+===============+======*/
struct ModelSpatialClassifier
{

    enum Display
        {
        DISPLAY_Off = 0,
        DISPLAY_On = 1,
        DISPLAY_Dimmed = 2,
        DISPLAY_Hilite = 3,
        DISPLAY_ElementColor = 4,
        };

    enum Type
        {
        TYPE_Model = 0,
#ifdef NOTYET_SUPPORTED
        TYPE_Element = 1,
        TYPE_NamedGroup = 2,
        TYPE_Category = 3,
#endif
        };

    struct Flags
        {
        unsigned    m_type:5;
        unsigned    m_outsideDisplay:5;
        unsigned    m_insideDisplay:5;
        unsigned    m_selectedDisplay:5;
        unsigned    m_unused:12;

        Flags() : m_type (TYPE_Model), m_outsideDisplay (DISPLAY_On), m_insideDisplay(DISPLAY_On), m_selectedDisplay(DISPLAY_Hilite), m_unused(0)  { }
        Flags(Type type, Display outsideDisplay, Display insideDisplay, Display selectedDisplay) : m_type(type), m_outsideDisplay(outsideDisplay), m_insideDisplay(insideDisplay), m_selectedDisplay(selectedDisplay), m_unused(0) { }
        BentleyStatus FromJson (Json::Value const& value);
        Json::Value ToJson() const;
        };

private:
    DgnModelId      m_modelId;
    Flags           m_flags;
    double          m_expandDistance;
    Utf8String      m_name;

public:
    ModelSpatialClassifier() { }
    ModelSpatialClassifier(DgnModelId modelId, Flags flags, Utf8StringCR name, double expandDistance) : m_modelId(modelId), m_name(name), m_flags(flags), m_expandDistance(expandDistance) { }

    DgnModelId GetModelId()  const { return m_modelId; }
    Utf8String  GetName() const { return m_name; }
    double ExpandDistance() const { return m_expandDistance; }

    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT BentleyStatus FromJson(Json::Value const& value);

};

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     07/2017
+===============+===============+===============+===============+===============+======*/
struct ModelSpatialClassifiers : bvector<ModelSpatialClassifier>
{
    DGNPLATFORM_EXPORT Json::Value ToJson() const;
    DGNPLATFORM_EXPORT BentleyStatus FromJson(Json::Value const& value);

};

END_BENTLEY_DGN_NAMESPACE
