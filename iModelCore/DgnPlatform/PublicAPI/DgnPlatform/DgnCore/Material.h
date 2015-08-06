/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Material.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// __PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/MaterialSettings.h>

BEGIN_BENTLEY_DGN_NAMESPACE

// __PUBLISH_SECTION_END__

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     08/08
+===============+===============+===============+===============+===============+======*/
typedef RefCountedPtr<struct MaterialQvGeometryMap> MaterialQvGeometryMapPtr;

struct MaterialQvGeometryMap : RefCountedBase 
{
private:
    WString         m_cellName;
    bool            m_useCellColors;

protected:
                                    MaterialQvGeometryMap (MaterialMapCR map);

public:
                                    ~MaterialQvGeometryMap ();
    DGNPLATFORM_EXPORT  bool            Matches (MaterialMapCR map) const;
static  MaterialQvGeometryMapPtr    Create (MaterialMapCR map);

};  // MaterialQvGeometryMap

// __PUBLISH_SECTION_START__

typedef RefCountedPtr <Material> MaterialPtr;
//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct Material : public RefCountedBase, public NonCopyableClass
{
private:
// __PUBLISH_SECTION_END__
    Utf8String                                  m_name;
    Utf8String                                  m_palette;
    DgnDbP                                 m_dgnProject;
    mutable bool                                m_sentToQV;
    mutable MaterialQvGeometryMapPtr            m_qvGeometryMap;
    DgnMaterialId                               m_id;

    MaterialSettings                            m_settings;

    explicit Material (DgnDbR dgnProject);
    ~Material ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    static DGNPLATFORM_EXPORT MaterialPtr Create (DgnDbR dgnProject);
    static DGNPLATFORM_EXPORT MaterialPtr Create (MaterialCR initFrom, DgnDbR dgnProject);
    static DGNPLATFORM_EXPORT MaterialCP  FromRendMatId (uintptr_t rendMatId);

    DGNPLATFORM_EXPORT void Copy (MaterialCR copyFrom);
    DGNPLATFORM_EXPORT bool Equals (MaterialCR rhs, bool testDgnProject) const;
    DGNPLATFORM_EXPORT void InitDefaults (DgnDbR dgnProject);

    DGNPLATFORM_EXPORT Utf8StringCR GetName () const;
    DGNPLATFORM_EXPORT Utf8StringR GetNameR ();

    DGNPLATFORM_EXPORT DgnMaterialId GetId () const;
    DGNPLATFORM_EXPORT void SetId (DgnMaterialId id);

    DGNPLATFORM_EXPORT Utf8StringCR GetPalette() const;
    DGNPLATFORM_EXPORT Utf8StringR GetPaletteR ();

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR () const;
    DGNPLATFORM_EXPORT void SetDgnProject (DgnDbR dgnProject);

    DGNPLATFORM_EXPORT MaterialSettingsCR GetSettings () const;
    DGNPLATFORM_EXPORT MaterialSettingsR GetSettingsR ();

// __PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT bool GetSentToQV () const;
    DGNPLATFORM_EXPORT void SetSentToQV (bool sentToQV) const;

    DGNPLATFORM_EXPORT double GetLayerScaleInUORs (MaterialMapLayerCR layer) const;

    static DGNPLATFORM_EXPORT WString ParseLegacyPaletteName (WCharCP legacyName);

    // Geometry map support
    DGNPLATFORM_EXPORT MaterialMapCP        GetGeometryMap() const;  

    DGNPLATFORM_EXPORT bool                 NeedsQvGeometryTexture () const;
    DGNPLATFORM_EXPORT void                 DefineQvGeometryTexture () const;
    DGNPLATFORM_EXPORT uintptr_t            GetQvGeometryTexture () const;
    DGNPLATFORM_EXPORT void                 ClearQvGeometryTexture () const;

// __PUBLISH_SECTION_START__
};

typedef bvector <MaterialCP> MaterialList;

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialManager : public NonCopyableClass
{
private:
// __PUBLISH_SECTION_END__
    MaterialManager ();

    void DefineQVMaterial (MaterialCR material, uintptr_t materialId);

    void GetQVPatternTransform (double transform[2][3], MaterialCR material, MaterialMapCR map, MaterialMapLayerCR layer);

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
// __PUBLISH_SECTION_END__

    void DefineQVTexture (MaterialCR material, uintptr_t materialId, DgnViewportP viewport, RgbFactorCP elementColor = NULL);
    void DefineQVTextureMapping (MaterialCR material, MaterialMapCR map, MaterialMapLayerCR layer, uintptr_t textureId, uintptr_t materialId);
    //! Send material information to qv.
    //! @param[in] material         material to send
    //! @param[in] elementColor     element color if material uses by element color settings
    //! @param[in] viewport         optional viewport view port associated with the material update
    BentleyStatus SendMaterialToQV (MaterialCR material, ColorDef elementColor, DgnViewportP viewport);

    DGNVIEW_EXPORT void DefineDefaultQVMaterial ();

    MaterialPtr CreateFromXml (Utf8CP xmlString, DgnDbR source);

// __PUBLISH_SECTION_START__
    DGNPLATFORM_EXPORT MaterialCP FindMaterial (DgnMaterialId id, DgnDbR source);
    DGNPLATFORM_EXPORT BentleyStatus FindMaterial (MaterialList& materials, Utf8CP name, DgnDbR source);
};

END_BENTLEY_DGN_NAMESPACE

/** @endcond */

