/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

#ifdef DWGTOOLKIT_OpenDwg

ODRX_CONS_DEFINE_MEMBERS (DwgResBuf, OdResBuf, RXIMPL_CONSTR)

DwgResBuf::~DwgResBuf () { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgResBufIterator   DwgResBufIterator::CreateFrom (const OdResBuf* odResbuf)
    {
    if (nullptr != odResbuf)
        {
        DwgResBufIterator   iter = DwgResBuf::createObject().get();
        if (iter.IsValid())
            {
            iter->copyFrom (odResbuf);
            iter->insert (odResbuf->next());
            return  iter;
            }
        }
    return  nullptr;
    }

#elif DWGTOOLKIT_RealDwg

DwgResBuf::~DwgResBuf () { acutRelRb(rbnext); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgResBufIterator   DwgResBufIterator::CreateFromAndFree (struct resbuf* rdResbuf)
    {
    /*--------------------------------------------------------------------------------------------------
    RealDWG's resbuf is a simple linked list as opposed to an RTTI. We must instantiate our refCounted 
    pointer from the sub-class in order to capture all inheritances. We copy the header from the input
    linked list, free the input header and keep rest of the chain from the linked list.  The new header 
    will act as an "iterator" which provides direct access to RealDWG's linked list.
    --------------------------------------------------------------------------------------------------*/
    if (nullptr == rdResbuf)
        return  nullptr;

    DwgResBufIterator   header = new DwgResBuf ();
    if (header.IsNull())
        {
        acutRelRb (rdResbuf);
        return  nullptr;
        }

    // copy the header data
    header->rbnext = rdResbuf->rbnext;
    header->restype = rdResbuf->restype;
    header->resval = rdResbuf->resval;

    // allocate & copy union values as necessary:
    AcDb::DwgDataType   type = acdbGroupCodeToType (rdResbuf->restype);
    if ((type == AcDb::kDwgText || type == AcDb::kDwgHandle) && nullptr != rdResbuf->resval.rstring)
        {
        header->resval.rstring = wcsdup (rdResbuf->resval.rstring);
        if (nullptr == header->resval.rstring)
            {
            acutRelRb (rdResbuf);
            return  nullptr;
            }
        }
    else if (type == AcDb::kDwgBChunk && rdResbuf->resval.rbinary.clen > 0 && nullptr != rdResbuf->resval.rbinary.buf)
        {
        header->resval.rbinary.buf = static_cast<char*> (malloc(rdResbuf->resval.rbinary.clen));
        if (nullptr == header->resval.rbinary.buf)
            {
            acutRelRb (rdResbuf);
            return  nullptr;
            }
        memcpy (header->resval.rbinary.buf, rdResbuf->resval.rbinary.buf, rdResbuf->resval.rbinary.clen);
        }

    // we have a valid header - only free the input header but keep the rest of the chain:
    rdResbuf->rbnext = nullptr;
    acutRelRb (rdResbuf);

    return  header;
    }
#endif // DWGTOOLKIT_

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int8_t          DwgResBuf::GetInteger8 ()
    {
    int8_t  val = 0;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = static_cast<int8_t> (T_Super::getInt8());
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong XData type Integer8!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgInt8 && "Wrong XData type Integer8!");
    val = static_cast<int8_t> (T_Super::resval.rint);
#endif  // DWGTOOLKIT_

    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int16_t         DwgResBuf::GetInteger16 ()
    {
    int16_t  val = 0;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = static_cast<int16_t> (T_Super::getInt16());
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type Integer16!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgInt16 && "Wrong XData type Integer16!");
    val = static_cast<int16_t> (T_Super::resval.rint);
#endif  // DWGTOOLKIT_

    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         DwgResBuf::GetInteger32 ()
    {
    int32_t val = 0;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = static_cast<int32_t> (T_Super::getInt32());
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type Integer32!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgInt32 && "Wrong XData type Integer32!");
    val = T_Super::resval.rlong;
#endif  // DWGTOOLKIT_

    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t         DwgResBuf::GetInteger64 ()
    {
    int64_t val = 0;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = static_cast<int64_t> (T_Super::getInt64());
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type Integer64!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgInt64 && "Wrong XData type Integer64!");
    val = T_Super::resval.mnInt64;
#endif  // DWGTOOLKIT_

    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgResBuf::GetDouble ()
    {
    double  val = 0.0;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = T_Super::getDouble ();
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type Double!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgReal && "Wrong XData type Double!");
    val = T_Super::resval.rreal;
#endif  // DWGTOOLKIT_

    return  val;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgResBuf::GetBoolean ()
    {
    bool    val = false;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = T_Super::getBool ();
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type Boolean!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgInt8 && "Wrong XData type Boolean!");
    val = 0 != T_Super::resval.rint;
#endif  // DWGTOOLKIT_

    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbHandle     DwgResBuf::GetHandle ()
    {
    DwgDbHandle val;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = T_Super::getHandle ();
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type Handle!");
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgHandle && "Wrong XData type Handle!");
    val = AcDbHandle (T_Super::resval.rstring);
#endif  // DWGTOOLKIT_

    return  val;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgResBuf::GetObjectId ()
    {
    DwgDbObjectId   val;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        val = T_Super::getObjectId (nullptr);
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type ObjectId!");
        }
#elif DWGTOOLKIT_RealDwg

    int32_t     acType = acdbGroupCodeToType (T_Super::restype);
    BeAssert(acType > 7 && acType < 12 && "Wrong XData type ObjectId!");

    val = reinterpret_cast<AcDbStub*> (T_Super::resval.mnLongPtr);
#endif  // DWGTOOLKIT_

    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgResBuf::GetString ()
    {
    DwgString   string;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        OdString    odString = T_Super::getString ();
        if (!odString.isEmpty())
            string = odString;
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type String!");
        }

#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgText && "Wrong XData type String!");
    if (nullptr != T_Super::resval.rstring)
        string.Assign (T_Super::resval.rstring);
#endif  // DWGTOOLKIT_

    return  string;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgResBuf::GetBinaryData (DwgBinaryData& out)
    {
    DwgDbStatus     status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        OdBinaryData    odbin = T_Super::getBinaryChunk  ();
        if (odbin.isEmpty())
            {
            out.Clear ();
            status = DwgDbStatus::InvalidData;
            }
        else
            {
            out.Set (odbin.getPtr(), odbin.length());
            }
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type BinaryData!");
        out.Clear ();
        status = DwgDbStatus::InvalidData;
        }

#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwgBChunk && "Wrong XData type BinaryChunk!");
    if (T_Super::resval.rbinary.buf == nullptr || T_Super::resval.rbinary.clen == 0)
        {
        out.Clear ();
        status = DwgDbStatus::InvalidData;
        }
    else
        {
        out.Set (T_Super::resval.rbinary.buf, T_Super::resval.rbinary.clen);
        }
#endif  // DWGTOOLKIT_

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgResBuf::GetPoint3d (DPoint3dR out)
    {
#ifdef DWGTOOLKIT_OpenDwg
    try
        {
        out = Util::DPoint3dFrom (T_Super::getPoint3d());
        }
    catch (...)
        {
        DwgToolkitHost::GetHost().warning (L"Exception thrown on wrong type point3d!");
        out.Zero ();
        return  DwgDbStatus::InvalidData;
        }
#elif DWGTOOLKIT_RealDwg

    BeAssert(acdbGroupCodeToType(T_Super::restype) == AcDb::kDwg3Real && "Wrong XData type Point3d!");
    out.Init (T_Super::resval.rpoint[0], T_Super::resval.rpoint[1], T_Super::resval.rpoint[2]);
#endif
    return  DwgDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgResBuf::DataType      DwgResBuf::GetDataType ()
    {
    DataType            ourType = DataType::None;
#ifdef DWGTOOLKIT_OpenDwg

    OdDxfCode::Type     odType = OdDxfCode::_getType (T_Super::restype());
    switch (odType)
        {
        case OdDxfCode::Unknown:            ourType = DataType::None;               break;
        case OdDxfCode::Name:
        case OdDxfCode::String:
        case OdDxfCode::LayerName:          ourType = DataType::Text;               break;
        case OdDxfCode::Bool:
        case OdDxfCode::Integer8:           ourType = DataType::Integer8;           break;
        case OdDxfCode::Integer16:          ourType = DataType::Integer16;          break;
        case OdDxfCode::Integer32:          ourType = DataType::Integer32;          break;
        case OdDxfCode::Integer64:          ourType = DataType::Integer64;          break;
        case OdDxfCode::Double:
        case OdDxfCode::Angle:              ourType = DataType::Double;             break;
        case OdDxfCode::Point:              ourType = DataType::Point3d;            break;
        case OdDxfCode::BinaryChunk:        ourType = DataType::BinaryChunk;        break;
        case OdDxfCode::Handle:             ourType = DataType::Handle;             break;
        case OdDxfCode::ObjectId:
        case OdDxfCode::HardPointerId:      ourType = DataType::HardPointerId;      break;
        case OdDxfCode::SoftPointerId:      ourType = DataType::SoftPointerId;      break;
        case OdDxfCode::SoftOwnershipId:    ourType = DataType::SoftOwnershipId;    break;
        case OdDxfCode::HardOwnershipId:    ourType = DataType::HardOwnershipId;    break;
        default:                            ourType = DataType::None;
        }
    
#elif DWGTOOLKIT_RealDwg

    AcDb::DwgDataType   acType = acdbGroupCodeToType (T_Super::restype);
    ourType = static_cast <DataType> (acType);

#endif  // DWGTOOLKIT_
    return  ourType;
    }

DwgResBufP  DwgResBuf::Start () { return this; }
DwgResBufP  DwgResBuf::Next () { return static_cast<DwgResBufP>(DWGDB_CALLSDKMETHOD(T_Super::next().get(), T_Super::rbnext)); }
DwgResBufP  DwgResBuf::End () { return nullptr; }

