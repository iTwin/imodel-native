/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ModelInfo.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    "DgnElements.h"
#include    "UnitDefinition.h"
#include    <Bentley/ValueFormat.h>
#include    "../DgnCore/DgnCore.h"
//__PUBLISH_SECTION_END__
#include    <vector>

#include <Bentley/bvector.h>
#include <Bentley/ValueFormat.h>

struct ModelInfoAccess;

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
struct ModelPropertyFlags
    {
    UInt32 m_locked:1;                   //!< do not allow changes to data
    UInt32 m_useBackgroundColor:1;       //!< 0=Don't use model color override
    UInt32 ToUInt32() const {return *(UInt32 const*) this;}
    };

struct ModelSettingFlags
    {
    UInt32 m_unitLock:1;             //!< unit lock in effect
    UInt32 ToUInt32() const {return *(UInt32 const*) this;}
    };

struct FormatterFlags
    {
    UInt32 m_linearUnitMode:2;
    UInt32 m_linearPrecType:4;
    UInt32 m_linearPrecision:8;
    UInt32 m_angularMode:3;
    UInt32 m_angularPrecision:8;
    UInt32 m_directionMode:2;
    UInt32 m_directionClockwise:1;
    UInt32 ToUInt32() const {return *(UInt32 const*) this;}
    };

//=======================================================================================
//! The properties and settings for a DgnModel.
//! @bsiclass
//=======================================================================================
struct ModelInfo
{
//__PUBLISH_SECTION_END__
    friend struct DgnModel;
    friend struct DgnFile;
    friend struct ModelInfoAccess;
//__PUBLISH_SECTION_START__

private:
    ModelPropertyFlags      m_flags;                        //!< Flags that are properties.
    ModelSettingFlags       m_settingFlags;                 //!< Flags saved on "save settings"
    FormatterFlags          m_formatterFlags;               //!< Flags saved on "save settings"
    UnitDefinition          m_masterUnit;                   //!< Master Unit information
    UnitDefinition          m_subUnit;                      //!< Sub Unit information
    double                  m_roundoffUnit;                 //!< unit lock roundoff val in uors
    double                  m_roundoffRatio;                //!< Unit roundoff ratio y to x (if 0 use Grid Ratio)
    double                  m_formatterBaseDir;             //!< Base Direction used for Direction To/From String

    void Init();

    //! Make this ModelInfo a copy of another one.
    //! @param[in] other The source from which this ModelInfo will be populated.
    ModelInfo(ModelInfoCR other);

public:
    ModelInfo() {Init();}

//__PUBLISH_SECTION_END__
    BentleyStatus FromSettingsJson (Json::Value const& inValue);
    void ToSettingsJson(Json::Value& outValue) const;
    BentleyStatus FromPropertiesJson (Json::Value const& inValue);
    void ToPropertiesJson(Json::Value& outValue) const;

//__PUBLISH_SECTION_START__
    void SetIsUnitLocked (bool isLocked){m_settingFlags.m_unitLock = isLocked;}
    DGNPLATFORM_EXPORT void SetRoundoffUnit (double const& roundoffUnit, double const& roundoffRatio);
    DGNPLATFORM_EXPORT BentleyStatus SetWorkingUnits (UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit); //!< Set working-units and sub-units. Units must be valid and comparable.
    DGNPLATFORM_EXPORT void SetLinearUnitMode (DgnUnitFormat value);
    DGNPLATFORM_EXPORT void SetLinearPrecision (PrecisionFormat value);
    DGNPLATFORM_EXPORT void SetAngularMode (AngleMode value);
    DGNPLATFORM_EXPORT void SetAngularPrecision (AnglePrecision value);
    DGNPLATFORM_EXPORT void SetDirectionMode (DirectionMode value);
    DGNPLATFORM_EXPORT void SetDirectionClockwise (bool value);
    DGNPLATFORM_EXPORT void SetDirectionBaseDir (double value);
    DGNPLATFORM_EXPORT bool GetIsUnitLocked () const;
    DGNPLATFORM_EXPORT bool GetIsLocked () const;
    DGNPLATFORM_EXPORT DgnUnitFormat GetLinearUnitMode () const;
    DGNPLATFORM_EXPORT PrecisionFormat GetLinearPrecision () const;
    DGNPLATFORM_EXPORT AngleMode GetAngularMode () const;
    DGNPLATFORM_EXPORT AnglePrecision GetAngularPrecision () const;
    DGNPLATFORM_EXPORT DirectionMode GetDirectionMode () const;
    DGNPLATFORM_EXPORT bool GetDirectionClockwise () const;
    DGNPLATFORM_EXPORT double GetDirectionBaseDir () const;

private:
    bool operator==(ModelInfoCR) const;                       //!< Determine whether this modelInfo is equivalent to another
    bool operator!=(ModelInfoCR other) const {return !(*this == other);}

public:
    DGNPLATFORM_EXPORT bool IsEqual (ModelInfoCR) const;  //!< Compare modelinfo to see if they are the same.

    DGNPLATFORM_EXPORT double GetRoundoffUnit () const; //!< Get the roundoff unit for this model.
    DGNPLATFORM_EXPORT double GetRoundoffRatio () const; //!< Get the roundoff ratio for this model.

    ModelPropertyFlags GetPropertyFlags() const {return m_flags;}
    ModelSettingFlags GetSettingsFlags() const  {return m_settingFlags;}
    FormatterFlags GetFormatterFlags() const    {return m_formatterFlags;}
    UnitDefinitionCR GetMasterUnit () const {return m_masterUnit;}
    UnitDefinitionCR GetSubUnit () const {return m_subUnit;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
/** @endcond */
