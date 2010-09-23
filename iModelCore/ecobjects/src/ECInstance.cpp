/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECInstance.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE
    
static UInt32 g_totalAllocs = 0;
static UInt32 g_totalFrees  = 0;
static UInt32 g_currentLive = 0;

//#define DEBUG_INSTANCE_LEAKS
#ifdef DEBUG_INSTANCE_LEAKS
typedef std::map<IECInstance*, UInt32> DebugInstanceLeakMap;
DebugInstanceLeakMap    g_debugInstanceLeakMap;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::IECInstance()
    {
    g_totalAllocs++;
    g_currentLive++;
#ifdef DEBUG_INSTANCE_LEAKS
    g_debugInstanceLeakMap[this] = g_totalAllocs; // record this so we know if it was the 1st, 2nd allocation
#endif

    size_t sizeofInstance = sizeof(IECInstance);
    size_t sizeofVoid = sizeof (void*);
    
    assert (sizeof(IECInstance) == sizeof (RefCountedBase) && L"Increasing the size or memory layout of the base EC::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
    };    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::~IECInstance()
    {
#ifdef DEBUG_INSTANCE_LEAKS
    g_debugInstanceLeakMap.erase(this);
#endif

    g_totalFrees++;
    g_currentLive--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_DumpAllocationStats(const wchar_t* prefix)
    {
    if (!prefix)
        prefix = L"";

    ECObjectsLogger::Log()->debugv (L"%s Live IECInstances: %d, Total Allocs: %d, TotalFrees: %d", prefix, g_currentLive, g_totalAllocs, g_totalFrees);
#ifdef DEBUG_INSTANCE_LEAKS
    for each (DebugInstanceLeakMap::value_type leak in g_debugInstanceLeakMap)
        {
        IECInstance* leakedInstance = leak.first;
        UInt32    orderOfAllocation = leak.second;
        ECObjectsLogger::Log()->debugv (L"Leaked the %dth IECInstance that was allocated.", orderOfAllocation);
        leakedInstance->Dump();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsExcluded(bwstring& className, std::vector<bwstring>& classNamesToExclude)
    {
    for each (bwstring excludedClass in classNamesToExclude)
        {
        if (0 == className.compare (excludedClass))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_ReportLeaks(std::vector<bwstring>& classNamesToExclude)
    {
#ifdef DEBUG_INSTANCE_LEAKS
    for each (DebugInstanceLeakMap::value_type leak in g_debugInstanceLeakMap)
        {
        IECInstance* leakedInstance = leak.first;
        UInt32    orderOfAllocation = leak.second;
        
        bwstring className = leakedInstance->GetClass().GetName();
        if (IsExcluded (className, classNamesToExclude))
            continue;
        
        ECObjectsLogger::Log()->errorv (L"Leaked the %dth IECInstance that was allocated: ECClass=%s, InstanceId=%s", 
            orderOfAllocation, className.c_str(), leakedInstance->GetInstanceId().c_str());
        leakedInstance->Dump();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_GetAllocationStats(int* currentLive, int* totalAllocs, int* totalFrees)
    {
    if (currentLive)
        *currentLive = g_currentLive;

    if (totalAllocs)
        *totalAllocs = g_totalAllocs;

    if (totalFrees)
        *totalFrees  = g_totalFrees;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_ResetAllocationStats()
    {
    g_totalAllocs = g_totalFrees = g_currentLive = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring        IECInstance::GetInstanceId() const
    {
    return _GetInstanceId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR             IECInstance::GetClass() const 
    {
    ECEnablerCR enabler = GetEnabler();
        
    return enabler.GetClass();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
int             IECInstance::ParseExpectedNIndices (const wchar_t * propertyAccessString)
    {
    const wchar_t * pointerToBrackets = pointerToBrackets = wcsstr (propertyAccessString, L"[]"); ;
    int nBrackets = 0;
    while (NULL != pointerToBrackets)
        {
        nBrackets++;
        pointerToBrackets += 2; // skip past the brackets
        pointerToBrackets = wcsstr (pointerToBrackets, L"[]"); ;
        }   
    
    return nBrackets;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECEnablerCR         IECInstance::GetEnabler() const { return _GetEnabler();  }
bool                IECInstance::IsReadOnly() const { return _IsReadOnly();  }

ECObjectsStatus     IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString) const { return _GetValue (v, propertyAccessString, false, 0); }
ECObjectsStatus     IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 arrayIndex) const { return _GetValue (v, propertyAccessString, true, arrayIndex); }
ECObjectsStatus     IECInstance::GetValue (ECValueR v, UInt32 propertyIndex) const { return _GetValue (v, propertyIndex, false, 0); }
ECObjectsStatus     IECInstance::GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const { return _GetValue (v, propertyIndex, true, arrayIndex); }
ECObjectsStatus     IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v) { return _SetValue (propertyAccessString, v, false, 0); }
ECObjectsStatus     IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 arrayIndex) { return _SetValue (propertyAccessString, v, true, arrayIndex); }
ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v) { return _SetValue (propertyIndex, v, false, 0); }
ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) { return _SetValue (propertyIndex, v, true, arrayIndex); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetLong (IECInstanceCR instance, Int64 & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetLong();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
ECObjectsStatus ECInstanceInteropHelper::GetInteger (IECInstanceCR instance, int & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetInteger();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/                
ECObjectsStatus ECInstanceInteropHelper::GetDouble (IECInstanceCR instance, double& value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)        
        value = v.GetDouble();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, const wchar_t * & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetString();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean (IECInstanceCR instance, bool & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetBoolean();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint2D (IECInstanceCR instance, DPoint2d & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetPoint2D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint3D (IECInstanceCR instance, DPoint3d & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
        
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetPoint3D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime (IECInstanceCR instance, SystemTime & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
    
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetDateTime();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks (IECInstanceCR instance, Int64 & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.GetValue (v, propertyAccessString, *indices);
    else
        status = instance.GetValue (v, propertyAccessString);
    
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetDateTimeTicks();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetLongValue (IECInstanceR instance, const wchar_t * propertyAccessString, Int64 value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetIntegerValue (IECInstanceR instance, const wchar_t * propertyAccessString, int value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetDoubleValue (IECInstanceR instance, const wchar_t * propertyAccessString, double value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetStringValue  (IECInstanceR instance, const wchar_t * propertyAccessString, const wchar_t * value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value, false);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetBooleanValue  (IECInstanceR instance, const wchar_t * propertyAccessString, bool value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint2DValue  (IECInstanceR instance, const wchar_t * propertyAccessString, DPoint2dCR value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint3DValue  (IECInstanceR instance, const wchar_t * propertyAccessString, DPoint3dCR value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue (IECInstanceR instance, const wchar_t * propertyAccessString, SystemTime& value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks (IECInstanceR instance, const wchar_t * propertyAccessString, Int64 value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    v.SetDateTimeTicks(value);

    ECObjectsStatus status;
    if (nIndices == 1)
        status = instance.SetValue (propertyAccessString, v, *indices);
    else
        status = instance.SetValue (propertyAccessString, v);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size)
    {
    return _InsertArrayElements (propertyAccessString, index, size);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::AddArrayElements (const wchar_t * propertyAccessString, UInt32 size)
    {
    return _AddArrayElements (propertyAccessString, size);
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index)
    {
    return _RemoveArrayElement (propertyAccessString, index);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::ClearArray (const wchar_t * propertyAccessString)
    {
    return _ClearArray (propertyAccessString);
    }           
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring        IECInstance::ToString (const wchar_t* indent) const
    {
    return _ToString (indent);
    }

END_BENTLEY_EC_NAMESPACE




 
#include <xmllite.h>
#include <atlbase.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void             AppendAccessString (bwstring& compoundAccessString, bwstring& baseAccessString, const bwstring& propertyName)
    {
    compoundAccessString = baseAccessString;
    compoundAccessString.append (propertyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static const wchar_t*   GetPrimitiveTypeString (PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            return L"binary";

        case PRIMITIVETYPE_Boolean:
            return L"boolean";

        case PRIMITIVETYPE_DateTime:
            return L"dateTime";

        case PRIMITIVETYPE_Double:
            return L"double";

        case PRIMITIVETYPE_Integer:
            return L"int";

        case PRIMITIVETYPE_Long:
            return L"long";

        case PRIMITIVETYPE_Point2D:
            return L"point2d";

        case PRIMITIVETYPE_Point3D:
            return L"point3d";

        case PRIMITIVETYPE_String:
            return L"string";
        }

    assert (false);
    return L"";
    }


typedef std::vector<byte>   T_ByteArray;

static const wchar_t INSTANCEID_ATTRIBUTE[]         = L"instanceID";
static const wchar_t SOURCECLASS_ATTRIBUTE[]        = L"sourceClass";
static const wchar_t SOURCEINSTANCEID_ATTRIBUTE[]   = L"sourceInstanceID";
static const wchar_t TARGETCLASS_ATTRIBUTE[]        = L"targetClass";
static const wchar_t TARGETINSTANCEID_ATTRIBUTE[]   = L"targetInstanceID";
static const wchar_t XMLNS_ATTRIBUTE[]              = L"xmlns";
static const wchar_t XSI_NIL_ATTRIBUTE[]            = L"nil";

// =====================================================================================
// InstanceXMLReader class
// =====================================================================================
struct  InstanceXmlReader
{
private:
    bwstring                    m_fileName;
    CComPtr <IStream>           m_stream;
    CComPtr <IXmlReader>        m_xmlReader;
    ECSchemaCR                  m_schema;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECSchemaCR schema, CComPtr <IStream> stream)
    :
    m_schema (schema), m_stream (stream), m_xmlReader (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECSchemaCR schema, const wchar_t* fileName)
    :
    m_schema (schema), m_fileName (fileName), m_stream (NULL), m_xmlReader (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus       Init ()
    {
    // different constructors set different members, according to the source of the stream and the reader.
    HRESULT     status;
    if (NULL == m_stream)
        {
        if FAILED (status = SHCreateStreamOnFileW (m_fileName.c_str (), STGM_READ, &m_stream))
            return INSTANCE_DESERIALIZATION_STATUS_FileNotFound;
        }

    if (NULL == m_xmlReader)
        {
        if (FAILED (status = CreateXmlReader (__uuidof(IXmlReader), (void**) &m_xmlReader, NULL)))
            return INSTANCE_DESERIALIZATION_STATUS_CantCreateXmlReader;

        if (FAILED (status= m_xmlReader->SetInput (m_stream)))
            return INSTANCE_DESERIALIZATION_STATUS_CantSetStream;
        }
    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus       TranslateStatus (HRESULT status)
    {
    struct ErrorMap { HRESULT m_hResult; InstanceDeserializationStatus m_ixrStatus; };
    static ErrorMap s_errorMap[] = 
        {
        { WC_E_DECLELEMENT,         INSTANCE_DESERIALIZATION_STATUS_BadElement              },
        { WC_E_NAME,                INSTANCE_DESERIALIZATION_STATUS_NoElementName           },  
        { WC_E_ELEMENTMATCH,        INSTANCE_DESERIALIZATION_STATUS_EndElementDoesntMatch   },
        { WC_E_ENTITYCONTENT,       INSTANCE_DESERIALIZATION_STATUS_BadElement              },  
        { WC_E_ROOTELEMENT,         INSTANCE_DESERIALIZATION_STATUS_BadElement              },    
        { S_FALSE,                  INSTANCE_DESERIALIZATION_STATUS_XmlFileIncomplete       },
        };
    
    for (int iError=0; iError < _countof (s_errorMap); iError++)
        {
        if (status == s_errorMap[iError].m_hResult)
            return s_errorMap[iError].m_ixrStatus;
        }
    return INSTANCE_DESERIALIZATION_STATUS_XmlParseError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus       ReadInstance (IECInstancePtr& ecInstance)
    {
    // The Instance XML element starts with a node that has the name of the class of the instance.
    HRESULT         status;
    XmlNodeType     nodeType;

    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        switch (nodeType) 
            {
            case XmlNodeType_EndElement:
                {
                // we should not hit this.
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_Success;
                }

            case XmlNodeType_Element:
                {
                ECClassCP       ecClass;

                // the first Element tells us ECClass of the instance.
                InstanceDeserializationStatus       ixrStatus;
                if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = GetInstance (&ecClass, ecInstance)))
                    return ixrStatus;

                // this reads the property members and consumes the XmlNodeType_EndElement corresponding to this XmlNodeType_Element.
                return ReadInstanceOrStructMembers (*ecClass, ecInstance.get(), NULL);
                }

            default:
                // we can ignore the other types.
                break;
            }
        }

    // should not get here.
    return TranslateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   GetInstance (ECClassCP* ecClass, IECInstancePtr& ecInstance)
    {
    *ecClass = NULL;

    // when this method is called, we are positioned at the node that should have a className.
    HRESULT         status;
    if (FAILED (status = m_xmlReader->MoveToElement()))
        return INSTANCE_DESERIALIZATION_STATUS_BadElement;

    const wchar_t*        className;
    if (FAILED (status = m_xmlReader->GetLocalName (&className, NULL)))
        return INSTANCE_DESERIALIZATION_STATUS_NoElementName;

    // see if we can find the class from the schema we have.
    ECClassCP    foundClass;
    if (NULL == (foundClass = m_schema.GetClassP (className)))
        {
        ECSchemaReferenceList refList = m_schema.GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator;
        for (schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
            {
            if (NULL != (foundClass = (*schemaIterator)->GetClassP (className)))
                break;
            }
        }
    if (NULL == foundClass)
        return INSTANCE_DESERIALIZATION_STATUS_ECClassNotFound;

    *ecClass = foundClass;

    // create a StandAloneECInstance instance of the class
    SchemaLayout                schemaLayout (24);
    ClassLayoutP                classLayout         = ClassLayout::BuildFromClass (*foundClass, 42, schemaLayout.GetSchemaIndex());
    StandaloneECEnablerPtr      standaloneEnabler   = StandaloneECEnabler::CreateEnabler (*foundClass, *classLayout);

    // create the instance.
    ecInstance                                      = standaloneEnabler->CreateInstance().get();

    IECRelationshipInstance*    relationshipInstance = dynamic_cast <IECRelationshipInstance*> (ecInstance.get());

    bool                        needSourceClass    = false;
    bool                        needSourceId       = false;
    bool                        needTargetClass    = false;
    bool                        needTargetId       = false;

    // if relationship, need the attributes.
    if (NULL != relationshipInstance)
        needSourceClass = needSourceId = needTargetClass = needTargetId = true;

    // read the instance attributes.
    for (status = m_xmlReader->MoveToFirstAttribute(); S_OK == status; status = m_xmlReader->MoveToNextAttribute())
        {
        // we have an attribute.
        const wchar_t*      attributeName;
        if (FAILED (status = m_xmlReader->GetLocalName (&attributeName, NULL)))
            return TranslateStatus (status);

        // see if it's the instanceId attribute.
        if (0 == wcscmp (INSTANCEID_ATTRIBUTE, attributeName))
            {
            // get the value.
            const wchar_t*  instanceId;
            if (FAILED (status = m_xmlReader->GetValue (&instanceId, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_INSTANCEID)
            ecInstance->SetInstanceId (instanceId);
#endif
            }

        else if (0 == wcscmp (SOURCEINSTANCEID_ATTRIBUTE, attributeName))
            {
            // get the value.
            const wchar_t*  sourceInstanceId;
            if (FAILED (status = m_xmlReader->GetValue (&sourceInstanceId, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceInstanceId (sourceInstanceId);
#endif
            }
        else if (0 == wcscmp (SOURCECLASS_ATTRIBUTE, attributeName))
            {
            // get the value.
            const wchar_t*  sourceClass;
            if (FAILED (status = m_xmlReader->GetValue (&sourceClass, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceClass (sourceClass);
#endif
            }
        else if (0 == wcscmp (TARGETINSTANCEID_ATTRIBUTE, attributeName))
            {
            // get the value.
            const wchar_t*  targetInstanceId;
            if (FAILED (status = m_xmlReader->GetValue (&targetInstanceId, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceInstanceId (targetInstanceId);
#endif
            }
        else if (0 == wcscmp (TARGETCLASS_ATTRIBUTE, attributeName))
            {
            // get the value.
            const wchar_t*  targetClass;
            if (FAILED (status = m_xmlReader->GetValue (&targetClass, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceClass (targetClass);
#endif
            }
        else if (0 == wcscmp (XMLNS_ATTRIBUTE, attributeName))
            {
            const wchar_t*  nameSpace;
            if (FAILED (status = m_xmlReader->GetValue (&nameSpace, NULL)))
                return TranslateStatus (status);
            const wchar_t*  schemaName = foundClass->Schema.Name.c_str();
            assert (0 == wcsncmp (schemaName, nameSpace, wcslen (schemaName)));
            }

        else
            {
            // unexpected attribute.
            assert (false);
            }

        // the namespace should agree with the schema name.
        }
    // IsEmptyElement returns false if the reader is positioned on an attribute node, even if attribute's parent element is empty. 
    if (S_OK != (status = m_xmlReader->MoveToElement()))
        return TranslateStatus (status);

    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadInstanceOrStructMembers (ECClassCR ecClass, IECInstanceP ecInstance, bwstring* baseAccessString)
    {
    // On entry, the reader is positioned in the content of an instance or struct.

    HRESULT         status;

    // if it's an empty node, all members of the instance are NULL.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_DESERIALIZATION_STATUS_Success;

    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                InstanceDeserializationStatus   propertyStatus;
                if (INSTANCE_DESERIALIZATION_STATUS_Success != (propertyStatus = ReadProperty (ecClass, ecInstance, baseAccessString)))
                    return propertyStatus;
                break;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct.
                return INSTANCE_DESERIALIZATION_STATUS_Success;

            case XmlNodeType_Text:
                {
                // we do not expect there to be any "value" in a struct of class, only property nodes.
                assert (false);
                break;
                }

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }
        }
    // shouldn't get here.
    assert (false);
    return TranslateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadProperty (ECClassCR ecClass, IECInstanceP ecInstance, bwstring* baseAccessString)
    {
    // on entry, the reader is positioned at the Element.
    // get the element name, which is the property name.
    HRESULT         status;
    const wchar_t*        propertyName;
    if (FAILED (status = m_xmlReader->GetLocalName (&propertyName, NULL)))
        return INSTANCE_DESERIALIZATION_STATUS_NoElementName;

    // try to find the property in the class.
    ECPropertyP ecProperty;
    if (NULL == (ecProperty = ecClass.GetPropertyP (propertyName)))
        {
        // couldn't find it, skip the rest of the property.
        return SkipToElementEnd ();
        }

    PrimitiveECPropertyP    primitiveProperty;
    ArrayECPropertyP        arrayProperty;
    StructECPropertyP       structProperty;
    if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
        return ReadPrimitiveProperty (primitiveProperty, ecInstance, baseAccessString);
		//Above is good, if SkipToElementEnd() is returned from ReadPrimitiveValue.
    else if (NULL != (arrayProperty = ecProperty->GetAsArrayProperty()))
        return ReadArrayProperty (arrayProperty, ecInstance, baseAccessString);
    else if (NULL != (structProperty = ecProperty->GetAsStructProperty()))
        return ReadEmbeddedStructProperty (structProperty, ecInstance, baseAccessString);

    // should be one of those!
    assert (false);
    return INSTANCE_DESERIALIZATION_STATUS_BadECProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadEmbeddedStructProperty (StructECPropertyP structProperty, IECInstanceP ecInstance, bwstring* baseAccessString)
    {
    // empty element OK for struct - all members are null.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_DESERIALIZATION_STATUS_Success;

    bwstring    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty->Name);
    else
        thisAccessString = structProperty->Name.c_str();
    thisAccessString.append (L".");

    return ReadInstanceOrStructMembers (structProperty->Type, ecInstance, &thisAccessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadPrimitiveProperty (PrimitiveECPropertyP primitiveProperty, IECInstanceP ecInstance, bwstring* baseAccessString)
    {
    // on entry, we are positioned in the PrimitiveProperty element.
    PrimitiveType                   propertyType = primitiveProperty->Type;
    InstanceDeserializationStatus   ixrStatus;
    ECValue         ecValue;
    if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, propertyType)))
        return ixrStatus;

	if(ecValue.IsUninitialized())
		{
		//A malformed value was found.  A warning was shown; just move on.
		return INSTANCE_DESERIALIZATION_STATUS_Success;
		}

    ECObjectsStatus setStatus;
    if (NULL == baseAccessString)
        {
        setStatus = ecInstance->SetValue (primitiveProperty->Name.c_str(), ecValue);
        }
    else
        {
        bwstring compoundAccessString;
        AppendAccessString (compoundAccessString, *baseAccessString, primitiveProperty->Name);
        setStatus = ecInstance->SetValue (compoundAccessString.c_str(), ecValue);
        }
    assert (ECOBJECTS_STATUS_Success == setStatus);

    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadArrayProperty (ArrayECPropertyP arrayProperty, IECInstanceP ecInstance, bwstring* baseAccessString)
    {
    // on entry, the reader is positioned at the element that indicates the start of the array.
    // empty element OK for array - no members.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_DESERIALIZATION_STATUS_Success;

    bwstring    accessString;
    if (NULL == baseAccessString)
        accessString = arrayProperty->Name;    
    else
        AppendAccessString (accessString, *baseAccessString, arrayProperty->Name);

    accessString.append (L"[]");

    // start the address out as zero.
    UInt32      index = 0;

    // we have to find out what type the array is.
    ArrayKind   arrayKind = arrayProperty->Kind;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        PrimitiveType   memberType = arrayProperty->PrimitiveElementType;

        // read nodes - we expect to find an Element with LocalName matching the primitive type.
        XmlNodeType     nodeType;
        HRESULT         status;
        while (S_OK == (status = m_xmlReader->Read (&nodeType)))
            {
            switch (nodeType)
                {
                case XmlNodeType_Element:
                    {
                    // validate the LocalName against the expected primitiveType.
                    const wchar_t*        primitiveTypeName;
                    if (FAILED (status = m_xmlReader->GetLocalName (&primitiveTypeName, NULL)))
                        return INSTANCE_DESERIALIZATION_STATUS_NoElementName;

                    if (!ValidateArrayPrimitiveType (primitiveTypeName, memberType))
					{
						Logger::GetLogger()->warningv(L"Incorrectly formatted array element found in array %ls.  Expected: %ls  Found: %ls",
							accessString, GetPrimitiveTypeString (memberType), primitiveTypeName);
						//Skip this element to start looking for elements with the correct primitive type.
						SkipToElementEnd();
						continue;
						//By continuing here, we make sure that the bad value is not set.
					}

                    // now we know the type and we are positioned at the element containing the value.
                    // read it, populating the ECInstance using accessString and arrayIndex.
                    InstanceDeserializationStatus   ixrStatus;
                    ECValue                         ecValue;
                    if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, memberType)))
                        return ixrStatus;
					if(ecValue.IsUninitialized())
						{
						//A malformed value was found.  A warning was shown; just move on.
						continue;
						}
                    ecInstance->AddArrayElements (accessString.c_str(), 1);
                    ECObjectsStatus   setStatus;
                    if (ECOBJECTS_STATUS_Success != (setStatus = ecInstance->SetValue (accessString.c_str(), ecValue, index)))
                        {
                        assert (false);
                        return INSTANCE_DESERIALIZATION_STATUS_CantSetValue;
                        }

                    // increment the array index.
                    index++;

                    break;
                    }

                case XmlNodeType_Text:
                    {
                    // we don't expend that there is any text in an array element, just child elements.
                    assert (false);
                    break;
                    }

                case XmlNodeType_EndElement:
                    // we have encountered the end of the array.
                    return INSTANCE_DESERIALIZATION_STATUS_Success;

                default:
                    {
                    // simply ignore white space, comments, etc.
                    break;
                    }
                }
            }

        // shouldn't get here.
        return TranslateStatus (status);
        }

    else if (ARRAYKIND_Struct == arrayKind)
        {
        ECClassCP   structMemberType = arrayProperty->StructElementType;

        // read nodes - we expect to find an Element with LocalName being the class name of structMemberType.
        // For polymorphic arrays, the LocalName might also be the name of a class that has structMemberType as a BaseType.
        XmlNodeType     nodeType;
        HRESULT         status;
        while (S_OK == (status = m_xmlReader->Read (&nodeType)))
            {
            switch (nodeType)
                {
                case XmlNodeType_Element:
                    {
                    // validate the LocalName against the structMemberType
                    const wchar_t*        structName;
                    if (FAILED (status = m_xmlReader->GetLocalName (&structName, NULL)))
                        return INSTANCE_DESERIALIZATION_STATUS_NoElementName;

                    ECClassCP   thisMemberType;
                    if (NULL == (thisMemberType = ValidateArrayStructType (structName, structMemberType)))
                        return INSTANCE_DESERIALIZATION_STATUS_BadArrayElement;

                    InstanceDeserializationStatus ixrStatus;
                    if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ReadStructArrayMember (*thisMemberType, ecInstance, accessString, index)))
                        return ixrStatus;

                    // increment the array index.
                    index++;

                    break;
                    }

                case XmlNodeType_EndElement:
                    // we have encountered the end of the array.
                    return INSTANCE_DESERIALIZATION_STATUS_Success;

                default:
                    {
                    // simply ignore white space, comments, etc.
                    break;
                    }
                }
            }
        // shouldn't get here.
        return TranslateStatus (status);
        }
    
    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadStructArrayMember (ECClassCR structClass, IECInstanceP owningInstance, bwstring& accessString, UInt32 index)
    {
    // On entry, the reader is positioned at the element that starts the struct.
    // we have to create an IECInstance for the array member.
    SchemaLayout                    schemaLayout (24);
    ClassLayoutP                    classLayout         = ClassLayout::BuildFromClass (structClass, 42, schemaLayout.GetSchemaIndex());
    StandaloneECEnablerPtr          standaloneEnabler   = StandaloneECEnabler::CreateEnabler (structClass, *classLayout);

    // create the instance.
    IECInstancePtr                  structInstance      = standaloneEnabler->CreateInstance().get();

    InstanceDeserializationStatus   ixrStatus;
    if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ReadInstanceOrStructMembers (structClass, structInstance.get(), NULL)))
        return ixrStatus;

    // every StructArrayMember is a new ECInstance, 
    // set the value 
    ECValue structValue;
    structValue.SetStruct (structInstance.get());

    // add the value to the array.
    owningInstance->AddArrayElements (accessString.c_str(), 1);
    ECObjectsStatus setStatus = owningInstance->SetValue (accessString.c_str(), structValue, index);
    assert (ECOBJECTS_STATUS_Success == setStatus);

    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadPrimitiveValue (ECValueR ecValue, PrimitiveType propertyType)
    {
    // The reader is positioned at the XmlNodeType_Element that holds the value. 
    // We expect to find a Text element with the value, advance until we do.

    HRESULT         status;
    // First check to see if this is set to NULL
    for (status = m_xmlReader->MoveToFirstAttribute(); S_OK == status; status = m_xmlReader->MoveToNextAttribute())
        {
        // we have an attribute.
        const wchar_t*      attributeName;
        if (FAILED (status = m_xmlReader->GetLocalName (&attributeName, NULL)))
            return TranslateStatus (status);

        // see if it's the instanceId attribute.
        if (0 == wcscmp (XSI_NIL_ATTRIBUTE, attributeName))
            {
            // get the value.
            const wchar_t *  isNil;
            if (FAILED (status = m_xmlReader->GetValue (&isNil, NULL)))
                return TranslateStatus (status);

            if ((0 == wcscmp (isNil, L"True")) || (0 == wcscmp (isNil, L"true")) || 
                (0 == wcscmp (isNil, L"TRUE")) || (0 == wcscmp (isNil, L"1")))
                {
                ecValue.SetToNull();
                }
            }
            m_xmlReader->MoveToElement();
        }

    if (m_xmlReader->IsEmptyElement())
        {
		Logger::GetLogger()->warningv(L"Empty element encountered in deserialization.  Setting ECValue to NULL...");
        return INSTANCE_DESERIALIZATION_STATUS_Success;
        }

    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        bool positionedAtText = false;
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_UnexpectedElement;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct without getting a value from the element.
                // we will break here to keep the ECValue null.
				Logger::GetLogger()->warningv(L"Element encountered in deserialization with no value.  Setting ECValue to NULL...");
				return INSTANCE_DESERIALIZATION_STATUS_Success;

            case XmlNodeType_Text:
                {
                // we do not expect there to be any "value" in a struct of class, only property nodes.
                positionedAtText = true;
                break;
                }

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }

        if (positionedAtText)
            break;
        }

    const wchar_t*  propertyValueString;
    if (FAILED (status = m_xmlReader->GetValue (&propertyValueString, NULL)))
        return TranslateStatus (status);

    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            T_ByteArray                     byteArray;

            if (INSTANCE_DESERIALIZATION_STATUS_Success != ConvertStringToByteArray (byteArray, propertyValueString))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Binary", propertyValueString);
				return SkipToElementEnd();
                }
            ecValue.SetBinary (&byteArray.front(), byteArray.size(), true);
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            bool    boolValue = ((0 == wcscmp (propertyValueString, L"True")) || (0 == wcscmp (propertyValueString, L"true")) || 
                                 (0 == wcscmp (propertyValueString, L"TRUE")) || (0 == wcscmp (propertyValueString, L"1")));
            ecValue.SetBoolean (boolValue);
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            Int64   ticks;
            if (1 != swscanf (propertyValueString, L"%I64d", &ticks))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not DateTime", propertyValueString);
				return SkipToElementEnd();
                }

            ecValue.SetDateTimeTicks (ticks);
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            double  doubleValue;
            if (1 != swscanf (propertyValueString, L"%lg", &doubleValue))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Double", propertyValueString);
				return SkipToElementEnd();
                }
            ecValue.SetDouble (doubleValue);
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            Int32   intValue;
            if (1 != swscanf (propertyValueString, L"%d", &intValue))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Integer", propertyValueString);
				return SkipToElementEnd();
                }
            ecValue.SetInteger (intValue);
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64   longValue;
            if (1 != swscanf (propertyValueString, L"%I64d", &longValue))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Long", propertyValueString);
				return SkipToElementEnd();
                }
            ecValue.SetLong (longValue);
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d point2d;
            if (2 != swscanf (propertyValueString, L"%lg,%lg", &point2d.x, &point2d.y))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Point2D", propertyValueString);
				return SkipToElementEnd();
                }
            ecValue.SetPoint2D (point2d);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            if (3 != swscanf (propertyValueString, L"%lg,%lg,%lg", &point3d.x, &point3d.y, &point3d.z))
                {
				Logger::GetLogger()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Point3D", propertyValueString);
				return SkipToElementEnd();
                }
            ecValue.SetPoint3D (point3d);
            break;
            }

        case PRIMITIVETYPE_String:
            {
			ecValue.SetString (propertyValueString);
            break;
            }

        default:
            {
            assert (false);
            return INSTANCE_DESERIALIZATION_STATUS_BadPrimitivePropertyType;
            }
        }

    // we want to find the EndElement for this node.
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                return INSTANCE_DESERIALIZATION_STATUS_UnexpectedElement;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct.
                return INSTANCE_DESERIALIZATION_STATUS_Success;

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }
        }

    // shouldn't get here.
    assert (false);
    return TranslateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ConvertStringToByteArray (T_ByteArray& byteData, const wchar_t* stringData)
    {
    // the length of stringData should be a muttiple of four.
    size_t  stringLen = wcslen (stringData);
    if (0 != (stringLen % 4))
        return INSTANCE_DESERIALIZATION_STATUS_BadBinaryData;

    // from each 4 characters we get 3 byte values.
    for (size_t iPos=0; iPos < stringLen; iPos+= 4)
        {
        Int32   nextThreeBytes = 0;
        int     numBytesToPush = 3;
        int     shift;
        int     jPos;
        for (jPos=0, shift=0; jPos < 4; jPos++, shift += 6)
            {
            wchar_t charValue = stringData[iPos+jPos];
            if ( (charValue >= L'A') && (charValue <= L'Z') )
                nextThreeBytes |= ((charValue - L'A') << shift);
            else if ((charValue >= L'a') && (charValue <= L'z') )
                nextThreeBytes |= ( ((charValue - L'a') + 26) << shift);
            else if ((charValue >= L'0') && (charValue <= L'9') )
                nextThreeBytes |= ( ((charValue - L'0') + 52) << shift);
            else if (charValue == L'+')
                nextThreeBytes |= ( 62 << shift);
            else if (charValue == L'/')
                nextThreeBytes |= ( 63 << shift);
            else if (charValue == L'=')
                {
                // = should only appear in the last two characters of the string.
                if (stringLen - (iPos + jPos) > 2)
					return INSTANCE_DESERIALIZATION_STATUS_BadBinaryData;
                numBytesToPush = jPos-1;
                break;
                }
            else
                {
                return INSTANCE_DESERIALIZATION_STATUS_BadBinaryData;
                }

            }

        byte*   bytes = (byte*)&nextThreeBytes;
        byteData.push_back (*bytes);
        if (numBytesToPush > 1)
            byteData.push_back (*(bytes+1));
        if (numBytesToPush > 2)
            byteData.push_back (*(bytes+2));
        }

    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ValidateArrayPrimitiveType (const wchar_t* typeFound, PrimitiveType expectedType)
    {
    return (0 == wcscmp (typeFound, GetPrimitiveTypeString (expectedType)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP       ValidateArrayStructType (const wchar_t* typeFound, ECClassCP expectedType)
    {
    // the common case is that they're all of the expected ECClass.
    if (0 == wcscmp (typeFound, expectedType->Name.c_str()))
        return expectedType;

    // Probably need some namespace / schema find stuff here.

    // typeFound must resolve to an ECClass that is either expectedType or a class that has expectedType as a Base Class.
    ECClassCP    classFound;
    if (NULL == (classFound = m_schema.GetClassP (typeFound)))
        {
        assert (false);
        return NULL;
        }
    if (!classFound->Is (expectedType))
        {
        assert (false);
        return NULL;
        }

    return classFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   SkipToElementEnd ()
    {
    // skips from current point to the end of the current element.
    HRESULT         status;
    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        // ignore everything except the end of the element.
        if (XmlNodeType_EndElement == nodeType)
            return INSTANCE_DESERIALIZATION_STATUS_Success;
        }

    return INSTANCE_DESERIALIZATION_STATUS_BadElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          GetLineNumber ()
    {
    UInt32  lineNumber = 0;
    m_xmlReader->GetLineNumber (&lineNumber);
    return lineNumber;
    }

};

// =====================================================================================
// InstanceXMLWriter class
// =====================================================================================
struct  InstanceXmlWriter
{
private:
    bwstring                    m_fileName;
    CComPtr <IStream>           m_stream;
    CComPtr <IXmlWriter>        m_xmlWriter;
    CComPtr <IXmlWriterOutput>  m_xmlOutput;
    bool                        m_compacted;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlWriter (CComPtr <IStream> stream)
    {
    m_stream            = stream;
    m_xmlWriter         = NULL;
    m_compacted         = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlWriter (bwstring fileName)
    {
    m_fileName          = fileName;
    m_stream            = NULL;
    m_xmlWriter         = NULL;
    m_compacted         = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     Init ()
    {
    // different constructors set different members, according to the source of the stream and the writer.
    HRESULT     status;
    if (NULL == m_stream)
        {
        if FAILED (status = SHCreateStreamOnFileW (m_fileName.c_str (), STGM_CREATE | STGM_WRITE, &m_stream))
            return INSTANCE_SERIALIZATION_STATUS_CantCreateStream;
        }

    if (NULL == m_xmlWriter)
        {
        if (FAILED (status = CreateXmlWriter (__uuidof(IXmlWriter), (void**) &m_xmlWriter, NULL)))
            return INSTANCE_SERIALIZATION_STATUS_CantCreateXmlWriter;


        if (FAILED (status = CreateXmlWriterOutputWithEncodingName(m_stream, 0, L"utf-16", &m_xmlOutput)))
            return INSTANCE_SERIALIZATION_STATUS_CantCreateXmlWriter;

        if (FAILED (status= m_xmlWriter->SetOutput (m_xmlOutput)))
            return INSTANCE_SERIALIZATION_STATUS_CantSetStream;

        if (!m_compacted)
            m_xmlWriter->SetProperty (XmlWriterProperty_Indent, true);
        }
    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     WriteInstance (IECInstanceCR instance, bool writeStart)
    {
    ECClassCR               ecClass     = instance.GetClass();
    ECSchemaCR              ecSchema    = ecClass.Schema;

    HRESULT status;
    if (writeStart)
        {
        if (S_OK != (status = m_xmlWriter->WriteStartDocument (XmlStandalone_Omit)))
            return TranslateStatus (status);
        }

    // start by writing the name of the class as an element, with the schema name as the namespace.
    size_t size = wcslen(ecSchema.Name.c_str()) + 8;
    wchar_t *fullSchemaName = (wchar_t*)malloc(size * sizeof(wchar_t));
    swprintf(fullSchemaName, size, L"%s.%02d.%02d", ecSchema.Name.c_str(), ecSchema.VersionMajor, ecSchema.VersionMinor);
    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, ecClass.Name.c_str(), fullSchemaName)))
        {
        free(fullSchemaName);
        return TranslateStatus (status);
        }
    free(fullSchemaName);

    WritePropertiesOfClassOrStructArrayMember (ecClass, instance, NULL);

    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    m_xmlWriter->WriteEndDocument ();

    m_xmlWriter->Flush();

    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     WritePropertiesOfClassOrStructArrayMember (ECClassCR ecClass, IECInstanceCR ecInstance, bwstring* baseAccessString)
    {
    ECPropertyIterableCR    collection  = ecClass.GetProperties (true);
    for each (ECPropertyP ecProperty in collection)
        {
        PrimitiveECPropertyP        primitiveProperty;
        ArrayECPropertyP            arrayProperty;
        StructECPropertyP           structProperty;
        InstanceSerializationStatus ixwStatus;
            
        if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
            ixwStatus = WritePrimitiveProperty (*primitiveProperty, ecInstance, baseAccessString);
        else if (NULL != (arrayProperty = ecProperty->GetAsArrayProperty()))
            ixwStatus = WriteArrayProperty (*arrayProperty, ecInstance, baseAccessString);
        else if (NULL != (structProperty = ecProperty->GetAsStructProperty()))
            ixwStatus = WriteEmbeddedStructProperty (*structProperty, ecInstance, baseAccessString);

        if (INSTANCE_SERIALIZATION_STATUS_Success != ixwStatus)
            {
            assert (false);
            return ixwStatus;
            }
        }

     return INSTANCE_SERIALIZATION_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     WritePrimitiveProperty (PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, bwstring* baseAccessString)
    {
    ECObjectsStatus     getStatus;
    ECValue             ecValue;
    bwstring const& propertyName = primitiveProperty.Name;

    if (NULL == baseAccessString)
        {
        getStatus = ecInstance.GetValue (ecValue, propertyName.c_str());
        }
    else
        {
        bwstring compoundAccessString;
        AppendAccessString (compoundAccessString, *baseAccessString, propertyName);
        getStatus = ecInstance.GetValue (ecValue, compoundAccessString.c_str());
        }

    // couldn't get, or NULL value, write nothing.
    if ( (ECOBJECTS_STATUS_Success != getStatus) || ecValue.IsNull() )
        return INSTANCE_SERIALIZATION_STATUS_Success;

    HRESULT     status;
    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, propertyName.c_str(), NULL)))
        return TranslateStatus (status);

    InstanceSerializationStatus     ixwStatus;
    PrimitiveType                   propertyType = primitiveProperty.Type;
    if (INSTANCE_SERIALIZATION_STATUS_Success != (ixwStatus = WritePrimitiveValue (ecValue, propertyType)))
        return ixwStatus;

    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     WritePrimitiveValue (ECValueCR ecValue, PrimitiveType propertyType)
    {
    wchar_t     outString[512];

    // write the content according to type.
    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            size_t      numBytes;
            const byte* byteData; 
            if (NULL != (byteData = ecValue.GetBinary (numBytes)))
                {
                bwstring    byteString = ConvertByteArrayToString (byteData, numBytes);
                HRESULT         status;
                if (S_OK != (status = m_xmlWriter->WriteChars (byteString.c_str(), static_cast <UINT> (byteString.length()))))
                    return TranslateStatus (status);
                else
                    return INSTANCE_SERIALIZATION_STATUS_Success;
                }
            else
                return INSTANCE_SERIALIZATION_STATUS_Success;
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            wcscpy (outString, ecValue.GetBoolean () ? L"True" : L"False");
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            swprintf (outString, L"%I64d", ecValue.GetDateTimeTicks());
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            swprintf (outString, L"%.13g", ecValue.GetDouble());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            swprintf (outString, L"%d", ecValue.GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            swprintf (outString, L"%I64d", ecValue.GetLong());
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d    point2d = ecValue.GetPoint2D();
            swprintf (outString, L"%.13g,%.13g", point2d.x, point2d.y);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d    point3d = ecValue.GetPoint3D();
            swprintf (outString, L"%.13g,%.13g,%.13g", point3d.x, point3d.y, point3d.z);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            const wchar_t*  stringValue = ecValue.GetString ();
            HRESULT         status;
            if (S_OK != (status = m_xmlWriter->WriteChars (stringValue, static_cast <UINT> (wcslen (stringValue)))))
                return TranslateStatus (status);
            return INSTANCE_SERIALIZATION_STATUS_Success;
            }

        default:
            {
            assert (false);
            return INSTANCE_SERIALIZATION_STATUS_BadPrimitivePropertyType;
            }
        }

    HRESULT     status;
    if (S_OK != (status = m_xmlWriter->WriteChars (outString, static_cast <UINT> (wcslen (outString)))))
        return TranslateStatus (status);

    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     WriteArrayProperty (ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, bwstring* baseAccessString)
    {
    ArrayKind       arrayKind = arrayProperty.Kind;

    bwstring    accessString;
    if (NULL == baseAccessString)
        accessString = arrayProperty.Name;    
    else
        AppendAccessString (accessString, *baseAccessString, arrayProperty.Name);

    accessString.append (L"[]");

    // no members, don't write anything.
    ECValue         ecValue;
    if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), 0) || ecValue.IsNull())
        return INSTANCE_SERIALIZATION_STATUS_Success;

    // write the start element, which consists of the member name.
    HRESULT     status;

    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, arrayProperty.Name.c_str(), NULL)))
        return TranslateStatus (status);

    InstanceSerializationStatus     ixwStatus;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        PrimitiveType   memberType  = arrayProperty.PrimitiveElementType;
        const wchar_t*  typeString  = GetPrimitiveTypeString (memberType);
        for (int index=0; ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, typeString, NULL)))
                return TranslateStatus (status);

            // write the primitve value
            if (INSTANCE_SERIALIZATION_STATUS_Success != (ixwStatus = WritePrimitiveValue (ecValue, memberType)))
                {
                assert (false);
                return ixwStatus;
                }

            if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
                return TranslateStatus (status);
            }
        }
    else if (ARRAYKIND_Struct == arrayKind)
        {
        ECClassCP   memberClass = arrayProperty.StructElementType;
        for (int index=0; ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            // the XML element tag is the struct type.
            assert (ecValue.IsStruct());

            IECInstancePtr  structInstance = ecValue.GetStruct();
            if (!structInstance.IsValid())
                {
                assert (false);
                break;
                }

            ECClassCR   structClass = structInstance->GetClass();
            assert (structClass.Is (memberClass));

            if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, structClass.Name.c_str(), NULL)))
                return TranslateStatus (status);

            WritePropertiesOfClassOrStructArrayMember (structClass, *structInstance.get(), NULL);

            if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
                return TranslateStatus (status);
            }
        }
    else
        {
        // unexpected arrayKind - should never happen.
        assert (false);
        }


    // write the end element for the array as a whole.
    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     WriteEmbeddedStructProperty (StructECPropertyR structProperty, IECInstanceCR ecInstance, bwstring* baseAccessString)
    {
    // the tag of the element for an embedded struct is the property name.
    HRESULT     status;
    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, structProperty.Name.c_str(), NULL)))
        return TranslateStatus (status);

    bwstring    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty.Name);
    else
        thisAccessString = structProperty.Name.c_str();
    thisAccessString.append (L".");

    ECClassCR   structClass = structProperty.Type;
    WritePropertiesOfClassOrStructArrayMember (structClass, ecInstance, &thisAccessString);

    // write the end element for the array as a whole.
    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring    ConvertByteArrayToString (const byte *byteData, size_t numBytes)
    {
    static const wchar_t    base64Chars[] = {L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

    if (0 == numBytes)
        return L"";

    // from each 3 bytes we get 4 output characters, rounded up.
    bwstring    outString;
    for (size_t iByte=0; iByte < numBytes; iByte += 3)
        {
        UInt32      nextThreeBytes = byteData[iByte] | (byteData[iByte+1] << 8) | (byteData[iByte+2] << 16);

        for (size_t jPos=0; jPos < 4; jPos++)
            {
            byte    sixBits = nextThreeBytes & 0x3f;

            if ( (iByte + jPos) < (numBytes + 1) )
                outString.append (1, base64Chars[sixBits]);
            else
                outString.append (1, L'=');
            
            nextThreeBytes = nextThreeBytes >> 6;
            }
        }

    return outString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     TranslateStatus (HRESULT status)
    {
#if 0
    struct ErrorMap { HRESULT m_hResult; InstanceSerializationStatus m_ixrStatus; };
    // I'm not sure what the possible errors are when writing, so this method does pretty much nothing.
    static ErrorMap s_errorMap[] = 
        {
        { WC_E_DECLELEMENT,         INSTANCE_DESERIALIZATION_STATUS_BadElement              },
        { WC_E_NAME,                INSTANCE_DESERIALIZATION_STATUS_NoElementName           },  
        { WC_E_ELEMENTMATCH,        INSTANCE_DESERIALIZATION_STATUS_EndElementDoesntMatch   },
        { WC_E_ENTITYCONTENT,       INSTANCE_DESERIALIZATION_STATUS_BadElement              },      
        { S_FALSE,                  INSTANCE_DESERIALIZATION_STATUS_XmlFileIncomplete       },
        };
    
    for (int iError=0; iError < _countof (s_errorMap); iError++)
        {
        if (status == s_errorMap[iError].m_hResult)
            return s_errorMap[iError].m_ixrStatus;
        }
#endif
    return INSTANCE_SERIALIZATION_STATUS_XmlWriteError;
    }

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   IECInstance::ReadXmlFromFile (IECInstancePtr& ecInstance, const wchar_t* fileName, ECSchemaCR schema)
    {
    InstanceXmlReader reader (schema, fileName);

    InstanceDeserializationStatus   status;
    if (INSTANCE_DESERIALIZATION_STATUS_Success != (status = reader.Init ()))
        return status;

    return reader.ReadInstance (ecInstance);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   IECInstance::ReadXmlFromStream (IECInstancePtr& ecInstance, IStreamP stream, ECSchemaCR schema)
    {
    InstanceXmlReader reader (schema, stream);

    InstanceDeserializationStatus   status;
    if (INSTANCE_DESERIALIZATION_STATUS_Success != (status = reader.Init ()))
        return status;

    return reader.ReadInstance (ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   IECInstance::ReadXmlFromString (IECInstancePtr& ecInstance, const wchar_t* xmlString, ECSchemaCR schema)
    {
    CComPtr <IStream> stream;
    if (S_OK != ::CreateStreamOnHGlobal(NULL,TRUE,&stream))
        return INSTANCE_DESERIALIZATION_STATUS_CantCreateStream;

    LARGE_INTEGER liPos = {0};
    if (S_OK != stream->Seek(liPos, STREAM_SEEK_SET, NULL))
        return INSTANCE_DESERIALIZATION_STATUS_CantCreateStream;

    ULARGE_INTEGER uliSize = { 0 };
    stream->SetSize(uliSize);

    ULONG bytesWritten;
    ULONG ulSize = (ULONG) wcslen(xmlString) * sizeof(wchar_t);

    if (S_OK != stream->Write(xmlString, ulSize, &bytesWritten))
        return INSTANCE_DESERIALIZATION_STATUS_CantCreateStream;

    if (ulSize != bytesWritten)
        return INSTANCE_DESERIALIZATION_STATUS_CantCreateStream;

    if (S_OK != stream->Seek(liPos, STREAM_SEEK_SET, NULL))
        return INSTANCE_DESERIALIZATION_STATUS_CantCreateStream;

    InstanceXmlReader reader (schema, stream);

    InstanceDeserializationStatus   status;
    if (INSTANCE_DESERIALIZATION_STATUS_Success != (status = reader.Init ()))
        return status;

    return reader.ReadInstance (ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     IECInstance::WriteXmlToFile (const wchar_t* fileName, bool isStandAlone)
    {
    InstanceXmlWriter writer (fileName);

    InstanceSerializationStatus   status;
    if (INSTANCE_SERIALIZATION_STATUS_Success != (status = writer.Init ()))
        return status;

    return writer.WriteInstance (*this, isStandAlone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus     IECInstance::WriteXmlToStream (IStreamP stream, bool isStandAlone)
    {
    InstanceXmlWriter writer (stream);

    InstanceSerializationStatus   status;
    if (INSTANCE_SERIALIZATION_STATUS_Success != (status = writer.Init ()))
        return status;

    return writer.WriteInstance (*this, isStandAlone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceSerializationStatus IECInstance::WriteXmlToString (bwstring & ecInstanceXml, bool isStandAlone )
    {
    InstanceSerializationStatus   status;

    CComPtr <IStream> stream;
    if (S_OK != ::CreateStreamOnHGlobal(NULL,TRUE,&stream))
        return INSTANCE_SERIALIZATION_STATUS_CantCreateStream;

    InstanceXmlWriter writer (stream);
    if (INSTANCE_SERIALIZATION_STATUS_Success != (status = writer.Init ()))
        return status;

    if (INSTANCE_SERIALIZATION_STATUS_Success != (status = writer.WriteInstance(*this, isStandAlone)))
        return status;

    LARGE_INTEGER liPos = {0};
    STATSTG statstg;
    
    ULARGE_INTEGER beginningPos;
    if (S_OK != stream->Seek(liPos, STREAM_SEEK_SET, &beginningPos))
        return INSTANCE_SERIALIZATION_STATUS_CantSetStream;

    memset (&statstg, 0, sizeof(statstg));
    if (S_OK != stream->Stat(&statstg, STATFLAG_NONAME))
        return INSTANCE_SERIALIZATION_STATUS_CantReadFromStream;
    
    wchar_t *xml = (wchar_t *) malloc((statstg.cbSize.LowPart + 1) * sizeof(wchar_t) );
    ULONG bytesRead;
    stream->Read(xml, statstg.cbSize.LowPart * sizeof(wchar_t), &bytesRead);
    xml[bytesRead / sizeof(wchar_t)] = L'\0';
    ecInstanceXml = xml;

    // There is an invisible UNICODE character in the beginning that messes things up
    ecInstanceXml = ecInstanceXml.substr(1);
    free(xml);
    return INSTANCE_SERIALIZATION_STATUS_Success;
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
void          IECRelationshipInstance::SetSource (IECInstanceP instance)
    {
    _SetSource (instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP  IECRelationshipInstance::GetSource () const
    {
    return _GetSource ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void          IECRelationshipInstance::SetTarget (IECInstanceP instance)
    {
    _SetTarget (instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP  IECRelationshipInstance::GetTarget () const
    {
    return _GetTarget ();
    }

END_BENTLEY_EC_NAMESPACE

