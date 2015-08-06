/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/MaterialSettings.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if !defined (resource) && !defined (type_resource_generator)

// __PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "LxoProcedure.h"
#include <list>

DGNPLATFORM_TYPEDEFS (MaterialSettings)
DGNPLATFORM_TYPEDEFS (MaterialMap)
DGNPLATFORM_TYPEDEFS (MaterialMapCollection)
DGNPLATFORM_TYPEDEFS (MaterialMapLayer)
DGNPLATFORM_TYPEDEFS (MaterialMapLayerCollection)
DGNPLATFORM_TYPEDEFS (MaterialMapLayerIterator)
DGNPLATFORM_TYPEDEFS (MaterialShader)
DGNPLATFORM_TYPEDEFS (MaterialShaderCollection)
DGNPLATFORM_TYPEDEFS (MaterialShaderIterator)
DGNPLATFORM_TYPEDEFS (MaterialFur)
DGNPLATFORM_TYPEDEFS (MaterialProjectionParameters)

BEGIN_BENTLEY_DGN_NAMESPACE

// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)

enum MaterialVersion
    {
    MATERIALVERSION_Invalid                     = 0,
    MATERIALVERSION_BeforeGlowMap               = 2,
    MATERIALVERSION_AfterGlowMap                = 3,
    MATERIALVERSION_SplitBumpDisplacementMap    = 4,
    MATERIALVERSION_ReflectFresnel              = 5,
    MATERIALVERSION_ReflectColor                = 6,
    MATERIALVERSION_SplitFresnelFromSpecular    = 7,
    MATERIALVERSION_Antialiasing                = 8,
    MATERIALVERSION_Current                     = MATERIALVERSION_Antialiasing,
    };

#define LAYEREDPROCEDURALNAME L"layers.pma"
// __PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
enum class MapMode
    {
    None                    = -1,
    Parametric              = 0,
    ElevationDrape          = 1,
    Planar                  = 2,
    DirectionalDrape        = 3,
    Cubic                   = 4,
    Spherical               = 5,
    Cylindrical             = 6,
    Solid                   = 7,
    //! Only valid for lights.
    FrontProject            = 8,
// __PUBLISH_SECTION_END__
    // Only valid in luxology export
    ImplicitUV              = 9,
// __PUBLISH_SECTION_START__
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
enum class MapUnits
    {
    Relative               = 0,
    MasterUnits            = 1,
    SubUnits               = 2,
    Meters                 = 3,
    Millimeters            = 4,
    Feet                   = 5,
    Inches                 = 6,
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     08/11
//=======================================================================================
enum class ProjectionAttachmentType
    {
    None           = -1,
    Material       = 0,
    Group          = 1,
    Element        = 2,
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     08/11
//=======================================================================================
enum class ProjectionVariant
    {
    None                  = 0,
    CylindricalCapped     = 1,
    };

// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

typedef RefCountedPtr <MaterialMapLayer> MaterialMapLayerPtr;

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialMapLayer : public RefCountedBase, public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct MaterialMapLayerCollection;
// __PUBLISH_SECTION_START__

public:
    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

    enum LayerType
        {
        LAYERTYPE_None                  = 0,
        LAYERTYPE_Image                 = 1,
        LAYERTYPE_Procedure             = 2,
        LAYERTYPE_Gradient              = 3,
        LAYERTYPE_LxoProcedure          = 4,
        LAYERTYPE_Cell                  = 5,
        LAYERTYPE_8119LxoProcedure      = 6, // only used in internal material io
        LAYERTYPE_Operator              = 0xf000,
        LAYERTYPE_Normal                = 0xf000,
        LAYERTYPE_Add                   = LAYERTYPE_Operator + 0x01,
        LAYERTYPE_Subtract              = LAYERTYPE_Operator + 0x02,
        LAYERTYPE_Alpha                 = LAYERTYPE_Operator + 0x03,
        LAYERTYPE_Dissolve              = LAYERTYPE_Operator + 0x04,
        LAYERTYPE_Atop                  = LAYERTYPE_Operator + 0x05,
        LAYERTYPE_In                    = LAYERTYPE_Operator + 0x06,
        LAYERTYPE_Out                   = LAYERTYPE_Operator + 0x07,
        LAYERTYPE_UnaryOperator         = LAYERTYPE_Operator + 0x0A,
        LAYERTYPE_Offset                = LAYERTYPE_Operator + 0x0A,
        LAYERTYPE_Scale                 = LAYERTYPE_Operator + 0x0B,
        LAYERTYPE_Gamma                 = LAYERTYPE_Operator + 0x0C,
        LAYERTYPE_Tint                  = LAYERTYPE_Operator + 0x0D,
        LAYERTYPE_Brightness            = LAYERTYPE_Operator + 0x0E,
        LAYERTYPE_Contrast              = LAYERTYPE_Operator + 0x0F,
        LAYERTYPE_Negate                = LAYERTYPE_Operator + 0x10,
        LAYERTYPE_Equalize              = LAYERTYPE_Operator + 0x11,
        LAYERTYPE_GroupStart            = LAYERTYPE_Operator + 0x12,
        LAYERTYPE_GroupEnd              = LAYERTYPE_Operator + 0x13,
        LAYERTYPE_AlphaBackgroundStart  = LAYERTYPE_Operator + 0x14,
        LAYERTYPE_AlphaBackgroundEnd    = LAYERTYPE_Operator + 0x15,
        LAYERTYPE_Difference            = LAYERTYPE_Operator + 0x16,
        LAYERTYPE_NormalMultiply        = LAYERTYPE_Operator + 0x17,
        LAYERTYPE_Divide                = LAYERTYPE_Operator + 0x18,
        LAYERTYPE_Multiply              = LAYERTYPE_Operator + 0x19,
        LAYERTYPE_Screen                = LAYERTYPE_Operator + 0x1A,
        LAYERTYPE_Overlay               = LAYERTYPE_Operator + 0x1B,
        LAYERTYPE_SoftLight             = LAYERTYPE_Operator + 0x1C,
        LAYERTYPE_HardLight             = LAYERTYPE_Operator + 0x1D,
        LAYERTYPE_Darken                = LAYERTYPE_Operator + 0x1E,
        LAYERTYPE_Lighten               = LAYERTYPE_Operator + 0x1F,
        LAYERTYPE_ColorDodge            = LAYERTYPE_Operator + 0x20,
        LAYERTYPE_ColorBurn             = LAYERTYPE_Operator + 0x21,
        };

    //=======================================================================================
    // @bsiclass                                                    PaulChater     08/11
    //=======================================================================================
    enum TextureFilterType
        {
        TEXTUREFILTERTYPE_Nearest                   = 0,
        TEXTUREFILTERTYPE_Bilinear                  = 1,
        TEXTUREFILTERTYPE_Bicubic                   = 2,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

// __PUBLISH_SECTION_END__
    typedef bvector <WString>               LegacyProcedureData;
    typedef LegacyProcedureData*            LegacyProcedureDataP;
    typedef LegacyProcedureData const*      LegacyProcedureDataCP;
    typedef LegacyProcedureData&            LegacyProcedureDataR;
    typedef LegacyProcedureData const&      LegacyProcedureDataCR;
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    // This structure (m_basicFlags) is serialized in V8 DGN files.
    struct
        {
        unsigned int m_flipV            : 1;
        unsigned int m_lockSize         : 1;
        unsigned int m_capped           : 1;
        unsigned int m_lockProjection   : 1;
        unsigned int m_flipU            : 1;
        unsigned int m_decalU           : 1;
        unsigned int m_decalV           : 1;
        unsigned int m_mirrorU          : 1;
        unsigned int m_mirrorV          : 1;
        unsigned int m_snappable        : 1;
        unsigned int m_useCellColors    : 1;
        unsigned int m_antialiasing     : 1;
        unsigned int m_padding          : 19;
        unsigned int m_mark             : 1;
        } m_basicFlags;

    bool                                    m_enabled;
    WString                                 m_fileName; // NEEDS WORK - should be moniker
    MapMode                                 m_mode;
    MapUnits                                m_units;
    double                                  m_rotation;
    DPoint3d                                m_scale;
    DPoint3d                                m_offset;
    double                                  m_opacity;
    bool                                    m_invert;
    bool                                    m_bgTrans;
    LxoProcedurePtr                         m_lxoProcedure;
    LegacyProcedureData                     m_legacyProcedureData;
    double                                  m_gamma;
    TextureFilterType                       m_textureFilterType;
    double                                  m_lowValue;           // Only used with displacement for now.
    double                                  m_highValue;
    double                                  m_antialiasStrength;
    double                                  m_minimumSpot;

    LayerType                               m_type;

    // The operator value used is determined by the layer type.
    double                                  m_operatorDouble;
    RgbFactor                               m_operatorColor;
    WString                                 m_operatorString;

    MaterialMapLayer ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
// __PUBLISH_SECTION_END__
    //! Internal convenience methods since V8 dgns serialized m_basicFlags as a single UInt32.
    DGNPLATFORM_EXPORT uint32_t const* GetBasicFlagsCP () const;
    DGNPLATFORM_EXPORT void SetBasicFlags (uint32_t flags);

    DGNPLATFORM_EXPORT LegacyProcedureDataCR GetLegacyProcedureData () const;
    DGNPLATFORM_EXPORT LegacyProcedureDataR GetLegacyProcedureDataR ();
// __PUBLISH_SECTION_START__

    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialMapLayerCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialMapLayerCR rhs);

    DGNPLATFORM_EXPORT bool IsEnabled () const;
    DGNPLATFORM_EXPORT void SetIsEnabled (bool isEnabled);

    DGNPLATFORM_EXPORT bool GetFlipU () const;
    DGNPLATFORM_EXPORT void SetFlipU (bool flip);

    DGNPLATFORM_EXPORT bool GetFlipV () const;
    DGNPLATFORM_EXPORT void SetFlipV (bool flip);

    DGNPLATFORM_EXPORT bool GetMirrorU () const;
    DGNPLATFORM_EXPORT void SetMirrorU (bool mirror);

    DGNPLATFORM_EXPORT bool GetMirrorV () const;
    DGNPLATFORM_EXPORT void SetMirrorV (bool mirror);

    DGNPLATFORM_EXPORT bool GetRepeatU () const;
    DGNPLATFORM_EXPORT void SetRepeatU (bool repeat);

    DGNPLATFORM_EXPORT bool GetRepeatV () const;
    DGNPLATFORM_EXPORT void SetRepeatV (bool repeat);

    DGNPLATFORM_EXPORT bool GetLockSize () const;
    DGNPLATFORM_EXPORT void SetLockSize (bool lockSize);

    DGNPLATFORM_EXPORT bool IsCapped () const;
    DGNPLATFORM_EXPORT void SetIsCapped (bool isCapped);

    DGNPLATFORM_EXPORT bool IsProjectionLocked () const;
    DGNPLATFORM_EXPORT void SetIsProjectionLocked (bool isLocked);

    DGNPLATFORM_EXPORT bool UseCellColors () const;
    DGNPLATFORM_EXPORT void SetUseCellColors (bool useCellColors);

    DGNPLATFORM_EXPORT bool IsSnappable () const;
    DGNPLATFORM_EXPORT void SetIsSnappable (bool isSnappable);

    DGNPLATFORM_EXPORT bool IsAntialiasing () const;
    DGNPLATFORM_EXPORT void SetIsAntialiasing (bool antialiasing);

    DGNPLATFORM_EXPORT WStringCR GetFileName () const;
    DGNPLATFORM_EXPORT void SetFileName (WCharCP fileName);

    DGNPLATFORM_EXPORT MapMode GetMode () const;
    DGNPLATFORM_EXPORT void SetMode (MapMode mode);

    DGNPLATFORM_EXPORT MapUnits GetUnits () const;
    DGNPLATFORM_EXPORT void SetUnits (MapUnits units);

    DGNPLATFORM_EXPORT double GetRotation () const;
    DGNPLATFORM_EXPORT void SetRotation (double rotation);

    DGNPLATFORM_EXPORT DPoint3dCR GetScale () const;
    DGNPLATFORM_EXPORT DPoint3dR GetScaleR ();
    DGNPLATFORM_EXPORT void SetScale (double x, double y, double z);

    DGNPLATFORM_EXPORT DPoint3dCR GetOffset () const;
    DGNPLATFORM_EXPORT DPoint3dR GetOffsetR ();
    DGNPLATFORM_EXPORT void SetOffset (double x, double y, double z);

    //! These values express the offset value adjusted to units of the map's scale, with the sign of the x coordinate reversed.
    //! This is how the material map editor presents the actual stored (GetOffsetR) value to the user
    DGNPLATFORM_EXPORT void GetAdjustedOffset (DPoint3dR offset) const;
    DGNPLATFORM_EXPORT void SetAdjustedOffset (double x, double y, double z);
    DGNPLATFORM_EXPORT void SetAdjustedOffset (DPoint3dCR offset);

    DGNPLATFORM_EXPORT double GetOpacity () const;
    DGNPLATFORM_EXPORT void SetOpacity (double opacity);

    DGNPLATFORM_EXPORT double GetGamma () const;
    DGNPLATFORM_EXPORT void SetGamma (double gamma);

    DGNPLATFORM_EXPORT bool IsInverted () const;
    DGNPLATFORM_EXPORT void SetIsInverted (bool isInverted);

    DGNPLATFORM_EXPORT bool IsBackgroundTransparent () const;
    DGNPLATFORM_EXPORT void SetIsBackgroundTransparent (bool isTransparent);

    DGNPLATFORM_EXPORT double GetLowValue () const;
    DGNPLATFORM_EXPORT void SetLowValue (double lowValue);

    DGNPLATFORM_EXPORT double GetHighValue () const;
    DGNPLATFORM_EXPORT void SetHighValue (double highValue);

    DGNPLATFORM_EXPORT double GetAntiAliasStrength () const;
    DGNPLATFORM_EXPORT void SetAntiAliasStrength (double antiAliasStrength);

    DGNPLATFORM_EXPORT double GetMinimumSpot () const;
    DGNPLATFORM_EXPORT void SetMinimumSpot (double minimumSpot);

    DGNPLATFORM_EXPORT TextureFilterType GetTextureFilterType () const;
    DGNPLATFORM_EXPORT void SetTextureFilterType (TextureFilterType type);

    DGNPLATFORM_EXPORT LayerType GetType () const;
    DGNPLATFORM_EXPORT void SetType (LayerType type);

    DGNPLATFORM_EXPORT double GetOperatorDoubleValue () const;
    DGNPLATFORM_EXPORT void SetOperatorDoubleValue (double value);

    DGNPLATFORM_EXPORT RgbFactor const& GetOperatorColorValue () const;
    DGNPLATFORM_EXPORT RgbFactor& GetOperatorColorValueR ();
    DGNPLATFORM_EXPORT void SetOperatorColorValue (double red, double green, double blue);

    DGNPLATFORM_EXPORT WStringCR GetOperatorStringValue () const;
    DGNPLATFORM_EXPORT void SetOperatorStringValue (WCharCP value);

    DGNPLATFORM_EXPORT LxoProcedureCP GetLxoProcedureCP () const;
    DGNPLATFORM_EXPORT LxoProcedureP GetLxoProcedureP ();

    //! If an LxoProcedure already exists for this map, it will be replaced.
    DGNPLATFORM_EXPORT LxoProcedureP AddLxoProcedure (LxoProcedure::ProcedureType type);
    DGNPLATFORM_EXPORT void ClearLxoProcedure ();
};


// __PUBLISH_SECTION_END__
typedef std::list<MaterialMapLayerPtr> MaterialMapLayerPtrList; // WIP_MATERIALS -- DON'T USE STD::LIST IN AN API
// __PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass                                                    MattGooding     06/10
//=======================================================================================
struct MaterialMapLayerIterator
    {
// __PUBLISH_SECTION_END__
    friend struct MaterialMapLayerCollection;
// __PUBLISH_SECTION_START__
    private:
        void*   m_iter;

        MaterialMapLayerIterator (MaterialMapLayerCollectionR layers, bool wantBegin);
// __PUBLISH_SECTION_END__
        MaterialMapLayerIterator (MaterialMapLayerPtrList::iterator iter);
// __PUBLISH_SECTION_START__

    public:
        DGNPLATFORM_EXPORT ~MaterialMapLayerIterator ();
        DGNPLATFORM_EXPORT MaterialMapLayerIterator (MaterialMapLayerIterator const&);

        DGNPLATFORM_EXPORT MaterialMapLayerIteratorR operator++ ();
        DGNPLATFORM_EXPORT MaterialMapLayerIteratorR operator-- ();
        DGNPLATFORM_EXPORT MaterialMapLayerR operator* ();
        DGNPLATFORM_EXPORT MaterialMapLayerP operator-> ();
        DGNPLATFORM_EXPORT bool operator== (MaterialMapLayerIteratorCR rhs) const;
        DGNPLATFORM_EXPORT bool operator!= (MaterialMapLayerIteratorCR rhs) const;
// __PUBLISH_SECTION_END__
        MaterialMapLayerPtrList::iterator Get ();
// __PUBLISH_SECTION_START__
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     06/10
//=======================================================================================
struct MaterialMapLayerConstIterator
    {
// __PUBLISH_SECTION_END__
    friend struct MaterialMapLayerCollection;
// __PUBLISH_SECTION_START__
    private:
        void*   m_iter;

        MaterialMapLayerConstIterator (MaterialMapLayerCollectionCR layers, bool wantBegin);

    public:
        DGNPLATFORM_EXPORT ~MaterialMapLayerConstIterator ();
        DGNPLATFORM_EXPORT MaterialMapLayerConstIterator (MaterialMapLayerConstIterator const&);

        DGNPLATFORM_EXPORT MaterialMapLayerConstIterator& operator++ ();
        DGNPLATFORM_EXPORT MaterialMapLayerConstIterator& operator-- ();
        DGNPLATFORM_EXPORT MaterialMapLayerCR operator* () const;
        DGNPLATFORM_EXPORT MaterialMapLayerCP operator-> () const;
        DGNPLATFORM_EXPORT bool operator== (MaterialMapLayerConstIterator const& rhs) const;
        DGNPLATFORM_EXPORT bool operator!= (MaterialMapLayerConstIterator const& rhs) const;
// __PUBLISH_SECTION_END__
        MaterialMapLayerPtrList::const_iterator Get () const;
// __PUBLISH_SECTION_START__
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialMapLayerCollection : public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct MaterialMap;
    friend struct MaterialMapLayerIterator;
    friend struct MaterialMapLayerConstIterator;
// __PUBLISH_SECTION_START__
private:
// __PUBLISH_SECTION_END__
    MaterialMapLayerPtrList         m_layers;
public:
    DGNPLATFORM_EXPORT MaterialMapLayerCollection ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialMapLayerCollectionCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialMapLayerCollectionCR rhs);

    DGNPLATFORM_EXPORT MaterialMapLayerIterator begin ();
    DGNPLATFORM_EXPORT MaterialMapLayerConstIterator begin () const;
    DGNPLATFORM_EXPORT MaterialMapLayerIterator end ();
    DGNPLATFORM_EXPORT MaterialMapLayerConstIterator end () const;

    DGNPLATFORM_EXPORT size_t Size () const;
    DGNPLATFORM_EXPORT MaterialMapLayerCR GetTopLayer () const;
    DGNPLATFORM_EXPORT MaterialMapLayerR GetTopLayerR ();
    DGNPLATFORM_EXPORT MaterialMapLayerR AddLayer ();
    DGNPLATFORM_EXPORT MaterialMapLayerR AddLayerToFront ();
    DGNPLATFORM_EXPORT MaterialMapLayerIterator InsertLayer (MaterialMapLayerIterator& pos);
    DGNPLATFORM_EXPORT MaterialMapLayerCR GetLastDataLayer () const;

    //! Deleting the last layer will cause a default layer to be added - maps must always have at least one layer.
    DGNPLATFORM_EXPORT BentleyStatus DeleteLayer (MaterialMapLayerIterator& iter);
    DGNPLATFORM_EXPORT BentleyStatus DeleteLayers (MaterialMapLayerIterator& startIter, MaterialMapLayerIterator& endIter);
// __PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT void DeleteAllLayers ();
    DGNPLATFORM_EXPORT void InsertCollection (MaterialMapLayerCollectionR collection, MaterialMapLayerIterator& iter);
// __PUBLISH_SECTION_START__

};

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialMap : public RefCountedBase, public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct MaterialMapCollection;
// __PUBLISH_SECTION_START__

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum MapType
        {
        MAPTYPE_None                    = 0,
        MAPTYPE_Pattern                 = 1,
        MAPTYPE_Bump                    = 2,
        MAPTYPE_Specular                = 3,
        MAPTYPE_Reflect                 = 4,
        MAPTYPE_Transparency            = 5,
        MAPTYPE_Translucency            = 6,
        MAPTYPE_Finish                  = 7,
        MAPTYPE_Diffuse                 = 8,
        MAPTYPE_GlowAmount              = 9,
        MAPTYPE_ClearcoatAmount         = 10,
        MAPTYPE_AnisotropicDirection    = 11,
        MAPTYPE_SpecularColor           = 12,
        MAPTYPE_TransparentColor        = 13,
        MAPTYPE_TranslucencyColor       = 14,
        MAPTYPE_Displacement            = 15,
        MAPTYPE_Normal                  = 16,
        MAPTYPE_FurLength               = 17,
        MAPTYPE_FurDensity              = 18,
        MAPTYPE_FurJitter               = 19,
        MAPTYPE_FurFlex                 = 20,
        MAPTYPE_FurClumps               = 21,
        MAPTYPE_FurDirection            = 22,
        MAPTYPE_FurVector               = 23,
        MAPTYPE_FurBump                 = 24,
        MAPTYPE_FurCurls                = 25,
        MAPTYPE_GlowColor               = 26,
        MAPTYPE_ReflectColor            = 27,
        MAPTYPE_RefractionRoughness     = 28,
        MAPTYPE_SpecularFresnel         = 29,
        MAPTYPE_Geometry                = 30,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    MapType                                         m_type;
    MapType                                         m_linkType;
    bool                                            m_enabled;
    double                                          m_value;
    DPoint3d                                        m_projectionOffset;
    DPoint3d                                        m_projectionRotation;
    DPoint3d                                        m_projectionScale;
    mutable MaterialMapLayerCollection              m_layers;
    MaterialMapCollectionR                          m_mapCollection;

    MaterialMap (MapType type, MaterialMapCollectionR collection);

    MaterialMapCP GetLinkedMapCP () const;
    MaterialMapP GetLinkedMapP ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialMapCR rhs) const;

    //! This will copy everything except the map's type - map type must be set when a map is first created.
    DGNPLATFORM_EXPORT void Copy (MaterialMapCR rhs);

// __PUBLISH_SECTION_END__
    //! Check if this map uses its values for color or for manipulating some property.  Value maps can be inverted.
    DGNPLATFORM_EXPORT bool IsValueMap () const;
    static DGNPLATFORM_EXPORT bool IsValueMap (MaterialMap::MapType type);
// __PUBLISH_SECTION_START__

    DGNPLATFORM_EXPORT MapType GetType () const;

    DGNPLATFORM_EXPORT MapType GetLinkType () const;
    DGNPLATFORM_EXPORT BentleyStatus SetLinkType (MapType type);

    DGNPLATFORM_EXPORT bool IsEnabled () const;
    DGNPLATFORM_EXPORT void SetIsEnabled (bool isEnabled);

    //! This is only used by bump and pattern maps.  For bump maps, it expresses the height of the map.  For pattern maps, it expresses the weight of the pattern
    //! relative to the base material color.
    DGNPLATFORM_EXPORT double GetValue () const;
    DGNPLATFORM_EXPORT void SetValue (double value);

    //! If GetLinkType () != MapType_None the linked maps value will be returned/set
    DGNPLATFORM_EXPORT DPoint3dCR GetProjectionOffset () const;
    DGNPLATFORM_EXPORT DPoint3dR GetProjectionOffsetR ();
    DGNPLATFORM_EXPORT void SetProjectionOffset (double x, double y, double z);

    //! If GetLinkType () != MapType_None the linked maps value will be returned/set
    DGNPLATFORM_EXPORT DPoint3dCR GetProjectionRotation () const;
    DGNPLATFORM_EXPORT DPoint3dR GetProjectionRotationR ();
    DGNPLATFORM_EXPORT void SetProjectionRotation (double x, double y, double z);

    //! If GetLinkType () != MapType_None the linked maps value will be returned/set
    DGNPLATFORM_EXPORT DPoint3dCR GetProjectionScale () const;
    DGNPLATFORM_EXPORT DPoint3dR GetProjectionScaleR ();
    DGNPLATFORM_EXPORT void SetProjectionScale (double x, double y, double z);

    //! If GetLinkType () != MapType_None the linked maps value will be returned/set
    DGNPLATFORM_EXPORT MaterialMapLayerCollectionCR GetLayers () const;
    DGNPLATFORM_EXPORT MaterialMapLayerCollectionR GetLayersR ();
};

typedef RefCountedPtr<MaterialMap> MaterialMapPtr;
typedef bmap <MaterialMap::MapType, MaterialMapPtr> MaterialMapPtrList;

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialMapCollection : public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct MaterialSettings;
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    MaterialMapPtrList                                          m_maps;

    MaterialMapCollection ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    typedef MaterialMapPtrList::const_iterator                  const_iterator;
    typedef MaterialMapPtrList::iterator                        iterator;

    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialMapCollectionCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialMapCollectionCR rhs);

    DGNPLATFORM_EXPORT const_iterator begin () const;
    DGNPLATFORM_EXPORT iterator begin ();
    DGNPLATFORM_EXPORT const_iterator end () const;
    DGNPLATFORM_EXPORT iterator end ();

    DGNPLATFORM_EXPORT size_t Size () const;

    DGNPLATFORM_EXPORT MaterialMapCP GetMapCP (MaterialMap::MapType type) const;
    DGNPLATFORM_EXPORT MaterialMapP GetMapP (MaterialMap::MapType type);
    DGNPLATFORM_EXPORT void DeleteMap (MaterialMap::MapType type);

    //! If a map of this type is already associated with this material, it will be replaced.
    DGNPLATFORM_EXPORT MaterialMapP AddMap (MaterialMap::MapType type);
};

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialFur : public RefCountedBase, public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct MaterialSettings;
// __PUBLISH_SECTION_START__

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum FurType
        {
        FURTYPE_Strips      = 0,
        FURTYPE_Cylinders   = 1,
        };

    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum FurGuides
        {
        FURGUIDES_None              = 0,
        FURGUIDES_Clump             = 1,
        FURGUIDES_Direction         = 2,
        FURGUIDES_DirectionLength   = 3,
        FURGUIDES_Shape             = 4,
        FURGUIDES_Range             = 5,
        };

    //=======================================================================================
    // @bsiclass                                                    PaulChater     08/11
    //=======================================================================================
    enum FurBillboard
        {
        FURBILLBOARD_Off    = 0,
        FURBILLBOARD_Tree   = 1,
        FURBILLBOARD_Leaves = 2,
        };

// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    // This structure (m_flags) is serialized in V8 DGN files.
    struct
        {
        unsigned int m_adaptiveSampling     : 1;
        unsigned int m_removeBaseSurface    : 1;
        unsigned int m_autoFading           : 1;
        unsigned int m_useHairShader        : 1;
        unsigned int m_useFurMaterial       : 1;
        unsigned int m_frustumCulling       : 1;
        unsigned int m_padding              : 26;
        } m_flags;

    double                          m_spacing;          // Stored in meters
    double                          m_length;           // Stored in meters
    double                          m_width;
    double                          m_taper;
    double                          m_offset;           // Stored in meters
    double                          m_stripRotation;
    double                          m_growthJitter;
    double                          m_positionJitter;
    double                          m_directionJitter;
    double                          m_sizeJitter;
    double                          m_flex;
    double                          m_rootBend;
    double                          m_curls;
    double                          m_bumpAmplitude;
    double                          m_guideRange;       // Stored in meters
    double                          m_guideLength;
    double                          m_blendAmount;
    double                          m_blendAngle;
    double                          m_clumps;
    double                          m_clumpRange;       // Stored in meters
    double                          m_rate;

    FurType                         m_type;
    FurGuides                       m_guides;
    uint32_t                        m_segments;
    FurBillboard                    m_billboard;

    Utf8String                      m_furMaterialName;
    Utf8String                      m_furMaterialPalette;

    MaterialFur ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
// __PUBLISH_SECTION_END__
    // Internal methods since V8 dgns serialized m_flags as a single UInt32.
    uint32_t const* GetFlagsCP () const;
    void SetFlags (uint32_t flags);
// __PUBLISH_SECTION_START__

    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialFurCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialFurCR rhs);

    DGNPLATFORM_EXPORT bool UseAdaptiveSampling () const;
    DGNPLATFORM_EXPORT void SetUseAdaptiveSampling (bool useAdaptive);

    DGNPLATFORM_EXPORT bool RemoveBaseSurface () const;
    DGNPLATFORM_EXPORT void SetRemoveBaseSurface (bool removeBase);

    DGNPLATFORM_EXPORT bool UseAutoFading () const;
    DGNPLATFORM_EXPORT void SetUseAutoFading (bool useAutoFading);

    DGNPLATFORM_EXPORT bool UseHairShader () const;
    DGNPLATFORM_EXPORT void SetUseHairShader (bool useHairShader);

    DGNPLATFORM_EXPORT bool UseFurMaterial () const;
    DGNPLATFORM_EXPORT void SetUseFurMaterial (bool useFurMaterial);

    DGNPLATFORM_EXPORT bool UseFrustumCulling () const;
    DGNPLATFORM_EXPORT void SetUseFrustumCulling (bool frustumCulling);

    DGNPLATFORM_EXPORT double GetSpacingInMeters () const;
    DGNPLATFORM_EXPORT void SetSpacingInMeters (double spacing);

    DGNPLATFORM_EXPORT double GetLengthInMeters () const;
    DGNPLATFORM_EXPORT void SetLengthInMeters (double length);

    //! Width of individual strands at the root.  Expressed as a percentage of the spacing distance.
    DGNPLATFORM_EXPORT double GetWidthScale () const;
    DGNPLATFORM_EXPORT void SetWidthScale (double width);

    DGNPLATFORM_EXPORT double GetTaperScale () const;
    DGNPLATFORM_EXPORT void SetTaperScale (double taper);

    DGNPLATFORM_EXPORT double GetOffsetInMeters () const;
    DGNPLATFORM_EXPORT void SetOffsetInMeters (double offset);

    DGNPLATFORM_EXPORT double GetStripRotation () const;
    DGNPLATFORM_EXPORT void SetStripRotation (double rotation);

    DGNPLATFORM_EXPORT double GetGrowthJitter () const;
    DGNPLATFORM_EXPORT void SetGrowthJitter (double jitter);

    DGNPLATFORM_EXPORT double GetPositionJitter () const;
    DGNPLATFORM_EXPORT void SetPositionJitter (double jitter);

    DGNPLATFORM_EXPORT double GetDirectionJitter () const;
    DGNPLATFORM_EXPORT void SetDirectionJitter (double jitter);

    DGNPLATFORM_EXPORT double GetSizeJitter () const;
    DGNPLATFORM_EXPORT void SetSizeJitter (double jitter);

    DGNPLATFORM_EXPORT double GetFlex () const;
    DGNPLATFORM_EXPORT void SetFlex (double flex);

    DGNPLATFORM_EXPORT double GetRootBend () const;
    DGNPLATFORM_EXPORT void SetRootBend (double rootBend);

    DGNPLATFORM_EXPORT double GetCurls () const;
    DGNPLATFORM_EXPORT void SetCurls (double curls);

    DGNPLATFORM_EXPORT double GetBumpAmplitude () const;
    DGNPLATFORM_EXPORT void SetBumpAmplitude (double amplitude);

    DGNPLATFORM_EXPORT double GetGuideRangeInMeters () const;
    DGNPLATFORM_EXPORT void SetGuideRangeInMeters (double range);

    DGNPLATFORM_EXPORT double GetGuideLengthInMeters () const;
    DGNPLATFORM_EXPORT void SetGuideLengthInMeters (double length);

    DGNPLATFORM_EXPORT double GetBlendAmount () const;
    DGNPLATFORM_EXPORT void SetBlendAmount (double amount);

    DGNPLATFORM_EXPORT double GetBlendAngle () const;
    DGNPLATFORM_EXPORT void SetBlendAngle (double angle);

    //! Determines closeness of fur clumps.
    DGNPLATFORM_EXPORT double GetClumpScale () const;
    DGNPLATFORM_EXPORT void SetClumpScale (double scale);

    DGNPLATFORM_EXPORT double GetClumpRangeInMeters () const;
    DGNPLATFORM_EXPORT void SetClumpRangeInMeters (double range);

    DGNPLATFORM_EXPORT double GetRate () const;
    DGNPLATFORM_EXPORT void SetRate (double rate);

    DGNPLATFORM_EXPORT FurType GetType () const;
    DGNPLATFORM_EXPORT void SetType (FurType type);

    DGNPLATFORM_EXPORT FurGuides GetFurGuides () const;
    DGNPLATFORM_EXPORT void SetFurGuides (FurGuides guides);

    DGNPLATFORM_EXPORT uint32_t GetSegmentCount () const;
    DGNPLATFORM_EXPORT void SetSegmentCount (uint32_t count);

    DGNPLATFORM_EXPORT FurBillboard GetFurBillboard () const;
    DGNPLATFORM_EXPORT void SetFurBillboard (FurBillboard billboard);

    DGNPLATFORM_EXPORT Utf8StringCR GetFurMaterialName () const;
    DGNPLATFORM_EXPORT Utf8StringR GetFurMaterialNameR ();

    DGNPLATFORM_EXPORT Utf8StringCR GetFurMaterialPalette () const;
    DGNPLATFORM_EXPORT Utf8StringR GetFurMaterialPaletteR ();
};

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialShader : public RefCountedBase, public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct MaterialShaderCollection;
// __PUBLISH_SECTION_START__

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum ShaderType
        {
        SHADERTYPE_Default          = 0,
        SHADERTYPE_MultiMaterial    = 1,
        };

    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum IlluminationType
        {
        ILLUMTYPE_None                  = 0,
        ILLUMTYPE_MonteCarlo            = 1,
        ILLUMTYPE_IrradianceCaching     = 2,
        };

    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum BlendMode
        {
        BLENDMODE_Normal            = 0,
        BLENDMODE_Add               = 1,
        BLENDMODE_Subtract          = 2,
        BLENDMODE_Difference        = 3,
        BLENDMODE_NormalMultiply    = 4,
        BLENDMODE_Divide            = 5,
        BLENDMODE_Multiply          = 6,
        BLENDMODE_Screen            = 7,
        BLENDMODE_Overlay           = 8,
        BLENDMODE_SoftLight         = 9,
        BLENDMODE_HardLight         = 10,
        BLENDMODE_Darken            = 11,
        BLENDMODE_Lighten           = 12,
        BLENDMODE_ColorDodge        = 13,
        BLENDMODE_ColorBurn         = 14,
        };

    //=======================================================================================
    // @bsiclass                                                    MattGooding     01/10
    //=======================================================================================
    enum ShaderEffect
        {
        SHADEREFFECT_FullShading    = 0,
        SHADEREFFECT_Diffuse        = 1,
        SHADEREFFECT_Luminous       = 2,
        SHADEREFFECT_Reflection     = 3,
        SHADEREFFECT_Specular       = 4,
        SHADEREFFECT_SubSurface     = 5,
        SHADEREFFECT_Transparent    = 6,
        SHADEREFFECT_Fog            = 7,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    // This structure (m_flags) is serialized in V8 DGN files.
    struct
    {
        unsigned int m_enabled                  : 1;
        unsigned int m_invert                   : 1;
        unsigned int m_receiveShadows           : 1;
        unsigned int m_visibleToCamera          : 1;
        unsigned int m_visibleToIndirectRays    : 1;
        unsigned int m_visibleToReflectionRays  : 1;
        unsigned int m_visibleToRefractionRays  : 1;
        unsigned int m_castShadows              : 1;
        unsigned int m_padding                  : 24;
    } m_flags;

    ShaderType                          m_type;
    double                              m_shadingRate;
    double                              m_directIllumMultiplier;
    double                              m_indirectIllumMultiplier;
    double                              m_indirectIllumSaturation;
    double                              m_opacity;
    IlluminationType                    m_indirectIllumType;
    BlendMode                           m_blend;
    ShaderEffect                        m_effect;

    MaterialShader ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
// __PUBLISH_SECTION_END__
    // Internal methods since V8 dgns serialized m_flags as a single UInt32.
    uint32_t const* GetFlagsCP () const;
    void SetFlags (uint32_t flags);
// __PUBLISH_SECTION_START__

    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialShaderCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialShaderCR rhs);

    DGNPLATFORM_EXPORT bool IsEnabled () const;
    DGNPLATFORM_EXPORT void SetIsEnabled (bool isEnabled);

    DGNPLATFORM_EXPORT ShaderType GetType () const;
    DGNPLATFORM_EXPORT void SetType (ShaderType type);

    DGNPLATFORM_EXPORT bool IsInverted () const;
    DGNPLATFORM_EXPORT void SetIsInverted (bool isInverted);

    DGNPLATFORM_EXPORT bool ReceivesShadows () const;
    DGNPLATFORM_EXPORT void SetReceivesShadows (bool receivesShadows);

    DGNPLATFORM_EXPORT bool CastsShadows () const;
    DGNPLATFORM_EXPORT void SetCastsShadows (bool castsShadows);

    DGNPLATFORM_EXPORT bool IsVisibleToCamera () const;
    DGNPLATFORM_EXPORT void SetIsVisibleToCamera (bool isVisible);

    DGNPLATFORM_EXPORT bool IsVisibleToIndirectRays () const;
    DGNPLATFORM_EXPORT void SetIsVisibleToIndirectRays (bool isVisible);

    DGNPLATFORM_EXPORT bool IsVisibleToReflectionRays () const;
    DGNPLATFORM_EXPORT void SetIsVisibleToReflectionRays (bool isVisible);

    DGNPLATFORM_EXPORT bool IsVisibleToRefractionRays () const;
    DGNPLATFORM_EXPORT void SetIsVisibleToRefractionRays (bool isVisible);

    DGNPLATFORM_EXPORT double GetShadingRate () const;
    DGNPLATFORM_EXPORT void SetShadingRate (double shadingRate);

    DGNPLATFORM_EXPORT double GetDirectIlluminationMultiplier () const;
    DGNPLATFORM_EXPORT void SetDirectIlluminationMultiplier (double multiplier);

    DGNPLATFORM_EXPORT double GetIndirectIlluminationMultiplier () const;
    DGNPLATFORM_EXPORT void SetIndirectIlluminationMultiplier (double multiplier);

    DGNPLATFORM_EXPORT double GetIndirectIlluminationSaturation () const;
    DGNPLATFORM_EXPORT void SetIndirectIlluminationSaturation (double saturation);

    DGNPLATFORM_EXPORT double GetOpacity () const;
    DGNPLATFORM_EXPORT void SetOpacity (double opacity);

    DGNPLATFORM_EXPORT IlluminationType GetIndirectIlluminationType () const;
    DGNPLATFORM_EXPORT void SetIndirectIlluminationType (IlluminationType type);

    DGNPLATFORM_EXPORT BlendMode GetBlendMode () const;
    DGNPLATFORM_EXPORT void SetBlendMode (BlendMode blendMode);

    DGNPLATFORM_EXPORT ShaderEffect GetEffect () const;
    DGNPLATFORM_EXPORT void SetEffect (ShaderEffect effect);
};

typedef RefCountedPtr <MaterialShader>                  MaterialShaderPtr;
// __PUBLISH_SECTION_END__
typedef std::list<MaterialShaderPtr>  MaterialShaderPtrList; // WIP_MATERIALS -- DON'T USE STD::LIST IN AN API
// __PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass                                                    MattGooding     06/10
//=======================================================================================
struct MaterialShaderIterator
    {
// __PUBLISH_SECTION_END__
    friend struct MaterialShaderCollection;
// __PUBLISH_SECTION_START__
    private:
        void*   m_iter;

        MaterialShaderIterator (MaterialShaderCollectionR shaders, bool wantBegin);

    public:
        DGNPLATFORM_EXPORT ~MaterialShaderIterator ();
        DGNPLATFORM_EXPORT MaterialShaderIterator (MaterialShaderIterator const&);

        DGNPLATFORM_EXPORT MaterialShaderIteratorR operator++ ();
        DGNPLATFORM_EXPORT MaterialShaderIteratorR operator-- ();
        DGNPLATFORM_EXPORT MaterialShaderR operator* ();
        DGNPLATFORM_EXPORT MaterialShaderP operator-> ();
        DGNPLATFORM_EXPORT bool operator== (MaterialShaderIteratorCR rhs) const;
        DGNPLATFORM_EXPORT bool operator!= (MaterialShaderIteratorCR rhs) const;
// __PUBLISH_SECTION_END__
        MaterialShaderPtrList::iterator Get ();
// __PUBLISH_SECTION_START__
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     06/10
//=======================================================================================
struct MaterialShaderConstIterator
    {
// WIP_NONPORT - redo this using VirtualCollectionIterator

// __PUBLISH_SECTION_END__
    friend struct MaterialShaderCollection;
// __PUBLISH_SECTION_START__
    private:
        void*   m_iter;

        MaterialShaderConstIterator (MaterialShaderCollectionCR shaders, bool wantBegin);

    public:
        DGNPLATFORM_EXPORT ~MaterialShaderConstIterator ();
        DGNPLATFORM_EXPORT MaterialShaderConstIterator (MaterialShaderConstIterator const&);

        DGNPLATFORM_EXPORT MaterialShaderConstIterator& operator++ ();
        DGNPLATFORM_EXPORT MaterialShaderConstIterator& operator-- ();
        DGNPLATFORM_EXPORT MaterialShaderCR operator* () const;
        DGNPLATFORM_EXPORT MaterialShaderCP operator-> () const;
        DGNPLATFORM_EXPORT bool operator== (MaterialShaderConstIterator const& rhs) const;
        DGNPLATFORM_EXPORT bool operator!= (MaterialShaderConstIterator const& rhs) const;
// __PUBLISH_SECTION_END__
        MaterialShaderPtrList::const_iterator Get () const;
// __PUBLISH_SECTION_START__
    };

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialShaderCollection : public NonCopyableClass
{
// WIP_NONPORT - redo this using VirtualCollectionIterator
// __PUBLISH_SECTION_END__
    friend struct MaterialSettings;
    friend struct MaterialShaderIterator;
    friend struct MaterialShaderConstIterator;
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    MaterialShaderPtrList                                       m_shaders;

    MaterialShaderCollection ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (MaterialShaderCollectionCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialShaderCollectionCR rhs);

    DGNPLATFORM_EXPORT MaterialShaderConstIterator begin () const;
    DGNPLATFORM_EXPORT MaterialShaderIterator begin ();
    DGNPLATFORM_EXPORT MaterialShaderConstIterator end () const;
    DGNPLATFORM_EXPORT MaterialShaderIterator end ();

    DGNPLATFORM_EXPORT size_t Size () const;
    DGNPLATFORM_EXPORT MaterialShaderCR GetTopShader () const;
    DGNPLATFORM_EXPORT MaterialShaderR GetTopShaderR ();
    DGNPLATFORM_EXPORT MaterialShaderR AddShader ();

    //! Deleting the last shader will cause a default shader to be added - materials must always have at least one shader.
    DGNPLATFORM_EXPORT BentleyStatus DeleteShader (MaterialShaderIterator iter);
};

// __PUBLISH_SECTION_END__

typedef RefCountedPtr <MaterialFur> MaterialFurPtr;
// __PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass                                                    MattGooding     01/10
//=======================================================================================
struct MaterialSettings : public NonCopyableClass
{
// __PUBLISH_SECTION_END__
    friend struct Material;

#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

    //=======================================================================================
    // @bsiclass                                                    PaulChater     08/11
    //=======================================================================================
    enum BackFaceCulling
        {
        BACKFACECULLING_UseGeometryDefault          = 0,
        BACKFACECULLING_ForceSingleSided            = 1,
        BACKFACECULLING_ForceDoubleSided            = 2,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__

    // This structure (m_flags) is serialized in V8 DGN files.
    struct
        {
        unsigned int m_participatesInSpotlib            : 1; // Participates in radiosity/particle trace solutions (legacy variable)
        unsigned int m_noShadows                        : 1; // Does not cast shadows

        unsigned int m_hasBaseColor                     : 1;
        unsigned int m_hasSpecularColor                 : 1;
        unsigned int m_hasFinish                        : 1;
        unsigned int m_hasReflect                       : 1;
        unsigned int m_hasTransmit                      : 1;
        unsigned int m_hasDiffuse                       : 1;
        unsigned int m_hasRefract                       : 1;
        unsigned int m_hasSpecular                      : 1;

        unsigned int m_lockSpecularAndReflect           : 1; // Reflect value is same as specular
        unsigned int m_lockEfficiency                   : 1; // Restrict other properties to maintain efficiency value
        unsigned int m_lockSpecularAndBase              : 1; // Specular value is same as base color
        unsigned int m_lockFinishToSpecular             : 1; // Finish is kept in synch with specular
        unsigned int m_customSpecular                   : 1; // Indicates whether a custom specular or a preset has been specified.

        unsigned int m_linkedToLxp                      : 1; // Material linked to external lxp file
        unsigned int m_invisible                        : 1; // Not visible to eye/camera
        unsigned int m_hasTransmitColor                 : 1; // Transmit color is specified
        unsigned int m_hasTranslucencyColor             : 1; // Translucency color is specified

        unsigned int m_lockFresnelToReflect             : 1;
        unsigned int m_lockRefractionRoughnessToFinish  : 1;
        unsigned int m_hasGlowColor                     : 1;
        unsigned int m_hasReflectColor                  : 1;
        unsigned int m_hasExitColor                     : 1;
        unsigned int m_padding                          : 8;
        } m_flags;

    RgbFactor                                           m_baseColor;
    RgbFactor                                           m_specularColor;
    RgbFactor                                           m_transmitColor;
    RgbFactor                                           m_translucencyColor;
    RgbFactor                                           m_glowColor;
    RgbFactor                                           m_reflectColor;
    RgbFactor                                           m_exitColor;
    double                                              m_ambient;
    double                                              m_finish;
    double                                              m_reflect;
    double                                              m_transmit;
    double                                              m_diffuse;
    double                                              m_specular;
    double                                              m_refract;
    double                                              m_thickness;
    double                                              m_glow;
    double                                              m_translucency;
    int32_t                                             m_reflectionRays;
    int32_t                                             m_refractionRays;
    int32_t                                             m_subsurfaceSamples;
    double                                              m_displacementDistance;
    double                                              m_dispersion;
    double                                              m_clearcoat;
    double                                              m_anisotropy;
    double                                              m_frontWeighting;
    double                                              m_scatterDistance;
    double                                              m_reflectFresnel;
    double                                              m_dissolve;
    double                                              m_absorptionDistance;
    double                                              m_refractionRoughness;

    bool                                                m_blurReflections;
    bool                                                m_blurRefractions;
    bool                                                m_useFur;
    bool                                                m_useCutSectionMaterial;
    BackFaceCulling                                     m_backFaceCulling;

    MaterialShaderCollection                            m_shaders;
    MaterialFurPtr                                      m_fur;
    MaterialMapCollection                               m_maps;
    bvector<Byte>                                       m_preset;

    WString                                             m_cutSectionMaterialName;
    WString                                             m_cutSectionPalette;

    MaterialSettings ();
    ~MaterialSettings ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
// __PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT bool IsLinkedToLxp () const;
    DGNPLATFORM_EXPORT void SetIsLinkedToLxp (bool isLinkedToLxp);

    // Legacy setting, provided only to maintain V8 file compatibility.
    bool ParticipatesInSpotlib () const;
    void SetParticipatesInSpotlib (bool participatesInSpotlibRenderings);

    // Legacy setting, provided only to maintain V8 file compatibility.
    double GetThickness () const;
    void SetThickness (double thickness);

    // Internal methods since V8 dgns serialized m_flags as a single UInt32.
    uint32_t const* GetFlagsCP () const;
    void SetFlags (uint32_t flags);

    void UpgradeFromVersion (MaterialVersion version);

// __PUBLISH_SECTION_START__
    DGNPLATFORM_EXPORT void InitDefaults ();

    DGNPLATFORM_EXPORT bool Equals (MaterialSettingsCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (MaterialSettingsCR rhs);

    DGNPLATFORM_EXPORT MaterialShaderCollectionCR GetShaders () const;
    DGNPLATFORM_EXPORT MaterialShaderCollectionR GetShadersR ();

    //! If fur does not already exist for this material, it will be added.
    DGNPLATFORM_EXPORT MaterialFurCR GetFur ();

    //! If fur does not already exist for this material, it will be added.
    DGNPLATFORM_EXPORT MaterialFurR GetFurR ();

    DGNPLATFORM_EXPORT MaterialFurCP GetFurCP () const;
    DGNPLATFORM_EXPORT MaterialFurP GetFurP ();

    //! If this material already has fur, it will be replaced.
    DGNPLATFORM_EXPORT MaterialFurR AddFur ();
    DGNPLATFORM_EXPORT void DeleteFur ();

    DGNPLATFORM_EXPORT bool UseFur () const;
    DGNPLATFORM_EXPORT void SetUseFur (bool useFur);

    DGNPLATFORM_EXPORT MaterialMapCollectionCR GetMaps () const;
    DGNPLATFORM_EXPORT MaterialMapCollectionR GetMapsR ();

    DGNPLATFORM_EXPORT bool HasBaseColor () const;
    DGNPLATFORM_EXPORT void SetHasBaseColor (bool hasBaseColor);

    DGNPLATFORM_EXPORT bool HasSpecularColor () const;
    DGNPLATFORM_EXPORT void SetHasSpecularColor (bool hasSpecularColor);

    DGNPLATFORM_EXPORT bool HasTransmitColor () const;
    DGNPLATFORM_EXPORT void SetHasTransmitColor (bool hasTransmitColor);

    DGNPLATFORM_EXPORT bool HasTranslucencyColor () const;
    DGNPLATFORM_EXPORT void SetHasTranslucencyColor (bool hasTranslucencyColor);

    DGNPLATFORM_EXPORT bool HasGlowColor () const;
    DGNPLATFORM_EXPORT void SetHasGlowColor (bool hasGlowColor);

    DGNPLATFORM_EXPORT bool HasReflectColor () const;
    DGNPLATFORM_EXPORT void SetHasReflectColor (bool hasReflectColor);

    DGNPLATFORM_EXPORT bool HasExitColor () const;
    DGNPLATFORM_EXPORT void SetHasExitColor (bool hasExitColor);

    DGNPLATFORM_EXPORT bool HasDiffuseIntensity () const;
    DGNPLATFORM_EXPORT void SetHasDiffuseIntensity (bool hasDiffuseIntensity);

    DGNPLATFORM_EXPORT bool HasSpecularIntensity () const;
    DGNPLATFORM_EXPORT void SetHasSpecularIntensity (bool hasSpecularIntensity);

    DGNPLATFORM_EXPORT bool HasFinishScale () const;
    DGNPLATFORM_EXPORT void SetHasFinishScale (bool hasFinishScale);

    DGNPLATFORM_EXPORT bool HasRefractIndex () const;
    DGNPLATFORM_EXPORT void SetHasRefractIndex (bool hasRefractIndex);

    DGNPLATFORM_EXPORT bool HasReflectIntensity () const;
    DGNPLATFORM_EXPORT void SetHasReflectIntensity (bool hasReflectIntensity);

    DGNPLATFORM_EXPORT bool HasTransmitIntensity () const;
    DGNPLATFORM_EXPORT void SetHasTransmitIntensity (bool hasTransmitIntensity);

    DGNPLATFORM_EXPORT RgbFactor const& GetBaseColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetBaseColorR ();
    DGNPLATFORM_EXPORT void SetBaseColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetSpecularColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetSpecularColorR ();
    DGNPLATFORM_EXPORT void SetSpecularColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetTransmitColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetTransmitColorR ();
    DGNPLATFORM_EXPORT void SetTransmitColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetTranslucencyColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetTranslucencyColorR ();
    DGNPLATFORM_EXPORT void SetTranslucencyColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetGlowColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetGlowColorR ();
    DGNPLATFORM_EXPORT void SetGlowColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetReflectColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetReflectColorR ();
    DGNPLATFORM_EXPORT void SetReflectColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetExitColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetExitColorR ();
    DGNPLATFORM_EXPORT void SetExitColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetAmbientIntensity () const;
    DGNPLATFORM_EXPORT void SetAmbientIntensity (double intensity);
    // __PUBLISH_SECTION_END__
    // This is the value that is stored to files... in V8, this value was "finish" on MaterialProperties and convertFinishToMatedit
    // was used to receive the displayed value.  GetFinishScale and SetFinishScale always work with the displayed value.
    DGNPLATFORM_EXPORT double GetFinishScaleForStorage () const;
    DGNPLATFORM_EXPORT void SetFinishScaleFromStorage (double scale);
    // __PUBLISH_SECTION_START__

    DGNPLATFORM_EXPORT double GetFinishScale () const;
    DGNPLATFORM_EXPORT void SetFinishScale (double scale);

    DGNPLATFORM_EXPORT double GetReflectIntensity () const;
    DGNPLATFORM_EXPORT void SetReflectIntensity (double intensity);

    DGNPLATFORM_EXPORT double GetTransmitIntensity () const;
    DGNPLATFORM_EXPORT void SetTransmitIntensity (double intensity);

    DGNPLATFORM_EXPORT double GetDiffuseIntensity () const;
    DGNPLATFORM_EXPORT void SetDiffuseIntensity (double intensity);

    DGNPLATFORM_EXPORT double GetSpecularIntensity () const;
    DGNPLATFORM_EXPORT void SetSpecularIntensity (double intensity);

    DGNPLATFORM_EXPORT double GetRefractIndex () const;
    DGNPLATFORM_EXPORT void SetRefractIndex (double index);

    //! Returns the amount of light emitted by this material in lumens/meterSquared
    DGNPLATFORM_EXPORT double GetGlowIntensity () const;
    DGNPLATFORM_EXPORT void SetGlowIntensity (double intensity);

    DGNPLATFORM_EXPORT double GetTranslucencyScale () const;
    DGNPLATFORM_EXPORT void SetTranslucencyScale (double scale);

    DGNPLATFORM_EXPORT bool CastsShadows () const;
    DGNPLATFORM_EXPORT void SetCastsShadows (bool castsShadows);

    DGNPLATFORM_EXPORT bool IsVisible () const;
    DGNPLATFORM_EXPORT void SetIsVisible (bool isVisible);

    DGNPLATFORM_EXPORT int32_t GetReflectionRays () const;
    DGNPLATFORM_EXPORT void SetReflectionRays (int32_t rays);

    DGNPLATFORM_EXPORT int32_t GetRefractionRays () const;
    DGNPLATFORM_EXPORT void SetRefractionRays (int32_t rays);

    DGNPLATFORM_EXPORT int32_t GetSubsurfaceSamples () const;
    DGNPLATFORM_EXPORT void SetSubsurfaceSamples (int32_t samples);

    DGNPLATFORM_EXPORT double GetDisplacementDistanceInMillimeters () const;
    DGNPLATFORM_EXPORT void SetDisplacementDistanceInMillimeters (double distance);

    DGNPLATFORM_EXPORT double GetDispersionAmount () const;
    DGNPLATFORM_EXPORT void SetDispersionAmount (double dispersion);

    DGNPLATFORM_EXPORT double GetClearcoatAmount () const;
    DGNPLATFORM_EXPORT void SetClearcoatAmount (double clearcoat);

    DGNPLATFORM_EXPORT double GetAnisotropyAmount () const;
    DGNPLATFORM_EXPORT void SetAnisotropyAmount (double anisotropy);

    DGNPLATFORM_EXPORT double GetFrontWeightingAmount () const;
    DGNPLATFORM_EXPORT void SetFrontWeightingAmount (double frontWeighting);

    DGNPLATFORM_EXPORT double GetScatterDistanceInMeters () const;
    DGNPLATFORM_EXPORT void SetScatterDistanceInMeters (double scatterDistance);

    DGNPLATFORM_EXPORT double GetReflectionFresnel () const;
    DGNPLATFORM_EXPORT void SetReflectionFresnel (double fresnel);

    DGNPLATFORM_EXPORT double GetDissolveAmount () const;
    DGNPLATFORM_EXPORT void SetDissolveAmount (double dissolve);

    DGNPLATFORM_EXPORT double GetAbsorptionDistance () const;
    DGNPLATFORM_EXPORT void SetAbsorptionDistance (double absorption);

    DGNPLATFORM_EXPORT double GetRefractionRoughness () const;
    DGNPLATFORM_EXPORT void SetRefractionRoughness (double roughness);

    DGNPLATFORM_EXPORT bool BlurReflections () const;
    DGNPLATFORM_EXPORT void SetBlurReflections (bool blurReflections);

    DGNPLATFORM_EXPORT bool BlurRefractions () const;
    DGNPLATFORM_EXPORT void SetBlurRefractions (bool blurRefractions);

    DGNPLATFORM_EXPORT BackFaceCulling GetBackFaceCulling () const;
    DGNPLATFORM_EXPORT void SetBackFaceCulling (BackFaceCulling backFaceCulling);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool LockSpecularAndReflect () const;
    DGNPLATFORM_EXPORT void SetLockSpecularAndReflect (bool lock);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool LockEfficiency () const;
    DGNPLATFORM_EXPORT void SetLockEfficiency (bool lock);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool LockSpecularAndBase () const;
    DGNPLATFORM_EXPORT void SetLockSpecularAndBase (bool lock);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool LockFinishAndSpecular () const;
    DGNPLATFORM_EXPORT void SetLockFinishAndSpecular (bool lock);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool LockFresnelToReflect () const;
    DGNPLATFORM_EXPORT void SetLockFresnelToReflect (bool lock);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool LockRefractionRoughnessToFinish () const;
    DGNPLATFORM_EXPORT void SetLockRefractionRoughnessToFinish (bool lock);

    //! This is a flag used by the GUI of MicroStation and its derivatives.  Its effect is not maintained when programmatically setting values.
    DGNPLATFORM_EXPORT bool HasCustomSpecular () const;
    DGNPLATFORM_EXPORT void SetHasCustomSpecular (bool customSpecular);

    DGNPLATFORM_EXPORT WStringCR GetCutSectionMaterialName () const;
    //! The maximum length for a material name is 30 characters - if the provided name exceeds that length, this function will return ERROR and the name will not be changed.
    DGNPLATFORM_EXPORT void SetCutSectionMaterialName (WCharCP name);

    DGNPLATFORM_EXPORT WStringCR GetCutSectionMaterialPalette () const;
    DGNPLATFORM_EXPORT WStringR GetCutSectionMaterialPaletteR ();

    DGNPLATFORM_EXPORT bool UseCutSectionMaterial () const;
    DGNPLATFORM_EXPORT void SetUseCutSectionMaterial (bool useCutSectionMaterial);

    // __PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT const Byte* GetPresetDataCP () const;

    // If preset data already exists for this material, it will be deleted.
    DGNPLATFORM_EXPORT void AddPresetData (Byte const* data, size_t size);
    // __PUBLISH_SECTION_START__

    //! This is binary data used internally by MicroStation and its derivatives to work with presets from other material systems.
    DGNPLATFORM_EXPORT size_t GetPresetDataSize () const;
    DGNPLATFORM_EXPORT void DeletePresetData ();
};

END_BENTLEY_DGN_NAMESPACE

/** @endcond */

// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
