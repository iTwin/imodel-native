/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ValueFormat.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */
#include    "UnitDefinition.h"
#include    "DgnModel.h"
#include    <Bentley/ValueFormat.h>

BEGIN_BENTLEY_DGN_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(AngleFormatter);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DirectionFormatter);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DistanceFormatter);
DEFINE_POINTER_SUFFIX_TYPEDEFS(PointFormatter);
DEFINE_POINTER_SUFFIX_TYPEDEFS(AreaFormatter);
DEFINE_POINTER_SUFFIX_TYPEDEFS(VolumeFormatter);
DEFINE_POINTER_SUFFIX_TYPEDEFS(DateTimeFormatter);

typedef RefCountedPtr<AngleFormatter>       AngleFormatterPtr;
typedef RefCountedPtr<DirectionFormatter>   DirectionFormatterPtr;
typedef RefCountedPtr<DistanceFormatter>    DistanceFormatterPtr;
typedef RefCountedPtr<PointFormatter>       PointFormatterPtr;
typedef RefCountedPtr<AreaFormatter>        AreaFormatterPtr;
typedef RefCountedPtr<VolumeFormatter>      VolumeFormatterPtr;
typedef RefCountedPtr<DateTimeFormatter>    DateTimeFormatterPtr;

//=======================================================================================
//! Used to construct a string from a numerical angle value.  This class provides various
//! angle formatting options, including:
//!   - Angular Units:
//!       - Degrees ex. 30.0^
//!       - Degree-Minute-Second ex. 30^30'30"
//!       - Gradians ex. 100g
//!       - Radians ex. 1.57r
//!   - Precision:
//!       - Whole numbers ex. 30^
//!       - Decimal precision ex. 30.1^ or 30.123^
//!   - Trailing Zero Control:
//!       - Supress trailing zeros ex. 30.5^ vs 30.500^
//!
//! @note The output strings are unicode.  This means that the unicode degree
//! character (0xb0) is used.  In this documentation, the unicode degree symbol is
//! represented by the caret symbol: ^.
//=======================================================================================
struct AngleFormatter : RefCountedBase
{
private: 
    enum AngleUnit
    {
    ANGLE_UNIT_Invalid              = 0,
    ANGLE_UNIT_Degrees              = 1,
    ANGLE_UNIT_Minutes              = 2,
    ANGLE_UNIT_Seconds              = 3,
    ANGLE_UNIT_Grads                = 4,
    ANGLE_UNIT_Radians              = 5,
    };

   AngleMode      m_angleMode;
   AnglePrecision m_precision;
   bool           m_leadingZero;
   bool           m_trailingZeros;
   bool           m_allowNegative;
   bool           m_allowUnclamped;

protected:  
    Utf8Char m_decimalSeparator;

private:
    AngleFormatter() {Init();}
    AngleFormatter(AngleFormatterCR other);

    DGNPLATFORM_EXPORT void Init();
    void ConcatUnitLabel(Utf8StringR, AngleUnit) const;
    void ConcatIntegerString(Utf8StringR, int value, AngleUnit) const;
    void ConcatPrecisionString(Utf8StringR, double value, AngleUnit, double delta) const;

    bool UseTwoDigitMinWidth() const;
    void PrependLeadingZeroIfNeeded(Utf8StringR inString, double value) const;

public: 
    void InitModelSettings(GeometricModelCR);
    DGNPLATFORM_EXPORT uint16_t GetLegacyFormat() const;
    uint16_t GetLegacyPrecision() const  {return static_cast <uint16_t> (m_precision);}
    DGNPLATFORM_EXPORT StatusInt SetAngleModeFromLegacy(AngleFormatVals value);
    DGNPLATFORM_EXPORT StatusInt SetAnglePrecisionFromLegacy(int value);
    bool GetAllowUnclamped() const {return m_allowUnclamped;}
    void SetAllowUnclamped(bool newVal) {m_allowUnclamped = newVal;}

    //! Get the AngleMode used by this formatter.
    AngleMode GetAngleMode() const {return m_angleMode;}

    //! Get the Precision used by this formatter.
    AnglePrecision GetAnglePrecision() const {return m_precision;}

    //! Get the decimal separator used by this formatter.
    WChar GetDecimalSeparator() const {return m_decimalSeparator;}


    //! Test if this formatter will include a leading zero.  A leading zero is only
    //! included for values less than 1.0.  Ex. "0.5" vs. ".5"
    bool GetLeadingZero() const {return m_leadingZero;}

    //! Test if this formatter will include trailing zeros.  Trailing zeros are only included
    //! up to the requested precision.  Ex. "30.500" vs. "30.5"
    bool GetTrailingZeros() const {return m_trailingZeros;}

    //! Test if this formatter will include a negative sign for input values less than zero.
    //! Ex. "-30" vs. "30"
    bool GetAllowNegative() const {return m_allowNegative;}

    //! Change the AngleMode used by this formatter.
    void SetAngleMode(AngleMode newVal) {m_angleMode = newVal;}

    //! Change the Precision used by this formatter.
    void SetAnglePrecision(AnglePrecision newVal) {m_precision = newVal;}

    //! Set the formatter's decimal separator.
    void SetDecimalSeparator(Utf8Char newVal) {m_decimalSeparator = newVal;}

    //! Set the formatter's leading zero behavior.  A leading zero is only
    //! included for values less than 1.0.  Ex. "0.5" vs. ".5"
    //! @param[in] newVal pass true to include a leading zero for values less than 1.0
    void SetLeadingZero(bool newVal) {m_leadingZero = newVal;}

    //! Set the formatter's trailing zeros behavior.  Trailing zeros are only included
    //! up to the requested precision.  Ex. "30.500" vs. "30.5"
    //! @param[in] newVal pass true to zero pad the output string to the requested precision.
    void SetTrailingZeros(bool newVal) {m_trailingZeros = newVal;}

    //! Set the formatter's negative value behavior.  If allowed a negative sign will be
    //! included for values less than zero. Ex. "-30" vs. "30"
    void SetAllowNegative(bool newVal) {m_allowNegative = newVal;}

    //! Construct a formatter with default settings.
    static AngleFormatterPtr Create() {return new AngleFormatter();}

    //! Construct a formatter with settings from a model.
    //! @param[in] model Initialize the formatter from the settings in this model.
    static AngleFormatterPtr Create(GeometricModelCR model) {AngleFormatterPtr formatter = Create(); formatter->InitModelSettings(model); return formatter;}

    //! Construct a formatter which is a duplicate of an existing formatter.
    AngleFormatterPtr Clone() const {return new AngleFormatter(*this);}

    //! Use the settings defined in this formatter to convert an angle value to a string.
    //! @param[in] value Angle in degrees.
    DGNPLATFORM_EXPORT Utf8String ToString(double value) const;

    //! Use the settings defined in this formatter to convert an angle value to a string.
    //! @param[in]  value       Angle in radians.
    Utf8String ToStringFromRadians(double value) const {double degrees = Angle::RadiansToDegrees(value); return ToString(degrees);}
}; // AngleFormatter

//=======================================================================================
//! Used to construct a string from a numerical direction value.  Directions are different
//! from angles in that directions are absolute whereas angles are relative.  A direction
//! is always defined within a coordinate system whereas an angle represents the difference
//! between two directions.
//!
//! This class provides various direction formatting options including:
//!   - Bearings
//!       - N 30^ E
//!       - N 30^30'30" E
//!   - Azimuths
//!       - From a specified base direction.
//!       - Using a specied orientation.
//!
//! @note The input value is always assumed to be in degrees measured counter-clockwise
//!       from the positive x-axis.
//!
//! @note Azimuths can be formatted using a base direction and an orientation.  Common
//!     azimuth settings include:
//!       - Counter-clockwise from the positive x-axis.
//!           - East:  Input 0.0, Output 0.0^
//!           - North: Input 90.0, Output 90.0^
//!       - Clockwise from the positive y-axis.
//!           - East:  Input 0.0, Output 90.0^
//!           - North: Input 90.0, Output 0.0^
//!       - Clockwise from the negative y-axis.
//!           - East:  Input 0.0, Output -90.0^
//!           - North: Input 90.0, Output 180.0^
//!
//! @note The TrueNorth adjustment option applies in both Bearing and Azimuth modes.
//!       See #SetAddTrueNorth and #SetTrueNorthValue.
//!
//! @note The numerical portion of the direction string is formatted using the rules
//!     for angle formatting.  The settings for angle formatting are specified
//!     using methods from the base class AngleFormatter.
//=======================================================================================
struct DirectionFormatter : RefCountedBase
{
private:    
    AngleFormatterPtr   m_angleFormatter;
    DirectionMode       m_mode;
    bool                m_addTrueNorth;
    double              m_trueNorth;
    double              m_baseDirection;
    bool                m_clockwise;
    bool                m_bearingSpaces;

    DirectionFormatter() {Init();}
    DirectionFormatter(DirectionFormatterCR other);
    DGNPLATFORM_EXPORT void Init();
    DGNPLATFORM_EXPORT void InitModelSettings(GeometricModelCR);

public: 
    static DGNPLATFORM_EXPORT void DirFormatFromLegacyAngleMode(DirFormat& dirFormat, int tentsubmode);
    static DGNPLATFORM_EXPORT int DirFormatToLegacyAngleMode(DirFormat const& dirFormat);

    DGNPLATFORM_EXPORT StatusInt SetDirectionModeFromLegacy(int value);
    DGNPLATFORM_EXPORT int GetLegacyAngleMode() const;

    //! Get the angle formatter used by this formatter for the numeric portion of the direction.
    //! Changes made to this object will affect the future behavior of the DirectionFormatter.
    AngleFormatterR GetAngleFormatter() {return *m_angleFormatter;}

    //! Get the DirectionMode used by this formatter.
    DirectionMode GetDirectionMode() const {return m_mode;}

    //! Test if the formatter's true north value will be used.
    bool GetAddTrueNorth() const {return m_addTrueNorth;}

    //! Get the formatter's true north value.  If it is enabled, the formatter will
    //! add the true north value to the input value before formatting it.
    double GetTrueNorthValue() const {return m_trueNorth;}

    //! Get the base direction for this formatter.  For details see #SetBaseDirection.
    double GetBaseDirection() const {return m_baseDirection;}

    //! Get the orientation for azimuth directions.
    //! @note: Only used when DirectionMode is DirectionMode::Azimuth.
    //! @return true if directions are measure clockwise.
    bool GetClockwise() const {return m_clockwise;}

    //! Test if bearing directions will use additional spaces.  Ex. "N 60 E" vs. "N60E".
    //! @note: Only used when DirectionMode is DirectionMode::Bearing.
    bool GetBearingSpaces() const {return m_bearingSpaces;}

    //! Set the angle formatter used by this formatter for the numeric portion of the direction.
    //! A copy of the supplied object will be stored by the DirectionFormatter.  To access the
    //! copy, use GetAngleFormatter.
    void SetAngleFormatter(AngleFormatterCR f) {m_angleFormatter = f.Clone();}

    //! Set the DirectionMode used by this formatter.
    void SetDirectionMode(DirectionMode newVal) {m_mode = newVal;}

    //! Enable or disable the formatter's true north adjustment.
    void SetAddTrueNorth(bool newVal) {m_addTrueNorth = newVal;}

    //! Set the formatter's true north value.  If it is enabled, the formatter will
    //! add the true north value to the input value before formatting it.
    void SetTrueNorthValue(double newVal) {m_trueNorth = newVal;}

    //! Set the base direction for this formatter.  The base direction is defined as
    //! the direction that is formatted as 0.0.
    //! @note: Only used when DirectionMode is DirectionMode::Azimuth.
    //! @return The base direction is specified as an angle in degrees measured
    //! counter-clockwise from the positive x-axis.  For example, the positive x-axis
    //! is 0.0 and the positive y-axis is 90.0.
    void SetBaseDirection(double newVal) {m_baseDirection = newVal;}

    //! Set the orientation for azimuth directions.
    //! @note: Only used when DirectionMode is DirectionMode::Azimuth.
    void SetClockwise(bool newVal) {m_clockwise = newVal;}

    //! Enable or disable additional spaces for bearing directions.  Ex. "N 60 E" vs. "N60E".
    //! @note: Only used when DirectionMode is DirectionMode::Bearing.
    void SetBearingSpaces(bool newVal) {m_bearingSpaces = newVal;}

    //! Construct a formatter with default settings.
    static DirectionFormatterPtr Create() {return new DirectionFormatter();}

    //! Construct a formatter with settings from a model.
    //! @param[in] model Initialize the formatter from the settings in this model.
    static DirectionFormatterPtr Create(GeometricModelCR model) {DirectionFormatterPtr formatter = Create(); formatter->InitModelSettings(model); return formatter;}

    //! Construct a formatter which is a duplicate of an existing formatter.
    DirectionFormatterPtr Clone() const {return new DirectionFormatter(*this);}

    //! Use the settings defined in this formatter to convert a direction value to a string.
    //! @param[in]  value       Direction in degrees measured counter clockwise from the positive x-axis.
    DGNPLATFORM_EXPORT Utf8String ToString(double value) const;

    //! Use the settings defined in this formatter to convert a direction value to a string.
    //! @param[in]  value       Direction in radians measured counter clockwise from the positive x-axis.
    Utf8String ToStringFromRadians(double value) const {double degrees = Angle::RadiansToDegrees(value); return ToString(degrees);}
}; // DirectionFormatter

//=======================================================================================
//! Used to construct a string from a numerical distance value.
//!
//! This class provides various distance formatting options including:
//!   - UnitFormat
//!       - Master Units Only ex. 10.25'
//!       - Master Units and Sub Units ex. 10'-3"
//!   - PrecisionFormat
//!       - Whole numbers ex. 10
//!       - Decimal precision ex. 10.25
//!       - Fractional precision ex. 10 1/4
//!       - Scientific precision ex. 1.025E2
//!   - Decimal Separator
//!       - Period ex. 10.5
//!       - Comma ex. 10,5
//!   - Many more
//!
//! @note  The input value is assumed to be supplied in meters.
//!
// @bsiclass
//=======================================================================================
struct DistanceFormatter : DoubleFormatterBase, RefCountedBase
{
    DEFINE_T_SUPER(DoubleFormatterBase)
private:
    bool m_unitFlag;
    bool m_suppressZeroMasterUnits;
    bool m_suppressZeroSubUnits;
    bool m_isDgnCoordReadOutCapable;
    DgnUnitFormat m_unitFormat;
    UnitDefinition m_masterUnit;       //!< Master Unit information
    UnitDefinition m_subUnit;          //!< Sub Unit information
    double m_scaleFactor;
    bool m_useDWGFormattingLogic;
    DwgUnitFormat m_dwgUnitFormat;

    DistanceFormatter() {Init();}
    DistanceFormatter(DistanceFormatterCR other);
    DGNPLATFORM_EXPORT void Init();
    DGNPLATFORM_EXPORT void InitModelSettings(GeometricModelCR);
    double GetSubPerMaster() const;

public: 
    bool GetUseDWGFormattingLogic() const {return m_useDWGFormattingLogic;}
    void SetUseDWGFormattingLogic(bool newVal) {m_useDWGFormattingLogic = newVal;}
    DwgUnitFormat GetDWGUnitFormat() const {return m_dwgUnitFormat;}
    DGNPLATFORM_EXPORT void SetDWGUnitFormat(DwgUnitFormat newVal);
    void SetPrecisionByte(Byte newVal) {m_precisionByte = newVal;}

    static DGNPLATFORM_EXPORT BentleyStatus ToDwgUnitFormat(DwgUnitFormat& dwgUnitFormatOut, DgnUnitFormat dgnUnitFormat, PrecisionFormat dgnPrecision, UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit);

    //! Get the DgnUnitFormat used by this formatter.
    DgnUnitFormat GetUnitFormat() const {return m_unitFormat;}

    //! Get the Master UnitDefinition used by this formatter.
    UnitDefinitionCR GetMasterUnits() const {return m_masterUnit;}

    //! Get the Sub UnitDefinition used by this formatter.
    UnitDefinitionCR GetSubUnits() const {return m_subUnit;}

    //! Get the system scale factor applied by this formatter.
    double GetScaleFactor() const {return m_scaleFactor;}

    //! Get the unit flag used by this formatter. Ex. "1M 0.000mm"
    bool GetUnitLabelFlag() const {return m_unitFlag;}

    //! Get the suppress zero master unit flag used by this formatter.
    bool GetSuppressZeroMasterUnits() const {return m_suppressZeroMasterUnits;}

    //! Get the suppress zero sub unit flag used by this formatter.
    bool GetSuppressZeroSubUnits() const {return m_suppressZeroSubUnits;}

    //! Get the DgnCoorinateReadOutCapable flag used by this formatter.
    bool GetIsDgnCoordReadOutCapable() const {return m_isDgnCoordReadOutCapable;}

    //! Get the displayable resolution of this formatter.  This is a calculated value.
    DGNPLATFORM_EXPORT double GetMinimumResolution() const;

    //! Set the formatter's UnitFormat.
    void SetUnitFormat(DgnUnitFormat newVal) {m_unitFormat = newVal;}

    //! Set the formatter's working units.
    DGNPLATFORM_EXPORT StatusInt SetUnits(UnitDefinitionCR newMasterUnit, UnitDefinitionCP newSubUnit);

    //! Set the formatter's scale factor.
    void SetScaleFactor(double scaleFactor) {if (scaleFactor != 0.0) m_scaleFactor = scaleFactor;}

    //! Set the formatter's unit flag.
    void SetUnitLabelFlag(bool newVal) {m_unitFlag = newVal;}

    //! Set the formatter's suppress zero master units flag (only applies if UnitFormat=UNIT_FORMAT_MUSU).
    void SetSuppressZeroMasterUnits(bool newVal) {m_suppressZeroMasterUnits = newVal;}

    //! Set the formatter's suppress zero sub units flag (only applies if UnitFormat=UNIT_FORMAT_MUSU).
    void SetSuppressZeroSubUnits(bool newVal) {m_suppressZeroSubUnits = newVal;}

    //! Set the formatter's IsDgnCoordinateReadoutCapable flag.
    void SetIsDgnCoordReadOutCapable(bool newVal) {m_isDgnCoordReadOutCapable = newVal;}

    //! Construct a formatter with default settings.
    static DistanceFormatterPtr Create() {return new DistanceFormatter();}

    //! Construct a formatter with settings from a model.
    //! @param[in] model Initialize the formatter from the settings in this model.
    static DistanceFormatterPtr Create(GeometricModelCR model) {DistanceFormatterPtr formatter = Create(); formatter->InitModelSettings(model); return formatter;}

    //! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the scale from the viewport's ACS.
    //! @param[in] vp                       Initialize the formatter from the settings in this viewport.
    static DGNPLATFORM_EXPORT DistanceFormatterPtr Create(DgnViewportR vp);

    //! Construct a formatter which is a duplicate of an existing formatter.
    DistanceFormatterPtr Clone() const {return new DistanceFormatter(*this);}

    //! Use the settings defined in this formatter to convert a value in meters to a string.
    DGNPLATFORM_EXPORT Utf8String ToString(double meters) const;
};

//=======================================================================================
//! Used to construct a string from a DPoint3d value.
//!
//! @note The individual coordinates of the point string are formatted using the rules
//!     for distance formatting.  The settings for distance formatting are specified
//!     by calling 'set' methods on the internal DistanceFormatter,
//!     ex. pointFormatter->GetDistanceFormatter().SetTrailingZeros();
//!
// @bsiclass
//=======================================================================================
struct PointFormatter : RefCountedBase
{

private:
    RefCountedPtr<IAuxCoordSys>     m_acs;
    DistanceFormatterPtr            m_distanceFormatter;
    bool                            m_is3d;

    PointFormatter();
    PointFormatter(PointFormatterCR other);
    PointFormatter(DistanceFormatterCR other);
    void Init();
    void InitModelSettings(GeometricModelCR, bool addGlobalOrigin);

public:

    //! Get the distance formatter used by this formatter for rectangular coordinates.
    //! Changes made to this object will affect the future behavior of the PointFormatter.
    DGNPLATFORM_EXPORT  DistanceFormatterR  GetDistanceFormatter();

    //! Get the auxilliary coordinate system used by this formatter.
    //! Changes made to this object will affect the future behavior of the PointFormatter.
    DGNPLATFORM_EXPORT  IAuxCoordSysR   GetAuxCoordSys();

    //! Get the is3d value used by this formatter.
    bool GetIs3d() const {return m_is3d;}

    //! Set the distance formatter used by this formatter for rectangular coordinates.
    //! A copy of the supplied object will be stored by the PointFormatter.  To access the
    //! copy, use GetDistanceFormatter.
    DGNPLATFORM_EXPORT void SetDistanceFormatter(DistanceFormatterCR);

    //! Set the auxilliary coordinate system used by this formatter.
    //! A copy of the supplied object will be stored by the PointFormatter.  To access the
    //! copy, use GetACS.
    DGNPLATFORM_EXPORT void SetAuxCoordSys(IAuxCoordSysCR);

    //! Set the formatter's is3d flag.
    void SetIs3d(bool newVal) {m_is3d = newVal;}

    //! Construct a formatter with default settings.
    static DGNPLATFORM_EXPORT PointFormatterPtr Create();

    //! Construct a formatter with settings from a model.
    //! @param[in] model Initialize the formatter from the settings in this model.
    //! @param[in] addGlobalOrigin Apply the global origin offset to points before formatting.
    static DGNPLATFORM_EXPORT PointFormatterPtr Create(GeometricModelCR model, bool addGlobalOrigin);

    //! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
    //! target model and uses the viewport's ACS.
    //! @param[in] vp                       Initialize the formatter from the settings in this viewport.
    static DGNPLATFORM_EXPORT PointFormatterPtr Create(DgnViewportR vp);

    //! Construct a PointFormatter from a DistanceFormatter.
    //! @param[in] distanceFormatter Initialize the formatter from the settings in this viewport.
    static DGNPLATFORM_EXPORT PointFormatterPtr Create(DistanceFormatterCR distanceFormatter);

    //! Construct a formatter which is a duplicate of an existing formatter.
    DGNPLATFORM_EXPORT PointFormatterPtr Clone() const;

    //! Use the settings defined in this formatter to convert a point value to a string.
    DGNPLATFORM_EXPORT Utf8String ToString(DPoint3dCR point) const;
}; // PointFormatter

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AreaOrVolumeFormatterBase : DoubleFormatterBase
{
    DEFINE_T_SUPER(DoubleFormatterBase)
protected:  
    bool m_showUnitLabel;
    bool m_useDWGFormattingLogic;
    bool m_labelDecoratorAsSuffix;
    UnitDefinition m_masterUnit;       //!< Master Unit information
    double m_scaleFactor;
    DwgUnitFormat m_dwgUnitFormat;

    DGNPLATFORM_EXPORT void Init();
    void InitFrom(AreaOrVolumeFormatterBase const&);
    void InitModelSettings(GeometricModelCR);

public:     
    bool GetUseDWGFormattingLogic() const {return m_useDWGFormattingLogic;}
    void SetUseDWGFormattingLogic(bool newVal) {m_useDWGFormattingLogic = newVal;}
    DwgUnitFormat GetDWGUnitFormat() const {return m_dwgUnitFormat;}
    void SetDWGUnitFormat(DwgUnitFormat newVal);

protected:
    AreaOrVolumeFormatterBase() {Init();}

//! Get the Master UnitDefinition used by this formatter.
public: 
    UnitDefinitionCR GetMasterUnits() const {return m_masterUnit;}

    //! Get the system scale factor applied by this formatter.
    double GetScaleFactor() const {return m_scaleFactor;}

    //! Get the flag which controls the use of unit labels by this formatter. Ex. "100mm"
    bool GetShowUnitLabel() const {return m_showUnitLabel;}

    //! Get the flag which sets unit label to m2 or Sq.m. Returns true for m2 and false for Sq.m.
    bool GetLabelDecoratorAsSuffix() const {return m_labelDecoratorAsSuffix;}

    //! Set the formatter's working units.
    StatusInt SetMasterUnit(UnitDefinitionCR newMasterUnit) {if (!newMasterUnit.IsValid()) return ERROR; m_masterUnit = newMasterUnit; return SUCCESS;}

    //! Set the formatter's scale factor.
    void SetScaleFactor(double scaleFactor) {if (scaleFactor != 0.0) m_scaleFactor = scaleFactor;}

    //! Set the formatter's unit flag.
    void SetShowUnitLabel(bool newVal) {m_showUnitLabel = newVal;}

    //! Set the formatter's unit label to m2 or Sq.m. Pass true for m2 and false for Sq.m.
    void SetLabelDecoratorAsSuffix(bool newVal) {m_labelDecoratorAsSuffix = newVal;}
};

//=======================================================================================
//! Used to construct a string from a numerical area value.
//!
//! @note  The input value is assumed to be supplied in squared millimeters
//!
// @bsiclass
//=======================================================================================
struct AreaFormatter : AreaOrVolumeFormatterBase, RefCountedBase
{
    DEFINE_T_SUPER(AreaOrVolumeFormatterBase)
private:
    AreaFormatter();
    AreaFormatter(AreaFormatterCR other);
    void Init();

    void SetPrecisionByte(Byte newVal) {m_precisionByte = newVal;}

public:
    //! Construct a formatter with default settings.
    static DGNPLATFORM_EXPORT AreaFormatterPtr Create();

    //! Construct a formatter with settings from a model.
    //! @param[in] model Initialize the formatter from the settings in this model.
    static DGNPLATFORM_EXPORT AreaFormatterPtr Create(GeometricModelCR model);

    //! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the scale from the viewport's ACS.
    //! @param[in] vp Initialize the formatter from the settings in this viewport.
    static DGNPLATFORM_EXPORT AreaFormatterPtr Create(DgnViewportR vp);

    //! Construct a formatter which is a duplicate of an existing formatter.
    DGNPLATFORM_EXPORT AreaFormatterPtr Clone() const;

    //! Use the settings defined in this formatter to convert a value in meters to a string.
    DGNPLATFORM_EXPORT Utf8String ToString(double meters) const;
};

//=======================================================================================
//! Used to construct a string from a numerical area value.
// @bsiclass
//=======================================================================================
struct VolumeFormatter : AreaOrVolumeFormatterBase, RefCountedBase
{
    DEFINE_T_SUPER(AreaOrVolumeFormatterBase)
private:    
    VolumeFormatter();
    VolumeFormatter(VolumeFormatterCR other);
    void Init();

public: 
    //! Construct a formatter with default settings.
    static DGNPLATFORM_EXPORT VolumeFormatterPtr Create();

    //! Construct a formatter with settings from a model.
    //! @param[in] model Initialize the formatter from the settings in this model.
    static DGNPLATFORM_EXPORT VolumeFormatterPtr Create(GeometricModelCR model);

    //! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the scale from the viewport's ACS.
    //! @param[in] vp                       Initialize the formatter from the settings in this viewport.
    static DGNPLATFORM_EXPORT VolumeFormatterPtr Create(DgnViewportR vp);

    //! Construct a formatter which is a duplicate of an existing formatter.
    DGNPLATFORM_EXPORT VolumeFormatterPtr Clone() const;

    //! Use the settings defined in this formatter to convert a value in meters to a string.
    //! @param[in] meters value.
    DGNPLATFORM_EXPORT Utf8String ToString(double meters) const;
};

//=======================================================================================
//! Used to construct a string from a time point value.
//! The formatting of a time point is specified as an ordered sequence of
//! DateTimeFormatPart components. The components are output in order from left to right.
//! A component can represent:
//!     - An element of the time like hour, month, or day of the week
//!     - A designator like AM or PM
//!     - A separator, such as the colon and whitespace in "4:30 PM"
//!     - A composition of several of the above
//! Components may also convey information as to how they should be formatted. For example
//! DATETIME_PART_HH and DATETIME_PART_h both refer to the hour element, but the former
//! is formatted as a 2-digit value from 00-23 while the latter is formatted as a
//! 1- or 2-digit value from 1-12.
// @bsiclass
//=======================================================================================
struct DateTimeFormatter : RefCountedBase
{
private:
    typedef bvector<DateTimeFormatPart> PartList;

    PartList m_partList;
    uint8_t m_fractionalPrecision;
    bool m_fractionalTrailingZeros;
    bool m_convertToLocalTime;
    Utf8Char m_dateSeparator;
    Utf8Char m_timeSeparator;
    Utf8Char m_decimalSeparator;

    DateTimeFormatter() {Reset();}
    DGNPLATFORM_EXPORT DateTimeFormatter(DateTimeFormatterCR other);

public:
    //! Get the character used as a decimal separator.
    Utf8Char GetDecimalSeparator() const {return m_decimalSeparator;}
    //! Get the character used to separate time components. Ex: ':' in "4:50"
    Utf8Char GetTimeSeparator() const {return m_timeSeparator;}
    //! Get the character used to separate date components. Ex: '/' in "4/5/2016"
    Utf8Char GetDateSeparator() const {return m_dateSeparator;}
    //! Get the precision used when formatting fractional seconds
    uint8_t GetFractionalSecondPrecision() const {return m_fractionalPrecision;}
    //! Get whether trailing zeros are included when formatting fractional seconds in order to match the specified precision
    bool GetTrailingZeros() const {return m_fractionalTrailingZeros;}
    //! Get whether times should be converted to user's local time before formatting is applied. True by default.
    bool GetConvertToLocalTime() const {return m_convertToLocalTime;}

    //! Set the character used as a decimal separator.
    void SetDecimalSeparator(Utf8Char separator) {m_decimalSeparator = separator;}
    //! Set the character used to separate time components.
    void SetTimeSeparator(Utf8Char separator) {m_timeSeparator = separator;}
    //! Set the character used to separate date components.
    void SetDateSeparator(Utf8Char separator) {m_dateSeparator = separator;}
    //! Set the precision used when formatting fractional seconds.
    void SetFractionalSecondPrecision(uint8_t precision) {m_fractionalPrecision = precision;}
    //! Set whether trailing zeros should be included when formatting fractional seconds in order to match the specified precision.
    void SetTrailingZeros(bool show) {m_fractionalTrailingZeros = show;}
    //! Set whether times should be converted to user's local time before formatting is applied.
    void SetConvertToLocalTime(bool convert){m_convertToLocalTime = convert;}

    //! Add a component to the ordered formatting sequence.
    DGNPLATFORM_EXPORT void AppendFormatPart(DateTimeFormatPart part);

    //! Remove all components from the formatting sequence.
    void ClearFormatParts() {m_partList.clear();}

    //! Reset all options to defaults and remove all components from the formatting sequence.
    DGNPLATFORM_EXPORT void Reset();

    //! Construct a new DateTimeFormatter with default settings.
    static DateTimeFormatterPtr Create() {return new DateTimeFormatter();}

    //! Construct a copy of this DateTimeFormatter.
    DateTimeFormatterPtr Clone() const {return new DateTimeFormatter(*this);}

    //! Format the time point value.
    //! If no components have been added to the formatting sequence, uses DATETIME_PART_General.
    DGNPLATFORM_EXPORT Utf8String ToString(DateTimeCR) const;
}; // DateTimeFormatter

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
