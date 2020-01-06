/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/19
+===============+===============+===============+===============+===============+======*/
class HashFiler : public OdDbDwgFiler
{
private:
    OdInt64 m_pos;
    OdResult    m_status;
    OdDbObject const&   m_object;
    OdDbDatabase&   m_dwgdb;
    BentleyApi::MD5 m_hasher;
    bset<OdDbObjectId>  m_objectIds;

public:
// the constructor
HashFiler (OdDbObject const& o, OdDbDatabase& dwg) : m_object(o), m_dwgdb(dwg), m_pos(0), m_status(OdResult::eOk) 
    {
    m_hasher.Reset ();
    m_objectIds.clear ();
    }

// overrides from OdDbFiler
virtual FilerType filerType() const override { return OdDbFiler::kBagFiler; }
virtual OdDbDatabase* database() const override { return &m_dwgdb; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void seek(OdInt64 offset, OdDb::FilerSeekType seekType) override
    {
    switch (seekType)
        {
        case OdDb::kSeekFromStart:  m_pos = 0; break;
        case OdDb::kSeekFromCurrent: m_pos += offset; break;
        case OdDb::kSeekFromEnd: m_pos -= offset; break;
        }
    }

// Nop for reads
virtual OdUInt64 tell() const override { return m_pos; }
virtual bool rdBool() override { return false; }
virtual OdString rdString() override { return L""; }
virtual void rdBytes(void* buffer, OdUInt32 numBytes) override {}
virtual OdInt8 rdInt8() override { return 0; }
virtual OdUInt8 rdUInt8() override { return 0; }
virtual OdInt16 rdInt16() override { return 0; }
virtual OdInt32 rdInt32() override { return 0; }
virtual OdInt64 rdInt64() override { return 0; }
virtual double rdDouble() override { return 0; }
virtual OdDbHandle rdDbHandle() override { return OdDbHandle(); }
virtual OdDbObjectId rdSoftOwnershipId() override { return OdDbObjectId::kNull; }
virtual OdDbObjectId rdHardOwnershipId() override { return OdDbObjectId::kNull; }
virtual OdDbObjectId rdHardPointerId() override { return OdDbObjectId::kNull; }
virtual OdDbObjectId rdSoftPointerId() override { return OdDbObjectId::kNull; }
virtual OdGePoint2d rdPoint2d() override { return OdGePoint2d::kOrigin; }
virtual OdGePoint3d rdPoint3d() override { return OdGePoint3d::kOrigin; }
virtual OdGeVector2d rdVector2d() override { return OdGeVector2d::kIdentity; }
virtual OdGeVector3d rdVector3d() override { return OdGeVector3d::kIdentity; }
virtual OdGeScale3d rdScale3d() override { return OdGeScale3d::kIdentity; }
virtual void addRef() override {;}
virtual void release() override {;}

// implement writes
virtual void wrBool(bool value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrString(const OdString& value) override { m_hasher.Add(value.c_str(), value.getLength() * sizeof(OdChar)); }
virtual void wrBytes(const void* buffer, OdUInt32 numBytes) override { m_hasher.Add(buffer, numBytes); }
virtual void wrInt8(OdInt8 value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrUInt8(OdUInt8 value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrInt16(OdInt16 value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrInt32(OdInt32 value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrInt64(OdInt64 value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrAddress(const void* value) { OdDbDwgFiler::wrAddress(value); }
virtual void wrDouble(double value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrSoftOwnershipId(const OdDbObjectId& value) override { this->AddObjectId(value); }
virtual void wrHardOwnershipId(const OdDbObjectId& value) override { this->AddObjectId(value); }
virtual void wrSoftPointerId(const OdDbObjectId& value) override { this->AddObjectId(value); }
virtual void wrHardPointerId(const OdDbObjectId& value) override { this->AddObjectId(value); }
virtual void wrPoint2d(const OdGePoint2d& value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrPoint3d(const OdGePoint3d& value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrVector2d(const OdGeVector2d& value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrVector3d(const OdGeVector3d& value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrScale3d(const OdGeScale3d& value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrExtrusion(const OdGeVector3d& value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void wrThickness(double value) override { m_hasher.Add(&value, sizeof(value)); }
virtual void addReference(OdDbObjectId id, OdDb::ReferenceType rt) override { this->AddObjectId(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void wrDbHandle(const OdDbHandle& handle) override 
    {
    OdUInt8 bytes[8] = {0};
    handle.bytes (bytes);
    m_hasher.Add(bytes, 8); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AddObjectId (const OdDbObjectId& objectId)
    {
    this->wrDbHandle (objectId.getHandle());
    m_objectIds.insert (objectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::MD5::HashVal GetHashValue ()
    {
    return  m_hasher.GetHashVal();
    }
};  // HashFiler

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/19
+===============+===============+===============+===============+===============+======*/
struct C3dObjectProvenance
{
private:
    BentleyApi::MD5::HashVal&   m_outputHash;
    DwgDbObjectCR   m_inputObject;
    C3dImporter&    m_importer;
    
public:
// the constructor
explicit C3dObjectProvenance(BentleyApi::MD5::HashVal& h, DwgDbObjectCR o, C3dImporter& i) : m_outputHash(h), m_inputObject(o), m_importer(i)
    {
    ::memset (m_outputHash.m_buffer, 0, MD5::HashBytes);
    this->CreateOutputHash ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsNull () const
    {
    for (size_t i = 0; i < BentleyApi::MD5::HashBytes; i++)
        {
        if (m_outputHash.m_buffer[i] != 0)
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateOutputHash ()
    {
    auto dwg = m_inputObject.database ();
    if (dwg == nullptr)
        dwg = m_importer.GetDwgDbP ();
    if (dwg == nullptr)
        return;

    HashFiler   filer(m_inputObject, *dwg);
    m_inputObject.dwgOut (&filer);

    m_outputHash = filer.GetHashValue ();
    }
};  // C3dObjectProvenance

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    C3dImporter::_CreateObjectProvenance (BentleyApi::MD5::HashVal& hash, DwgDbObjectCR object)
    {
    if (object.isKindOf(AECCDbAlignment::desc()))
        {
        C3dObjectProvenance prov(hash, object, *this);
        return  !prov.IsNull();
        }
    return  false;
    }

END_C3D_NAMESPACE
