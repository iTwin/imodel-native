/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

private:
    ECSchemaCR m_schema;

    bool m_hasExplicitDisplayLabel;
    Utf8String m_displayLabel;
    Utf8String m_description;
    CachedSchemaQualifiedName m_fullName; // Cached when GetFullName is called the first time.
    mutable UnitSystemId m_unitSystemId;

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {m_displayLabel = value.c_str(); m_hasExplicitDisplayLabel = true; return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}

    SchemaReadStatus ReadXml(BeXmlNodeR unitSystemNode, ECSchemaReadContextR context);
    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    bool ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    UnitSystem(ECSchemaCR schema, Utf8CP name);

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this UnitSystem is defined in.

    void SetId(UnitSystemId id) {BeAssert(!m_unitSystemId.IsValid()); m_unitSystemId = id;} //!< Intended to be called by ECDb or a similar system.
    UnitSystemId GetId() const {BeAssert(HasId()); return m_unitSystemId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    bool HasId() const {return m_unitSystemId.IsValid();}

    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //<! The full name of this UnitSystem in the format, {SchemaName}:{UnitSystemName}.
    //! Gets a qualified name of the UnitSystem, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this UnitSystem. Returns the localized description if one exists.
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this UnitSystem.
    bool GetIsDescriptionDefined() const {return m_description.length() > 0;} //!< Returns true if description length is 0

    //! Gets the display label of this UnitSystem.  If no display label has been set explicitly, it will return the name of the UnitSystem.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    Utf8StringCR GetInvariantDisplayLabel() const {return (m_hasExplicitDisplayLabel) ? m_displayLabel : GetName();} //!< Gets the invariant display label for this UnitSystem.
    bool GetIsDisplayLabelDefined() const {return m_hasExplicitDisplayLabel;} //!< Whether the display label is explicitly defined or not.

    //! Write the UnitSystem as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct Phenomenon : Units::Phenomenon, NonCopyableClass
{
DEFINE_T_SUPER(Units::Phenomenon)
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

private:
    bool m_isDisplayLabelExplicitlyDefined; // displayLabel is in Units::Phenomenon
    Utf8String m_description;
    ECSchemaCR m_schema;
    PhenomenonId m_phenomenonId;
    CachedSchemaQualifiedName m_fullName;

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {Units::Phenomenon::SetLabel(value.c_str()); m_isDisplayLabelExplicitlyDefined = true; return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}

    SchemaReadStatus ReadXml(BeXmlNodeR phenomenonNode, ECSchemaReadContextR context);

    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    bool ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    Phenomenon(ECSchemaCR schema, Utf8CP name);
    Phenomenon(ECSchemaCR schema, Utf8CP name, Utf8CP definition);

public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this Phenomenon is defined in.

    void SetId(PhenomenonId id) {BeAssert(!m_phenomenonId.IsValid()); m_phenomenonId = id;} //!< Intended to be called by ECDb or a similar system.
    PhenomenonId GetId() const {BeAssert(HasId()); return m_phenomenonId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    bool HasId() const {return m_phenomenonId.IsValid();}

    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //!< The full name of this Phenomenon in the format {SchemaName}:{PhenomenonName}.
    //! Gets a qualified name of the Phenomenon, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this Phenomenon. Returns the localized description if one exists.
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this Phenomenon.
    bool GetIsDescriptionDefined() const {return m_description.length() > 0;} //!< Returns true if description length is 0

    //! Gets the display label of this Phenomenon.  If no display label has been set explicitly, it will return the name of the Phenomenon.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    ECOBJECTS_EXPORT Utf8StringCR GetInvariantDisplayLabel() const; //!< Gets the invariant display label for this Phenomenon.
    bool GetIsDisplayLabelDefined() const {return m_isDisplayLabelExplicitlyDefined;} //!< Whether the display label is explicitly defined or not.

    ECUnitCP GetSIUnit() const {return (ECUnitCP) T_Super::GetSIUnit();}

    //! Write the Phenomenon as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct ECUnit : Units::Unit, NonCopyableClass 
{
DEFINE_T_SUPER(Units::Unit)
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

private:
    ECSchemaCR m_schema;
    UnitId m_unitId;

    bool m_isNumeratorExplicitlyDefined = false;
    bool m_isDenominatorExplicitlyDefined = false;
    bool m_isOffsetExplicitlyDefined = false;

    Utf8String m_description;
    CachedSchemaQualifiedName m_fullName;

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

    ECOBJECTS_EXPORT bool ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;
    bool InvertedUnitToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;
    bool ConstantToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    ECUnit(ECSchemaCR schema, Utf8CP name);
    ECUnit(ECSchemaCR schema, Units::UnitCR parentUnit, Units::UnitSystemCR system, Utf8CP unitName); //!< Creates and inverted unit
    ECUnit(ECSchemaCR schema, Units::PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition, double numerator, Nullable<double> denominator); //!< Creates a constant.

    ECSchemaR GetSchemaR() const {return const_cast<ECSchemaR>(m_schema);}
    
public:
    ECSchemaCR GetSchema() const {return m_schema;} //!< The ECSchema that this Unit is defined in.

    void SetId(UnitId id) {BeAssert(!m_unitId.IsValid()); m_unitId = id;} //!< Intended to be called by ECDb or a similar system.
    UnitId GetId() const {BeAssert(HasId()); return m_unitId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    bool HasId() const {return m_unitId.IsValid();}

    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //!< The full name of this ECUnit in the format {SchemaName}:{UnitFormatName}.
    //! Gets a qualified name of the ECUnit, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this ECUnit. Returns the localized description if one exists.
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this ECUnit.
    bool GetIsDescriptionDefined() const {return m_description.length() > 0;} //!< Returns true if description length is 0

    //! Gets the display label of this ECUnit.  If no display label has been set explicitly, it will return the name of the ECUnit.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const override;

    bool HasNumerator() const override {return m_isNumeratorExplicitlyDefined;} //!< Returns true if the numerator has been explicitly set
    bool HasDenominator() const override {return m_isDenominatorExplicitlyDefined;} //!< Returns true if the denominator has been explicitly set
    bool HasOffset() const override {return m_isOffsetExplicitlyDefined;} //!< Returns true if the offset has been explicitly set
    PhenomenonCP GetPhenomenon() const override {return (ECN::PhenomenonCP) T_Super::GetPhenomenon();}

    UnitSystemCP GetUnitSystem() const {return (ECN::UnitSystemCP) T_Super::GetUnitSystem();}

    //! If this unit is an inverted unit, the method returns the unit, this inverted unit inverts.
    ECUnitCP GetInvertingUnit() const {BeAssert(IsInvertedUnit()); return (ECUnitCP) GetParent();}

    //! Write the ECUnit as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    bool ToJson(Json::Value& outValue, bool includeSchemaVersion = false) const {return ToJson(outValue, true, includeSchemaVersion);}
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
    ECOBJECTS_EXPORT bool _ToJson(Json::Value& out, bool verbose) const override;
    void SetParentFormat(ECFormatCP parent) {m_ecFormat = parent;}

public:
    ECOBJECTS_EXPORT NamedFormat(Utf8StringCR name = "", ECFormatCP format = nullptr);
    Utf8StringCR GetName() const {return m_nameOrFormatString;}
    //! Gets a qualified name of the Format, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedFormatString(ECSchemaCR primarySchema) const;
    bool IsOverride() const {return this != (NamedFormatCP)m_ecFormat;}
    ECFormatCP GetParentFormat() const {return m_ecFormat;}
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct ECFormat : NamedFormat, NonCopyableClass
{
DEFINE_T_SUPER(NamedFormat)
friend struct ECSchema; // needed for SetName() method
friend struct SchemaXmlWriter; // needed for WriteXml() method
friend struct SchemaXmlReaderImpl; // needed for ReadXml() method
friend struct SchemaJsonWriter; // needed for the ToJson() method

private:
    ECSchemaCP m_schema;
    bool m_isDisplayLabelExplicitlyDefined;
    CachedSchemaQualifiedName m_fullName;
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
    bool ToJsonInternal(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    ECFormat(ECSchemaCR schema, Utf8StringCR name);
    bool _ToJson(Json::Value& out, bool verbose) const override {return ToJsonInternal(out, true, verbose);}
public:
    ECSchemaCR GetSchema() const {return *m_schema;} //!< The ECSchema that this Format is defined in.

    FormatId GetId() const {BeAssert(HasId()); return m_formatId;} //!< Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system).
    void SetId(FormatId id) {BeAssert(!m_formatId.IsValid()); m_formatId = id;} //!< Intended to be called by ECDb or a similar system.
    bool HasId() const {return m_formatId.IsValid();}

    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const; //!< The full name of this Format in the format {SchemaName}:{FormatName}.
    //!< Gets a qualified name of the ECFormat, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Returns the description of this Format. Returns the localized description if one exists.
    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Returns the invariant description of this Format.
    bool GetIsDescriptionDefined() const {return !m_description.empty();} //!< Returns true if description length is 0.

    //! Returns the localized display label of this Format. Returns the localized display label if one exists.
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    //! Returns true if the display label is explicitly defined.
    bool GetIsDisplayLabelDefined() const {return m_isDisplayLabelExplicitlyDefined;}
    //! Returns the invariant display label for this Format. Returns the name of the Format if no display label has been explicitly defined.
    Utf8StringCR GetInvariantDisplayLabel() const {return GetIsDisplayLabelDefined() ? m_displayLabel : GetName();}
};

END_BENTLEY_ECOBJECT_NAMESPACE
