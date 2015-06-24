/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjects.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>

#ifdef __ECOBJECTS_BUILD__
    #define ECOBJECTS_EXPORT EXPORT_ATTRIBUTE
#else
    #define ECOBJECTS_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_ECOBJECT_NAMESPACE    BEGIN_BENTLEY_NAMESPACE namespace ECN {
#define END_BENTLEY_ECOBJECT_NAMESPACE      } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_EC                  using namespace BentleyApi::ECN;

#define EC_TYPEDEFS(_name_)  \
    BEGIN_BENTLEY_ECOBJECT_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ECOBJECT_NAMESPACE

EC_TYPEDEFS(ECValue);
EC_TYPEDEFS(ECValueAccessor);
EC_TYPEDEFS(ECValueAccessorPair);
EC_TYPEDEFS(ECValueAccessorPairCollection);
EC_TYPEDEFS(ECValueAccessorPairCollectionOptions);
EC_TYPEDEFS(ECValuesCollection);
EC_TYPEDEFS(ArrayInfo);
EC_TYPEDEFS(ECSchema);
EC_TYPEDEFS (SchemaKey);
EC_TYPEDEFS(ECSchemaReadContext);
EC_TYPEDEFS(ECProperty);
EC_TYPEDEFS(PrimitiveECProperty);
EC_TYPEDEFS(DateTimeInfo);
EC_TYPEDEFS(StructECProperty);
EC_TYPEDEFS(ArrayECProperty);
EC_TYPEDEFS(ECPropertyIterable);
EC_TYPEDEFS(ECClassContainer);
EC_TYPEDEFS(ECClass);
EC_TYPEDEFS(ECRelationshipConstraintClass);
EC_TYPEDEFS(ECRelationshipClass);
EC_TYPEDEFS(ECRelationshipConstraint);
EC_TYPEDEFS(RelationshipCardinality);
EC_TYPEDEFS(IECInstance);
EC_TYPEDEFS(IECInstanceInterface);
EC_TYPEDEFS(IECRelationshipInstance);
EC_TYPEDEFS(IECSchemaLocater);
EC_TYPEDEFS(IECCustomAttributeContainer);
EC_TYPEDEFS(ECInstanceReadContext);
EC_TYPEDEFS(ECSchemaCache);
EC_TYPEDEFS(ECPropertyValue);
EC_TYPEDEFS(IECWipRelationshipInstance);
EC_TYPEDEFS(ECRelationshipInstanceHolder);

EC_TYPEDEFS(ECEnabler);
EC_TYPEDEFS(StandaloneECEnabler);
EC_TYPEDEFS(IStandaloneEnablerLocater);
EC_TYPEDEFS(IArrayManipulator);

EC_TYPEDEFS(SchemaLayout);
EC_TYPEDEFS(ClassLayout);
EC_TYPEDEFS(PropertyLayout);
EC_TYPEDEFS(StandaloneECInstance);
EC_TYPEDEFS(MemoryECInstanceBase);
EC_TYPEDEFS(ECDBuffer);

EC_TYPEDEFS(ICustomECStructSerializer);
EC_TYPEDEFS(CustomStructSerializerManager);
EC_TYPEDEFS (ECInstanceIterable);
EC_TYPEDEFS(SupplementalSchemaMetaData);
EC_TYPEDEFS (PresentationMetadataHelper);

EC_TYPEDEFS (CalculatedPropertySpecification);
EC_TYPEDEFS (ParserRegex);
EC_TYPEDEFS(IECTypeAdapterContext);
EC_TYPEDEFS(IECSchemaRemapper);

EC_TYPEDEFS (SchemaNameClassNamePair);
EC_TYPEDEFS (UnitSpec);
EC_TYPEDEFS (Unit);
EC_TYPEDEFS (UnitConverter);

//__PUBLISH_SECTION_END__
EC_TYPEDEFS (AdhocPropertyMetadata);
EC_TYPEDEFS (AdhocPropertyQuery);
EC_TYPEDEFS (AdhocPropertyEdit);
//__PUBLISH_SECTION_START__

struct IStream;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef struct IStream* IStreamP;

//! Error code categories
typedef enum ECErrorCategories
    {
    ECOBJECTS_ERROR_BASE            = 0x31000,
    SCHEMA_READ_STATUS_BASE         = 0x32000,
    SCHEMA_WRITE_STATUS_BASE        = 0x33000,
    INSTANCE_READ_STATUS_BASE       = 0x34000,
    INSTANCE_WRITE_STATUS_BASE      = 0x35000,
    SUPPLEMENTED_SCHEMA_STATUS_BASE = 0x36000,
    } ECErrorCategories;

//! General purpose result codes
enum ECObjectsStatus
    {
    ECOBJECTS_STATUS_Success                                            = SUCCESS,
    ECOBJECTS_STATUS_PropertyNotFound                                   = ECOBJECTS_ERROR_BASE + 0x01,
    ECOBJECTS_STATUS_DataTypeMismatch                                   = ECOBJECTS_ERROR_BASE + 0x02,
    ECOBJECTS_STATUS_ECInstanceImplementationNotSupported               = ECOBJECTS_ERROR_BASE + 0x03,
    ECOBJECTS_STATUS_InvalidPropertyAccessString                        = ECOBJECTS_ERROR_BASE + 0x04,
    ECOBJECTS_STATUS_IndexOutOfRange                                    = ECOBJECTS_ERROR_BASE + 0x05,
    ECOBJECTS_STATUS_ECClassNotSupported                                = ECOBJECTS_ERROR_BASE + 0x06,
    ECOBJECTS_STATUS_ECSchemaNotSupported                               = ECOBJECTS_ERROR_BASE + 0x07,
    ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices                  = ECOBJECTS_ERROR_BASE + 0x08,
    ECOBJECTS_STATUS_EnablerNotFound                                    = ECOBJECTS_ERROR_BASE + 0x09,
    ECOBJECTS_STATUS_OperationNotSupported                              = ECOBJECTS_ERROR_BASE + 0x0A,
    ECOBJECTS_STATUS_ParseError                                         = ECOBJECTS_ERROR_BASE + 0x0B,
    ECOBJECTS_STATUS_NamedItemAlreadyExists                             = ECOBJECTS_ERROR_BASE + 0x0C,
    ECOBJECTS_STATUS_PreconditionViolated                               = ECOBJECTS_ERROR_BASE + 0x0D,
    ECOBJECTS_STATUS_SchemaNotFound                                     = ECOBJECTS_ERROR_BASE + 0x0E,
    ECOBJECTS_STATUS_ClassNotFound                                      = ECOBJECTS_ERROR_BASE + 0x0F,
    ECOBJECTS_STATUS_BaseClassUnacceptable                              = ECOBJECTS_ERROR_BASE + 0x10,
    ECOBJECTS_STATUS_SchemaInUse                                        = ECOBJECTS_ERROR_BASE + 0x11,
    ECOBJECTS_STATUS_InvalidName                                        = ECOBJECTS_ERROR_BASE + 0x12,
    ECOBJECTS_STATUS_DataTypeNotSupported                               = ECOBJECTS_ERROR_BASE + 0x13,
    ECOBJECTS_STATUS_UnableToAllocateMemory                             = ECOBJECTS_ERROR_BASE + 0x14,
    ECOBJECTS_STATUS_MemoryBoundsOverrun                                = ECOBJECTS_ERROR_BASE + 0x15,
    ECOBJECTS_STATUS_NullPointerValue                                   = ECOBJECTS_ERROR_BASE + 0x16,
    ECOBJECTS_STATUS_NotCustomAttributeClass                            = ECOBJECTS_ERROR_BASE + 0x17,
    ECOBJECTS_STATUS_DuplicateSchema                                    = ECOBJECTS_ERROR_BASE + 0x18,
    ECOBJECTS_STATUS_UnableToSetReadOnlyInstance                        = ECOBJECTS_ERROR_BASE + 0x19,
    ECOBJECTS_STATUS_UnableToSetReadOnlyProperty                        = ECOBJECTS_ERROR_BASE + 0x1A,
    ECOBJECTS_STATUS_ArrayIndexDoesNotExist                             = ECOBJECTS_ERROR_BASE + 0x1B,
    ECOBJECTS_STATUS_PropertyValueMatchesNoChange                       = ECOBJECTS_ERROR_BASE + 0x1C,
    ECOBJECTS_STATUS_NoChildProperties                                  = ECOBJECTS_ERROR_BASE + 0x1D,
    ECOBJECTS_STATUS_UnableToAllocateManagedMemory                      = ECOBJECTS_ERROR_BASE + 0x1E,
    ECOBJECTS_STATUS_MemoryAllocationCallbackRequired                   = ECOBJECTS_ERROR_BASE + 0x1F,
    ECOBJECTS_STATUS_UnableToAddStructArrayMember                       = ECOBJECTS_ERROR_BASE + 0x20,
    ECOBJECTS_STATUS_UnableToSetStructArrayMemberInstance               = ECOBJECTS_ERROR_BASE + 0x21,
    ECOBJECTS_STATUS_UnableToGetStructArrayMemberInstance               = ECOBJECTS_ERROR_BASE + 0x22,
    ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag                     = ECOBJECTS_ERROR_BASE + 0x23,
    ECOBJECTS_STATUS_SchemaHasReferenceCycle                            = ECOBJECTS_ERROR_BASE + 0x24,
    ECOBJECTS_STATUS_SchemaNotSupplemented                              = ECOBJECTS_ERROR_BASE + 0x25,
    ECOBJECTS_STATUS_UnableToQueryForNullPropertyFlag                   = ECOBJECTS_ERROR_BASE + 0x26,
    ECOBJECTS_STATUS_UnableToResizeFixedSizedArray                      = ECOBJECTS_ERROR_BASE + 0x27,
    ECOBJECTS_STATUS_SchemaIsImmutable                                  = ECOBJECTS_ERROR_BASE + 0x28,
    ECOBJECTS_STATUS_DynamicSchemaCustomAttributeWasNotFound            = ECOBJECTS_ERROR_BASE + 0x29,
    ECOBJECTS_STATUS_Error                                              = ECOBJECTS_ERROR_BASE + 0xFFF,
    };

//! Result status for deserializing an ECSchema from Xml
enum SchemaReadStatus
    {
    SCHEMA_READ_STATUS_Success                               = SUCCESS,
    SCHEMA_READ_STATUS_FailedToParseXml                      = SCHEMA_READ_STATUS_BASE + 0x02,
    SCHEMA_READ_STATUS_InvalidECSchemaXml                    = SCHEMA_READ_STATUS_BASE + 0x03,
    SCHEMA_READ_STATUS_ReferencedSchemaNotFound              = SCHEMA_READ_STATUS_BASE + 0x04,
    SCHEMA_READ_STATUS_DuplicateSchema                       = SCHEMA_READ_STATUS_BASE + 0x05,
    SCHEMA_READ_STATUS_InvalidPrimitiveType                  = SCHEMA_READ_STATUS_BASE + 0x06,
    SCHEMA_READ_STATUS_HasReferenceCycle                     = SCHEMA_READ_STATUS_BASE + 0x07,
    };

//! Result status for serializing an ECSchema to Xml
enum SchemaWriteStatus
    {
    SCHEMA_WRITE_STATUS_Success                                 = SUCCESS,
    SCHEMA_WRITE_STATUS_FailedToSaveXml                         = SCHEMA_WRITE_STATUS_BASE + 0x01,
    SCHEMA_WRITE_STATUS_FailedToCreateXml                       = SCHEMA_WRITE_STATUS_BASE + 0x02,
    SCHEMA_WRITE_STATUS_FailedToWriteFile                       = SCHEMA_WRITE_STATUS_BASE + 0x03,
    };

//! Result status of deserializing an IECInstance from Xml
enum InstanceReadStatus
    {
    INSTANCE_READ_STATUS_Success                             = 0,
    INSTANCE_READ_STATUS_FileNotFound                        = INSTANCE_READ_STATUS_BASE + 1,
    INSTANCE_READ_STATUS_CantCreateStream                    = INSTANCE_READ_STATUS_BASE + 2,
    INSTANCE_READ_STATUS_CantCreateXmlReader                 = INSTANCE_READ_STATUS_BASE + 3,
    INSTANCE_READ_STATUS_CantSetStream                       = INSTANCE_READ_STATUS_BASE + 4,
    INSTANCE_READ_STATUS_NoElementName                       = INSTANCE_READ_STATUS_BASE + 5,
    INSTANCE_READ_STATUS_BadElement                          = INSTANCE_READ_STATUS_BASE + 6,
    INSTANCE_READ_STATUS_UnexpectedElement                   = INSTANCE_READ_STATUS_BASE + 7,
    INSTANCE_READ_STATUS_EmptyElement                        = INSTANCE_READ_STATUS_BASE + 8,
    INSTANCE_READ_STATUS_EndElementDoesntMatch               = INSTANCE_READ_STATUS_BASE + 9,
    INSTANCE_READ_STATUS_XmlFileIncomplete                   = INSTANCE_READ_STATUS_BASE + 10,
    INSTANCE_READ_STATUS_XmlParseError                       = INSTANCE_READ_STATUS_BASE + 20,

    INSTANCE_READ_STATUS_ECClassNotFound                     = INSTANCE_READ_STATUS_BASE + 30,
    INSTANCE_READ_STATUS_BadECProperty                       = INSTANCE_READ_STATUS_BASE + 31,
    INSTANCE_READ_STATUS_BadPrimitivePropertyType            = INSTANCE_READ_STATUS_BASE + 32,
    INSTANCE_READ_STATUS_BadBinaryData                       = INSTANCE_READ_STATUS_BASE + 33,
    INSTANCE_READ_STATUS_BadTimeValue                        = INSTANCE_READ_STATUS_BASE + 34,
    INSTANCE_READ_STATUS_BadDoubleValue                      = INSTANCE_READ_STATUS_BASE + 35,
    INSTANCE_READ_STATUS_BadIntegerValue                     = INSTANCE_READ_STATUS_BASE + 36,
    INSTANCE_READ_STATUS_BadLongValue                        = INSTANCE_READ_STATUS_BASE + 37,
    INSTANCE_READ_STATUS_BadPoint2dValue                     = INSTANCE_READ_STATUS_BASE + 38,
    INSTANCE_READ_STATUS_BadPoint3dValue                     = INSTANCE_READ_STATUS_BASE + 39,
    INSTANCE_READ_STATUS_BadArrayElement                     = INSTANCE_READ_STATUS_BASE + 40,
    INSTANCE_READ_STATUS_TypeMismatch                        = INSTANCE_READ_STATUS_BASE + 41,
    INSTANCE_READ_STATUS_CantSetValue                        = INSTANCE_READ_STATUS_BASE + 42,
    INSTANCE_READ_STATUS_ECSchemaNotFound                    = INSTANCE_READ_STATUS_BASE + 43,
    INSTANCE_READ_STATUS_UnableToGetStandaloneEnabler        = INSTANCE_READ_STATUS_BASE + 44,
    INSTANCE_READ_STATUS_CommentOnly                         = INSTANCE_READ_STATUS_BASE + 45,
    INSTANCE_READ_STATUS_PropertyNotFound                    = INSTANCE_READ_STATUS_BASE + 46,
    };

//! Result status of writing an IECInstance to Xml
enum InstanceWriteStatus
    {
    INSTANCE_WRITE_STATUS_Success                               = 0,
    INSTANCE_WRITE_STATUS_CantCreateStream                      = INSTANCE_WRITE_STATUS_BASE + 1,
    INSTANCE_WRITE_STATUS_CantCreateXmlWriter                   = INSTANCE_WRITE_STATUS_BASE + 3,
    INSTANCE_WRITE_STATUS_CantSetStream                         = INSTANCE_WRITE_STATUS_BASE + 4,
    INSTANCE_WRITE_STATUS_XmlWriteError                         = INSTANCE_WRITE_STATUS_BASE + 5,
    INSTANCE_WRITE_STATUS_CantReadFromStream                    = INSTANCE_WRITE_STATUS_BASE + 6,
    INSTANCE_WRITE_STATUS_FailedToWriteFile                     = INSTANCE_WRITE_STATUS_BASE + 7,

    INSTANCE_WRITE_STATUS_BadPrimitivePropertyType              = INSTANCE_WRITE_STATUS_BASE + 30,
    };

//! Result status of trying to supplement an ECSchema
enum SupplementedSchemaStatus
    {
    SUPPLEMENTED_SCHEMA_STATUS_Success                          = 0,
    SUPPLEMENTED_SCHEMA_STATUS_Metadata_Missing                 = SUPPLEMENTED_SCHEMA_STATUS_BASE + 1,
    SUPPLEMENTED_SCHEMA_STATUS_Duplicate_Precedence_Error       = SUPPLEMENTED_SCHEMA_STATUS_BASE + 2,
    SUPPLEMENTED_SCHEMA_STATUS_IECRelationship_Not_Allowed      = SUPPLEMENTED_SCHEMA_STATUS_BASE + 3,
    SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException             = SUPPLEMENTED_SCHEMA_STATUS_BASE + 4,
    SUPPLEMENTED_SCHEMA_STATUS_SupplementalClassHasBaseClass    = SUPPLEMENTED_SCHEMA_STATUS_BASE + 5,
    };

/*---------------------------------------------------------------------------------**//**
* Options to be used when evaluating an ECExpression.
* The options are specified on an ExpressionContext. Inner contexts inherit the options
* associated with their outer context.
* @bsistruct                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
enum EvaluationOptions
    {
    //! Legacy behavior. IECTypeAdapter::_ConvertTo/FromExpressionType() will be invoked for each property value used in the ECExpression.
    //! This can have undesirable results:
    //!  -Converts linear units to dimensionsed meters
    //!  -Converts boolean and StandardValues values to localized strings
    //!  -etc.
    //! This option is not appropriate for expressions intended to be persisted.
    EVALOPT_Legacy                          = 0,

    //! IECTypeAdapter::_ConvertTo/FromExpressionType() will not be invoked for property values used in the ECExpression.
    //!  -Property values will retain their storage units
    //!  -StandardValues values will retain their internal integer values
    //!  -Boolean property values will be represented as non-localized true/false
    //! This option is recommended when units are important, or if the results of the ECExpression will not be displayed to the user directly, or
    //! if the expression will be persisted.
    EVALOPT_SuppressTypeConversions         = 1 << 0,

    //! Arithmetic, comparison, and assignment operations will check the units of their operands and:
    //!  -Convert values to compatible units if possible, or
    //!  -Produce an error if units are not compatible
    //! If units are not specified for a numeric value within the ECExpression, an attempt will be made to infer the units from the other operand.
    //! Currently, units are enforced as follows:
    //!  -Addition, subtraction, and comparison: Units must be compatible (have same base unit).
    //!  -Multiplication and division: Only one operand may have units; the other is a scalar.
    //! This option implies type conversions will be suppressed.
    EVALOPT_EnforceUnits                    = (1 << 1) | EVALOPT_SuppressTypeConversions,
    
    //! IECTypeAdapter::_ConvertTo/FromExpressionType() will be invoked only for property values used in the ECExpression 
    //! that requires evaluation based on their global representation. For example: in Microstation LevelId for the same
    //! LevelName in different files (references) can be different and evaluating against LevelId will not work. So instead
    //! LevelId should be converted to global representation and compared against it.
    //! When this option is turned on, ECExpression string should also be formated accordingly based on what global representation
    //! is for that particular property. IECTypeAdapter::_RequiresExpressionTypeConversion() and IECTypeAdapter::_ConvertToExpressionType()
    //! functions can be used to detect that.
    EVALOPT_EnforceGlobalRepresentation     = (1 << 2),
    };


/*__PUBLISH_SECTION_END__*/

//=======================================================================================
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ValueKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
//=======================================================================================
/*__PUBLISH_SECTION_START__*/
//=======================================================================================
//! Represents the classification of the data type of an ECValue. The classification is not the data type itself, but a category of type
//! such as struct, array or primitive.
//! @ingroup ECObjectsGroup
//=======================================================================================
enum ValueKind ENUM_UNDERLYING_TYPE(unsigned short)
    {
    VALUEKIND_Uninitialized                  = 0x00, //!< The ECValue has not be initialized yet
    VALUEKIND_Primitive                      = 0x01, //!< The ECValue holds a Primitive type 
    VALUEKIND_Struct                         = 0x02, //!< The ECValue holds a struct
    VALUEKIND_Array                          = 0x04, //!< The ECValue holds an array
    };

/*__PUBLISH_SECTION_END__*/
//=======================================================================================
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ArrayKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
//=======================================================================================
/*__PUBLISH_SECTION_START__*/
//=======================================================================================
//! Represents the classification of the data type of an EC array element.  The classification is not the data type itself, but a category of type.
//! Currently an ECArray can only contain primitive or struct data types.
//! @ingroup ECObjectsGroup
//=======================================================================================
enum ArrayKind ENUM_UNDERLYING_TYPE(unsigned short)
    {
    ARRAYKIND_Primitive       = 0x01,
    ARRAYKIND_Struct          = 0x02
    };

/*__PUBLISH_SECTION_END__*/
//=======================================================================================
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the PrimitiveType enum please be sure to note that the lower order byte must stay fixed as '1' and the upper order byte can be incremented.
// If you add any additional types you must update
//    - ECXML_TYPENAME_X constants
//    - PrimitiveECProperty::_GetTypeName
// NEEDSWORK types: common geometry, installed primitives
//=======================================================================================
/*__PUBLISH_SECTION_START__*/

//=======================================================================================
//! Enumeration of primitive data types for ECProperties
//! @ingroup ECObjectsGroup
//=======================================================================================
enum PrimitiveType ENUM_UNDERLYING_TYPE(unsigned short)
    {
    PRIMITIVETYPE_Binary                    = 0x101,
    PRIMITIVETYPE_Boolean                   = 0x201,
    //!Additional date time metadata can be specified on an ECProperty of this type via
    //!the custom attribute \b %DateTimeInfo defined in the schema \b Bentley_Standard_CustomAttributes.
    //!@see DateTimeInfo
    PRIMITIVETYPE_DateTime                  = 0x301,
    PRIMITIVETYPE_Double                    = 0x401,
    PRIMITIVETYPE_Integer                   = 0x501,
    PRIMITIVETYPE_Long                      = 0x601,
    PRIMITIVETYPE_Point2D                   = 0x701,
    PRIMITIVETYPE_Point3D                   = 0x801,
    PRIMITIVETYPE_String                    = 0x901,
    PRIMITIVETYPE_IGeometry                 = 0xa01,
    };

//! @ingroup ECObjectsGroup
//! Enumerates the possible return values for evaluating an expression or its value
enum ExpressionStatus
    {
    ExprStatus_Success              =   0, //!< Success
    ExprStatus_UnknownError         =   1, //!< There as an unknown error in evaluation
    ExprStatus_UnknownMember        =   2, //!< Returned if a property name in the expression cannot be found in the containing class
    ExprStatus_PrimitiveRequired    =   3, //!< Returned when a primitive is expected and not found
    ExprStatus_StructRequired       =   4, //!< Returned when a struct is expected and not found
    ExprStatus_ArrayRequired        =   5, //!< Returned when an array is expected and not found
    ExprStatus_UnknownSymbol        =   6, //!< Returned when the symbol in the expression cannot be resolved
    ExprStatus_DotNotSupported      =   7,

    //  Returning ExprStatus_NotImpl in base methods is the lazy approach for methods that should be 
    //  pure virtual.  Should be eliminated after prototyping phase is done
    ExprStatus_NotImpl              =   8,

    ExprStatus_NeedsLValue          =   9, //!< Returned when the symbol needs to be an lvalue
    ExprStatus_WrongType            =  10, //!< Returned when the symbol type is of the wrong type for the expression
    ExprStatus_IncompatibleTypes    =  11, //!< Returned when expression uses incompatible types (ie, trying to perform arithmetic on two strings)
    ExprStatus_MethodRequired       =  12, //!< Returned when a method token is expected and not found
    ExprStatus_InstanceMethodRequired =  13, //!< Returned when an instance method is called, but has not been defined
    ExprStatus_StaticMethodRequired =  14, //!< Returned when a static method is called, but has not been defined
    ExprStatus_InvalidTypesForDivision =  15, //!< Returned when the expression tries to perform a division operation on types that cannot be divided
    ExprStatus_DivideByZero             =  16, //!< Returned when the division operation tries to divide by zero
    ExprStatus_WrongNumberOfArguments   =  17, //!< Returned when the number of arguments to a method in an expression do not match the number of arguments actually expected
    ExprStatus_IndexOutOfRange          = 18, //!< Returned when array index is used which is outside the bounds of the array.
    ExprStatus_IncompatibleUnits        = 19, //!< Returned when units are combined in an unsupported manner within the expression, for example adding angles and lengths.
    };

//! Used to define how the relationship OrderId is handled.
//! @ingroup ECObjectsGroup
enum OrderIdStorageMode : uint8_t
    {
    ORDERIDSTORAGEMODE_None = 0,
    ORDERIDSTORAGEMODE_ProvidedByPersistence = 1,
    ORDERIDSTORAGEMODE_ProvidedByClient = 2,
    };

//! Used to define which end of the relationship, source or target
//! @ingroup ECObjectsGroup
enum ECRelationshipEnd
    {
    ECRelationshipEnd_Source = 0, //!< End is the source
    ECRelationshipEnd_Target  //!< End is the target
    };

//! Used to describe the direction of a related instance within the context
//! of an IECRelationshipInstance
//! @ingroup ECObjectsGroup
enum class ECRelatedInstanceDirection
    {
    //! Related instance is the target in the relationship instance
    Forward = 1,
    //! Related instance is the source in the relationship instance
    Backward = 2
    };

//! The various strengths supported on a relationship class.
//! @ingroup ECObjectsGroup
enum StrengthType
    {
    //!  'Referencing' relationships imply no ownership and no cascading deletes when the
    //! object on either end of the relationship is deleted.  For example, a document
    //! object may have a reference to the User that last modified it.
    //! This is like "Association" in UML.
    STRENGTHTYPE_Referencing,
    //! 'Holding' relationships imply shared ownership.  A given object can be "held" by
    //! many different objects, and the object will not get deleted unless all of the
    //! objects holding it are first deleted (or the relationships severed.)
    //! This is like "Aggregation" in UML.
    STRENGTHTYPE_Holding,
    //! 'Embedding' relationships imply exclusive ownership and cascading deletes.  An
    //! object that is the target of an 'embedding' relationship may also be the target
    //! of other 'referencing' relationships, but cannot be the target of any 'holding'
    //! relationships.  For examples, a Folder 'embeds' the Documents that it contains.
    //! This is like "Composition" in UML.
    STRENGTHTYPE_Embedding
    };

END_BENTLEY_ECOBJECT_NAMESPACE
