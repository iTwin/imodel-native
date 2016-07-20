/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ValueParse.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ValueFormat.h"

DGNPLATFORM_TYPEDEFS(DoubleParser)
DGNPLATFORM_TYPEDEFS(AngleParser)
DGNPLATFORM_TYPEDEFS(DirectionParser)
DGNPLATFORM_TYPEDEFS(DistanceParser)
DGNPLATFORM_TYPEDEFS(PointParser)
DGNPLATFORM_TYPEDEFS(AreaOrVolumeParser)
DGNPLATFORM_TYPEDEFS(AreaParser)
DGNPLATFORM_TYPEDEFS(VolumeParser);

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<DoubleParser>         DoubleParserPtr;
typedef RefCountedPtr<AngleParser>          AngleParserPtr;
typedef RefCountedPtr<DirectionParser>      DirectionParserPtr;
typedef RefCountedPtr<DistanceParser>       DistanceParserPtr;
typedef RefCountedPtr<PointParser>          PointParserPtr;
typedef RefCountedPtr<AreaParser>           AreaParserPtr;
typedef RefCountedPtr<VolumeParser>         VolumeParserPtr;

//=======================================================================================
//! Used to parse values. 
// @bsiclass
//=======================================================================================
struct DoubleParser : RefCountedBase
{
private: 
    DoubleParser();
    DoubleParser(DoubleParserCR other);

public:
    //! Construct a parser with default settings.
    static DGNPLATFORM_EXPORT DoubleParserPtr Create();

    //! Construct a parser which is a duplicate of an existing parser.
    DGNPLATFORM_EXPORT DoubleParserPtr Clone() const;

    //! Parse a string into a numeric value.
    //! @param[out] outresulting value if successfully parsed.
    //! @param[in] in input string.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(double& out, Utf8CP in) const;
};

//=======================================================================================
//! Used to parse angles. 
//! Default parsing mode is in radians, results are always in degrees.
//! This assumes input is ready to be parsed as an angle.
//! If it is just a number the AngleMode determines how the number will be interpreted.
// @bsiclass
//=======================================================================================
struct AngleParser : RefCountedBase
{
private: 
    AngleMode m_angleMode;

    AngleParser() {Init();}
    AngleParser(AngleParserCR other) {m_angleMode = other.m_angleMode;}
    void Init() {m_angleMode = AngleMode::Degrees;}

public:
    void InitModelSettings(GeometricModelCR);
    DGNPLATFORM_EXPORT BentleyStatus SetAngleModeFromLegacy(AngleFormatVals value);
    DGNPLATFORM_EXPORT uint16_t GetLegacyFormat() const;

    //! Construct a parser with default settings.
    static AngleParserPtr Create() {return new AngleParser();}

    //! Construct a parser with the settings as stored in the model.
    //! @param[in] model Initialize the parser from the settings in this model.
    static AngleParserPtr Create(GeometricModelCR model) {AngleParserPtr formatter = Create(); formatter->InitModelSettings(model); return formatter;}

    //! Construct a parser which is a duplicate of an existing parser.
    AngleParserPtr Clone() const {return new AngleParser(*this);}

    //! Sets the angle mode for parsing the input string.
    //! AngleMode::Degrees, AngleMode::DegMin, and AngleMode::DegMinSec, are all treated the same.
    //! @param[in]  mode The mode to set.
    void SetAngleMode(AngleMode mode) {m_angleMode = mode;}

    //! Gets the angle mode for parsing the input string.
    //! AngleMode::Degrees, AngleMode::DegMin, and AngleMode::DegMinSec, are all treated the same.
    AngleMode GetAngleMode() {return m_angleMode;}

    //! Parse a string into an angle value in degrees.
    //! @param[out] out resulting angle in degrees if successfully parsed.
    //! @param[in]  in input string.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(double& out, Utf8CP in) const;
};

//=======================================================================================
//! Used to parse directions. 
//! Default parsing mode is in radians, results are always in degrees.
//! This assumes input is ready to be parsed as an angle.
//! If it is just a number the AngleMode determines how the number will be interpreted.
// @bsiclass
//=======================================================================================
struct DirectionParser : RefCountedBase
{
private: 
    double m_trueNorth;
    DirectionMode m_mode;
    bool m_isClockwise;
    double m_baseDirection;
    AngleParserPtr m_angleParser;

    DirectionParser();
    DirectionParser(DirectionParserCR other);
    void Init();
    void InitModelSettings(GeometricModelCR);
    int StringToDirection(double& dir, WCharCP inputString) const;

public:
    //! Construct a parser with default settings.
    static DGNPLATFORM_EXPORT DirectionParserPtr Create();

    //! Construct a parser with settings from a model.
    //! @param[in] model Initialize the parser from the settings in this model.
    static DGNPLATFORM_EXPORT DirectionParserPtr Create(GeometricModelCR model);

    //! Construct a parser which is a duplicate of an existing parser.
    DGNPLATFORM_EXPORT DirectionParserPtr Clone() const;

    //! Get the angle parser used by this parser for the numeric portion of the direction.
    //! Changes made to this object will affect the future behavior of the DirectionParser.
    AngleParserR GetAngleParser() {return *m_angleParser;}

    //! Set an offset in degrees that will be added to the parsed value before it is returned.
    void SetTrueNorthValue(double trueNorth) {m_trueNorth = trueNorth;}

    //! Get the offset in degrees that will be added to the parsed value before it is returned.
    double GetTrueNorthValue() {return m_trueNorth;}

    //! If the direction mode is Azimuth, then the baseDirection and clockwise flag will be applied.
    void SetDirectionMode(DirectionMode newVal) {m_mode = newVal;}

    //! If the direction mode is Azimuth, then the baseDirection and clockwise flag will be applied.
    DirectionMode GetDirectionMode() {return m_mode;}

    //! Directions can be interpretted clockwise or counter-clockwise with respect to the base direction.
    //! Used only if the direction mode is Azimuth.
    void SetClockwise(bool isCw) {m_isClockwise = isCw;}

    //! Directions can be interpretted clockwise or counter-clockwise with respect to the base direction.
    //! Used only if the direction mode is Azimuth.
    bool GetClockwise() {return m_isClockwise;}

    //! Directions can be interpretted with respect to the base direction.
    //! Used only if the direction mode is Azimuth.
    void SetBaseDirection(double newVal) {m_baseDirection = newVal;}

    //! Directions can be interpretted with respect to the base direction.
    //! Used only if the direction mode is Azimuth.
    double GetBaseDirection() {return m_baseDirection;}

    //! Parse a string into a direction value in degrees.
    //! @param[out] out resulting direction in degrees if successfully parsed.
    //! @param[in]  in input string.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(double& out, Utf8CP in);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DistanceParser : RefCountedBase
{
private: 
    Utf8String m_masterUnitLabel;
    Utf8String m_subUnitLabel;
    double m_masterUnitScale;
    double m_subUnitScale;
    double m_scale;

    DistanceParser();
    DistanceParser(DistanceParserCR other);
    void Init();

public:  
    void InitModelSettings(GeometricModelCR);

    //! Parse a string into a distance value.
    //! @param[out] out resulting distance if successfully parsed.
    //! @param[out] numCharsParsed number of characters consumed while parsing.
    //! @param[in] in input string.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(double& out, size_t& numCharsParsed, Utf8CP in);

    //! Construct a parser with default settings.
    static DGNPLATFORM_EXPORT  DistanceParserPtr Create();

    //! Construct a parser with settings from a model.
    //! @param[in] model Initialize the parser from the settings in this model.
    static DGNPLATFORM_EXPORT  DistanceParserPtr Create(GeometricModelCR model);

    //! Construct a parser with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the scale from the viewport's ACS.
    //! @param[in] vp Initialize the parser from the settings in this viewport.
    static DGNPLATFORM_EXPORT  DistanceParserPtr Create(DgnViewportR vp);

    //! Construct a parser with settings from a DgnModel and an Auxiliary Coordinate System.
    //! The ACS provides the scale for the parser.
    //! @param[in] model Initialize the parser from the settings in this model.
    //! @param[in] acs   Initialize the scale from the settings in this Auxiliary Coordinate System.
    static DGNPLATFORM_EXPORT DistanceParserPtr Create(GeometricModelCR model, IAuxCoordSysCR acs);

    //! Construct a parser which is a duplicate of an existing parser.
    DGNPLATFORM_EXPORT DistanceParserPtr Clone() const;

    //! Set Master and Sub Units manually. Normally, use the SetUnits()* method
    void SetMasterUnitLabel(Utf8CP label) {m_masterUnitLabel = label;}
    Utf8CP GetMasterUnitsLabel() {return m_masterUnitLabel.c_str();}
    void SetSubUnitLabel(Utf8CP label) {m_subUnitLabel = label;}
    Utf8CP GetSubUnitsLabel() {return m_subUnitLabel.c_str();}
    void SetMasterUnitScale(double scale) {m_masterUnitScale = scale;}
    double GetMasterUnitsScale() {return m_masterUnitScale;}
    void SetSubUnitScale(double scale) {m_subUnitScale = scale;}
    double GetSubUnitsScale() {return m_subUnitScale;}

    //! The value will be scaled by this factor after it is parsed.
    void SetScale(double scale) {m_scale = scale;}

    //! The value will be scaled by this factor after it is parsed.
    double GetScale() {return m_scale;}

    //! Parse a string into a distance value.
    //! @param[out] out         resulting distance if successfully parsed.
    //! @param[in]  in          input string.
    //! @return     SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(double& out, Utf8CP in);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PointParser : RefCountedBase
{
private: 
    bool m_is3d;
    DistanceParserPtr m_distanceParser;

    PointParser();
    PointParser(PointParserCR other);
    void Init();
    void InitModelSettings(GeometricModelCR);
    BentleyStatus StringToPoint(DPoint3dR out, Point3dP relativeFlags, Utf8CP in);

public: 
    //! Parse a string into a point value.
    //! @param[out] out resulting distance if successfully parsed.
    //! @param[out] relativeFlags true if a coordinate should be considered relative.
    //! @param[in]  in string to parse.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(DPoint3dR out, Point3dR relativeFlags, Utf8CP in);

    //! Construct a parser with default settings.
    static DGNPLATFORM_EXPORT PointParserPtr Create();

    //! Construct a parser with settings from a model.
    //! @param[in] model Initialize the parser from the settings in this model.
    static DGNPLATFORM_EXPORT PointParserPtr Create(GeometricModelCR model);

    //! Construct a parser with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the ACS from the viewport.
    //! @param[in] vp  Initialize the parser from the settings in this viewport.
    static DGNPLATFORM_EXPORT PointParserPtr Create(DgnViewportR vp);

    //! Construct a parser with settings from a DgnModel and an Auxiliary Coordinate System.
    //! The ACS provides the scale for the parser.
    //! @param[in] model Initialize the parser from the settings in this model.
    //! @param[in] acs   Initialize the scale from the settings in this Auxiliary Coordinate System.
    static DGNPLATFORM_EXPORT PointParserPtr Create(GeometricModelCR model, IAuxCoordSysCR acs);

    //! Construct a parser which is a duplicate of an existing parser.
    DGNPLATFORM_EXPORT PointParserPtr Clone() const;

    //! Parse a string into a point value.
    //! @param[out] out resulting distance if successfully parsed.
    //! @param[in]  in input string.
    //! @return     SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(DPoint3dR out, Utf8CP in);

    //! Get the distance parser used by this parser for each coordinate of the point.
    //! Changes made to this object will affect the future behavior of the PointParser.
    DistanceParserR GetDistanceParser() {return *m_distanceParser;}

    //! If this flag is true, the parse requires three coordinates in the string.
    void SetIs3d(bool is3d) {m_is3d = is3d;}

    //! If this flag is true, the parse requires three coordinates in the string.
    bool GetIs3d() const {return m_is3d;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AreaOrVolumeParser : RefCountedBase
{
private: 
    double m_masterUnitScale;
    double m_scale;
    uint8_t m_dimension;

protected: 
    AreaOrVolumeParser(uint8_t dimension) {m_dimension = dimension; Init();}
    AreaOrVolumeParser(AreaOrVolumeParserCR source) {m_masterUnitScale = source.m_masterUnitScale; m_scale = source.m_scale; m_dimension = source.m_dimension;}
    void Init() {m_masterUnitScale = 1.0; m_scale = 1.0;}

public:  
    DGNPLATFORM_EXPORT void InitModelSettings(GeometricModelCR);

    //! Parse a string into a distance value.
    //! @param[out] out resulting distance if successfully parsed.
    //! @param[out] numCharsParsed number of characters consumed while parsing.
    //! @param[in] in input string.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus ToValue(double& out, size_t& numCharsParsed, Utf8CP in);

    //! Set Master and Sub Units manually. Normally, use the SetUnits()* method
    void SetMasterUnitScale(double scale) {m_masterUnitScale = scale;}
    double GetMasterUnitsScale() {return m_masterUnitScale;}

    //! The value will be scaled by the square of this factor after it is parsed.
    void SetScale(double scale) {m_scale = scale;}

    //! The value will be scaled by the square of this factor after it is parsed.
    double GetScale() {return m_scale;}

    //! Parse a string into a distance value.
    //! @param[out] out resulting distance if successfully parsed.
    //! @param[in] input string to parse.
    //! @return SUCCESS if parsed successfully. ERROR otherwise.
    BentleyStatus ToValue(double& out, Utf8CP input) {size_t numChars; return ToValue(out, numChars, input);}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AreaParser : AreaOrVolumeParser
{
private:
    AreaParser() : AreaOrVolumeParser(2) {}
    AreaParser(AreaParserCR other): AreaOrVolumeParser(other){}

public:
    //! Construct a parser with default settings.
    static AreaParserPtr Create() {return new AreaParser();}

    //! Construct a parser with settings from a model.
    //! @param[in] model Initialize the parser from the settings in this model.
    static DGNPLATFORM_EXPORT AreaParserPtr Create(GeometricModelCR model);

    //! Construct a parser with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the scale from the viewport's ACS.
    //! @param[in] vp Initialize the parser from the settings in this viewport.
    static DGNPLATFORM_EXPORT AreaParserPtr Create(DgnViewportR vp);

    //! Construct a parser which is a duplicate of an existing parser.
    AreaParserPtr Clone() const {return new AreaParser(*this);}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct VolumeParser : AreaOrVolumeParser
{
private:
    VolumeParser() : AreaOrVolumeParser(3) {}
    VolumeParser(VolumeParserCR other) : AreaOrVolumeParser (other) {}

public:
    //! Construct a parser with default settings.
    static VolumeParserPtr Create() {return new VolumeParser();}

    //! Construct a parser with settings from a model.
    //! @param[in] model Initialize the parser from the settings in this model.
    static VolumeParserPtr Create(GeometricModelCR model) {VolumeParserPtr parser = Create(); parser->InitModelSettings(model); return parser;}

    //! Construct a parser with settings from a viewport.  Gets the settings from the viewport's
    //! target model and the scale from the viewport's ACS.
    //! @param[in] vp Initialize the parser from the settings in this viewport.
    static DGNPLATFORM_EXPORT VolumeParserPtr Create(DgnViewportR vp);

    //! Construct a parser which is a duplicate of an existing parser.
    VolumeParserPtr Clone() const {return new VolumeParser(*this);}
};

END_BENTLEY_DGN_NAMESPACE
