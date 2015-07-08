/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceSerializer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbBuffer
    {
    private:
        Byte*   m_data;
        size_t  m_length;
        bool    m_ownsBuffer;
    protected:
        void     Reset();
        bool     Allocate(size_t length);

    public:
        DbBuffer();
        DbBuffer(size_t length);
        virtual ~DbBuffer();
        Byte*    GetData()   const { return m_data; }
        size_t   GetLength() const { return m_length; }
        bool     SetData(Byte* data, size_t length, bool createCopy = false);
        void     Dettach();
        void     Resize(size_t length);
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceSerializer
    {
private:
    Byte*                 m_ptr;
    IECClassResolver*     m_ecClassResolver;
    uint32_t              m_bufferSize;
    enum BinFlags ENUM_UNDERLYING_TYPE(Byte)
        {
        FLAG_None                   = 0x0,
        FLAG_HasSupportingInstances = 0x1,
        FLAG_Reserved1              = 0x2,
        FLAG_Reserved2              = 0x4,
        FLAG_Reserved3              = 0x8,
        };
private:
    void                  Write                       (const Byte value);
    void                  Write                       (const uint32_t value);
    void                  Write                       (const int64_t value);
    void                  Write                       (const StructValueIdentifier value);
    void                  Write                       (Byte const* value, uint32_t length);
    StructValueIdentifier ReadStructValueIdentifier   ();
    int64_t               ReadInt64                   ();
    Byte ReadByte                    ();
    uint32_t              ReadUInt32                  ();
    void                  ReadBytes                   (Byte*& value, uint32_t length);
    void                  MoveForward                 (int nBytes);
    ECClassP              GetECClass                  (int64_t ecClassId);
    IECInstancePtr        ReadECInstance              (ECClassCR ecClass);
    void                  ReadSupportingECInstance    (MemoryECInstanceBaseR parentECInstance);
    void                  WriteECInstance             (MemoryECInstanceBaseCR instance);
    void                  WriteSupportingECInstance   (MemoryECInstanceBaseCR structValue, StructValueIdentifier structValueId);
    uint32_t              GetECInstanceSize           (MemoryECInstanceBaseCR instance);
    uint32_t              GetSupportingECInstanceSize (MemoryECInstanceBaseCR instance);
public:
                          ECInstanceSerializer        ();
    bool                  Serialize                   (DbBufferR buffer, IECInstancePtr& instance);
    IECInstancePtr        Deserialize                 (DbBufferR buffer, ECClassCR ecClass, IECClassResolver& resolver);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE