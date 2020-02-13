/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    <Dwg/DwgDb/BasicTypes.h>

BEGIN_DWGDB_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgResBuf
#ifdef DWGTOOLKIT_OpenDwg
                    : public OdResBuf
    {
public:
    DEFINE_T_SUPER (OdResBuf)
    ODRX_DECLARE_MEMBERS (DwgResBuf);

    DWGDB_EXPORT DwgResBuf () : T_Super() {}
    DWGDB_EXPORT DwgResBuf (const OdResBuf* rb) { T_Super::setResBuf(rb); }

#elif DWGTOOLKIT_RealDwg
                    : public resbuf, public RefCounted<IRefCounted>
    {
public:
    DEFINE_T_SUPER (resbuf)

    DWGDB_EXPORT DwgResBuf () { rbnext = nullptr; restype = 0; resval.rreal = 0.0; }
#endif  // DWGTOOLKIT_

private:
    DwgResBuf (DwgResBuf& copy);
    DwgResBuf& operator = (DwgResBuf& copy);

public:
    DWGDB_EXPORT ~DwgResBuf ();

    //! Keep the enum values as they are - they will be static casted to/from a toolkit:
    enum class DataType
        {
        None                = 0,
        Double              = 1,
        Integer32           = 2,
        Integer16           = 3,
        Integer8            = 4,
        Text                = 5,
        BinaryChunk         = 6,
        Handle              = 7,
        HardOwnershipId     = 8,
        SoftOwnershipId     = 9,
        HardPointerId       = 10,
        SoftPointerId       = 11,
        Point3d             = 12,
        Integer64           = 13,
        NotRecognized       = 19
        };  // DataType

    DWGDB_EXPORT DwgResBuf*     Start ();
    DWGDB_EXPORT DwgResBuf*     Next ();
    DWGDB_EXPORT DwgResBuf*     End ();
    DWGDB_EXPORT DataType       GetDataType ();
    DWGDB_EXPORT int8_t         GetInteger8 ();
    DWGDB_EXPORT int16_t        GetInteger16 ();
    DWGDB_EXPORT int32_t        GetInteger32 ();
    DWGDB_EXPORT int64_t        GetInteger64 ();
    DWGDB_EXPORT double         GetDouble ();
    DWGDB_EXPORT bool           GetBoolean ();  // from int8
    DWGDB_EXPORT DwgString      GetString ();
    DWGDB_EXPORT DwgDbStatus    GetBinaryData (DwgBinaryData& out);
    DWGDB_EXPORT DwgDbStatus    GetPoint3d (DPoint3dR out);
    DWGDB_EXPORT DwgDbHandle    GetHandle ();
    DWGDB_EXPORT DwgDbObjectId  GetObjectId ();
    //! A regapp entry is of type DataType::Text - call this method to tell the difference between 1001 from 1000
    DWGDB_EXPORT bool           IsRegappName ();
    };  // DwgResBuf
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgResBuf)

/*=================================================================================**//**
This is a refCounted pointer that points to the header of a DwgResBuf. Example to 
iterate through the xdata on an entity:
  
@verbatim
  DwgResBufIterator iter = entity.GetXData (L"MyRegapp");
  if (iter.IsValid())
    {
    for (DwgResBufP curr = iter->Start(); curr != iter->End(); curr = curr->Next())
        {
        switch (curr->GetDataType())
             {
             case DwgResBuf::DataType::Text:
                if (curr->IsRegappName())
                    regappName = curr->GetString();
                else
                    textString = curr->GetString();
            ...
    }
@endverbatim
 
 @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgResBufIterator
#ifdef DWGTOOLKIT_OpenDwg
                    : public OdSmartPtr<DwgResBuf>
    {
    DEFINE_T_SUPER (OdSmartPtr)
public:
    DWGDB_EXPORT DwgResBufIterator () : OdSmartPtr<DwgResBuf>() { ; }
    DWGDB_EXPORT DwgResBufIterator (const OdResBufPtr& resbuf) : OdSmartPtr<DwgResBuf>(resbuf) { ; }
    DWGDB_EXPORT DwgResBufIterator (const OdResBuf* resbuf) : OdSmartPtr<DwgResBuf>(resbuf) { ; }

    DWGDB_EXPORT bool       IsNull () const { return T_Super::isNull(); }
    DWGDB_EXPORT bool       IsValid() const { return !T_Super::isNull(); }

//__PUBLISH_SECTION_END__
    static DWGDB_EXPORT DwgResBufIterator   CreateFrom (const OdResBuf* odResbuf);
//__PUBLISH_SECTION_START__

#elif DWGTOOLKIT_RealDwg
                    : public RefCountedPtr<DwgResBuf>
    {
public:
    DwgResBufIterator () : RefCountedPtr() {}
    DwgResBufIterator (DwgResBufP rb) : RefCountedPtr(rb) {}

//__PUBLISH_SECTION_END__
    static DWGDB_EXPORT DwgResBufIterator   CreateFromAndFree (struct resbuf* rb);
//__PUBLISH_SECTION_START__
#endif  // DWGTOOLKIT_
    };  // DwgResBufIterator
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgResBufIterator)

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
