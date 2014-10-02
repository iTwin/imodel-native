/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/DgnManagedGCS.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <vcclr.h>
#include    <BentleyManagedUtil\StringInterop.h>

#using      <mscorlib.dll>
#using      <Bentley.GeoCoord2.dll>
#using      <Bentley.GeometryNET.Structs.dll>
#using      <Bentley.DgnPlatformNET.dll>

#include    <DgnGeoCoord\DgnGeoCoord.h>

namespace Bentley {

namespace GeoCoordinatesNET {

namespace BI        = Bentley::Interop;
namespace BDPN      = Bentley::DgnPlatformNET;
namespace GEOM      = Bentley::GeometryNET;
namespace SRI       = System::Runtime::InteropServices;
namespace BGC       = Bentley::GeoCoordinates;

// this makes it so we can use just String, rather than System::String all over the place.
using   System::String;
using   System::Object;

/*=================================================================================**//**
*
* This is a managed class that represents a Mstn GeoCoordinate system.
*
+===============+===============+===============+===============+===============+======*/
public ref class DgnGCS : BaseGCS
{
private:
DgnGCSP    m_dgnGCSPeer;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS
(
DgnGCSP     peer
) : BaseGCS ((BGC::BaseGCSP)peer)
    {
    // we don't AddRef because that is taken care of in the base class.
    m_dgnGCSPeer = peer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS
(
System::IntPtr   peer
) : BaseGCS (peer)
    {
    // we don't AddRef because that is taken care of in the base class.
    m_dgnGCSPeer = (DgnGCSP) peer.ToPointer();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS
(
String^         keyString,
DgnModelRefP    modelRef
) : BaseGCS ((BGC::DgnGCS::CreateGCS(BI::ScopedString (keyString).Uni(), modelRef)).get())
    {
    // we don't AddRef because that is taken care of in the base class.
    m_dgnGCSPeer = (DgnGCSP) (__super::Peer.ToPointer());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnGCS^     FromModel
(
DgnModelRefP    modelRef,
bool            primaryCoordSys
)
    {
    DgnGCSP     dgnGCSP;
    if (NULL != (dgnGCSP = BGC::DgnGCS::FromModel (modelRef, primaryCoordSys)))
        return gcnew DgnGCS (dgnGCSP);
    else
        return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnGCS^     FromModel
(
BDPN::DgnModelRef^      modelReference,
bool                    primaryCoordSys
)
    {
    return FromModel (static_cast <DgnModelRefP> (modelReference->GetNative().ToPointer()), primaryCoordSys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               ToModel
(
DgnModelRefP            modelRef,
bool                    primaryCoordSys,
bool                    writeToFile,
bool                    reprojectData,
bool                    showProblems
)
    {
    return m_dgnGCSPeer->ToModel (modelRef, primaryCoordSys, writeToFile, reprojectData, showProblems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               ToModel
(
BDPN::DgnModelRef^      modelReference,
bool                    primaryCoordSys,
bool                    writeToFile,
bool                    reprojectData,
bool                    showProblems
)
    {
    return ToModel (static_cast <DgnModelRefP> (modelReference->GetNative().ToPointer()), primaryCoordSys, writeToFile, reprojectData, showProblems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    CartesianFromUors
(
[SRI::Out] GEOM::DPoint3d%      outCartesian,
GEOM::DPoint3d                  inUors
)
    {
    pin_ptr<GEOM::DPoint3d> pinnedCartesian = &outCartesian;
    DPoint3dP nativeCartesian = reinterpret_cast <DPoint3dP> (pinnedCartesian);

    DPoint3dCP nativeUors = reinterpret_cast <DPoint3dCP> (&inUors);
    return m_dgnGCSPeer->CartesianFromUors (*nativeCartesian, *nativeUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    UorsFromCartesian
(
[SRI::Out] GEOM::DPoint3d%      outUors,
GEOM::DPoint3d                  inCartesian
)
    {
    pin_ptr<GEOM::DPoint3d> pinnedUors = &outUors;
    DPoint3dP nativeUors = reinterpret_cast <DPoint3dP> (pinnedUors);

    DPoint3dCP nativeCartesian = reinterpret_cast <DPoint3dCP> (&inCartesian);
    return m_dgnGCSPeer->UorsFromCartesian (*nativeUors, *nativeCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               UorsFromLatLong
(
[SRI::Out] GEOM::DPoint3d%      outUors,
GEOM::GeoPoint                  inLatLong
)
    {
    pin_ptr<GEOM::DPoint3d> pinnedUors = &outUors;
    DPoint3dP nativeUors = reinterpret_cast <DPoint3dP> (pinnedUors);

    pin_ptr<GEOM::GeoPoint> pinnedLatLong = &inLatLong;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedLatLong);
    return m_dgnGCSPeer->UorsFromLatLong (*nativeUors, *nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               LatLongFromUors
(
[SRI::Out] GEOM::GeoPoint%      outLatLong,
GEOM::DPoint3d                  inUors
)
    {
    pin_ptr<GEOM::GeoPoint> pinnedOutLL = &outLatLong;
    GeoPointP nativeOutLL = reinterpret_cast <GeoPointP> (pinnedOutLL);

    pin_ptr<GEOM::DPoint3d> pinnedUors = &inUors;
    DPoint3dCP nativeUors = reinterpret_cast <DPoint3dCP> (pinnedUors);
    return m_dgnGCSPeer->LatLongFromUors (*nativeOutLL, *nativeUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               ReprojectUors
(
[SRI::Out] array<GEOM::DPoint3d>^%  outUors,
[SRI::Out] array<GEOM::GeoPoint>^%  outLatLongDest,
[SRI::Out] array<GEOM::GeoPoint>^%  outLatLongSrc,
array<GEOM::DPoint3d>^              inUors,
DgnGCS^                             destDgnGCS
)
    {
    outUors = gcnew array<GEOM::DPoint3d> (inUors->Length);
    pin_ptr<GEOM::DPoint3d> pinnedOutUors = &outUors[0];
    DPoint3dP nativeOutUors = reinterpret_cast <DPoint3dP> (pinnedOutUors);

    outLatLongDest = gcnew array<GEOM::GeoPoint> (inUors->Length);
    pin_ptr<GEOM::GeoPoint> pinnedOutLatLongDest = &outLatLongDest[0];
    GeoPointP nativeOutLatLongDest = reinterpret_cast <GeoPointP> (pinnedOutLatLongDest);

    outLatLongSrc = gcnew array<GEOM::GeoPoint> (inUors->Length);
    pin_ptr<GEOM::GeoPoint> pinnedOutLatLongSrc = &outLatLongSrc[0];
    GeoPointP nativeOutLatLongSrc = reinterpret_cast <GeoPointP> (pinnedOutLatLongSrc);

    pin_ptr<GEOM::DPoint3d> pinnedInUors = &inUors[0];
    DPoint3dP nativeInUors = reinterpret_cast <DPoint3dP> (pinnedInUors);

    return m_dgnGCSPeer->ReprojectUors (nativeOutUors, nativeOutLatLongDest, nativeOutLatLongSrc, nativeInUors, inUors->Length, *destDgnGCS->m_dgnGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               ReprojectUors
(
[SRI::Out] array<GEOM::DPoint3d>^%  outUors,
array<GEOM::DPoint3d>^              inUors,
DgnGCS^                             destDgnGCS
)
    {
    outUors = gcnew array<GEOM::DPoint3d> (inUors->Length);
    pin_ptr<GEOM::DPoint3d> pinnedOutUors = &outUors[0];
    DPoint3dP nativeOutUors = reinterpret_cast <DPoint3dP> (pinnedOutUors);

    pin_ptr<GEOM::DPoint3d> pinnedInUors = &inUors[0];
    DPoint3dP nativeInUors = reinterpret_cast <DPoint3dP> (pinnedInUors);

    return m_dgnGCSPeer->ReprojectUors (nativeOutUors, NULL, NULL, nativeInUors, inUors->Length, *destDgnGCS->m_dgnGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               UorsFromLatLong2D
(
[SRI::Out] GEOM::DPoint2d%          outUors,
GEOM::GeoPoint2d                    inLatLong
)
    {
    pin_ptr<GEOM::DPoint2d> pinnedUors = &outUors;
    DPoint2dP nativeUors = reinterpret_cast <DPoint2dP> (pinnedUors);

    pin_ptr<GEOM::GeoPoint2d> pinnedLatLong = &inLatLong;
    GeoPoint2dCP nativeInLL = reinterpret_cast <GeoPoint2dCP> (pinnedLatLong);
    return m_dgnGCSPeer->UorsFromLatLong2D (*nativeUors, *nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               LatLongFromUors2D
(
[SRI::Out] GeoPoint2d%              outLatLong,
GEOM::DPoint2d                      inUors
)
    {
    pin_ptr<GeoPoint2d> pinnedOutLL = &outLatLong;
    GeoPoint2dP nativeOutLL = reinterpret_cast <GeoPoint2dP> (pinnedOutLL);

    pin_ptr<GEOM::DPoint2d> pinnedUors = &inUors;
    DPoint2dCP nativeUors = reinterpret_cast <DPoint2dCP> (pinnedUors);
    return m_dgnGCSPeer->LatLongFromUors2D (*nativeOutLL, *nativeUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               ReprojectUors2D
(
[SRI::Out] array<GEOM::DPoint2d>^%      outUors,
[SRI::Out] array<GEOM::GeoPoint2d>^%    outLatLongDest,
[SRI::Out] array<GEOM::GeoPoint2d>^%    outLatLongSrc,
array<GEOM::DPoint2d>^                  inUors,
DgnGCS^                                 destDgnGCS
)
    {
    outUors = gcnew array<GEOM::DPoint2d> (inUors->Length);
    pin_ptr<GEOM::DPoint2d> pinnedOutUors = &outUors[0];
    DPoint2dP nativeOutUors = reinterpret_cast <DPoint2dP> (pinnedOutUors);

    outLatLongDest = gcnew array<GEOM::GeoPoint2d> (inUors->Length);
    pin_ptr<GEOM::GeoPoint2d> pinnedOutLatLongDest = &outLatLongDest[0];
    GeoPoint2dP nativeOutLatLongDest = reinterpret_cast <GeoPoint2dP> (pinnedOutLatLongDest);

    outLatLongSrc = gcnew array<GEOM::GeoPoint2d> (inUors->Length);
    pin_ptr<GEOM::GeoPoint2d> pinnedOutLatLongSrc = &outLatLongSrc[0];
    GeoPoint2dP nativeOutLatLongSrc = reinterpret_cast <GeoPoint2dP> (pinnedOutLatLongSrc);

    pin_ptr<GEOM::DPoint2d> pinnedInUors = &inUors[0];
    DPoint2dP nativeInUors = reinterpret_cast <DPoint2dP> (pinnedInUors);

    return m_dgnGCSPeer->ReprojectUors2D (nativeOutUors, nativeOutLatLongDest, nativeOutLatLongSrc, nativeInUors, inUors->Length, *destDgnGCS->m_dgnGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               ReprojectUors2D
(
[SRI::Out] array<GEOM::DPoint2d>^%  outUors,
array<GEOM::DPoint2d>^              inUors,
DgnGCS^                             destDgnGCS
)
    {
    outUors = gcnew array<GEOM::DPoint2d> (inUors->Length);
    pin_ptr<GEOM::DPoint2d> pinnedOutUors = &outUors[0];
    DPoint2dP nativeOutUors = reinterpret_cast <DPoint2dP> (pinnedOutUors);

    pin_ptr<GEOM::DPoint2d> pinnedInUors = &inUors[0];
    DPoint2dP nativeInUors = reinterpret_cast <DPoint2dP> (pinnedInUors);

    return m_dgnGCSPeer->ReprojectUors2D (nativeOutUors, NULL, NULL, nativeInUors, inUors->Length, *destDgnGCS->m_dgnGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               GetLocalTransform
(
[SRI::Out] GEOM::DTransform3d%  outTransform,
GEOM::DPoint3d                  elementOrigin,
bool                            doRotate,
bool                            doScale,
DgnGCS^                         destDgnGCS
)
    {
    pin_ptr<GEOM::DTransform3d> pinnedTransform = &outTransform;
    TransformP nativeOutTransform = reinterpret_cast <TransformP> (pinnedTransform);

    DPoint3dCP nativeElementOrigin = reinterpret_cast <DPoint3dCP> (&elementOrigin);

    return m_dgnGCSPeer->GetLocalTransform (nativeOutTransform, *nativeElementOrigin, NULL, doRotate, doScale, *destDgnGCS->m_dgnGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
property System::String^ ProjectionName
    {
    System::String^ get() 
        {
        WString displayName;
        m_dgnGCSPeer->GetProjectionName (displayName);
        return gcnew System::String (displayName.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
property System::String^ DisplayName
    {
    System::String^ get() 
        {
        WString displayName;
        m_dgnGCSPeer->GetDisplayName (displayName);
        return gcnew System::String (displayName.c_str());
        }
    }

property double PaperScale
    {
    double get ()
        {
        return m_dgnGCSPeer->GetPaperScale ();
        }
    }

StatusInt               SetPaperScale (double value, BDPN::DgnModelRef^ modelReference)
    {
    return m_dgnGCSPeer->SetPaperScale (value, (DgnModelRefP) modelReference->GetNative().ToPointer());
    }



};  // End of DgnGCS class

}   // End of GeoCoordinatesNET Namespace

}   // End of Bentley Namespace








