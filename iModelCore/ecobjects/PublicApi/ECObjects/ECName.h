/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjects.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Used to hold property name and display label for ECSchema, ECClass, ECProperty, 
* ECEnumeration, KindOfQuantity and PropertyCategory
* Property name supports only a limited set of characters; unsupported characters must
* be escaped as "__x####__" where "####" is a UTF-16 character code.
* If no explicit display label is provided, the property name is used as the display
* label, with encoded special characters decoded.
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECValidatedName
{
private:
    bool m_hasExplicitDisplayLabel;
    Utf8String m_name;
    Utf8String m_displayLabel;
public:
    ECValidatedName() : m_hasExplicitDisplayLabel(false) {}

    Utf8StringCR GetName() const {return m_name;}
    bool IsDisplayLabelDefined() const {return m_hasExplicitDisplayLabel;}
    //! Returns the display label if set or the name if display label is not set
    Utf8StringCR GetDisplayLabel() const {return m_hasExplicitDisplayLabel ? m_displayLabel : m_name;}
#ifndef DOCUMENTATION_GENERATOR
    //! Sets the name with no encoding if input string is valid ECName, returns false if input is not valid ECName.
    //! @param[in]  name                        Characters to set as name
    //! @param[in]  decodeNameToDisplayLabel    If true and no display label set the name will be decoded into the display label using ECNameValidation::DecodeFromValidName
    //! @remarks    If decodeNameToDisplayLabel is set and the name is decoded into a display label IsDisplaylabelDefined will be set to true, if not it will be set to false.
    bool SetValidName(Utf8CP name, bool decodeNameToDisplayLabel);
    void SetName(Utf8CP name);
    void SetDisplayLabel(Utf8CP label);
#endif
};

//=======================================================================================
//! Handles validation, encoding, and decoding of names for ECSchemas, ECClasses, 
//! ECProperties, ECEnumerations, KindOfQuantities and PropertyCategories.
//! The names of ECSchemas, ECClasses, ECProperties, ECEnumerations, KindOfQuantities,
//! PropertyCategories must conform to the following rules:
//!     -Contains only alphanumeric characters in the ranges ['A'..'Z'], ['a'..'z'], ['0'..'9'], and ['_']
//!     -Contains at least one character
//!     -Does not begin with a digit
//! @addtogroup ECObjectsGroup
//! @beginGroup
//! @bsiclass
//=======================================================================================
struct ECNameValidation
{
private:
    static void AppendEncodedCharacter(WStringR encoded, WChar c);
public:
    //! Encodes special characters in a possibly invalid name to produce a valid name
    //! Depreciated, use bool EncodeToValidName(Utf8StringR encoded, Utf8StringCR name)
    ECOBJECTS_EXPORT static Utf8String EncodeToValidName(Utf8StringCR name);

    //! Enumeration defining the result of a validation check
    enum ValidationResult
        {
        RESULT_Valid = 0,                   //!< The name is valid
        RESULT_NullOrEmpty,                 //!< The string to check was NULL or empty
        RESULT_BeginsWithDigit,             //!< The string begins with a digit
        RESULT_IncludesInvalidCharacters    //!< The string contains invalid characters
        };

    //! Encodes special characters in a possibly invalid name to produce a valid name
    //! @param[out] encoded     Will hold the valid name
    //! @param[in]  name        The name to encode
    //! @returns true if any special characters were encoded, false if the name was already valid. In either case encoded will contain a valid name.
    ECOBJECTS_EXPORT static bool EncodeToValidName(Utf8StringR encoded, Utf8StringCR name);

    //! Decodes special characters in a name encoded by EncodeToValidName() to produce a name suitable for display
    //! @param[out] decoded     Will hold the decoded name
    //! @param[in]  name        The name to decode
    //! @returns true if any special characters were decoded. Regardless of return value, decoded will contain a decoded name.
    ECOBJECTS_EXPORT static bool DecodeFromValidName(Utf8StringR decoded, Utf8StringCR name);

    //! Checks a name against the rules for valid names
    //! @param[in] name     The name to validate
    //! @returns RESULT_Valid if the name is valid, or a ValidationResult indicating why the name is invalid.
    ECOBJECTS_EXPORT static ValidationResult Validate(Utf8CP name);

    //! Returns true if the specified name is a valid EC name
    static bool IsValidName(Utf8CP name) {return RESULT_Valid == Validate(name);}

    //! Checks whether a character is valid for use in an ECName, e.g. alphanumeric, plus '_'
    static bool IsValidAlphaNumericCharacter(WChar c) {return (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'));}
    static bool IsValidAlphaNumericCharacter(Utf8Char c) {return (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'));}
};

//=======================================================================================
//! The full name of some item belonging to an ECSchema, formatted as
//! "SchemaName:ItemName".
// @bsistruct                                                   Paul.Connelly   03/19
//=======================================================================================
struct CachedSchemaQualifiedName : CachedUtf8String
{
private:
    template<typename NamedItem> static Utf8String ComputeName(NamedItem const& item)
        {
        return item.GetSchema().GetName() + ":" + item.GetName();
        }
public:
    template<typename NamedItem> Utf8StringCR GetName(NamedItem const& item) const
        {
        return Get([&]() { return ComputeName(item); });
        }

    template<typename NamedItem> void RecomputeName(NamedItem const& item) const
        {
        Set(ComputeName(item));
        }
};

END_BENTLEY_ECOBJECT_NAMESPACE
