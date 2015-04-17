/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/IEcPropertyHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IEcPropertyHandler::_IsPropertyReadOnly (ElementHandleCR eh, uint32_t, size_t)
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IEcPropertyHandler::_IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName)
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JoshSchifter                    02/11
+---------------+---------------+---------------+---------------+---------------+------*/
IsNullReturnType   IEcPropertyHandler::_IsNullProperty (ElementHandleCR eh, uint32_t propId, size_t arrayIndex)
    {
    return ISNULLRETURN_NotNull;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void IEcPropertyHandler::EcValueAccessor::Clear ()
    {
    m_get = m_set = m_toString = m_fromString = NULL;
    m_getIntStringPairs = NULL;
    m_subType = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor ()
    {
    Clear ();
    m_type = VALUETYPE_Uninitialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_StringValueGet* g,
T_StringValueSet* s
)
    {
    Clear ();
    m_type = VALUETYPE_String;
    m_get = (void*)g;
    m_set = (void*)s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_IntValueGet*        g,
T_IntValueSet*        s,
T_IntValueToString*   ts,
T_IntValueFromString* fs
)   {
    Clear ();
    m_type = VALUETYPE_Int;
    m_get = (void*)g;
    m_set = (void*)s;
    m_toString   = (void*)ts;
    m_fromString = (void*)fs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_IntValueGet*        g,
T_IntValueSet*        s,
StandardIntType       st
)   {
    Clear ();
    m_type = VALUETYPE_Int;
    m_subType = st;
    m_get = (void*)g;
    m_set = (void*)s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_IntValueGet*        g,
T_IntValueSet*        s,
T_GetIntStringPairs*  gs
)
    {
    Clear ();
    m_type = VALUETYPE_StringList;
    m_get = (void*)g;
    m_set = (void*)s;
    m_getIntStringPairs = gs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_DoubleValueGet*        g,
T_DoubleValueSet*        s,
T_DoubleValueToString*   ts,
T_DoubleValueFromString* fs
)
    {
    Clear ();
    m_type = VALUETYPE_Double;
    m_get = (void*)g;
    m_set = (void*)s;
    m_toString   = (void*)ts;
    m_fromString = (void*)fs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_DoubleValueGet* g,
T_DoubleValueSet* s,
StandardDoubleType st
)
    {
    Clear ();
    m_type = VALUETYPE_Double;
    m_subType = st;
    m_get = (void*)g;
    m_set = (void*)s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_BooleanValueGet*        g,
T_BooleanValueSet*        s,
T_BooleanValueToString*   ts
)
    {
    Clear ();
    m_type = VALUETYPE_Boolean;
    m_get = (void*)g;
    m_set = (void*)s;
    m_toString   = (void*)ts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_BooleanValueGet*        g,
T_BooleanValueSet*        s,
StandardBooleanType       st
)
    {
    Clear ();
    m_type = VALUETYPE_Boolean;
    m_subType = st;
    m_get = (void*)g;
    m_set = (void*)s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_DPoint3dValueGet*        g,
T_DPoint3dValueSet*        s,
StandardDPoint3dType       st
)
    {
    Clear ();
    m_type = VALUETYPE_DPoint3d;
    m_subType = st;
    m_get = (void*)g;
    m_set = (void*)s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mukesh.Pant                     06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcValueAccessor::EcValueAccessor
(
T_NamedScaleValueGet *      g,
T_NamedScaleValueSet *      s
)
    {
    Clear ();
    m_type = VALUETYPE_Scale;
    m_get = (void*)g;
    m_set = (void*)s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcPropertyDescriptor::EcPropertyDescriptor ()
    :
    m_propID (0),
    m_priority (0),
    m_struct (0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcPropertyDescriptor::EcPropertyDescriptor ( EcValueAccessor const& va, uint32_t propId, Bentley::WString const& name, Bentley::WString const& displayName, uint32_t pri)
    :
    m_propID (propId),
    m_name (name),
    m_displayName (displayName.c_str()),
    m_value (va),
    m_priority (pri),
    m_struct (0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcPropertyDescriptor::EcPropertyDescriptor ( EcArrayValueAccessor const& va, uint32_t propId, Bentley::WString const& name, Bentley::WString const& displayName, uint32_t pri)
    :
    m_propID (propId),
    m_name (name),
    m_displayName (displayName.c_str()),
    m_priority (pri),
    m_struct (0)
    {
    m_value = EcValueAccessor ((EcValueAccessor::T_IntValueGet*)va.m_getCount, NULL);
    m_value.m_type = EcValueAccessor::VALUETYPE_Array;
    m_underlying = va.m_underlying;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcPropertyDescriptor::EcPropertyDescriptor ( IRefCountedEcPropertyHandler* s, Bentley::WString const& name, Bentley::WString const& displayName, uint32_t pri)
    :
    m_propID ((uint32_t)-1),
    m_name (name),
    m_displayName (displayName.c_str()),
    m_priority (pri),
    m_struct (s)
    {
    s->AddRef ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcPropertyCategory::EcPropertyCategory
(
IEcPropertyHandler::EcPropertyCategory::StandardId  i
)   :
    m_standardId (i),
    m_name (L""),
    m_description (L""),
    m_priority (0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2007
+---------------+---------------+---------------+---------------+---------------+------*/
IEcPropertyHandler::EcPropertyCategory::EcPropertyCategory
(
Bentley::WString const&  n,
Bentley::WString const&  d,
uint32_t                 p
)   :
    m_standardId (NonStandard),
    m_name (n.c_str()),
    m_description (d.c_str()),
    m_priority (p)
    {
    }
