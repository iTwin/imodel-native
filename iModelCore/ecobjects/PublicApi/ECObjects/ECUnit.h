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
#include <Bentley/Nullable.h>
#include <Formatting/FormattingApi.h>

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

    UnitSystem(ECSchemaCR schema, Utf8CP name);

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

    bool GetIsDescriptionDefined() const {return m_description.length() > 0;} //!< Returns true if description length is 0
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

    Phenomenon(ECSchemaCR schema, Utf8CP name);
    Phenomenon(ECSchemaCR schema, Utf8CP name, Utf8CP definition);

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this Phenomenon is defined in.

    //! The fully qualified name of this Phenomenon in the format {SchemaName}:{PhenomenonName}.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
    //! Gets a qualified name of the Phenomenon, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;
    //! Gets the display label of this Phenomenon.  If no display label has been set explicitly, it will return the name of the Phenomenon.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    bool GetIsDisplayLabelDefined() const {return m_isDisplayLabelExplicitlyDefined;} //!< Whether the display label is explicitly defined or not.
    ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const; //!< Gets the invariant display label for this Phenomenon.

    bool GetIsDescriptionDefined() const {return m_description.length() > 0;} //!< Returns true if description length is 0
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

    ECUnitCP GetSIUnit() const {return (ECUnitCP) T_Super::GetSIUnit();}
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
    bool m_isNumeratorExplicitlyDefined;
    bool m_isDenominatorExplicitlyDefined;
    bool m_isOffsetExplicitlyDefined;

    Utf8String m_description;
    mutable Utf8String m_fullName;

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {Units::Unit::SetLabel(value.c_str()); m_isDisplayLabelExplicitlyDefined = true; return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}
    BentleyStatus SetNumerator(double value) override {m_isNumeratorExplicitlyDefined = true; return Units::Unit::SetNumerator(value);}
    BentleyStatus SetDenominator(double value) override {m_isDenominatorExplicitlyDefined = true; return Units::Unit::SetDenominator(value);}
    BentleyStatus SetOffset(double value) override {m_isOffsetExplicitlyDefined = true; return Units::Unit::SetOffset(value);}

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

    ECUnit(ECSchemaCR schema, Utf8CP name);
    ECUnit(ECSchemaCR schema, Units::UnitCR parentUnit, Units::UnitSystemCR system, Utf8CP unitName); //!< Creates and inverted unit
    ECUnit(ECSchemaCR schema, Units::PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition, double numerator, double denominator); //!< Creates a constant.
    ECUnit(ECSchemaCR schema, Units::UnitSystemCR unitSystem, Units::PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant); //! Creates a unit.
    ECUnit(ECSchemaCR schema, Units::UnitSystemCR unitSystem, Units::PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition); //! Creates a unit.

    ECSchemaR GetSchemaR() const {return const_cast<ECSchemaR>(m_schema);}

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

    bool GetIsDescriptionDefined() const {return m_description.length() > 0;} //!< Returns true if description length is 0
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this ECUnit.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this ECUnit. Returns the localized description if one exists.

    bool HasNumerator() const override {return m_isNumeratorExplicitlyDefined;} //!< Returns true if the numerator has been explicitly set
    bool HasDenominator() const override {return m_isDenominatorExplicitlyDefined;} //!< Returns true if the denominator has been explicitly set
    bool HasOffset() const override {return m_isOffsetExplicitlyDefined;} //!< Returns true if the offset has been explicitly set
    PhenomenonCP GetPhenomenon() const override {return (ECN::PhenomenonCP) T_Super::GetPhenomenon();}

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

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct NamedFormat : Formatting::Format
{
DEFINE_T_SUPER(Formatting::Format)
friend ECSchema;

private:
    Utf8String m_nameOrFormatString;
    ECFormatCP m_ecFormat;

protected:
    void SetParentFormat(ECFormatCP parent) {m_ecFormat = parent;}

public:
    ECOBJECTS_EXPORT NamedFormat(Utf8StringCR name = "", ECFormatCP format = nullptr);
    Utf8StringCR GetName() const {return m_nameOrFormatString;}
    bool IsOverride() const {return this != (NamedFormatCP)m_ecFormat;}
    ECFormatCP GetParentFormat() const {return m_ecFormat;}
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct ECFormat : NamedFormat, NonCopyableClass
{
DEFINE_T_SUPER(NamedFormat)
friend struct ECSchema;
friend struct SchemaXmlWriter;
friend struct SchemaXmlReaderImpl;
friend struct SchemaJsonWriter;

private:
    ECSchemaCP m_schema;
    bool m_isDisplayLabelExplicitlyDefined;
    mutable Utf8String m_fullName;
    Utf8String m_displayLabel;
    Utf8String m_description;
    mutable FormatId m_formatId;

    ECObjectsStatus SetSchema(ECSchemaCR schema);

    ECObjectsStatus SetDisplayLabel(Utf8StringCR displayLabel) {m_displayLabel = displayLabel; m_isDisplayLabelExplicitlyDefined = true; return ECObjectsStatus::Success;}

    ECObjectsStatus SetDescription(Utf8StringCR description) {m_description = description; return ECObjectsStatus::Success;}

    SchemaReadStatus ReadXml(BeXmlNodeR unitFormatNode, ECSchemaReadContextR context);
    SchemaReadStatus ReadCompositeSpecXml(BeXmlNodeR compositeNode, ECSchemaReadContextR context);
    SchemaReadStatus ReadCompositeUnitXml(BeXmlNodeR unitNode, ECSchemaReadContextR context, bvector<ECUnitCP>& units, bvector<Nullable<Utf8String>>& labels);

    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;
    SchemaWriteStatus WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    ECFormat(ECSchemaCR schema, Utf8StringCR name);

public:
    ECOBJECTS_EXPORT ECSchemaCR GetSchema() const {return *m_schema;} //!< The ECSchema that this Format is defined in.

    //! The fully qualified name of this Format in the format {SchemaName}:{FormatName}.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
    //! Gets a qualified name of the Format, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    //! Returns the localized display label of this Format. Returns the localized display label if one exists.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    //! Returns true if the display label is explicitly defined.
    bool GetIsDisplayLabelDefined() const {return m_isDisplayLabelExplicitlyDefined;}
    //! Returns the invariant display label for this Format. Returns the name of the Format if no display label has been explicitly defined.
    Utf8StringCR GetInvariantDisplayLabel() const {return GetIsDisplayLabelDefined() ? m_displayLabel : GetName();}
    bool GetIsDescriptionDefined() const {return m_displayLabel.length() > 0;} //!< Returns true if description length is 0
    //! Returns the description of this Format. Returns the localized description if one exists.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const;
    //! Returns the invariant description of this Format.
    Utf8StringCR GetInvariantDescription() const {return m_description;}

    //! Write the Format as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteJson(Json::Value& outValue, bool includeSchemaVersion = false) const;

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    FormatId GetId() const {BeAssert(HasId()); return m_formatId;}
    //! Intended to be called by ECDb or a similar system.
    void SetId(FormatId id) {BeAssert(!m_formatId.IsValid()); m_formatId = id;}
    bool HasId() const {return m_formatId.IsValid();}
};

END_BENTLEY_ECOBJECT_NAMESPACE
