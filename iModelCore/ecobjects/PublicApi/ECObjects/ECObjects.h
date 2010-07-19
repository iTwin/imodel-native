/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjects.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#define NO_USING_NAMESPACE_BENTLEY 1

/*__PUBLISH_SECTION_START__*/

#include <Bentley\Bentley.h>

/*__PUBLISH_SECTION_END__*/
// In many of the DgnPlatform libraries we redefine the below macros based on __cplusplus.  This is because there
// are existing C callers that we can not get rid of.  I've spoken to Sam and he recommends that for any new libraries we
// ONLY support cpp callers and therefore do not repeat this pattern.
/*__PUBLISH_SECTION_START__*/

#ifdef __ECOBJECTS_BUILD__
#define ECOBJECTS_EXPORT __declspec(dllexport)
#else
#define ECOBJECTS_EXPORT __declspec(dllimport)
#endif

#define BEGIN_BENTLEY_EC_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace EC {

#define END_BENTLEY_EC_NAMESPACE    }}

#define USING_NAMESPACE_EC  using namespace Bentley::EC;

#define EC_TYPEDEFS(_name_)  \
        BEGIN_BENTLEY_EC_NAMESPACE      \
            struct _name_;      \
            typedef _name_ *         _name_##P;  \
            typedef _name_ &         _name_##R;  \
            typedef _name_ const*    _name_##CP; \
            typedef _name_ const&    _name_##CR; \
        END_BENTLEY_EC_NAMESPACE


EC_TYPEDEFS(ECValue);
EC_TYPEDEFS(ArrayInfo);
EC_TYPEDEFS(ECSchema);
EC_TYPEDEFS(ECSchemaDeserializationContext);
EC_TYPEDEFS(ECProperty);
EC_TYPEDEFS(PrimitiveECProperty);
EC_TYPEDEFS(StructECProperty);
EC_TYPEDEFS(ArrayECProperty);
EC_TYPEDEFS(ECPropertyIterable);
EC_TYPEDEFS(ECClassContainer);
EC_TYPEDEFS(ECClass);
EC_TYPEDEFS(ECRelationshipClass);
EC_TYPEDEFS(ECRelationshipConstraint);
EC_TYPEDEFS(RelationshipCardinality);
EC_TYPEDEFS(IECInstance);
EC_TYPEDEFS(IECRelationshipInstance);
EC_TYPEDEFS(ECSchemaOwner);
EC_TYPEDEFS(IECSchemaOwner);
EC_TYPEDEFS(IECSchemaLocator);
EC_TYPEDEFS(IECCustomAttributeContainer);

EC_TYPEDEFS(ECEnabler);
EC_TYPEDEFS(IArrayManipulator);

EC_TYPEDEFS(SchemaLayout);
EC_TYPEDEFS(ClassLayout);
EC_TYPEDEFS(PropertyLayout);
EC_TYPEDEFS(StandaloneECInstance);
EC_TYPEDEFS(MemoryECInstanceBase);
EC_TYPEDEFS(ClassLayoutHolder);
EC_TYPEDEFS(SystemTime);

#define EXPORTED_PROPERTY(TYPE, NAME) \
    ECOBJECTS_EXPORT TYPE Get##NAME() const; \
    ECOBJECTS_EXPORT ECObjectsStatus Set##NAME (TYPE value); \
    __declspec(property(get=Get##NAME,put=Set##NAME)) TYPE NAME

#define READONLY_PROPERTY(TYPE, NAME) TYPE Get##NAME() const; \
    __declspec(property(get=Get##NAME)) TYPE NAME;

#define EXPORTED_READONLY_PROPERTY(TYPE, NAME) \
    ECOBJECTS_EXPORT TYPE Get##NAME() const; \
    __declspec(property(get=Get##NAME)) TYPE NAME;

#define WRITEONLY_PROPERTY(TYPE, NAME) __declspec(property(put=Set##NAME)) TYPE NAME

typedef struct IStream* IStreamP;

BEGIN_BENTLEY_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef enum ECErrorCategories
    {
    ECOBJECTS_ERROR_BASE                    = 0x31000,
    SCHEMA_DESERIALIZATION_STATUS_BASE      = 0x32000,
    SCHEMA_SERIALIZATION_STATUS_BASE        = 0x33000,
    INSTANCE_DESERIALIZATION_STATUS_BASE    = 0x34000,
    INSTANCE_SERIALIZATION_STATUS_BASE      = 0x35000,
    } ECErrorCategories;


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
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
    ECOBJECTS_STATUS_Error                                              = ECOBJECTS_ERROR_BASE + 0xFFF,
    }; 

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum SchemaDeserializationStatus
    {
    SCHEMA_DESERIALIZATION_STATUS_Success                               = SUCCESS,
    SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl               = SCHEMA_DESERIALIZATION_STATUS_BASE + 0x01,
    SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml                      = SCHEMA_DESERIALIZATION_STATUS_BASE + 0x02,
    SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml                    = SCHEMA_DESERIALIZATION_STATUS_BASE + 0x03,
    SCHEMA_DESERIALIZATION_STATUS_ReferencedSchemaNotFound              = SCHEMA_DESERIALIZATION_STATUS_BASE + 0x04,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum SchemaSerializationStatus
    {
    SCHEMA_SERIALIZATION_STATUS_Success                                 = SUCCESS,
    SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl                 = SCHEMA_SERIALIZATION_STATUS_BASE + 0x01,
    SCHEMA_SERIALIZATION_STATUS_FailedToSaveXml                         = SCHEMA_SERIALIZATION_STATUS_BASE + 0x02,
    SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml                       = SCHEMA_SERIALIZATION_STATUS_BASE + 0x03
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum InstanceDeserializationStatus
    {
    INSTANCE_DESERIALIZATION_STATUS_Success                             = 0,
    INSTANCE_DESERIALIZATION_STATUS_FileNotFound                        = INSTANCE_DESERIALIZATION_STATUS_BASE + 1,
    INSTANCE_DESERIALIZATION_STATUS_CantCreateStream                    = INSTANCE_DESERIALIZATION_STATUS_BASE + 2,
    INSTANCE_DESERIALIZATION_STATUS_CantCreateXmlReader                 = INSTANCE_DESERIALIZATION_STATUS_BASE + 3,
    INSTANCE_DESERIALIZATION_STATUS_CantSetStream                       = INSTANCE_DESERIALIZATION_STATUS_BASE + 4,
    INSTANCE_DESERIALIZATION_STATUS_NoElementName                       = INSTANCE_DESERIALIZATION_STATUS_BASE + 5,
    INSTANCE_DESERIALIZATION_STATUS_BadElement                          = INSTANCE_DESERIALIZATION_STATUS_BASE + 6,
    INSTANCE_DESERIALIZATION_STATUS_UnexpectedElement                   = INSTANCE_DESERIALIZATION_STATUS_BASE + 7,
    INSTANCE_DESERIALIZATION_STATUS_EmptyElement                        = INSTANCE_DESERIALIZATION_STATUS_BASE + 8,
    INSTANCE_DESERIALIZATION_STATUS_EndElementDoesntMatch               = INSTANCE_DESERIALIZATION_STATUS_BASE + 9,
    INSTANCE_DESERIALIZATION_STATUS_XmlFileIncomplete                   = INSTANCE_DESERIALIZATION_STATUS_BASE + 10,
    INSTANCE_DESERIALIZATION_STATUS_XmlParseError                       = INSTANCE_DESERIALIZATION_STATUS_BASE + 20,

    INSTANCE_DESERIALIZATION_STATUS_ECClassNotFound                     = INSTANCE_DESERIALIZATION_STATUS_BASE + 30,
    INSTANCE_DESERIALIZATION_STATUS_BadECProperty                       = INSTANCE_DESERIALIZATION_STATUS_BASE + 31,
    INSTANCE_DESERIALIZATION_STATUS_BadPrimitivePropertyType            = INSTANCE_DESERIALIZATION_STATUS_BASE + 32,
    INSTANCE_DESERIALIZATION_STATUS_BadBinaryData                       = INSTANCE_DESERIALIZATION_STATUS_BASE + 33,
    INSTANCE_DESERIALIZATION_STATUS_BadTimeValue                        = INSTANCE_DESERIALIZATION_STATUS_BASE + 34,
    INSTANCE_DESERIALIZATION_STATUS_BadDoubleValue                      = INSTANCE_DESERIALIZATION_STATUS_BASE + 35,
    INSTANCE_DESERIALIZATION_STATUS_BadIntegerValue                     = INSTANCE_DESERIALIZATION_STATUS_BASE + 36,
    INSTANCE_DESERIALIZATION_STATUS_BadLongValue                        = INSTANCE_DESERIALIZATION_STATUS_BASE + 37,
    INSTANCE_DESERIALIZATION_STATUS_BadPoint2dValue                     = INSTANCE_DESERIALIZATION_STATUS_BASE + 38,
    INSTANCE_DESERIALIZATION_STATUS_BadPoint3dValue                     = INSTANCE_DESERIALIZATION_STATUS_BASE + 39,
    INSTANCE_DESERIALIZATION_STATUS_BadArrayElement                     = INSTANCE_DESERIALIZATION_STATUS_BASE + 40,
    INSTANCE_DESERIALIZATION_STATUS_CantSetValue                        = INSTANCE_DESERIALIZATION_STATUS_BASE + 41,
    };
    
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum InstanceSerializationStatus
    {
    INSTANCE_SERIALIZATION_STATUS_Success                               = 0,
    INSTANCE_SERIALIZATION_STATUS_CantCreateStream                      = INSTANCE_SERIALIZATION_STATUS_BASE + 1,
    INSTANCE_SERIALIZATION_STATUS_CantCreateXmlWriter                   = INSTANCE_SERIALIZATION_STATUS_BASE + 3,
    INSTANCE_SERIALIZATION_STATUS_CantSetStream                         = INSTANCE_SERIALIZATION_STATUS_BASE + 4,
    INSTANCE_SERIALIZATION_STATUS_XmlWriteError                         = INSTANCE_SERIALIZATION_STATUS_BASE + 5,
    INSTANCE_SERIALIZATION_STATUS_CantReadFromStream                    = INSTANCE_SERIALIZATION_STATUS_BASE + 6,

    INSTANCE_SERIALIZATION_STATUS_BadPrimitivePropertyType              = INSTANCE_SERIALIZATION_STATUS_BASE + 30,
    };
    
END_BENTLEY_EC_NAMESPACE

USING_NAMESPACE_BENTLEY
