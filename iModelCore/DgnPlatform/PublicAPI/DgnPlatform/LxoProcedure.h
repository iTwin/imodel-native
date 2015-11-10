/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/LxoProcedure.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if !defined (resource) && !defined (type_resource_generator)

// __PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "DgnPlatform.h"
//__PUBLISH_SECTION_END__
#include <RmgrTools/Tools/typecode.h>
//__PUBLISH_SECTION_START__

DGNPLATFORM_TYPEDEFS (LxoProcedureCexprMembers)
DGNPLATFORM_TYPEDEFS (LxoProcedure)
DGNPLATFORM_TYPEDEFS (LxoNoiseProcedure)
DGNPLATFORM_TYPEDEFS (LxoCheckerProcedure)
DGNPLATFORM_TYPEDEFS (LxoGridProcedure)
DGNPLATFORM_TYPEDEFS (LxoDotProcedure)
DGNPLATFORM_TYPEDEFS (LxoConstantProcedure)
DGNPLATFORM_TYPEDEFS (LxoCellularProcedure)
DGNPLATFORM_TYPEDEFS (LxoWoodProcedure)
DGNPLATFORM_TYPEDEFS (LxoWeaveProcedure)
DGNPLATFORM_TYPEDEFS (LxoRipplesProcedure)
DGNPLATFORM_TYPEDEFS (LxoGradientProcedure)
DGNPLATFORM_TYPEDEFS (LxoFloatEnvelopeKey)
DGNPLATFORM_TYPEDEFS (LxoIntEnvelopeKey)
DGNPLATFORM_TYPEDEFS (LxoEnvelopeTangentIn)
DGNPLATFORM_TYPEDEFS (LxoEnvelopeTangentOut)
DGNPLATFORM_TYPEDEFS (LxoFloatEnvelopeComponent)
DGNPLATFORM_TYPEDEFS (LxoFloatEnvelopeComponentCollection)
DGNPLATFORM_TYPEDEFS (LxoFloatEnvelope)
DGNPLATFORM_TYPEDEFS (LxoIntEnvelopeComponent)
DGNPLATFORM_TYPEDEFS (LxoIntEnvelopeComponentCollection)
DGNPLATFORM_TYPEDEFS (LxoIntEnvelope)
DGNPLATFORM_TYPEDEFS (LxoBGradientProcedure)
DGNPLATFORM_TYPEDEFS (LxoBoardsProcedure)
DGNPLATFORM_TYPEDEFS (LxoBrickProcedure)
DGNPLATFORM_TYPEDEFS (LxoBentleyCheckerProcedure)
DGNPLATFORM_TYPEDEFS (LxoBWNoiseProcedure)
DGNPLATFORM_TYPEDEFS (LxoChecker3dProcedure)
DGNPLATFORM_TYPEDEFS (LxoCloudsProcedure)
DGNPLATFORM_TYPEDEFS (LxoColorNoiseProcedure)
DGNPLATFORM_TYPEDEFS (LxoFlameProcedure)
DGNPLATFORM_TYPEDEFS (LxoFogProcedure)
DGNPLATFORM_TYPEDEFS (LxoMarbleProcedure)
DGNPLATFORM_TYPEDEFS (LxoRGBColorCubeProcedure)
DGNPLATFORM_TYPEDEFS (LxoSandProcedure)
DGNPLATFORM_TYPEDEFS (LxoStoneProcedure)
DGNPLATFORM_TYPEDEFS (LxoTurbulenceProcedure)
DGNPLATFORM_TYPEDEFS (LxoTurfProcedure)
DGNPLATFORM_TYPEDEFS (LxoWaterProcedure)
DGNPLATFORM_TYPEDEFS (LxoWavesProcedure)
DGNPLATFORM_TYPEDEFS (LxoBentleyWoodProcedure)
DGNPLATFORM_TYPEDEFS (LxoAdvancedWoodProcedure)
DGNPLATFORM_TYPEDEFS (LxoOcclusionProcedure)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

// __PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
enum LxoEnvelopeXmlVersion
    {
    LXOENVELOPEXMLVERSION_KeysOnly          = 1, // Only keys are guaranteed to be accurate.
    LXOENVELOPEXMLVERSION_Current           = LXOENVELOPEXMLVERSION_KeysOnly,
    };

typedef RefCountedPtr <LxoProcedure> LxoProcedurePtr;

//=======================================================================================
// @bsiclass                                                      PaulChater     07/11
//=======================================================================================
struct LxoProcedureCexprMember
    {
    uintptr_t       m_member;
    std::string     m_name;
    MdlTypecodes    m_typecode;

    LxoProcedureCexprMember (MdlTypecodes typecode, const char * name, uintptr_t member); // WIP_CHAR_OK - Destined for the MDL dialog published variable system, which will not go Unicode
    };
typedef bvector <LxoProcedureCexprMember>          LxoProcedureCexprMemberList;
// __PUBLISH_SECTION_START__

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoProcedure : public RefCountedBase
{
public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum ProcedureType
        {
        PROCEDURETYPE_None              = 0,
        PROCEDURETYPE_Noise             = 1,
        PROCEDURETYPE_Checker           = 2,
        PROCEDURETYPE_Grid              = 3,
        PROCEDURETYPE_Dot               = 4,
        PROCEDURETYPE_Constant          = 5,
        PROCEDURETYPE_Cellular          = 6,
        PROCEDURETYPE_Wood              = 7,
        PROCEDURETYPE_Weave             = 8,
        PROCEDURETYPE_Ripples           = 9,
        PROCEDURETYPE_Gradient          = 10,
        PROCEDURETYPE_Boards            = 11,
        PROCEDURETYPE_Brick             = 12,
        PROCEDURETYPE_BWNoise           = 13,
        PROCEDURETYPE_BentleyChecker    = 14,
        PROCEDURETYPE_Checker3d         = 15,
        PROCEDURETYPE_Clouds            = 16,
        PROCEDURETYPE_ColorNoise        = 17,
        PROCEDURETYPE_Flame             = 18,
        PROCEDURETYPE_Fog               = 19,
        PROCEDURETYPE_Marble            = 20,
        PROCEDURETYPE_RGBColorCube      = 21,
        PROCEDURETYPE_Sand              = 22,
        PROCEDURETYPE_Stone             = 23,
        PROCEDURETYPE_Turbulence        = 24,
        PROCEDURETYPE_Turf              = 25,
        PROCEDURETYPE_Water             = 26,
        PROCEDURETYPE_Waves             = 27,
        PROCEDURETYPE_BentleyWood       = 28,
        PROCEDURETYPE_AdvancedWood      = 29,
        PROCEDURETYPE_Occlusion         = 30,
        PROCEDURETYPE_BGradient         = 31,
        };

// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
private:
// __PUBLISH_SECTION_END__
    virtual ProcedureType _GetType () const = 0;
    virtual void _InitDefaults () = 0;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) = 0;
    virtual bool _Equals (LxoProcedureCR rhs) const = 0;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) = 0;

protected:
    virtual ~LxoProcedure ();

// __PUBLISH_SECTION_START__
private:
// __PUBLISH_CLASS_VIRTUAL__

public:
    DGNPLATFORM_EXPORT ProcedureType GetType () const;
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT BentleyStatus Copy (LxoProcedureCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoProcedureCR rhs) const;
// __PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT void GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList);

    DGNPLATFORM_EXPORT static LxoProcedurePtr Create (ProcedureType type);
// __PUBLISH_SECTION_START__
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoNoiseProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:

// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum NoiseType
        {
        NOISETYPE_Simple        = 0,
        NOISETYPE_Fractal       = 1,
        NOISETYPE_Turbulence    = 2,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_color1;
    double                                      m_alpha1;
    RgbFactor                                   m_color2;
    double                                      m_alpha2;
    uint32_t                                    m_frequencies;
    double                                      m_frequencyRatio;
    double                                      m_amplitudeRatio;
    double                                      m_bias;
    double                                      m_gain;
    double                                      m_value1;
    double                                      m_value2;
    NoiseType                                   m_noiseType;

    LxoNoiseProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetColor1 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor1R ();
    DGNPLATFORM_EXPORT void SetColor1 (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetColor2 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor2R ();
    DGNPLATFORM_EXPORT void SetColor2 (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetAlpha1 () const;
    DGNPLATFORM_EXPORT void SetAlpha1 (double alpha);

    DGNPLATFORM_EXPORT double GetAlpha2 () const;
    DGNPLATFORM_EXPORT void SetAlpha2 (double alpha);

    DGNPLATFORM_EXPORT uint32_t GetFrequencies () const;
    DGNPLATFORM_EXPORT void SetFrequencies (uint32_t frequencies);

    DGNPLATFORM_EXPORT double GetFrequencyRatio () const;
    DGNPLATFORM_EXPORT void SetFrequencyRatio (double ratio);

    DGNPLATFORM_EXPORT double GetAmplitudeRatio () const;
    DGNPLATFORM_EXPORT void SetAmplitudeRatio (double ratio);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetValue1 () const;
    DGNPLATFORM_EXPORT void SetValue1 (double value);

    DGNPLATFORM_EXPORT double GetValue2 () const;
    DGNPLATFORM_EXPORT void SetValue2 (double value);

    DGNPLATFORM_EXPORT NoiseType GetNoiseType () const;
    DGNPLATFORM_EXPORT void SetNoiseType (NoiseType type);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoCheckerProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum CheckerType
        {
        CHECKERTYPE_Square  = 0,
        CHECKERTYPE_Cube    = 1,
        };

// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_color1;
    double                                      m_alpha1;
    RgbFactor                                   m_color2;
    double                                      m_alpha2;
    double                                      m_bias;
    double                                      m_gain;
    CheckerType                                 m_checkerType;
    double                                      m_transitionWidth;
    double                                      m_value1;
    double                                      m_value2;

    LxoCheckerProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetColor1 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor1R ();
    DGNPLATFORM_EXPORT void SetColor1 (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetColor2 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor2R ();
    DGNPLATFORM_EXPORT void SetColor2 (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetAlpha1 () const;
    DGNPLATFORM_EXPORT void SetAlpha1 (double alpha);

    DGNPLATFORM_EXPORT double GetAlpha2 () const;
    DGNPLATFORM_EXPORT void SetAlpha2 (double alpha);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetValue1 () const;
    DGNPLATFORM_EXPORT void SetValue1 (double value);

    DGNPLATFORM_EXPORT double GetValue2 () const;
    DGNPLATFORM_EXPORT void SetValue2 (double value);

    DGNPLATFORM_EXPORT double GetTransitionWidth () const;
    DGNPLATFORM_EXPORT void SetTransitionWidth (double width);

    DGNPLATFORM_EXPORT CheckerType GetCheckerType () const;
    DGNPLATFORM_EXPORT void SetCheckerType (CheckerType type);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoGridProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum GridType
        {
        GRIDTYPE_Line       = 0,
        GRIDTYPE_Triangle   = 1,
        GRIDTYPE_Square     = 2,
        GRIDTYPE_Hexagon    = 3,
        GRIDTYPE_Cube       = 4,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_lineColor;
    double                                      m_lineAlpha;
    RgbFactor                                   m_fillerColor;
    double                                      m_fillerAlpha;
    double                                      m_bias;
    double                                      m_gain;
    GridType                                    m_gridType;
    double                                      m_lineWidth;
    double                                      m_transitionWidth;
    double                                      m_lineValue;
    double                                      m_fillerValue;

    LxoGridProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetLineColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetLineColorR ();
    DGNPLATFORM_EXPORT void SetLineColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetFillerColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetFillerColorR ();
    DGNPLATFORM_EXPORT void SetFillerColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetLineAlpha () const;
    DGNPLATFORM_EXPORT void SetLineAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetFillerAlpha () const;
    DGNPLATFORM_EXPORT void SetFillerAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetLineWidth () const;
    DGNPLATFORM_EXPORT void SetLineWidth (double width);

    DGNPLATFORM_EXPORT double GetTransitionWidth () const;
    DGNPLATFORM_EXPORT void SetTransitionWidth (double width);

    DGNPLATFORM_EXPORT double GetLineValue () const;
    DGNPLATFORM_EXPORT void SetLineValue (double value);

    DGNPLATFORM_EXPORT double GetFillerValue () const;
    DGNPLATFORM_EXPORT void SetFillerValue (double value);

    DGNPLATFORM_EXPORT GridType GetGridType () const;
    DGNPLATFORM_EXPORT void SetGridType (GridType type);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoDotProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum DotType
        {
        DOTTYPE_Square  = 0,
        DOTTYPE_Hexagon = 1,
        DOTTYPE_Cube    = 2,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_dotColor;
    double                                      m_dotAlpha;
    RgbFactor                                   m_fillerColor;
    double                                      m_fillerAlpha;
    double                                      m_bias;
    double                                      m_gain;
    DotType                                     m_dotType;
    double                                      m_dotWidth;
    double                                      m_transitionWidth;
    double                                      m_dotValue;
    double                                      m_fillerValue;

    LxoDotProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetDotColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetDotColorR ();
    DGNPLATFORM_EXPORT void SetDotColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetFillerColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetFillerColorR ();
    DGNPLATFORM_EXPORT void SetFillerColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetDotAlpha () const;
    DGNPLATFORM_EXPORT void SetDotAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetFillerAlpha () const;
    DGNPLATFORM_EXPORT void SetFillerAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetDotWidth () const;
    DGNPLATFORM_EXPORT void SetDotWidth (double width);

    DGNPLATFORM_EXPORT double GetTransitionWidth () const;
    DGNPLATFORM_EXPORT void SetTransitionWidth (double width);

    DGNPLATFORM_EXPORT double GetDotValue () const;
    DGNPLATFORM_EXPORT void SetDotValue (double value);

    DGNPLATFORM_EXPORT double GetFillerValue () const;
    DGNPLATFORM_EXPORT void SetFillerValue (double value);

    DGNPLATFORM_EXPORT DotType GetDotType () const;
    DGNPLATFORM_EXPORT void SetDotType (DotType type);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoConstantProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_color;
    double                                      m_value;

    LxoConstantProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColorR ();
    DGNPLATFORM_EXPORT void SetColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetValue () const;
    DGNPLATFORM_EXPORT void SetValue (double value);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoCellularProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum CellularType
        {
        CELLULARTYPE_Round      = 0,
        CELLULARTYPE_Angular    = 1,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__


private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_cellColor;
    double                                      m_cellAlpha;
    RgbFactor                                   m_fillerColor;
    double                                      m_fillerAlpha;
    double                                      m_bias;
    double                                      m_gain;
    CellularType                                m_cellularType;
    double                                      m_cellWidth;
    double                                      m_transitionWidth;
    int32_t                                     m_frequencies;
    double                                      m_frequencyRatio;
    double                                      m_amplitudeRatio;
    double                                      m_valueVariation;
    double                                      m_hueVariation;
    double                                      m_saturationVariation;
    double                                      m_cellValue;
    double                                      m_fillerValue;

    LxoCellularProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetCellColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetCellColorR ();
    DGNPLATFORM_EXPORT void SetCellColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetFillerColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetFillerColorR ();
    DGNPLATFORM_EXPORT void SetFillerColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetCellAlpha () const;
    DGNPLATFORM_EXPORT void SetCellAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetFillerAlpha () const;
    DGNPLATFORM_EXPORT void SetFillerAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetCellWidth () const;
    DGNPLATFORM_EXPORT void SetCellWidth (double width);

    DGNPLATFORM_EXPORT double GetTransitionWidth () const;
    DGNPLATFORM_EXPORT void SetTransitionWidth (double width);

    DGNPLATFORM_EXPORT double GetCellValue () const;
    DGNPLATFORM_EXPORT void SetCellValue (double value);

    DGNPLATFORM_EXPORT double GetFillerValue () const;
    DGNPLATFORM_EXPORT void SetFillerValue (double value);

    DGNPLATFORM_EXPORT int32_t GetFrequencies () const;
    DGNPLATFORM_EXPORT void SetFrequencies (int32_t frequencies);

    DGNPLATFORM_EXPORT double GetFrequencyRatio () const;
    DGNPLATFORM_EXPORT void SetFrequencyRatio (double ratio);

    DGNPLATFORM_EXPORT double GetAmplitudeRatio () const;
    DGNPLATFORM_EXPORT void SetAmplitudeRatio (double ratio);

    DGNPLATFORM_EXPORT double GetValueVariation () const;
    DGNPLATFORM_EXPORT void SetValueVariation (double variation);

    DGNPLATFORM_EXPORT double GetHueVariation () const;
    DGNPLATFORM_EXPORT void SetHueVariation (double variation);

    DGNPLATFORM_EXPORT double GetSaturationVariation () const;
    DGNPLATFORM_EXPORT void SetSaturationVariation (double variation);

    DGNPLATFORM_EXPORT CellularType GetCellularType () const;
    DGNPLATFORM_EXPORT void SetCellularType (CellularType type);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoWoodProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_ringColor;
    double                                      m_ringAlpha;
    RgbFactor                                   m_fillerColor;
    double                                      m_fillerAlpha;
    double                                      m_bias;
    double                                      m_gain;
    double                                      m_ringScale;
    double                                      m_waveScale;
    double                                      m_waviness;
    double                                      m_graininess;
    double                                      m_grainScale;
    double                                      m_ringValue;
    double                                      m_fillerValue;

    LxoWoodProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetRingColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetRingColorR ();
    DGNPLATFORM_EXPORT void SetRingColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetFillerColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetFillerColorR ();
    DGNPLATFORM_EXPORT void SetFillerColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetRingAlpha () const;
    DGNPLATFORM_EXPORT void SetRingAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetFillerAlpha () const;
    DGNPLATFORM_EXPORT void SetFillerAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetRingScale () const;
    DGNPLATFORM_EXPORT void SetRingScale (double scale);

    DGNPLATFORM_EXPORT double GetWaveScale () const;
    DGNPLATFORM_EXPORT void SetWaveScale (double scale);

    DGNPLATFORM_EXPORT double GetWaviness () const;
    DGNPLATFORM_EXPORT void SetWaviness (double waviness);

    DGNPLATFORM_EXPORT double GetGraininess () const;
    DGNPLATFORM_EXPORT void SetGraininess (double graininess);

    DGNPLATFORM_EXPORT double GetGrainScale () const;
    DGNPLATFORM_EXPORT void SetGrainScale (double scale);

    DGNPLATFORM_EXPORT double GetRingValue () const;
    DGNPLATFORM_EXPORT void SetRingValue (double value);

    DGNPLATFORM_EXPORT double GetFillerValue () const;
    DGNPLATFORM_EXPORT void SetFillerValue (double value);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoWeaveProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_yarnColor;
    double          m_yarnAlpha;
    RgbFactor       m_gapColor;
    double          m_gapAlpha;
    double          m_yarnWidth;
    double          m_roundness;
    double          m_bias;
    double          m_gain;
    double          m_yarnValue;
    double          m_gapValue;

    LxoWeaveProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetYarnColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetYarnColorR ();
    DGNPLATFORM_EXPORT void SetYarnColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetGapColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetGapColorR ();
    DGNPLATFORM_EXPORT void SetGapColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetYarnAlpha () const;
    DGNPLATFORM_EXPORT void SetYarnAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetGapAlpha () const;
    DGNPLATFORM_EXPORT void SetGapAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT double GetYarnWidth () const;
    DGNPLATFORM_EXPORT void SetYarnWidth (double width);

    DGNPLATFORM_EXPORT double GetRoundness () const;
    DGNPLATFORM_EXPORT void SetRoundness (double roundness);

    DGNPLATFORM_EXPORT double GetYarnValue () const;
    DGNPLATFORM_EXPORT void SetYarnValue (double value);

    DGNPLATFORM_EXPORT double GetGapValue () const;
    DGNPLATFORM_EXPORT void SetGapValue (double value);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoRipplesProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor                                   m_crestColor;
    double                                      m_crestAlpha;
    RgbFactor                                   m_troughColor;
    double                                      m_troughAlpha;
    double                                      m_wavelength;
    double                                      m_phase;
    double                                      m_bias;
    double                                      m_gain;
    uint32_t                                    m_sources;
    double                                      m_crestValue;
    double                                      m_troughValue;

    LxoRipplesProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetCrestColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetCrestColorR ();
    DGNPLATFORM_EXPORT void SetCrestColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetTroughColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetTroughColorR ();
    DGNPLATFORM_EXPORT void SetTroughColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetCrestAlpha () const;
    DGNPLATFORM_EXPORT void SetCrestAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetTroughAlpha () const;
    DGNPLATFORM_EXPORT void SetTroughAlpha (double alpha);

    DGNPLATFORM_EXPORT double GetWavelength () const;
    DGNPLATFORM_EXPORT void SetWavelength (double length);

    DGNPLATFORM_EXPORT double GetPhase () const;
    DGNPLATFORM_EXPORT void SetPhase (double phase);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT uint32_t GetSources () const;
    DGNPLATFORM_EXPORT void SetSources (uint32_t sources);

    DGNPLATFORM_EXPORT double GetCrestValue () const;
    DGNPLATFORM_EXPORT void SetCrestValue (double value);

    DGNPLATFORM_EXPORT double GetTroughValue () const;
    DGNPLATFORM_EXPORT void SetTroughValue (double value);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoFloatEnvelopeKey
{
    friend struct LxoFloatEnvelopeComponent;

private:
// __PUBLISH_SECTION_END__
    DPoint2d                        m_value;

    LxoFloatEnvelopeKey ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoFloatEnvelopeKeyCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoFloatEnvelopeKeyCR rhs) const;

    DGNPLATFORM_EXPORT DPoint2dCR GetValue () const;
    DGNPLATFORM_EXPORT DPoint2dR GetValueR ();
    DGNPLATFORM_EXPORT void SetValue (double x, double y);
};

//=======================================================================================
// @bsiclass                                                      PaulChater    03/12
//=======================================================================================
struct LxoIntEnvelopeKey
{
    friend struct LxoIntEnvelopeComponent;

private:
// __PUBLISH_SECTION_END__
    double                          m_valueX;
    int                             m_valueY;

    LxoIntEnvelopeKey ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoIntEnvelopeKeyCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoIntEnvelopeKeyCR rhs) const;

    DGNPLATFORM_EXPORT double GetValueX () const;
    DGNPLATFORM_EXPORT int GetValueY () const;
    DGNPLATFORM_EXPORT void SetValue (double x, int y);
    DGNPLATFORM_EXPORT void SetValueX (double x);
    DGNPLATFORM_EXPORT void SetValueY (int y);

};
//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
enum class LxoEnvelopeSlopeType
{
    Direct     = 0,
    Auto       = 1,
    LinearIn   = 1 << 1,
    LinearOut  = 1 << 2,
    Flat       = 1 << 3,
    AutoFlat   = 1 << 4,
    Stepped    = 1 << 5,
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
enum class  LxoEnvelopeWeightType
{
    Auto      = 0,
    Manual    = 1,
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoEnvelopeTangentIn
{
    friend struct LxoFloatEnvelopeComponent;
    friend struct LxoIntEnvelopeComponent;

private:
// __PUBLISH_SECTION_END__
    LxoEnvelopeSlopeType                        m_slopeType;
    LxoEnvelopeWeightType                       m_weightType;
    double                                      m_slope;
    double                                      m_weight;
    double                                      m_value;

    LxoEnvelopeTangentIn ();

protected:
    DGNPLATFORM_EXPORT LxoEnvelopeTangentIn (LxoEnvelopeTangentInCR rhs);

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoEnvelopeTangentInCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoEnvelopeTangentInCR rhs) const;

    DGNPLATFORM_EXPORT LxoEnvelopeSlopeType GetSlopeType () const;
    DGNPLATFORM_EXPORT void SetSlopeType (LxoEnvelopeSlopeType type);

    DGNPLATFORM_EXPORT LxoEnvelopeWeightType GetWeightType () const;
    DGNPLATFORM_EXPORT void SetWeightType (LxoEnvelopeWeightType type);

    DGNPLATFORM_EXPORT double GetSlope () const;
    DGNPLATFORM_EXPORT void SetSlope (double slope);

    DGNPLATFORM_EXPORT double GetWeight () const;
    DGNPLATFORM_EXPORT void SetWeight (double weight);

    DGNPLATFORM_EXPORT double GetValue () const;
    DGNPLATFORM_EXPORT void SetValue (double value);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
enum class LxoEnvelopeBreak
{
    Value          = 1,
    Slope          = 1 << 1,
    Weight         = 1 << 2,
    ActiveValue    = 1 << 3,
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoEnvelopeTangentOut
{
    friend struct LxoFloatEnvelopeComponent;
    friend struct LxoIntEnvelopeComponent;

private:
// __PUBLISH_SECTION_END__
    LxoEnvelopeBreak                            m_break;
    LxoEnvelopeSlopeType                        m_slopeType;
    LxoEnvelopeWeightType                       m_weightType;
    double                                      m_slope;
    double                                      m_weight;
    double                                      m_value;

    LxoEnvelopeTangentOut ();
protected:
    DGNPLATFORM_EXPORT LxoEnvelopeTangentOut (LxoEnvelopeTangentOutCR rhs);
// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoEnvelopeTangentOutCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoEnvelopeTangentOutCR rhs) const;

    DGNPLATFORM_EXPORT LxoEnvelopeBreak GetBreak () const;
    DGNPLATFORM_EXPORT void SetBreak (LxoEnvelopeBreak breakType);

    DGNPLATFORM_EXPORT LxoEnvelopeSlopeType GetSlopeType () const;
    DGNPLATFORM_EXPORT void SetSlopeType (LxoEnvelopeSlopeType type);

    DGNPLATFORM_EXPORT LxoEnvelopeWeightType GetWeightType () const;
    DGNPLATFORM_EXPORT void SetWeightType (LxoEnvelopeWeightType type);

    DGNPLATFORM_EXPORT double GetSlope () const;
    DGNPLATFORM_EXPORT void SetSlope (double slope);

    DGNPLATFORM_EXPORT double GetWeight () const;
    DGNPLATFORM_EXPORT void SetWeight (double weight);

    DGNPLATFORM_EXPORT double GetValue () const;
    DGNPLATFORM_EXPORT void SetValue (double value);
};

typedef RefCountedPtr <LxoFloatEnvelopeComponent>       LxoFloatEnvelopeComponentPtr;
typedef bvector <LxoFloatEnvelopeComponentPtr>          LxoFloatEnvelopeComponentPtrList;

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoFloatEnvelopeComponent : public RefCountedBase
{
    friend struct LxoFloatEnvelopeComponentCollection;

private:
// __PUBLISH_SECTION_END__
    LxoFloatEnvelopeKey                                 m_key;
    LxoEnvelopeTangentIn                                m_tangentIn;
    LxoEnvelopeTangentOut                               m_tangentOut;
    uint32_t                                            m_flag;

    LxoFloatEnvelopeComponent ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoFloatEnvelopeComponentCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoFloatEnvelopeComponentCR rhs) const;

    DGNPLATFORM_EXPORT LxoFloatEnvelopeKeyCR GetKey () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeKeyR GetKeyR ();

    DGNPLATFORM_EXPORT LxoEnvelopeTangentInCR GetTangentIn () const;
    DGNPLATFORM_EXPORT LxoEnvelopeTangentInR GetTangentInR ();

    DGNPLATFORM_EXPORT LxoEnvelopeTangentOutCR GetTangentOut () const;
    DGNPLATFORM_EXPORT LxoEnvelopeTangentOutR GetTangentOutR ();

    DGNPLATFORM_EXPORT uint32_t GetFlag () const;
    DGNPLATFORM_EXPORT void SetFlag (uint32_t flag);
};

//=======================================================================================
// @bsiclass                                                      MattGooding     12/09
//=======================================================================================
struct LxoFloatEnvelopeComponentCollection
{
    friend struct LxoFloatEnvelope;

private:
// __PUBLISH_SECTION_END__
    LxoFloatEnvelopeComponentPtrList                            m_components;

    LxoFloatEnvelopeComponentCollection ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    typedef LxoFloatEnvelopeComponentPtrList::const_iterator    const_iterator;
    typedef LxoFloatEnvelopeComponentPtrList::iterator          iterator;

    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (LxoFloatEnvelopeComponentCollectionCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (LxoFloatEnvelopeComponentCollectionCR rhs);

    DGNPLATFORM_EXPORT const_iterator begin () const;
    DGNPLATFORM_EXPORT iterator begin ();
    DGNPLATFORM_EXPORT const_iterator end () const;
    DGNPLATFORM_EXPORT iterator end ();

    DGNPLATFORM_EXPORT size_t Size () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeComponentR AddComponent ();
    DGNPLATFORM_EXPORT void DeleteComponent (iterator iter);
    DGNPLATFORM_EXPORT void Clear ();
};

typedef RefCountedPtr <LxoIntEnvelopeComponent>       LxoIntEnvelopeComponentPtr;
typedef bvector <LxoIntEnvelopeComponentPtr>          LxoIntEnvelopeComponentPtrList;

//=======================================================================================
// @bsiclass                                                      PaulChater    03/12
//=======================================================================================
struct LxoIntEnvelopeComponent : public RefCountedBase
{
    friend struct LxoIntEnvelopeComponentCollection;

private:
// __PUBLISH_SECTION_END__
    LxoIntEnvelopeKey                                   m_key;
    LxoEnvelopeTangentIn                                m_tangentIn;
    LxoEnvelopeTangentOut                               m_tangentOut;
    uint32_t                                            m_flag;

    LxoIntEnvelopeComponent ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoIntEnvelopeComponentCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoIntEnvelopeComponentCR rhs) const;

    DGNPLATFORM_EXPORT LxoIntEnvelopeKeyCR GetKey () const;
    DGNPLATFORM_EXPORT LxoIntEnvelopeKeyR GetKeyR ();

    DGNPLATFORM_EXPORT LxoEnvelopeTangentInCR GetTangentIn () const;
    DGNPLATFORM_EXPORT LxoEnvelopeTangentInR GetTangentInR ();

    DGNPLATFORM_EXPORT LxoEnvelopeTangentOutCR GetTangentOut () const;
    DGNPLATFORM_EXPORT LxoEnvelopeTangentOutR GetTangentOutR ();

    DGNPLATFORM_EXPORT uint32_t GetFlag () const;
    DGNPLATFORM_EXPORT void SetFlag (uint32_t flag);
};

//=======================================================================================
// @bsiclass                                                      PaulChater    03/12
//=======================================================================================
struct LxoIntEnvelopeComponentCollection
{
    friend struct LxoIntEnvelope;

private:
// __PUBLISH_SECTION_END__
    LxoIntEnvelopeComponentPtrList                            m_components;

    LxoIntEnvelopeComponentCollection ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    typedef LxoIntEnvelopeComponentPtrList::const_iterator    const_iterator;
    typedef LxoIntEnvelopeComponentPtrList::iterator          iterator;

    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT bool Equals (LxoIntEnvelopeComponentCollectionCR rhs) const;
    DGNPLATFORM_EXPORT void Copy (LxoIntEnvelopeComponentCollectionCR rhs);

    DGNPLATFORM_EXPORT const_iterator begin () const;
    DGNPLATFORM_EXPORT iterator begin ();
    DGNPLATFORM_EXPORT const_iterator end () const;
    DGNPLATFORM_EXPORT iterator end ();

    DGNPLATFORM_EXPORT size_t Size () const;
    DGNPLATFORM_EXPORT LxoIntEnvelopeComponentR AddComponent ();
    DGNPLATFORM_EXPORT void DeleteComponent (iterator iter);
    DGNPLATFORM_EXPORT void Clear ();
};

// __PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                    MattGooding     12/09
//=======================================================================================
enum LxoEnvelopeType
    {
    LXOENVELOPETYPE_Float   = 0,
    LXOENVELOPETYPE_Int     = 1,
    };
// __PUBLISH_SECTION_START__
typedef RefCountedPtr <LxoFloatEnvelope> LxoFloatEnvelopePtr;

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoFloatEnvelope : public RefCountedBase
{
    friend struct LxoGradientProcedure;

public:
    //=======================================================================================
    // @bsiclass                                                      MattGooding     11/09
    //=======================================================================================
    enum EndBehavior
        {
        ENDBEHAVIOR_Reset        = 0,
        ENDBEHAVIOR_Constant     = 1,
        ENDBEHAVIOR_Repeat       = 2,
        ENDBEHAVIOR_Oscillate    = 3,
        ENDBEHAVIOR_OffsetRepeat = 4,
        ENDBEHAVIOR_Linear       = 5,
        ENDBEHAVIOR_None         = 6,
        };

private:
// __PUBLISH_SECTION_END__
    EndBehavior                                                 m_preBehavior;
    EndBehavior                                                 m_postBehavior;
    LxoFloatEnvelopeComponentCollection                         m_components;

    LxoFloatEnvelope ();
// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoFloatEnvelopeCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoFloatEnvelopeCR rhs) const;

    DGNPLATFORM_EXPORT EndBehavior GetPreBehavior () const;
    DGNPLATFORM_EXPORT void SetPreBehavior (EndBehavior behavior);

    DGNPLATFORM_EXPORT EndBehavior GetPostBehavior () const;
    DGNPLATFORM_EXPORT void SetPostBehavior (EndBehavior behavior);

    DGNPLATFORM_EXPORT LxoFloatEnvelopeComponentCollectionCR GetComponents () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeComponentCollectionR GetComponentsR ();

    DGNPLATFORM_EXPORT static LxoFloatEnvelopePtr Create ();
};

typedef RefCountedPtr <LxoIntEnvelope> LxoIntEnvelopePtr;

//=======================================================================================
// @bsiclass                                                      PaulChater    03/12
//=======================================================================================
struct LxoIntEnvelope : public RefCountedBase
{

public:
    //=======================================================================================
    // @bsiclass                                                      PaulChater    03/12
    //=======================================================================================
    enum EndBehavior
        {
        ENDBEHAVIOR_Reset        = 0,
        ENDBEHAVIOR_Constant     = 1,
        ENDBEHAVIOR_Repeat       = 2,
        ENDBEHAVIOR_Oscillate    = 3,
        ENDBEHAVIOR_OffsetRepeat = 4,
        ENDBEHAVIOR_Linear       = 5,
        ENDBEHAVIOR_None         = 6,
        };

private:
// __PUBLISH_SECTION_END__
    EndBehavior                                                 m_preBehavior;
    EndBehavior                                                 m_postBehavior;
    LxoIntEnvelopeComponentCollection                           m_components;

    LxoIntEnvelope ();
// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT void InitDefaults ();
    DGNPLATFORM_EXPORT void Copy (LxoIntEnvelopeCR rhs);
    DGNPLATFORM_EXPORT bool Equals (LxoIntEnvelopeCR rhs) const;

    DGNPLATFORM_EXPORT EndBehavior GetPreBehavior () const;
    DGNPLATFORM_EXPORT void SetPreBehavior (EndBehavior behavior);

    DGNPLATFORM_EXPORT EndBehavior GetPostBehavior () const;
    DGNPLATFORM_EXPORT void SetPostBehavior (EndBehavior behavior);

    DGNPLATFORM_EXPORT LxoIntEnvelopeComponentCollectionCR GetComponents () const;
    DGNPLATFORM_EXPORT LxoIntEnvelopeComponentCollectionR GetComponentsR ();

    DGNPLATFORM_EXPORT static LxoIntEnvelopePtr Create ();
};

//=======================================================================================
// @bsiclass                                                      MattGooding     11/09
//=======================================================================================
struct LxoGradientProcedure : public LxoProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                     MattGooding     11/09
    //=======================================================================================
    enum GradientInput
        {
        GRADIENTINPUT_TextureU              = 0,
        GRADIENTINPUT_TextureV              = 1,
        GRADIENTINPUT_BumpHeight            = 2,
        GRADIENTINPUT_DisplacementHeight    = 3,
        GRADIENTINPUT_FurParametricLength   = 4,
        GRADIENTINPUT_Slope                 = 5,
        GRADIENTINPUT_TextureValue          = 6,
        GRADIENTINPUT_DistanceToCamera      = 7,
        GRADIENTINPUT_Incidence             = 8,
        GRADIENTINPUT_DistanceToLocator     = 9,
        GRADIENTINPUT_XDistanceToLocator    = 10,
        GRADIENTINPUT_YDistanceToLocator    = 11,
        GRADIENTINPUT_ZDistanceToLocator    = 12,
        GRADIENTINPUT_LocatorIncidence      = 13,
        GRADIENTINPUT_DriverA               = 14,
        GRADIENTINPUT_DriverB               = 15,
        GRADIENTINPUT_DriverC               = 16,
        GRADIENTINPUT_DriverD               = 17,
        GRADIENTINPUT_ParticleID            = 18,
        GRADIENTINPUT_LayerMask             = 19,
        GRADIENTINPUT_GroupMask             = 20,
        GRADIENTINPUT_ClearcoatAmount       = 21,
        GRADIENTINPUT_DiffuseAmount         = 22,
        GRADIENTINPUT_Dissolve              = 23,
        GRADIENTINPUT_LuminousAmount        = 24,
        GRADIENTINPUT_ReflectionAmount      = 25,
        GRADIENTINPUT_ReflectionFresnel     = 26,
        GRADIENTINPUT_RefractionRoughness   = 27,
        GRADIENTINPUT_Roughness             = 28,
        GRADIENTINPUT_SpecularAmount        = 29,
        GRADIENTINPUT_SpecularFresnel       = 30,
        GRADIENTINPUT_SubsurfaceAmount      = 31,
        GRADIENTINPUT_TransparentAmount     = 32,
        GRADIENTINPUT_FurBump               = 33,
        GRADIENTINPUT_FurClumpDensity       = 34,
        GRADIENTINPUT_FurClumps             = 35,
        GRADIENTINPUT_FurDensity            = 36,
        GRADIENTINPUT_FurFlex               = 37,
        GRADIENTINPUT_FurGrowth             = 38,
        GRADIENTINPUT_FurLength             = 39,
        GRADIENTINPUT_FurRootBend           = 40,
        GRADIENTINPUT_LightDiffuseAmount    = 41,  // relates to lighting maps only
        GRADIENTINPUT_LightSpecularAmount   = 42,  // relates to lighting maps only
        GRADIENTINPUT_LightIncidence        = 43,  // relates to lighting maps only
        GRADIENTINPUT_VolumetricDensity     = 44,  // relates to lighting maps only
        GRADIENTINPUT_DistanceToLight       = 45,  // relates to lighting maps only
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    GradientInput                               m_input;
    LxoFloatEnvelope                            m_value;
    LxoFloatEnvelope                            m_red;
    LxoFloatEnvelope                            m_green;
    LxoFloatEnvelope                            m_blue;
    LxoFloatEnvelope                            m_alpha;


    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

protected:

    LxoGradientProcedure ();

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT GradientInput GetGradientInput () const;
    DGNPLATFORM_EXPORT void SetGradientInput (GradientInput input);

    DGNPLATFORM_EXPORT LxoFloatEnvelopeCR GetValueEnvelope () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeR GetValueEnvelopeR ();

    DGNPLATFORM_EXPORT LxoFloatEnvelopeCR GetRedEnvelope () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeR GetRedEnvelopeR ();

    DGNPLATFORM_EXPORT LxoFloatEnvelopeCR GetGreenEnvelope () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeR GetGreenEnvelopeR ();

    DGNPLATFORM_EXPORT LxoFloatEnvelopeCR GetBlueEnvelope () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeR GetBlueEnvelopeR ();

    DGNPLATFORM_EXPORT LxoFloatEnvelopeCR GetAlphaEnvelope () const;
    DGNPLATFORM_EXPORT LxoFloatEnvelopeR GetAlphaEnvelopeR ();
};

//=======================================================================================
// @bsiclass                                                      PaulChater     09/11
//=======================================================================================
struct LxoBGradientProcedure : public LxoGradientProcedure
{
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;
    friend struct LxoGradientProcedure;
// __PUBLISH_SECTION_END__

private:
    LxoBGradientProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
// __PUBLISH_SECTION_START__
};

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoBoardsProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_woodColor;
    RgbFactor       m_ringColor;
    int32_t         m_boardsPerColumn;
    int32_t         m_boardsPerRow;
    int32_t         m_patternId;
    double          m_crackWidth;
    double          m_brightnessVariation;
    double          m_horizontalVariation;
    double          m_grainScale;

    LxoBoardsProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetWoodColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetWoodColorR ();
    DGNPLATFORM_EXPORT void SetWoodColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetRingColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetRingColorR ();
    DGNPLATFORM_EXPORT void SetRingColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetCrackWidth () const;
    DGNPLATFORM_EXPORT void SetCrackWidth (double crackWidth);

    DGNPLATFORM_EXPORT double GetBrightnessVariation () const;
    DGNPLATFORM_EXPORT void SetBrightnessVariation (double brightnessVariation);

    DGNPLATFORM_EXPORT double GetHorizontalVariation () const;
    DGNPLATFORM_EXPORT void SetHorizontalVariation (double horizontalVariation);

    DGNPLATFORM_EXPORT double GetGrainScale () const;
    DGNPLATFORM_EXPORT void SetGrainScale (double grainScale);

    DGNPLATFORM_EXPORT int32_t GetBoardsPerColumn () const;
    DGNPLATFORM_EXPORT void SetBoardsPerColumn (int32_t boardsPerColumn);

    DGNPLATFORM_EXPORT int32_t GetBoardsPerRow () const;
    DGNPLATFORM_EXPORT void SetBoardsPerRow (int32_t boardsPerRow);

    DGNPLATFORM_EXPORT int32_t GetPatternId () const;
    DGNPLATFORM_EXPORT void SetPatternId (int32_t patternId);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoBrickProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                      Paul.Chater     11/09
    //=======================================================================================
    enum BrickBondType
        {
        BRICKBONDTYPE_Running   = 0,
        BRICKBONDTYPE_Stack,
        BRICKBONDTYPE_OneThirdRunning,
        BRICKBONDTYPE_Flemish,
        BRICKBONDTYPE_Common,
        BRICKBONDTYPE_Dutch,
        BRICKBONDTYPE_English,
        };
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_brickColor;
    RgbFactor       m_mortarColor;
    BrickBondType   m_bondType;
    int32_t         m_patternId;
    int32_t         m_headerCourseInterval;
    bool            m_flemishHeaders;
    double          m_mortarWidth;
    double          m_brightnessVariation;
    double          m_horizontalVariation;
    double          m_noisiness;
    double          m_aspectRatio;

    LxoBrickProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetBrickColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetBrickColorR ();
    DGNPLATFORM_EXPORT void SetBrickColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetMortarColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetMortarColorR ();
    DGNPLATFORM_EXPORT void SetMortarColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetMortarWidth () const;
    DGNPLATFORM_EXPORT void SetMortarWidth (double mortarWidth);

    DGNPLATFORM_EXPORT double GetBrightnessVariation () const;
    DGNPLATFORM_EXPORT void SetBrightnessVariation (double brightnessVariation);

    DGNPLATFORM_EXPORT double GetHorizontalVariation () const;
    DGNPLATFORM_EXPORT void SetHorizontalVariation (double horizontalVariation);

    DGNPLATFORM_EXPORT double GetNoisiness () const;
    DGNPLATFORM_EXPORT void SetNoisiness (double noisiness);

    DGNPLATFORM_EXPORT double GetAspectRatio () const;
    DGNPLATFORM_EXPORT void SetAspectRatio (double aspectRatio);

    DGNPLATFORM_EXPORT BrickBondType GetBondType () const;
    DGNPLATFORM_EXPORT void SetBondType (BrickBondType bondType);

    DGNPLATFORM_EXPORT int32_t GetPatternId () const;
    DGNPLATFORM_EXPORT void SetPatternId (int32_t patternId);

    DGNPLATFORM_EXPORT int32_t GetHeaderCourseInterval () const;
    DGNPLATFORM_EXPORT void SetHeaderCourseInterval (int32_t headerCourseInterval);

    DGNPLATFORM_EXPORT bool GetFlemishHeaders () const;
    DGNPLATFORM_EXPORT void SetFlemishHeaders (bool flemishHeaders);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoBentleyCheckerProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__

    RgbFactor       m_color1;
    RgbFactor       m_color2;
    double          m_checksPerMeter;

    LxoBentleyCheckerProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetColor1 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor1R ();
    DGNPLATFORM_EXPORT void SetColor1 (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetColor2 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor2R ();
    DGNPLATFORM_EXPORT void SetColor2 (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetChecksPerMeter () const;
    DGNPLATFORM_EXPORT void SetChecksPerMeter (double checksPerMeter);

    };


/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoBWNoiseProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    LxoBWNoiseProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoChecker3dProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_color1;
    RgbFactor       m_color2;
    double          m_checksPerMeter;

    LxoChecker3dProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetColor1 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor1R ();
    DGNPLATFORM_EXPORT void SetColor1 (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetColor2 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor2R ();
    DGNPLATFORM_EXPORT void SetColor2 (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetChecksPerMeter () const;
    DGNPLATFORM_EXPORT void SetChecksPerMeter (double checksPerMeter);
    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoCloudsProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_cloudColor;
    RgbFactor       m_skyColor;

    double          m_thickness;
    double          m_complexity;
    double          m_noise;
    bool            m_cloudsOnly;

private:
// __PUBLISH_SECTION_END__
    LxoCloudsProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetCloudColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetCloudColorR ();
    DGNPLATFORM_EXPORT void SetCloudColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetSkyColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetSkyColorR ();
    DGNPLATFORM_EXPORT void SetSkyColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetThickness () const;
    DGNPLATFORM_EXPORT void SetThickness (double thickness);

    DGNPLATFORM_EXPORT double GetComplexity () const;
    DGNPLATFORM_EXPORT void SetComplexity (double complexity);

    DGNPLATFORM_EXPORT double GetNoise () const;
    DGNPLATFORM_EXPORT void SetNoise (double noise);

    DGNPLATFORM_EXPORT bool GetCloudsOnly () const;
    DGNPLATFORM_EXPORT void SetCloudsOnly (bool cloudsOnly);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoColorNoiseProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    LxoColorNoiseProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoFlameProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_color;
    double          m_flameHeight;
    double          m_flameWidth;
    double          m_turbulence;
    double          m_complexity;
    double          m_flickerSpeed;

private:
// __PUBLISH_SECTION_END__
    LxoFlameProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColorR ();
    DGNPLATFORM_EXPORT void SetColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetFlameHeight () const;
    DGNPLATFORM_EXPORT void SetFlameHeight (double flameHeight);

    DGNPLATFORM_EXPORT double GetFlameWidth () const;
    DGNPLATFORM_EXPORT void SetFlameWidth (double flameWidth);

    DGNPLATFORM_EXPORT double GetTurbulence () const;
    DGNPLATFORM_EXPORT void SetTurbulence (double turbulence);

    DGNPLATFORM_EXPORT double GetComplexity () const;
    DGNPLATFORM_EXPORT void SetComplexity (double complexity);

    DGNPLATFORM_EXPORT double GetFlickerSpeed () const;
    DGNPLATFORM_EXPORT void SetFlickerSpeed (double flickerSpeed);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoFogProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_color;
    double          m_minDensity;
    double          m_maxDensity;
    double          m_driftSpeed;
    double          m_swirlSpeed;
    double          m_thickness;
    double          m_contrast;
    double          m_complexity;


private:
// __PUBLISH_SECTION_END__
    LxoFogProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColorR ();
    DGNPLATFORM_EXPORT void SetColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetMinDensity () const;
    DGNPLATFORM_EXPORT void SetMinDensity (double minDensity);

    DGNPLATFORM_EXPORT double GetMaxDensity () const;
    DGNPLATFORM_EXPORT void SetMaxDensity (double maxDensity);

    DGNPLATFORM_EXPORT double GetDriftSpeed () const;
    DGNPLATFORM_EXPORT void SetDriftSpeed (double driftSpeed);

    DGNPLATFORM_EXPORT double GetSwirlSpeed () const;
    DGNPLATFORM_EXPORT void SetSwirlSpeed (double swirlSpeed);

    DGNPLATFORM_EXPORT double GetThickness () const;
    DGNPLATFORM_EXPORT void SetThickness (double thickness);

    DGNPLATFORM_EXPORT double GetContrast () const;
    DGNPLATFORM_EXPORT void SetContrast (double contrast);

    DGNPLATFORM_EXPORT double GetComplexity () const;
    DGNPLATFORM_EXPORT void SetComplexity (double complexity);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoMarbleProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_marbleColor;
    RgbFactor       m_veinColor;
    double          m_veinTightness;
    double          m_complexity;

private:
// __PUBLISH_SECTION_END__
    LxoMarbleProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetMarbleColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetMarbleColorR ();
    DGNPLATFORM_EXPORT void SetMarbleColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetVeinColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetVeinColorR ();
    DGNPLATFORM_EXPORT void SetVeinColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetVeinTightness () const;
    DGNPLATFORM_EXPORT void SetVeinTightness (double veinTightness);

    DGNPLATFORM_EXPORT double GetComplexity () const;
    DGNPLATFORM_EXPORT void SetComplexity (double complexity);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoRGBColorCubeProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    LxoRGBColorCubeProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoSandProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_sandColor;
    RgbFactor       m_contrastColor;
    double          m_fraction;
private:
// __PUBLISH_SECTION_END__
    LxoSandProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:
    DGNPLATFORM_EXPORT RgbFactor const& GetSandColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetSandColorR ();
    DGNPLATFORM_EXPORT void SetSandColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetContrastColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetContrastColorR ();
    DGNPLATFORM_EXPORT void SetContrastColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetFraction () const;
    DGNPLATFORM_EXPORT void SetFraction (double fraction);


    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoStoneProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_tintColor;
    RgbFactor       m_mortarColor;
    int32_t         m_stoneColors;
    int32_t         m_stoneColorOffset;
    double          m_mortarWidth;
    double          m_noisiness;

private:
// __PUBLISH_SECTION_END__
    LxoStoneProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetTintColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetTintColorR ();
    DGNPLATFORM_EXPORT void SetTintColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetMortarColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetMortarColorR ();
    DGNPLATFORM_EXPORT void SetMortarColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT int32_t GetStoneColors () const;
    DGNPLATFORM_EXPORT void SetStoneColors (int32_t stoneColors);

    DGNPLATFORM_EXPORT int32_t GetStoneColorOffset () const;
    DGNPLATFORM_EXPORT void SetStoneColorOffset (int32_t stoneColorOffset);

    DGNPLATFORM_EXPORT double GetMortarWidth () const;
    DGNPLATFORM_EXPORT void SetMortarWidth (double mortarWidth);

    DGNPLATFORM_EXPORT double GetNoisiness () const;
    DGNPLATFORM_EXPORT void SetNoisiness (double noisiness);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoTurbulenceProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    double          m_complexity;
private:
// __PUBLISH_SECTION_END__
    LxoTurbulenceProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT double GetComplexity () const;
    DGNPLATFORM_EXPORT void SetComplexity (double complexity);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoTurfProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_turfColor;
    RgbFactor       m_contrastColor;
private:
// __PUBLISH_SECTION_END__
    LxoTurfProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetTurfColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetTurfColorR ();
    DGNPLATFORM_EXPORT void SetTurfColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetContrastColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetContrastColorR ();
    DGNPLATFORM_EXPORT void SetContrastColor (double red, double green, double blue);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoWaterProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_waterColor;
    double          m_rippleScale;
    double          m_rippleComplexity;
    double          m_waveScale;
    double          m_waveComplexity;
    double          m_waveMinimum;
    double          m_ripplesPerWave;
    double          m_roughness;

private:
// __PUBLISH_SECTION_END__
    LxoWaterProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetWaterColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetWaterColorR ();
    DGNPLATFORM_EXPORT void SetWaterColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetRippleScale () const;
    DGNPLATFORM_EXPORT void SetRippleScale (double rippleScale);

    DGNPLATFORM_EXPORT double GetRippleComplexity () const;
    DGNPLATFORM_EXPORT void SetRippleComplexity (double rippleComplexity);

    DGNPLATFORM_EXPORT double GetWaveScale () const;
    DGNPLATFORM_EXPORT void SetWaveScale (double waveScale);

    DGNPLATFORM_EXPORT double GetWaveComplexity () const;
    DGNPLATFORM_EXPORT void SetWaveComplexity (double waveComplexity);

    DGNPLATFORM_EXPORT double GetWaveMinimum () const;
    DGNPLATFORM_EXPORT void SetWaveMinimum (double waveMinimum);

    DGNPLATFORM_EXPORT double GetRipplesPerWave () const;
    DGNPLATFORM_EXPORT void SetRipplesPerWave (double ripplesPerWave);

    DGNPLATFORM_EXPORT double GetRoughness () const;
    DGNPLATFORM_EXPORT void SetRoughness (double roughness);

    };


/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoWavesProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_wavesColor;
    RgbFactor       m_contrastColor;

private:
// __PUBLISH_SECTION_END__
    LxoWavesProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetWavesColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetWavesColorR ();
    DGNPLATFORM_EXPORT void SetWavesColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetContrastColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetContrastColorR ();
    DGNPLATFORM_EXPORT void SetContrastColor (double red, double green, double blue);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoBentleyWoodProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_woodColor;
    RgbFactor       m_ringColor;
private:
// __PUBLISH_SECTION_END__
    LxoBentleyWoodProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetWoodColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetWoodColorR ();
    DGNPLATFORM_EXPORT void SetWoodColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetRingColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetRingColorR ();
    DGNPLATFORM_EXPORT void SetRingColor (double red, double green, double blue);

    };


/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoAdvancedWoodProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_woodColor;
    RgbFactor       m_ringColor;
private:
// __PUBLISH_SECTION_END__
    LxoAdvancedWoodProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetWoodColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetWoodColorR ();
    DGNPLATFORM_EXPORT void SetWoodColor (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetRingColor () const;
    DGNPLATFORM_EXPORT RgbFactor& GetRingColorR ();
    DGNPLATFORM_EXPORT void SetRingColor (double red, double green, double blue);

    };

/*=================================================================================**//**
* @bsiclass                                                     Paul.Chater     08/10
+===============+===============+===============+===============+===============+======*/
struct LxoOcclusionProcedure : public LxoProcedure
    {
    friend struct MaterialMapLayer;
    friend struct LightMap;
    friend struct LxoProcedure;

public:
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__
    //=======================================================================================
    // @bsiclass                                                     Paul.Chater     11/09
    //=======================================================================================
    enum OcclusionType
        {
        LXOOCCLUSIONTYPE_Uniform        = 0,
        LXOOCCLUSIONTYPE_Up             = 1,
        LXOOCCLUSIONTYPE_DownSlope      = 2,
        LXOOCCLUSIONTYPE_Reflection     = 3,
        LXOOCCLUSIONTYPE_Concavity      = 4,
        LXOOCCLUSIONTYPE_Convexity      = 5,
        LXOOCCLUSIONTYPE_BothCC         = 6,
        } ;
// __PUBLISH_SECTION_END__
#if !defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

private:
// __PUBLISH_SECTION_END__
    RgbFactor       m_color1;
    double          m_alpha1;
    RgbFactor       m_color2;
    double          m_alpha2;
    double          m_value1;
    double          m_value2;
    OcclusionType   m_occType;
    uint32_t        m_occlusionRays;
    double          m_occlusionDistance;
    double          m_variance;
    double          m_varianceScale;
    double          m_spreadAngle;
    double          m_maxCavityAngle;
    bool            m_sameSurface;
    double          m_bias;
    double          m_gain;
private:
// __PUBLISH_SECTION_END__
    LxoOcclusionProcedure ();

    virtual LxoProcedure::ProcedureType _GetType () const override;
    virtual void _InitDefaults () override;
    virtual BentleyStatus _Copy (LxoProcedureCR rhs) override;
    virtual bool _Equals (LxoProcedureCR rhs) const override;
    virtual void _GetSymbolsForPublishing (LxoProcedureCexprMemberList& variableList, LxoProcedureCexprMemberList& pointerList) override;

// __PUBLISH_SECTION_START__
// __PUBLISH_CLASS_VIRTUAL__
public:

    DGNPLATFORM_EXPORT RgbFactor const& GetColor1 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor1R ();
    DGNPLATFORM_EXPORT void SetColor1 (double red, double green, double blue);

    DGNPLATFORM_EXPORT RgbFactor const& GetColor2 () const;
    DGNPLATFORM_EXPORT RgbFactor& GetColor2R ();
    DGNPLATFORM_EXPORT void SetColor2 (double red, double green, double blue);

    DGNPLATFORM_EXPORT double GetAlpha1 () const;
    DGNPLATFORM_EXPORT void SetAlpha1 (double alpha1);

    DGNPLATFORM_EXPORT double GetAlpha2 () const;
    DGNPLATFORM_EXPORT void SetAlpha2 (double alpha2);

    DGNPLATFORM_EXPORT double GetValue1 () const;
    DGNPLATFORM_EXPORT void SetValue1 (double value1);

    DGNPLATFORM_EXPORT double GetValue2 () const;
    DGNPLATFORM_EXPORT void SetValue2 (double value2);

    DGNPLATFORM_EXPORT double GetOcclusionDistance () const;
    DGNPLATFORM_EXPORT void SetOcclusionDistance (double occlusionDistance);

    DGNPLATFORM_EXPORT double GetVariance () const;
    DGNPLATFORM_EXPORT void SetVariance (double variance);

    DGNPLATFORM_EXPORT double GetVarianceScale () const;
    DGNPLATFORM_EXPORT void SetVarianceScale (double varianceScale);

    DGNPLATFORM_EXPORT double GetSpreadAngle () const;
    DGNPLATFORM_EXPORT void SetSpreadAngle (double spreadAngle);

    DGNPLATFORM_EXPORT double GetMaxCavityAngle () const;
    DGNPLATFORM_EXPORT void SetMaxCavityAngle (double maxCavityAngle);

    DGNPLATFORM_EXPORT double GetBias () const;
    DGNPLATFORM_EXPORT void SetBias (double bias);

    DGNPLATFORM_EXPORT double GetGain () const;
    DGNPLATFORM_EXPORT void SetGain (double gain);

    DGNPLATFORM_EXPORT OcclusionType GetOcclusionType () const;
    DGNPLATFORM_EXPORT void SetOcclusionType (OcclusionType type);

    DGNPLATFORM_EXPORT uint32_t GetOcclusionRays () const;
    DGNPLATFORM_EXPORT void SetOcclusionRays (uint32_t occlusionRays);

    DGNPLATFORM_EXPORT bool GetSameSurface () const;
    DGNPLATFORM_EXPORT void SetSameSurface (bool sameSurface);

    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
// __PUBLISH_SECTION_END__
#endif //!defined (resource) && !defined (type_resource_generator)
// __PUBLISH_SECTION_START__

/** @endcond */
