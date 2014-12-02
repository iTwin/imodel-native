/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/modelinfo.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void fromLinearPrecisionEnum (FormatterFlags& formatterFlags, PrecisionFormat value)
    {
    formatterFlags.m_linearPrecType  = static_cast<UInt32>(DoubleFormatter::GetTypeFromPrecision (value));
    formatterFlags.m_linearPrecision = DoubleFormatter::GetByteFromPrecision (value);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  ModelInfo::SetWorkingUnits (UnitDefinitionCR newMasterUnit, UnitDefinitionCR newSubUnit)
    {
    if (!newMasterUnit.IsValid() || !newSubUnit.IsValid())
        return ERROR;

    if (!newMasterUnit.AreComparable(newSubUnit))
        return ERROR;

    // subunits must be smaller than master.passed in, validate that they are smaller than master
    int subunitRelationship  = newMasterUnit.CompareByScale (newSubUnit);
    if (0 < subunitRelationship)
        return ERROR;

    m_masterUnit = newMasterUnit;
    m_subUnit    = newSubUnit;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelInfo::SetRoundoffUnit (double const& roundoffUnit, double const& roundoffRatio)
    {
    m_roundoffUnit  = roundoffUnit;
    m_roundoffRatio = roundoffRatio;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelInfo::SetLinearUnitMode (DgnUnitFormat value) { m_formatterFlags.m_linearUnitMode = (UInt32)value; }
void ModelInfo::SetLinearPrecision (PrecisionFormat value) { fromLinearPrecisionEnum (m_formatterFlags, value); }
void ModelInfo::SetAngularMode (AngleMode value) { m_formatterFlags.m_angularMode = (UInt32)value; }
void ModelInfo::SetAngularPrecision (AnglePrecision value) { m_formatterFlags.m_angularPrecision = (UInt32)value; }
void ModelInfo::SetDirectionMode (DirectionMode value) {m_formatterFlags.m_directionMode = (UInt32)value; }
void ModelInfo::SetDirectionClockwise (bool value) { m_formatterFlags.m_directionClockwise = value; }
void ModelInfo::SetDirectionBaseDir (double value) { m_formatterBaseDir = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelInfo::Init()
    {
    memset (&m_flags, 0, sizeof(m_flags));
    memset (&m_settingFlags, 0, sizeof(m_settingFlags));
    memset (&m_formatterFlags, 0, sizeof(m_formatterFlags));

    m_roundoffRatio= 0;
    m_formatterBaseDir = 0;
    m_roundoffUnit = 0;

    m_subUnit.Init (UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, L"m");
    m_masterUnit = m_subUnit;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
// settings
bool                ModelInfo::GetIsUnitLocked () const                     {return m_settingFlags.m_unitLock;}
DgnUnitFormat       ModelInfo::GetLinearUnitMode () const                   {return (DgnUnitFormat) m_formatterFlags.m_linearUnitMode; }
PrecisionFormat     ModelInfo::GetLinearPrecision () const                  {return DoubleFormatter::ToPrecisionEnum ((PrecisionType) m_formatterFlags.m_linearPrecType, m_formatterFlags.m_linearPrecision); }
AngleMode           ModelInfo::GetAngularMode () const                      {return (AngleMode) m_formatterFlags.m_angularMode; }
AnglePrecision      ModelInfo::GetAngularPrecision () const                 {return (AnglePrecision) m_formatterFlags.m_angularPrecision; }
DirectionMode       ModelInfo::GetDirectionMode () const                    {return (DirectionMode) m_formatterFlags.m_directionMode; }
bool                ModelInfo::GetDirectionClockwise () const               {return m_formatterFlags.m_directionClockwise; }
double              ModelInfo::GetDirectionBaseDir () const                 {return m_formatterBaseDir; }

// properties
bool                ModelInfo::GetIsLocked () const                         {return m_flags.m_locked;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double              ModelInfo::GetRoundoffUnit () const     {return m_roundoffUnit;}
double              ModelInfo::GetRoundoffRatio () const    {return m_roundoffRatio;}
