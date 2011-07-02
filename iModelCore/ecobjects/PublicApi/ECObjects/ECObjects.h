/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjects.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#define NO_USING_NAMESPACE_BENTLEY 1

/*__PUBLISH_SECTION_START__*/

#include <Bentley\Bentley.h>

// In many of the DgnPlatform libraries we redefine the below macros based on __cplusplus.  This is because there
// are existing C callers that we can not get rid of.  I've spoken to Sam and he recommends that for any new libraries we
// ONLY support cpp callers and therefore do not repeat this pattern.

#ifdef __ECOBJECTS_BUILD__
#define ECOBJECTS_EXPORT EXPORT_ATTRIBUTE
#else
#define ECOBJECTS_EXPORT IMPORT_ATTRIBUTE
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

/*__PUBLISH_SECTION_END__*/
// These BENTLEY_EXCLUDE_WINDOWS_HEADERS shenanigans are necessary to allow ECObjects headers to be included without sucking in conflicting windows headers
// and to help us split off the non-portable code and to move to a different XML parser
#ifdef BENTLEY_EXCLUDE_WINDOWS_HEADERS
/*__PUBLISH_SECTION_START__*/        
    #define MSXML2_IXMLDOMNode      void *
    #define MSXML2_IXMLDOMNodePtr   void *
    #define MSXML2_IXMLDOMDocument2 void *
    #define MSXML2_IXMLDOMElement   void *
/*__PUBLISH_SECTION_END__*/
#else
    #define MSXML2_IXMLDOMNode      MSXML2::IXMLDOMNode
    #define MSXML2_IXMLDOMNodePtr   MSXML2::IXMLDOMNodePtr
    #define MSXML2_IXMLDOMDocument2 MSXML2::IXMLDOMDocument2
    #define MSXML2_IXMLDOMElement   MSXML2::IXMLDOMElement
#endif
/*__PUBLISH_SECTION_START__*/

EC_TYPEDEFS(ECValue);
EC_TYPEDEFS(ECValueAccessor);
EC_TYPEDEFS(ECValueAccessorPair);
EC_TYPEDEFS(ECValueAccessorPairCollection);
EC_TYPEDEFS(ECValueAccessorPairCollectionOptions);
EC_TYPEDEFS(ECValuesCollection);
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
EC_TYPEDEFS(IECSchemaOwner);
EC_TYPEDEFS(IECSchemaLocater);
EC_TYPEDEFS(IECCustomAttributeContainer);
EC_TYPEDEFS(ECInstanceDeserializationContext);
EC_TYPEDEFS(ECSchemaCache);
EC_TYPEDEFS(ECPropertyValue);
EC_TYPEDEFS(IECWipRelationshipInstance);
EC_TYPEDEFS(ECRelationshipInstanceHolder);

EC_TYPEDEFS(ECEnabler);
EC_TYPEDEFS(IStandaloneEnablerLocater);
EC_TYPEDEFS(IArrayManipulator);

EC_TYPEDEFS(SchemaLayout);
EC_TYPEDEFS(ClassLayout);
EC_TYPEDEFS(PropertyLayout);
EC_TYPEDEFS(StandaloneECInstance);
EC_TYPEDEFS(MemoryECInstanceBase);
EC_TYPEDEFS(ClassLayoutHolder);
EC_TYPEDEFS(SystemTime);

EC_TYPEDEFS(ICustomECStructSerializer);
EC_TYPEDEFS(CustomStructSerializerManager);

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
    ECOBJECTS_STATUS_DuplicateSchema                                    = ECOBJECTS_ERROR_BASE + 0x18,
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
    SCHEMA_DESERIALIZATION_STATUS_DuplicateSchema                       = SCHEMA_DESERIALIZATION_STATUS_BASE + 0x05,
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
    INSTANCE_DESERIALIZATION_STATUS_ECSchemaNotFound                    = INSTANCE_DESERIALIZATION_STATUS_BASE + 42,
    INSTANCE_DESERIALIZATION_STATUS_UnableToGetStandaloneEnabler        = INSTANCE_DESERIALIZATION_STATUS_BASE + 43,
    INSTANCE_DESERIALIZATION_STATUS_CommentOnly                         = INSTANCE_DESERIALIZATION_STATUS_BASE + 44,
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
    
/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
 @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct          ILeakDetector
{
virtual void    GetStats(Int32& currentLive, Int32& totalAllocs, Int32& totalFrees) const = 0;
virtual void    ResetStats() = 0;
virtual void    ReportStats (WCharCP prefix) const = 0;
virtual Int32   CheckForLeaks () const = 0;
};

//=======================================================================================    
// ValueKind, ArrayKind & Primitivetype enums are 16-bit types but the intention is that the values are defined in such a way so that when 
// ValueKind or ArrayKind is necessary, we can union PrimitiveType in the same 16-bit memory location and get some synergy between the two.
// If you add more values to the ValueKind enum please be sure to note that these are bit flags and not incremental values.  Also be sure the value does not
// exceed a single byte.
//=======================================================================================    
/*__PUBLISH_SECTION_START__*/
//=======================================================================================    
//! Represents the classification of the data type of an EC ECValue.  The classification is not the data type itself, but a category of type
//! such as struct, array or primitive.
//=======================================================================================    
enum ValueKind ENUM_UNDERLYING_TYPE(unsigned short)
    {
    VALUEKIND_Uninitialized                  = 0x00,
    VALUEKIND_Primitive                      = 0x01,
    VALUEKIND_Struct                         = 0x02,
    VALUEKIND_Array                          = 0x04,
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
//! Enumeration of primitive datatypes supported by native "ECObjects" implementation.
//! These should correspond to all of the datatypes supported in .NET ECObjects
//=======================================================================================    
enum PrimitiveType ENUM_UNDERLYING_TYPE(unsigned short)
    {
    PRIMITIVETYPE_Binary                    = 0x101,
    PRIMITIVETYPE_Boolean                   = 0x201,
    PRIMITIVETYPE_DateTime                  = 0x301,
    PRIMITIVETYPE_Double                    = 0x401,
    PRIMITIVETYPE_Integer                   = 0x501,
    PRIMITIVETYPE_Long                      = 0x601,
    PRIMITIVETYPE_Point2D                   = 0x701,
    PRIMITIVETYPE_Point3D                   = 0x801,
    PRIMITIVETYPE_String                    = 0x901
    };

END_BENTLEY_EC_NAMESPACE

USING_NAMESPACE_BENTLEY
