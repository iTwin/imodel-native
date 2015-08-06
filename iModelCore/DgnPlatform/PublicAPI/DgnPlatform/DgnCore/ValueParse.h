/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ValueParse.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include    "ValueFormat.h"

DGNPLATFORM_TYPEDEFS (DoubleParser)
DGNPLATFORM_TYPEDEFS (AngleParser)
DGNPLATFORM_TYPEDEFS (DirectionParser)
DGNPLATFORM_TYPEDEFS (DistanceParser)
DGNPLATFORM_TYPEDEFS (PointParser)
DGNPLATFORM_TYPEDEFS (AreaOrVolumeParser)
DGNPLATFORM_TYPEDEFS (AreaParser)
DGNPLATFORM_TYPEDEFS (VolumeParser);

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
private:  /* ctor */          DoubleParser ();
private:  /* ctor */          DoubleParser (DoubleParserCR other);

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  DoubleParserPtr     Create ();

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  DoubleParserPtr    Clone () const;

//! Parse a string into a numeric value. 
//! @param[out] out         resulting value if successfully parsed.
//! @param[in]  in          input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, WCharCP in) const;

}; // DoubleParser 

//=======================================================================================
//! Used to parse angles. 
//! Default parsing mode is in radians, results are always in degrees.
//! This assumes input is ready to be parsed as an angle.
//! If it is just a number the AngleMode determines how the number will be interpreted.
// @bsiclass
//=======================================================================================
struct AngleParser : RefCountedBase
{
private: AngleMode              m_angleMode;

private:  /* ctor */            AngleParser ();
private:  /* ctor */            AngleParser (AngleParserCR other);
private:  void                  Init ();

public:   void                  InitModelSettings (DgnModelCR);
public: DGNPLATFORM_EXPORT  BentleyStatus   SetAngleModeFromLegacy (AngleFormatVals value);
public: DGNPLATFORM_EXPORT  uint16_t        GetLegacyFormat () const;

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  AngleParserPtr   Create ();

//! Construct a parser with the settings as stored in the model.
//! @param[in] model Initialize the parser from the settings in this model.
public: static DGNPLATFORM_EXPORT AngleParserPtr    Create (DgnModelCR model);

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  AngleParserPtr    Clone () const;

//! Sets the angle mode for parsing the input string.
//! AngleMode::Degrees, AngleMode::DegMin, and AngleMode::DegMinSec, are all treated the same.
//! @param[in]  mode The mode to set.
public: DGNPLATFORM_EXPORT void SetAngleMode (AngleMode mode);

//! Gets the angle mode for parsing the input string.
//! AngleMode::Degrees, AngleMode::DegMin, and AngleMode::DegMinSec, are all treated the same.
public: DGNPLATFORM_EXPORT AngleMode GetAngleMode ();

//! Parse a string into an angle value in degrees.
//! @param[out] out         resulting angle in degrees if successfully parsed.
//! @param[in]  in          input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, WCharCP in) const;

}; // AngleParser

//=======================================================================================
//! Used to parse directions. 
//! Default parsing mode is in radians, results are always in degrees.
//! This assumes input is ready to be parsed as an angle.
//! If it is just a number the AngleMode determines how the number will be interpreted.
// @bsiclass
//=======================================================================================
struct DirectionParser : RefCountedBase
{
private: double                 m_trueNorth;
private: DirectionMode          m_mode;
private: bool                   m_isClockwise;
private: double                 m_baseDirection;
private: AngleParserPtr         m_angleParser;

private: /* ctor */            DirectionParser ();
private: /* ctor */            DirectionParser (DirectionParserCR other);
private: void                  Init ();
private: void                  InitModelSettings (DgnModelCR);

private: int StringToDirection (double& dir, WCharCP inputString) const;

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  DirectionParserPtr   Create ();

//! Construct a parser with settings from a model.
//! @param[in] model Initialize the parser from the settings in this model.
public: static DGNPLATFORM_EXPORT  DirectionParserPtr   Create (DgnModelCR model);

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  DirectionParserPtr    Clone () const;

//! Get the angle parser used by this parser for the numeric portion of the direction.
//! Changes made to this object will affect the future behavior of the DirectionParser.
public: DGNPLATFORM_EXPORT AngleParserR     GetAngleParser ();

//! Set an offset in degrees that will be added to the parsed value before it is returned.
public: DGNPLATFORM_EXPORT void             SetTrueNorthValue (double trueNorth);

//! Get the offset in degrees that will be added to the parsed value before it is returned.
public: DGNPLATFORM_EXPORT double           GetTrueNorthValue ();

//! If the direction mode is Azimuth, then the baseDirection and clockwise flag will be applied.
public: DGNPLATFORM_EXPORT void             SetDirectionMode  (DirectionMode newVal);

//! If the direction mode is Azimuth, then the baseDirection and clockwise flag will be applied.
public: DGNPLATFORM_EXPORT DirectionMode    GetDirectionMode  ();

//! Directions can be interpretted clockwise or counter-clockwise with respect to the base direction.
//! Used only if the direction mode is Azimuth.
public: DGNPLATFORM_EXPORT void             SetClockwise (bool isCw);

//! Directions can be interpretted clockwise or counter-clockwise with respect to the base direction.
//! Used only if the direction mode is Azimuth.
public: DGNPLATFORM_EXPORT bool             GetClockwise ();

//! Directions can be interpretted with respect to the base direction.
//! Used only if the direction mode is Azimuth.
public: DGNPLATFORM_EXPORT void             SetBaseDirection  (double newVal);

//! Directions can be interpretted with respect to the base direction.
//! Used only if the direction mode is Azimuth.
public: DGNPLATFORM_EXPORT double           GetBaseDirection  ();

//! Parse a string into a direction value in degrees.
//! @param[out] out         resulting direction in degrees if successfully parsed.
//! @param[in]  in          input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, WCharCP in);

}; // DirectionParser 

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DistanceParser : RefCountedBase
{
#if defined (DGNPLATFORM_WIP_UNITS)
// This parser should store master/sub/storage unit definitions, not scales.  Need to change
// API as well.
#endif
private: WString                m_masterUnitLabel;
private: WString                m_subUnitLabel;
private: double                 m_masterUnitScale;
private: double                 m_subUnitScale;
private: double                 m_scale;

private: /* ctor */             DistanceParser ();
private: /* ctor */             DistanceParser (DistanceParserCR other);
private: void                   Init ();

public:  void                   InitModelSettings (DgnModelCR);

//! Parse a string into a distance value in uors.
//! @param[out] out resulting distance in uors if successfully parsed.
//! @param[out] numCharsParsed number of characters consumed while parsing.
//! @param[in] in input string.
//! @return SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, size_t& numCharsParsed, WCharCP in);

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  DistanceParserPtr Create ();

//! Construct a parser with settings from a model.
//! @param[in] model Initialize the parser from the settings in this model.
public: static DGNPLATFORM_EXPORT  DistanceParserPtr Create (DgnModelCR model);

//! Construct a parser with settings from a viewport.  Gets the settings from the viewport's 
//! target model and the scale from the viewport's ACS.
//! @param[in] vp Initialize the parser from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  DistanceParserPtr Create (DgnViewportR vp);

//! Construct a parser with settings from a DgnModel and an Auxiliary Coordinate System.  
//! The ACS provides the scale for the parser.
//! @param[in] model Initialize the parser from the settings in this model.
//! @param[in] acs   Initialize the scale from the settings in this Auxiliary Coordinate System.
public: static DGNPLATFORM_EXPORT  DistanceParserPtr    Create (DgnModelCR model, IAuxCoordSysCR acs);

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  DistanceParserPtr    Clone () const;

//! Set Master and Sub Units manually. Normally, use the SetUnits()* method 
public: DGNPLATFORM_EXPORT void     SetMasterUnitLabel     (WCharCP label);
public: DGNPLATFORM_EXPORT WCharCP  GetMasterUnitsLabel     ();
public: DGNPLATFORM_EXPORT void     SetSubUnitLabel        (WCharCP label);
public: DGNPLATFORM_EXPORT WCharCP  GetSubUnitsLabel        ();
public: DGNPLATFORM_EXPORT void     SetMasterUnitScale     (double scale);
public: DGNPLATFORM_EXPORT double   GetMasterUnitsScale     ();
public: DGNPLATFORM_EXPORT void     SetSubUnitScale        (double scale);
public: DGNPLATFORM_EXPORT double   GetSubUnitsScale        ();

//! The value will be scaled by this factor after it is parsed.
public: DGNPLATFORM_EXPORT void     SetScale               (double scale);

//! The value will be scaled by this factor after it is parsed.
public: DGNPLATFORM_EXPORT double   GetScale               ();

//! Parse a string into a distance value in uors.
//! @param[out] out         resulting distance in uors if successfully parsed.
//! @param[in]  in          input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, WCharCP in);

}; // DistanceParser 

//=======================================================================================
// @bsiclass
//=======================================================================================
struct PointParser : RefCountedBase
{
private: bool              m_is3d;
private: DistanceParserPtr m_distanceParser;

private: /* ctor */             PointParser ();
private: /* ctor */             PointParser (PointParserCR other);
private: void                   Init ();
private: void                   InitModelSettings (DgnModelCR);

private:  BentleyStatus         StringToPoint (DPoint3dR out, Point3dP relativeFlags, WCharCP in);

//! Parse a string into a point value in uors.
//! @param[out] out             resulting distance in uors if successfully parsed.
//! @param[out] relativeFlags   true if a coordinate should be considered relative.
//! @param[in]  in              input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (DPoint3dR out, Point3dR relativeFlags, WCharCP in);

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  PointParserPtr     Create ();

//! Construct a parser with settings from a model.
//! @param[in] model Initialize the parser from the settings in this model.
public: static DGNPLATFORM_EXPORT  PointParserPtr     Create (DgnModelCR model);

//! Construct a parser with settings from a viewport.  Gets the settings from the viewport's 
//! target model and the ACS from the viewport.
//! @param[in] vp                       Initialize the parser from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  PointParserPtr     Create (DgnViewportR vp);

//! Construct a parser with settings from a DgnModel and an Auxiliary Coordinate System.  
//! The ACS provides the scale for the parser.
//! @param[in] model Initialize the parser from the settings in this model.
//! @param[in] acs   Initialize the scale from the settings in this Auxiliary Coordinate System.
public: static DGNPLATFORM_EXPORT  PointParserPtr     Create (DgnModelCR model, IAuxCoordSysCR acs);

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  PointParserPtr    Clone () const;

//! Parse a string into a point value in uors.
//! @param[out] out         resulting distance in uors if successfully parsed.
//! @param[in]  in          input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (DPoint3dR out, WCharCP in);

//! Get the distance parser used by this parser for each coordinate of the point.
//! Changes made to this object will affect the future behavior of the PointParser.
public: DGNPLATFORM_EXPORT DistanceParserR    GetDistanceParser ();

//! If this flag is true, the parse requires three coordinates in the string.
public: DGNPLATFORM_EXPORT void SetIs3d (bool is3d);

//! If this flag is true, the parse requires three coordinates in the string.
public: DGNPLATFORM_EXPORT bool GetIs3d () const;

}; // PointParser 

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AreaOrVolumeParser : RefCountedBase
{
#if defined (DGNPLATFORM_WIP_UNITS)
// This parser should store master unit definitions, not scales.  Need to change
// API as well.
#endif
private: double                 m_masterUnitScale;
private: double                 m_scale;
private: uint8_t                m_dimension;

protected: /* ctor */           AreaOrVolumeParser (uint8_t dimension);
protected: /* ctor */           AreaOrVolumeParser (AreaOrVolumeParserCR other);

private: void                   Init ();

public:  DGNPLATFORM_EXPORT void    InitModelSettings (DgnModelCR);

//! Parse a string into a distance value in uors.
//! @param[out] out resulting distance in uors if successfully parsed.
//! @param[out] numCharsParsed number of characters consumed while parsing.
//! @param[in] in input string.
//! @return SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, size_t& numCharsParsed, WCharCP in);

//! Set Master and Sub Units manually. Normally, use the SetUnits()* method 
public: DGNPLATFORM_EXPORT void     SetMasterUnitScale     (double scale);
public: DGNPLATFORM_EXPORT double   GetMasterUnitsScale     ();

//! The value will be scaled by the square of this factor after it is parsed.
public: DGNPLATFORM_EXPORT void     SetScale               (double scale);

//! The value will be scaled by the square of this factor after it is parsed.
public: DGNPLATFORM_EXPORT double   GetScale               ();

//! Parse a string into a distance value in uors.
//! @param[out] out         resulting distance in uors if successfully parsed.
//! @param[in]  in          input string.
//! @return     SUCCESS if parsed successfully. ERROR otherwise. 
public: DGNPLATFORM_EXPORT BentleyStatus ToValue (double& out, WCharCP in);

}; // AreaOrVolumeParser 

//=======================================================================================
// @bsiclass
//=======================================================================================
struct AreaParser : AreaOrVolumeParser
{
private: /* ctor */             AreaParser ();
private: /* ctor */             AreaParser (AreaParserCR other);

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  AreaParserPtr    Create ();

//! Construct a parser with settings from a model.
//! @param[in] model Initialize the parser from the settings in this model.
public: static DGNPLATFORM_EXPORT  AreaParserPtr    Create (DgnModelCR model);

//! Construct a parser with settings from a viewport.  Gets the settings from the viewport's 
//! target model and the scale from the viewport's ACS.
//! @param[in] vp                       Initialize the parser from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  AreaParserPtr    Create (DgnViewportR vp);

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  AreaParserPtr    Clone () const;

};  //AreaParser

//=======================================================================================
// @bsiclass
//=======================================================================================
struct VolumeParser : AreaOrVolumeParser
{
private: /* ctor */             VolumeParser ();
private: /* ctor */             VolumeParser (VolumeParserCR other);

//! Construct a parser with default settings.
public: static DGNPLATFORM_EXPORT  VolumeParserPtr    Create ();

//! Construct a parser with settings from a model.
//! @param[in] model Initialize the parser from the settings in this model.
public: static DGNPLATFORM_EXPORT  VolumeParserPtr    Create (DgnModelCR model);

//! Construct a parser with settings from a viewport.  Gets the settings from the viewport's 
//! target model and the scale from the viewport's ACS.
//! @param[in] vp                       Initialize the parser from the settings in this viewport.
public: static DGNPLATFORM_EXPORT  VolumeParserPtr    Create (DgnViewportR vp);

//! Construct a parser which is a duplicate of an existing parser.
public: DGNPLATFORM_EXPORT  VolumeParserPtr    Clone () const;

};  //VolumeParser

END_BENTLEY_DGN_NAMESPACE
