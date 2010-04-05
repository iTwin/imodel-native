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
ECEnablerCR           IECInstance::GetEnabler() const
    {
    return _GetEnabler();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
bool                IECInstance::IsReadOnly() const
    {
    return _IsReadOnly();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 index) const
    {
    return _GetValue (v, propertyAccessString, index);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::GetValue (ECValueR v, const wchar_t * propertyAccessString) const
    {
    return _GetValue (v, propertyAccessString);
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index)
    {
    return _SetValue (propertyAccessString, v, index);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt           IECInstance::SetValue (const wchar_t * propertyAccessString, ECValueCR v)
    {
    return _SetValue (propertyAccessString, v);
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           IECInstance::GetLong (Int64 & value, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v;
    StatusInt status;
    if (nIndices == 1)
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
        
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
    
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
        status = _GetValue (v, propertyAccessString, *indices);
    else
        status = _GetValue (v, propertyAccessString);
    
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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);    

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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetPoint2DValue  (const wchar_t * propertyAccessString, DPoint2dR value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

    return status;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IECInstance::SetPoint3DValue  (const wchar_t * propertyAccessString, DPoint3dR value, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (nIndices <= 1 && "Access strings containing nested arrays are not yet implemented", ECOBJECTS_STATUS_OperationNotSupported);
    ECValue v(value);
    StatusInt status;
    if (nIndices == 1)
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

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
        status = _SetValue (propertyAccessString, v, *indices);
    else
        status = _SetValue (propertyAccessString, v);

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
