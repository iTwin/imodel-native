/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ValueFormat.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */
#include    "ModelInfo.h"
#include    "UnitDefinition.h"
#include    <Bentley/ValueFormat.h>

DGNPLATFORM_TYPEDEFS(AngleFormatter);
DGNPLATFORM_TYPEDEFS(DirectionFormatter);
DGNPLATFORM_TYPEDEFS(DistanceFormatter);
DGNPLATFORM_TYPEDEFS(PointFormatter);
DGNPLATFORM_TYPEDEFS(AreaFormatter);
DGNPLATFORM_TYPEDEFS(VolumeFormatter);
DGNPLATFORM_TYPEDEFS(DateTimeFormatter);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
//! @Note The output strings are unicode.  This means that the unicode degree
//! character (0xb0) is used.  In this documentation, the unicode degree symbol is
//! represented by the caret symbol: ^.
//!
//! @bsiclass
//=======================================================================================
struct AngleFormatter : RefCountedBase
{
//__PUBLISH_SECTION_END__

private: enum AngleUnit
    {
    ANGLE_UNIT_Invalid              = 0,
    ANGLE_UNIT_Degrees              = 1,
    ANGLE_UNIT_Minutes              = 2,
    ANGLE_UNIT_Seconds              = 3,
    ANGLE_UNIT_Grads                = 4,
    ANGLE_UNIT_Radians              = 5,
    };

private:    AngleMode           m_angleMode;
private:    AnglePrecision      m_precision;
private:    bool                m_leadingZero;
private:    bool                m_trailingZeros;
private:    bool                m_allowNegative;
private:    bool                m_allowUnclamped;

protected:  WChar               m_decimalSeparator;

private:  /* ctor */            AngleFormatter();
private:  /* ctor */            AngleFormatter (AngleFormatterCR other);

private:  void                  Init                        ();
private:  void                  ConcatUnitLabel             (WStringR, AngleUnit) const;
private:  void                  ConcatIntegerString         (WStringR, int value, AngleUnit) const;
private:  void                  ConcatPrecisionString       (WStringR, double value, AngleUnit, double delta) const;

private:    bool                UseTwoDigitMinWidth         () const;
private:    void                PrependLeadingZeroIfNeeded  (WStringR inString, double value) const;

public: void                                InitModelSettings (DgnModelCR);
public: DGNPLATFORM_EXPORT  UInt16          GetLegacyFormat              () const;
public: DGNPLATFORM_EXPORT  UInt16          GetLegacyPrecision           () const;
public: DGNPLATFORM_EXPORT  StatusInt       SetAngleModeFromLegacy       (AngleFormatVals value);
public: DGNPLATFORM_EXPORT  StatusInt       SetAnglePrecisionFromLegacy  (int value);
public: DGNPLATFORM_EXPORT  bool            GetAllowUnclamped () const;
public: DGNPLATFORM_EXPORT  void            SetAllowUnclamped (bool newVal);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

//! Get the AngleMode used by this formatter.
public: DGNPLATFORM_EXPORT  AngleMode           GetAngleMode        () const;

//! Get the Precision used by this formatter.
public: DGNPLATFORM_EXPORT  AnglePrecision      GetAnglePrecision   () const;

//! Get the decimal separator used by this formatter.
public: DGNPLATFORM_EXPORT  WChar               GetDecimalSeparator () const;

//! Test if this formatter will include a leading zero.  A leading zero is only
//! included for values less than 1.0.  Ex. "0.5" vs. ".5"
public: DGNPLATFORM_EXPORT  bool                GetLeadingZero      () const;

//! Test if this formatter will include trailing zeros.  Trailing zeros are only included
//! up to the requested precision.  Ex. "30.500" vs. "30.5"
public: DGNPLATFORM_EXPORT  bool                GetTrailingZeros    () const;

//! Test if this formatter will include a negative sign for input values less than zero.
//! Ex. "-30" vs. "30"
public: DGNPLATFORM_EXPORT  bool                GetAllowNegative    () const;

//! Change the AngleMode used by this formatter.
public: DGNPLATFORM_EXPORT  void                SetAngleMode        (AngleMode newVal);

//! Change the Precision used by this formatter.
public: DGNPLATFORM_EXPORT  void                SetAnglePrecision   (AnglePrecision newVal);

//! Set the formatter's decimal separator.
public: DGNPLATFORM_EXPORT  void                SetDecimalSeparator (WChar newVal);

//! Set the formatter's leading zero behavior.  A leading zero is only
//! included for values less than 1.0.  Ex. "0.5" vs. ".5"
//! @param[in] newVal pass true to include a leading zero for values less than 1.0
public: DGNPLATFORM_EXPORT  void                SetLeadingZero      (bool newVal);

//! Set the formatter's trailing zeros behavior.  Trailing zeros are only included
//! up to the requested precision.  Ex. "30.500" vs. "30.5"
//! @param[in] newVal pass true to zero pad the output string to the requested precision.
public: DGNPLATFORM_EXPORT  void                SetTrailingZeros    (bool newVal);

//! Set the formatter's negative value behavior.  If allowed a negative sign will be
//! included for values less than zero. Ex. "-30" vs. "30"
public: DGNPLATFORM_EXPORT  void                SetAllowNegative    (bool newVal);

//! Construct a formatter with default settings.
public: static DGNPLATFORM_EXPORT AngleFormatterPtr Create();

//! Construct a formatter with settings from a model.
//! @param[in] model Initialize the formatter from the settings in this model.
public: static DGNPLATFORM_EXPORT AngleFormatterPtr Create (DgnModelCR model);

//! Construct a formatter which is a duplicate of an existing formatter.
public: DGNPLATFORM_EXPORT  AngleFormatterPtr    Clone () const;

//! Use the settings defined in this formatter to convert an angle value to a string.
//! @param[in] value Angle in degrees.
public: DGNPLATFORM_EXPORT WString              ToString         (double value) const;

//! Use the settings defined in this formatter to convert an angle value to a string.
//! @param[in]  value       Angle in radians.
public: DGNPLATFORM_EXPORT WString              ToStringFromRadians   (double value) const;


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
//! @Note The input value is always assumed to be in degrees measured counter-clockwise
//!       from the positive x-axis.
//!
//! @Note Azimuths can be formatted using a base direction and an orientation.  Common
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
//! @Note The TrueNorth adjustment option applies in both Bearing and Azimuth modes.
//!       See #SetAddTrueNorth and #SetTrueNorthValue.
//!
//! @Note The numerical portion of the direction string is formatted using the rules
//!     for angle formatting.  The settings for angle formatting are specified
//!     using methods from the base class AngleFormatter.
//!
//! @bsiclass
//=======================================================================================
struct DirectionFormatter : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:    AngleFormatterPtr   m_angleFormatter;
private:    DirectionMode       m_mode;
private:    bool                m_addTrueNorth;
private:    double              m_trueNorth;
private:    double              m_baseDirection;
private:    bool                m_clockwise;
private:    bool                m_bearingSpaces;

private:    /* ctor */          DirectionFormatter ();
private:    /* ctor */          DirectionFormatter (DirectionFormatterCR other);
private:    void                Init ();
private:    void                InitModelSettings (DgnModelCR);

public:     static DGNPLATFORM_EXPORT void   DirFormatFromLegacyAngleMode   (DirFormat& dirFormat, int tentsubmode);
public:     static DGNPLATFORM_EXPORT int    DirFormatToLegacyAngleMode     (DirFormat const& dirFormat);

public:     DGNPLATFORM_EXPORT void          InitFromLegacyTCBFormat        (DirFormat const&, UInt16 const& angleFmt, UInt16 const& precision);
public:     DGNPLATFORM_EXPORT void          ToLegacyTCBFormat              (DirFormat& format, UInt16& angleFmt, UInt16& precision) const;
public:     DGNPLATFORM_EXPORT StatusInt     SetDirectionModeFromLegacy     (int value);
public:     DGNPLATFORM_EXPORT int           GetLegacyAngleMode             () const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

//! Get the angle formatter used by this formatter for the numeric portion of the direction.
//! Changes made to this object will affect the future behavior of the DirectionFormatter.
public:     DGNPLATFORM_EXPORT  AngleFormatterR    GetAngleFormatter ();

//! Get the DirectionMode used by this formatter.
public:     DGNPLATFORM_EXPORT DirectionMode       GetDirectionMode    () const;

//! Test if the formatter's true north value will be used.
public:     DGNPLATFORM_EXPORT bool                GetAddTrueNorth     () const;

//! Get the formatter's true north value.  If it is enabled, the formatter will
//! add the true north value to the input value before formatting it.
public:     DGNPLATFORM_EXPORT double              GetTrueNorthValue   () const;

//! Get the base direction for this formatter.  For details see #SetBaseDirection.
public:     DGNPLATFORM_EXPORT double              GetBaseDirection    () const;

//! Get the orientation for azimuth directions.
//! @Note: Only used when DirectionMode is DirectionMode::Azimuth.
//! @Return true if directions are measure clockwise.
public:     DGNPLATFORM_EXPORT bool                GetClockwise        () const;

//! Test if bearing directions will use additional spaces.  Ex. "N 60 E" vs. "N60E".
//! @Note: Only used when DirectionMode is DirectionMode::Bearing.
public:     DGNPLATFORM_EXPORT bool                GetBearingSpaces    () const;

//! Set the angle formatter used by this formatter for the numeric portion of the direction.
//! A copy of the supplied object will be stored by the DirectionFormatter.  To access the
//! copy, use GetAngleFormatter.
public:     DGNPLATFORM_EXPORT  void               SetAngleFormatter (AngleFormatterCR);

//! Set the DirectionMode used by this formatter.
public:     DGNPLATFORM_EXPORT void                SetDirectionMode    (DirectionMode newVal);

//! Enable or disable the formatter's true north adjustment.
public:     DGNPLATFORM_EXPORT void                SetAddTrueNorth     (bool newVal);

//! Set the formatter's true north value.  If it is enabled, the formatter will
//! add the true north value to the input value before formatting it.
public:     DGNPLATFORM_EXPORT void                SetTrueNorthValue   (double newVal);

//! Set the base direction for this formatter.  The base direction is defined as
//! the direction that is formatted as 0.0.
//! @Note: Only used when DirectionMode is DirectionMode::Azimuth.
//! @Return The base direction is specified as an angle in degrees measured
//! counter-clockwise from the positive x-axis.  For example, the positive x-axis
//! is 0.0 and the positive y-axis is 90.0.
public:     DGNPLATFORM_EXPORT void                SetBaseDirection    (double newVal);

//! Set the orientation for azimuth directions.
//! @Note: Only used when DirectionMode is DirectionMode::Azimuth.
public:     DGNPLATFORM_EXPORT void                SetClockwise        (bool newVal);

//! Enable or disable additional spaces for bearing directions.  Ex. "N 60 E" vs. "N60E".
//! @Note: Only used when DirectionMode is DirectionMode::Bearing.
public:     DGNPLATFORM_EXPORT void                SetBearingSpaces    (bool newVal);

//! Construct a formatter with default settings.
public: static DGNPLATFORM_EXPORT DirectionFormatterPtr Create();

//! Construct a formatter with settings from a model.
//! @param[in] model Initialize the formatter from the settings in this model.
public: static DGNPLATFORM_EXPORT DirectionFormatterPtr Create (DgnModelCR model);

//! Construct a formatter which is a duplicate of an existing formatter.
public: DGNPLATFORM_EXPORT  DirectionFormatterPtr    Clone () const;

//! Use the settings defined in this formatter to convert a direction value to a string.
//! @param[in]  value       Direction in degrees measured counter clockwise from the positive x-axis.
public:     DGNPLATFORM_EXPORT WString       ToString   (double value) const;

//! Use the settings defined in this formatter to convert a direction value to a string.
//! @param[in]  value       Direction in radians measured counter clockwise from the positive x-axis.
public:     DGNPLATFORM_EXPORT WString       ToStringFromRadians   (double value) const;

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
//! @Note  The input value is assumed to be supplied in Units of Resolution (uors).  Before
//! being formatted, the uor value will be scaled by the supplied UorPerStorageUnit value and
//! then converted from Storage units to the desired output units using the provided
//! UnitDefinitions.  An optional additional scale factor can also be supplied.
//!
// @bsiclass
//=======================================================================================
struct DistanceFormatter : DoubleFormatterBase, RefCountedBase
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(DoubleFormatterBase)
private:    bool                m_unitFlag;
private:    bool                m_suppressZeroMasterUnits;
private:    bool                m_suppressZeroSubUnits;
private:    bool                m_isDgnCoordReadOutCapable;
private:    DgnUnitFormat       m_unitFormat;
private:    UnitDefinition      m_masterUnit;       //!< Master Unit information
private:    UnitDefinition      m_subUnit;          //!< Sub Unit information
private:    double              m_scaleFactor;
private:    bool                m_useDWGFormattingLogic;
private:    DwgUnitFormat       m_dwgUnitFormat;

private:    WString             ToStringForDWG (double uors) const;

private:    /* ctor */          DistanceFormatter ();
private:    /* ctor */          DistanceFormatter (DistanceFormatterCR other);
private:    void                Init();
private:    void                InitModelSettings (DgnModelCR);
private:    double              GetSubPerMaster () const;
private:    double              GetMillimetersPerSub () const;

public: 
        bool GetUseDWGFormattingLogic () const;
        DGNPLATFORM_EXPORT void SetUseDWGFormattingLogic (bool newVal);
        DGNPLATFORM_EXPORT DwgUnitFormat GetDWGUnitFormat () const;
        DGNPLATFORM_EXPORT void SetDWGUnitFormat (DwgUnitFormat newVal);
        DGNPLATFORM_EXPORT void SetPrecisionByte (byte newVal);
        void GetUorsToMasterSubPositional (double inUors, Int64 *muP, Int64 *suP, double *puP, double* dmuP, double* dsuP, bool* negFlagP) const;
        DGNPLATFORM_EXPORT void ToLegacyTCBFormat (Autodim1& ad1) const;
        DGNPLATFORM_EXPORT void InitializeForDwg (ModelInfoCR);

static DGNPLATFORM_EXPORT void ReadLegacyTCBFormat (DgnUnitFormat&, PrecisionFormat&, Autodim1 const& ad1);
static DGNPLATFORM_EXPORT BentleyStatus ToDwgUnitFormat (DwgUnitFormat& dwgUnitFormatOut, DgnUnitFormat dgnUnitFormat, PrecisionFormat dgnPrecision, UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit);

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/

//! Get the DgnUnitFormat used by this formatter.
public: DGNPLATFORM_EXPORT DgnUnitFormat GetUnitFormat () const;

//! Get the Master UnitDefinition used by this formatter.
public: DGNPLATFORM_EXPORT UnitDefinitionCR GetMasterUnit () const;

//! Get the Sub UnitDefinition used by this formatter.
public: DGNPLATFORM_EXPORT UnitDefinitionCR GetSubUnit () const;

//! Get the system scale factor applied by this formatter.
public: DGNPLATFORM_EXPORT double GetScaleFactor () const;

//! Get the unit flag used by this formatter. Ex. "1M 0.000mm"
public: DGNPLATFORM_EXPORT bool GetUnitLabelFlag () const;

//! Get the suppress zero master unit flag used by this formatter.
public: DGNPLATFORM_EXPORT bool GetSuppressZeroMasterUnits () const;

//! Get the suppress zero sub unit flag used by this formatter.
public: DGNPLATFORM_EXPORT bool GetSuppressZeroSubUnits () const;

//! Get the DgnCoorinateReadOutCapable flag used by this formatter.
public: DGNPLATFORM_EXPORT bool GetIsDgnCoordReadOutCapable () const;

//! Get the displayable resolution of this formatter in uors.  This is a calculated value.
public: DGNPLATFORM_EXPORT double GetMinimumResolution () const;

//! Set the formatter's UnitFormat.
public: DGNPLATFORM_EXPORT void SetUnitFormat (DgnUnitFormat newVal);

//! Set the formatter's working units.
public: DGNPLATFORM_EXPORT StatusInt SetWorkingUnits (UnitDefinitionCR newMasterUnit, UnitDefinitionCP newSubUnit);

//! Set the formatter's scale factor.
public: DGNPLATFORM_EXPORT void SetScaleFactor (double newVal);

//! Set the formatter's unit flag.
public: DGNPLATFORM_EXPORT void SetUnitLabelFlag (bool newVal);

//! Set the formatter's suppress zero master units flag (only applies if UnitFormat=UNIT_FORMAT_MUSU).
public: DGNPLATFORM_EXPORT void SetSuppressZeroMasterUnits (bool newVal);

//! Set the formatter's suppress zero sub units flag (only applies if UnitFormat=UNIT_FORMAT_MUSU).
public: DGNPLATFORM_EXPORT void SetSuppressZeroSubUnits (bool newVal);

//! Set the formatter's IsDgnCoordinateReadoutCapable flag.
public: DGNPLATFORM_EXPORT void SetIsDgnCoordReadOutCapable (bool newVal);

//! Construct a formatter with default settings.
public: static DGNPLATFORM_EXPORT DistanceFormatterPtr Create ();

//! Construct a formatter with settings from a model.
//! @param[in] model Initialize the formatter from the settings in this model.
public: static DGNPLATFORM_EXPORT DistanceFormatterPtr Create (DgnModelCR model);

//! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
//! target model and the scale from the viewport's ACS.
//! @param[in] vp                       Initialize the formatter from the settings in this viewport.
public: static DGNPLATFORM_EXPORT DistanceFormatterPtr Create (ViewportR vp);

//! Construct a formatter which is a duplicate of an existing formatter.
public: DGNPLATFORM_EXPORT DistanceFormatterPtr Clone () const;

//! Use the settings defined in this formatter to convert a uor value to a string.
//! @param[in] uors uor value.
public: DGNPLATFORM_EXPORT WString ToString (double uors) const;

}; // DistanceFormatter

//=======================================================================================
//! Used to construct a string from a DPoint3d value.
//!
//! @Note The individual coordinates of the point string are formatted using the rules
//!     for distance formatting.  The settings for distance formatting are specified
//!     by calling 'set' methods on the internal DistanceFormatter,
//!     ex. pointFormatter->GetDistanceFormatter().SetTrailingZeros();
//!
// @bsiclass
//=======================================================================================
struct PointFormatter : RefCountedBase
{
//__PUBLISH_SECTION_END__

private:    RefCountedPtr<IAuxCoordSys>     m_acs;
private:    DistanceFormatterPtr            m_distanceFormatter;
private:    bool                            m_is3d;

private:  /* ctor */      PointFormatter   ();
private:  /* ctor */      PointFormatter (PointFormatterCR other);
private:  /* ctor */      PointFormatter (DistanceFormatterCR other);
private:  void            Init();
private:  void            InitModelSettings (DgnModelCR, bool addGlobalOrigin);

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/

//! Get the distance formatter used by this formatter for rectangular coordinates.
//! Changes made to this object will affect the future behavior of the PointFormatter.
public: DGNPLATFORM_EXPORT  DistanceFormatterR  GetDistanceFormatter ();

//! Get the auxilliary coordinate system used by this formatter.
//! Changes made to this object will affect the future behavior of the PointFormatter.
public: DGNPLATFORM_EXPORT  IAuxCoordSysR   GetAuxCoordSys ();

//! Get the is3d value used by this formatter.
public: DGNPLATFORM_EXPORT  bool            GetIs3d () const;

//! Set the distance formatter used by this formatter for rectangular coordinates.
//! A copy of the supplied object will be stored by the PointFormatter.  To access the
//! copy, use GetDistanceFormatter.
public: DGNPLATFORM_EXPORT  void            SetDistanceFormatter (DistanceFormatterCR);

//! Set the auxilliary coordinate system used by this formatter.
//! A copy of the supplied object will be stored by the PointFormatter.  To access the
//! copy, use GetACS.
public: DGNPLATFORM_EXPORT  void            SetAuxCoordSys (IAuxCoordSysCR);

//! Set the formatter's is3d flag.
public: DGNPLATFORM_EXPORT  void            SetIs3d (bool newVal);

//! Construct a formatter with default settings.
public: static DGNPLATFORM_EXPORT  PointFormatterPtr    Create ();

//! Construct a formatter with settings from a model.
//! @param[in] model Initialize the formatter from the settings in this model.
//! @param[in] addGlobalOrigin Apply the global origin offset to points before formatting.
public: static DGNPLATFORM_EXPORT  PointFormatterPtr    Create (DgnModelCR model, bool addGlobalOrigin);

//! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
//! target model and uses the viewport's ACS.
//! @param[in] vp                       Initialize the formatter from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  PointFormatterPtr    Create (ViewportR vp);

//! Construct a PointFormatter from a DistanceFormatter.
//! @param[in] distanceFormatter Initialize the formatter from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  PointFormatterPtr    Create (DistanceFormatterCR distanceFormatter);

//! Construct a formatter which is a duplicate of an existing formatter.
public: DGNPLATFORM_EXPORT  PointFormatterPtr    Clone () const;

//! Use the settings defined in this formatter to convert a point value to a string.
//! @param[in] point uor value.
public: DGNPLATFORM_EXPORT WString      ToString (DPoint3dCR point) const;

}; // PointFormatter

//=======================================================================================
//!
//!
// @bsiclass
//=======================================================================================
struct AreaOrVolumeFormatterBase : DoubleFormatterBase
{
private:    bool                m_dummy;    // published base class must have at least one member

//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(DoubleFormatterBase)
protected:  bool                m_showUnitLabel;
protected:  UnitDefinition      m_masterUnit;       //!< Master Unit information
protected:  double              m_scaleFactor;
protected:  bool                m_useDWGFormattingLogic;
protected:  DwgUnitFormat       m_dwgUnitFormat;
protected:  bool                m_labelDecoratorAsSuffix;

protected:  void                Init();
protected:  void                InitFrom (AreaOrVolumeFormatterBase const&);
protected:  void                InitModelSettings (DgnModelCR);

public:     bool                            GetUseDWGFormattingLogic () const;
public:     DGNPLATFORM_EXPORT void             SetUseDWGFormattingLogic (bool newVal);
public:     DGNPLATFORM_EXPORT DwgUnitFormat    GetDWGUnitFormat () const;
public:     DGNPLATFORM_EXPORT void             SetDWGUnitFormat (DwgUnitFormat newVal);
public:     DGNPLATFORM_EXPORT void             InitializeForDwg (ModelInfoCR);

/*__PUBLISH_SECTION_START__*/
private:
//__PUBLISH_SECTION_END__
protected:                                  // Make sure this is not constructible in the published API
/*__PUBLISH_SECTION_START__*/
AreaOrVolumeFormatterBase ();

//! Get the Master UnitDefinition used by this formatter.
public: DGNPLATFORM_EXPORT  UnitDefinitionCR    GetMasterUnit () const;

//! Get the system scale factor applied by this formatter.
public: DGNPLATFORM_EXPORT double GetScaleFactor () const;

//! Get the flag which controls the use of unit labels by this formatter. Ex. "100mm"
public: DGNPLATFORM_EXPORT bool GetShowUnitLabel () const;

//! Get the flag which sets unit label to m2 or Sq.m. Returns true for m2 and false for Sq.m.
public: DGNPLATFORM_EXPORT bool GetLabelDecoratorAsSuffix () const;

//! Set the formatter's working units.
public: DGNPLATFORM_EXPORT StatusInt SetMasterUnit (UnitDefinitionCR newUnit);

//! Set the formatter's scale factor.
public: DGNPLATFORM_EXPORT void SetScaleFactor (double newVal);

//! Set the formatter's unit flag.
public: DGNPLATFORM_EXPORT void SetShowUnitLabel (bool newVal);

//! Set the formatter's unit label to m2 or Sq.m. Pass true for m2 and false for Sq.m.
public: DGNPLATFORM_EXPORT void SetLabelDecoratorAsSuffix (bool newVal);
};

//=======================================================================================
//! Used to construct a string from a numerical area value.
//!
//! @Note  The input value is assumed to be supplied in squared millimeters
//!
// @bsiclass
//=======================================================================================
struct AreaFormatter : AreaOrVolumeFormatterBase, RefCountedBase
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(AreaOrVolumeFormatterBase)
private:    /* ctor */          AreaFormatter ();
private:    /* ctor */          AreaFormatter (AreaFormatterCR other);
private:    void                Init();

private:    WString             ToStringForDWG (double uors) const;

public:     DGNPLATFORM_EXPORT void             SetPrecisionByte (byte newVal);

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/

//! Construct a formatter with default settings.
public: static DGNPLATFORM_EXPORT  AreaFormatterPtr     Create ();

//! Construct a formatter with settings from a model.
//! @param[in] model Initialize the formatter from the settings in this model.
public: static DGNPLATFORM_EXPORT  AreaFormatterPtr     Create (DgnModelCR model);

//! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
//! target model and the scale from the viewport's ACS.
//! @param[in] vp                       Initialize the formatter from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  AreaFormatterPtr     Create (ViewportR vp);

//! Construct a formatter which is a duplicate of an existing formatter.
public: DGNPLATFORM_EXPORT  AreaFormatterPtr    Clone () const;

//! Use the settings defined in this formatter to convert a uor value to a string.
//! @param[in] uors uor value.
public: DGNPLATFORM_EXPORT WString              ToString (double uors) const;

}; // AreaFormatter

//=======================================================================================
//! Used to construct a string from a numerical area value.
//!
//! @Note  The input value is assumed to be supplied in squared Units of Resolution (uors).
//! Before being formatted, the uor value will be scaled by the supplied UorPerStorage
//! value and then converted from millimeters to the desired output unit using the provided
//! UnitDefinitions.  An optional additional scale factor can also be supplied.
//!
// @bsiclass
//=======================================================================================
struct VolumeFormatter : AreaOrVolumeFormatterBase, RefCountedBase
{
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(AreaOrVolumeFormatterBase)
private:    /* ctor */          VolumeFormatter ();
private:    /* ctor */          VolumeFormatter (VolumeFormatterCR other);
private:    void                Init();

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/

//! Construct a formatter with default settings.
public: static DGNPLATFORM_EXPORT  VolumeFormatterPtr     Create ();

//! Construct a formatter with settings from a model.
//! @param[in] model Initialize the formatter from the settings in this model.
public: static DGNPLATFORM_EXPORT  VolumeFormatterPtr     Create (DgnModelCR model);

//! Construct a formatter with settings from a viewport.  Gets the settings from the viewport's
//! target model and the scale from the viewport's ACS.
//! @param[in] vp                       Initialize the formatter from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  VolumeFormatterPtr     Create (ViewportR vp);

//! Construct a formatter which is a duplicate of an existing formatter.
public: DGNPLATFORM_EXPORT  VolumeFormatterPtr    Clone () const;

//! Use the settings defined in this formatter to convert a uor value to a string.
//! @param[in] uors uor value.
public: DGNPLATFORM_EXPORT WString              ToString (double uors) const;

}; // VolumeFormatter

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
//__PUBLISH_SECTION_END__
private:
    typedef bvector<DateTimeFormatPart> PartList;

    PartList                m_partList;
    UInt8                   m_fractionalPrecision;
    bool                    m_fractionalTrailingZeros;
    bool                    m_convertToLocalTime;
    WChar                   m_dateSeparator;
    WChar                   m_timeSeparator;
    WChar                   m_decimalSeparator;

    DateTimeFormatter();
    DateTimeFormatter (DateTimeFormatterCR other);
public:
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    //! Get the character used as a decimal separator.
    DGNPLATFORM_EXPORT WChar                           GetDecimalSeparator() const;
    //! Get the character used to separate time components. Ex: ':' in "4:50"
    DGNPLATFORM_EXPORT WChar                           GetTimeSeparator() const;
    //! Get the character used to separate date components. Ex: '/' in "4/5/2012"
    DGNPLATFORM_EXPORT WChar                           GetDateSeparator() const;
    //! Get the precision used when formatting fractional seconds
    DGNPLATFORM_EXPORT UInt8                           GetFractionalSecondPrecision() const;
    //! Get whether trailing zeros are included when formatting fractional seconds in order to match the specified precision
    DGNPLATFORM_EXPORT bool                            GetTrailingZeros() const;
    //! Get whether times should be converted to user's local time before formatting is applied. True by default.
    DGNPLATFORM_EXPORT bool                            GetConvertToLocalTime() const;

    //! Set the character used as a decimal separator.
    DGNPLATFORM_EXPORT void                            SetDecimalSeparator (WChar separator);
    //! Set the character used to separate time components.
    DGNPLATFORM_EXPORT void                            SetTimeSeparator (WChar separator);
    //! Set the character used to separate date components.
    DGNPLATFORM_EXPORT void                            SetDateSeparator (WChar separator);
    //! Set the precision used when formatting fractional seconds.
    DGNPLATFORM_EXPORT void                            SetFractionalSecondPrecision (UInt8 precision);
    //! Set whether trailing zeros should be included when formatting fractional seconds in order to match the specified precision.
    DGNPLATFORM_EXPORT void                            SetTrailingZeros (bool show);
    //! Set whether times should be converted to user's local time before formatting is applied.
    DGNPLATFORM_EXPORT void                            SetConvertToLocalTime (bool convert);

    //! Add a component to the ordered formatting sequence.
    DGNPLATFORM_EXPORT void                            AppendFormatPart (DateTimeFormatPart part);
    //! Remove all components from the formatting sequence.
    DGNPLATFORM_EXPORT void                            ClearFormatParts();
    //! Reset all options to defaults and remove all components from the formatting sequence.
    DGNPLATFORM_EXPORT void                            Reset();

    //! Construct a new DateTimeFormatter with default settings.
    static DGNPLATFORM_EXPORT DateTimeFormatterPtr     Create();
    //! Construct a copy of this DateTimeFormatter.
    DGNPLATFORM_EXPORT DateTimeFormatterPtr            Clone() const;
    //! Format the time point value.
    //! If no components have been added to the formatting sequence, uses DATETIME_PART_General.
    DGNPLATFORM_EXPORT WString                         ToString (DateTimeCR) const;
    }; // DateTimeFormatter

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
