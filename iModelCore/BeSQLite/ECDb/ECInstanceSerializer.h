/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECInstanceSerializer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceSerializer
    {
private:
    byte*                 m_ptr;
    IECClassResolver*     m_ecClassResolver;
    UInt32                m_bufferSize;
    enum BinFlags ENUM_UNDERLYING_TYPE(byte)
        {
        FLAG_None                   = 0x0,
        FLAG_HasSupportingInstances = 0x1,
        FLAG_Reserved1              = 0x2,
        FLAG_Reserved2              = 0x4,
        FLAG_Reserved3              = 0x8,
        };
private:
    void                  Write                       (const byte   value);
    void                  Write                       (const UInt32 value);
    void                  Write                       (const Int64  value);
    void                  Write                       (const StructValueIdentifier value);
    void                  Write                       (byte const* value, UInt32 length);
    StructValueIdentifier ReadStructValueIdentifier   ();
    Int64                 ReadInt64                   ();
    byte                  ReadByte                    ();
    UInt32                ReadUInt32                  ();
    void                  ReadBytes                   (byte*& value, UInt32 length);
    void                  MoveForward                 (int nBytes);
    ECClassP              GetECClass                  (Int64 ecClassId);
    IECInstancePtr        ReadECInstance              (ECClassCR ecClass);
    void                  ReadSupportingECInstance    (MemoryECInstanceBaseR parentECInstance);
    void                  WriteECInstance             (MemoryECInstanceBaseCR instance);
    void                  WriteSupportingECInstance   (MemoryECInstanceBaseCR structValue, StructValueIdentifier structValueId);
    UInt32                GetECInstanceSize           (MemoryECInstanceBaseCR instance);
    UInt32                GetSupportingECInstanceSize (MemoryECInstanceBaseCR instance);
public:
                          ECInstanceSerializer        ();
    bool                  Serialize                   (DbBufferR buffer, IECInstancePtr& instance);
    IECInstancePtr        Deserialize                 (DbBufferR buffer, ECClassCR ecClass, IECClassResolver& resolver);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE