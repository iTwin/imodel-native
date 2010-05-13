/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECInstance.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE
    
UInt32 g_totalAllocs = 0;
UInt32 g_totalFrees  = 0;
UInt32 g_currentLive = 0;

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

    Logger::GetLogger()->debugv (L"%s Live IECInstances: %d, Total Allocs: %d, TotalFrees: %d", prefix, g_currentLive, g_totalAllocs, g_totalFrees);
#ifdef DEBUG_INSTANCE_LEAKS
    for each (DebugInstanceLeakMap::value_type leak in g_debugInstanceLeakMap)
        {
        IECInstance* leakedInstance = leak.first;
        UInt32    orderOfAllocation = leak.second;
        Logger::GetLogger()->debugv (L"Leaked the %dth IECInstance that was allocated.", orderOfAllocation);
        leakedInstance->Dump();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsExcluded(std::wstring& className, std::vector<std::wstring> classNamesToExclude)
    {
    for each (std::wstring excludedClass in classNamesToExclude)
        {
        if (0 == className.compare (excludedClass))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_ReportLeaks(std::vector<std::wstring> classNamesToExclude)
    {
#ifdef DEBUG_INSTANCE_LEAKS
    for each (DebugInstanceLeakMap::value_type leak in g_debugInstanceLeakMap)
        {
        IECInstance* leakedInstance = leak.first;
        UInt32    orderOfAllocation = leak.second;
        
        std::wstring className = leakedInstance->GetClass().GetName();
        if (IsExcluded (className, classNamesToExclude))
            continue;
        
        Logger::GetLogger()->errorv (L"Leaked the %dth IECInstance that was allocated: ECClass=%s, InstanceId=%s", 
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
std::wstring        IECInstance::GetInstanceId() const
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

StatusInt           IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString) const { return _GetValue (v, propertyAccessString, false, 0); }
StatusInt           IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 arrayIndex) const { return _GetValue (v, propertyAccessString, true, arrayIndex); }
StatusInt           IECInstance::GetValue (ECValueR v, UInt32 propertyIndex) const { return _GetValue (v, propertyIndex, false, 0); }
StatusInt           IECInstance::GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const { return _GetValue (v, propertyIndex, true, arrayIndex); }
StatusInt           IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v) { return _SetValue (propertyAccessString, v, false, 0); }
StatusInt           IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 arrayIndex) { return _SetValue (propertyAccessString, v, true, arrayIndex); }
StatusInt           IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v) { return _SetValue (propertyIndex, v, false, 0); }
StatusInt           IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) { return _SetValue (propertyIndex, v, true, arrayIndex); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::GetLong (Int64 & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)
        value = v.GetLong();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StatusInt           IECInstance::GetInteger (int & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)
        value = v.GetInteger();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/                
StatusInt           IECInstance::GetDouble (double& value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)        
        value = v.GetDouble();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
StatusInt           IECInstance::GetString (const wchar_t * & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)
        value = v.GetString();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::GetBoolean (bool & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)
        value = v.GetBoolean();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::GetPoint2D (DPoint2d & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)
        value = v.GetPoint2D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::GetPoint3D (DPoint3d & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
        
    if (status == SUCCESS)
        value = v.GetPoint3D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::GetDateTime (SystemTime & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
    
    if (status == SUCCESS)
        value = v.GetDateTime();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::GetDateTimeTicks (Int64 & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = GetValue (v, propertyAccessString, *indices);
    else
        status = GetValue (v, propertyAccessString);
    
    if (status == SUCCESS)
        value = v.GetDateTimeTicks();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
StatusInt           IECInstance::SetLongValue (const wchar_t * propertyAccessString, Int64 value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
StatusInt           IECInstance::SetIntegerValue (const wchar_t * propertyAccessString, int value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/       
StatusInt           IECInstance::SetDoubleValue (const wchar_t * propertyAccessString, double value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
StatusInt           IECInstance::SetStringValue  (const wchar_t * propertyAccessString, const wchar_t * value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value, false);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetBooleanValue  (const wchar_t * propertyAccessString, bool value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetPoint2DValue  (const wchar_t * propertyAccessString, DPoint2dCR value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetPoint3DValue  (const wchar_t * propertyAccessString, DPoint3dCR value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetDateTimeValue (const wchar_t * propertyAccessString, SystemTime& value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetDateTimeTicks (const wchar_t * propertyAccessString, Int64 value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    v.SetDateTimeTicks(value);

    StatusInt status;
    if (nIndices == 1)
        status = SetValue (propertyAccessString, v, *indices);
    else
        status = SetValue (propertyAccessString, v);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size)
    {
    return _InsertArrayElements (propertyAccessString, index, size);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::AddArrayElements (const wchar_t * propertyAccessString, UInt32 size)
    {
    return _AddArrayElements (propertyAccessString, size);
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index)
    {
    return _RemoveArrayElement (propertyAccessString, index);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::ClearArray (const wchar_t * propertyAccessString)
    {
    return _ClearArray (propertyAccessString);
    }           
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                IECInstance::Dump () const
    {
    _Dump();
    }

END_BENTLEY_EC_NAMESPACE




 
#include <xmllite.h>
#include <atlbase.h>

BEGIN_BENTLEY_EC_NAMESPACE



// =====================================================================================
// StringSaver class
// =====================================================================================
struct  StringSaver
{
private:
    std::wstring    m_savedString;
    std::wstring&   m_stringDestination;
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
StringSaver (std::wstring& stringToSave) : m_stringDestination (stringToSave)
    {
    // save the contents.
    m_savedString = stringToSave;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
~StringSaver()
    {
    m_stringDestination = m_savedString;
    }
};



typedef std::vector<byte>   T_ByteArray;

// =====================================================================================
// InstanceXMLReader class
// =====================================================================================
static const wchar_t INSTANCEID_ATTRIBUTE[]         = L"instanceID";
static const wchar_t SOURCECLASS_ATTRIBUTE[]        = L"sourceClass";
static const wchar_t SOURCEINSTANCEID_ATTRIBUTE[]   = L"sourceInstanceID";
static const wchar_t TARGETCLASS_ATTRIBUTE[]        = L"targetClass";
static const wchar_t TARGETINSTANCEID_ATTRIBUTE[]   = L"targetInstanceID";
static const wchar_t XMLNS_ATTRIBUTE[]              = L"xmlns";

struct  InstanceXmlReader
{

private:
    std::wstring                m_fileName;
    CComPtr <IStream>           m_stream;
    CComPtr <IXmlReader>        m_xmlReader;
    ECSchemaPtr                 m_schema;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECSchemaP schema, CComPtr <IStream> stream)
    {
    m_schema                = schema;
    m_stream                = stream;
    m_xmlReader             = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECSchemaP schema, std::wstring fileName)
    {
    m_schema                = schema;
    m_fileName              = fileName;
    m_stream                = NULL;
    m_xmlReader             = NULL;
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
            return INSTANCE_DESERIALIZATION_STATUS_CantCreateStream;
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
    if (NULL == (foundClass = m_schema->GetClassP (className)))
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
            const wchar_t*  schemaName = m_schema->Name.c_str();
            assert (0 == wcsncmp (schemaName, nameSpace, wcslen (schemaName)));
            }

        else
            {
            // unexpected attribute.
            assert (false);
            }

        // the namespace should agree with the schema name.
        }
    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadInstanceOrStructMembers (ECClassCR ecClass, IECInstanceP ecInstance, std::wstring* baseAccessString)
    {
    // On entry, the reader is positioned in the content of an instance or struct.

    // if it's an empty node, all members of the instance are NULL.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_DESERIALIZATION_STATUS_Success;

    HRESULT         status;
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
InstanceDeserializationStatus   ReadProperty (ECClassCR ecClass, IECInstanceP ecInstance, std::wstring* baseAccessString)
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

    // set up to save and restore the current access string.
    PrimitiveECPropertyP    primitiveProperty;
    ArrayECPropertyP        arrayProperty;
    StructECPropertyP       structProperty;
    if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
        return ReadPrimitiveProperty (primitiveProperty, ecInstance, baseAccessString);
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
InstanceDeserializationStatus   ReadEmbeddedStructProperty (StructECPropertyP structProperty, IECInstanceP ecInstance, std::wstring* baseAccessString)
    {
    // empty element OK for struct - all members are null.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_DESERIALIZATION_STATUS_Success;

    std::wstring    thisAccessString;
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
InstanceDeserializationStatus   ReadPrimitiveProperty (PrimitiveECPropertyP primitiveProperty, IECInstanceP ecInstance, std::wstring* baseAccessString)
    {
    // on entry, we are positioned in the PrimitiveProperty element.
    PrimitiveType                   propertyType = primitiveProperty->Type;
    InstanceDeserializationStatus   ixrStatus;
    ECValue         ecValue;
    if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, propertyType)))
        return ixrStatus;

    StatusInt setStatus;
    if (NULL == baseAccessString)
        {
        setStatus = ecInstance->SetValue (primitiveProperty->Name.c_str(), ecValue);
        }
    else
        {
        std::wstring compoundAccessString;
        AppendAccessString (compoundAccessString, *baseAccessString, primitiveProperty->Name);
        setStatus = ecInstance->SetValue (compoundAccessString.c_str(), ecValue);
        }
    assert (SUCCESS == setStatus);

    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadArrayProperty (ArrayECPropertyP arrayProperty, IECInstanceP ecInstance, std::wstring* baseAccessString)
    {
    // on entry, the reader is positioned at the element that indicates the start of the array.
    // empty element OK for array - no members.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_DESERIALIZATION_STATUS_Success;

    std::wstring    accessString;
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
                        return INSTANCE_DESERIALIZATION_STATUS_BadArrayElement;

                    // now we know the type and we are positioned at the element containing the value.
                    // read it, populating the ECInstance using accessString and arrayIndex.
                    InstanceDeserializationStatus   ixrStatus;
                    ECValue                         ecValue;
                    if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, memberType)))
                        return ixrStatus;

                    ecInstance->AddArrayElements (accessString.c_str(), 1);
                    StatusInt   setStatus;
                    if (SUCCESS != (setStatus = ecInstance->SetValue (accessString.c_str(), ecValue, index)))
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
InstanceDeserializationStatus   ReadStructArrayMember (ECClassCR structClass, IECInstanceP owningInstance, std::wstring& accessString, UInt32 index)
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
    StatusInt setStatus = owningInstance->SetValue (accessString.c_str(), structValue, index);
    assert (SUCCESS == setStatus);

    return INSTANCE_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ReadPrimitiveValue (ECValueR ecValue, PrimitiveType propertyType)
    {
    // The reader is positioned at the XmlNodeType_Element that holds the value. 
    // We expect to find a Text element with the value, advance until we do.

    // we don't expect an empty element when reading a primitive.
    if (m_xmlReader->IsEmptyElement())
        {
        assert (false);
        return INSTANCE_DESERIALIZATION_STATUS_EmptyElement;
        }

    HRESULT         status;
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
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_EmptyElement;

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
            InstanceDeserializationStatus   ixrStatus;
            if (INSTANCE_DESERIALIZATION_STATUS_Success != (ixrStatus = ConvertStringToByteArray (byteArray, propertyValueString)))
                return ixrStatus;

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
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_BadTimeValue;
                }

            ecValue.SetDateTimeTicks (ticks);
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            double  doubleValue;
            if (1 != swscanf (propertyValueString, L"%lg", &doubleValue))
                {
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_BadDoubleValue;
                }
            ecValue.SetDouble (doubleValue);
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            Int32   intValue;
            if (1 != swscanf (propertyValueString, L"%d", &intValue))
                {
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_BadIntegerValue;
                }
            ecValue.SetInteger (intValue);
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64   longValue;
            if (1 != swscanf (propertyValueString, L"%I64d", &longValue))
                {
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_BadLongValue;
                }
            ecValue.SetLong (longValue);
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d point2d;
            if (2 != swscanf (propertyValueString, L"%lg,%lg", &point2d.x, &point2d.y))
                {
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_BadPoint2dValue;
                }
            ecValue.SetPoint2D (point2d);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            if (3 != swscanf (propertyValueString, L"%lg,%lg,%lg", &point3d.x, &point3d.y, &point3d.z))
                {
                assert (false);
                return INSTANCE_DESERIALIZATION_STATUS_BadPoint3dValue;
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
void            AppendAccessString (std::wstring& compoundAccessString, std::wstring& baseAccessString, const std::wstring& propertyName)
    {
    compoundAccessString = baseAccessString;
    compoundAccessString.append (propertyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   ConvertStringToByteArray (T_ByteArray& byteData, const wchar_t* stringData)
    {
    // the length of stringData should be a muttiple of four.
    size_t  stringLen = wcslen (stringData);
    if (0 != (stringLen % 4))
        {
        assert (false);
        return INSTANCE_DESERIALIZATION_STATUS_BadBinaryData;
        }

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
                assert ( stringLen - (iPos + jPos) < 2);
                numBytesToPush = jPos;
                break;
                }
            else
                {
                assert (false);
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
    switch (expectedType)
        {
        case PRIMITIVETYPE_Binary:
            return (0 == wcscmp (typeFound, L"binary"));

        case PRIMITIVETYPE_Boolean:
            return (0 == wcscmp (typeFound, L"boolean"));

        case PRIMITIVETYPE_DateTime:
            return (0 == wcscmp (typeFound, L"dateTime"));

        case PRIMITIVETYPE_Double:
            return (0 == wcscmp (typeFound, L"double"));

        case PRIMITIVETYPE_Integer:
            return (0 == wcscmp (typeFound, L"int"));

        case PRIMITIVETYPE_Long:
            return (0 == wcscmp (typeFound, L"long"));

        case PRIMITIVETYPE_Point2D:
            return (0 == wcscmp (typeFound, L"point2d"));

        case PRIMITIVETYPE_Point3D:
            return (0 == wcscmp (typeFound, L"point3d"));

        case PRIMITIVETYPE_String:
            return (0 == wcscmp (typeFound, L"string"));
        }

    assert (false);
    return false;
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
    if (NULL == (classFound = m_schema->GetClassP (typeFound)))
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeserializationStatus   IECInstance::ReadXmlFromFile (IECInstancePtr& ecInstance, const wchar_t* fileName, ECSchemaP schema)
    {
    InstanceXmlReader reader (schema, fileName);
    reader.Init ();
    return reader.ReadInstance (ecInstance);
    }

END_BENTLEY_EC_NAMESPACE

