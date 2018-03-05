/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECUnit.h $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjects.h>
#include <Units/Units.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct UnitSystem : Units::UnitSystem, NonCopyableClass
{
DEFINE_T_SUPER(Units::UnitSystem)
friend struct ECSchema;
friend struct SchemaXmlWriter;
friend struct SchemaXmlReaderImpl;
friend struct SchemaJsonWriter;

private:
    ECSchemaCR m_schema;

    bool m_hasExplicitDisplayLabel;
    Utf8String m_displayLabel;
    Utf8String m_description;
    mutable Utf8String m_fullName; // Cached when GetFullName is called the first time.
    mutable UnitSystemId m_unitSystemId;

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {m_displayLabel = value.c_str(); m_hasExplicitDisplayLabel = true; return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}

    SchemaReadStatus ReadXml(BeXmlNodeR unitSystemNode, ECSchemaReadContextR context);
    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    SchemaWriteStatus WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    UnitSystem(ECSchemaCR schema, Utf8CP name) : Units::UnitSystem(name), m_schema(schema), m_hasExplicitDisplayLabel(false) 
        {
        m_unitsContext = (Units::IUnitsContextCP) &schema;
        }

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this UnitSystem is defined in.

    //! The fully qualified name of this UnitSystem in the format {SchemaName}:{UnitSystemName}.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
    //! Gets a qualified name of the UnitSystem, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    //! Gets the display label of this UnitSystem.  If no display label has been set explicitly, it will return the name of the UnitSystem.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    bool GetIsDisplayLabelDefined() const {return m_hasExplicitDisplayLabel;} //!< Whether the display label is explicitly defined or not.
    Utf8StringCR GetInvariantDisplayLabel() const {return (m_hasExplicitDisplayLabel) ? m_displayLabel : GetName();} //!< Gets the invariant display label for this UnitSystem.

    bool GetIsDescriptionDefined() const {return !m_description.empty();} //!< Returns true if description is not empty
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this UnitSystem.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this UnitSystem. Returns the localized description if one exists.

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    UnitSystemId GetId() const {BeAssert(HasId()); return m_unitSystemId;}
    //! Intended to be called by ECDb or a similar system.
    void SetId(UnitSystemId id) {BeAssert(!m_unitSystemId.IsValid()); m_unitSystemId = id;}
    bool HasId() const {return m_unitSystemId.IsValid();}

    //! Write the UnitSystem as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct Phenomenon : Units::Phenomenon, NonCopyableClass
{
DEFINE_T_SUPER(Units::Phenomenon)
friend struct ECSchema;
friend struct SchemaXmlWriter;
friend struct SchemaXmlReaderImpl;
friend struct SchemaJsonWriter;

private:
    bool m_isDisplayLabelExplicitlyDefined; // displayLabel is in Units::Phenomenon
    Utf8String m_description;
    ECSchemaCR m_schema;
    PhenomenonId m_phenomenonId;
    mutable Utf8String m_fullName;

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {Units::Phenomenon::SetLabel(value.c_str()); m_isDisplayLabelExplicitlyDefined = true; return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}

    SchemaReadStatus ReadXml(BeXmlNodeR phenomenonNode, ECSchemaReadContextR context);

    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    SchemaWriteStatus WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    Phenomenon(ECSchemaCR schema, Utf8CP name) : Phenomenon(schema, name, nullptr) 
        {
        m_unitsContext = (Units::IUnitsContextCP) &schema;
        }
    Phenomenon(ECSchemaCR schema, Utf8CP name, Utf8CP definition) : Units::Phenomenon(name, definition), m_schema(schema), m_isDisplayLabelExplicitlyDefined(false) 
        {
        m_unitsContext = (Units::IUnitsContextCP) &schema;
        }

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this Phenomenon is defined in.

    //! The fully qualified name of this UnitFormat in the format {SchemaName}:{PhenomenonName}.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
    //! Gets a qualified name of the Phenomenon, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;
    //! Gets the display label of this Phenomenon.  If no display label has been set explicitly, it will return the name of the Phenomenon.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    bool GetIsDisplayLabelDefined() const {return m_isDisplayLabelExplicitlyDefined;} //!< Whether the display label is explicitly defined or not.
    ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const; //!< Gets the invariant display label for this Phenomenon.

    bool GetIsDescriptionDefined() const {return !m_description.empty();} //!< Returns true if description is not empty
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this Phenomenon.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this Phenomenon. Returns the localized description if one exists.

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    PhenomenonId GetId() const {BeAssert(HasId()); return m_phenomenonId;}
    //! Intended to be called by ECDb or a similar system.
    void SetId(PhenomenonId id) {BeAssert(!m_phenomenonId.IsValid()); m_phenomenonId = id;}
    bool HasId() const {return m_phenomenonId.IsValid();}

    //! Write the Phenomenon as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct ECUnit : Units::Unit, NonCopyableClass 
{
DEFINE_T_SUPER(Units::Unit)
friend struct ECSchema;
friend struct SchemaXmlWriter;
friend struct SchemaXmlReaderImpl;
friend struct SchemaJsonWriter;

private:
    ECSchemaCR m_schema;
    UnitId m_unitId;
    bool m_isDisplayLabelExplicitlyDefined;
    Utf8String m_description;
    mutable Utf8String m_fullName;

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {Units::Unit::SetLabel(value.c_str()); m_isDisplayLabelExplicitlyDefined = true; return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}

    SchemaReadStatus ReadXml(BeXmlNodeR unitNode, ECSchemaReadContextR context);
    SchemaReadStatus ReadStandardUnitXml(BeXmlNodeR, ECSchemaReadContextR context);
    SchemaReadStatus ReadInvertedUnitXml(BeXmlNodeR, ECSchemaReadContextR context);
    SchemaReadStatus ReadConstantXml(BeXmlNodeR, ECSchemaReadContextR context);

    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;
    SchemaWriteStatus WriteInvertedUnitXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;
    SchemaWriteStatus WriteConstantXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    SchemaWriteStatus WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;
    SchemaWriteStatus WriteInvertedUnitJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;
    SchemaWriteStatus WriteConstantJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    ECUnit(ECSchemaCR schema, Utf8CP name) : Units::Unit(name), m_schema(schema) 
        {
        m_unitsContext = (Units::IUnitsContextCP) &schema;
        }
    ECUnit(ECSchemaCR schema, Units::UnitCR parentUnit, Units::UnitSystemCR system, Utf8CP unitName) : Units::Unit(parentUnit, system, unitName), m_schema(schema) 
        {
        m_unitsContext = (Units::IUnitsContextCP) &schema;
        }
    ECUnit(ECSchemaCR schema, Units::UnitSystemCR unitSystem, Units::PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant) : 
        Units::Unit(unitSystem, phenomenon, name, definition, numerator, denominator, offset, isConstant), m_isDisplayLabelExplicitlyDefined(false),m_schema(schema)
        {
        m_unitsContext = (Units::IUnitsContextCP) &schema;
        }

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this Unit is defined in.

    //! The fully qualified name of this ECUnit in the format {SchemaName}:{UnitFormatName}.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
    //! Gets a qualified name of the ECUnit, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;
    //! Gets the display label of this ECUnit.  If no display label has been set explicitly, it will return the name of the ECUnit.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    bool GetIsDisplayLabelDefined() const {return m_isDisplayLabelExplicitlyDefined;} //!< Whether the display label is explicitly defined or not.
    ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const; //!< Gets the invariant display label for this ECUnit.

    bool GetIsDescriptionDefined() const {return !m_description.empty();} //!< Returns true if description is not empty
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this ECUnit.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this ECUnit. Returns the localized description if one exists.

    PhenomenonCP GetPhenomenon() const {return (ECN::PhenomenonCP) T_Super::GetPhenomenon();}

    UnitSystemCP GetUnitSystem() const {return (ECN::UnitSystemCP) T_Super::GetUnitSystem();}

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    UnitId GetId() const {BeAssert(HasId()); return m_unitId;}
    //! Intended to be called by ECDb or a similar system.
    void SetId(UnitId id) {BeAssert(!m_unitId.IsValid()); m_unitId = id;}
    bool HasId() const {return m_unitId.IsValid();}

    //! If this unit is an inverted unit, the method returns the unit, this inverted unit inverts.
    ECUnitCP GetInvertingUnit() const { BeAssert(IsInvertedUnit()); return (ECUnitCP) GetParent(); }

    //! Write the ECUnit as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
    //! Write the inverted ECUnit as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteInvertedUnitJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
    //! Write the constant ECUnit as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteConstantJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
};
END_BENTLEY_ECOBJECT_NAMESPACE
