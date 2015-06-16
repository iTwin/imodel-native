/*----------------------------------------------------------------------+
|
|   $Source: DgnCore/MaterialSettings.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <RmgrTools/Tools/typecode.h>

static const double s_minimumFinish     = 0.4;
static const double s_finishExponent    = 0.016;
static const double s_finishFactor      = 2.59;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool valuesEqual (double val1, double val2) { return fabs (val1 - val2) < 1.0E-4; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool colorsEqual (RgbFactor const& val1, RgbFactor const& val2)
    {
    return valuesEqual (val1.red, val2.red) && valuesEqual (val1.green, val2.green) && valuesEqual (val1.blue, val2.blue);
    }

#define RETURN_IF_FALSE(val) if (!(val)) { return false; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedureCexprMember::LxoProcedureCexprMember (MdlTypecodes typecode, const char * name, uintptr_t member)
    {
    m_member = member;
    m_name = name;
    m_typecode = typecode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialSettings::MaterialSettings ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialSettings::~MaterialSettings ()
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::UpgradeFromVersion (MaterialVersion version)
    {
    // For versions before the glow color map etc. - when rendering luxology we automatically output
    // glow, reflect, specular etc. color maps equal to the pattern map. In order to maintain this, we
    // need to link the glow color map etc. to the pattern map.
    if (version <= MATERIALVERSION_BeforeGlowMap)
        {
        GetTransmitColorR ()        = GetSpecularColor ();
        GetTranslucencyColorR ()    = GetBaseColor ();

        MaterialMapCP pattern = GetMaps ().GetMapCP (MaterialMap::MAPTYPE_Pattern);
        if (NULL != pattern && pattern->IsEnabled ())
            {
            MaterialMapP translucency = GetMapsR ().AddMap (MaterialMap::MAPTYPE_TranslucencyColor);
            translucency->SetLinkType (MaterialMap::MAPTYPE_Pattern);
            translucency->SetIsEnabled (true);

            if (LockSpecularAndBase ())
                {
                MaterialMapP specular = GetMapsR ().AddMap (MaterialMap::MAPTYPE_SpecularColor);
                specular->SetLinkType (MaterialMap::MAPTYPE_Pattern);
                specular->SetIsEnabled (true);

                MaterialMapP transparent = GetMapsR ().AddMap (MaterialMap::MAPTYPE_TransparentColor);
                transparent->SetLinkType (MaterialMap::MAPTYPE_SpecularColor);
                transparent->SetIsEnabled (true);
                }
            }
        }

    if (version < MATERIALVERSION_SplitBumpDisplacementMap && 0.0 != GetDisplacementDistanceInMillimeters ())
        {
        MaterialMapP bump = GetMapsR ().GetMapP (MaterialMap::MAPTYPE_Bump);
        if (NULL != bump)
            {
            MaterialMapP displacement = GetMapsR ().AddMap (MaterialMap::MAPTYPE_Displacement);
            displacement->Copy (*bump);
            GetMapsR ().DeleteMap (MaterialMap::MAPTYPE_Bump);
            }
        }

    if (version < MATERIALVERSION_ReflectFresnel)
        {
        SetLockFresnelToReflect (true);
        SetLockRefractionRoughnessToFinish (true);
        SetHasGlowColor (false);

        GetGlowColorR () = GetBaseColorR ();
        SetRefractionRoughness (GetFinishScale ());
        SetReflectionFresnel (GetSpecularIntensity ());
        }

    if (version < MATERIALVERSION_ReflectColor)
        {
        MaterialMapCP   specularColor   = GetMaps ().GetMapCP (MaterialMap::MAPTYPE_SpecularColor);

        if (specularColor)
            {
            MaterialMapP    reflect         = GetMapsR ().AddMap (MaterialMap::MAPTYPE_Reflect);
            reflect->SetLinkType (MaterialMap::MAPTYPE_SpecularColor);
            reflect->SetIsEnabled (NULL != specularColor && specularColor->IsEnabled ());
            }
        GetReflectColorR () = GetSpecularColor ();

        MaterialMapCP   pattern         = GetMaps ().GetMapCP (MaterialMap::MAPTYPE_Pattern);

        if (pattern)
            {
            MaterialMapP    glow            = GetMapsR ().AddMap (MaterialMap::MAPTYPE_GlowColor);
            glow->SetLinkType (MaterialMap::MAPTYPE_Pattern);
            glow->SetIsEnabled (NULL != pattern && pattern->IsEnabled ());
            }
        SetLockSpecularAndReflect (false);
        }

    if (version < MATERIALVERSION_SplitFresnelFromSpecular)
        SetReflectionFresnel (GetReflectIntensity ());

    if (version < MATERIALVERSION_Antialiasing)
        {
        for (MaterialMapCollection::iterator it = GetMapsR ().begin (); it != GetMapsR ().end (); ++it)
            for (MaterialMapLayerIterator layerIt = it->second->GetLayersR ().begin (); layerIt != it->second->GetLayersR ().end (); ++layerIt)
                layerIt->SetIsAntialiasing (true);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::InitDefaults ()
    {
    memset (&m_flags, 0, sizeof (m_flags));
    m_flags.m_lockFinishToSpecular              = true;
    m_flags.m_lockFresnelToReflect              = true;
    m_flags.m_lockRefractionRoughnessToFinish   = true;
    m_flags.m_linkedToLxp                       = false;
    m_flags.m_participatesInSpotlib             = true;

    m_baseColor.red                             = m_baseColor.green = m_baseColor.blue = 1.0;
    m_specularColor                             = m_baseColor;
    m_transmitColor                             = m_baseColor;
    m_translucencyColor                         = m_baseColor;
    m_glowColor                                 = m_baseColor;
    m_reflectColor                              = m_baseColor;
    m_exitColor.red                             = m_exitColor.green = m_exitColor.blue = 0.0;

    m_ambient                                   = 0.5;
    m_finish                                    = 0.05;
    m_reflect                                   = 0.0;
    m_transmit                                  = 0.0;
    m_diffuse                                   = 0.5;
    m_specular                                  = 0.05;
    m_refract                                   = 1.0;
    m_thickness                                 = 0.0;
    m_glow                                      = 0.0;
    m_translucency                              = 0.0;
    m_reflectionRays                            = 64;
    m_refractionRays                            = 64;
    m_subsurfaceSamples                         = 64;
    m_displacementDistance                      = 0.0;
    m_dispersion                                = 0.0;
    m_clearcoat                                 = 0.0;
    m_anisotropy                                = 0.0;
    m_frontWeighting                            = 50.0;
    m_scatterDistance                           = 0.0;
    m_absorptionDistance                        = 0.0;
    m_dissolve                                  = 0.0;
    m_reflectFresnel                            = m_specular;
    m_refractionRoughness                       = m_finish;

    m_blurReflections                           = false;
    m_blurRefractions                           = false;
    m_useFur                                    = false;
    m_useCutSectionMaterial                     = false;
    m_backFaceCulling                           = BACKFACECULLING_UseGeometryDefault;

    m_shaders.InitDefaults ();
    m_maps.InitDefaults ();
    m_fur = NULL;
    m_preset.clear ();
    m_cutSectionMaterialName                    = L"";
    m_cutSectionPalette                         = L"";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialSettings::Equals (MaterialSettingsCR rhs) const
    {
    RETURN_IF_FALSE                 (rhs.m_flags.m_participatesInSpotlib            ==  m_flags.m_participatesInSpotlib);
    RETURN_IF_FALSE                 (rhs.m_flags.m_noShadows                        ==  m_flags.m_noShadows);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasBaseColor                     ==  m_flags.m_hasBaseColor);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasSpecularColor                 ==  m_flags.m_hasSpecularColor);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasFinish                        ==  m_flags.m_hasFinish);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasReflect                       ==  m_flags.m_hasReflect);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasTransmit                      ==  m_flags.m_hasTransmit);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasDiffuse                       ==  m_flags.m_hasDiffuse);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasRefract                       ==  m_flags.m_hasRefract);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasSpecular                      ==  m_flags.m_hasSpecular);
    RETURN_IF_FALSE                 (rhs.m_flags.m_lockSpecularAndReflect           ==  m_flags.m_lockSpecularAndReflect);
    RETURN_IF_FALSE                 (rhs.m_flags.m_lockEfficiency                   ==  m_flags.m_lockEfficiency);
    RETURN_IF_FALSE                 (rhs.m_flags.m_lockSpecularAndBase              ==  m_flags.m_lockSpecularAndBase);
    RETURN_IF_FALSE                 (rhs.m_flags.m_lockFinishToSpecular             ==  m_flags.m_lockFinishToSpecular);
    RETURN_IF_FALSE                 (rhs.m_flags.m_lockFresnelToReflect             ==  m_flags.m_lockFresnelToReflect);
    RETURN_IF_FALSE                 (rhs.m_flags.m_lockRefractionRoughnessToFinish  ==  m_flags.m_lockRefractionRoughnessToFinish);
    RETURN_IF_FALSE                 (rhs.m_flags.m_customSpecular                   ==  m_flags.m_customSpecular);
    RETURN_IF_FALSE                 (rhs.m_flags.m_linkedToLxp                      ==  m_flags.m_linkedToLxp);
    RETURN_IF_FALSE                 (rhs.m_flags.m_invisible                        ==  m_flags.m_invisible);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasTransmitColor                 ==  m_flags.m_hasTransmitColor);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasTranslucencyColor             ==  m_flags.m_hasTranslucencyColor);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasGlowColor                     ==  m_flags.m_hasGlowColor);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasReflectColor                  ==  m_flags.m_hasReflectColor);
    RETURN_IF_FALSE                 (rhs.m_flags.m_hasExitColor                     ==  m_flags.m_hasExitColor);
    RETURN_IF_FALSE                 (rhs.m_reflectionRays                           ==  m_reflectionRays);
    RETURN_IF_FALSE                 (rhs.m_refractionRays                           ==  m_refractionRays);
    RETURN_IF_FALSE                 (rhs.m_subsurfaceSamples                        ==  m_subsurfaceSamples);
    RETURN_IF_FALSE                 (rhs.m_blurReflections                          ==  m_blurReflections);
    RETURN_IF_FALSE                 (rhs.m_blurRefractions                          ==  m_blurRefractions);
    RETURN_IF_FALSE                 (rhs.m_useFur                                   ==  m_useFur);
    RETURN_IF_FALSE                 (rhs.m_useCutSectionMaterial                    ==  m_useCutSectionMaterial);
    RETURN_IF_FALSE                 (rhs.m_backFaceCulling                          ==  m_backFaceCulling);
    RETURN_IF_FALSE (colorsEqual    (rhs.m_baseColor,                                   m_baseColor));
    RETURN_IF_FALSE (colorsEqual    (rhs.m_specularColor,                               m_specularColor));
    RETURN_IF_FALSE (colorsEqual    (rhs.m_transmitColor,                               m_transmitColor));
    RETURN_IF_FALSE (colorsEqual    (rhs.m_translucencyColor,                           m_translucencyColor));
    RETURN_IF_FALSE (colorsEqual    (rhs.m_glowColor,                                   m_glowColor));
    RETURN_IF_FALSE (colorsEqual    (rhs.m_reflectColor,                                m_reflectColor));
    RETURN_IF_FALSE (colorsEqual    (rhs.m_exitColor,                                   m_exitColor));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_ambient,                                     m_ambient));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_finish,                                      m_finish));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_reflect,                                     m_reflect));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_transmit,                                    m_transmit));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_diffuse,                                     m_diffuse));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_specular,                                    m_specular));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_refract,                                     m_refract));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_thickness,                                   m_thickness));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_glow,                                        m_glow));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_translucency,                                m_translucency));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_displacementDistance,                        m_displacementDistance));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_dispersion,                                  m_dispersion));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_clearcoat,                                   m_clearcoat));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_anisotropy,                                  m_anisotropy));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_frontWeighting,                              m_frontWeighting));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_scatterDistance,                             m_scatterDistance));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_reflectFresnel,                              m_reflectFresnel));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_dissolve,                                    m_dissolve));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_absorptionDistance,                          m_absorptionDistance));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_refractionRoughness,                         m_refractionRoughness));
    RETURN_IF_FALSE                 (rhs.m_shaders.Equals                               (m_shaders));

    if (m_fur.IsNull ())
        {
        RETURN_IF_FALSE (rhs.m_fur.IsNull ());
        }
    else
        {
        if (rhs.m_fur.IsNull ())
            return false;

        RETURN_IF_FALSE (rhs.m_fur->Equals (*m_fur.get ()));
        }

    RETURN_IF_FALSE (rhs.m_maps.Equals (m_maps));
    RETURN_IF_FALSE (rhs.m_preset == m_preset);
    RETURN_IF_FALSE (rhs.m_cutSectionMaterialName.EqualsI (m_cutSectionMaterialName.c_str()));
    RETURN_IF_FALSE (rhs.m_cutSectionPalette.EqualsI (m_cutSectionPalette.c_str()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::Copy (MaterialSettingsCR rhs)
    {
    m_shaders.Copy (rhs.m_shaders);
    m_maps.Copy (rhs.m_maps);

    if (NULL != rhs.GetFurCP ())
        AddFur ().Copy (*rhs.GetFurCP ());

    AddPresetData (rhs.GetPresetDataCP (), rhs.GetPresetDataSize ());

    m_flags                 = rhs.m_flags;
    m_baseColor             = rhs.m_baseColor;
    m_specularColor         = rhs.m_specularColor;
    m_transmitColor         = rhs.m_transmitColor;
    m_translucencyColor     = rhs.m_translucencyColor;
    m_glowColor             = rhs.m_glowColor;
    m_reflectColor          = rhs.m_reflectColor;
    m_exitColor             = rhs.m_exitColor;
    m_ambient               = rhs.m_ambient;
    m_finish                = rhs.m_finish;
    m_reflect               = rhs.m_reflect;
    m_transmit              = rhs.m_transmit;
    m_diffuse               = rhs.m_diffuse;
    m_specular              = rhs.m_specular;
    m_refract               = rhs.m_refract;
    m_thickness             = rhs.m_thickness;
    m_glow                  = rhs.m_glow;
    m_translucency          = rhs.m_translucency;
    m_reflectionRays        = rhs.m_reflectionRays;
    m_refractionRays        = rhs.m_refractionRays;
    m_subsurfaceSamples     = rhs.m_subsurfaceSamples;
    m_displacementDistance  = rhs.m_displacementDistance;
    m_dispersion            = rhs.m_dispersion;
    m_clearcoat             = rhs.m_clearcoat;
    m_anisotropy            = rhs.m_anisotropy;
    m_frontWeighting        = rhs.m_frontWeighting;
    m_scatterDistance       = rhs.m_scatterDistance;
    m_reflectFresnel        = rhs.m_reflectFresnel;
    m_dissolve              = rhs.m_dissolve;
    m_absorptionDistance    = rhs.m_absorptionDistance;
    m_refractionRoughness   = rhs.m_refractionRoughness;
    m_blurReflections       = rhs.m_blurReflections;
    m_blurRefractions       = rhs.m_blurRefractions;
    m_useFur                = rhs.m_useFur;
    m_useCutSectionMaterial = rhs.m_useCutSectionMaterial;
    m_backFaceCulling       = rhs.m_backFaceCulling;

    m_cutSectionMaterialName = rhs.m_cutSectionMaterialName;
    m_cutSectionPalette = rhs.m_cutSectionPalette;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialFurR MaterialSettings::AddFur ()
    {
    if (m_fur.IsNull ())
        m_fur = new MaterialFur ();
    else
        m_fur->InitDefaults ();

    return *m_fur.get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::DeleteFur ()
    {
    m_fur = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
const Byte* MaterialSettings::GetPresetDataCP () const
    {
    if (m_preset.empty())
        return NULL;
    return &m_preset[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::AddPresetData (Byte const* data, size_t size)
    {
    m_preset.resize (size, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MaterialSettings::GetPresetDataSize () const
    {
    return m_preset.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::DeletePresetData ()
    {
    m_preset.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        MaterialSettings::IsLinkedToLxp () const                                    { return TO_BOOL (m_flags.m_linkedToLxp); }
void                        MaterialSettings::SetIsLinkedToLxp (bool isLinkedToLxp)                     { m_flags.m_linkedToLxp = isLinkedToLxp; }

bool                        MaterialSettings::ParticipatesInSpotlib () const                            { return TO_BOOL (m_flags.m_participatesInSpotlib); }
void                        MaterialSettings::SetParticipatesInSpotlib (bool participatesInSpotlib)     { m_flags.m_participatesInSpotlib = participatesInSpotlib; }

uint32_t const*               MaterialSettings::GetFlagsCP () const                                       { return (uint32_t const *) &m_flags; }
void                        MaterialSettings::SetFlags (uint32_t flags)                                   { memcpy (&m_flags, &flags, sizeof (flags)); }

MaterialShaderCollectionCR  MaterialSettings::GetShaders () const                                       { return m_shaders; }
MaterialShaderCollectionR   MaterialSettings::GetShadersR ()                                            { return m_shaders; }

MaterialFurCR               MaterialSettings::GetFur ()                                                 { if (m_fur.IsNull ()) { AddFur (); } return *m_fur.get (); }
MaterialFurR                MaterialSettings::GetFurR ()                                                { if (m_fur.IsNull ()) { AddFur (); } return *m_fur.get (); }
MaterialFurCP               MaterialSettings::GetFurCP () const                                         { return m_fur.get (); }
MaterialFurP                MaterialSettings::GetFurP ()                                                { return m_fur.get (); }

MaterialMapCollectionCR     MaterialSettings::GetMaps () const                                          { return m_maps; }
MaterialMapCollectionR      MaterialSettings::GetMapsR ()                                               { return m_maps; }

bool                        MaterialSettings::HasBaseColor () const                                     { return TO_BOOL (m_flags.m_hasBaseColor); }
void                        MaterialSettings::SetHasBaseColor (bool hasBaseColor)                       { m_flags.m_hasBaseColor = hasBaseColor; }

bool                        MaterialSettings::HasSpecularColor () const                                 { return TO_BOOL (m_flags.m_hasSpecularColor); }
void                        MaterialSettings::SetHasSpecularColor (bool hasSpecularColor)               { m_flags.m_hasSpecularColor = hasSpecularColor; }

bool                        MaterialSettings::HasTransmitColor () const                                 { return TO_BOOL (m_flags.m_hasTransmitColor); }
void                        MaterialSettings::SetHasTransmitColor (bool hasTransmitColor)               { m_flags.m_hasTransmitColor = hasTransmitColor; }

bool                        MaterialSettings::HasTranslucencyColor () const                             { return TO_BOOL (m_flags.m_hasTranslucencyColor); }
void                        MaterialSettings::SetHasTranslucencyColor (bool hasTranslucencyColor)       { m_flags.m_hasTranslucencyColor = hasTranslucencyColor; }

bool                        MaterialSettings::HasGlowColor () const                                     { return TO_BOOL (m_flags.m_hasGlowColor); }
void                        MaterialSettings::SetHasGlowColor (bool hasGlowColor)                       { m_flags.m_hasGlowColor = hasGlowColor; }

bool                        MaterialSettings::HasReflectColor () const                                  { return TO_BOOL (m_flags.m_hasReflectColor); }
void                        MaterialSettings::SetHasReflectColor (bool hasReflectColor)                 { m_flags.m_hasReflectColor = hasReflectColor; }

bool                        MaterialSettings::HasExitColor () const                                     { return TO_BOOL (m_flags.m_hasExitColor); }
void                        MaterialSettings::SetHasExitColor (bool hasExitColor)                       { m_flags.m_hasExitColor = hasExitColor; }

bool                        MaterialSettings::HasDiffuseIntensity () const                              { return TO_BOOL (m_flags.m_hasDiffuse); }
void                        MaterialSettings::SetHasDiffuseIntensity (bool hasDiffuse)                  { m_flags.m_hasDiffuse = hasDiffuse; }

bool                        MaterialSettings::HasSpecularIntensity () const                             { return TO_BOOL (m_flags.m_hasSpecular); }
void                        MaterialSettings::SetHasSpecularIntensity (bool hasSpecular)                { m_flags.m_hasSpecular = hasSpecular; }

bool                        MaterialSettings::HasFinishScale () const                                   { return TO_BOOL (m_flags.m_hasFinish); }
void                        MaterialSettings::SetHasFinishScale (bool hasFinish)                        { m_flags.m_hasFinish = hasFinish; }

bool                        MaterialSettings::HasRefractIndex () const                                  { return TO_BOOL (m_flags.m_hasRefract); }
void                        MaterialSettings::SetHasRefractIndex (bool hasRefract)                      { m_flags.m_hasRefract = hasRefract; }

bool                        MaterialSettings::HasReflectIntensity () const                              { return TO_BOOL (m_flags.m_hasReflect); }
void                        MaterialSettings::SetHasReflectIntensity (bool hasReflect)                  { m_flags.m_hasReflect = hasReflect; }

bool                        MaterialSettings::HasTransmitIntensity () const                             { return TO_BOOL (m_flags.m_hasTransmit); }
void                        MaterialSettings::SetHasTransmitIntensity (bool hasTransmit)                { m_flags.m_hasTransmit = hasTransmit; }

RgbFactor const&            MaterialSettings::GetBaseColor () const                                     { return m_baseColor; }
RgbFactor&                  MaterialSettings::GetBaseColorR ()                                          { return m_baseColor; }
void                        MaterialSettings::SetBaseColor (double r, double g, double b)               { m_baseColor.red = r; m_baseColor.green = g; m_baseColor.blue = b; }

RgbFactor const&            MaterialSettings::GetSpecularColor () const                                 { return m_specularColor; }
RgbFactor&                  MaterialSettings::GetSpecularColorR ()                                      { return m_specularColor; }
void                        MaterialSettings::SetSpecularColor (double r, double g, double b)           { m_specularColor.red = r; m_specularColor.green = g; m_specularColor.blue = b; }

RgbFactor const&            MaterialSettings::GetTransmitColor () const                                 { return m_transmitColor; }
RgbFactor&                  MaterialSettings::GetTransmitColorR ()                                      { return m_transmitColor; }
void                        MaterialSettings::SetTransmitColor (double r, double g, double b)           { m_transmitColor.red = r; m_transmitColor.green = g; m_transmitColor.blue = b; }

RgbFactor const&            MaterialSettings::GetTranslucencyColor () const                             { return m_translucencyColor; }
RgbFactor&                  MaterialSettings::GetTranslucencyColorR ()                                  { return m_translucencyColor; }
void                        MaterialSettings::SetTranslucencyColor (double r, double g, double b)       { m_translucencyColor.red = r; m_translucencyColor.green = g; m_translucencyColor.blue = b; }

RgbFactor const&            MaterialSettings::GetGlowColor () const                                     { return m_glowColor; }
RgbFactor&                  MaterialSettings::GetGlowColorR ()                                          { return m_glowColor; }
void                        MaterialSettings::SetGlowColor (double r, double g, double b)               { m_glowColor.red = r; m_glowColor.green = g; m_glowColor.blue = b; }

RgbFactor const&            MaterialSettings::GetReflectColor () const                                  { return m_reflectColor; }
RgbFactor&                  MaterialSettings::GetReflectColorR ()                                       { return m_reflectColor; }
void                        MaterialSettings::SetReflectColor (double r, double g, double b)            { m_reflectColor.red = r; m_reflectColor.green = g; m_reflectColor.blue = b; }

RgbFactor const&            MaterialSettings::GetExitColor () const                                  { return m_exitColor; }
RgbFactor&                  MaterialSettings::GetExitColorR ()                                       { return m_exitColor; }
void                        MaterialSettings::SetExitColor (double r, double g, double b)            { m_exitColor.red = r; m_exitColor.green = g; m_exitColor.blue = b; }

double                      MaterialSettings::GetAmbientIntensity () const                              { return m_ambient; }
void                        MaterialSettings::SetAmbientIntensity (double intensity)                    { m_ambient = intensity; }

double                      MaterialSettings::GetReflectIntensity () const                              { return m_reflect; }
void                        MaterialSettings::SetReflectIntensity (double intensity)                    { m_reflect = intensity; }

double                      MaterialSettings::GetTransmitIntensity () const                             { return m_transmit; }
void                        MaterialSettings::SetTransmitIntensity (double intensity)                   { m_transmit = intensity; }

double                      MaterialSettings::GetDiffuseIntensity () const                              { return m_diffuse; }
void                        MaterialSettings::SetDiffuseIntensity (double intensity)                    { m_diffuse = intensity; }

double                      MaterialSettings::GetSpecularIntensity () const                             { return m_specular; }
void                        MaterialSettings::SetSpecularIntensity (double intensity)                   { m_specular = intensity; }

double                      MaterialSettings::GetRefractIndex () const                                  { return m_refract; }
void                        MaterialSettings::SetRefractIndex (double index)                            { m_refract = index; }

double                      MaterialSettings::GetThickness () const                                     { return m_thickness; }
void                        MaterialSettings::SetThickness (double thickness)                           { m_thickness = thickness; }

double                      MaterialSettings::GetGlowIntensity () const                                 { return m_glow; }
void                        MaterialSettings::SetGlowIntensity (double intensity)                       { m_glow = intensity; }

double                      MaterialSettings::GetTranslucencyScale () const                             { return m_translucency; }
void                        MaterialSettings::SetTranslucencyScale (double scale)                       { m_translucency = scale; }

bool                        MaterialSettings::CastsShadows () const                                     { return !m_flags.m_noShadows; }
void                        MaterialSettings::SetCastsShadows (bool castsShadows)                       { m_flags.m_noShadows = !castsShadows; }

bool                        MaterialSettings::IsVisible () const                                        { return !m_flags.m_invisible; }
void                        MaterialSettings::SetIsVisible (bool isVisible)                             { m_flags.m_invisible = !isVisible; }

int32_t                     MaterialSettings::GetReflectionRays () const                                { return m_reflectionRays; }
void                        MaterialSettings::SetReflectionRays (int32_t rays)                            { m_reflectionRays = rays; }

int32_t                     MaterialSettings::GetRefractionRays () const                                { return m_refractionRays; }
void                        MaterialSettings::SetRefractionRays (int32_t rays)                            { m_refractionRays = rays; }

int32_t                     MaterialSettings::GetSubsurfaceSamples () const                             { return m_subsurfaceSamples; }
void                        MaterialSettings::SetSubsurfaceSamples (int32_t samples)                      { m_subsurfaceSamples = samples; }

double                      MaterialSettings::GetDisplacementDistanceInMillimeters () const             { return m_displacementDistance; }
void                        MaterialSettings::SetDisplacementDistanceInMillimeters (double distance)    { m_displacementDistance = distance; }

double                      MaterialSettings::GetDispersionAmount () const                              { return m_dispersion; }
void                        MaterialSettings::SetDispersionAmount (double dispersion)                   { m_dispersion = dispersion; }

double                      MaterialSettings::GetClearcoatAmount () const                               { return m_clearcoat; }
void                        MaterialSettings::SetClearcoatAmount (double clearcoat)                     { m_clearcoat = clearcoat; }

double                      MaterialSettings::GetAnisotropyAmount () const                              { return m_anisotropy; }
void                        MaterialSettings::SetAnisotropyAmount (double anisotropy)                   { m_anisotropy = anisotropy; }

double                      MaterialSettings::GetFrontWeightingAmount () const                          { return m_frontWeighting; }
void                        MaterialSettings::SetFrontWeightingAmount (double frontWeighting)           { m_frontWeighting = frontWeighting; }

double                      MaterialSettings::GetScatterDistanceInMeters () const                       { return m_scatterDistance; }
void                        MaterialSettings::SetScatterDistanceInMeters (double scatterDistance)       { m_scatterDistance = scatterDistance; }

double                      MaterialSettings::GetReflectionFresnel () const                             { return m_reflectFresnel; }
void                        MaterialSettings::SetReflectionFresnel (double fresnel)                     { m_reflectFresnel = fresnel; }

double                      MaterialSettings::GetDissolveAmount () const                                { return m_dissolve; }
void                        MaterialSettings::SetDissolveAmount (double dissolve)                       { m_dissolve = dissolve; }

double                      MaterialSettings::GetAbsorptionDistance () const                            { return m_absorptionDistance; }
void                        MaterialSettings::SetAbsorptionDistance (double distance)                   { m_absorptionDistance = distance; }

double                      MaterialSettings::GetRefractionRoughness () const                           { return m_refractionRoughness; }
void                        MaterialSettings::SetRefractionRoughness (double roughness)                 { m_refractionRoughness = roughness; }

bool                        MaterialSettings::BlurReflections () const                                  { return m_blurReflections; }
void                        MaterialSettings::SetBlurReflections (bool blurReflections)                 { m_blurReflections = blurReflections; }

bool                        MaterialSettings::BlurRefractions () const                                  { return m_blurRefractions; }
void                        MaterialSettings::SetBlurRefractions (bool blurRefractions)                 { m_blurRefractions = blurRefractions; }

MaterialSettings::BackFaceCulling MaterialSettings::GetBackFaceCulling () const                               { return m_backFaceCulling;}
void                        MaterialSettings::SetBackFaceCulling (MaterialSettings::BackFaceCulling backFaceCulling)      { m_backFaceCulling = backFaceCulling;}

bool                        MaterialSettings::UseFur () const                                           { return m_useFur; }
void                        MaterialSettings::SetUseFur (bool useFur)                                   { m_useFur = useFur; }

bool                        MaterialSettings::LockSpecularAndReflect () const                           { return m_flags.m_lockSpecularAndReflect; }
void                        MaterialSettings::SetLockSpecularAndReflect (bool lock)                     { m_flags.m_lockSpecularAndReflect = lock; }

bool                        MaterialSettings::LockEfficiency () const                                   { return m_flags.m_lockEfficiency; }
void                        MaterialSettings::SetLockEfficiency (bool lock)                             { m_flags.m_lockEfficiency = lock; }

bool                        MaterialSettings::LockSpecularAndBase () const                              { return m_flags.m_lockSpecularAndBase; }
void                        MaterialSettings::SetLockSpecularAndBase (bool lock)                        { m_flags.m_lockSpecularAndBase = lock; }

bool                        MaterialSettings::LockFinishAndSpecular () const                            { return m_flags.m_lockFinishToSpecular; }
void                        MaterialSettings::SetLockFinishAndSpecular (bool lock)                      { m_flags.m_lockFinishToSpecular = lock; }

bool                        MaterialSettings::LockFresnelToReflect () const                             { return m_flags.m_lockFresnelToReflect; }
void                        MaterialSettings::SetLockFresnelToReflect (bool lock)                       { m_flags.m_lockFresnelToReflect = lock; }

bool                        MaterialSettings::LockRefractionRoughnessToFinish () const                  { return m_flags.m_lockRefractionRoughnessToFinish; }
void                        MaterialSettings::SetLockRefractionRoughnessToFinish (bool lock)            { m_flags.m_lockRefractionRoughnessToFinish = lock; }

bool                        MaterialSettings::HasCustomSpecular () const                                { return m_flags.m_customSpecular; }
void                        MaterialSettings::SetHasCustomSpecular (bool customSpecular)                { m_flags.m_customSpecular = customSpecular; }

WStringCR                   MaterialSettings::GetCutSectionMaterialName () const                        {return m_cutSectionMaterialName;}
void                        MaterialSettings::SetCutSectionMaterialName (WCharCP name)                  {m_cutSectionMaterialName = name;}

WStringCR                   MaterialSettings::GetCutSectionMaterialPalette () const                     {return m_cutSectionPalette;}
WStringR                    MaterialSettings::GetCutSectionMaterialPaletteR ()                          {return m_cutSectionPalette;}

bool                        MaterialSettings::UseCutSectionMaterial () const                            {return m_useCutSectionMaterial;}
void                        MaterialSettings::SetUseCutSectionMaterial (bool useCutSectionMaterial)     {m_useCutSectionMaterial = useCutSectionMaterial;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     07/10
//---------------------------------------------------------------------------------------
double MaterialSettings::GetFinishScale () const
    {
    // This was "convertFinishToMatedit".
    return floor (log10 (m_finish < s_minimumFinish ? 1.0 : s_finishFactor * m_finish) / s_finishExponent + 0.5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     07/10
//---------------------------------------------------------------------------------------
void MaterialSettings::SetFinishScale (double scale)
    {
    // This was "convertFinishFromMatedit".
    m_finish = pow (10.0, s_finishExponent * (scale > 2000.0 ? 2000.0 : scale)) / s_finishFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
double MaterialSettings::GetFinishScaleForStorage () const
    {
    return m_finish;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialSettings::SetFinishScaleFromStorage (double scale)
    {
    m_finish = scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialShader::MaterialShader ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialShader::InitDefaults ()
    {
    m_flags.m_enabled                   = true;
    m_flags.m_invert                    = false;
    m_flags.m_receiveShadows            = true;
    m_flags.m_visibleToCamera           = true;
    m_flags.m_visibleToIndirectRays     = true;
    m_flags.m_visibleToReflectionRays   = true;
    m_flags.m_visibleToRefractionRays   = true;
    m_flags.m_castShadows               = true;

    m_shadingRate                       = 1.0;
    m_directIllumMultiplier             = 100.0;
    m_indirectIllumMultiplier           = 100.0;
    m_indirectIllumSaturation           = 100.0;
    m_indirectIllumType                 = ILLUMTYPE_IrradianceCaching;
    m_blend                             = BLENDMODE_Normal;
    m_opacity                           = 100.0;
    m_type                              = SHADERTYPE_Default;
    m_effect                            = SHADEREFFECT_FullShading;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialShader::Equals (MaterialShaderCR rhs) const
    {
    RETURN_IF_FALSE                 (rhs.m_type                             ==  m_type);
    RETURN_IF_FALSE                 (rhs.m_flags.m_enabled                  ==  m_flags.m_enabled);
    RETURN_IF_FALSE                 (rhs.m_flags.m_invert                   ==  m_flags.m_invert);
    RETURN_IF_FALSE                 (rhs.m_flags.m_receiveShadows           ==  m_flags.m_receiveShadows);
    RETURN_IF_FALSE                 (rhs.m_flags.m_visibleToCamera          ==  m_flags.m_visibleToCamera);
    RETURN_IF_FALSE                 (rhs.m_flags.m_visibleToIndirectRays    ==  m_flags.m_visibleToIndirectRays);
    RETURN_IF_FALSE                 (rhs.m_flags.m_visibleToReflectionRays  ==  m_flags.m_visibleToReflectionRays);
    RETURN_IF_FALSE                 (rhs.m_flags.m_visibleToRefractionRays  ==  m_flags.m_visibleToRefractionRays);
    RETURN_IF_FALSE                 (rhs.m_flags.m_castShadows              ==  m_flags.m_castShadows);
    RETURN_IF_FALSE                 (rhs.m_indirectIllumType                ==  m_indirectIllumType);
    RETURN_IF_FALSE                 (rhs.m_blend                            ==  m_blend);
    RETURN_IF_FALSE                 (rhs.m_effect                           ==  m_effect);
    RETURN_IF_FALSE (valuesEqual    (rhs.m_shadingRate,                         m_shadingRate));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_directIllumMultiplier,               m_directIllumMultiplier));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_indirectIllumMultiplier,             m_indirectIllumMultiplier));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_indirectIllumSaturation,             m_indirectIllumSaturation));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_opacity,                             m_opacity));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialShader::Copy (MaterialShaderCR rhs)
    {
    m_type                      = rhs.m_type;
    m_flags                     = rhs.m_flags;
    m_shadingRate               = rhs.m_shadingRate;
    m_directIllumMultiplier     = rhs.m_directIllumMultiplier;
    m_indirectIllumMultiplier   = rhs.m_indirectIllumMultiplier;
    m_indirectIllumSaturation   = rhs.m_indirectIllumSaturation;
    m_indirectIllumType         = rhs.m_indirectIllumType;
    m_opacity                   = rhs.m_opacity;
    m_blend                     = rhs.m_blend;
    m_effect                    = rhs.m_effect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t const*                       MaterialShader::GetFlagsCP () const                                                     { return (uint32_t const *) &m_flags; }
void                                MaterialShader::SetFlags (uint32_t flags)                                                 { memcpy (&m_flags, &flags, sizeof (flags)); }

MaterialShader::ShaderType          MaterialShader::GetType () const                                                        { return m_type; }
void                                MaterialShader::SetType (MaterialShader::ShaderType type)                               { m_type = type; }

bool                                MaterialShader::IsEnabled () const                                                      { return m_flags.m_enabled; }
void                                MaterialShader::SetIsEnabled (bool isEnabled)                                           { m_flags.m_enabled = isEnabled; }

bool                                MaterialShader::IsInverted () const                                                     { return m_flags.m_invert; }
void                                MaterialShader::SetIsInverted (bool isInverted)                                         { m_flags.m_invert = isInverted; }

bool                                MaterialShader::ReceivesShadows () const                                                { return m_flags.m_receiveShadows; }
void                                MaterialShader::SetReceivesShadows (bool receivesShadows)                               { m_flags.m_receiveShadows = receivesShadows; }

bool                                MaterialShader::CastsShadows () const                                                   { return m_flags.m_castShadows; }
void                                MaterialShader::SetCastsShadows (bool castsShadows)                                     { m_flags.m_castShadows = castsShadows; }

bool                                MaterialShader::IsVisibleToCamera () const                                              { return m_flags.m_visibleToCamera; }
void                                MaterialShader::SetIsVisibleToCamera (bool isVisible)                                   { m_flags.m_visibleToCamera = isVisible; }

bool                                MaterialShader::IsVisibleToIndirectRays () const                                        { return m_flags.m_visibleToIndirectRays; }
void                                MaterialShader::SetIsVisibleToIndirectRays (bool isVisible)                             { m_flags.m_visibleToIndirectRays = isVisible; }

bool                                MaterialShader::IsVisibleToReflectionRays () const                                      { return m_flags.m_visibleToReflectionRays; }
void                                MaterialShader::SetIsVisibleToReflectionRays (bool isVisible)                           { m_flags.m_visibleToReflectionRays = isVisible; }

bool                                MaterialShader::IsVisibleToRefractionRays () const                                      { return m_flags.m_visibleToRefractionRays; }
void                                MaterialShader::SetIsVisibleToRefractionRays (bool isVisible)                           { m_flags.m_visibleToRefractionRays = isVisible; }

double                              MaterialShader::GetShadingRate () const                                                 { return m_shadingRate; }
void                                MaterialShader::SetShadingRate (double shadingRate)                                     { m_shadingRate = shadingRate; }

double                              MaterialShader::GetDirectIlluminationMultiplier () const                                { return m_directIllumMultiplier; }
void                                MaterialShader::SetDirectIlluminationMultiplier (double multiplier)                     { m_directIllumMultiplier = multiplier; }

double                              MaterialShader::GetIndirectIlluminationMultiplier () const                              { return m_indirectIllumMultiplier; }
void                                MaterialShader::SetIndirectIlluminationMultiplier (double multiplier)                   { m_indirectIllumMultiplier = multiplier; }

double                              MaterialShader::GetIndirectIlluminationSaturation () const                              { return m_indirectIllumSaturation; }
void                                MaterialShader::SetIndirectIlluminationSaturation (double saturation)                   { m_indirectIllumSaturation = saturation; }

double                              MaterialShader::GetOpacity () const                                                     { return m_opacity; }
void                                MaterialShader::SetOpacity (double opacity)                                             { m_opacity = opacity; }

MaterialShader::IlluminationType    MaterialShader::GetIndirectIlluminationType () const                                    { return m_indirectIllumType; }
void                                MaterialShader::SetIndirectIlluminationType (MaterialShader::IlluminationType type)     { m_indirectIllumType = type; }

MaterialShader::BlendMode           MaterialShader::GetBlendMode () const                                                   { return m_blend; }
void                                MaterialShader::SetBlendMode (MaterialShader::BlendMode mode)                           { m_blend = mode; }

MaterialShader::ShaderEffect        MaterialShader::GetEffect () const                                                      { return m_effect; }
void                                MaterialShader::SetEffect (MaterialShader::ShaderEffect effect)                         { m_effect = effect; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderIterator::MaterialShaderIterator (MaterialShaderCollectionR shaders, bool wantBegin)
    {
    MaterialShaderPtrList::iterator* iter = new MaterialShaderPtrList::iterator ();
    *iter = wantBegin ? shaders.m_shaders.begin () : shaders.m_shaders.end ();
    m_iter = iter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderIterator::~MaterialShaderIterator ()
    {
    delete reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderIterator& MaterialShaderIterator::operator++ ()
    {
    ++*reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderIterator& MaterialShaderIterator::operator-- ()
    {
    --*reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderR MaterialShaderIterator::operator* ()
    {
    MaterialShaderPtrList::iterator* iter = reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter);
    return *(*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderP MaterialShaderIterator::operator-> ()
    {
    MaterialShaderPtrList::iterator* iter = reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter);
    return (*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialShaderIterator::operator== (MaterialShaderIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter)) == *(reinterpret_cast <MaterialShaderPtrList::iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialShaderIterator::operator!= (MaterialShaderIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter)) != *(reinterpret_cast <MaterialShaderPtrList::iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderPtrList::iterator MaterialShaderIterator::Get ()
    {
    return *reinterpret_cast <MaterialShaderPtrList::iterator *> (m_iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderConstIterator::MaterialShaderConstIterator (MaterialShaderCollectionCR shaders, bool wantBegin)
    {
    MaterialShaderPtrList::const_iterator* iter = new MaterialShaderPtrList::const_iterator ();
    *iter = wantBegin ? shaders.m_shaders.begin () : shaders.m_shaders.end ();
    m_iter = iter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderConstIterator::~MaterialShaderConstIterator ()
    {
    delete reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderConstIterator& MaterialShaderConstIterator::operator++ ()
    {
    ++*reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderConstIterator& MaterialShaderConstIterator::operator-- ()
    {
    --*reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderCR MaterialShaderConstIterator::operator* () const
    {
    MaterialShaderPtrList::const_iterator* iter = reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter);
    return *(*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderCP MaterialShaderConstIterator::operator-> () const
    {
    MaterialShaderPtrList::const_iterator* iter = reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter);
    return (*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialShaderConstIterator::operator== (MaterialShaderConstIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter)) == *(reinterpret_cast <MaterialShaderPtrList::const_iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialShaderConstIterator::operator!= (MaterialShaderConstIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter)) != *(reinterpret_cast <MaterialShaderPtrList::const_iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialShaderPtrList::const_iterator MaterialShaderConstIterator::Get () const
    {
    return *reinterpret_cast <MaterialShaderPtrList::const_iterator *> (m_iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialShaderCollection::MaterialShaderCollection ()
    {
    m_shaders.push_back (MaterialShaderPtr (new MaterialShader ()));
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialShaderCollection::InitDefaults ()
    {
    if (1 < m_shaders.size ())
        {
        MaterialShaderPtrList::iterator iter = m_shaders.begin ();
        m_shaders.erase (++iter, m_shaders.end ());
        }

    m_shaders.back ()->InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialShaderConstIterator     MaterialShaderCollection::begin () const        { return MaterialShaderConstIterator (*this, true); }
MaterialShaderConstIterator     MaterialShaderCollection::end () const          { return MaterialShaderConstIterator (*this, false); }
MaterialShaderIterator          MaterialShaderCollection::begin ()              { return MaterialShaderIterator (*this, true); }
MaterialShaderIterator          MaterialShaderCollection::end ()                { return MaterialShaderIterator (*this, false); }
size_t                          MaterialShaderCollection::Size () const         { return m_shaders.size (); }
MaterialShaderCR                MaterialShaderCollection::GetTopShader () const { return *m_shaders.begin ()->get (); }
MaterialShaderR                 MaterialShaderCollection::GetTopShaderR ()      { return *m_shaders.begin ()->get (); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialShaderCollection::Copy (MaterialShaderCollectionCR rhs)
    {
    m_shaders.clear ();
    FOR_EACH (MaterialShaderPtr const& shader , rhs.m_shaders)
        AddShader ().Copy (*shader.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialShaderCollection::Equals (MaterialShaderCollectionCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_shaders.size () == m_shaders.size ());
    for (MaterialShaderPtrList::const_iterator rhsIter = rhs.m_shaders.begin (), iter = m_shaders.begin (); m_shaders.end () != iter; ++iter, ++rhsIter)
        {
        RETURN_IF_FALSE ((*rhsIter)->Equals (*iter->get ()));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialShaderR MaterialShaderCollection::AddShader ()
    {
    m_shaders.push_back (MaterialShaderPtr (new MaterialShader ()));
    return *m_shaders.back ().get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MaterialShaderCollection::DeleteShader (MaterialShaderIterator iter)
    {
    if (m_shaders.end () == iter.Get ())
        return ERROR;

    if (1 == m_shaders.size ())
        m_shaders.back ()->InitDefaults ();
    else
        m_shaders.erase (iter.Get ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialFur::MaterialFur ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialFur::InitDefaults ()
    {
    memset (&m_flags, 0, sizeof (m_flags));
    m_flags.m_adaptiveSampling  = true;

    m_spacing                   = 0.05;
    m_length                    = 0.12;
    m_width                     = 50.0;
    m_taper                     = 100.0;
    m_offset                    = 0.0;
    m_stripRotation             = 0.0;
    m_growthJitter              = 25.0;
    m_positionJitter            = 50.0;
    m_directionJitter           = 25.0;
    m_sizeJitter                = 50.0;
    m_flex                      = 50.0;
    m_rootBend                  = 0.0;
    m_curls                     = 0.0;
    m_bumpAmplitude             = 50.0;
    m_guideRange                = 0.1;
    m_guideLength               = 100.0;
    m_blendAmount               = 0.0;
    m_blendAngle                = 45.0;     // Degrees
    m_clumps                    = 0.0;
    m_clumpRange                = 0.1;

    m_type                      = FURTYPE_Strips;
    m_guides                    = FURGUIDES_None;
    m_segments                  = 4;
    m_rate                      = 5.0;
    m_billboard                 = FURBILLBOARD_Off;

    m_furMaterialName           = "";
    m_furMaterialPalette        = "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialFur::Equals (MaterialFurCR rhs) const
    {
    RETURN_IF_FALSE                 (rhs.m_flags.m_adaptiveSampling     ==  m_flags.m_adaptiveSampling);
    RETURN_IF_FALSE                 (rhs.m_flags.m_removeBaseSurface    ==  m_flags.m_removeBaseSurface);
    RETURN_IF_FALSE                 (rhs.m_flags.m_autoFading           ==  m_flags.m_autoFading);
    RETURN_IF_FALSE                 (rhs.m_flags.m_useHairShader        ==  m_flags.m_useHairShader);
    RETURN_IF_FALSE                 (rhs.m_flags.m_useFurMaterial       ==  m_flags.m_useFurMaterial);
    RETURN_IF_FALSE                 (rhs.m_flags.m_frustumCulling       ==  m_flags.m_frustumCulling);
    RETURN_IF_FALSE                 (rhs.m_type                         ==  m_type);
    RETURN_IF_FALSE                 (rhs.m_guides                       ==  m_guides);
    RETURN_IF_FALSE                 (rhs.m_segments                     ==  m_segments);
    RETURN_IF_FALSE                 (rhs.m_billboard                    ==  m_billboard);
    RETURN_IF_FALSE (valuesEqual    (rhs.m_spacing,                         m_spacing));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_length,                          m_length));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_width,                           m_width));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_taper,                           m_taper));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_offset,                          m_offset));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_stripRotation,                   m_stripRotation));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_growthJitter,                    m_growthJitter));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_positionJitter,                  m_positionJitter));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_directionJitter,                 m_directionJitter));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_sizeJitter,                      m_sizeJitter));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_flex,                            m_flex));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_rootBend,                        m_rootBend));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_curls,                           m_curls));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_bumpAmplitude,                   m_bumpAmplitude));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_guideRange,                      m_guideRange));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_guideLength,                     m_guideLength));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_blendAmount,                     m_blendAmount));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_blendAngle,                      m_blendAngle));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_clumps,                          m_clumps));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_clumpRange,                      m_clumpRange));
    RETURN_IF_FALSE (valuesEqual    (rhs.m_rate,                            m_rate));

    RETURN_IF_FALSE (rhs.m_furMaterialName.EqualsI (m_furMaterialName.c_str()));
    RETURN_IF_FALSE (rhs.m_furMaterialPalette.EqualsI (m_furMaterialPalette.c_str()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialFur::Copy (MaterialFurCR rhs)
    {
    m_spacing                   = rhs.m_spacing;
    m_length                    = rhs.m_length;
    m_width                     = rhs.m_width;
    m_taper                     = rhs.m_taper;
    m_offset                    = rhs.m_offset;
    m_stripRotation             = rhs.m_stripRotation;
    m_growthJitter              = rhs.m_growthJitter;
    m_positionJitter            = rhs.m_positionJitter;
    m_directionJitter           = rhs.m_directionJitter;
    m_sizeJitter                = rhs.m_sizeJitter;
    m_flex                      = rhs.m_flex;
    m_rootBend                  = rhs.m_rootBend;
    m_curls                     = rhs.m_curls;
    m_bumpAmplitude             = rhs.m_bumpAmplitude;
    m_guideRange                = rhs.m_guideRange;
    m_guideLength               = rhs.m_guideLength;
    m_blendAmount               = rhs.m_blendAmount;
    m_blendAngle                = rhs.m_blendAngle;
    m_clumps                    = rhs.m_clumps;
    m_clumpRange                = rhs.m_clumpRange;
    m_rate                      = rhs.m_rate;

    m_flags.m_adaptiveSampling  = rhs.m_flags.m_adaptiveSampling;
    m_flags.m_removeBaseSurface = rhs.m_flags.m_removeBaseSurface;
    m_flags.m_autoFading        = rhs.m_flags.m_autoFading;
    m_flags.m_useHairShader     = rhs.m_flags.m_useHairShader;
    m_flags.m_useFurMaterial    = rhs.m_flags.m_useFurMaterial;
    m_flags.m_frustumCulling    = rhs.m_flags.m_frustumCulling;

    m_type                      = rhs.m_type;
    m_guides                    = rhs.m_guides;
    m_segments                  = rhs.m_segments;
    m_billboard                 = rhs.m_billboard;

    m_furMaterialName           = rhs.m_furMaterialName;
    m_furMaterialPalette        = rhs.m_furMaterialPalette;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t const*           MaterialFur::GetFlagsCP () const                            { return (uint32_t const *) &m_flags; }
void                    MaterialFur::SetFlags (uint32_t flags)                        { memcpy (&m_flags, &flags, sizeof (flags)); }

bool                    MaterialFur::UseAdaptiveSampling () const                   { return m_flags.m_adaptiveSampling; }
void                    MaterialFur::SetUseAdaptiveSampling (bool useAdaptive)      { m_flags.m_adaptiveSampling = useAdaptive; }

bool                    MaterialFur::RemoveBaseSurface () const                     { return m_flags.m_removeBaseSurface; }
void                    MaterialFur::SetRemoveBaseSurface (bool removeBase)         { m_flags.m_removeBaseSurface = removeBase; }

bool                    MaterialFur::UseAutoFading () const                         { return m_flags.m_autoFading; }
void                    MaterialFur::SetUseAutoFading (bool useAutoFading)          { m_flags.m_autoFading = useAutoFading; }

bool                    MaterialFur::UseHairShader () const                         { return m_flags.m_useHairShader; }
void                    MaterialFur::SetUseHairShader (bool useHairShader)          { m_flags.m_useHairShader = useHairShader; }

bool                    MaterialFur::UseFurMaterial () const                        { return m_flags.m_useFurMaterial; }
void                    MaterialFur::SetUseFurMaterial (bool useFurMaterial)        { m_flags.m_useFurMaterial = useFurMaterial; }

bool                    MaterialFur::UseFrustumCulling () const                     {return m_flags.m_frustumCulling;}
void                    MaterialFur::SetUseFrustumCulling (bool frustumCulling)     {m_flags.m_frustumCulling = frustumCulling;}

double                  MaterialFur::GetSpacingInMeters () const                    { return m_spacing; }
void                    MaterialFur::SetSpacingInMeters (double spacing)            { m_spacing = spacing; }

double                  MaterialFur::GetLengthInMeters () const                     { return m_length; }
void                    MaterialFur::SetLengthInMeters (double length)              { m_length = length; }

double                  MaterialFur::GetWidthScale () const                         { return m_width; }
void                    MaterialFur::SetWidthScale (double width)                   { m_width = width; }

double                  MaterialFur::GetTaperScale () const                         { return m_taper; }
void                    MaterialFur::SetTaperScale (double taper)                   { m_taper = taper; }

double                  MaterialFur::GetOffsetInMeters () const                     { return m_offset; }
void                    MaterialFur::SetOffsetInMeters (double offset)              { m_offset = offset; }

double                  MaterialFur::GetStripRotation () const                      { return m_stripRotation; }
void                    MaterialFur::SetStripRotation (double rotation)             { m_stripRotation = rotation; }

double                  MaterialFur::GetGrowthJitter () const                       { return m_growthJitter; }
void                    MaterialFur::SetGrowthJitter (double jitter)                { m_growthJitter = jitter; }

double                  MaterialFur::GetPositionJitter () const                     { return m_positionJitter; }
void                    MaterialFur::SetPositionJitter (double jitter)              { m_positionJitter = jitter; }

double                  MaterialFur::GetDirectionJitter () const                    { return m_directionJitter; }
void                    MaterialFur::SetDirectionJitter (double jitter)             { m_directionJitter = jitter; }

double                  MaterialFur::GetSizeJitter () const                         { return m_sizeJitter; }
void                    MaterialFur::SetSizeJitter (double jitter)                  { m_sizeJitter = jitter; }

double                  MaterialFur::GetFlex () const                               { return m_flex; }
void                    MaterialFur::SetFlex (double flex)                          { m_flex = flex; }

double                  MaterialFur::GetRootBend () const                           { return m_rootBend; }
void                    MaterialFur::SetRootBend (double rootBend)                  { m_rootBend = rootBend; }

double                  MaterialFur::GetCurls () const                              { return m_curls; }
void                    MaterialFur::SetCurls (double curls)                        { m_curls = curls; }

double                  MaterialFur::GetBumpAmplitude () const                      { return m_bumpAmplitude; }
void                    MaterialFur::SetBumpAmplitude (double amplitude)            { m_bumpAmplitude = amplitude; }

double                  MaterialFur::GetGuideRangeInMeters () const                 { return m_guideRange; }
void                    MaterialFur::SetGuideRangeInMeters (double range)           { m_guideRange = range; }

double                  MaterialFur::GetGuideLengthInMeters () const                { return m_guideLength; }
void                    MaterialFur::SetGuideLengthInMeters (double length)         { m_guideLength = length; }

double                  MaterialFur::GetBlendAmount () const                        { return m_blendAmount; }
void                    MaterialFur::SetBlendAmount (double amount)                 { m_blendAmount = amount; }

double                  MaterialFur::GetBlendAngle () const                         { return m_blendAngle; }
void                    MaterialFur::SetBlendAngle (double angle)                   { m_blendAngle = angle; }

double                  MaterialFur::GetClumpScale () const                         { return m_clumps; }
void                    MaterialFur::SetClumpScale (double scale)                   { m_clumps = scale; }

double                  MaterialFur::GetClumpRangeInMeters () const                 { return m_clumpRange; }
void                    MaterialFur::SetClumpRangeInMeters (double range)           { m_clumpRange = range; }

double                  MaterialFur::GetRate () const                               { return m_rate; }
void                    MaterialFur::SetRate (double rate)                          { m_rate = rate; }

MaterialFur::FurType    MaterialFur::GetType () const                               { return m_type; }
void                    MaterialFur::SetType (MaterialFur::FurType type)            { m_type = type; }

MaterialFur::FurGuides  MaterialFur::GetFurGuides () const                          { return m_guides; }
void                    MaterialFur::SetFurGuides (MaterialFur::FurGuides guides)   { m_guides = guides; }

uint32_t                MaterialFur::GetSegmentCount () const                       { return m_segments; }
void                    MaterialFur::SetSegmentCount (uint32_t count)                 { m_segments = count; }

MaterialFur::FurBillboard MaterialFur::GetFurBillboard () const                       {return m_billboard;}
void                    MaterialFur::SetFurBillboard (MaterialFur::FurBillboard billboard) { m_billboard = billboard;}

Utf8StringCR            MaterialFur::GetFurMaterialName () const                    { return m_furMaterialName; }
Utf8StringR             MaterialFur::GetFurMaterialNameR ()                         { return m_furMaterialName; }

Utf8StringCR            MaterialFur::GetFurMaterialPalette () const                 { return m_furMaterialPalette; }
Utf8StringR             MaterialFur::GetFurMaterialPaletteR ()                      { return m_furMaterialPalette; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMap::MaterialMap (MaterialMap::MapType type, MaterialMapCollectionR collection) : m_mapCollection (collection)
    {
    m_type = type;
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMap::InitDefaults ()
    {
    m_linkType  = MAPTYPE_None;
    m_enabled   = true;
    m_value     = 1.0;
    m_projectionOffset.Zero ();
    m_projectionRotation.Zero ();
    m_projectionScale.One ();
    m_layers.InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCP MaterialMap::GetLinkedMapCP () const
    {
    if (MAPTYPE_None == m_linkType)
        return this;
    else
        {
        MaterialMapCP map = this;
        while (NULL != (map = m_mapCollection.GetMapCP (map->GetLinkType ())) && MAPTYPE_None != map->GetLinkType ());

        return map;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapP MaterialMap::GetLinkedMapP ()
    {
    if (MAPTYPE_None == m_linkType)
        return this;
    else
        {
        MaterialMapP map = this;
        while (NULL != (map = m_mapCollection.GetMapP (map->GetLinkType ())) && MAPTYPE_None != map->GetLinkType ());

        return map;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialMap::Equals (MaterialMapCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_type     ==                  m_type);
    RETURN_IF_FALSE (rhs.m_linkType ==                  m_linkType);
    RETURN_IF_FALSE (rhs.m_enabled  ==                  m_enabled);

    MaterialMapCP myLinkedMap = (MAPTYPE_None == m_linkType) ? NULL : GetLinkedMapCP ();
    MaterialMapCP rhsLinkedMap = (MAPTYPE_None == rhs.m_linkType) ? NULL : rhs.GetLinkedMapCP ();

    if (MAPTYPE_None == m_linkType || NULL == myLinkedMap || NULL == rhsLinkedMap)
        {
        RETURN_IF_FALSE (rhs.m_projectionOffset.IsEqual     (m_projectionOffset,    1.0E-4));
        RETURN_IF_FALSE (rhs.m_projectionRotation.IsEqual   (m_projectionRotation,  1.0E-4));
        RETURN_IF_FALSE (rhs.m_projectionScale.IsEqual      (m_projectionScale,     1.0E-4));

        if (MAPTYPE_Pattern == m_type || MAPTYPE_Bump == m_type)
            {
            RETURN_IF_FALSE (valuesEqual (rhs.m_value, m_value));
            }

        RETURN_IF_FALSE (rhs.m_layers.Equals (m_layers));
        }
    else if (myLinkedMap && rhsLinkedMap)
        {
        RETURN_IF_FALSE (rhsLinkedMap->m_projectionOffset.IsEqual     (myLinkedMap->m_projectionOffset,    1.0E-4));
        RETURN_IF_FALSE (rhsLinkedMap->m_projectionRotation.IsEqual   (myLinkedMap->m_projectionRotation,  1.0E-4));
        RETURN_IF_FALSE (rhsLinkedMap->m_projectionScale.IsEqual      (myLinkedMap->m_projectionScale,     1.0E-4));

        if (MAPTYPE_Pattern == m_type || MAPTYPE_Bump == m_type)
            {
            RETURN_IF_FALSE (valuesEqual (rhsLinkedMap->m_value, myLinkedMap->m_value));
            }

        RETURN_IF_FALSE (rhsLinkedMap->m_layers.Equals (myLinkedMap->m_layers));
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMap::Copy (MaterialMapCR rhs)
    {
    m_linkType              = rhs.m_linkType;
    // m_type intentionally not set - that must be set when a map is first created.
    m_enabled               = rhs.m_enabled;
    m_value                 = rhs.m_value;
    m_projectionOffset      = rhs.m_projectionOffset;
    m_projectionRotation    = rhs.m_projectionRotation;
    m_projectionScale       = rhs.m_projectionScale;

    if (MaterialMap::MAPTYPE_Geometry == m_type)
        {
        MaterialMapLayerCR  rhsTopLayer = rhs.GetLayers ().GetTopLayer ();
        MaterialMapLayerR   myLayer = m_layers.GetTopLayerR ();

        myLayer.SetMode (rhsTopLayer.GetMode ());
        myLayer.SetUnits (rhsTopLayer.GetUnits ());
        myLayer.SetRotation (rhsTopLayer.GetRotation ());
        myLayer.GetScaleR () = rhsTopLayer.GetScale ();
        myLayer.GetOffsetR () = rhsTopLayer.GetOffset ();
        }
    else
        m_layers.Copy (rhs.m_layers);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/10
//---------------------------------------------------------------------------------------
bool MaterialMap::IsValueMap (MaterialMap::MapType type)
    {
    switch (type)
        {
        case MaterialMap::MAPTYPE_Bump:
        case MaterialMap::MAPTYPE_Specular:
        case MaterialMap::MAPTYPE_Reflect:
        case MaterialMap::MAPTYPE_Transparency:
        case MaterialMap::MAPTYPE_Translucency:
        case MaterialMap::MAPTYPE_Finish:
        case MaterialMap::MAPTYPE_Diffuse:
        case MaterialMap::MAPTYPE_GlowAmount:
        case MaterialMap::MAPTYPE_ClearcoatAmount:
        case MaterialMap::MAPTYPE_AnisotropicDirection:
        case MaterialMap::MAPTYPE_Displacement:
        case MaterialMap::MAPTYPE_Normal:
        case MaterialMap::MAPTYPE_FurLength:
        case MaterialMap::MAPTYPE_FurDensity:
        case MaterialMap::MAPTYPE_FurJitter:
        case MaterialMap::MAPTYPE_FurFlex:
        case MaterialMap::MAPTYPE_FurClumps:
        case MaterialMap::MAPTYPE_FurDirection:
        case MaterialMap::MAPTYPE_FurVector:
        case MaterialMap::MAPTYPE_FurBump:
        case MaterialMap::MAPTYPE_FurCurls:
        case MaterialMap::MAPTYPE_RefractionRoughness:
        case MaterialMap::MAPTYPE_SpecularFresnel:
            return true;
        case MaterialMap::MAPTYPE_None:
        case MaterialMap::MAPTYPE_Pattern:
        case MaterialMap::MAPTYPE_SpecularColor:
        case MaterialMap::MAPTYPE_TransparentColor:
        case MaterialMap::MAPTYPE_TranslucencyColor:
        case MaterialMap::MAPTYPE_GlowColor:
        case MaterialMap::MAPTYPE_ReflectColor:
        default:
            return false;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialMap::IsValueMap () const
    {
    return IsValueMap (GetType ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMap::MapType            MaterialMap::GetType () const                                       { return m_type; }

MaterialMap::MapType            MaterialMap::GetLinkType () const                                   { return m_linkType; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  MaterialMap::SetLinkType (MaterialMap::MapType type)
    {
    MaterialMapP map = m_mapCollection.GetMapP (type);

    if (map)
        while (map != this && MAPTYPE_None != map->GetLinkType () && NULL != (map = m_mapCollection.GetMapP (map->GetLinkType ())));

    if (NULL == map  || map == this)
        return ERROR;

    m_linkType = type;

    return SUCCESS;
    }

bool                            MaterialMap::IsEnabled () const                                     { return m_enabled; }
void                            MaterialMap::SetIsEnabled (bool isEnabled)                          { m_enabled = isEnabled; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
double                          MaterialMap::GetValue () const
    {
    MaterialMapCP map = GetLinkedMapCP ();
    return NULL != map ? map->m_value : m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                            MaterialMap::SetValue (double value)
    {
    MaterialMapP map = GetLinkedMapP ();
    if (map)
        map->m_value = value;
    else
        m_value = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR                      MaterialMap::GetProjectionOffset () const
    {
    MaterialMapCP map = GetLinkedMapCP ();
    return NULL != map ? map->m_projectionOffset : m_projectionOffset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dR                       MaterialMap::GetProjectionOffsetR ()
    {
    MaterialMapP map = GetLinkedMapP ();
    return NULL != map ? map->m_projectionOffset : m_projectionOffset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                            MaterialMap::SetProjectionOffset (double x, double y, double z)
    {
    MaterialMapP map = GetLinkedMapP ();
    if (map)
        map->m_projectionOffset.Init (x, y, z);
    else
        m_projectionOffset.Init (x, y, z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR                      MaterialMap::GetProjectionRotation () const
    {
    MaterialMapCP map = GetLinkedMapCP ();
    return NULL != map ? map->m_projectionRotation : m_projectionRotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dR                       MaterialMap::GetProjectionRotationR ()
    {
    MaterialMapP map = GetLinkedMapP ();
    return NULL != map ? map->m_projectionRotation : m_projectionRotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                            MaterialMap::SetProjectionRotation (double x, double y, double z)
    {
    MaterialMapP map = GetLinkedMapP ();
    if (map)
        map->m_projectionRotation.Init (x, y, z);
    else
        m_projectionRotation.Init (x, y, z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR                      MaterialMap::GetProjectionScale () const
    {
    MaterialMapCP map = GetLinkedMapCP ();
    return NULL != map ? map->m_projectionScale : m_projectionScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dR                       MaterialMap::GetProjectionScaleR ()
    {
    MaterialMapP map = GetLinkedMapP ();
    return NULL != map ? map->m_projectionScale : m_projectionScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                            MaterialMap::SetProjectionScale (double x, double y, double z)
    {
    MaterialMapP map = GetLinkedMapP ();
    if (map)
        map->m_projectionScale.Init (x, y, z);
    else
        m_projectionScale.Init (x, y, z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerCollectionCR    MaterialMap::GetLayers () const
    {
    MaterialMapCP map = GetLinkedMapCP ();

    if (MAPTYPE_None != m_linkType && MAPTYPE_Geometry == m_type && map)
        {
        MaterialMapLayerCR  linkedLayer = map->GetLayers ().GetTopLayer ();
        MaterialMapLayerR   myLayer = m_layers.GetTopLayerR ();

        myLayer.SetMode (linkedLayer.GetMode ());
        myLayer.SetUnits (linkedLayer.GetUnits ());
        myLayer.SetRotation (linkedLayer.GetRotation ());
        myLayer.GetScaleR () = linkedLayer.GetScale ();
        myLayer.GetOffsetR () = linkedLayer.GetOffset ();
        return m_layers;
        }
    else if (map)
        return map->m_layers;
    else
        return m_layers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     09/11
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerCollectionR     MaterialMap::GetLayersR ()
    {
    MaterialMapP map = GetLinkedMapP ();

    if (MAPTYPE_None != m_linkType && MAPTYPE_Geometry == m_type && map)
        {
        MaterialMapLayerCR  linkedLayer = map->GetLayers ().GetTopLayer ();
        MaterialMapLayerR   myLayer = m_layers.GetTopLayerR ();

        myLayer.SetMode (linkedLayer.GetMode ());
        myLayer.SetUnits (linkedLayer.GetUnits ());
        myLayer.SetRotation (linkedLayer.GetRotation ());
        myLayer.GetScaleR () = linkedLayer.GetScale ();
        myLayer.GetOffsetR () = linkedLayer.GetOffset ();
        return m_layers;
        }
    else if (map)
        return map->m_layers;
    else
        return m_layers;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCollection::MaterialMapCollection ()
    {
    InitDefaults ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapCollection::InitDefaults ()
    {
    m_maps.clear ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialMapCollection::Equals (MaterialMapCollectionCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_maps.size () == m_maps.size ());
    for (MaterialMapPtrList::const_iterator rhsIter = rhs.m_maps.begin (), iter = m_maps.begin (); iter != m_maps.end (); ++rhsIter, ++iter)
        {
        RETURN_IF_FALSE (rhsIter->second->Equals (*iter->second.get ()));
        }

    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapCollection::Copy (MaterialMapCollectionCR rhs)
    {
    m_maps.clear ();
    FOR_EACH (MaterialMapPtrList::value_type const& pair , rhs.m_maps)
        {
        MaterialMapPtr map = new MaterialMap (pair.second->GetType (),*this);
        map->Copy (*pair.second.get ());
        m_maps.insert (MaterialMapPtrList::value_type (pair.second->GetType (), map));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCP MaterialMapCollection::GetMapCP (MaterialMap::MapType type) const
    {
    MaterialMapPtrList::const_iterator iter = m_maps.find (type);
    return (m_maps.end () == iter) ? NULL : iter->second.get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapP MaterialMapCollection::GetMapP (MaterialMap::MapType type)
    {
    return const_cast <MaterialMapP> (GetMapCP (type));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapCollection::DeleteMap (MaterialMap::MapType type)
    {
    MaterialMapPtrList::iterator iter = m_maps.find (type);
    if (iter != m_maps.end ())
        m_maps.erase (iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapP MaterialMapCollection::AddMap (MaterialMap::MapType type)
    {
    if (MaterialMap::MAPTYPE_None == type)
        return NULL;

    DeleteMap (type);

    MaterialMapPtr map = new MaterialMap (type, *this);
    m_maps.insert (MaterialMapPtrList::value_type (type, map));
    return map.get ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapCollection::const_iterator   MaterialMapCollection::begin () const   { return m_maps.begin (); }
MaterialMapCollection::iterator         MaterialMapCollection::begin ()         { return m_maps.begin (); }

MaterialMapCollection::const_iterator   MaterialMapCollection::end () const     { return m_maps.end (); }
MaterialMapCollection::iterator         MaterialMapCollection::end ()           { return m_maps.end (); }

size_t                                  MaterialMapCollection::Size () const    { return m_maps.size (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator::MaterialMapLayerIterator (MaterialMapLayerCollectionR layers, bool wantBegin)
    {
    MaterialMapLayerPtrList::iterator* iter = new MaterialMapLayerPtrList::iterator ();
    *iter = wantBegin ? layers.m_layers.begin () : layers.m_layers.end ();
    m_iter = iter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   PaulChater     1/11
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator::MaterialMapLayerIterator (MaterialMapLayerPtrList::iterator iter)
    {
    MaterialMapLayerPtrList::iterator* myIter = new MaterialMapLayerPtrList::iterator ();
    *myIter = iter;
    m_iter = myIter;
    }

MaterialMapLayerIterator::MaterialMapLayerIterator (MaterialMapLayerIterator const& cc)
    {
    m_iter = (cc.m_iter)? new MaterialMapLayerPtrList::iterator(*(MaterialMapLayerPtrList::iterator*)cc.m_iter): NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator::~MaterialMapLayerIterator ()
    {
    delete reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator& MaterialMapLayerIterator::operator++ ()
    {
    ++*reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator& MaterialMapLayerIterator::operator-- ()
    {
    --*reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerR MaterialMapLayerIterator::operator* ()
    {
    MaterialMapLayerPtrList::iterator* iter = reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter);
    return *(*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerP MaterialMapLayerIterator::operator-> ()
    {
    MaterialMapLayerPtrList::iterator* iter = reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter);
    return (*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialMapLayerIterator::operator== (MaterialMapLayerIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter)) == *(reinterpret_cast <MaterialMapLayerPtrList::iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialMapLayerIterator::operator!= (MaterialMapLayerIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter)) != *(reinterpret_cast <MaterialMapLayerPtrList::iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerPtrList::iterator MaterialMapLayerIterator::Get ()
    {
    return *reinterpret_cast <MaterialMapLayerPtrList::iterator *> (m_iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerConstIterator::MaterialMapLayerConstIterator (MaterialMapLayerCollectionCR layers, bool wantBegin)
    {
    MaterialMapLayerPtrList::const_iterator* iter = new MaterialMapLayerPtrList::const_iterator ();
    *iter = wantBegin ? layers.m_layers.begin () : layers.m_layers.end ();
    m_iter = iter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerConstIterator::~MaterialMapLayerConstIterator ()
    {
    delete reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerConstIterator& MaterialMapLayerConstIterator::operator++ ()
    {
    ++*reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerConstIterator& MaterialMapLayerConstIterator::operator-- ()
    {
    --*reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerCR MaterialMapLayerConstIterator::operator* () const
    {
    MaterialMapLayerPtrList::const_iterator* iter = reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter);
    return *(*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerCP MaterialMapLayerConstIterator::operator-> () const
    {
    MaterialMapLayerPtrList::const_iterator* iter = reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter);
    return (*iter)->get ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialMapLayerConstIterator::operator== (MaterialMapLayerConstIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter)) == *(reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
bool MaterialMapLayerConstIterator::operator!= (MaterialMapLayerConstIterator const& rhs) const
    {
    return *(reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter)) != *(reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (rhs.m_iter));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerPtrList::const_iterator MaterialMapLayerConstIterator::Get () const
    {
    return *reinterpret_cast <MaterialMapLayerPtrList::const_iterator *> (m_iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerCollection::MaterialMapLayerCollection ()
    {
    m_layers.push_back (MaterialMapLayerPtr (new MaterialMapLayer ()));
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayerCollection::InitDefaults ()
    {
    if (1 < m_layers.size ())
        {
        MaterialMapLayerPtrList::iterator iter = m_layers.begin ();
        m_layers.erase (++iter, m_layers.end ());
        }

    m_layers.back ()->InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                                      MaterialMapLayerCollection::Size () const           { return m_layers.size (); }

MaterialMapLayerCR                          MaterialMapLayerCollection::GetTopLayer () const    { return *m_layers.begin ()->get (); }
MaterialMapLayerR                           MaterialMapLayerCollection::GetTopLayerR ()         { return *m_layers.begin ()->get (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator MaterialMapLayerCollection::begin ()
    {
    return MaterialMapLayerIterator (*this, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerIterator MaterialMapLayerCollection::end ()
    {
    return MaterialMapLayerIterator (*this, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerConstIterator MaterialMapLayerCollection::begin () const
    {
    return MaterialMapLayerConstIterator (*this, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/10
//---------------------------------------------------------------------------------------
MaterialMapLayerConstIterator MaterialMapLayerCollection::end () const
    {
    return MaterialMapLayerConstIterator (*this, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayerCollection::Copy (MaterialMapLayerCollectionCR rhs)
    {
    m_layers.clear ();
    FOR_EACH (MaterialMapLayerPtr const& layer , rhs.m_layers)
        AddLayer ().Copy (*layer.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialMapLayerCollection::Equals (MaterialMapLayerCollectionCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_layers.size () == m_layers.size ());
    for (MaterialMapLayerPtrList::const_iterator rhsIter = rhs.m_layers.begin (), iter = m_layers.begin (); m_layers.end () != iter; ++iter, ++rhsIter)
        {
        RETURN_IF_FALSE ((*rhsIter)->Equals (*iter->get ()));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerR MaterialMapLayerCollection::AddLayer ()
    {
    m_layers.push_back (MaterialMapLayerPtr (new MaterialMapLayer ()));
    return *m_layers.back ().get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerR MaterialMapLayerCollection::AddLayerToFront ()
    {
    m_layers.insert (m_layers.begin (), MaterialMapLayerPtr (new MaterialMapLayer ()));
    return *m_layers.front ().get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerIterator MaterialMapLayerCollection::InsertLayer (MaterialMapLayerIterator& pos)
    {
    MaterialMapLayerPtrList::iterator iter  = m_layers.insert (pos.Get (), MaterialMapLayerPtr (new MaterialMapLayer ()));
    return MaterialMapLayerIterator (iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MaterialMapLayerCollection::DeleteLayer (MaterialMapLayerIterator& iter)
    {
    if (m_layers.end () == iter.Get ())
        return ERROR;

    if (1 == m_layers.size ())
        m_layers.back ()->InitDefaults ();
    else
        *reinterpret_cast <MaterialMapLayerPtrList::iterator *> (iter.m_iter) = m_layers.erase (iter.Get ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     1/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MaterialMapLayerCollection::DeleteLayers (MaterialMapLayerIterator& startIter, MaterialMapLayerIterator& endIter)
    {
    m_layers.erase (startIter.Get (), endIter.Get ());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     1/11
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayerCollection::DeleteAllLayers ()
    {
    m_layers.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     1/11
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayerCollection::InsertCollection (MaterialMapLayerCollectionR collection, MaterialMapLayerIterator& iter)
    {
    m_layers.insert (iter.Get (), collection.m_layers.begin (), collection.m_layers.end ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayer::MaterialMapLayer ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     1/12
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialMapLayerCR                          MaterialMapLayerCollection::GetLastDataLayer () const
    {
    if (1 == Size ())
        return GetTopLayer ();

    for (MaterialMapLayerPtrList::const_reverse_iterator iter = m_layers.rbegin (); m_layers.rend () != iter; ++iter)
        {
        switch (iter->get ()->GetType ())
            {
            case MaterialMapLayer::LAYERTYPE_Image    :
            case MaterialMapLayer::LAYERTYPE_Procedure:
            case MaterialMapLayer::LAYERTYPE_Gradient :
            case MaterialMapLayer::LAYERTYPE_LxoProcedure:
                return *iter->get ();
            }
        }

    return GetTopLayer ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayer::InitDefaults ()
    {
    memset (&m_basicFlags, 0, sizeof (m_basicFlags));
    m_basicFlags.m_antialiasing = true;
    m_enabled               = true;
    m_fileName              = L"";
    m_mode                  = MapMode::Parametric;
    m_units                 = MapUnits::Relative;
    m_rotation              = 0.0;
    m_scale.One ();
    m_offset.Zero ();
    m_opacity               = 1.0;
    m_gamma                 = 1.0;
    m_invert                = false;
    m_bgTrans               = false;
    m_lxoProcedure          = NULL;
    m_operatorString        = L"";
    m_operatorDouble        = 0.0;
    m_operatorColor.red     = m_operatorColor.green = m_operatorColor.blue = 0.0;
    m_type                  = LAYERTYPE_Image;
    m_textureFilterType     = TEXTUREFILTERTYPE_Bilinear;
    m_lowValue              = 0.0;
    m_highValue             = 100.0;
    m_antialiasStrength     = 100.0;
    m_minimumSpot           = 1.0;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialMapLayer::Equals (MaterialMapLayerCR rhs) const
    {
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_flipV          ==  m_basicFlags.m_flipV);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_lockSize       ==  m_basicFlags.m_lockSize);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_capped         ==  m_basicFlags.m_capped);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_lockProjection ==  m_basicFlags.m_lockProjection);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_flipU          ==  m_basicFlags.m_flipU);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_decalU         ==  m_basicFlags.m_decalU);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_decalV         ==  m_basicFlags.m_decalV);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_mirrorU        ==  m_basicFlags.m_mirrorU);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_mirrorV        ==  m_basicFlags.m_mirrorV);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_snappable      ==  m_basicFlags.m_snappable);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_useCellColors  ==  m_basicFlags.m_useCellColors);
    RETURN_IF_FALSE              (rhs.m_basicFlags.m_antialiasing   ==  m_basicFlags.m_antialiasing);
    RETURN_IF_FALSE              (rhs.m_enabled                     ==  m_enabled);
    RETURN_IF_FALSE              (rhs.m_mode                        ==  m_mode);
    RETURN_IF_FALSE              (rhs.m_units                       ==  m_units);
    RETURN_IF_FALSE              (rhs.m_invert                      ==  m_invert);
    RETURN_IF_FALSE              (rhs.m_bgTrans                     ==  m_bgTrans);
    RETURN_IF_FALSE              (rhs.m_type                        ==  m_type);
    RETURN_IF_FALSE              (rhs.m_textureFilterType           ==  m_textureFilterType);
    RETURN_IF_FALSE              (rhs.m_fileName.EqualsI                (m_fileName.c_str ()));
    RETURN_IF_FALSE              (rhs.m_operatorString.Equals           (m_operatorString.c_str ()));
    RETURN_IF_FALSE              (rhs.m_scale.IsEqual                   (m_scale,               1.0E-4));
    RETURN_IF_FALSE              (rhs.m_offset.IsEqual                  (m_offset,              1.0E-4));
    RETURN_IF_FALSE (valuesEqual (rhs.m_rotation,                       m_rotation));
    RETURN_IF_FALSE (valuesEqual (rhs.m_opacity,                        m_opacity));
    RETURN_IF_FALSE (valuesEqual (rhs.m_gamma,                          m_gamma));
    RETURN_IF_FALSE (valuesEqual (rhs.m_operatorDouble,                 m_operatorDouble));
    RETURN_IF_FALSE (valuesEqual (rhs.m_lowValue,                       m_lowValue));
    RETURN_IF_FALSE (valuesEqual (rhs.m_highValue,                      m_highValue));
    RETURN_IF_FALSE (valuesEqual (rhs.m_antialiasStrength,              m_antialiasStrength));
    RETURN_IF_FALSE (valuesEqual (rhs.m_minimumSpot,                    m_minimumSpot));
    RETURN_IF_FALSE (colorsEqual (rhs.m_operatorColor,                  m_operatorColor));

    if (m_lxoProcedure.IsNull ())
        {
        RETURN_IF_FALSE (rhs.m_lxoProcedure.IsNull ());
        }
    else
        {
        if (rhs.m_lxoProcedure.IsNull ())
            return false;

        RETURN_IF_FALSE (rhs.m_lxoProcedure->Equals (*m_lxoProcedure.get ()));
        }

    RETURN_IF_FALSE (rhs.m_legacyProcedureData.size () == m_legacyProcedureData.size ());
    for (bvector<WString>::const_iterator iter = m_legacyProcedureData.begin (), rhsIter = rhs.m_legacyProcedureData.begin (); m_legacyProcedureData.end () != iter; ++rhsIter, ++iter)
        {
        RETURN_IF_FALSE (rhsIter->EqualsI (iter->c_str ()));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayer::Copy (MaterialMapLayerCR rhs)
    {
    m_basicFlags            = rhs.m_basicFlags;
    m_enabled               = rhs.m_enabled;
    m_fileName              = rhs.m_fileName;
    m_mode                  = rhs.m_mode;
    m_units                 = rhs.m_units;
    m_rotation              = rhs.m_rotation;
    m_scale                 = rhs.m_scale;
    m_offset                = rhs.m_offset;
    m_opacity               = rhs.m_opacity;
    m_gamma                 = rhs.m_gamma;
    m_invert                = rhs.m_invert;
    m_bgTrans               = rhs.m_bgTrans;
    m_type                  = rhs.m_type;
    m_operatorDouble        = rhs.m_operatorDouble;
    m_operatorColor         = rhs.m_operatorColor;
    m_operatorString        = rhs.m_operatorString;
    m_legacyProcedureData   = rhs.m_legacyProcedureData;
    m_textureFilterType     = rhs.m_textureFilterType;
    m_lowValue              = rhs.m_lowValue;
    m_highValue             = rhs.m_highValue;
    m_antialiasStrength     = rhs.m_antialiasStrength;
    m_minimumSpot           = rhs.m_minimumSpot;

    if (rhs.m_lxoProcedure.IsNull ())
        m_lxoProcedure = NULL;
    else
        {
        AddLxoProcedure (rhs.m_lxoProcedure->GetType ());
        m_lxoProcedure->Copy (*rhs.m_lxoProcedure.get ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t const*                           MaterialMapLayer::GetBasicFlagsCP () const                              { return (uint32_t const *) &m_basicFlags; }
void                                    MaterialMapLayer::SetBasicFlags (uint32_t flags)                          { memcpy (&m_basicFlags, &flags, sizeof (flags)); }

MaterialMapLayer::LegacyProcedureDataCR MaterialMapLayer::GetLegacyProcedureData () const                       { return m_legacyProcedureData; }
MaterialMapLayer::LegacyProcedureDataR  MaterialMapLayer::GetLegacyProcedureDataR ()                            { return m_legacyProcedureData; }

bool                                    MaterialMapLayer::IsEnabled () const                                    { return m_enabled; }
void                                    MaterialMapLayer::SetIsEnabled (bool isEnabled)                         { m_enabled = isEnabled; }

bool                                    MaterialMapLayer::GetFlipU () const                                     { return m_basicFlags.m_flipU; }
void                                    MaterialMapLayer::SetFlipU (bool flip)                                  { m_basicFlags.m_flipU = flip; }

bool                                    MaterialMapLayer::GetFlipV () const                                     { return m_basicFlags.m_flipV; }
void                                    MaterialMapLayer::SetFlipV (bool flip)                                  { m_basicFlags.m_flipV = flip; }

bool                                    MaterialMapLayer::GetMirrorU () const                                   { return m_basicFlags.m_mirrorU; }
void                                    MaterialMapLayer::SetMirrorU (bool mirror)                              { m_basicFlags.m_mirrorU = mirror; }

bool                                    MaterialMapLayer::GetMirrorV () const                                   { return m_basicFlags.m_mirrorV; }
void                                    MaterialMapLayer::SetMirrorV (bool mirror)                              { m_basicFlags.m_mirrorV = mirror; }

bool                                    MaterialMapLayer::GetRepeatU () const                                   { return !m_basicFlags.m_decalU; }
void                                    MaterialMapLayer::SetRepeatU (bool repeat)                              { m_basicFlags.m_decalU = !repeat; }

bool                                    MaterialMapLayer::GetRepeatV () const                                   { return !m_basicFlags.m_decalV; }
void                                    MaterialMapLayer::SetRepeatV (bool repeat)                              { m_basicFlags.m_decalV = !repeat; }

bool                                    MaterialMapLayer::GetLockSize () const                                  { return m_basicFlags.m_lockSize; }
void                                    MaterialMapLayer::SetLockSize (bool lockSize)                           { m_basicFlags.m_lockSize = lockSize; }

bool                                    MaterialMapLayer::IsCapped () const                                     { return m_basicFlags.m_capped; }
void                                    MaterialMapLayer::SetIsCapped (bool isCapped)                           { m_basicFlags.m_capped = isCapped; }

bool                                    MaterialMapLayer::IsProjectionLocked () const                           { return m_basicFlags.m_lockProjection; }
void                                    MaterialMapLayer::SetIsProjectionLocked (bool isLocked)                 { m_basicFlags.m_lockProjection = isLocked; }

bool                                    MaterialMapLayer::UseCellColors () const                                { return m_basicFlags.m_useCellColors;}
void                                    MaterialMapLayer::SetUseCellColors (bool useCellColors)                 { m_basicFlags.m_useCellColors = useCellColors;}

bool                                    MaterialMapLayer::IsSnappable () const                                  { return m_basicFlags.m_snappable;}
void                                    MaterialMapLayer::SetIsSnappable (bool isSnappable)                     { m_basicFlags.m_snappable = isSnappable;}

bool                                    MaterialMapLayer::IsAntialiasing () const                               { return m_basicFlags.m_antialiasing;}
void                                    MaterialMapLayer::SetIsAntialiasing (bool antialiasing)                 { m_basicFlags.m_antialiasing = antialiasing;}

WStringCR                               MaterialMapLayer::GetFileName () const                                  { return m_fileName; }
void                                    MaterialMapLayer::SetFileName (WCharCP fileName)                        { m_fileName.assign (fileName); }

MapMode                                 MaterialMapLayer::GetMode () const                                      { return m_mode; }
void                                    MaterialMapLayer::SetMode (MapMode mode)                                { m_mode = mode; }

MapUnits                                MaterialMapLayer::GetUnits () const                                     { return m_units; }
void                                    MaterialMapLayer::SetUnits (MapUnits units)                             { m_units = units; }

double                                  MaterialMapLayer::GetRotation () const                                  { return m_rotation; }
void                                    MaterialMapLayer::SetRotation (double rotation)                         { m_rotation = rotation; }

DPoint3dCR                              MaterialMapLayer::GetScale () const                                     { return m_scale; }
DPoint3dR                               MaterialMapLayer::GetScaleR ()                                          { return m_scale; }
void                                    MaterialMapLayer::SetScale (double x, double y, double z)               { m_scale.Init (x, y, z); }

DPoint3dCR                              MaterialMapLayer::GetOffset () const                                    { return m_offset; }
DPoint3dR                               MaterialMapLayer::GetOffsetR ()                                         { return m_offset; }
void                                    MaterialMapLayer::SetOffset (double x, double y, double z)              { m_offset.Init (x, y, z); }

double                                  MaterialMapLayer::GetOpacity () const                                   { return m_opacity; }
void                                    MaterialMapLayer::SetOpacity (double opacity)                           { m_opacity = opacity; }

double                                  MaterialMapLayer::GetGamma () const                                     { return m_gamma; }
void                                    MaterialMapLayer::SetGamma (double gamma)                               { m_gamma = gamma; }

bool                                    MaterialMapLayer::IsInverted () const                                   { return m_invert; }
void                                    MaterialMapLayer::SetIsInverted (bool isInverted)                       { m_invert = isInverted; }

bool                                    MaterialMapLayer::IsBackgroundTransparent () const                      { return m_bgTrans; }
void                                    MaterialMapLayer::SetIsBackgroundTransparent (bool isTransparent)       { m_bgTrans = isTransparent; }

double                                  MaterialMapLayer::GetLowValue () const                                  { return m_lowValue;}
void                                    MaterialMapLayer::SetLowValue (double lowValue)                         { m_lowValue = lowValue;}

double                                  MaterialMapLayer::GetHighValue () const                                 { return m_highValue;}
void                                    MaterialMapLayer::SetHighValue (double highValue)                       { m_highValue = highValue;}

double                                  MaterialMapLayer::GetAntiAliasStrength () const                         { return m_antialiasStrength;}
void                                    MaterialMapLayer::SetAntiAliasStrength (double antiAliasStrength)       { m_antialiasStrength = antiAliasStrength;}

double                                  MaterialMapLayer::GetMinimumSpot () const                               { return m_minimumSpot;}
void                                    MaterialMapLayer::SetMinimumSpot (double minimumSpot)                   { m_minimumSpot = minimumSpot;}

MaterialMapLayer::TextureFilterType     MaterialMapLayer::GetTextureFilterType () const                         { return m_textureFilterType;}
void                                    MaterialMapLayer::SetTextureFilterType (TextureFilterType type)         { m_textureFilterType = type;}

MaterialMapLayer::LayerType             MaterialMapLayer::GetType () const                                      { return m_type; }
void                                    MaterialMapLayer::SetType (MaterialMapLayer::LayerType type)            { m_type = type; }

double                                  MaterialMapLayer::GetOperatorDoubleValue () const                       { return m_operatorDouble; }
void                                    MaterialMapLayer::SetOperatorDoubleValue (double value)                 { m_operatorDouble = value; }

RgbFactor const&                        MaterialMapLayer::GetOperatorColorValue () const                        { return m_operatorColor; }
RgbFactor&                              MaterialMapLayer::GetOperatorColorValueR ()                             { return m_operatorColor; }
void                                    MaterialMapLayer::SetOperatorColorValue (double r, double g, double b)  { m_operatorColor.red = r; m_operatorColor.green = g; m_operatorColor.blue = b; }

WStringCR                               MaterialMapLayer::GetOperatorStringValue () const                       { return m_operatorString; }
void                                    MaterialMapLayer::SetOperatorStringValue (WCharCP value)              { m_operatorString = value; }

LxoProcedureCP                          MaterialMapLayer::GetLxoProcedureCP () const                            { return m_lxoProcedure.get (); }
LxoProcedureP                           MaterialMapLayer::GetLxoProcedureP ()                                   { return m_lxoProcedure.get (); }

void                                    MaterialMapLayer::ClearLxoProcedure ()                                  { m_lxoProcedure = NULL; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedurePtr LxoProcedure::Create (ProcedureType type)
    {
    switch (type)
        {
        case LxoProcedure::PROCEDURETYPE_Noise:
            return new LxoNoiseProcedure ();
        case LxoProcedure::PROCEDURETYPE_Checker:
            return new LxoCheckerProcedure ();
        case LxoProcedure::PROCEDURETYPE_Grid:
            return new LxoGridProcedure ();
        case LxoProcedure::PROCEDURETYPE_Dot:
            return new LxoDotProcedure ();
        case LxoProcedure::PROCEDURETYPE_Constant:
            return new LxoConstantProcedure ();
        case LxoProcedure::PROCEDURETYPE_Cellular:
            return new LxoCellularProcedure ();
        case LxoProcedure::PROCEDURETYPE_Wood:
            return new LxoWoodProcedure ();
        case LxoProcedure::PROCEDURETYPE_Weave:
            return new LxoWeaveProcedure ();
        case LxoProcedure::PROCEDURETYPE_Ripples:
            return  new LxoRipplesProcedure ();
        case LxoProcedure::PROCEDURETYPE_Gradient:
            return new LxoGradientProcedure ();
        case LxoProcedure::PROCEDURETYPE_Boards :
            return new LxoBoardsProcedure ();
        case LxoProcedure::PROCEDURETYPE_Brick :
            return new LxoBrickProcedure ();
        case LxoProcedure::PROCEDURETYPE_BWNoise :
            return new LxoBWNoiseProcedure ();
        case LxoProcedure::PROCEDURETYPE_BentleyChecker :
            return new LxoBentleyCheckerProcedure ();
        case LxoProcedure::PROCEDURETYPE_Checker3d :
            return new LxoChecker3dProcedure ();
        case LxoProcedure::PROCEDURETYPE_Clouds :
            return new LxoCloudsProcedure ();
        case LxoProcedure::PROCEDURETYPE_ColorNoise :
            return new LxoColorNoiseProcedure ();
        case LxoProcedure::PROCEDURETYPE_Flame :
            return new LxoFlameProcedure ();
        case LxoProcedure::PROCEDURETYPE_Fog :
            return new LxoFogProcedure ();
        case LxoProcedure::PROCEDURETYPE_Marble :
            return new LxoMarbleProcedure ();
        case LxoProcedure::PROCEDURETYPE_RGBColorCube :
            return new LxoRGBColorCubeProcedure ();
        case LxoProcedure::PROCEDURETYPE_Sand :
            return new LxoSandProcedure ();
        case LxoProcedure::PROCEDURETYPE_Stone :
            return new LxoStoneProcedure ();
        case LxoProcedure::PROCEDURETYPE_Turbulence :
            return new LxoTurbulenceProcedure ();
        case LxoProcedure::PROCEDURETYPE_Turf :
            return new LxoTurfProcedure ();
        case LxoProcedure::PROCEDURETYPE_Water :
            return new LxoWaterProcedure ();
        case LxoProcedure::PROCEDURETYPE_Waves :
            return new LxoWavesProcedure ();
        case LxoProcedure::PROCEDURETYPE_BentleyWood :
            return new LxoBentleyWoodProcedure ();
        case LxoProcedure::PROCEDURETYPE_AdvancedWood :
            return new LxoAdvancedWoodProcedure ();
        case LxoProcedure::PROCEDURETYPE_Occlusion :
            return new LxoOcclusionProcedure ();

        default:
            return NULL;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedureP MaterialMapLayer::AddLxoProcedure (LxoProcedure::ProcedureType type)
    {
    return (m_lxoProcedure = LxoProcedure::Create (type)).get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayer::GetAdjustedOffset (DPoint3dR offset) const
    {
    if (MapUnits::Relative == GetUnits ())
        offset.Init (-m_offset.x, m_offset.y, m_offset.z);
    else
        offset.Init (-m_offset.x * m_scale.x, m_offset.y * m_scale.y, m_offset.z * m_scale.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayer::SetAdjustedOffset (double x, double y, double z)
    {
    if (MapUnits::Relative == GetUnits ())
        {
        m_offset.x = -x;
        m_offset.y = y;
        m_offset.z = z;
        }
    else
        {
        m_offset.x = 0.0 == m_scale.x ? 0.0 : -x / m_scale.x;
        m_offset.y = 0.0 == m_scale.y ? 0.0 : y / m_scale.y;
        m_offset.z = 0.0 == m_scale.z ? 0.0 : z / m_scale.z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void MaterialMapLayer::SetAdjustedOffset (DPoint3dCR offset)
    {
    SetAdjustedOffset (offset.x, offset.y, offset.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
                            LxoProcedure::~LxoProcedure ()                                  { }
LxoProcedure::ProcedureType LxoProcedure::GetType () const                                  { return _GetType (); }
void                        LxoProcedure::InitDefaults ()                                   { _InitDefaults (); }
BentleyStatus               LxoProcedure::Copy (LxoProcedureCR rhs)                         { return _Copy (rhs); }
bool                        LxoProcedure::Equals (LxoProcedureCR rhs) const                 { return _Equals (rhs); }
void                        LxoProcedure::GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) {_GetSymbolsForPublishing (variableList, pointerList);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoNoiseProcedure::LxoNoiseProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoNoiseProcedure::_GetType () const { return PROCEDURETYPE_Noise; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoNoiseProcedure::_InitDefaults ()
    {
    m_color1.red = m_color1.green = m_color1.blue = 0.0;
    m_color2.red = m_color2.green = m_color2.blue = 1.0;

    m_alpha1            = 100.0;
    m_alpha2            = 100.0;
    m_frequencies       = 4;
    m_frequencyRatio    = 2.0;
    m_amplitudeRatio    = 0.5;
    m_bias              = 50.0;
    m_gain              = 50.0;
    m_noiseType         = NOISETYPE_Simple;
    m_value1            = 0.0;
    m_value2            = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoNoiseProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Noise != rhs.GetType ())
        return ERROR;

    LxoNoiseProcedureCP castRhs = reinterpret_cast <LxoNoiseProcedureCP> (&rhs);

    m_color1            = castRhs->m_color1;
    m_alpha1            = castRhs->m_alpha1;
    m_color2            = castRhs->m_color2;
    m_alpha2            = castRhs->m_alpha2;
    m_frequencies       = castRhs->m_frequencies;
    m_frequencyRatio    = castRhs->m_frequencyRatio;
    m_amplitudeRatio    = castRhs->m_amplitudeRatio;
    m_bias              = castRhs->m_bias;
    m_gain              = castRhs->m_gain;
    m_value1            = castRhs->m_value1;
    m_value2            = castRhs->m_value2;
    m_noiseType         = castRhs->m_noiseType;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoNoiseProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Noise != rhs.GetType ())
        return false;

    LxoNoiseProcedureCP castRhs = reinterpret_cast <LxoNoiseProcedureCP> (&rhs);

    RETURN_IF_FALSE                 (castRhs->m_frequencies ==  m_frequencies);
    RETURN_IF_FALSE                 (castRhs->m_noiseType   ==  m_noiseType);
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_color1,         m_color1));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_alpha1,         m_alpha1));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_color2,         m_color2));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_alpha2,         m_alpha2));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_frequencyRatio, m_frequencyRatio));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_amplitudeRatio, m_amplitudeRatio));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,           m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,           m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_value1,         m_value1));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_value2,         m_value2));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoNoiseProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseAlpha1",              (uintptr_t)&m_alpha1));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseAlpha2",              (uintptr_t)&m_alpha2));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseFreqRatio",           (uintptr_t)&m_frequencyRatio));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseAmplitudeRatio",      (uintptr_t)&m_amplitudeRatio));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseBias",                (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseGain",                (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoNoiseFreq",                (uintptr_t)&m_frequencies));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoNoiseType",                (uintptr_t)&m_noiseType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseValue1",              (uintptr_t)&m_value1));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseValue2",              (uintptr_t)&m_value2));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseColor1",              (uintptr_t)&m_color1));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoNoiseColor2",              (uintptr_t)&m_color2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&                LxoNoiseProcedure::GetColor1 () const                               { return m_color1; }
RgbFactor&                      LxoNoiseProcedure::GetColor1R ()                                    { return m_color1; }
void                            LxoNoiseProcedure::SetColor1 (double r, double g, double b)         { m_color1.red = r; m_color1.green = g; m_color1.blue = b; }

RgbFactor const&                LxoNoiseProcedure::GetColor2 () const                               { return m_color2; }
RgbFactor&                      LxoNoiseProcedure::GetColor2R ()                                    { return m_color2; }
void                            LxoNoiseProcedure::SetColor2 (double r, double g, double b)         { m_color2.red = r; m_color2.green = g; m_color2.blue = b; }

double                          LxoNoiseProcedure::GetAlpha1 () const                               { return m_alpha1; }
void                            LxoNoiseProcedure::SetAlpha1 (double alpha)                         { m_alpha1 = alpha; }

double                          LxoNoiseProcedure::GetAlpha2 () const                               { return m_alpha2; }
void                            LxoNoiseProcedure::SetAlpha2 (double alpha)                         { m_alpha2 = alpha; }

uint32_t                        LxoNoiseProcedure::GetFrequencies () const                          { return m_frequencies; }
void                            LxoNoiseProcedure::SetFrequencies (uint32_t frequencies)              { m_frequencies = frequencies; }

double                          LxoNoiseProcedure::GetFrequencyRatio () const                       { return m_frequencyRatio; }
void                            LxoNoiseProcedure::SetFrequencyRatio (double ratio)                 { m_frequencyRatio = ratio; }

double                          LxoNoiseProcedure::GetAmplitudeRatio () const                       { return m_amplitudeRatio; }
void                            LxoNoiseProcedure::SetAmplitudeRatio (double ratio)                 { m_amplitudeRatio = ratio; }

double                          LxoNoiseProcedure::GetBias () const                                 { return m_bias; }
void                            LxoNoiseProcedure::SetBias (double bias)                            { m_bias = bias; }

double                          LxoNoiseProcedure::GetGain () const                                 { return m_gain; }
void                            LxoNoiseProcedure::SetGain (double gain)                            { m_gain = gain; }

double                          LxoNoiseProcedure::GetValue1 () const                               { return m_value1; }
void                            LxoNoiseProcedure::SetValue1 (double value1)                        { m_value1 = value1; }

double                          LxoNoiseProcedure::GetValue2 () const                               { return m_value2; }
void                            LxoNoiseProcedure::SetValue2 (double value2)                        { m_value2 = value2; }

LxoNoiseProcedure::NoiseType    LxoNoiseProcedure::GetNoiseType () const                            { return m_noiseType; }
void                            LxoNoiseProcedure::SetNoiseType (LxoNoiseProcedure::NoiseType type) { m_noiseType = type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoCheckerProcedure::LxoCheckerProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoCheckerProcedure::_GetType () const { return PROCEDURETYPE_Checker; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoCheckerProcedure::_InitDefaults ()
    {
    m_color1.red = m_color1.green = m_color1.blue = 0.0;
    m_color2.red = m_color2.green = m_color2.blue = 1.0;

    m_alpha1            = 100.0;
    m_alpha2            = 100.0;
    m_transitionWidth   = 10.0;
    m_bias              = 50.0;
    m_gain              = 50.0;
    m_checkerType       = CHECKERTYPE_Square;
    m_value1            = 0.0;
    m_value2            = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoCheckerProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Checker != rhs.GetType ())
        return ERROR;

    LxoCheckerProcedureCP castRhs = reinterpret_cast <LxoCheckerProcedureCP> (&rhs);

    m_color1            = castRhs->m_color1;
    m_color2            = castRhs->m_color2;
    m_alpha1            = castRhs->m_alpha1;
    m_alpha2            = castRhs->m_alpha2;
    m_transitionWidth   = castRhs->m_transitionWidth;
    m_bias              = castRhs->m_bias;
    m_gain              = castRhs->m_gain;
    m_checkerType       = castRhs->m_checkerType;
    m_value1            = castRhs->m_value1;
    m_value2            = castRhs->m_value2;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoCheckerProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Checker != rhs.GetType ())
        return false;

    LxoCheckerProcedureCP castRhs = reinterpret_cast <LxoCheckerProcedureCP> (&rhs);

    RETURN_IF_FALSE                 (castRhs->m_checkerType ==      m_checkerType);
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_color1,             m_color1));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_color2,             m_color2));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_alpha1,             m_alpha1));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_alpha2,             m_alpha2));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_transitionWidth,    m_transitionWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,               m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,               m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_value1,             m_value1));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_value2,             m_value2));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoCheckerProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerAlpha1",              (uintptr_t)&m_alpha1));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerAlpha2",              (uintptr_t)&m_alpha2));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerTransitionWidth",     (uintptr_t)&m_transitionWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerBias",                (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerGain",                (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoCheckerType",                (uintptr_t)&m_checkerType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerValue1",              (uintptr_t)&m_value1));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerValue2",              (uintptr_t)&m_value2));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerColor1",              (uintptr_t)&m_color1));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCheckerColor2",              (uintptr_t)&m_color2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&                    LxoCheckerProcedure::GetColor1 () const                                     { return m_color1; }
RgbFactor&                          LxoCheckerProcedure::GetColor1R ()                                          { return m_color1; }
void                                LxoCheckerProcedure::SetColor1 (double r, double g, double b)               { m_color1.red = r; m_color1.green = g; m_color1.blue = b; }

RgbFactor const&                    LxoCheckerProcedure::GetColor2 () const                                     { return m_color2; }
RgbFactor&                          LxoCheckerProcedure::GetColor2R ()                                          { return m_color2; }
void                                LxoCheckerProcedure::SetColor2 (double r, double g, double b)               { m_color2.red = r; m_color2.green = g; m_color2.blue = b; }

double                              LxoCheckerProcedure::GetAlpha1 () const                                     { return m_alpha1; }
void                                LxoCheckerProcedure::SetAlpha1 (double alpha)                               { m_alpha1 = alpha; }

double                              LxoCheckerProcedure::GetAlpha2 () const                                     { return m_alpha2; }
void                                LxoCheckerProcedure::SetAlpha2 (double alpha)                               { m_alpha2 = alpha; }

double                              LxoCheckerProcedure::GetTransitionWidth () const                            { return m_transitionWidth; }
void                                LxoCheckerProcedure::SetTransitionWidth (double width)                      { m_transitionWidth = width; }

double                              LxoCheckerProcedure::GetBias () const                                       { return m_bias; }
void                                LxoCheckerProcedure::SetBias (double bias)                                  { m_bias = bias; }

double                              LxoCheckerProcedure::GetGain () const                                       { return m_gain; }
void                                LxoCheckerProcedure::SetGain (double gain)                                  { m_gain = gain; }

double                              LxoCheckerProcedure::GetValue1 () const                                     { return m_value1; }
void                                LxoCheckerProcedure::SetValue1 (double value1)                              { m_value1 = value1; }

double                              LxoCheckerProcedure::GetValue2 () const                                     { return m_value2; }
void                                LxoCheckerProcedure::SetValue2 (double value2)                              { m_value2 = value2; }

LxoCheckerProcedure::CheckerType    LxoCheckerProcedure::GetCheckerType () const                                { return m_checkerType; }
void                                LxoCheckerProcedure::SetCheckerType (LxoCheckerProcedure::CheckerType type) { m_checkerType = type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoGridProcedure::LxoGridProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoGridProcedure::_GetType () const { return PROCEDURETYPE_Grid; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoGridProcedure::_InitDefaults ()
    {
    m_lineColor.red   = m_lineColor.green   = m_lineColor.blue   = 0.0;
    m_fillerColor.red = m_fillerColor.green = m_fillerColor.blue = 1.0;

    m_lineAlpha         = 100.0;
    m_fillerAlpha       = 100.0;
    m_transitionWidth   = 10.0;
    m_lineWidth         = 10.0;
    m_bias              = 50.0;
    m_gain              = 50.0;
    m_gridType          = GRIDTYPE_Square;
    m_lineValue         = 0.0;
    m_fillerValue       = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoGridProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Grid != rhs.GetType ())
        return ERROR;

    LxoGridProcedureCP castRhs = reinterpret_cast <LxoGridProcedureCP> (&rhs);

    m_lineColor         = castRhs->m_lineColor;
    m_fillerColor       = castRhs->m_fillerColor;
    m_lineAlpha         = castRhs->m_lineAlpha;
    m_fillerAlpha       = castRhs->m_fillerAlpha;
    m_transitionWidth   = castRhs->m_transitionWidth;
    m_lineWidth         = castRhs->m_lineWidth;
    m_bias              = castRhs->m_bias;
    m_gain              = castRhs->m_gain;
    m_gridType          = castRhs->m_gridType;
    m_lineValue         = castRhs->m_lineValue;
    m_fillerValue       = castRhs->m_fillerValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoGridProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Grid != rhs.GetType ())
        return false;

    LxoGridProcedureCP castRhs = reinterpret_cast <LxoGridProcedureCP> (&rhs);

    RETURN_IF_FALSE                 (castRhs->m_gridType ==         m_gridType);
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_lineColor,          m_lineColor));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_fillerColor,        m_fillerColor));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_lineAlpha,          m_lineAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerAlpha,        m_fillerAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_transitionWidth,    m_transitionWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_lineWidth,          m_lineWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,               m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,               m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_lineValue,          m_lineValue));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerValue,        m_fillerValue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoGridProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridLineAlpha",           (uintptr_t)&m_lineAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridFillerAlpha",         (uintptr_t)&m_fillerAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridTransitionWidth",     (uintptr_t)&m_transitionWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridBias",                (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridGain",                (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridLineWidth",           (uintptr_t)&m_lineWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoGridType",                (uintptr_t)&m_gridType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridLineValue",           (uintptr_t)&m_lineValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridFillerValue",         (uintptr_t)&m_fillerValue));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridLineColor",           (uintptr_t)&m_lineColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoGridFillerColor",         (uintptr_t)&m_fillerColor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&            LxoGridProcedure::GetLineColor () const                         { return m_lineColor; }
RgbFactor&                  LxoGridProcedure::GetLineColorR ()                              { return m_lineColor; }
void                        LxoGridProcedure::SetLineColor (double r, double g, double b)   { m_lineColor.red = r; m_lineColor.green = g; m_lineColor.blue = b; }

RgbFactor const&            LxoGridProcedure::GetFillerColor () const                       { return m_fillerColor; }
RgbFactor&                  LxoGridProcedure::GetFillerColorR ()                            { return m_fillerColor; }
void                        LxoGridProcedure::SetFillerColor (double r, double g, double b) { m_fillerColor.red = r; m_fillerColor.green = g; m_fillerColor.blue = b; }

double                      LxoGridProcedure::GetLineAlpha () const                         { return m_lineAlpha; }
void                        LxoGridProcedure::SetLineAlpha (double alpha)                   { m_lineAlpha = alpha; }

double                      LxoGridProcedure::GetFillerAlpha () const                       { return m_fillerAlpha; }
void                        LxoGridProcedure::SetFillerAlpha (double alpha)                 { m_fillerAlpha = alpha; }

double                      LxoGridProcedure::GetLineWidth () const                         { return m_lineWidth; }
void                        LxoGridProcedure::SetLineWidth (double width)                   { m_lineWidth = width; }

double                      LxoGridProcedure::GetTransitionWidth () const                   { return m_transitionWidth; }
void                        LxoGridProcedure::SetTransitionWidth (double width)             { m_transitionWidth = width; }

double                      LxoGridProcedure::GetBias () const                              { return m_bias; }
void                        LxoGridProcedure::SetBias (double bias)                         { m_bias = bias; }

double                      LxoGridProcedure::GetGain () const                              { return m_gain; }
void                        LxoGridProcedure::SetGain (double gain)                         { m_gain = gain; }

double                      LxoGridProcedure::GetLineValue () const                         { return m_lineValue; }
void                        LxoGridProcedure::SetLineValue (double value)                   { m_lineValue = value; }

double                      LxoGridProcedure::GetFillerValue () const                       { return m_fillerValue; }
void                        LxoGridProcedure::SetFillerValue (double value)                 { m_fillerValue = value; }

LxoGridProcedure::GridType  LxoGridProcedure::GetGridType () const                          { return m_gridType; }
void                        LxoGridProcedure::SetGridType (LxoGridProcedure::GridType type) { m_gridType = type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoDotProcedure::LxoDotProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoDotProcedure::_GetType () const { return PROCEDURETYPE_Dot; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoDotProcedure::_InitDefaults ()
    {
    m_dotColor.red    = m_dotColor.green    = m_dotColor.blue    = 0.0;
    m_fillerColor.red = m_fillerColor.green = m_fillerColor.blue = 1.0;

    m_dotAlpha          = 100.0;
    m_fillerAlpha       = 100.0;
    m_transitionWidth   = 10.0;
    m_dotWidth          = 80.0;
    m_bias              = 50.0;
    m_gain              = 50.0;
    m_dotType           = DOTTYPE_Square;
    m_dotValue          = 0.0;
    m_fillerValue       = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoDotProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Dot != rhs.GetType ())
        return ERROR;

    LxoDotProcedureCP castRhs = reinterpret_cast <LxoDotProcedureCP> (&rhs);

    m_dotColor          = castRhs->m_dotColor;
    m_fillerColor       = castRhs->m_fillerColor;
    m_dotAlpha          = castRhs->m_dotAlpha;
    m_fillerAlpha       = castRhs->m_fillerAlpha;
    m_transitionWidth   = castRhs->m_transitionWidth;
    m_dotWidth          = castRhs->m_dotWidth;
    m_bias              = castRhs->m_bias;
    m_gain              = castRhs->m_gain;
    m_dotType           = castRhs->m_dotType;
    m_dotValue          = castRhs->m_dotValue;
    m_fillerValue       = castRhs->m_fillerValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoDotProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Dot != rhs.GetType ())
        return false;

    LxoDotProcedureCP castRhs = reinterpret_cast <LxoDotProcedureCP> (&rhs);

    RETURN_IF_FALSE                 (castRhs->m_dotType ==          m_dotType);
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_dotColor,           m_dotColor));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_fillerColor,        m_fillerColor));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_dotAlpha,           m_dotAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerAlpha,        m_fillerAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_transitionWidth,    m_transitionWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_dotWidth,           m_dotWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,               m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,               m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_dotValue,           m_dotValue));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerValue,        m_fillerValue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoDotProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {

    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotAlpha",               (uintptr_t)&m_dotAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotFillerAlpha",         (uintptr_t)&m_fillerAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotTransitionWidth",     (uintptr_t)&m_transitionWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotBias",                (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotGain",                (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotWidth",               (uintptr_t)&m_dotWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoDotType",                (uintptr_t)&m_dotType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotValue",               (uintptr_t)&m_dotValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotFillerValue",         (uintptr_t)&m_fillerValue));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotColor",               (uintptr_t)&m_dotColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoDotFillerColor",         (uintptr_t)&m_fillerColor));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&            LxoDotProcedure::GetDotColor () const                           { return m_dotColor; }
RgbFactor&                  LxoDotProcedure::GetDotColorR ()                                { return m_dotColor; }
void                        LxoDotProcedure::SetDotColor (double r, double g, double b)     { m_dotColor.red = r; m_dotColor.green = g; m_dotColor.blue = b; }

RgbFactor const&            LxoDotProcedure::GetFillerColor () const                        { return m_fillerColor; }
RgbFactor&                  LxoDotProcedure::GetFillerColorR ()                             { return m_fillerColor; }
void                        LxoDotProcedure::SetFillerColor (double r, double g, double b)  { m_fillerColor.red = r; m_fillerColor.green = g; m_fillerColor.blue = b; }

double                      LxoDotProcedure::GetDotAlpha () const                           { return m_dotAlpha; }
void                        LxoDotProcedure::SetDotAlpha (double alpha)                     { m_dotAlpha = alpha; }

double                      LxoDotProcedure::GetFillerAlpha () const                        { return m_fillerAlpha; }
void                        LxoDotProcedure::SetFillerAlpha (double alpha)                  { m_fillerAlpha = alpha; }

double                      LxoDotProcedure::GetDotWidth () const                           { return m_dotWidth; }
void                        LxoDotProcedure::SetDotWidth (double width)                     { m_dotWidth = width; }

double                      LxoDotProcedure::GetTransitionWidth () const                    { return m_transitionWidth; }
void                        LxoDotProcedure::SetTransitionWidth (double width)              { m_transitionWidth = width; }

double                      LxoDotProcedure::GetBias () const                               { return m_bias; }
void                        LxoDotProcedure::SetBias (double bias)                          { m_bias = bias; }

double                      LxoDotProcedure::GetGain () const                               { return m_gain; }
void                        LxoDotProcedure::SetGain (double gain)                          { m_gain = gain; }

double                      LxoDotProcedure::GetDotValue () const                           { return m_dotValue; }
void                        LxoDotProcedure::SetDotValue (double value)                     { m_dotValue = value; }

double                      LxoDotProcedure::GetFillerValue () const                        { return m_fillerValue; }
void                        LxoDotProcedure::SetFillerValue (double value)                  { m_fillerValue = value; }

LxoDotProcedure::DotType    LxoDotProcedure::GetDotType () const                            { return m_dotType; }
void                        LxoDotProcedure::SetDotType (LxoDotProcedure::DotType type)     { m_dotType = type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoConstantProcedure::LxoConstantProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoConstantProcedure::_GetType () const { return PROCEDURETYPE_Constant; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoConstantProcedure::_InitDefaults ()
    {
    m_color.red = m_color.green = m_color.blue = 0.0;
    m_value     = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoConstantProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Constant != rhs.GetType ())
        return ERROR;

    LxoConstantProcedureCP castRhs = reinterpret_cast <LxoConstantProcedureCP> (&rhs);

    m_color = castRhs->m_color;
    m_value = castRhs->m_value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoConstantProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Constant != rhs.GetType ())
        return false;

    LxoConstantProcedureCP castRhs = reinterpret_cast <LxoConstantProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual    (castRhs->m_color,  m_color));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_value,  m_value));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoConstantProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoConstantValue",          (uintptr_t)&m_value));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoConstantColor",          (uintptr_t)&m_color));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&    LxoConstantProcedure::GetColor () const                         { return m_color; }
RgbFactor&          LxoConstantProcedure::GetColorR ()                              { return m_color; }
void                LxoConstantProcedure::SetColor (double r, double g, double b)   { m_color.red = r; m_color.green = g; m_color.blue = b; }

double              LxoConstantProcedure::GetValue () const                         { return m_value; }
void                LxoConstantProcedure::SetValue (double value)                   { m_value = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoCellularProcedure::LxoCellularProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoCellularProcedure::_GetType () const { return PROCEDURETYPE_Cellular; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoCellularProcedure::_InitDefaults ()
    {
    m_cellColor.red   = m_cellColor.green   = m_cellColor.blue   = 0.0;
    m_fillerColor.red = m_fillerColor.green = m_fillerColor.blue = 1.0;

    m_cellAlpha             = 100.0;
    m_fillerAlpha           = 100.0;
    m_transitionWidth       = 30.0;
    m_cellWidth             = 60.0;
    m_bias                  = 50.0;
    m_gain                  = 50.0;
    m_cellularType          = CELLULARTYPE_Angular;
    m_frequencies           = 1;
    m_frequencyRatio        = 2.0;
    m_amplitudeRatio        = 0.5;
    m_valueVariation        = 0.0;
    m_hueVariation          = 0.0;
    m_saturationVariation   = 0.0;
    m_cellValue             = 0.0;
    m_fillerValue           = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoCellularProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Cellular != rhs.GetType ())
        return ERROR;

    LxoCellularProcedureCP castRhs = reinterpret_cast <LxoCellularProcedureCP> (&rhs);

    m_cellColor             = castRhs->m_cellColor;
    m_fillerColor           = castRhs->m_fillerColor;
    m_cellAlpha             = castRhs->m_cellAlpha;
    m_fillerAlpha           = castRhs->m_fillerAlpha;
    m_transitionWidth       = castRhs->m_transitionWidth;
    m_cellWidth             = castRhs->m_cellWidth;
    m_bias                  = castRhs->m_bias;
    m_gain                  = castRhs->m_gain;
    m_cellularType          = castRhs->m_cellularType;
    m_frequencies           = castRhs->m_frequencies;
    m_frequencyRatio        = castRhs->m_frequencyRatio;
    m_amplitudeRatio        = castRhs->m_amplitudeRatio;
    m_valueVariation        = castRhs->m_valueVariation;
    m_hueVariation          = castRhs->m_hueVariation;
    m_saturationVariation   = castRhs->m_saturationVariation;
    m_cellValue             = castRhs->m_cellValue;
    m_fillerValue           = castRhs->m_fillerValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoCellularProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Cellular != rhs.GetType ())
        return false;

    LxoCellularProcedureCP castRhs = reinterpret_cast <LxoCellularProcedureCP> (&rhs);

    RETURN_IF_FALSE                 (castRhs->m_cellularType    ==      m_cellularType);
    RETURN_IF_FALSE                 (castRhs->m_frequencies     ==      m_frequencies);
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_cellColor,              m_cellColor));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_fillerColor,            m_fillerColor));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_cellAlpha,              m_cellAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerAlpha,            m_fillerAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_transitionWidth,        m_transitionWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_cellWidth,              m_cellWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,                   m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,                   m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_frequencyRatio,         m_frequencyRatio));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_amplitudeRatio,         m_amplitudeRatio));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_valueVariation,         m_valueVariation));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_hueVariation,           m_hueVariation));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_saturationVariation,    m_saturationVariation));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_cellValue,              m_cellValue));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerValue,            m_fillerValue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoCellularProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularAlpha",               (uintptr_t)&m_cellAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularFillerAlpha",         (uintptr_t)&m_fillerAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularTransitionWidth",     (uintptr_t)&m_transitionWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularBias",                (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularGain",                (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularWidth",               (uintptr_t)&m_cellWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularFreqRatio",           (uintptr_t)&m_frequencyRatio));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularAmpRatio",            (uintptr_t)&m_amplitudeRatio));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularCellValue",           (uintptr_t)&m_cellValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularFillerValue",         (uintptr_t)&m_fillerValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularValue",               (uintptr_t)&m_valueVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularHue",                 (uintptr_t)&m_hueVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularSaturation",          (uintptr_t)&m_saturationVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoCellularType",                (uintptr_t)&m_cellularType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoCellularFreq",                (uintptr_t)&m_frequencies));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularColor",               (uintptr_t)&m_cellColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCellularFillerColor",         (uintptr_t)&m_fillerColor));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&                    LxoCellularProcedure::GetCellColor () const                                     { return m_cellColor; }
RgbFactor&                          LxoCellularProcedure::GetCellColorR ()                                          { return m_cellColor; }
void                                LxoCellularProcedure::SetCellColor (double r, double g, double b)               { m_cellColor.red = r; m_cellColor.green = g; m_cellColor.blue = b; }

RgbFactor const&                    LxoCellularProcedure::GetFillerColor () const                                   { return m_fillerColor; }
RgbFactor&                          LxoCellularProcedure::GetFillerColorR ()                                        { return m_fillerColor; }
void                                LxoCellularProcedure::SetFillerColor (double r, double g, double b)             { m_fillerColor.red = r; m_fillerColor.green = g; m_fillerColor.blue = b; }

double                              LxoCellularProcedure::GetCellAlpha () const                                     { return m_cellAlpha; }
void                                LxoCellularProcedure::SetCellAlpha (double alpha)                               { m_cellAlpha = alpha; }

double                              LxoCellularProcedure::GetFillerAlpha () const                                   { return m_fillerAlpha; }
void                                LxoCellularProcedure::SetFillerAlpha (double alpha)                             { m_fillerAlpha = alpha; }

double                              LxoCellularProcedure::GetBias () const                                          { return m_bias; }
void                                LxoCellularProcedure::SetBias (double bias)                                     { m_bias = bias; }

double                              LxoCellularProcedure::GetGain () const                                          { return m_gain; }
void                                LxoCellularProcedure::SetGain (double gain)                                     { m_gain = gain; }

double                              LxoCellularProcedure::GetCellWidth () const                                     { return m_cellWidth; }
void                                LxoCellularProcedure::SetCellWidth (double width)                               { m_cellWidth = width; }

double                              LxoCellularProcedure::GetTransitionWidth () const                               { return m_transitionWidth; }
void                                LxoCellularProcedure::SetTransitionWidth (double width)                         { m_transitionWidth = width; }

double                              LxoCellularProcedure::GetCellValue () const                                     { return m_cellValue; }
void                                LxoCellularProcedure::SetCellValue (double value)                               { m_cellValue = value; }

double                              LxoCellularProcedure::GetFillerValue () const                                   { return m_fillerValue; }
void                                LxoCellularProcedure::SetFillerValue (double value)                             { m_fillerValue = value; }

int32_t                             LxoCellularProcedure::GetFrequencies () const                                   { return m_frequencies; }
void                                LxoCellularProcedure::SetFrequencies (int32_t frequencies)                        { m_frequencies = frequencies; }

double                              LxoCellularProcedure::GetFrequencyRatio () const                                { return m_frequencyRatio; }
void                                LxoCellularProcedure::SetFrequencyRatio (double ratio)                          { m_frequencyRatio = ratio; }

double                              LxoCellularProcedure::GetAmplitudeRatio () const                                { return m_amplitudeRatio; }
void                                LxoCellularProcedure::SetAmplitudeRatio (double ratio)                          { m_amplitudeRatio = ratio; }

double                              LxoCellularProcedure::GetValueVariation () const                                { return m_valueVariation; }
void                                LxoCellularProcedure::SetValueVariation (double variation)                      { m_valueVariation = variation; }

double                              LxoCellularProcedure::GetHueVariation () const                                  { return m_hueVariation; }
void                                LxoCellularProcedure::SetHueVariation (double variation)                        { m_hueVariation = variation; }

double                              LxoCellularProcedure::GetSaturationVariation () const                           { return m_saturationVariation; }
void                                LxoCellularProcedure::SetSaturationVariation (double variation)                 { m_saturationVariation = variation; }

LxoCellularProcedure::CellularType  LxoCellularProcedure::GetCellularType () const                                  { return m_cellularType; }
void                                LxoCellularProcedure::SetCellularType (LxoCellularProcedure::CellularType type) { m_cellularType = type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoWoodProcedure::LxoWoodProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoWoodProcedure::_GetType () const { return PROCEDURETYPE_Wood; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWoodProcedure::_InitDefaults ()
    {
    m_ringColor.red     = 0.4;
    m_ringColor.green   = 0.25;
    m_ringColor.blue    = 0.1;

    m_fillerColor.red   = 0.6;
    m_fillerColor.green = 0.45;
    m_fillerColor.blue  = 0.3;

    m_ringAlpha         = 100.0;
    m_fillerAlpha       = 100.0;
    m_bias              = 50.0;
    m_gain              = 50.0;
    m_ringScale         = 25.0;
    m_waveScale         = 100.0;
    m_waviness          = 50.0;
    m_graininess        = 50.0;
    m_grainScale        = 10.0;
    m_ringValue         = 0.0;
    m_fillerValue       = 100.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoWoodProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Wood != rhs.GetType ())
        return ERROR;

    LxoWoodProcedureCP castRhs = reinterpret_cast <LxoWoodProcedureCP> (&rhs);

    m_ringColor             = castRhs->m_ringColor;
    m_fillerColor           = castRhs->m_fillerColor;
    m_ringAlpha             = castRhs->m_ringAlpha;
    m_fillerAlpha           = castRhs->m_fillerAlpha;
    m_bias                  = castRhs->m_bias;
    m_gain                  = castRhs->m_gain;
    m_ringScale             = castRhs->m_ringScale;
    m_waveScale             = castRhs->m_waveScale;
    m_waviness              = castRhs->m_waviness;
    m_graininess            = castRhs->m_graininess;
    m_grainScale            = castRhs->m_grainScale;
    m_ringValue             = castRhs->m_ringValue;
    m_fillerValue           = castRhs->m_fillerValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoWoodProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Wood != rhs.GetType ())
        return false;

    LxoWoodProcedureCP castRhs = reinterpret_cast <LxoWoodProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual    (castRhs->m_ringColor,              m_ringColor));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_fillerColor,            m_fillerColor));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_ringAlpha,              m_ringAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerAlpha,            m_fillerAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,                   m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,                   m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_ringScale,              m_ringScale));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_waveScale,              m_waveScale));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_waviness,               m_waviness));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_graininess,             m_graininess));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_grainScale,             m_grainScale));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_ringValue,              m_ringValue));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_fillerValue,            m_fillerValue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWoodProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodAlpha",               (uintptr_t)&m_ringAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodFillerAlpha",         (uintptr_t)&m_fillerAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodRingScale",           (uintptr_t)&m_ringScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodWaveScale",           (uintptr_t)&m_waveScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodBias",                (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodGain",                (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodWaviness",            (uintptr_t)&m_waviness));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodGraininess",          (uintptr_t)&m_graininess));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodGrainScale",          (uintptr_t)&m_grainScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodRingValue",           (uintptr_t)&m_ringValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodFillerValue",         (uintptr_t)&m_fillerValue));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodColor",               (uintptr_t)&m_ringColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWoodFillerColor",         (uintptr_t)&m_fillerColor));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&                    LxoWoodProcedure::GetRingColor () const                                     { return m_ringColor; }
RgbFactor&                          LxoWoodProcedure::GetRingColorR ()                                          { return m_ringColor; }
void                                LxoWoodProcedure::SetRingColor (double r, double g, double b)               { m_ringColor.red = r; m_ringColor.green = g; m_ringColor.blue = b; }

RgbFactor const&                    LxoWoodProcedure::GetFillerColor () const                                   { return m_fillerColor; }
RgbFactor&                          LxoWoodProcedure::GetFillerColorR ()                                        { return m_fillerColor; }
void                                LxoWoodProcedure::SetFillerColor (double r, double g, double b)             { m_fillerColor.red = r; m_fillerColor.green = g; m_fillerColor.blue = b; }

double                              LxoWoodProcedure::GetRingAlpha () const                                     { return m_ringAlpha; }
void                                LxoWoodProcedure::SetRingAlpha (double alpha)                               { m_ringAlpha = alpha; }

double                              LxoWoodProcedure::GetFillerAlpha () const                                   { return m_fillerAlpha; }
void                                LxoWoodProcedure::SetFillerAlpha (double alpha)                             { m_fillerAlpha = alpha; }

double                              LxoWoodProcedure::GetBias () const                                          { return m_bias; }
void                                LxoWoodProcedure::SetBias (double bias)                                     { m_bias = bias; }

double                              LxoWoodProcedure::GetGain () const                                          { return m_gain; }
void                                LxoWoodProcedure::SetGain (double gain)                                     { m_gain = gain; }

double                              LxoWoodProcedure::GetRingScale () const                                     { return m_ringScale; }
void                                LxoWoodProcedure::SetRingScale (double scale)                               { m_ringScale = scale; }

double                              LxoWoodProcedure::GetWaveScale () const                                     { return m_waveScale; }
void                                LxoWoodProcedure::SetWaveScale (double scale)                               { m_waveScale = scale; }

double                              LxoWoodProcedure::GetWaviness () const                                      { return m_waviness; }
void                                LxoWoodProcedure::SetWaviness (double waviness)                             { m_waviness = waviness; }

double                              LxoWoodProcedure::GetGraininess () const                                    { return m_graininess; }
void                                LxoWoodProcedure::SetGraininess (double graininess)                         { m_graininess = graininess; }

double                              LxoWoodProcedure::GetGrainScale () const                                    { return m_grainScale; }
void                                LxoWoodProcedure::SetGrainScale (double scale)                              { m_grainScale = scale; }

double                              LxoWoodProcedure::GetRingValue () const                                     { return m_ringValue; }
void                                LxoWoodProcedure::SetRingValue (double value)                               { m_ringValue = value; }

double                              LxoWoodProcedure::GetFillerValue () const                                   { return m_fillerValue; }
void                                LxoWoodProcedure::SetFillerValue (double value)                             { m_fillerValue = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoWeaveProcedure::LxoWeaveProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoWeaveProcedure::_GetType () const { return PROCEDURETYPE_Weave; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWeaveProcedure::_InitDefaults ()
    {
    m_gapColor.red  = m_gapColor.green  = m_gapColor.blue  = 0.0;
    m_yarnColor.red = m_yarnColor.green = m_yarnColor.blue = 1.0;

    m_yarnAlpha     = 100.0;
    m_gapAlpha      = 100.0;
    m_yarnWidth     = 60.0;
    m_roundness     = 60.0;
    m_bias          = 50.0;
    m_gain          = 50.0;
    m_yarnValue     = 100.0;
    m_gapValue      = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoWeaveProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Weave != rhs.GetType ())
        return ERROR;

    LxoWeaveProcedureCP castRhs = reinterpret_cast <LxoWeaveProcedureCP> (&rhs);

    m_yarnColor     = castRhs->m_yarnColor;
    m_gapColor      = castRhs->m_gapColor;
    m_yarnAlpha     = castRhs->m_yarnAlpha;
    m_gapAlpha      = castRhs->m_gapAlpha;
    m_yarnWidth     = castRhs->m_yarnWidth;
    m_roundness     = castRhs->m_roundness;
    m_bias          = castRhs->m_bias;
    m_gain          = castRhs->m_gain;
    m_yarnValue     = castRhs->m_yarnValue;
    m_gapValue      = castRhs->m_gapValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoWeaveProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Weave != rhs.GetType ())
        return false;

    LxoWeaveProcedureCP castRhs = reinterpret_cast <LxoWeaveProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual    (castRhs->m_yarnColor,  m_yarnColor));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_gapColor,   m_gapColor));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_yarnAlpha,  m_yarnAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gapAlpha,   m_gapAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_roundness,  m_roundness));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_yarnWidth,  m_yarnWidth));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,       m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,       m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_yarnValue,  m_yarnValue));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gapValue,   m_gapValue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWeaveProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveYarnAlpha",            (uintptr_t)&m_yarnAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveGapAlpha",             (uintptr_t)&m_gapAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveYarnWidth",            (uintptr_t)&m_yarnWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveRoundness",            (uintptr_t)&m_roundness));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveBias",                 (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveGain",                 (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveYarnValue",            (uintptr_t)&m_yarnValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveGapValue",             (uintptr_t)&m_gapValue));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveYarnColor",            (uintptr_t)&m_yarnColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWeaveGapColor",             (uintptr_t)&m_gapColor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&    LxoWeaveProcedure::GetYarnColor () const                        { return m_yarnColor; }
RgbFactor&          LxoWeaveProcedure::GetYarnColorR ()                             { return m_yarnColor; }
void                LxoWeaveProcedure::SetYarnColor (double r, double g, double b)  { m_yarnColor.red = r; m_yarnColor.green = g; m_yarnColor.blue = b; }

RgbFactor const&    LxoWeaveProcedure::GetGapColor () const                         { return m_gapColor; }
RgbFactor&          LxoWeaveProcedure::GetGapColorR ()                              { return m_gapColor; }
void                LxoWeaveProcedure::SetGapColor (double r, double g, double b)   { m_gapColor.red = r; m_gapColor.green = g; m_gapColor.blue = b; }

double              LxoWeaveProcedure::GetYarnAlpha () const                        { return m_yarnAlpha; }
void                LxoWeaveProcedure::SetYarnAlpha (double alpha)                  { m_yarnAlpha = alpha; }

double              LxoWeaveProcedure::GetGapAlpha () const                         { return m_gapAlpha; }
void                LxoWeaveProcedure::SetGapAlpha (double alpha)                   { m_gapAlpha = alpha; }

double              LxoWeaveProcedure::GetBias () const                             { return m_bias; }
void                LxoWeaveProcedure::SetBias (double bias)                        { m_bias = bias; }

double              LxoWeaveProcedure::GetGain () const                             { return m_gain; }
void                LxoWeaveProcedure::SetGain (double gain)                        { m_gain = gain; }

double              LxoWeaveProcedure::GetYarnWidth () const                        { return m_yarnWidth; }
void                LxoWeaveProcedure::SetYarnWidth (double width)                  { m_yarnWidth = width; }

double              LxoWeaveProcedure::GetRoundness () const                        { return m_roundness; }
void                LxoWeaveProcedure::SetRoundness (double roundness)              { m_roundness = roundness; }

double              LxoWeaveProcedure::GetYarnValue () const                        { return m_yarnValue; }
void                LxoWeaveProcedure::SetYarnValue (double value)                  { m_yarnValue = value; }

double              LxoWeaveProcedure::GetGapValue () const                         { return m_gapValue; }
void                LxoWeaveProcedure::SetGapValue (double value)                   { m_gapValue = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoRipplesProcedure::LxoRipplesProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoRipplesProcedure::_GetType () const { return PROCEDURETYPE_Ripples; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoRipplesProcedure::_InitDefaults ()
    {
    m_troughColor.red = m_troughColor.green = m_troughColor.blue = 0.0;
    m_crestColor.red  = m_crestColor.green  = m_crestColor.blue  = 1.0;

    m_crestAlpha    = 100.0;
    m_troughAlpha   = 100.0;
    m_wavelength    = 20.0;
    m_phase         = 0.0;
    m_bias          = 50.0;
    m_gain          = 50.0;
    m_sources       = 8;
    m_crestValue    = 100.0;
    m_troughValue   = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoRipplesProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Ripples != rhs.GetType ())
        return ERROR;

    LxoRipplesProcedureCP castRhs = reinterpret_cast <LxoRipplesProcedureCP> (&rhs);

    m_crestColor    = castRhs->m_crestColor;
    m_troughColor   = castRhs->m_troughColor;
    m_crestAlpha    = castRhs->m_crestAlpha;
    m_troughAlpha   = castRhs->m_troughAlpha;
    m_wavelength    = castRhs->m_wavelength;
    m_phase         = castRhs->m_phase;
    m_bias          = castRhs->m_bias;
    m_gain          = castRhs->m_gain;
    m_sources       = castRhs->m_sources;
    m_crestValue    = castRhs->m_crestValue;
    m_troughValue   = castRhs->m_troughValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoRipplesProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Ripples != rhs.GetType ())
        return false;

    LxoRipplesProcedureCP castRhs = reinterpret_cast <LxoRipplesProcedureCP> (&rhs);

    RETURN_IF_FALSE                 (castRhs->m_sources ==      m_sources);
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_crestColor,     m_crestColor));
    RETURN_IF_FALSE (colorsEqual    (castRhs->m_troughColor,    m_troughColor));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_crestAlpha,     m_crestAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_troughAlpha,    m_troughAlpha));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_wavelength,     m_wavelength));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_phase,          m_phase));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_bias,           m_bias));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_gain,           m_gain));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_crestValue,     m_crestValue));
    RETURN_IF_FALSE (valuesEqual    (castRhs->m_troughValue,    m_troughValue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoRipplesProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesCrestAlpha",     (uintptr_t)&m_crestAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesTroughAlpha",    (uintptr_t)&m_troughAlpha));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesWaveLength",     (uintptr_t)&m_wavelength));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesPhase",          (uintptr_t)&m_phase));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesBias",           (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesGain",           (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoRipplesSources",        (uintptr_t)&m_sources));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesCrestValue",     (uintptr_t)&m_crestValue));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesTroughValue",    (uintptr_t)&m_troughValue));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesCrestColor",     (uintptr_t)&m_crestColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoRipplesTroughColor",    (uintptr_t)&m_troughColor));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
RgbFactor const&    LxoRipplesProcedure::GetCrestColor () const                         { return m_crestColor; }
RgbFactor&          LxoRipplesProcedure::GetCrestColorR ()                              { return m_crestColor; }
void                LxoRipplesProcedure::SetCrestColor (double r, double g, double b)   { m_crestColor.red = r; m_crestColor.green = g; m_crestColor.blue = b; }

RgbFactor const&    LxoRipplesProcedure::GetTroughColor () const                        { return m_troughColor; }
RgbFactor&          LxoRipplesProcedure::GetTroughColorR ()                             { return m_troughColor; }
void                LxoRipplesProcedure::SetTroughColor (double r, double g, double b)  { m_troughColor.red = r; m_troughColor.green = g; m_troughColor.blue = b; }

double              LxoRipplesProcedure::GetCrestAlpha () const                         { return m_crestAlpha; }
void                LxoRipplesProcedure::SetCrestAlpha (double alpha)                   { m_crestAlpha = alpha; }

double              LxoRipplesProcedure::GetTroughAlpha () const                        { return m_troughAlpha; }
void                LxoRipplesProcedure::SetTroughAlpha (double alpha)                  { m_troughAlpha = alpha; }

double              LxoRipplesProcedure::GetWavelength () const                         { return m_wavelength; }
void                LxoRipplesProcedure::SetWavelength (double length)                  { m_wavelength = length; }

double              LxoRipplesProcedure::GetPhase () const                              { return m_phase; }
void                LxoRipplesProcedure::SetPhase (double phase)                        { m_phase = phase; }

double              LxoRipplesProcedure::GetBias () const                               { return m_bias; }
void                LxoRipplesProcedure::SetBias (double bias)                          { m_bias = bias; }

double              LxoRipplesProcedure::GetGain () const                               { return m_gain; }
void                LxoRipplesProcedure::SetGain (double gain)                          { m_gain = gain; }

uint32_t            LxoRipplesProcedure::GetSources () const                            { return m_sources; }
void                LxoRipplesProcedure::SetSources (uint32_t sources)                    { m_sources = sources; }

double              LxoRipplesProcedure::GetCrestValue () const                         { return m_crestValue; }
void                LxoRipplesProcedure::SetCrestValue (double value)                   { m_crestValue = value; }

double              LxoRipplesProcedure::GetTroughValue () const                        { return m_troughValue; }
void                LxoRipplesProcedure::SetTroughValue (double value)                  { m_troughValue = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopeKey::LxoFloatEnvelopeKey ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeKey::InitDefaults ()
    {
    m_value.Init (0.0, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeKey::Copy (LxoFloatEnvelopeKeyCR rhs)
    {
    m_value = rhs.m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoFloatEnvelopeKey::Equals (LxoFloatEnvelopeKeyCR rhs) const
    {
    return TO_BOOL (m_value.IsEqual (rhs.m_value, 1.0E-4));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2dCR  LxoFloatEnvelopeKey::GetValue () const              { return m_value; }
DPoint2dR   LxoFloatEnvelopeKey::GetValueR ()                   { return m_value; }
void        LxoFloatEnvelopeKey::SetValue (double x, double y)  { m_value.Init (x, y); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopeKey::LxoIntEnvelopeKey ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeKey::InitDefaults ()
    {
    m_valueX = 0.0;
    m_valueY = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeKey::Copy (LxoIntEnvelopeKeyCR rhs)
    {
    m_valueX = rhs.m_valueX;
    m_valueY = rhs.m_valueY;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoIntEnvelopeKey::Equals (LxoIntEnvelopeKeyCR rhs) const
    {
    return valuesEqual (m_valueX, rhs.m_valueX) && m_valueY == rhs.m_valueY;
    }

double LxoIntEnvelopeKey::GetValueX () const {return m_valueX;}
int LxoIntEnvelopeKey::GetValueY () const {return m_valueY;}
void LxoIntEnvelopeKey::SetValue (double x, int y) {m_valueX = x; m_valueY = y;}
void LxoIntEnvelopeKey::SetValueX (double x) {m_valueX = x;}
void LxoIntEnvelopeKey::SetValueY (int y) {m_valueY = y;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoEnvelopeTangentIn::LxoEnvelopeTangentIn ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      01/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoEnvelopeTangentIn::LxoEnvelopeTangentIn (LxoEnvelopeTangentInCR rhs)
    {
    Copy (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoEnvelopeTangentIn::InitDefaults ()
    {
    m_slopeType     = LxoEnvelopeSlopeType::Direct;
    m_weightType    = LxoEnvelopeWeightType::Auto;
    m_slope         = 0.0;
    m_weight        = 0.0;
    m_value         = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoEnvelopeTangentIn::Copy (LxoEnvelopeTangentInCR rhs)
    {
    m_slopeType     = rhs.m_slopeType;
    m_weightType    = rhs.m_weightType;
    m_slope         = rhs.m_slope;
    m_weight        = rhs.m_weight;
    m_value         = rhs.m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoEnvelopeTangentIn::Equals (LxoEnvelopeTangentInCR rhs) const
    {
    RETURN_IF_FALSE              (rhs.m_slopeType   ==  m_slopeType);
    RETURN_IF_FALSE              (rhs.m_weightType  ==  m_weightType);
    RETURN_IF_FALSE (valuesEqual (rhs.m_slope,          m_slope));
    RETURN_IF_FALSE (valuesEqual (rhs.m_weight,         m_weight));
    RETURN_IF_FALSE (valuesEqual (rhs.m_value,          m_value));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoEnvelopeSlopeType    LxoEnvelopeTangentIn::GetSlopeType () const                         { return m_slopeType; }
void                    LxoEnvelopeTangentIn::SetSlopeType (LxoEnvelopeSlopeType type)      { m_slopeType = type; }

LxoEnvelopeWeightType   LxoEnvelopeTangentIn::GetWeightType () const                        { return m_weightType; }
void                    LxoEnvelopeTangentIn::SetWeightType (LxoEnvelopeWeightType type)    { m_weightType = type; }

double                  LxoEnvelopeTangentIn::GetSlope () const                             { return m_slope; }
void                    LxoEnvelopeTangentIn::SetSlope (double slope)                       { m_slope = slope; }

double                  LxoEnvelopeTangentIn::GetWeight () const                            { return m_weight; }
void                    LxoEnvelopeTangentIn::SetWeight (double weight)                     { m_weight = weight; }

double                  LxoEnvelopeTangentIn::GetValue () const                             { return m_value; }
void                    LxoEnvelopeTangentIn::SetValue (double value)                       { m_value = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoEnvelopeTangentOut::LxoEnvelopeTangentOut ()
    {
    InitDefaults ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     01/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoEnvelopeTangentOut::LxoEnvelopeTangentOut (LxoEnvelopeTangentOutCR rhs)
    {
    Copy (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoEnvelopeTangentOut::InitDefaults ()
    {
    m_break         = LxoEnvelopeBreak::Value;
    m_slopeType     = LxoEnvelopeSlopeType::Direct;
    m_weightType    = LxoEnvelopeWeightType::Auto;
    m_slope         = 0.0;
    m_weight        = 0.0;
    m_value         = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoEnvelopeTangentOut::Copy (LxoEnvelopeTangentOutCR rhs)
    {
    m_break         = rhs.m_break;
    m_slopeType     = rhs.m_slopeType;
    m_weightType    = rhs.m_weightType;
    m_slope         = rhs.m_slope;
    m_weight        = rhs.m_weight;
    m_value         = rhs.m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoEnvelopeTangentOut::Equals (LxoEnvelopeTangentOutCR rhs) const
    {
    RETURN_IF_FALSE              (rhs.m_break       ==  m_break);
    RETURN_IF_FALSE              (rhs.m_slopeType   ==  m_slopeType);
    RETURN_IF_FALSE              (rhs.m_weightType  ==  m_weightType);
    RETURN_IF_FALSE (valuesEqual (rhs.m_slope,          m_slope));
    RETURN_IF_FALSE (valuesEqual (rhs.m_weight,         m_weight));
    RETURN_IF_FALSE (valuesEqual (rhs.m_value,          m_value));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoEnvelopeBreak        LxoEnvelopeTangentOut::GetBreak () const                            { return m_break; }
void                    LxoEnvelopeTangentOut::SetBreak (LxoEnvelopeBreak breakType)        { m_break = breakType; }

LxoEnvelopeSlopeType    LxoEnvelopeTangentOut::GetSlopeType () const                        { return m_slopeType; }
void                    LxoEnvelopeTangentOut::SetSlopeType (LxoEnvelopeSlopeType type)     { m_slopeType = type; }

LxoEnvelopeWeightType   LxoEnvelopeTangentOut::GetWeightType () const                       { return m_weightType; }
void                    LxoEnvelopeTangentOut::SetWeightType (LxoEnvelopeWeightType type)   { m_weightType = type; }

double                  LxoEnvelopeTangentOut::GetSlope () const                            { return m_slope; }
void                    LxoEnvelopeTangentOut::SetSlope (double slope)                      { m_slope = slope; }

double                  LxoEnvelopeTangentOut::GetWeight () const                           { return m_weight; }
void                    LxoEnvelopeTangentOut::SetWeight (double weight)                    { m_weight = weight; }

double                  LxoEnvelopeTangentOut::GetValue () const                            { return m_value; }
void                    LxoEnvelopeTangentOut::SetValue (double value)                      { m_value = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopeComponent::LxoFloatEnvelopeComponent ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeComponent::InitDefaults ()
    {
    m_key.InitDefaults ();
    m_tangentIn.InitDefaults ();
    m_tangentOut.InitDefaults ();
    m_flag = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeComponent::Copy (LxoFloatEnvelopeComponentCR rhs)
    {
    m_key.Copy          (rhs.m_key);
    m_tangentIn.Copy    (rhs.m_tangentIn);
    m_tangentOut.Copy   (rhs.m_tangentOut);
    m_flag =             rhs.m_flag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoFloatEnvelopeComponent::Equals (LxoFloatEnvelopeComponentCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_flag ==              m_flag);
    RETURN_IF_FALSE (rhs.m_key.Equals          (m_key));
    RETURN_IF_FALSE (rhs.m_tangentIn.Equals    (m_tangentIn));
    RETURN_IF_FALSE (rhs.m_tangentOut.Equals   (m_tangentOut));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopeKeyCR       LxoFloatEnvelopeComponent::GetKey () const          { return m_key; }
LxoFloatEnvelopeKeyR        LxoFloatEnvelopeComponent::GetKeyR ()               { return m_key; }

LxoEnvelopeTangentInCR      LxoFloatEnvelopeComponent::GetTangentIn () const    { return m_tangentIn; }
LxoEnvelopeTangentInR       LxoFloatEnvelopeComponent::GetTangentInR ()         { return m_tangentIn; }

LxoEnvelopeTangentOutCR     LxoFloatEnvelopeComponent::GetTangentOut () const   { return m_tangentOut; }
LxoEnvelopeTangentOutR      LxoFloatEnvelopeComponent::GetTangentOutR ()        { return m_tangentOut; }

uint32_t                    LxoFloatEnvelopeComponent::GetFlag () const         { return m_flag; }
void                        LxoFloatEnvelopeComponent::SetFlag (uint32_t flag)    { m_flag = flag; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopeComponentCollection::LxoFloatEnvelopeComponentCollection ()
    {
    InitDefaults ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeComponentCollection::InitDefaults ()
    {
    m_components.clear ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoFloatEnvelopeComponentCollection::Equals (LxoFloatEnvelopeComponentCollectionCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_components.size () == m_components.size ());
    for (size_t i = 0, limit = rhs.m_components.size (); i < limit; ++i)
        {
        if (!rhs.m_components[i].get ()->Equals (*m_components[i].get ()))
            return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeComponentCollection::Copy (LxoFloatEnvelopeComponentCollectionCR rhs)
    {
    m_components.clear ();
    FOR_EACH (LxoFloatEnvelopeComponentPtr const& rhsComponent , rhs.m_components)
        AddComponent ().Copy (*rhsComponent.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopeComponentCollection::const_iterator     LxoFloatEnvelopeComponentCollection::begin () const     { return m_components.begin (); }
LxoFloatEnvelopeComponentCollection::iterator           LxoFloatEnvelopeComponentCollection::begin ()           { return m_components.begin (); }
LxoFloatEnvelopeComponentCollection::const_iterator     LxoFloatEnvelopeComponentCollection::end () const       { return m_components.end (); }
LxoFloatEnvelopeComponentCollection::iterator           LxoFloatEnvelopeComponentCollection::end ()             { return m_components.end (); }
size_t                                                  LxoFloatEnvelopeComponentCollection::Size () const      { return m_components.size (); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopeComponentR LxoFloatEnvelopeComponentCollection::AddComponent ()
    {
    m_components.push_back (LxoFloatEnvelopeComponentPtr (new LxoFloatEnvelopeComponent ()));
    return *m_components.back ().get ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeComponentCollection::DeleteComponent (LxoFloatEnvelopeComponentCollection::iterator iter)
    {
    if (m_components.end () == iter)
        return;

    m_components.erase (iter);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelopeComponentCollection::Clear ()
    {
    m_components.clear ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelope::LxoFloatEnvelope ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      01/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelopePtr LxoFloatEnvelope::Create ()
    {
    return new LxoFloatEnvelope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelope::InitDefaults ()
    {
    m_preBehavior   = ENDBEHAVIOR_Constant;
    m_postBehavior  = ENDBEHAVIOR_Constant;
    m_components.InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFloatEnvelope::Copy (LxoFloatEnvelopeCR rhs)
    {
    m_preBehavior   = rhs.m_preBehavior;
    m_postBehavior  = rhs.m_postBehavior;
    m_components.Copy (rhs.m_components);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoFloatEnvelope::Equals (LxoFloatEnvelopeCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_preBehavior      == m_preBehavior);
    RETURN_IF_FALSE (rhs.m_postBehavior     == m_postBehavior);
    RETURN_IF_FALSE (rhs.m_components.Equals (m_components));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFloatEnvelope::EndBehavior           LxoFloatEnvelope::GetPreBehavior () const                                   { return m_preBehavior; }
void                                    LxoFloatEnvelope::SetPreBehavior (LxoFloatEnvelope::EndBehavior behavior)   { m_preBehavior = behavior; }

LxoFloatEnvelope::EndBehavior           LxoFloatEnvelope::GetPostBehavior () const                                  { return m_preBehavior; }
void                                    LxoFloatEnvelope::SetPostBehavior (LxoFloatEnvelope::EndBehavior behavior)  { m_preBehavior = behavior; }

LxoFloatEnvelopeComponentCollectionCR   LxoFloatEnvelope::GetComponents () const                                    { return m_components; }
LxoFloatEnvelopeComponentCollectionR    LxoFloatEnvelope::GetComponentsR ()                                         { return m_components; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopeComponent::LxoIntEnvelopeComponent ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeComponent::InitDefaults ()
    {
    m_key.InitDefaults ();
    m_tangentIn.InitDefaults ();
    m_tangentOut.InitDefaults ();
    m_flag = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeComponent::Copy (LxoIntEnvelopeComponentCR rhs)
    {
    m_key.Copy          (rhs.m_key);
    m_tangentIn.Copy    (rhs.m_tangentIn);
    m_tangentOut.Copy   (rhs.m_tangentOut);
    m_flag =             rhs.m_flag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoIntEnvelopeComponent::Equals (LxoIntEnvelopeComponentCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_flag ==              m_flag);
    RETURN_IF_FALSE (rhs.m_key.Equals          (m_key));
    RETURN_IF_FALSE (rhs.m_tangentIn.Equals    (m_tangentIn));
    RETURN_IF_FALSE (rhs.m_tangentOut.Equals   (m_tangentOut));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopeKeyCR       LxoIntEnvelopeComponent::GetKey () const          { return m_key; }
LxoIntEnvelopeKeyR        LxoIntEnvelopeComponent::GetKeyR ()               { return m_key; }

LxoEnvelopeTangentInCR      LxoIntEnvelopeComponent::GetTangentIn () const    { return m_tangentIn; }
LxoEnvelopeTangentInR       LxoIntEnvelopeComponent::GetTangentInR ()         { return m_tangentIn; }

LxoEnvelopeTangentOutCR     LxoIntEnvelopeComponent::GetTangentOut () const   { return m_tangentOut; }
LxoEnvelopeTangentOutR      LxoIntEnvelopeComponent::GetTangentOutR ()        { return m_tangentOut; }

uint32_t                    LxoIntEnvelopeComponent::GetFlag () const         { return m_flag; }
void                        LxoIntEnvelopeComponent::SetFlag (uint32_t flag)    { m_flag = flag; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopeComponentCollection::LxoIntEnvelopeComponentCollection ()
    {
    InitDefaults ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeComponentCollection::InitDefaults ()
    {
    m_components.clear ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoIntEnvelopeComponentCollection::Equals (LxoIntEnvelopeComponentCollectionCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_components.size () == m_components.size ());
    for (size_t i = 0, limit = rhs.m_components.size (); i < limit; ++i)
        {
        if (!rhs.m_components[i].get ()->Equals (*m_components[i].get ()))
            return false;
        }

    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeComponentCollection::Copy (LxoIntEnvelopeComponentCollectionCR rhs)
    {
    m_components.clear ();
    FOR_EACH (LxoIntEnvelopeComponentPtr const& rhsComponent , rhs.m_components)
        AddComponent ().Copy (*rhsComponent.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopeComponentCollection::const_iterator     LxoIntEnvelopeComponentCollection::begin () const     { return m_components.begin (); }
LxoIntEnvelopeComponentCollection::iterator           LxoIntEnvelopeComponentCollection::begin ()           { return m_components.begin (); }
LxoIntEnvelopeComponentCollection::const_iterator     LxoIntEnvelopeComponentCollection::end () const       { return m_components.end (); }
LxoIntEnvelopeComponentCollection::iterator           LxoIntEnvelopeComponentCollection::end ()             { return m_components.end (); }
size_t                                                LxoIntEnvelopeComponentCollection::Size () const      { return m_components.size (); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopeComponentR LxoIntEnvelopeComponentCollection::AddComponent ()
    {
    m_components.push_back (LxoIntEnvelopeComponentPtr (new LxoIntEnvelopeComponent ()));
    return *m_components.back ().get ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeComponentCollection::DeleteComponent (LxoIntEnvelopeComponentCollection::iterator iter)
    {
    if (m_components.end () == iter)
        return;

    m_components.erase (iter);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelopeComponentCollection::Clear ()
    {
    m_components.clear ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelope::LxoIntEnvelope ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater      01/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelopePtr LxoIntEnvelope::Create ()
    {
    return new LxoIntEnvelope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelope::InitDefaults ()
    {
    m_preBehavior   = ENDBEHAVIOR_Constant;
    m_postBehavior  = ENDBEHAVIOR_Constant;
    m_components.InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoIntEnvelope::Copy (LxoIntEnvelopeCR rhs)
    {
    m_preBehavior   = rhs.m_preBehavior;
    m_postBehavior  = rhs.m_postBehavior;
    m_components.Copy (rhs.m_components);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoIntEnvelope::Equals (LxoIntEnvelopeCR rhs) const
    {
    RETURN_IF_FALSE (rhs.m_preBehavior      == m_preBehavior);
    RETURN_IF_FALSE (rhs.m_postBehavior     == m_postBehavior);
    RETURN_IF_FALSE (rhs.m_components.Equals (m_components));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
LxoIntEnvelope::EndBehavior             LxoIntEnvelope::GetPreBehavior () const                                 { return m_preBehavior; }
void                                    LxoIntEnvelope::SetPreBehavior (LxoIntEnvelope::EndBehavior behavior)   { m_preBehavior = behavior; }

LxoIntEnvelope::EndBehavior             LxoIntEnvelope::GetPostBehavior () const                                { return m_preBehavior; }
void                                    LxoIntEnvelope::SetPostBehavior (LxoIntEnvelope::EndBehavior behavior)  { m_preBehavior = behavior; }

LxoIntEnvelopeComponentCollectionCR   LxoIntEnvelope::GetComponents () const                                    { return m_components; }
LxoIntEnvelopeComponentCollectionR    LxoIntEnvelope::GetComponentsR ()                                         { return m_components; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoGradientProcedure::LxoGradientProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoGradientProcedure::_GetType () const { return PROCEDURETYPE_Gradient; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoGradientProcedure::_InitDefaults ()
    {
    m_input = GRADIENTINPUT_TextureU;

    // add 2 keys to each property. value & alpha envelopes both values = 1.
    // colour envelopes key 1 = 1.0 & 2 = 0.0 will be a fade to black across the texture
    LxoFloatEnvelope defaultEnvelope;
    defaultEnvelope.SetPreBehavior (LxoFloatEnvelope::ENDBEHAVIOR_Constant);
    defaultEnvelope.SetPostBehavior (LxoFloatEnvelope::ENDBEHAVIOR_Constant);
    defaultEnvelope.GetComponentsR ().AddComponent ().GetKeyR ().SetValue (0.0, 1.0);
    defaultEnvelope.GetComponentsR ().AddComponent ().GetKeyR ().SetValue (1.0, 1.0);

    m_value.Copy (defaultEnvelope);
    m_alpha.Copy (defaultEnvelope);

    defaultEnvelope.GetComponentsR ().InitDefaults ();
    defaultEnvelope.GetComponentsR ().AddComponent ().GetKeyR ().SetValue (0.0, 1.0);
    defaultEnvelope.GetComponentsR ().AddComponent ().GetKeyR ().SetValue (1.0, 0.0);

    m_red.Copy (defaultEnvelope);
    m_green.Copy (defaultEnvelope);
    m_blue.Copy (defaultEnvelope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoGradientProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Gradient != rhs.GetType ())
        return ERROR;

    LxoGradientProcedureCP castRhs = reinterpret_cast <LxoGradientProcedureCP> (&rhs);

    m_input = castRhs->m_input;
    m_value.Copy (castRhs->m_value);
    m_alpha.Copy (castRhs->m_alpha);
    m_red.Copy (castRhs->m_red);
    m_green.Copy (castRhs->m_green);
    m_blue.Copy (castRhs->m_blue);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoGradientProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Gradient != rhs.GetType ())
        return false;

    LxoGradientProcedureCP castRhs = reinterpret_cast <LxoGradientProcedureCP> (&rhs);

    RETURN_IF_FALSE (castRhs->m_input ==         m_input);
    RETURN_IF_FALSE (castRhs->m_value.Equals    (m_value));
    RETURN_IF_FALSE (castRhs->m_alpha.Equals    (m_alpha));
    RETURN_IF_FALSE (castRhs->m_red.Equals      (m_red));
    RETURN_IF_FALSE (castRhs->m_green.Equals    (m_green));
    RETURN_IF_FALSE (castRhs->m_blue.Equals     (m_blue));

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoGradientProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoGradientInput",        (uintptr_t)&m_input));
    // the data is not published. Use the callback function in mdlDialog_initGradientGUI
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoGradientProcedure::GradientInput     LxoGradientProcedure::GetGradientInput () const                                     { return m_input; }
void                                    LxoGradientProcedure::SetGradientInput (LxoGradientProcedure::GradientInput input)  { m_input = input; }

LxoFloatEnvelopeCR                      LxoGradientProcedure::GetValueEnvelope () const                                     { return m_value; }
LxoFloatEnvelopeR                       LxoGradientProcedure::GetValueEnvelopeR ()                                          { return m_value; }

LxoFloatEnvelopeCR                      LxoGradientProcedure::GetRedEnvelope () const                                       { return m_red; }
LxoFloatEnvelopeR                       LxoGradientProcedure::GetRedEnvelopeR ()                                            { return m_red; }

LxoFloatEnvelopeCR                      LxoGradientProcedure::GetGreenEnvelope () const                                     { return m_green; }
LxoFloatEnvelopeR                       LxoGradientProcedure::GetGreenEnvelopeR ()                                          { return m_green; }

LxoFloatEnvelopeCR                      LxoGradientProcedure::GetBlueEnvelope () const                                      { return m_blue; }
LxoFloatEnvelopeR                       LxoGradientProcedure::GetBlueEnvelopeR ()                                           { return m_blue; }

LxoFloatEnvelopeCR                      LxoGradientProcedure::GetAlphaEnvelope () const                                     { return m_alpha; }
LxoFloatEnvelopeR                       LxoGradientProcedure::GetAlphaEnvelopeR ()                                          { return m_alpha; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     08/11
+---------------+---------------+---------------+---------------+---------------+------*/
LxoBGradientProcedure::LxoBGradientProcedure () : LxoGradientProcedure ()
    {
    InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     08/11
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoBGradientProcedure::_GetType () const { return PROCEDURETYPE_BGradient; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoBoardsProcedure::LxoBoardsProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoBoardsProcedure::_GetType () const
    {
    return PROCEDURETYPE_Boards;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBoardsProcedure::_InitDefaults ()
    {
    m_woodColor.red = 0.671; m_woodColor.green = 0.38; m_woodColor.blue = 0.161;
    m_ringColor.red = 0.439; m_ringColor.green = 0.231; m_ringColor.blue = 0.09;

    m_boardsPerColumn       = 16;
    m_boardsPerRow          = 2;
    m_patternId             = 1;
    m_crackWidth            = 2;
    m_brightnessVariation   = 30;
    m_horizontalVariation   = 20;
    m_grainScale            = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoBoardsProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (PROCEDURETYPE_Boards != rhs.GetType ())
        return ERROR;
    LxoBoardsProcedureCP     boards = reinterpret_cast <LxoBoardsProcedureCP> (&rhs);

    if (NULL == boards)
        return ERROR;

    m_woodColor             = boards->m_woodColor;
    m_ringColor             = boards->m_ringColor;
    m_boardsPerColumn       = boards->m_boardsPerColumn;
    m_boardsPerRow          = boards->m_boardsPerRow;
    m_patternId             = boards->m_patternId;
    m_crackWidth            = boards->m_crackWidth;
    m_brightnessVariation   = boards->m_brightnessVariation;
    m_horizontalVariation   = boards->m_horizontalVariation;
    m_grainScale            = boards->m_grainScale;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoBoardsProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Boards != rhs.GetType ())
        return false;

    LxoBoardsProcedureCP castRhs = reinterpret_cast <LxoBoardsProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_woodColor             , castRhs->m_woodColor));
    RETURN_IF_FALSE (colorsEqual (m_ringColor             , castRhs->m_ringColor));
    RETURN_IF_FALSE (valuesEqual (m_crackWidth             , castRhs->m_crackWidth));
    RETURN_IF_FALSE (valuesEqual (m_brightnessVariation    , castRhs->m_brightnessVariation));
    RETURN_IF_FALSE (valuesEqual (m_horizontalVariation    , castRhs->m_horizontalVariation));
    RETURN_IF_FALSE (valuesEqual (m_grainScale             , castRhs->m_grainScale));
    RETURN_IF_FALSE              (m_boardsPerColumn       == castRhs->m_boardsPerColumn);
    RETURN_IF_FALSE              (m_boardsPerRow          == castRhs->m_boardsPerRow);
    RETURN_IF_FALSE              (m_patternId             == castRhs->m_patternId);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBoardsProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBoardsCrackWidth",           (uintptr_t)&m_crackWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBoardsBrightnessVariation",  (uintptr_t)&m_brightnessVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBoardsHorizontalVariation",  (uintptr_t)&m_horizontalVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBoardsGrainScale",           (uintptr_t)&m_grainScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBoardsBoardsPerColumn",      (uintptr_t)&m_boardsPerColumn));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBoardsBoardsPerRow",         (uintptr_t)&m_boardsPerRow));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBoardsPatternId",            (uintptr_t)&m_patternId));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBoardsWoodColor",            (uintptr_t)&m_woodColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBoardsRingColor",            (uintptr_t)&m_ringColor));
    }

RgbFactor const&        LxoBoardsProcedure::GetWoodColor () const {return m_woodColor;}
RgbFactor&              LxoBoardsProcedure::GetWoodColorR () {return m_woodColor;}
void                    LxoBoardsProcedure::SetWoodColor (double red, double green, double blue) {m_woodColor.red = red; m_woodColor.green = green; m_woodColor.blue = blue; }

RgbFactor const&        LxoBoardsProcedure::GetRingColor () const {return m_ringColor;}
RgbFactor&              LxoBoardsProcedure::GetRingColorR () {return m_ringColor;}
void                    LxoBoardsProcedure::SetRingColor (double red, double green, double blue) {m_ringColor.red = red; m_ringColor.green = green; m_ringColor.blue = blue;}

double                  LxoBoardsProcedure::GetCrackWidth () const {return m_crackWidth;}
void                    LxoBoardsProcedure::SetCrackWidth (double crackWidth) {m_crackWidth = crackWidth;}

double                  LxoBoardsProcedure::GetBrightnessVariation () const {return m_brightnessVariation;}
void                    LxoBoardsProcedure::SetBrightnessVariation (double brightnessVariation) {m_brightnessVariation = brightnessVariation;}

double                  LxoBoardsProcedure::GetHorizontalVariation () const {return m_horizontalVariation;}
void                    LxoBoardsProcedure::SetHorizontalVariation (double horizontalVariation){m_horizontalVariation = horizontalVariation;}

double                  LxoBoardsProcedure::GetGrainScale () const {return m_grainScale;}
void                    LxoBoardsProcedure::SetGrainScale (double grainScale) {m_grainScale = grainScale;}

int32_t                 LxoBoardsProcedure::GetBoardsPerColumn () const {return m_boardsPerColumn;}
void                    LxoBoardsProcedure::SetBoardsPerColumn (int32_t boardsPerColumn) {m_boardsPerColumn = boardsPerColumn;}

int32_t                 LxoBoardsProcedure::GetBoardsPerRow () const {return m_boardsPerRow;}
void                    LxoBoardsProcedure::SetBoardsPerRow (int32_t boardsPerRow) {m_boardsPerRow = boardsPerRow;}

int32_t                 LxoBoardsProcedure::GetPatternId () const {return m_patternId;}
void                    LxoBoardsProcedure::SetPatternId (int32_t patternId) {m_patternId = patternId;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoBrickProcedure::LxoBrickProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoBrickProcedure::_GetType () const
    {
    return PROCEDURETYPE_Brick;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBrickProcedure::_InitDefaults ()
    {
    m_brickColor.red = 0.502; m_brickColor.green = 0.102; m_brickColor.blue = 0.102;
    m_mortarColor.red = 0.902; m_mortarColor.green = 0.902; m_mortarColor.blue = 0.902;

    m_bondType              = BRICKBONDTYPE_Running;
    m_patternId             = 1;
    m_headerCourseInterval  = 6;
    m_flemishHeaders        = FALSE;
    m_mortarWidth           = 5;
    m_brightnessVariation   = 40;
    m_horizontalVariation   = 33;
    m_noisiness             = 3;
    m_aspectRatio           = 2.0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoBrickProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Brick != rhs.GetType ())
        return ERROR;

    LxoBrickProcedureCP castRhs = reinterpret_cast <LxoBrickProcedureCP> (&rhs);

    m_brickColor               = castRhs->m_brickColor;
    m_mortarColor              = castRhs->m_mortarColor;
    m_bondType                 = castRhs->m_bondType;
    m_patternId                = castRhs->m_patternId;
    m_headerCourseInterval     = castRhs->m_headerCourseInterval;
    m_flemishHeaders           = castRhs->m_flemishHeaders;
    m_mortarWidth              = castRhs->m_mortarWidth;
    m_brightnessVariation      = castRhs->m_brightnessVariation;
    m_horizontalVariation      = castRhs->m_horizontalVariation;
    m_noisiness                = castRhs->m_noisiness;
    m_aspectRatio              = castRhs->m_aspectRatio;
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoBrickProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Brick != rhs.GetType ())
        return false;

    LxoBrickProcedureCP castRhs = reinterpret_cast <LxoBrickProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_brickColor             , castRhs->m_brickColor));
    RETURN_IF_FALSE (colorsEqual (m_mortarColor            , castRhs->m_mortarColor));
    RETURN_IF_FALSE (valuesEqual (m_mortarWidth            , castRhs->m_mortarWidth));
    RETURN_IF_FALSE (valuesEqual (m_brightnessVariation    , castRhs->m_brightnessVariation));
    RETURN_IF_FALSE (valuesEqual (m_horizontalVariation    , castRhs->m_horizontalVariation));
    RETURN_IF_FALSE (valuesEqual (m_noisiness              , castRhs->m_noisiness));
    RETURN_IF_FALSE (valuesEqual (m_aspectRatio            , castRhs->m_aspectRatio));
    RETURN_IF_FALSE (m_bondType              == castRhs->m_bondType);
    RETURN_IF_FALSE (m_headerCourseInterval  == castRhs->m_headerCourseInterval);
    RETURN_IF_FALSE (m_flemishHeaders        == castRhs->m_flemishHeaders);
    RETURN_IF_FALSE (m_patternId             == castRhs->m_patternId);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBrickProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickMortarWidth",              (uintptr_t)&m_mortarWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickBrightnessVariation",      (uintptr_t)&m_brightnessVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickHorizontalVariation",      (uintptr_t)&m_horizontalVariation));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickNoisiness",                (uintptr_t)&m_noisiness));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickAspectRatio",              (uintptr_t)&m_aspectRatio));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBrickBondType",                 (uintptr_t)&m_bondType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBrickPatternId",                (uintptr_t)&m_patternId));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBrickHeaderCourseInterval",     (uintptr_t)&m_headerCourseInterval));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoBrickFlemishHeaders",           (uintptr_t)&m_flemishHeaders));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickColor",                    (uintptr_t)&m_brickColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBrickMortarColor",              (uintptr_t)&m_mortarColor));

    }

RgbFactor const&            LxoBrickProcedure::GetBrickColor () const {return m_brickColor;}
RgbFactor&                  LxoBrickProcedure::GetBrickColorR () {return m_brickColor;}
void                        LxoBrickProcedure::SetBrickColor (double red, double green, double blue) {  m_brickColor.red = red;  m_brickColor.green = green;  m_brickColor.blue = blue;}

RgbFactor const&            LxoBrickProcedure::GetMortarColor () const {return m_mortarColor;}
RgbFactor&                  LxoBrickProcedure::GetMortarColorR () {return m_mortarColor;}
void                        LxoBrickProcedure::SetMortarColor (double red, double green, double blue) {m_mortarColor.red = red; m_mortarColor.green = green; m_mortarColor.blue = blue;}

double                      LxoBrickProcedure::GetMortarWidth () const {return m_mortarWidth;}
void                        LxoBrickProcedure::SetMortarWidth (double mortarWidth) {m_mortarWidth = mortarWidth;}

double                      LxoBrickProcedure::GetBrightnessVariation () const {return m_brightnessVariation;}
void                        LxoBrickProcedure::SetBrightnessVariation (double brightnessVariation) {m_brightnessVariation = brightnessVariation;}

double                      LxoBrickProcedure::GetHorizontalVariation () const {return m_horizontalVariation;}
void                        LxoBrickProcedure::SetHorizontalVariation (double horizontalVariation) {m_horizontalVariation = horizontalVariation;}

double                      LxoBrickProcedure::GetNoisiness () const {return m_noisiness;}
void                        LxoBrickProcedure::SetNoisiness (double noisiness) {m_noisiness = noisiness;}

double                      LxoBrickProcedure::GetAspectRatio () const {return m_aspectRatio;}
void                        LxoBrickProcedure::SetAspectRatio (double aspectRatio) {m_aspectRatio = aspectRatio;}

LxoBrickProcedure::BrickBondType LxoBrickProcedure::GetBondType () const {return m_bondType;}
void                        LxoBrickProcedure::SetBondType (LxoBrickProcedure::BrickBondType bondType) {m_bondType = bondType;}

int32_t                     LxoBrickProcedure::GetPatternId () const {return m_patternId;}
void                        LxoBrickProcedure::SetPatternId (int32_t patternId) {m_patternId = patternId;}

int32_t                     LxoBrickProcedure::GetHeaderCourseInterval () const {return m_headerCourseInterval;}
void                        LxoBrickProcedure::SetHeaderCourseInterval (int32_t headerCourseInterval) {m_headerCourseInterval = headerCourseInterval;}

bool                        LxoBrickProcedure::GetFlemishHeaders () const {return m_flemishHeaders;}
void                        LxoBrickProcedure::SetFlemishHeaders (bool flemishHeaders) {m_flemishHeaders = flemishHeaders;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoBentleyCheckerProcedure::LxoBentleyCheckerProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoBentleyCheckerProcedure::_GetType () const  { return PROCEDURETYPE_BentleyChecker; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBentleyCheckerProcedure::_InitDefaults ()
    {
    m_color1.red = 0; m_color1.green = 0; m_color1.blue = 0;
    m_color2.red = 1; m_color2.green = 1; m_color2.blue = 1;
    m_checksPerMeter = 10.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoBentleyCheckerProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_BentleyChecker != rhs.GetType ())
        return ERROR;

    LxoBentleyCheckerProcedureCP castRhs = reinterpret_cast <LxoBentleyCheckerProcedureCP> (&rhs);

    m_color1            = castRhs->m_color1;
    m_color2            = castRhs->m_color2;
    m_checksPerMeter    = castRhs->m_checksPerMeter;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoBentleyCheckerProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_BentleyChecker != rhs.GetType ())
        return false;

    LxoBentleyCheckerProcedureCP castRhs = reinterpret_cast <LxoBentleyCheckerProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_color1               , castRhs->m_color1));
    RETURN_IF_FALSE (colorsEqual (m_color2               , castRhs->m_color2));
    RETURN_IF_FALSE (valuesEqual (m_checksPerMeter       , castRhs->m_checksPerMeter));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBentleyCheckerProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBCheckerChecksPerMeter",           (uintptr_t)&m_checksPerMeter));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBCheckerColor",                     (uintptr_t)&m_color1));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBCheckerColor1",                    (uintptr_t)&m_color2));
    }

RgbFactor const&            LxoBentleyCheckerProcedure::GetColor1 () const {return m_color1;}
RgbFactor&                  LxoBentleyCheckerProcedure::GetColor1R () {return m_color1;}
void                        LxoBentleyCheckerProcedure::SetColor1 (double red, double green, double blue) {m_color1.red = red; m_color1.green = green; m_color1.blue = blue;}

RgbFactor const&            LxoBentleyCheckerProcedure::GetColor2 () const {return m_color2;}
RgbFactor&                  LxoBentleyCheckerProcedure::GetColor2R () {return m_color2;}
void                        LxoBentleyCheckerProcedure::SetColor2 (double red, double green, double blue) {m_color2.red = red; m_color2.green = green; m_color2.blue = blue;}

double                      LxoBentleyCheckerProcedure::GetChecksPerMeter () const {return m_checksPerMeter;}
void                        LxoBentleyCheckerProcedure::SetChecksPerMeter (double checksPerMeter) {m_checksPerMeter = checksPerMeter;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoBWNoiseProcedure::LxoBWNoiseProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoBWNoiseProcedure::_GetType () const { return PROCEDURETYPE_BWNoise; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBWNoiseProcedure::_InitDefaults () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoBWNoiseProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_BWNoise != rhs.GetType ())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoBWNoiseProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_BWNoise != rhs.GetType ())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBWNoiseProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoChecker3dProcedure::LxoChecker3dProcedure () : LxoProcedure () {_InitDefaults ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoChecker3dProcedure::_GetType () const { return PROCEDURETYPE_Checker3d; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoChecker3dProcedure::_InitDefaults ()
    {
    m_color1.red = 0; m_color1.green = 0; m_color1.blue = 0;
    m_color2.red = 1; m_color2.green = 1; m_color2.blue = 1;
    m_checksPerMeter = 10.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoChecker3dProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Checker3d != rhs.GetType ())
        return ERROR;

    LxoChecker3dProcedureCP castRhs = reinterpret_cast <LxoChecker3dProcedureCP> (&rhs);

    m_color1            = castRhs->m_color1;
    m_color2            = castRhs->m_color2;
    m_checksPerMeter    = castRhs->m_checksPerMeter;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoChecker3dProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Checker3d != rhs.GetType ())
        return false;

    LxoChecker3dProcedureCP castRhs = reinterpret_cast <LxoChecker3dProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_color1               , castRhs->m_color1));
    RETURN_IF_FALSE (colorsEqual (m_color2               , castRhs->m_color2));
    RETURN_IF_FALSE (valuesEqual (m_checksPerMeter       , castRhs->m_checksPerMeter));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoChecker3dProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBDCheckerChecksPerMeter",           (uintptr_t)&m_checksPerMeter));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBDCheckerColor",                     (uintptr_t)&m_color1));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBDCheckerColor1",                    (uintptr_t)&m_color2));
    }

RgbFactor const&                LxoChecker3dProcedure::GetColor1 () const {return m_color1;}
RgbFactor&                      LxoChecker3dProcedure::GetColor1R () {return m_color1;}
void                            LxoChecker3dProcedure::SetColor1 (double red, double green, double blue) {m_color1.red = red; m_color1.green = green; m_color1.blue = blue;}

RgbFactor const&                LxoChecker3dProcedure::GetColor2 () const {return m_color2;}
RgbFactor&                      LxoChecker3dProcedure::GetColor2R () {return m_color2;}
void                            LxoChecker3dProcedure::SetColor2 (double red, double green, double blue) {m_color2.red = red; m_color2.green = green; m_color2.blue = blue;}

double                          LxoChecker3dProcedure::GetChecksPerMeter () const {return m_checksPerMeter;}
void                            LxoChecker3dProcedure::SetChecksPerMeter (double checksPerMeter) {m_checksPerMeter = checksPerMeter;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoCloudsProcedure::LxoCloudsProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoCloudsProcedure::_GetType () const { return PROCEDURETYPE_Clouds; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoCloudsProcedure::_InitDefaults ()
    {
    m_cloudColor.red = 0.92; m_cloudColor.green = 0.92; m_cloudColor.blue = 0.92;
    m_skyColor.red = 0.53; m_skyColor.green = 0.81; m_skyColor.blue = 0.92;

    m_thickness     = 100.0;
    m_complexity    = 4.0;
    m_noise         = 2.0;
    m_cloudsOnly    = FALSE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoCloudsProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Clouds != rhs.GetType ())
        return ERROR;

    LxoCloudsProcedureCP castRhs = reinterpret_cast <LxoCloudsProcedureCP> (&rhs);

    m_cloudColor        = castRhs->m_cloudColor;
    m_skyColor          = castRhs->m_skyColor;

    m_thickness         = castRhs->m_thickness;
    m_complexity        = castRhs->m_complexity;
    m_noise             = castRhs->m_noise;
    m_cloudsOnly        = castRhs->m_cloudsOnly;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoCloudsProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Clouds != rhs.GetType ())
        return false;

    LxoCloudsProcedureCP castRhs = reinterpret_cast <LxoCloudsProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_cloudColor  , castRhs->m_cloudColor));
    RETURN_IF_FALSE (colorsEqual (m_skyColor    , castRhs->m_skyColor));
    RETURN_IF_FALSE (valuesEqual (m_thickness   , castRhs->m_thickness));
    RETURN_IF_FALSE (valuesEqual (m_complexity  , castRhs->m_complexity));
    RETURN_IF_FALSE (valuesEqual (m_noise       , castRhs->m_noise));
    RETURN_IF_FALSE (m_cloudsOnly  == castRhs->m_cloudsOnly);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoCloudsProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCloudsThickness",           (uintptr_t)&m_thickness));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCloudsComplexity",          (uintptr_t)&m_complexity));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCloudsNoise",               (uintptr_t)&m_noise));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoCloudsCloudsOnly",          (uintptr_t)&m_cloudsOnly));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCloudsColor",               (uintptr_t)&m_cloudColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoCloudsSkyColor",            (uintptr_t)&m_skyColor));
    }

RgbFactor const&                LxoCloudsProcedure::GetCloudColor () const {return m_cloudColor;}
RgbFactor&                      LxoCloudsProcedure::GetCloudColorR () {return m_cloudColor;}
void                            LxoCloudsProcedure::SetCloudColor (double red, double green, double blue) {m_cloudColor.red = red; m_cloudColor.green = green; m_cloudColor.blue = blue;}

RgbFactor const&                LxoCloudsProcedure::GetSkyColor () const {return m_skyColor;}
RgbFactor&                      LxoCloudsProcedure::GetSkyColorR () {return m_skyColor;}
void                            LxoCloudsProcedure::SetSkyColor (double red, double green, double blue) {m_skyColor.red = red; m_skyColor.green = green; m_skyColor.blue = blue;}

double                          LxoCloudsProcedure::GetThickness () const {return m_thickness;}
void                            LxoCloudsProcedure::SetThickness (double thickness) {m_thickness = thickness;}

double                          LxoCloudsProcedure::GetComplexity () const {return m_complexity;}
void                            LxoCloudsProcedure::SetComplexity (double complexity) {m_complexity = complexity;}

double                          LxoCloudsProcedure::GetNoise () const {return m_noise;}
void                            LxoCloudsProcedure::SetNoise (double noise) {m_noise = noise;}

bool                            LxoCloudsProcedure::GetCloudsOnly () const {return m_cloudsOnly;}
void                            LxoCloudsProcedure::SetCloudsOnly (bool cloudsOnly) {m_cloudsOnly = cloudsOnly;}

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoColorNoiseProcedure::LxoColorNoiseProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoColorNoiseProcedure::_GetType () const { return PROCEDURETYPE_ColorNoise; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoColorNoiseProcedure::_InitDefaults () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoColorNoiseProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_ColorNoise != rhs.GetType ())
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoColorNoiseProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_ColorNoise != rhs.GetType ())
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoColorNoiseProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFlameProcedure::LxoFlameProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoFlameProcedure::_GetType () const { return PROCEDURETYPE_Flame; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFlameProcedure::_InitDefaults ()
    {
    m_color.red = 1.0; m_color.green = 0.651; m_color.blue = 0.0;
    m_flameHeight = 67.0;
    m_flameWidth  = 33.0;
    m_turbulence  = 6;
    m_complexity  = 6.0;
    m_flickerSpeed= 0.1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoFlameProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Flame != rhs.GetType ())
        return ERROR;

    LxoFlameProcedureCP castRhs = reinterpret_cast <LxoFlameProcedureCP> (&rhs);
    m_color         = castRhs->m_color;
    m_flameHeight   = castRhs->m_flameHeight;
    m_flameWidth    = castRhs->m_flameWidth;
    m_turbulence    = castRhs->m_turbulence;
    m_complexity    = castRhs->m_complexity;
    m_flickerSpeed  = castRhs->m_flickerSpeed;

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoFlameProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Flame != rhs.GetType ())
        return false;

    LxoFlameProcedureCP castRhs = reinterpret_cast <LxoFlameProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_color               , castRhs->m_color));
    RETURN_IF_FALSE (valuesEqual (m_flameHeight        , castRhs->m_flameHeight));
    RETURN_IF_FALSE (valuesEqual (m_flameWidth         , castRhs->m_flameWidth));
    RETURN_IF_FALSE (valuesEqual (m_turbulence         , castRhs->m_turbulence));
    RETURN_IF_FALSE (valuesEqual (m_complexity         , castRhs->m_complexity));
    RETURN_IF_FALSE (valuesEqual (m_flickerSpeed       , castRhs->m_flickerSpeed));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFlameProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFlameHeight",               (uintptr_t)&m_flameHeight));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFlameWidth",                (uintptr_t)&m_flameWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFlameTurbulence",           (uintptr_t)&m_turbulence));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFlameComplexity",           (uintptr_t)&m_complexity));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFlameFlickerSpeed",         (uintptr_t)&m_flickerSpeed));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFlameColor",                 (uintptr_t)&m_color));
    }

RgbFactor const&                LxoFlameProcedure::GetColor () const {return m_color;}
RgbFactor&                      LxoFlameProcedure::GetColorR () {return m_color;}
void                            LxoFlameProcedure::SetColor (double red, double green, double blue) { m_color.red = red; m_color.green = green; m_color.blue = blue;}

double                          LxoFlameProcedure::GetFlameHeight () const {return m_flameHeight;}
void                            LxoFlameProcedure::SetFlameHeight (double flameHeight) {m_flameHeight = flameHeight;}

double                          LxoFlameProcedure::GetFlameWidth () const {return m_flameWidth;}
void                            LxoFlameProcedure::SetFlameWidth (double flameWidth) {m_flameWidth = flameWidth;}

double                          LxoFlameProcedure::GetTurbulence () const {return m_turbulence;}
void                            LxoFlameProcedure::SetTurbulence (double turbulence) {m_turbulence = turbulence;}

double                          LxoFlameProcedure::GetComplexity () const {return m_complexity;}
void                            LxoFlameProcedure::SetComplexity (double complexity) {m_complexity = complexity;}

double                          LxoFlameProcedure::GetFlickerSpeed () const {return m_flickerSpeed;}
void                            LxoFlameProcedure::SetFlickerSpeed (double flickerSpeed) {m_flickerSpeed = flickerSpeed;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoFogProcedure::LxoFogProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoFogProcedure:: _GetType () const { return PROCEDURETYPE_Fog; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFogProcedure::_InitDefaults ()
    {
    m_color.red = 1.0; m_color.green = 0.902; m_color.blue = 0.8;
    m_minDensity  = 10;
    m_maxDensity  = 100;
    m_driftSpeed  = 0.3;
    m_swirlSpeed  = 90;
    m_thickness   = 1.0;
    m_contrast    = 1.0;
    m_complexity  = 3.0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoFogProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Fog != rhs.GetType ())
        return ERROR;

    LxoFogProcedureCP castRhs = reinterpret_cast <LxoFogProcedureCP> (&rhs);

     m_color        = castRhs->m_color;
     m_minDensity   = castRhs->m_minDensity;
     m_maxDensity   = castRhs->m_maxDensity;
     m_driftSpeed   = castRhs->m_driftSpeed;
     m_swirlSpeed   = castRhs->m_swirlSpeed;
     m_thickness    = castRhs->m_thickness;
     m_contrast     = castRhs->m_contrast;
     m_complexity   = castRhs->m_complexity;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoFogProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Fog != rhs.GetType ())
        return false;

    LxoFogProcedureCP castRhs = reinterpret_cast <LxoFogProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_color        , castRhs->m_color));
    RETURN_IF_FALSE (valuesEqual (m_minDensity   , castRhs->m_minDensity));
    RETURN_IF_FALSE (valuesEqual (m_maxDensity   , castRhs->m_maxDensity));
    RETURN_IF_FALSE (valuesEqual (m_driftSpeed   , castRhs->m_driftSpeed));
    RETURN_IF_FALSE (valuesEqual (m_swirlSpeed   , castRhs->m_swirlSpeed));
    RETURN_IF_FALSE (valuesEqual (m_thickness    , castRhs->m_thickness ));
    RETURN_IF_FALSE (valuesEqual (m_contrast     , castRhs->m_contrast  ));
    RETURN_IF_FALSE (valuesEqual (m_complexity   , castRhs->m_complexity));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoFogProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogMinDensity",              (uintptr_t)&m_minDensity));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogMaxDensity",              (uintptr_t)&m_maxDensity));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogDriftSpeed",              (uintptr_t)&m_driftSpeed));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogSwirlSpeed",              (uintptr_t)&m_swirlSpeed));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogThickness",               (uintptr_t)&m_thickness));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogContrast",                (uintptr_t)&m_contrast));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogComplexity",              (uintptr_t)&m_complexity));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoFogColor",                    (uintptr_t)&m_color));
    }

RgbFactor const&            LxoFogProcedure::GetColor () const {return m_color;}
RgbFactor&                  LxoFogProcedure::GetColorR () {return m_color;}
void                        LxoFogProcedure::SetColor (double red, double green, double blue) {m_color.red = red; m_color.green = green; m_color.blue = blue;}

double                      LxoFogProcedure::GetMinDensity () const {return m_minDensity;}
void                        LxoFogProcedure::SetMinDensity (double minDensity) {m_minDensity = minDensity;}

double                      LxoFogProcedure::GetMaxDensity () const {return m_maxDensity;}
void                        LxoFogProcedure::SetMaxDensity (double maxDensity) {m_maxDensity = maxDensity;}

double                      LxoFogProcedure::GetDriftSpeed () const {return m_driftSpeed;}
void                        LxoFogProcedure::SetDriftSpeed (double driftSpeed) {m_driftSpeed = driftSpeed;}

double                      LxoFogProcedure::GetSwirlSpeed () const {return m_swirlSpeed;}
void                        LxoFogProcedure::SetSwirlSpeed (double swirlSpeed) {m_swirlSpeed = swirlSpeed;}

double                      LxoFogProcedure::GetThickness () const {return m_thickness;}
void                        LxoFogProcedure::SetThickness (double thickness) {m_thickness = thickness;}

double                      LxoFogProcedure::GetContrast () const {return m_contrast;}
void                        LxoFogProcedure::SetContrast (double contrast) {m_contrast = contrast;}

double                      LxoFogProcedure::GetComplexity () const {return m_complexity;}
void                        LxoFogProcedure::SetComplexity (double complexity) {m_complexity = complexity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoMarbleProcedure::LxoMarbleProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoMarbleProcedure::_GetType () const { return PROCEDURETYPE_Marble; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoMarbleProcedure::_InitDefaults ()
    {
    m_marbleColor.red = 1.0; m_marbleColor.green = 1.0; m_marbleColor.blue = 1.0;
    m_veinColor.red = 0.2; m_veinColor.green = 0.1; m_veinColor.blue = 0.1;
    m_veinTightness = 71.0;
    m_complexity = 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoMarbleProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Marble != rhs.GetType ())
        return ERROR;

    LxoMarbleProcedureCP castRhs = reinterpret_cast <LxoMarbleProcedureCP> (&rhs);

    m_marbleColor       = castRhs->m_marbleColor;
    m_veinColor         = castRhs->m_veinColor;
    m_veinTightness     = castRhs->m_veinTightness;
    m_complexity        = castRhs->m_complexity;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoMarbleProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Marble != rhs.GetType ())
        return false;

    LxoMarbleProcedureCP castRhs = reinterpret_cast <LxoMarbleProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_marbleColor         , castRhs->m_marbleColor));
    RETURN_IF_FALSE (colorsEqual (m_veinColor           , castRhs->m_veinColor));
    RETURN_IF_FALSE (valuesEqual (m_veinTightness       , castRhs->m_veinTightness));
    RETURN_IF_FALSE (valuesEqual (m_complexity          , castRhs->m_complexity));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoMarbleProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoMarbleMarbleColor",                  (uintptr_t)&m_marbleColor));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoMarbleVeinColor",                    (uintptr_t)&m_veinColor));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoMarbleVeinTightness",                 (uintptr_t)&m_veinTightness));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoMarbleComplexity",                    (uintptr_t)&m_complexity));
    }

RgbFactor const&            LxoMarbleProcedure::GetMarbleColor () const {return m_marbleColor;}
RgbFactor&                  LxoMarbleProcedure::GetMarbleColorR () {return m_marbleColor;}
void                        LxoMarbleProcedure::SetMarbleColor (double red, double green, double blue) {m_marbleColor.red = red; m_marbleColor.green = green; m_marbleColor.blue = blue;}

RgbFactor const&            LxoMarbleProcedure::GetVeinColor () const {return m_veinColor;}
RgbFactor&                  LxoMarbleProcedure::GetVeinColorR () {return m_veinColor;}
void                        LxoMarbleProcedure::SetVeinColor (double red, double green, double blue) {m_veinColor.red = red; m_veinColor.green = green; m_veinColor.blue = blue;}

double                      LxoMarbleProcedure::GetVeinTightness () const {return m_veinTightness;}
void                        LxoMarbleProcedure::SetVeinTightness (double veinTightness) {m_veinTightness = veinTightness;}

double                      LxoMarbleProcedure::GetComplexity () const {return m_complexity;}
void                        LxoMarbleProcedure::SetComplexity (double complexity) {m_complexity = complexity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoRGBColorCubeProcedure::LxoRGBColorCubeProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoRGBColorCubeProcedure::_GetType () const { return PROCEDURETYPE_RGBColorCube; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoRGBColorCubeProcedure::_InitDefaults () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoRGBColorCubeProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_RGBColorCube != rhs.GetType ())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoRGBColorCubeProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_RGBColorCube != rhs.GetType ())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoRGBColorCubeProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoSandProcedure::LxoSandProcedure (): LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoSandProcedure::_GetType () const  { return PROCEDURETYPE_Sand; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoSandProcedure::_InitDefaults ()
    {
    m_sandColor.red = 0.961; m_sandColor.green = 0.890; m_sandColor.blue = 0.761;
    m_contrastColor.red = 0.839; m_contrastColor.green = 0.8; m_contrastColor.blue = 0.631;
    m_fraction = 10.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoSandProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Sand != rhs.GetType ())
        return ERROR;

    LxoSandProcedureCP castRhs = reinterpret_cast <LxoSandProcedureCP> (&rhs);

    m_sandColor     = castRhs->m_sandColor;
    m_contrastColor = castRhs->m_contrastColor;
    m_fraction      = castRhs->m_fraction;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoSandProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Sand != rhs.GetType ())
        return false;

    LxoSandProcedureCP castRhs = reinterpret_cast <LxoSandProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_sandColor           , castRhs->m_sandColor));
    RETURN_IF_FALSE (colorsEqual (m_contrastColor       , castRhs->m_contrastColor));
    RETURN_IF_FALSE (valuesEqual (m_fraction            , castRhs->m_fraction));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoSandProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoSandFraction",             (uintptr_t)&m_fraction));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoSandColor",                 (uintptr_t)&m_sandColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoSandContrastColor",         (uintptr_t)&m_contrastColor));
    }

RgbFactor const&            LxoSandProcedure::GetSandColor () const {return m_sandColor;}
RgbFactor&                  LxoSandProcedure::GetSandColorR () {return m_sandColor;}
void                        LxoSandProcedure::SetSandColor (double red, double green, double blue) {m_sandColor.red = red; m_sandColor.green = green; m_sandColor.blue = blue;}

RgbFactor const&            LxoSandProcedure::GetContrastColor () const {return m_contrastColor;}
RgbFactor&                  LxoSandProcedure::GetContrastColorR () {return m_contrastColor;}
void                        LxoSandProcedure::SetContrastColor (double red, double green, double blue) { m_contrastColor.red = red; m_contrastColor.green = green; m_contrastColor.blue = blue;}

double                      LxoSandProcedure::GetFraction () const {return m_fraction;}
void                        LxoSandProcedure::SetFraction (double fraction) {m_fraction = fraction;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoStoneProcedure::LxoStoneProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoStoneProcedure::_GetType () const  { return PROCEDURETYPE_Stone; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoStoneProcedure::_InitDefaults ()
    {
    m_tintColor.red = 0.749; m_tintColor.green = 0.749; m_tintColor.blue = 0.749;
    m_mortarColor.red = 0.902; m_mortarColor.green = 0.902; m_mortarColor.blue = 0.902;

    m_stoneColors     = 6;
    m_stoneColorOffset= 0;
    m_mortarWidth     = 10.0;
    m_noisiness       = 30.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoStoneProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Stone != rhs.GetType ())
        return ERROR;

    LxoStoneProcedureCP castRhs = reinterpret_cast <LxoStoneProcedureCP> (&rhs);

    m_tintColor         = castRhs->m_tintColor;
    m_mortarColor       = castRhs->m_mortarColor;
    m_stoneColors       = castRhs->m_stoneColors;
    m_stoneColorOffset  = castRhs->m_stoneColorOffset;
    m_mortarWidth       = castRhs->m_mortarWidth;
    m_noisiness         = castRhs->m_noisiness;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoStoneProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Stone != rhs.GetType ())
        return false;

    LxoStoneProcedureCP castRhs = reinterpret_cast <LxoStoneProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_tintColor          , castRhs->m_tintColor));
    RETURN_IF_FALSE (colorsEqual (m_mortarColor        , castRhs->m_mortarColor));
    RETURN_IF_FALSE (valuesEqual (m_mortarWidth        , castRhs->m_mortarWidth));
    RETURN_IF_FALSE (valuesEqual (m_noisiness          , castRhs->m_noisiness));
    RETURN_IF_FALSE ( m_stoneColors       == castRhs->m_stoneColors);
    RETURN_IF_FALSE (m_stoneColorOffset  == castRhs->m_stoneColorOffset);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoStoneProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoStoneStoneColors",             (uintptr_t)&m_stoneColors));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoStoneStoneColorOffset",        (uintptr_t)&m_stoneColorOffset));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoStoneMortarWidth",             (uintptr_t)&m_mortarWidth));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoStoneNoisiness",               (uintptr_t)&m_noisiness));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoStoneTintColor",                (uintptr_t)&m_tintColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoStoneMortarColor",              (uintptr_t)&m_mortarColor));

    }

RgbFactor const&            LxoStoneProcedure::GetTintColor () const {return m_tintColor;}
RgbFactor&                  LxoStoneProcedure::GetTintColorR () {return m_tintColor;}
void                        LxoStoneProcedure::SetTintColor (double red, double green, double blue) {m_tintColor.red = red; m_tintColor.green = green; m_tintColor.blue = blue;}

RgbFactor const&            LxoStoneProcedure::GetMortarColor () const {return m_mortarColor;}
RgbFactor&                  LxoStoneProcedure::GetMortarColorR () {return m_mortarColor;}
void                        LxoStoneProcedure::SetMortarColor (double red, double green, double blue) {m_mortarColor.red = red; m_mortarColor.green = green; m_mortarColor.blue = blue;}

int32_t                     LxoStoneProcedure::GetStoneColors () const {return m_stoneColors;}
void                        LxoStoneProcedure::SetStoneColors (int32_t stoneColors) {m_stoneColors = stoneColors;}

int32_t                     LxoStoneProcedure::GetStoneColorOffset () const {return m_stoneColorOffset;}
void                        LxoStoneProcedure::SetStoneColorOffset (int32_t stoneColorOffset) {m_stoneColorOffset = stoneColorOffset;}

double                      LxoStoneProcedure::GetMortarWidth () const {return m_mortarWidth;}
void                        LxoStoneProcedure::SetMortarWidth (double mortarWidth) {m_mortarWidth = mortarWidth;}

double                      LxoStoneProcedure::GetNoisiness () const {return m_noisiness;}
void                        LxoStoneProcedure::SetNoisiness (double noisiness) {m_noisiness = noisiness;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoTurbulenceProcedure::LxoTurbulenceProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoTurbulenceProcedure::_GetType () const { return PROCEDURETYPE_Turbulence; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoTurbulenceProcedure::_InitDefaults ()
    {
    m_complexity = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoTurbulenceProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Turbulence != rhs.GetType ())
        return ERROR;

    LxoTurbulenceProcedureCP castRhs = reinterpret_cast <LxoTurbulenceProcedureCP> (&rhs);
    m_complexity = castRhs->m_complexity;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoTurbulenceProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Turbulence != rhs.GetType ())
        return false;

    LxoTurbulenceProcedureCP castRhs = reinterpret_cast <LxoTurbulenceProcedureCP> (&rhs);

    return m_complexity == castRhs->m_complexity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoTurbulenceProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoTurbulenceComplexity",           (uintptr_t)&m_complexity));
    }

double              LxoTurbulenceProcedure::GetComplexity () const {return m_complexity;}
void                LxoTurbulenceProcedure::SetComplexity (double complexity) {m_complexity = complexity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoTurfProcedure::LxoTurfProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoTurfProcedure::_GetType () const { return PROCEDURETYPE_Turf; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoTurfProcedure::_InitDefaults ()
    {
    m_turfColor.red = 0.0; m_turfColor.green = 0.369; m_turfColor.blue = 0.0;
    m_contrastColor.red = 0.761; m_contrastColor.green = 0.761; m_contrastColor.blue = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoTurfProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Turbulence != rhs.GetType ())
        return ERROR;

    LxoTurfProcedureCP castRhs = reinterpret_cast <LxoTurfProcedureCP> (&rhs);

    m_turfColor     = castRhs->m_turfColor;
    m_contrastColor = castRhs->m_contrastColor;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoTurfProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Turbulence != rhs.GetType ())
        return false;

    LxoTurfProcedureCP castRhs = reinterpret_cast <LxoTurfProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_turfColor             , castRhs->m_turfColor));
    RETURN_IF_FALSE (colorsEqual (m_contrastColor         , castRhs->m_contrastColor));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoTurfProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoTurfColor",            (uintptr_t)&m_turfColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoTurfContrastColor",    (uintptr_t)&m_contrastColor));
    }

RgbFactor const&            LxoTurfProcedure::GetTurfColor () const {return m_turfColor;}
RgbFactor&                  LxoTurfProcedure::GetTurfColorR () {return m_turfColor;}
void                        LxoTurfProcedure::SetTurfColor (double red, double green, double blue) {m_turfColor.red = red; m_turfColor.green = green; m_turfColor.blue = blue;}

RgbFactor const&            LxoTurfProcedure::GetContrastColor () const {return m_contrastColor;}
RgbFactor&                  LxoTurfProcedure::GetContrastColorR () {return m_contrastColor;}
void                        LxoTurfProcedure::SetContrastColor (double red, double green, double blue) {m_contrastColor.red = red; m_contrastColor.green = green; m_contrastColor.blue = blue;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoWaterProcedure::LxoWaterProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoWaterProcedure::_GetType () const { return PROCEDURETYPE_Water; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWaterProcedure::_InitDefaults ()
    {
    m_waterColor.red = 0.47; m_waterColor.green = 0.84; m_waterColor.blue = 1.0;
    m_rippleScale         = 1.0;
    m_rippleComplexity    = 4.0;
    m_waveScale           = 1.0;
    m_waveComplexity      = 2.0;
    m_waveMinimum         = 0.5;
    m_ripplesPerWave      = 10.0;
    m_roughness           = 2.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoWaterProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Water != rhs.GetType ())
        return ERROR;

    LxoWaterProcedureCP castRhs = reinterpret_cast <LxoWaterProcedureCP> (&rhs);

    m_waterColor         = castRhs->m_waterColor;
    m_rippleScale        = castRhs->m_rippleScale;
    m_rippleComplexity   = castRhs->m_rippleComplexity;
    m_waveScale          = castRhs->m_waveScale;
    m_waveComplexity     = castRhs->m_waveComplexity;
    m_waveMinimum        = castRhs->m_waveMinimum;
    m_ripplesPerWave     = castRhs->m_ripplesPerWave;
    m_roughness          = castRhs->m_roughness;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoWaterProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Water != rhs.GetType ())
        return false;

    LxoWaterProcedureCP castRhs = reinterpret_cast <LxoWaterProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_waterColor          , castRhs->m_waterColor));
    RETURN_IF_FALSE (valuesEqual (m_rippleScale         , castRhs->m_rippleScale));
    RETURN_IF_FALSE (valuesEqual (m_rippleComplexity    , castRhs->m_rippleComplexity));
    RETURN_IF_FALSE (valuesEqual (m_waveScale           , castRhs->m_waveScale));
    RETURN_IF_FALSE (valuesEqual (m_waveComplexity      , castRhs->m_waveComplexity));
    RETURN_IF_FALSE (valuesEqual (m_waveMinimum         , castRhs->m_waveMinimum));
    RETURN_IF_FALSE (valuesEqual (m_ripplesPerWave      , castRhs->m_ripplesPerWave));
    RETURN_IF_FALSE (valuesEqual (m_roughness           , castRhs->m_roughness));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWaterProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterRippleScale",          (uintptr_t)&m_rippleScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterRippleComplexity",     (uintptr_t)&m_rippleComplexity));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterWaveScale",            (uintptr_t)&m_waveScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterWaveComplexity",       (uintptr_t)&m_waveComplexity));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterWaveMinimum",          (uintptr_t)&m_waveMinimum));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterRipplesPerWave",       (uintptr_t)&m_ripplesPerWave));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterRoughness",            (uintptr_t)&m_roughness));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWaterColor",                 (uintptr_t)&m_waterColor));
    }

RgbFactor const&            LxoWaterProcedure::GetWaterColor () const {return m_waterColor;}
RgbFactor&                  LxoWaterProcedure::GetWaterColorR () {return m_waterColor;}
void                        LxoWaterProcedure::SetWaterColor (double red, double green, double blue) { m_waterColor.red = red; m_waterColor.green = green; m_waterColor.blue = blue;}

double                      LxoWaterProcedure::GetRippleScale () const {return m_rippleScale;}
void                        LxoWaterProcedure::SetRippleScale (double rippleScale) {m_rippleScale = rippleScale;}

double                      LxoWaterProcedure::GetRippleComplexity () const {return m_rippleComplexity;}
void                        LxoWaterProcedure::SetRippleComplexity (double rippleComplexity) {m_rippleComplexity = rippleComplexity;}

double                      LxoWaterProcedure::GetWaveScale () const {return m_waveScale;}
void                        LxoWaterProcedure::SetWaveScale (double waveScale) {m_waveScale = waveScale;}

double                      LxoWaterProcedure::GetWaveComplexity () const {return m_waveComplexity;}
void                        LxoWaterProcedure::SetWaveComplexity (double waveComplexity) {m_waveComplexity = waveComplexity;}

double                      LxoWaterProcedure::GetWaveMinimum () const {return m_waveMinimum;}
void                        LxoWaterProcedure::SetWaveMinimum (double waveMinimum) {m_waveMinimum = waveMinimum;}

double                      LxoWaterProcedure::GetRipplesPerWave () const {return m_ripplesPerWave;}
void                        LxoWaterProcedure::SetRipplesPerWave (double ripplesPerWave) {m_ripplesPerWave = ripplesPerWave;}

double                      LxoWaterProcedure::GetRoughness () const {return m_roughness;}
void                        LxoWaterProcedure::SetRoughness (double roughness) {m_roughness = roughness;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoWavesProcedure::LxoWavesProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoWavesProcedure::_GetType () const  { return PROCEDURETYPE_Waves; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWavesProcedure::_InitDefaults ()
    {
    m_wavesColor.red = 0.0; m_wavesColor.green = 0.9; m_wavesColor.blue = 0.9;
    m_contrastColor.red = 0.0; m_contrastColor.green = 0.8; m_contrastColor.blue = 0.85;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoWavesProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Waves != rhs.GetType ())
        return ERROR;

    LxoWavesProcedureCP castRhs = reinterpret_cast <LxoWavesProcedureCP> (&rhs);

    m_wavesColor    = castRhs->m_wavesColor;
    m_contrastColor = castRhs->m_contrastColor;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoWavesProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Waves != rhs.GetType ())
        return false;

    LxoWavesProcedureCP castRhs = reinterpret_cast <LxoWavesProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_wavesColor             , castRhs->m_wavesColor));
    RETURN_IF_FALSE (colorsEqual (m_contrastColor          , castRhs->m_contrastColor));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoWavesProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWavesColor",            (uintptr_t)&m_wavesColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoWavesContrastColor",    (uintptr_t)&m_contrastColor));
    }

RgbFactor const&            LxoWavesProcedure::GetWavesColor () const {return m_wavesColor;}
RgbFactor&                  LxoWavesProcedure::GetWavesColorR () {return m_wavesColor;}
void                        LxoWavesProcedure::SetWavesColor (double red, double green, double blue) {m_wavesColor.red = red; m_wavesColor.green = green; m_wavesColor.blue = blue;}

RgbFactor const&            LxoWavesProcedure::GetContrastColor () const {return m_contrastColor;}
RgbFactor&                  LxoWavesProcedure::GetContrastColorR () {return m_contrastColor;}
void                        LxoWavesProcedure::SetContrastColor (double red, double green, double blue) {m_contrastColor.red = red; m_contrastColor.green = green; m_contrastColor.blue = blue;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoBentleyWoodProcedure::LxoBentleyWoodProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoBentleyWoodProcedure::_GetType () const  { return PROCEDURETYPE_BentleyWood; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBentleyWoodProcedure::_InitDefaults ()
    {
    m_woodColor.red = 0.67; m_woodColor.green = 0.38; m_woodColor.blue = 0.161;
    m_ringColor.red = 0.439; m_ringColor.green = 0.231; m_ringColor.blue = 0.090;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoBentleyWoodProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_BentleyWood != rhs.GetType ())
        return ERROR;

    LxoBentleyWoodProcedureCP castRhs = reinterpret_cast <LxoBentleyWoodProcedureCP> (&rhs);

    m_woodColor = castRhs->m_woodColor;
    m_ringColor = castRhs->m_ringColor;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoBentleyWoodProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_BentleyWood != rhs.GetType ())
        return false;

    LxoBentleyWoodProcedureCP castRhs = reinterpret_cast <LxoBentleyWoodProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_woodColor     , castRhs->m_woodColor));
    RETURN_IF_FALSE (colorsEqual (m_ringColor     , castRhs->m_ringColor));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoBentleyWoodProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBWoodColor",            (uintptr_t)&m_woodColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoBWoodRingColor",        (uintptr_t)&m_ringColor));
    }

RgbFactor const&            LxoBentleyWoodProcedure::GetWoodColor () const {return m_woodColor;}
RgbFactor&                  LxoBentleyWoodProcedure::GetWoodColorR () {return m_woodColor;}
void                        LxoBentleyWoodProcedure::SetWoodColor (double red, double green, double blue) {m_woodColor.red = red; m_woodColor.green = green; m_woodColor.blue = blue;}

RgbFactor const&            LxoBentleyWoodProcedure::GetRingColor () const {return m_ringColor;}
RgbFactor&                  LxoBentleyWoodProcedure::GetRingColorR () {return m_ringColor;}
void                        LxoBentleyWoodProcedure::SetRingColor (double red, double green, double blue) {m_ringColor.red = red; m_ringColor.green = green; m_ringColor.blue = blue;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoAdvancedWoodProcedure::LxoAdvancedWoodProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoAdvancedWoodProcedure::_GetType () const  { return PROCEDURETYPE_AdvancedWood; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoAdvancedWoodProcedure::_InitDefaults ()
    {
    m_woodColor.red = 0.8; m_woodColor.green = 0.8; m_woodColor.blue = 0.6;
    m_ringColor.red = 0.6; m_ringColor.green = 0.6; m_ringColor.blue = 0.451;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoAdvancedWoodProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_AdvancedWood != rhs.GetType ())
        return ERROR;

    LxoAdvancedWoodProcedureCP castRhs = reinterpret_cast <LxoAdvancedWoodProcedureCP> (&rhs);

    m_woodColor = castRhs->m_woodColor;
    m_ringColor = castRhs->m_ringColor;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoAdvancedWoodProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_AdvancedWood != rhs.GetType ())
        return false;

    LxoAdvancedWoodProcedureCP castRhs = reinterpret_cast <LxoAdvancedWoodProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_woodColor     , castRhs->m_woodColor));
    RETURN_IF_FALSE (colorsEqual (m_ringColor     , castRhs->m_ringColor));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoAdvancedWoodProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoAWoodColor",            (uintptr_t)&m_woodColor));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoAWoodRingColor",        (uintptr_t)&m_ringColor));
    }

RgbFactor const&            LxoAdvancedWoodProcedure::GetWoodColor () const {return m_woodColor;}
RgbFactor&                  LxoAdvancedWoodProcedure::GetWoodColorR () {return m_woodColor;}
void                        LxoAdvancedWoodProcedure::SetWoodColor (double red, double green, double blue) {m_woodColor.red = red; m_woodColor.green = green; m_woodColor.blue = blue;}

RgbFactor const&            LxoAdvancedWoodProcedure::GetRingColor () const {return m_ringColor;}
RgbFactor&                  LxoAdvancedWoodProcedure::GetRingColorR () {return m_ringColor;}
void                        LxoAdvancedWoodProcedure::SetRingColor (double red, double green, double blue) {m_ringColor.red = red; m_ringColor.green = green; m_ringColor.blue = blue;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoOcclusionProcedure::LxoOcclusionProcedure () : LxoProcedure ()
    {
    _InitDefaults ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
LxoProcedure::ProcedureType LxoOcclusionProcedure::_GetType () const  { return PROCEDURETYPE_Occlusion; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoOcclusionProcedure::_InitDefaults ()
    {
    m_color1.red = 0.0; m_color1.green = 0.0; m_color1.blue = 0.0;
    m_color2.red = 1.0; m_color2.green = 1.0; m_color2.blue = 1.0;

    m_alpha1           = 100.0;
    m_alpha2           = 100.0;
    m_value1           = 0.0;
    m_value2           = 100.0;
    m_occlusionDistance = .2; // Meters
    m_variance         = 0.0;
    m_varianceScale    = 1.0; // Meters
    m_spreadAngle      = 45.0; // degrees
    m_maxCavityAngle   = 45.0; // degrees
    m_bias             = 50.0;
    m_gain             = 50.0;
    m_occType             = LXOOCCLUSIONTYPE_Uniform;
    m_occlusionRays    = 32;
    m_sameSurface       = FALSE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LxoOcclusionProcedure::_Copy (LxoProcedureCR rhs)
    {
    if (LxoProcedure::PROCEDURETYPE_Occlusion != rhs.GetType ())
        return ERROR;

    LxoOcclusionProcedureCP castRhs = reinterpret_cast <LxoOcclusionProcedureCP> (&rhs);

    m_color1            = castRhs->m_color1;
    m_alpha1            = castRhs->m_alpha1;
    m_color2            = castRhs->m_color2;
    m_alpha2            = castRhs->m_alpha2;
    m_value1            = castRhs->m_value1;
    m_value2            = castRhs->m_value2;
    m_occType           = castRhs->m_occType;
    m_occlusionRays     = castRhs->m_occlusionRays;
    m_occlusionDistance = castRhs->m_occlusionDistance;
    m_variance          = castRhs->m_variance;
    m_varianceScale     = castRhs->m_varianceScale;
    m_spreadAngle       = castRhs->m_spreadAngle;
    m_maxCavityAngle    = castRhs->m_maxCavityAngle;
    m_sameSurface       = castRhs->m_sameSurface;
    m_bias              = castRhs->m_bias;
    m_gain              = castRhs->m_gain;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool LxoOcclusionProcedure::_Equals (LxoProcedureCR rhs) const
    {
    if (LxoProcedure::PROCEDURETYPE_Occlusion != rhs.GetType ())
        return false;

    LxoOcclusionProcedureCP castRhs = reinterpret_cast <LxoOcclusionProcedureCP> (&rhs);

    RETURN_IF_FALSE (colorsEqual (m_color1              , castRhs->m_color1));
    RETURN_IF_FALSE (colorsEqual (m_color2              , castRhs->m_color2));
    RETURN_IF_FALSE (valuesEqual (m_alpha1               , castRhs->m_alpha1));
    RETURN_IF_FALSE (valuesEqual (m_alpha2               , castRhs->m_alpha2));
    RETURN_IF_FALSE (valuesEqual (m_value1               , castRhs->m_value1));
    RETURN_IF_FALSE (valuesEqual (m_value2               , castRhs->m_value2));
    RETURN_IF_FALSE (valuesEqual (m_occlusionDistance    , castRhs->m_occlusionDistance));
    RETURN_IF_FALSE (valuesEqual (m_variance             , castRhs->m_variance));
    RETURN_IF_FALSE (valuesEqual (m_varianceScale        , castRhs->m_varianceScale));
    RETURN_IF_FALSE (valuesEqual (m_spreadAngle          , castRhs->m_spreadAngle));
    RETURN_IF_FALSE (valuesEqual (m_maxCavityAngle       , castRhs->m_maxCavityAngle));
    RETURN_IF_FALSE (valuesEqual (m_bias                 , castRhs->m_bias));
    RETURN_IF_FALSE (valuesEqual (m_gain                 , castRhs->m_gain));
    RETURN_IF_FALSE (m_occType  == castRhs->m_occType);
    RETURN_IF_FALSE (m_occlusionRays == castRhs->m_occlusionRays);
    RETURN_IF_FALSE (m_sameSurface == castRhs->m_sameSurface);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PaulChater     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
void LxoOcclusionProcedure::_GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList)
    {
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionAlpha1",               (uintptr_t)&m_alpha1));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionAlpha2",               (uintptr_t)&m_alpha2));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionValue1",               (uintptr_t)&m_value1));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionValue2",               (uintptr_t)&m_value2));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionDistance",             (uintptr_t)&m_occlusionDistance));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionVariance",             (uintptr_t)&m_variance));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionVarianceScale",        (uintptr_t)&m_varianceScale));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionSpreadAngle",          (uintptr_t)&m_spreadAngle));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionMaxCavityAngle",       (uintptr_t)&m_maxCavityAngle));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionBias",                 (uintptr_t)&m_bias));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionGain",                 (uintptr_t)&m_gain));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoOcclusionType",                 (uintptr_t)&m_occType));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoOcclusionOcclusionRays",        (uintptr_t)&m_occlusionRays));
    variableList.push_back (LxoProcedureCexprMember ((TYPECODE_INT),    "lxoOcclusionSameSurface",          (uintptr_t)&m_sameSurface));

    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionColor1",               (uintptr_t)&m_color1));
    pointerList.push_back (LxoProcedureCexprMember ((TYPECODE_DOUBLE), "lxoOcclusionColor2",               (uintptr_t)&m_color2));
    }

RgbFactor const&                LxoOcclusionProcedure::GetColor1 () const {return m_color1;}
RgbFactor&                      LxoOcclusionProcedure::GetColor1R () {return m_color1;}
void                            LxoOcclusionProcedure::SetColor1 (double red, double green, double blue) {m_color1.red = red; m_color1.green = green; m_color1.blue = blue;}

RgbFactor const&                LxoOcclusionProcedure::GetColor2 () const {return m_color2;}
RgbFactor&                      LxoOcclusionProcedure::GetColor2R () {return m_color2;}
void                            LxoOcclusionProcedure::SetColor2 (double red, double green, double blue) {m_color2.red = red; m_color2.green = green; m_color2.blue = blue;}

double                          LxoOcclusionProcedure::GetAlpha1 () const {return m_alpha1;}
void                            LxoOcclusionProcedure::SetAlpha1 (double alpha1) {m_alpha1 = alpha1;}

double                          LxoOcclusionProcedure::GetAlpha2 () const {return m_alpha2;}
void                            LxoOcclusionProcedure::SetAlpha2 (double alpha2) {m_alpha2 = alpha2;}

double                          LxoOcclusionProcedure::GetValue1 () const {return m_value1;}
void                            LxoOcclusionProcedure::SetValue1 (double value1) {m_value1 = value1;}

double                          LxoOcclusionProcedure::GetValue2 () const {return m_value2;}
void                            LxoOcclusionProcedure::SetValue2 (double value2) {m_value2 = value2;}

double                          LxoOcclusionProcedure::GetOcclusionDistance () const {return m_occlusionDistance;}
void                            LxoOcclusionProcedure::SetOcclusionDistance (double occlusionDistance) {m_occlusionDistance = occlusionDistance;}

double                          LxoOcclusionProcedure::GetVariance () const {return m_variance;}
void                            LxoOcclusionProcedure::SetVariance (double variance) {m_variance = variance;}

double                          LxoOcclusionProcedure::GetVarianceScale () const {return m_varianceScale;}
void                            LxoOcclusionProcedure::SetVarianceScale (double varianceScale) {m_varianceScale = varianceScale;}

double                          LxoOcclusionProcedure::GetSpreadAngle () const {return m_spreadAngle;}
void                            LxoOcclusionProcedure::SetSpreadAngle (double spreadAngle) {m_spreadAngle = spreadAngle;}

double                          LxoOcclusionProcedure::GetMaxCavityAngle () const {return m_maxCavityAngle;}
void                            LxoOcclusionProcedure::SetMaxCavityAngle (double maxCavityAngle) {m_maxCavityAngle = maxCavityAngle;}

double                          LxoOcclusionProcedure::GetBias () const {return m_bias;}
void                            LxoOcclusionProcedure::SetBias (double bias) {m_bias = bias;}

double                          LxoOcclusionProcedure::GetGain () const {return m_gain;}
void                            LxoOcclusionProcedure::SetGain (double gain) {m_gain = gain;}

LxoOcclusionProcedure::OcclusionType LxoOcclusionProcedure::GetOcclusionType () const {return m_occType;}
void                            LxoOcclusionProcedure::SetOcclusionType (LxoOcclusionProcedure::OcclusionType type) {m_occType = type;}

uint32_t                        LxoOcclusionProcedure::GetOcclusionRays () const {return m_occlusionRays;}
void                            LxoOcclusionProcedure::SetOcclusionRays (uint32_t occlusionRays) {m_occlusionRays = occlusionRays;}

bool                            LxoOcclusionProcedure::GetSameSurface () const  {return m_sameSurface;}
void                            LxoOcclusionProcedure::SetSameSurface (bool sameSurface) {m_sameSurface = sameSurface;}

