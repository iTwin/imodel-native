/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECObjects.h $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define NO_USING_NAMESPACE_BENTLEY 1
#include <Bentley\Bentley.h>

#ifdef __ECOBJECTS_BUILD__
#define ECOBJECTS_EXPORT __declspec(dllexport)
#else
#define ECOBJECTS_EXPORT __declspec(dllimport)
#endif
  
/*__PUBLISH_SECTION_END__*/
// In many of the DgnPlatform libraries we redefine the below macros based on __cplusplus.  This is because there
// are existing C callers that we can not get rid of.  I've spoken to Sam and he recommends that for any new libraries we
// ONLY support cpp callers and therefore do not repeat this pattern.
/*__PUBLISH_SECTION_START__*/

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


EC_TYPEDEFS(Value);
EC_TYPEDEFS(ArrayInfo);
EC_TYPEDEFS(Schema);
EC_TYPEDEFS(Property);
EC_TYPEDEFS(Class);
EC_TYPEDEFS(RelationshipClass);
EC_TYPEDEFS(Instance);
EC_TYPEDEFS(RelationshipInstance);

EC_TYPEDEFS(Enabler);
EC_TYPEDEFS(IGetValue);
EC_TYPEDEFS(ISetValue);
EC_TYPEDEFS(IArrayManipulator);
EC_TYPEDEFS(ICreateInstance);

#if !defined(ECAssert)
    #define ECAssert(expression) \
        if (!(expression)) \
             __asm int 3;
#endif    

USING_NAMESPACE_BENTLEY

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef enum ECErrorCategories
    {
    ECOBJECTS_ERROR_BASE        = 0x31000
    } ECErrorCategories;


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum ECObjectsStatus
    {
    ECOBJECTS_STATUS_Success                                           = SUCCESS,
    ECOBJECTS_STATUS_PropertyNotFound                                  = ECOBJECTS_ERROR_BASE + 0x01,
    ECOBJECTS_STATUS_DataTypeMismatch                                  = ECOBJECTS_ERROR_BASE + 0x02,
    ECOBJECTS_STATUS_ECInstanceImplementationNotSupported              = ECOBJECTS_ERROR_BASE + 0x03,
    ECOBJECTS_STATUS_InvalidPropertyAccessString                       = ECOBJECTS_ERROR_BASE + 0x04,
    ECOBJECTS_STATUS_IndexOutOfRange                                   = ECOBJECTS_ERROR_BASE + 0x05,
    ECOBJECTS_STATUS_ECClassNotSupported                               = ECOBJECTS_ERROR_BASE + 0x06,
    ECOBJECTS_STATUS_ECSchemaNotSupported                              = ECOBJECTS_ERROR_BASE + 0x07,
    ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices                 = ECOBJECTS_ERROR_BASE + 0x08,
    ECOBJECTS_STATUS_EnablerNotFound                                   = ECOBJECTS_ERROR_BASE + 0x09,
    ECOBJECTS_STATUS_OperationNotSupportedByEnabler                    = ECOBJECTS_ERROR_BASE + 0x0A
    }; 

