/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjects.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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
EC_TYPEDEFS(ECSchemaReadContext);
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
EC_TYPEDEFS(ECInstanceReadContext);
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

EC_TYPEDEFS(SupplementalSchemaMetaData);

typedef struct IStream* IStreamP;

BEGIN_BENTLEY_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef enum ECErrorCategories
    {
    ECOBJECTS_ERROR_BASE                    = 0x31000,
    SCHEMA_READ_STATUS_BASE      = 0x32000,
    SCHEMA_WRITE_STATUS_BASE        = 0x33000,
    INSTANCE_READ_STATUS_BASE    = 0x34000,
    INSTANCE_WRITE_STATUS_BASE      = 0x35000,
    SUPPLEMENTED_SCHEMA_STATUS_BASE = 0x36000,
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
    ECOBJECTS_STATUS_Error                                              = ECOBJECTS_ERROR_BASE + 0xFFF,
    }; 

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum SchemaReadStatus
    {
    SCHEMA_READ_STATUS_Success                               = SUCCESS,
    SCHEMA_READ_STATUS_FailedToInitializeMsmxl               = SCHEMA_READ_STATUS_BASE + 0x01,
    SCHEMA_READ_STATUS_FailedToParseXml                      = SCHEMA_READ_STATUS_BASE + 0x02,
    SCHEMA_READ_STATUS_InvalidECSchemaXml                    = SCHEMA_READ_STATUS_BASE + 0x03,
    SCHEMA_READ_STATUS_ReferencedSchemaNotFound              = SCHEMA_READ_STATUS_BASE + 0x04,
    SCHEMA_READ_STATUS_DuplicateSchema                       = SCHEMA_READ_STATUS_BASE + 0x05,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum SchemaWriteStatus
    {
    SCHEMA_WRITE_STATUS_Success                                 = SUCCESS,
    SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl                 = SCHEMA_WRITE_STATUS_BASE + 0x01,
    SCHEMA_WRITE_STATUS_FailedToSaveXml                         = SCHEMA_WRITE_STATUS_BASE + 0x02,
    SCHEMA_WRITE_STATUS_FailedToCreateXml                       = SCHEMA_WRITE_STATUS_BASE + 0x03
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
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
    INSTANCE_READ_STATUS_CantSetValue                        = INSTANCE_READ_STATUS_BASE + 41,
    INSTANCE_READ_STATUS_ECSchemaNotFound                    = INSTANCE_READ_STATUS_BASE + 42,
    INSTANCE_READ_STATUS_UnableToGetStandaloneEnabler        = INSTANCE_READ_STATUS_BASE + 43,
    INSTANCE_READ_STATUS_CommentOnly                         = INSTANCE_READ_STATUS_BASE + 44,
    };
    
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum InstanceWriteStatus
    {
    INSTANCE_WRITE_STATUS_Success                               = 0,
    INSTANCE_WRITE_STATUS_CantCreateStream                      = INSTANCE_WRITE_STATUS_BASE + 1,
    INSTANCE_WRITE_STATUS_CantCreateXmlWriter                   = INSTANCE_WRITE_STATUS_BASE + 3,
    INSTANCE_WRITE_STATUS_CantSetStream                         = INSTANCE_WRITE_STATUS_BASE + 4,
    INSTANCE_WRITE_STATUS_XmlWriteError                         = INSTANCE_WRITE_STATUS_BASE + 5,
    INSTANCE_WRITE_STATUS_CantReadFromStream                    = INSTANCE_WRITE_STATUS_BASE + 6,

    INSTANCE_WRITE_STATUS_BadPrimitivePropertyType              = INSTANCE_WRITE_STATUS_BASE + 30,
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Carole.MacDonald                04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
enum SupplementedSchemaStatus
    {
    SUPPLEMENTED_SCHEMA_STATUS_Success                          = 0,
    SUPPLEMENTED_SCHEMA_STATUS_Metadata_Missing                 = SUPPLEMENTED_SCHEMA_STATUS_BASE + 1,
    SUPPLEMENTED_SCHEMA_STATUS_Duplicate_Precedence_Error       = SUPPLEMENTED_SCHEMA_STATUS_BASE + 2,
    SUPPLEMENTED_SCHEMA_STATUS_IECRelationship_Not_Allowed      = SUPPLEMENTED_SCHEMA_STATUS_BASE + 3,
    SUPPLEMENTED_SCHEMA_STATUS_SchemaMergeException             = SUPPLEMENTED_SCHEMA_STATUS_BASE + 4,
    SUPPLEMENTED_SCHEMA_STATUS_SupplementalClassHasBaseClass    = SUPPLEMENTED_SCHEMA_STATUS_BASE + 5,
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
    PRIMITIVETYPE_String                    = 0x901,
    PRIMITIVETYPE_IGeometry                 = 0xa01,
    };

END_BENTLEY_EC_NAMESPACE

USING_NAMESPACE_BENTLEY
