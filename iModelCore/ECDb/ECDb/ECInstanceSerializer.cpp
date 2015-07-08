/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceSerializer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (const Byte value)
    {
    memcpy (m_ptr, &value, sizeof(Byte));
    MoveForward (sizeof(Byte));          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Byte ECInstanceSerializer::ReadByte ()
    {
    Byte value;
    memcpy (&value, m_ptr, sizeof(Byte));
    MoveForward (sizeof(Byte));          
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (const uint32_t value)
    {
    memcpy (m_ptr, &value, sizeof(uint32_t));
    MoveForward (sizeof(uint32_t));          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECInstanceSerializer::ReadUInt32 ()
    {
    uint32_t value;
    memcpy (&value, m_ptr, sizeof(uint32_t));
    MoveForward (sizeof(uint32_t));          
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (const int64_t value)
    {
    memcpy (m_ptr, &value, sizeof(int64_t));
    MoveForward (sizeof(int64_t));          
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
int64_t ECInstanceSerializer::ReadInt64 ()
    {
    int64_t value;
    memcpy (&value, m_ptr, sizeof(int64_t));
    MoveForward (sizeof(int64_t));          
    return value;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::Write (Byte const* value, uint32_t length)
    {
    memcpy (m_ptr, value, length);
    MoveForward (length);          
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::ReadBytes (Byte*& value, uint32_t length)
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
ECClassCP ECInstanceSerializer::GetECClass(int64_t ecClassId)
    {
    return m_ecClassResolver->GetECClass (ecClassId);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceSerializer::ReadECInstance(ECClassCR ecClass)
    {
    IECInstancePtr newInstance;
    uint32_t       bytesUsed;
    Byte binFlag       = ReadByte();
    StandaloneECEnablerP enabler = ecClass.GetDefaultStandaloneEnabler();

    BeAssert(enabler != nullptr);            

    if (binFlag & ECInstanceSerializer::FLAG_HasSupportingInstances)
        {
        uint32_t supportingInstanceCount = ReadUInt32 ();
        bytesUsed= ReadUInt32 ();
        newInstance = enabler->CreateInstance(bytesUsed);
        BeAssert (!newInstance.IsNull());
        MemoryECInstanceBase* mbInstance = newInstance->GetAsMemoryECInstanceP();
        BeAssert (mbInstance != nullptr);
        Byte* instanceBytes  = const_cast<Byte*>(mbInstance->GetData());
        ReadBytes (instanceBytes, bytesUsed);

        while (supportingInstanceCount-- > 0)
            ReadSupportingECInstance (*mbInstance);
        }
    else
        {
        bytesUsed= m_bufferSize - sizeof(Byte);
        newInstance = enabler->CreateInstance(bytesUsed);
        BeAssert (!newInstance.IsNull());
        MemoryECInstanceBase* mbInstance = newInstance->GetAsMemoryECInstanceP();
        BeAssert (mbInstance != nullptr);
        Byte* instanceBytes  = const_cast<Byte*>(mbInstance->GetData());
        ReadBytes (instanceBytes, bytesUsed);
        }
    return newInstance;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceSerializer::ReadSupportingECInstance(MemoryECInstanceBaseR parentECInstance)
    {
    uint32_t              supportingInstanceCount = ReadUInt32();
    int64_t               classId                 = ReadInt64 ();
    StructValueIdentifier structValueId           = ReadStructValueIdentifier ();
    uint32_t              bytesUsed            = ReadUInt32 (); 
    ECClassP              ecClass                 = GetECClass (classId);
    BeAssert(ecClass != nullptr);

    StandaloneECEnablerP enabler = ecClass->GetDefaultStandaloneEnabler();
    BeAssert(enabler != nullptr);            

    IECInstancePtr newInstance = enabler->CreateInstance(bytesUsed);
    BeAssert (!newInstance.IsNull());

    MemoryECInstanceBase* mbInstance = newInstance->GetAsMemoryECInstanceP();
    BeAssert (mbInstance != nullptr);

    Byte* instanceBytes  = const_cast<Byte*>(mbInstance->GetData());
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
    uint32_t              bytesUsed                  = instance.GetBytesUsed(); 
    uint32_t              i                             = 0;
    IECInstancePtr        supportingInstance            = instance.GetStructArrayInstanceByIndex (i, structValueId);
    bool                  hasSuppportingInstances       = !supportingInstance.IsNull();
    Byte binFlag                       = ECInstanceSerializer::FLAG_None; 
    if (hasSuppportingInstances)
        binFlag |= ECInstanceSerializer::FLAG_HasSupportingInstances;

    Write(binFlag);

    if (true == hasSuppportingInstances)
        {
        Byte*  supportingInstanceCountOffset = m_ptr;
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
    uint32_t  bytesUsed= structValue.GetBytesUsed(); 
    uint32_t  i           = 0;
    ECClassCR type        = structValue.GetAsIECInstance()->GetClass();
    int64_t   classId     = type.GetId();
 
    Byte* supportingInstanceCountOffset = m_ptr;
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
uint32_t ECInstanceSerializer::GetECInstanceSize (MemoryECInstanceBaseCR instance)
    {
    StructValueIdentifier structValueId;
    uint32_t              i                     = 0;
    IECInstancePtr        supportingInstance    = instance.GetStructArrayInstanceByIndex (i, structValueId);
    uint32_t              size                  = sizeof(Byte);
    bool                  hasSupportingInstance = !supportingInstance.IsNull ();
    if (hasSupportingInstance)
        {
        size += sizeof(uint32_t) * 2;
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
uint32_t ECInstanceSerializer::GetSupportingECInstanceSize (MemoryECInstanceBaseCR instance)
    {
    StructValueIdentifier structValueId;
    uint32_t              size               = (sizeof (StructValueIdentifier) + sizeof(uint32_t)*2 + sizeof (uint64_t)); 
    uint32_t              i                  = 0;
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
    uint32_t m_bufferSize = GetECInstanceSize (*mbInstance);
    buffer.Resize((size_t)m_bufferSize);
    m_ptr = buffer.GetData();
    WriteECInstance (*mbInstance);
    uint32_t bytesUsed = (uint32_t)(m_ptr - buffer.GetData ());
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
    m_bufferSize = (uint32_t)buffer.GetLength();
    IECInstancePtr instance = ReadECInstance (ecClass);
    uint32_t bytesUsed = (uint32_t)(m_ptr - buffer.GetData ());
    if (bytesUsed != (uint32_t)m_bufferSize)
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


//***************************** DbBuffer ************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::~DbBuffer()
    {
    Reset();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Dettach()
    {
    m_ownsBuffer = false;
    Reset();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Reset()
    {
    if (m_ownsBuffer && m_data != nullptr)
        free(m_data);

    m_data = nullptr;
    m_length = 0;
    m_ownsBuffer = false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::DbBuffer()
    :m_data(nullptr), m_length(0), m_ownsBuffer(false)
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::DbBuffer(size_t length)
    {
    Allocate(length);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbBuffer::Allocate(size_t length)
    {
    Reset();
    m_data = (Byte*) malloc(length);
    if (m_data != nullptr)
        {
        m_length = length;
        m_ownsBuffer = true;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool  DbBuffer::SetData(Byte* data, size_t length, bool createCopy)
    {
    Reset();
    if (createCopy == true)
        {
        if (Allocate(length) == true)
            {
            memcpy(m_data, data, length);
            return true;
            }
        else
            return false;
        }
    m_data = data;
    m_length = length;
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Resize(size_t length)
    {
    if (m_data != nullptr)
        {
        size_t minLength = length > m_length ? m_length : length;
        Byte* data = (Byte*) malloc(length);
        memcpy(data, m_data, minLength);
        Reset();
        m_data = data;
        m_ownsBuffer = true;
        m_length = length;
        }
    else
        Allocate(length);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE