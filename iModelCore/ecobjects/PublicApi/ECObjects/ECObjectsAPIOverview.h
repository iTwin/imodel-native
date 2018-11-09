/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjectsAPIOverview.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! @namespace BentleyApi::ECN Types defined by @ref ECObjectsGroup

//****** API doc main page *******

//! @addtogroup ECObjectsGroup
//! ECObjects is the heart of @b EC, Bentley's Information Modeling System. ECObjects is a set of abstractions 
//! working with engineering/business data and metadata (see @ref ECInstanceHowTos for details).
//! 
//! An ECSchema is used to describe the data model / metadata of the information model. An ECSchema is just a collection of 
//! @ref ECClass "ECClasses". You can think of an ECClass as being like a C++ class that only defines 
//! properties (ECClasses define no methods or behaviors.) In some ways, they are closer to 
//! C++ pure virtual abstract base classes that only contain property getters and setters. 
//! They are also very analogous to a database table definition.
//! ECClasses contain @ref ECProperty "ECProperties". These are property *definitions*, not values.
//!
//! There are also @ref ECRelationshipClass "ECRelationshipClasses" that are ECClasses that define @ref ECRelationshipConstraint "ECRelationshipConstraints" 
//! indicating what ECClasses they relate. 
//! ECRelationshipInstances represent the relationships between the ECInstances (defined/constrained by their ECRelationshipClass) 
//! ECRelationships are analogous to database foreign key constraint.
//! 
//! @ref IECInstance "ECInstances" represent instances of business data and refer to an ECClass and can hold values for each ECProperty. 
//! They are analogous to the rows of a database table.
//!
//! @see @ref ECInstanceHowTos


//****** ECInstancesHowTos *******

//! @page ECInstanceHowTos Working with ECInstances
//! @section ECInstancesStructPropertiesHowTos ECInstances and struct properties
//! Structs can only be set in an IECInstance member by member using the fully qualified property access string to each member.
//! Using %ECValue::SetStruct will result in an error. This is intentional to protect clients from performance issues which the 
//! %ECValue::SetStruct option would have.
//!
//! @e Right:
//!
//!     IECInstanceP instance = ...;
//!     ECValue v;
//!     v.SetUtf8CP (...);
//!     instance->SetValue (L"Headquarter.Name", v);
//!     v.SetUtf8CP (...);
//!     instance->SetValue (L"Headquarter.TelephoneNumber", v);
//!     ...
//!     v.SetInteger (...);
//!     instance->SetValue (L"Headquarter.Location.Zip", v);
//!     ...
//!
//!
//! @e Wrong:
//!
//!     IECInstanceP instance = ...;
//!     IECInstanceP headquarterStructInstance = ...;
//!     //populate the headquarterStructInstance...
//!     ...
//!     ECValue v;
//!     v.SetStruct (headquarterStructInstance);
//!     instance->SetValue (L"Headquarter", v); //returns error
//!
//!
//! @section ECInstancesStructArrayPropertiesHowTos ECInstances and struct array properties
//! In contrast to @ref ECInstancesStructPropertiesHowTos "struct properties", struct array properties can only be set 
//! in an IECInstance using ECValue::SetStruct. Here using the property access string option would fail.
//!
//! @note ECValue::SetStruct does not copy the struct object.
//!
//! @e Example:
//!
//!     IECInstanceP instance = ...;
//!     IECInstanceP affiliate1StructInstance = ...;
//!     IECInstanceP affiliate2StructInstance = ...;
//!     IECInstanceP affiliate3StructInstance = ...;
//!     ...
//!     instance->AddArrayElements (L"Affiliates", 3); //initialize the struct array to hold 3 struct elements
//!     ECValue v;
//!     v.SetStruct (affiliate1StructInstance); // does not copy the struct
//!     instance->SetValue (L"Affiliates", v, 0); 
//!     v.SetStruct (affiliate2StructInstance); // does not copy the struct
//!     instance->SetValue (L"Affiliates", v, 1);
//!     v.SetStruct (affiliate3StructInstance); // does not copy the struct
//!     instance->SetValue (L"Affiliates", v, 2);
//!
//!
//! @section ECInstancesDateTimePropertiesHowTos ECInstances and DateTime properties
//! %DateTime values in ECObjects are represented by the BentleyApi::DateTime class. Each DateTime instance
//! can contain metadata about the actual date time value (see also DateTime::Info). 
//! In order to preserve the metadata when persisting a DateTime, clients can decorate the respective
//! ECProperty with the @b %DateTimeInfo custom attribute from the standard ECSchema @b Bentley_Standard_CustomAttributes.
//! @note The metadata is not persisted per-instance. Instead it is the custom attribute that holds the information on a per-property
//! base. So all date times within a given ECProperty have the same date time metadata.
//! @see DateTimeInfo, StandardCustomAttributeHelper
//!
//! ### How the @b %DateTimeInfo custom attribute affects %IECInstance::SetValue
//! @li If the custom attribute is present, the metadata of the DateTime object is checked against the custom attribute. If it doesn't match,
//! IECInstance::SetValue is aborted with an error.
//! @li If the custom attribute is not present, no metadata checks are performed. 
//!
//!
//!     //Assume an ECProperty 'LastModified' without DateTimeInfo custom attribute and
//!     //an ECProperty 'LastModifiedUtc' with a DateTimeInfo custom attribute where DateTimeKind is set to 'Utc'
//!     DateTime dt (DateTime::Kind::Unspecified, 2013, 2, 26, 12, 30);
//!     ECValue v (dt);
//!     IECInstanceP instance = ...;
//!     instance->SetValue (L"LastModified", v); // works as LastModified doesn't have a DateTimeInfo custom attribute
//!     instance->SetValue (L"LastModifiedUtc", v); //returns error as DateTime metadata doesn't match ECProperty's DateTimeInfo custom attribute
//! 
//! ### How the @b %DateTimeInfo custom attribute affects IECInstance::GetValue
//! @li If the custom attribute is present, the resulting DateTime object will have the metadata from the custom attribute.
//! @li If the custom attribute is not present, the resulting DateTime object will get @e default metadata as defined by DateTimeInfo::GetDefault
//!
//! @note Only the use of the %DateTimeInfo custom attribute preserves the full information of a DateTime object during a roundtrip. Not using
//! the custom attribute is applicable for use cases where the metadata is irrelevant or of no interest.
//!
//!     //Assume an ECProperty 'LastModified' without DateTimeInfo custom attribute and
//!     //an ECProperty 'LastModifiedUtc' with a DateTimeInfo custom attribute where DateTimeKind is set to 'Utc'
//!     ECValue v;
//!     IECInstanceP instance = ...;
//!     instance->GetValue (v, L"LastModified");
//!     DateTime dt = v.GetDateTime (); //dt.GetInfo () will amount to default
//!     ...
//!     instance->GetValue (v, L"LastModifiedUtc");
//!     dt = v.GetDateTime (); //dt.GetInfo ().GetKind () will amount to DateTime::Kind::Utc
//! 
//!
//! @section ECInstancesPropertyIndex Property Index
//! The property index is the index into the PropertyLayout of the corresponding ECProperty.
//! @note The property index is @b 1-based
//!
//! @see BentleyApi::ECN

END_BENTLEY_ECOBJECT_NAMESPACE
