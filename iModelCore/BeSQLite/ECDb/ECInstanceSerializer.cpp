/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceSerializer.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (const byte value)
    {
    memcpy (m_ptr, &value, sizeof(byte));
    MoveForward (sizeof(byte));          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
byte ECInstanceSerializer::ReadByte ()
    {
    byte value;
    memcpy (&value, m_ptr, sizeof(byte));
    MoveForward (sizeof(byte));          
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (const UInt32 value)
    {
    memcpy (m_ptr, &value, sizeof(UInt32));
    MoveForward (sizeof(UInt32));          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECInstanceSerializer::ReadUInt32 ()
    {
    UInt32 value;
    memcpy (&value, m_ptr, sizeof(UInt32));
    MoveForward (sizeof(UInt32));          
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (const Int64 value)
    {
    memcpy (m_ptr, &value, sizeof(Int64));
    MoveForward (sizeof(Int64));          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/        
void ECInstanceSerializer::Write (const StructValueIdentifier value)
    {
    memcpy (m_ptr, &value, sizeof(StructValueIdentifier));
    MoveForward (sizeof(StructValueIdentifier));          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StructValueIdentifier ECInstanceSerializer::ReadStructValueIdentifier ()
    {
    StructValueIdentifier value;
    memcpy (&value, m_ptr, sizeof(StructValueIdentifier));
    MoveForward (sizeof(StructValueIdentifier));   
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/      
Int64 ECInstanceSerializer::ReadInt64 ()
    {
    Int64 value;
    memcpy (&value, m_ptr, sizeof(Int64));
    MoveForward (sizeof(Int64));          
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (byte const* value, UInt32 length)
    {
    memcpy (m_ptr, value, length);
    MoveForward (length);          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::ReadBytes (byte*& value, UInt32 length)
    {
    memcpy (value, m_ptr, length);
    MoveForward (length);          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::MoveForward(int nBytes)
    {
    m_ptr += nBytes;            
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECInstanceSerializer::GetECClass(Int64 ecClassId)
    {
    return m_ecClassResolver->GetECClass (ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceSerializer::ReadECInstance(ECClassCR ecClass)
    {
    IECInstancePtr newInstance;
    UInt32         bytesUsed;
    byte           binFlag       = ReadByte();
    StandaloneECEnablerP enabler = ecClass.GetDefaultStandaloneEnabler();

    BeAssert(enabler != nullptr);            

    if (binFlag & ECInstanceSerializer::FLAG_HasSupportingInstances)
        {
        UInt32 supportingInstanceCount = ReadUInt32 ();
        bytesUsed= ReadUInt32 ();
        newInstance = enabler->CreateInstance(bytesUsed);
        BeAssert (!newInstance.IsNull());
        MemoryECInstanceBase* mbInstance = newInstance->GetAsMemoryECInstanceP();
        BeAssert (mbInstance != nullptr);
        byte* instanceBytes  = const_cast<byte*>(mbInstance->GetData());
        ReadBytes (instanceBytes, bytesUsed);

        while (supportingInstanceCount-- > 0)
            ReadSupportingECInstance (*mbInstance);
        }
    else
        {
        bytesUsed= m_bufferSize - sizeof(byte);
        newInstance = enabler->CreateInstance(bytesUsed);
        BeAssert (!newInstance.IsNull());
        MemoryECInstanceBase* mbInstance = newInstance->GetAsMemoryECInstanceP();
        BeAssert (mbInstance != nullptr);
        byte* instanceBytes  = const_cast<byte*>(mbInstance->GetData());
        ReadBytes (instanceBytes, bytesUsed);
        }
    return newInstance;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::ReadSupportingECInstance(MemoryECInstanceBaseR parentECInstance)
    {
    UInt32                supportingInstanceCount = ReadUInt32();
    Int64                 classId                 = ReadInt64 ();
    StructValueIdentifier structValueId           = ReadStructValueIdentifier ();
    UInt32                bytesUsed            = ReadUInt32 (); 
    ECClassP              ecClass                 = GetECClass (classId);
    BeAssert(ecClass != nullptr);

    StandaloneECEnablerP enabler = ecClass->GetDefaultStandaloneEnabler();
    BeAssert(enabler != nullptr);            

    IECInstancePtr newInstance = enabler->CreateInstance(bytesUsed);
    BeAssert (!newInstance.IsNull());

    MemoryECInstanceBase* mbInstance = newInstance->GetAsMemoryECInstanceP();
    BeAssert (mbInstance != nullptr);

    byte* instanceBytes  = const_cast<byte*>(mbInstance->GetData());
    ReadBytes (instanceBytes, bytesUsed);

    parentECInstance.SetStructArrayInstance(*mbInstance, structValueId);
    while (supportingInstanceCount-- > 0)
        ReadSupportingECInstance (*mbInstance);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::WriteECInstance (MemoryECInstanceBaseCR instance)
    {
    StructValueIdentifier structValueId;
    UInt32                bytesUsed                  = instance.GetBytesUsed(); 
    UInt32                i                             = 0;
    IECInstancePtr        supportingInstance            = instance.GetStructArrayInstanceByIndex (i, structValueId);
    bool                  hasSuppportingInstances       = !supportingInstance.IsNull();
    byte                  binFlag                       = ECInstanceSerializer::FLAG_None; 
    if (hasSuppportingInstances)
        binFlag |= ECInstanceSerializer::FLAG_HasSupportingInstances;

    Write(binFlag);

    if (true == hasSuppportingInstances)
        {
        byte*  supportingInstanceCountOffset = m_ptr;
        Write (i);
        Write(bytesUsed);
        Write(instance.GetData(), bytesUsed);
        do 
            {        
            const MemoryECInstanceBase* mbInstance = supportingInstance->GetAsMemoryECInstance();
            BeAssert (nullptr != mbInstance);
            WriteSupportingECInstance(*mbInstance, structValueId);
            supportingInstance = instance.GetStructArrayInstanceByIndex (i++, structValueId);
            } while (!supportingInstance.IsNull());

        memcpy(supportingInstanceCountOffset, &i, sizeof(i));           
        }
    else
        Write(instance.GetData(), bytesUsed);
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::WriteSupportingECInstance (MemoryECInstanceBaseCR structValue, StructValueIdentifier structValueId)
    {
    UInt32    bytesUsed= structValue.GetBytesUsed(); 
    UInt32    i           = 0;
    ECClassCR type        = structValue.GetAsIECInstance()->GetClass();
    Int64     classId     = type.GetId();
 
    byte* supportingInstanceCountOffset = m_ptr;
    Write (i);
    Write (classId);
    Write (structValueId);
    Write (bytesUsed);
    Write (structValue.GetData(), bytesUsed);

    IECInstancePtr supportingInstance = structValue.GetStructArrayInstanceByIndex (i, structValueId);
    while (!supportingInstance.IsNull())
        {        
        const MemoryECInstanceBase* mbInstance = supportingInstance->GetAsMemoryECInstance();
        BeAssert (nullptr != mbInstance);
        WriteSupportingECInstance(*mbInstance, structValueId);
        supportingInstance = structValue.GetStructArrayInstanceByIndex(i++, structValueId);
        }
    memcpy (supportingInstanceCountOffset, &i, sizeof(i));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECInstanceSerializer::GetECInstanceSize (MemoryECInstanceBaseCR instance)
    {
    StructValueIdentifier structValueId;
    UInt32                i                     = 0;
    IECInstancePtr        supportingInstance    = instance.GetStructArrayInstanceByIndex (i, structValueId);
    UInt32                size                  = sizeof(byte);
    bool                  hasSupportingInstance = !supportingInstance.IsNull ();
    if (hasSupportingInstance)
        {
        size += sizeof(UInt32) * 2;
        do        
            {                
            const MemoryECInstanceBase* mbInstance = supportingInstance->GetAsMemoryECInstance ();
            BeAssert (nullptr != mbInstance);
            size += GetSupportingECInstanceSize (*mbInstance);
            supportingInstance = instance.GetStructArrayInstanceByIndex(i++, structValueId);
            } while (!supportingInstance.IsNull ());
        }
    return size + instance.GetBytesUsed();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECInstanceSerializer::GetSupportingECInstanceSize (MemoryECInstanceBaseCR instance)
    {
    StructValueIdentifier structValueId;
    UInt32                size               = (sizeof (StructValueIdentifier) + sizeof(UInt32)*2 + sizeof (UInt64)); 
    UInt32                i                  = 0;
    IECInstancePtr        supportingInstance = instance.GetStructArrayInstanceByIndex (i, structValueId);

    while (!supportingInstance.IsNull())
        {        
        const MemoryECInstanceBase* mbInstance = supportingInstance->GetAsMemoryECInstance();
        BeAssert (nullptr != mbInstance);
        size += GetSupportingECInstanceSize (*mbInstance);
        supportingInstance = instance.GetStructArrayInstanceByIndex(i++, structValueId);
        }
    return size + instance.GetBytesUsed();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceSerializer::Serialize (DbBuffer& buffer, IECInstancePtr& instance)
    {
    if (instance.IsNull())
        return false;
    MemoryECInstanceBase* mbInstance = instance->GetAsMemoryECInstanceP ();
    BeAssert (nullptr != mbInstance);
    UInt32 m_bufferSize = GetECInstanceSize (*mbInstance);
    buffer.Resize((size_t)m_bufferSize);
    m_ptr = buffer.GetData();
    WriteECInstance (*mbInstance);
    UInt32 bytesUsed = (UInt32)(m_ptr - buffer.GetData ());
    if (bytesUsed != m_bufferSize)
        BeAssert (false && "BytesNeededToStoreInstance != BytesActullyUseToWriteTheInstance");
    //BeAssert (bytesUsed != m_bufferSize);  
    m_ptr = nullptr;
    m_ecClassResolver = nullptr;
    m_bufferSize = 0;
    return true;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceSerializer::Deserialize (DbBuffer& buffer, ECClassCR ecClass, IECClassResolver& resolver)
    {
    m_ecClassResolver = &resolver;
    m_ptr = buffer.GetData();
    m_bufferSize = (UInt32)buffer.GetLength();
    IECInstancePtr instance = ReadECInstance (ecClass);
    UInt32 bytesUsed = (UInt32)(m_ptr - buffer.GetData ());
    if (bytesUsed != (UInt32)m_bufferSize)
        BeAssert (false && "BytesUseToCreateInstance != TotalBytesInBuffer");
    m_ptr = nullptr;
    m_ecClassResolver = nullptr;
    m_bufferSize = 0;
    return instance;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceSerializer::ECInstanceSerializer (): m_ptr (nullptr), m_ecClassResolver (nullptr), m_bufferSize (0) {}

END_BENTLEY_SQLITE_EC_NAMESPACE