/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjects.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
#define USING_NAMESPACE_BENTLEY_EC          using namespace BentleyApi::ECN;

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
EC_TYPEDEFS(ExtendedTypeECProperty);
EC_TYPEDEFS(StructArrayECProperty);
EC_TYPEDEFS(NavigationECProperty);
EC_TYPEDEFS(ECPropertyIterable);
EC_TYPEDEFS(ECClassContainer);
EC_TYPEDEFS(ECClass);
EC_TYPEDEFS(ECEnumeration);
EC_TYPEDEFS(ECEnumerator);
EC_TYPEDEFS(ECEnumerationContainer);
EC_TYPEDEFS(KindOfQuantity);
EC_TYPEDEFS(KindOfQuantityContainer);
EC_TYPEDEFS(ECEntityClass);
EC_TYPEDEFS(ECCustomAttributeClass);
EC_TYPEDEFS(ECStructClass);
EC_TYPEDEFS(ECRelationshipConstraintClass);
EC_TYPEDEFS(ECRelationshipClass);
EC_TYPEDEFS(ECRelationshipConstraint);
EC_TYPEDEFS(RelationshipMultiplicity);
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
EC_TYPEDEFS (ICustomAttributeDeserializer);
EC_TYPEDEFS (CustomAttributeDeserializerManager);
EC_TYPEDEFS (ECInstanceIterable);
EC_TYPEDEFS(SupplementalSchemaMetaData);
EC_TYPEDEFS (PresentationMetadataHelper);
EC_TYPEDEFS(ECSchemaConverter);
EC_TYPEDEFS(IECCustomAttributeConverter);

EC_TYPEDEFS (CalculatedPropertySpecification);
EC_TYPEDEFS (ParserRegex);
EC_TYPEDEFS(IECTypeAdapterContext);
EC_TYPEDEFS(IECSchemaRemapper);

EC_TYPEDEFS (SchemaNameClassNamePair);
EC_TYPEDEFS (UnitSpec);
EC_TYPEDEFS (Unit);
EC_TYPEDEFS (UnitConverter);
EC_TYPEDEFS(AdHocJsonContainer);
EC_TYPEDEFS(AdHocJsonPropertyValue);

//__PUBLISH_SECTION_END__
EC_TYPEDEFS (AdhocPropertyMetadata);
EC_TYPEDEFS (AdhocPropertyQuery);
EC_TYPEDEFS (AdhocPropertyEdit);
//__PUBLISH_SECTION_START__

struct IStream;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! @addtogroup ECObjectsGroup
//! @beginGroup

typedef struct IStream* IStreamP;

//! General purpose result codes
enum class ECObjectsStatus
    {
    Success,
    PropertyNotFound,
    DataTypeMismatch,
    ECInstanceImplementationNotSupported,
    InvalidPropertyAccessString,
    IndexOutOfRange,
    ECClassNotSupported,
    ECSchemaNotSupported,
    AccessStringDisagreesWithNIndices,
    EnablerNotFound,
    OperationNotSupported,
    ParseError,
    NamedItemAlreadyExists,
    PreconditionViolated,
    SchemaNotFound,
    ClassNotFound,
    EnumerationNotFound,
    EnumeratorNotFound,
    BaseClassUnacceptable,
    SchemaInUse,
    InvalidName,
    DataTypeNotSupported,
    UnableToAllocateMemory,
    MemoryBoundsOverrun,
    NullPointerValue,
    NotCustomAttributeClass,
    DuplicateSchema,
    UnableToSetReadOnlyInstance,
    UnableToSetReadOnlyProperty,
    ArrayIndexDoesNotExist,
    PropertyValueMatchesNoChange,
    NoChildProperties,
    UnableToAllocateManagedMemory,
    MemoryAllocationCallbackRequired,
    UnableToAddStructArrayMember,
    UnableToSetStructArrayMemberInstance,
    UnableToGetStructArrayMemberInstance,
    InvalidIndexForPerPropertyFlag,
    SchemaHasReferenceCycle,
    SchemaNotSupplemented,
    UnableToQueryForNullPropertyFlag,
    UnableToResizeFixedSizedArray,
    SchemaIsImmutable,
    DynamicSchemaCustomAttributeWasNotFound,
    Error,
    RelationshipConstraintsNotCompatible,
    CaseCollision,
    CustomAttributeContainerTypesNotCompatible,
    };

//! Result status for deserializing an ECSchema from Xml
enum class SchemaReadStatus
    {
    Success,
    FailedToParseXml,
    InvalidECSchemaXml,
    ReferencedSchemaNotFound,
    DuplicateSchema,
    DuplicateTypeName,
    InvalidPrimitiveType,
    HasReferenceCycle,
    };

//! Result status for serializing an ECSchema to Xml
enum class SchemaWriteStatus
    {
    Success,
    FailedToSaveXml,
    FailedToCreateXml,
    FailedToWriteFile,
    };

//! Result status for the deserialization of custom attributes applied to a custom attribute container
enum class CustomAttributeReadStatus
    {
    //! Successfully deserialized and applied all custom attributes for the container
    Success,
    //! One or more custom attributes skipped, non fatal error
    SkippedCustomAttributes,
    //! One or more custom attributes found which were invalid for this container because their CustomAttributeContainerType did not match the actual container type, fatal error
    InvalidCustomAttributes,
    };

//! Result status of deserializing an IECInstance from Xml
enum class InstanceReadStatus
    {
    Success,
    FileNotFound,
    CantCreateStream,
    CantCreateXmlReader,
    CantSetStream,
    NoElementName,
    BadElement,
    UnexpectedElement,
    EmptyElement,
    EndElementDoesntMatch,
    XmlFileIncomplete,
    XmlParseError,

    ECClassNotFound,
    BadECProperty,
    BadPrimitivePropertyType,
    BadBinaryData,
    BadTimeValue,
    BadDoubleValue,
    BadIntegerValue,
    BadLongValue,
    BadPoint2dValue,
    BadPoint3dValue,
    BadArrayElement,
    TypeMismatch,
    CantSetValue,
    ECSchemaNotFound,
    UnableToGetStandaloneEnabler,
    CommentOnly,
    PropertyNotFound,
    };

//! Result status of writing an IECInstance to Xml
enum class InstanceWriteStatus
    {
    Success,
    CantCreateStream,
    CantCreateXmlWriter,
    CantSetStream,
    XmlWriteError,
    CantReadFromStream,
    FailedToWriteFile,

    BadPrimitivePropertyType,
    };

//! Result status of trying to supplement an ECSchema
enum class SupplementedSchemaStatus
    {
    Success,
    Metadata_Missing,
    Duplicate_Precedence_Error,
    IECRelationship_Not_Allowed,
    SchemaMergeException,
    SupplementalClassHasBaseClass,
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
    //!  @li Converts linear units to dimensionsed meters
    //!  @li Converts boolean and StandardValues values to localized strings
    //!  @li etc.
    //! This option is not appropriate for expressions intended to be persisted.
    EVALOPT_Legacy                          = 0,

    //! IECTypeAdapter::_ConvertTo/FromExpressionType() will not be invoked for property values used in the ECExpression.
    //!  @li Property values will retain their storage units
    //!  @li StandardValues values will retain their internal integer values
    //!  @li Boolean property values will be represented as non-localized true/false
    //! This option is recommended when units are important, or if the results of the ECExpression will not be displayed to the user directly, or
    //! if the expression will be persisted.
    EVALOPT_SuppressTypeConversions         = 1 << 0,

    //! Arithmetic, comparison, and assignment operations will check the units of their operands and:
    //!  @li Convert values to compatible units if possible, or
    //!  @li Produce an error if units are not compatible
    //! If units are not specified for a numeric value within the ECExpression, an attempt will be made to infer the units from the other operand.
    //! Currently, units are enforced as follows:
    //!  @li Addition, subtraction, and comparison: Units must be compatible (have same base unit).
    //!  @li Multiplication and division: Only one operand may have units; the other is a scalar.
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

//! Enumerates the possible return values for evaluating an expression or its value
enum class ExpressionStatus
    {
    Success, //!< Success
    UnknownError, //!< There as an unknown error in evaluation
    UnknownMember, //!< Returned if a property name in the expression cannot be found in the containing class
    PrimitiveRequired, //!< Returned when a primitive is expected and not found
    StructRequired, //!< Returned when a struct is expected and not found
    ArrayRequired, //!< Returned when an array is expected and not found
    UnknownSymbol, //!< Returned when the symbol in the expression cannot be resolved
    DotNotSupported,

    //  Returning ExpressionStatus::NotImpl in base methods is the lazy approach for methods that should be 
    //  pure virtual.  Should be eliminated after prototyping phase is done
    NotImpl,

    NeedsLValue, //!< Returned when the symbol needs to be an lvalue
    WrongType, //!< Returned when the symbol type is of the wrong type for the expression
    IncompatibleTypes, //!< Returned when expression uses incompatible types (ie, trying to perform arithmetic on two strings)
    MethodRequired, //!< Returned when a method token is expected and not found
    InstanceMethodRequired, //!< Returned when an instance method is called, but has not been defined
    StaticMethodRequired, //!< Returned when a static method is called, but has not been defined
    InvalidTypesForDivision, //!< Returned when the expression tries to perform a division operation on types that cannot be divided
    DivideByZero, //!< Returned when the division operation tries to divide by zero
    WrongNumberOfArguments, //!< Returned when the number of arguments to a method in an expression do not match the number of arguments actually expected
    IndexOutOfRange, //!< Returned when array index is used which is outside the bounds of the array.
    IncompatibleUnits, //!< Returned when units are combined in an unsupported manner within the expression, for example adding angles and lengths.
    };

//! Used to define how the relationship OrderId is handled.
enum OrderIdStorageMode : uint8_t
    {
    ORDERIDSTORAGEMODE_None = 0,
    ORDERIDSTORAGEMODE_ProvidedByPersistence = 1,
    ORDERIDSTORAGEMODE_ProvidedByClient = 2,
    };

//! Used to define which end of the relationship, source or target
enum ECRelationshipEnd
    {
    ECRelationshipEnd_Source = 0, //!< End is the source
    ECRelationshipEnd_Target  //!< End is the target
    };

//! Used to describe the direction of a related instance within the context
//! of an IECRelationshipInstance
enum class ECRelatedInstanceDirection
    {
    //! Related instance is the target in the relationship instance
    Forward = 1,
    //! Related instance is the source in the relationship instance
    Backward = 2
    };

//! The various strengths supported on a relationship class.
enum class StrengthType
    {
    //!  'Referencing' relationships imply no ownership and no cascading deletes when the
    //! object on either end of the relationship is deleted.  For example, a document
    //! object may have a reference to the User that last modified it.
    //! This is like "Association" in UML.
    Referencing,
    //! 'Holding' relationships imply shared ownership.  A given object can be "held" by
    //! many different objects, and the object will not get deleted unless all of the
    //! objects holding it are first deleted (or the relationships severed.)
    //! This is like "Aggregation" in UML.
    Holding,
    //! 'Embedding' relationships imply exclusive ownership and cascading deletes.  An
    //! object that is the target of an 'embedding' relationship may also be the target
    //! of other 'referencing' relationships, but cannot be the target of any 'holding'
    //! relationships.  For examples, a Folder 'embeds' the Documents that it contains.
    //! This is like "Composition" in UML.
    Embedding
    };

//! Used to describe the attribute of an ECClass
enum class ECClassModifier
    {
    //! No modifiers
    None,
    //! Class is abstract and may not have instances contructed
    Abstract,
    //! Class is sealed and may not be used as a base class
    Sealed
    };

//! Used to define what type of IECCustomAttributeContainer this CustomAttribute can be applied to
enum class CustomAttributeContainerType
    {
    Schema                  = (0x0001 << 0),
    EntityClass             = (0x0001 << 1),
    CustomAttributeClass    = (0x0001 << 2),
    StructClass             = (0x0001 << 3),
    RelationshipClass       = (0x0001 << 4),
    AnyClass                = EntityClass | CustomAttributeClass | StructClass | RelationshipClass,
    PrimitiveProperty       = (0x0001 << 5),
    StructProperty          = (0x0001 << 6),
    ArrayProperty           = (0x0001 << 7),
    StructArrayProperty     = (0x0001 << 8),
    NavigationProperty      = (0x0001 << 9),
    AnyProperty             = PrimitiveProperty | StructProperty | ArrayProperty | StructArrayProperty | NavigationProperty,
    SourceRelationshipConstraint    = (0x0001 << 10),
    TargetRelationshipConstraint    = (0x0001 << 11),
    AnyRelationshipConstraint       = SourceRelationshipConstraint | TargetRelationshipConstraint,
    Any                     = Schema | AnyClass | AnyProperty | AnyRelationshipConstraint
    };

ENUM_IS_FLAGS(CustomAttributeContainerType)
/** @endGroup */

END_BENTLEY_ECOBJECT_NAMESPACE
