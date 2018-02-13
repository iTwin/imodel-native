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
friend struct Units::UnitRegistry;

private:
    // Inside of the validatedName we are caching the name without the Schema name prepended. This allows lookups to 
    // be done based on this name and not have to deconstruct the fullName stored in Units::UnitSystem every time.
    ECValidatedName m_validatedName;
    ECSchemaCP m_schema;
    Utf8String m_description;
    mutable UnitSystemId m_unitSystemId;
    
    void SetSchema(ECSchemaCR schema) {m_schema = &schema;}

    ECObjectsStatus SetName(Utf8StringCR name) {return (m_validatedName.SetValidName(name.c_str(), false)) ? ECObjectsStatus::Success : ECObjectsStatus::InvalidName;}

    ECObjectsStatus SetDisplayLabel(Utf8StringCR value) {m_validatedName.SetDisplayLabel(value.c_str()); return ECObjectsStatus::Success;}
    ECObjectsStatus SetDescription(Utf8StringCR value) {m_description = value; return ECObjectsStatus::Success;}

    static SchemaReadStatus ReadXml(UnitSystemP& system, BeXmlNodeR unitSystemNode, ECSchemaCR schema, ECSchemaReadContextR context);
    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    SchemaWriteStatus WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    // Should only be called by UnitRegistry
    UnitSystem(Utf8CP name) : Units::UnitSystem(name) {}

protected:
    // Needed by Units::UnitRegistry to create the UnitSystem
    ECOBJECTS_EXPORT static UnitSystemP _Create(Utf8CP name);

public:
    ECSchemaCR GetSchema() const {return *m_schema;} //!< The ECSchema that this UnitSystem is defined in

    Utf8StringCR GetName() const {return m_validatedName.GetName();}

    //! {SchemaName}:{UnitSystemName} The pointer will remain valid as long as the UnitSystem exists.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const {return T_Super::GetName();}
    //! Gets a qualified name of the UnitSystem, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;

    //! Gets the display label of this UnitSystem.  If no display label has been set explicitly, it will return the name of the UnitSystem
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    bool GetIsDisplayLabelDefined() const {return m_validatedName.IsDisplayLabelDefined();} //!< Whether the display label is explicitly defined or not
    Utf8StringCR GetInvariantDisplayLabel() const {return m_validatedName.GetDisplayLabel();} //!< Gets the invariant display label for this UnitSystem.

    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this UnitSystem.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this UnitSystem. Returns the localized description if one exists.

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    UnitSystemId GetId() const {BeAssert(HasId()); return m_unitSystemId;}
    //! Intended to be called by ECDb or a similar system
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
friend struct Units::UnitRegistry;

private:
    bool m_explicitDisplayLabel;
    Utf8String m_description;
    ECSchemaCP m_schema;

    void SetSchema(ECSchemaCR schema) {m_schema = &schema;}

    mutable Utf8String m_fullName;
    mutable PhenomenonId m_phenomenonId;

    ECObjectsStatus SetDisplayLabel(Utf8CP value);
    ECObjectsStatus SetDescription(Utf8CP value) {m_description = value; return ECObjectsStatus::Success;}

    static SchemaReadStatus ReadXml(PhenomenonP& phenomenon, BeXmlNodeR phenomenonNode, ECSchemaCR schema, ECSchemaReadContextR context);

    SchemaWriteStatus WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const;

    SchemaWriteStatus WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const;

    // Should only be called by UnitRegistry
    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id) : Units::Phenomenon(name, definition, baseSymbol, id), m_explicitDisplayLabel(false) {}

protected:
    // Needed by Units::UnitRegistry to create the Phenomenon
    ECOBJECTS_EXPORT static PhenomenonP _Create(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id);

public:
    ECSchemaCR GetSchema() const {return *m_schema;} //!< The ECSchema that this Phenomenon is defined in

    //! {SchemaName}:{PhenomenonName} The pointer will remain valid as long as the Phenomenon exists.
    ECOBJECTS_EXPORT Utf8StringCR GetFullName() const;
    //! Gets a qualified name of the Phenomenon, prefixed by the schema alias if it does not match the primary schema.
    ECOBJECTS_EXPORT Utf8String GetQualifiedName(ECSchemaCR primarySchema) const;
    //! Gets the display label of this Phenomenon.  If no display label has been set explicitly, it will return the name of the Phenomenon
    ECOBJECTS_EXPORT Utf8StringCR GetDisplayLabel() const;
    bool GetIsDisplayLabelDefined() const {return m_explicitDisplayLabel;} //!< Whether the display label is explicitly defined or not
    Utf8StringCR GetInvariantDisplayLabel() const {return Units::Phenomenon::GetInvariantLabel();} //!< Gets the invariant display label for this Phenomenon.

    Utf8StringCR GetInvariantDescription() const {return m_description;} //!< Gets the invariant description for this Phenomenon.
    ECOBJECTS_EXPORT Utf8StringCR GetDescription() const; //!< Gets the description of this Phenomenon. Returns the localized description if one exists.

    //! Return unique id (May return 0 until it has been explicitly set by ECDb or a similar system)
    PhenomenonId GetId() const {BeAssert(HasId()); return m_phenomenonId;}
    //! Intended to be called by ECDb or a similar system
    void SetId(PhenomenonId id) {BeAssert(!m_phenomenonId.IsValid()); m_phenomenonId = id;}
    bool HasId() const {return m_phenomenonId.IsValid();}

    //! Write the Phenomenon as a standalone schema child in the ECSchemaJSON format.
    //! @param[out] outValue                Json object containing the schema child Json if successfully written.
    //! @param[in]  includeSchemaVersion    If true the schema version will be included in the Json object.
    ECOBJECTS_EXPORT SchemaWriteStatus WriteJson(Json::Value& outValue, bool includeSchemaVersion = false) const;
};

END_BENTLEY_ECOBJECT_NAMESPACE
