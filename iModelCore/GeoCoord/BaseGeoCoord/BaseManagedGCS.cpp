/*----------------------------------------------------------------------+
|
|   $Source: BaseGeoCoord/BaseManagedGCS.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma  warning(disable:4189) // local variable is initialized but not referenced
#pragma  warning(disable:4456) // hides previous local declaration

#include    <windows.h>
#include    <commctrl.h>
#include    <vcclr.h>
#include    <BentleyManagedUtil\StringInterop.h>
#include    <string.h>
#include    <assert.h>

#pragma unmanaged
#include    <GeoCoord\BaseGeoCoord.h>
#include    <GeoCoord\gcslibrary.h>
#include    <CSMap\cs_map.h>
extern "C" struct   cs_Unittab_     cs_Unittab[];
#pragma managed

#using      <mscorlib.dll>
#using      <Bentley.ECObjects3.dll>
#using      <System.dll>
#using      <System.Drawing.dll>
#using      <System.Windows.Forms.dll>
#using      <system.Xml.dll>
#using      <GeoCoordExceptions.netmodule>        // C# netmodule
#using      <Bentley.Platform.dll>
#using      <Bentley.GeometryNET.Structs.dll>
#using      <Bentley.UI.dll>

namespace Bentley {

namespace GeoCoordinatesNET {

namespace SRSC  = System::Resources;
namespace SC    = System::Collections;
namespace SCG   = System::Collections::Generic;

namespace SRI   = System::Runtime::InteropServices;
namespace SWF   = System::Windows::Forms;
namespace SD    = System::Drawing;
namespace SIO   = System::IO;
namespace SRSC  = System::Resources;
namespace SREF  = System::Reflection;
namespace SRS   = System::Runtime::Serialization;
namespace SSP   = System::Security::Permissions;
namespace SCM   = System::ComponentModel;
namespace SG    = System::Globalization;

namespace EC    = Bentley::ECObjects;
namespace BI    = Bentley::Interop;
namespace ECS   = Bentley::ECObjects::Schema;
namespace ECI   = Bentley::ECObjects::Instance;
namespace ECL   = Bentley::ECObjects::Lightweight;
namespace ECUI  = Bentley::ECObjects::UI;
namespace ECXML = Bentley::ECObjects::XML;
namespace ECE   = Bentley::ECObjects::Expressions;
namespace BUCWG = Bentley::UI::Controls::WinForms::GroupPanel;
namespace GEOM  = Bentley::GeometryNET;
namespace BGC   = Bentley::GeoCoordinates;


// this makes it so we can use just String, rather than System::String all over the place.
using   System::String;
using   System::Object;

public enum class  GeoDatumToWGS84Method
    {
    ConvertType_NONE      =   0,
    ConvertType_MOLO      =   1,
    ConvertType_MREG      =   2,
    ConvertType_BURS      =   3,
    ConvertType_NAD27     =   4,
    ConvertType_NAD83     =   5,
    ConvertType_WGS84     =   6,
    ConvertType_WGS72     =   7,
    ConvertType_HPGN      =   8,
    ConvertType_7PARM     =   9,
    ConvertType_AGD66     =   10,
    ConvertType_3PARM     =   11,
    ConvertType_6PARM     =   12,
    ConvertType_4PARM     =   13,
    ConvertType_AGD84     =   14,
    ConvertType_NZGD4     =   15,
    ConvertType_ATS77     =   16,
    ConvertType_GDA94     =   17,
    ConvertType_NZGD2K    =   18,
    ConvertType_CSRS      =   19,
    ConvertType_TOKYO     =   20,
    ConvertType_RGF93     =   21,
    ConvertType_ED50      =   22,
    ConvertType_DHDN      =   23,
    ConvertType_ETRF89    =   24,
    ConvertType_GEOCTR    =   25,
    ConvertType_CHENYX    =   26,
#ifdef GEOCOORD_ENHANCEMENT
    ConvertType_GENGRID   =   27,
#endif
    };


/*=================================================================================**//**
* This managed class manages the GeoCoordinates in the system.
+===============+===============+===============+===============+===============+======*/
public ref class GeoCoordinateManager
{
private:
static System::Resources::ResourceManager^   s_stringResourceManager;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
static void                     Initialize
(
String^     dataDirectory
)
    {
    // this technique of getting an ansi string from a managed string requires a temporarily allocated string on the unmanaged heap.
    BI::ScopedString dataDir(dataDirectory);
    BGC::BaseGCS::Initialize (dataDir.Uni());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/11
+---------------+---------------+---------------+---------------+---------------+------*/
static void                     AddUserLibrary (String^ libPath, String^ guiName)
    {
    // this technique of getting an ansi string from a managed string requires a temporarily allocated string on the unmanaged heap.
    BI::ScopedString pathString (libPath);
    BI::ScopedString guiString (guiName);
    Bentley::GeoCoordinates::LibraryManager::Instance()->AddUserLibrary (pathString.Uni(), guiString.Uni());
    }

// these overloads are used from GeoCoordinateInterop in mstngeocoord.cpp
/*------------------------------------------------------------------------------------**/
/// <summary>Get Localized String</summary>
/// <author>Barry.Bentley</author>                              <date>11/2006/</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
static WCharCP    GetLocalizedString
(
WCharP          outString,
WCharCP         inKey,
int             outSize
)
    {
    String^     key = gcnew String (inKey);
    String^     localized = GeoCoordinateLocalization::GetLocalizedString (key);
    BI::ScopedString    localizedString (localized);
    wcsncpy (outString, localizedString.Uni(), outSize);
    return outString;
    }

};

ref class GCSGroup;
ref class BaseGCS;
/*=================================================================================**//**
*
* Member enumerator
*
+===============+===============+===============+===============+===============+======*/
public ref class GCSMemberEnumerator : SCG::IEnumerator<BaseGCS^>
{
private:
int             m_currentIndex;
BaseGCS^        m_current;
GCSGroup^       m_group;
BGC::LibraryP   m_library;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
GCSMemberEnumerator
(
GCSGroup^ group
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    m_group         = group;
    m_library       = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
GCSMemberEnumerator
(
BGC::LibraryP    library
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    m_group         = nullptr;
    m_library       = library;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property BaseGCS^ Current
    {
    BaseGCS^ get()
        {
        return m_current;
        }
    }

// Here is an example of explicit override of an interface property.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property System::Object^ NonGenericCurrent
    {
    System::Object^ get() = SC::IEnumerator::Current::get
        {
        return Current;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool MoveNext
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void Reset
(
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
~GCSMemberEnumerator
(
)
    {
    // clean up code to release managed resource
    // (none)
    // to avoid code duplication
    // call finalizer to release unmanaged resources
    this->!GCSMemberEnumerator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod (Dispose method)                                   Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
!GCSMemberEnumerator
(
)
    {
    // finalizer cleans up unmanaged resources
    // destructor or garbage collector will
    // clean up managed resources
    // clean up code to release unmanaged resource
    // (we don't really have any in this class, but need to implement Dispose to satisfy interface.
    }
}; // GCSMemberEnumerator

/*=================================================================================**//**
*
* Member Name enumerator
*
+===============+===============+===============+===============+===============+======*/
public ref class GCSNameEnumerator : SCG::IEnumerator<String^>
{
private:
int             m_currentIndex;
String^         m_current;
BGC::LibraryP   m_library;
GCSGroup^       m_group;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
GCSNameEnumerator
(
GCSGroup^ group
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    m_library       = NULL;
    m_group         = group;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
GCSNameEnumerator
(
BGC::LibraryP   library
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    m_library       = library;
    m_group         = nullptr;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property String^ Current
    {
    String^ get()
        {
        return m_current;
        }
    }

// Here is an example of explicit override of an interface property.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property System::Object^ NonGenericCurrent
    {
    System::Object^ get() = SC::IEnumerator::Current::get
        {
        return Current;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool MoveNext
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void Reset
(
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
~GCSNameEnumerator
(
)
    {
    // clean up code to release managed resource
    // (none)
    // to avoid code duplication
    // call finalizer to release unmanaged resources
    this->!GCSNameEnumerator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod (Dispose method)                                   Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
!GCSNameEnumerator
(
)
    {
    // finalizer cleans up unmanaged resources
    // destructor or garbage collector will
    // clean up managed resources
    // clean up code to release unmanaged resource
    // (we don't really have any in this class, but need to implement Dispose to satisfy interface.
    }
}; // GCSMemberEnumerator


/*=================================================================================**//**
*
* This is a managed class that represents a coordinate system.
*
+===============+===============+===============+===============+===============+======*/
public ref class BaseGCS
{
private:

BGC::BaseGCSP    m_baseGCSPeer;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
BaseGCS
(
BGC::BaseGCSP    peer
)
    {
    m_baseGCSPeer = peer;
    m_baseGCSPeer->AddRef();
    if (!m_baseGCSPeer->IsValid())
        ThrowConstructorException (nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS
(
System::IntPtr   peer
)
    {
    m_baseGCSPeer = (BGC::BaseGCSP) peer.ToPointer();
    m_baseGCSPeer->AddRef();
    if (!m_baseGCSPeer->IsValid())
        ThrowConstructorException (nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS
(
String^ keyString
)
    {
    BI::ScopedString key (keyString);
    BGC::BaseGCSPtr  gcs = BGC::BaseGCS::CreateGCS(key.Uni());
    m_baseGCSPeer = gcs.get();
    m_baseGCSPeer->AddRef();
    if (!m_baseGCSPeer->IsValid())
        ThrowConstructorException (keyString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS
(
WCharCP key
)
    {
    BGC::BaseGCSPtr  gcs = BGC::BaseGCS::CreateGCS(key);
    m_baseGCSPeer = gcs.get();
    m_baseGCSPeer->AddRef();
    if (!m_baseGCSPeer->IsValid())
        ThrowConstructorException (gcnew String (key));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            ThrowConstructorException
(
String^ nameString
)
    {
    StatusInt   errorNum    = m_baseGCSPeer->GetError();
    WString     errorMsg;
    String^     errorString = gcnew String(m_baseGCSPeer->GetErrorMessage(errorMsg));
    if (nullptr == nameString)
        {
        WCharCP    name     = m_baseGCSPeer->GetName();
        nameString          = gcnew String ((NULL ==  name) ? String::Empty : gcnew String (name));
        }
    m_baseGCSPeer->Release();
    m_baseGCSPeer           = nullptr;
    throw gcnew GeoCoordinateException::ConstructorFailure (errorNum, nameString, errorString);
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
~BaseGCS ()
    {
    // Note, this method is internally renamed to Dispose, and does the "SuppressFinalization" for us.
    if (NULL == m_baseGCSPeer)
        return;

    m_baseGCSPeer->Release();

    m_baseGCSPeer = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
!BaseGCS ()
    {
    // This is the finalize method. It is called in the finalizer thread.
    // But since all we are doing is freeing memory, it is Ok to release directly.
    if (NULL == m_baseGCSPeer)
        return;

    m_baseGCSPeer->Release();

    m_baseGCSPeer = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
property System::IntPtr Peer
    {
    System::IntPtr get() {return System::IntPtr ((void *)m_baseGCSPeer);}
    void set(System::IntPtr newGCS)
        {
        if (NULL != m_baseGCSPeer)
            m_baseGCSPeer->Release();

        m_baseGCSPeer = (BGC::BaseGCSP) newGCS.ToPointer();
        m_baseGCSPeer->AddRef();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             Matches
(
array<String^>^     mixedCaseMatchStrings,
array<String^>^     upperCaseMatchStrings,
bool                anyWord
)
    {
    int numMixedCaseStrings = mixedCaseMatchStrings->Length;
    int numUpperCaseStrings = upperCaseMatchStrings->Length;
    char ** nativeStrings = (char**)_alloca ((numMixedCaseStrings + numUpperCaseStrings) * sizeof (char *));
    for (int iString=0; iString < numMixedCaseStrings; iString++)
        {
        BI::ScopedString thisString (mixedCaseMatchStrings[iString]);
        char *scopedString = thisString.Ansi();
        char *stackString = (char *)_alloca (strlen(scopedString)+1);
        strcpy (stackString, scopedString);
        nativeStrings[iString] = stackString;
        }
    for (int iString=0; iString < numUpperCaseStrings; iString++)
        {
        BI::ScopedString thisString (upperCaseMatchStrings[iString]);
        char *scopedString = thisString.Ansi();
        char *stackString = (char *)_alloca (strlen(scopedString)+1);
        strcpy (stackString, scopedString);
        nativeStrings[numMixedCaseStrings + iString] = stackString;
        }
    return m_baseGCSPeer->Matches (nativeStrings, numMixedCaseStrings, numUpperCaseStrings, anyWord);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             Matches
(
SCG::List<String^>^     mixedCaseMatchStrings,
SCG::List<String^>^     upperCaseMatchStrings,
bool                    anyWord
)
    {
    int numMixedCaseStrings = mixedCaseMatchStrings->Count;
    int numUpperCaseStrings = upperCaseMatchStrings->Count;
    char ** nativeStrings = (char**)_alloca ((numMixedCaseStrings + numUpperCaseStrings) * sizeof (char *));
    for (int iString=0; iString < numMixedCaseStrings; iString++)
        {
        BI::ScopedString thisString (mixedCaseMatchStrings[iString]);
        char *scopedString = thisString.Ansi();
        char *stackString = (char *)_alloca (strlen(scopedString)+1);
        strcpy (stackString, scopedString);
        nativeStrings[iString] = stackString;
        }
    for (int iString=0; iString < numUpperCaseStrings; iString++)
        {
        BI::ScopedString thisString (upperCaseMatchStrings[iString]);
        char *scopedString = thisString.Ansi();
        char *stackString = (char *)_alloca (strlen(scopedString)+1);
        strcpy (stackString, scopedString);
        nativeStrings[numMixedCaseStrings + iString] = stackString;
        }
    return m_baseGCSPeer->Matches (nativeStrings, numMixedCaseStrings, numUpperCaseStrings, anyWord);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Validate
(
[SRI::Out] array<String^>^%    errors
)
    {
    T_WStringVector     csMapErrorStrings;
    bool result = m_baseGCSPeer->Validate (csMapErrorStrings);

    errors      = gcnew array<String^>(static_cast<int>(csMapErrorStrings.size()));

    if (!result && !csMapErrorStrings.empty())
        {
        T_WStringVector::iterator  errorIterator;
        int                     iError;
        for (iError=0, errorIterator = csMapErrorStrings.begin(); errorIterator != csMapErrorStrings.end(); errorIterator++, iError++)
            {
            errors[iError] = gcnew String ((*errorIterator).data());
            }
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             DefinitionComplete ()
    {
    return m_baseGCSPeer->DefinitionComplete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          GetScaleAlongMeridian
(
GEOM::GeoPoint        point
)
    {
    pin_ptr<const GEOM::GeoPoint> pinnedInLL = &point;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedInLL);
    return m_baseGCSPeer->GetScaleAlongMeridian (*nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          GetScaleAlongParallel
(
GEOM::GeoPoint        point
)
    {
    pin_ptr<const GEOM::GeoPoint> pinnedInLL = &point;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedInLL);
    return m_baseGCSPeer->GetScaleAlongParallel (*nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          GetGridScale
(
GEOM::GeoPoint        point
)
    {
    pin_ptr<const GEOM::GeoPoint> pinnedInLL = &point;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedInLL);
    return m_baseGCSPeer->GetGridScale (*nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          GetConvergenceAngle
(
GEOM::GeoPoint        point
)
    {
    pin_ptr<const GEOM::GeoPoint> pinnedInLL = &point;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedInLL);
    return m_baseGCSPeer->GetConvergenceAngle (*nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GetDistance
(
[SRI::Out] double%  distance,
[SRI::Out] double%  azimuth,
GEOM::GeoPoint      startPoint,
GEOM::GeoPoint      endPoint
)
    {
    pin_ptr<const GEOM::GeoPoint> pinnedStartPoint = &startPoint;
    pin_ptr<const GEOM::GeoPoint> pinnedEndPoint   = &endPoint;
    GeoPointCP nativeStartPoint = reinterpret_cast <GeoPointCP> (pinnedStartPoint);
    GeoPointCP nativeEndPoint   = reinterpret_cast <GeoPointCP> (pinnedEndPoint);
    pin_ptr<double> pinnedDistance = &distance;
    pin_ptr<double> pinnedAzimuth  = &azimuth;
    double* nativeDistance = reinterpret_cast <double *> (pinnedDistance);
    double* nativeAzimuth  = reinterpret_cast <double *> (pinnedAzimuth);
    return m_baseGCSPeer->GetDistance (nativeDistance, nativeAzimuth, *nativeStartPoint, *nativeEndPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GetCenterPoint
(
[SRI::Out] GEOM::GeoPoint%    point
)
    {
    pin_ptr<GEOM::GeoPoint> pinnedInLL = &point;
    ::GeoPoint* nativeInLL = reinterpret_cast <::GeoPoint*> (pinnedInLL);
    return m_baseGCSPeer->GetCenterPoint (*nativeInLL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsEquivalent
(
BaseGCS^    compareTo
)
    {
    return m_baseGCSPeer->IsEquivalent (*compareTo->m_baseGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            HasEquivalentDatum
(
BaseGCS^    compareTo
)
    {
    return m_baseGCSPeer->HasEquivalentDatum (*compareTo->m_baseGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             GetEPSGCode
(
bool    dontSearch
)
    {
    return m_baseGCSPeer->GetEPSGCode (dontSearch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             GetEPSGDatumCode
(
bool    dontSearch
)
    {
    return m_baseGCSPeer->GetEPSGDatumCode (dontSearch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CanSaveToGeoTiffKeys
(
)
    {
    return m_baseGCSPeer->CanSaveToGeoTiffKeys();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CartesianFromLatLong
(
[SRI::Out] GEOM::DPoint3d%  outCartesian,
GEOM::GeoPoint              inLatLong
)
    {
    pin_ptr<GEOM::DPoint3d> pinnedCartesian = &outCartesian;
    ::DPoint3d* nativeCartesian = reinterpret_cast <::DPoint3d*> (pinnedCartesian);
    pin_ptr<const GEOM::GeoPoint> pinnedInLL = &inLatLong;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedInLL);
    return m_baseGCSPeer->CartesianFromLatLong (*nativeCartesian, *nativeInLL);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LatLongFromCartesian
(
[SRI::Out] GEOM::GeoPoint%  outLatLong,
GEOM::DPoint3d              inCartesian         // => cartesian coordinates in this GCS
)
    {
    pin_ptr<GEOM::GeoPoint> pinnedLatLong = &outLatLong;
    GeoPointP nativeLatLong = reinterpret_cast <GeoPointP> (pinnedLatLong);
    pin_ptr<const GEOM::DPoint3d> pinnedCartesian = &inCartesian;
    ::DPoint3d const* nativeCartesian = reinterpret_cast <::DPoint3d const*> (pinnedCartesian);
    return m_baseGCSPeer->LatLongFromCartesian (*nativeLatLong, *nativeCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LatLongFromLatLong
(
[SRI::Out] GEOM::GeoPoint%  outLatLong,
GEOM::GeoPoint              inLatLong,
BaseGCS^                    destGCS
)
    {
    pin_ptr<GEOM::GeoPoint> pinnedOutLL = &outLatLong;
    GeoPointP nativeOutLL = reinterpret_cast <GeoPointP> (pinnedOutLL);
    pin_ptr<const GEOM::GeoPoint> pinnedInLL = &inLatLong;
    GeoPointCP nativeInLL = reinterpret_cast <GeoPointCP> (pinnedInLL);
    return m_baseGCSPeer->LatLongFromLatLong (*nativeOutLL, *nativeInLL, *destGCS->m_baseGCSPeer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LatLongFromLatLong2D
(
[SRI::Out] GEOM::GeoPoint2d%    outLatLong,
GEOM::GeoPoint2d                inLatLong,
BaseGCS^                        destGCS
)
    {
    pin_ptr<GEOM::GeoPoint2d> pinnedOutLL = &outLatLong;
    GeoPoint2dP nativeOutLL = reinterpret_cast <GeoPoint2dP> (pinnedOutLL);
    pin_ptr<const GEOM::GeoPoint2d> pinnedInLL = &inLatLong;
    GeoPoint2dCP nativeInLL = reinterpret_cast <GeoPoint2dCP> (pinnedInLL);
    return m_baseGCSPeer->LatLongFromLatLong2D (*nativeOutLL, *nativeInLL, *destGCS->m_baseGCSPeer);
    }

enum class WellKnownTextFlavor
    {
    wktFlavorOGC        = 1,    // Open Geospatial Consortium flavor
    wktFlavorGeoTiff    = 2,    // GeoTiff flavor.
    wktFlavorESRI       = 3,    // ESRI flavor.
    wktFlavorOracle     = 4,    // Oracle flavor.
    wktFlavorGeoTools   = 5,    // GeoTools flavor
    wktFlavorEPSG       = 6,    // EPSG flavor
    wktFlavorUnknown    = 7,    // used if the flavor is unknown. InitFromWellKnownText will do its best to figure it out.
    wktFlavorAppAlt     = 8,    // Not yet supported
    wktFlavorLclAlt     = 9,    // Not yet supported
    };

enum class RangeTestResult
    {
    RangeTestOk                 = 0,   // The points are within the useful range
    RangeTestOutsideRange       = 1,   // one of more points outside the useful range of the coordinate system.
    RangeTestOutsideMathDomain  = 2,   // one or more points outside the mathematical domain of the coordinate system.
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GetWellKnownText
(
[SRI::Out] String^%         wellKnownText,
WellKnownTextFlavor         flavor
)
    {
    WString wktString;
    StatusInt status;
    if (SUCCESS == (status = m_baseGCSPeer->GetWellKnownText (wktString, (BGC::BaseGCS::WktFlavor) flavor)))
        wellKnownText = gcnew String (wktString.c_str());
    else
        wellKnownText = String::Empty;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void       GetAffineParameters
(
[SRI::Out] double%  A0,
[SRI::Out] double%  A1,
[SRI::Out] double%  A2,
[SRI::Out] double%  B0,
[SRI::Out] double%  B1,
[SRI::Out] double%  B2
)
    {
    pin_ptr<double> pinnedA0 = &A0;
    pin_ptr<double> pinnedA1 = &A1;
    pin_ptr<double> pinnedA2 = &A2;
    pin_ptr<double> pinnedB0 = &B0;
    pin_ptr<double> pinnedB1 = &B1;
    pin_ptr<double> pinnedB2 = &B2;
    double* nativeA0 = reinterpret_cast <double *> (pinnedA0);
    double* nativeA1 = reinterpret_cast <double *> (pinnedA1);
    double* nativeA2 = reinterpret_cast <double *> (pinnedA2);
    double* nativeB0 = reinterpret_cast <double *> (pinnedB0);
    double* nativeB1 = reinterpret_cast <double *> (pinnedB1);
    double* nativeB2 = reinterpret_cast <double *> (pinnedB2);

    m_baseGCSPeer->GetAffineParameters (nativeA0, nativeA1, nativeA2, nativeB0, nativeB1, nativeB2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SetAffineParameters
(
double  A0,
double  A1,
double  A2,
double  B0,
double  B1,
double  B2
)
    {
    return m_baseGCSPeer->SetAffineParameters (A0, A1, A2, B0, B1, B2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Name
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetName());}
    void set (String^ value)
        {
        BI::ScopedString name (value);
        m_baseGCSPeer->SetName (name.Uni());
        }
    }
property String^ Description
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetDescription());}
    void set (String^ value)
        {
        BI::ScopedString name (value);
        m_baseGCSPeer->SetDescription (name.Uni());
        }
    }
property String^ Projection
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetProjection());}
    }

enum class ProjectionCodeValue
    {
    pcvInvalid                                      = BGC::BaseGCS::pcvInvalid,
    pcvUnity                                        = BGC::BaseGCS::pcvUnity,
    pcvTransverseMercator                           = BGC::BaseGCS::pcvTransverseMercator,
    pcvAlbersEqualArea                              = BGC::BaseGCS::pcvAlbersEqualArea,
    pcvHotineObliqueMercator                        = BGC::BaseGCS::pcvHotineObliqueMercator,
    pcvMercator                                     = BGC::BaseGCS::pcvMercator,
    pcvLambertEquidistantAzimuthal                  = BGC::BaseGCS::pcvLambertEquidistantAzimuthal,
    pcvLambertTangential                            = BGC::BaseGCS::pcvLambertTangential,
    pcvAmericanPolyconic                            = BGC::BaseGCS::pcvAmericanPolyconic,
    pcvModifiedPolyconic                            = BGC::BaseGCS::pcvModifiedPolyconic,
    pcvLambertEqualAreaAzimuthal                    = BGC::BaseGCS::pcvLambertEqualAreaAzimuthal,
    pcvEquidistantConic                             = BGC::BaseGCS::pcvEquidistantConic,
    pcvMillerCylindrical                            = BGC::BaseGCS::pcvMillerCylindrical,
    pcvModifiedStereographic                        = BGC::BaseGCS::pcvModifiedStereographic,
    pcvNewZealandNationalGrid                       = BGC::BaseGCS::pcvNewZealandNationalGrid,
    pcvSinusoidal                                   = BGC::BaseGCS::pcvSinusoidal,
    pcvOrthographic                                 = BGC::BaseGCS::pcvOrthographic,
    pcvGnomonic                                     = BGC::BaseGCS::pcvGnomonic,
    pcvEquidistantCylindrical                       = BGC::BaseGCS::pcvEquidistantCylindrical,
    pcvVanderGrinten                                = BGC::BaseGCS::pcvVanderGrinten,
    pcvCassini                                      = BGC::BaseGCS::pcvCassini,
    pcvRobinsonCylindrical                          = BGC::BaseGCS::pcvRobinsonCylindrical,
    pcvBonne                                        = BGC::BaseGCS::pcvBonne,
    pcvEckertIV                                     = BGC::BaseGCS::pcvEckertIV,
    pcvEckertVI                                     = BGC::BaseGCS::pcvEckertVI,
    pcvMollweide                                    = BGC::BaseGCS::pcvMollweide,
    pcvGoodeHomolosine                              = BGC::BaseGCS::pcvGoodeHomolosine,
    pcvEqualAreaAuthalicNormal                      = BGC::BaseGCS::pcvEqualAreaAuthalicNormal,
    pcvEqualAreaAuthalicTransverse                  = BGC::BaseGCS::pcvEqualAreaAuthalicTransverse,
    pcvBipolarObliqueConformalConic                 = BGC::BaseGCS::pcvBipolarObliqueConformalConic,
    pcvObliqueCylindricalSwiss                      = BGC::BaseGCS::pcvObliqueCylindricalSwiss,
    pcvPolarStereographic                           = BGC::BaseGCS::pcvPolarStereographic,
    pcvObliqueStereographic                         = BGC::BaseGCS::pcvObliqueStereographic,
    pcvSnyderObliqueStereographic                   = BGC::BaseGCS::pcvSnyderObliqueStereographic,
    pcvLambertConformalConicOneParallel             = BGC::BaseGCS::pcvLambertConformalConicOneParallel,
    pcvLambertConformalConicTwoParallel             = BGC::BaseGCS::pcvLambertConformalConicTwoParallel,
    pcvLambertConformalConicBelgian                 = BGC::BaseGCS::pcvLambertConformalConicBelgian,
    pcvLambertConformalConicWisconsin               = BGC::BaseGCS::pcvLambertConformalConicWisconsin,
    pcvTransverseMercatorWisconsin                  = BGC::BaseGCS::pcvTransverseMercatorWisconsin,
    pcvLambertConformalConicMinnesota               = BGC::BaseGCS::pcvLambertConformalConicMinnesota,
    pcvTransverseMercatorMinnesota                  = BGC::BaseGCS::pcvTransverseMercatorMinnesota,
    pcvSouthOrientedTransverseMercator              = BGC::BaseGCS::pcvSouthOrientedTransverseMercator,
    pcvUniversalTransverseMercator                  = BGC::BaseGCS::pcvUniversalTransverseMercator,
    pcvSnyderTransverseMercator                     = BGC::BaseGCS::pcvSnyderTransverseMercator,
    pcvGaussKrugerTranverseMercator                 = BGC::BaseGCS::pcvGaussKrugerTranverseMercator,
    pcvCzechKrovak                                  = BGC::BaseGCS::pcvCzechKrovak,
    pcvCzechKrovakObsolete                          = BGC::BaseGCS::pcvCzechKrovakObsolete,
    pcvMercatorScaleReduction                       = BGC::BaseGCS::pcvMercatorScaleReduction,
    pcvObliqueConformalConic                        = BGC::BaseGCS::pcvObliqueConformalConic,
    pcvCzechKrovak95                                = BGC::BaseGCS::pcvCzechKrovak95,
    pcvCzechKrovak95Obsolete                        = BGC::BaseGCS::pcvCzechKrovak95Obsolete,
    pcvPolarStereographicStandardLatitude           = BGC::BaseGCS::pcvPolarStereographicStandardLatitude,
    pcvTransverseMercatorAffinePostProcess          = BGC::BaseGCS::pcvTransverseMercatorAffinePostProcess,
    pcvNonEarth                                     = BGC::BaseGCS::pcvNonEarth,
    pcvObliqueCylindricalHungary                    = BGC::BaseGCS::pcvObliqueCylindricalHungary,
    pcvTransverseMercatorDenmarkSys34               = BGC::BaseGCS::pcvTransverseMercatorDenmarkSys34,
    pcvTransverseMercatorOstn97                     = BGC::BaseGCS::pcvTransverseMercatorOstn97,
    pcvAzimuthalEquidistantElevatedEllipsoid        = BGC::BaseGCS::pcvAzimuthalEquidistantElevatedEllipsoid,
    pcvTransverseMercatorOstn02                     = BGC::BaseGCS::pcvTransverseMercatorOstn02,
    pcvTransverseMercatorDenmarkSys3499             = BGC::BaseGCS::pcvTransverseMercatorDenmarkSys3499,
    pcvTransverseMercatorKruger                     = BGC::BaseGCS::pcvTransverseMercatorKruger,
    pcvWinkelTripel                                 = BGC::BaseGCS::pcvWinkelTripel,
    pcvNonEarthScaleRotation                        = BGC::BaseGCS::pcvNonEarthScaleRotation,
    pcvLambertConformalConicAffinePostProcess       = BGC::BaseGCS::pcvLambertConformalConicAffinePostProcess,
    pcvHotineObliqueMercator1UV                     = BGC::BaseGCS::pcvHotineObliqueMercator1UV,
    pcvHotineObliqueMercator1XY                     = BGC::BaseGCS::pcvHotineObliqueMercator1XY,
    pcvHotineObliqueMercator2UV                     = BGC::BaseGCS::pcvHotineObliqueMercator2UV,
    pcvHotineObliqueMercator2XY                     = BGC::BaseGCS::pcvHotineObliqueMercator2XY,
    pcvRectifiedSkewOrthomorphic                    = BGC::BaseGCS::pcvRectifiedSkewOrthomorphic,
    pcvRectifiedSkewOrthomorphicCentered            = BGC::BaseGCS::pcvRectifiedSkewOrthomorphicCentered,
    pcvRectifiedSkewOrthomorphicOrigin              = BGC::BaseGCS::pcvRectifiedSkewOrthomorphicOrigin,
    pcvTotalUniversalTransverseMercator             = BGC::BaseGCS::pcvTotalUniversalTransverseMercator,
    pcvTotalTransverseMercatorBF                    = BGC::BaseGCS::pcvTotalTransverseMercatorBF,
    pcvTransverseMercatorDenmarkSys3401             = BGC::BaseGCS::pcvTransverseMercatorDenmarkSys3401,
    pcvEquidistantCylindricalEllipsoid              = BGC::BaseGCS::pcvEquidistantCylindricalEllipsoid,
    pcvPlateCarree                                  = BGC::BaseGCS::pcvPlateCarree,
    pcvPopularVisualizationPseudoMercator           = BGC::BaseGCS::pcvPopularVisualizationPseudoMercator,
    pcvObliqueMercatorMinnesota                     = BGC::BaseGCS::pcvObliqueMercatorMinnesota,
    };

property ProjectionCodeValue ProjectionCode
    {
    ProjectionCodeValue get() {return (ProjectionCodeValue) m_baseGCSPeer->GetProjectionCode();}
    void set (ProjectionCodeValue value) { m_baseGCSPeer->SetProjectionCode ((BGC::BaseGCS::ProjectionCodeValue)value);}
    }
property int     UnitCode
    {
    int get() {return m_baseGCSPeer->GetUnitCode();}
    void set (int value) { m_baseGCSPeer->SetUnitCode (value); }
    }
property String^ Group
    {
    String^ get() {WString buffer; return gcnew String (m_baseGCSPeer->GetGroup (buffer)); }
    void set (String^ value)
        {
        BI::ScopedString name (value);
        m_baseGCSPeer->SetGroup (name.Uni());
        }
    }
property String^ Location
    {
    String^ get() { WString buffer; return gcnew String (m_baseGCSPeer->GetLocation (buffer)); }
    }
property String^ Source
    {
    String^ get() {WString buffer; return gcnew String (m_baseGCSPeer->GetSource (buffer)); }
    void set (String^ value)
        {
        BI::ScopedString name (value);
        m_baseGCSPeer->SetSource (name.Uni());
        }
    }

property String^ Units
    {
    String^ get()
        {
        WString buffer;
        String^ csMapUnitName = gcnew String (m_baseGCSPeer->GetUnits(buffer));
        String^ localizedUnitName;
        if (nullptr != (localizedUnitName = GeoCoordinateLocalization::GetLocalizedStringNoSubst (csMapUnitName)))
            return localizedUnitName;
        return csMapUnitName;
        }
    }
property String^ DatumName
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetDatumName());}
    }

property GeoDatumToWGS84Method DatumConvertMethod
    {
    GeoDatumToWGS84Method get () {return (GeoDatumToWGS84Method) m_baseGCSPeer->GetDatumConvertMethod(); }
    }

property int DatumCode
    {
    int get() {return m_baseGCSPeer->GetDatumCode();}
    void set (int value) { m_baseGCSPeer->SetDatumCode (value); }
    }
property String^ DatumDescription
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetDatumDescription());}
    }
property String^ DatumSource
    {
    String^ get() {WString buffer; return gcnew String (m_baseGCSPeer->GetDatumSource (buffer)); }
    }

void    DatumParametersValid ([SRI::Out] bool% outDeltaValid, [SRI::Out] bool% outRotationValid, [SRI::Out] bool% outScaleValid)
    {
    bool    deltaValid, rotationValid, scaleValid;
    m_baseGCSPeer->DatumParametersValid (deltaValid, rotationValid, scaleValid);
    outDeltaValid       = deltaValid;
    outRotationValid    = rotationValid;
    outScaleValid       = scaleValid;
    }

void    GetDatumDelta ([SRI::Out] GEOM::DPoint3d%  outDelta)
    {
    DPoint3d delta;
    m_baseGCSPeer->GetDatumDelta (delta);
    outDelta.X = delta.x;
    outDelta.Y = delta.y;
    outDelta.Z = delta.z;
    }

void    GetDatumRotation([SRI::Out] GEOM::DPoint3d%  outRotation)
    {
    DPoint3d rotation;
    m_baseGCSPeer->GetDatumRotation (rotation);
    outRotation.X = rotation.x;
    outRotation.Y = rotation.y;
    outRotation.Z = rotation.z;
    }

property double DatumScale
    {
    double get() { return m_baseGCSPeer->GetDatumScale(); }
    }

property String^ VerticalDatumName
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetVerticalDatumName());}
    }
enum class VerticalDatumCode
    {
    vdcFromDatum   = 0,    // Vertical Datum implied by Datum
    vdcNGVD29      = 1,    // Vertical Datum of 1929
    vdcNAVD88      = 2,    // Vertical Datum of 1988.
    vdcGeoid       = 3     // Other Geoid (indicates GeoidHeight.gdc catalog should be used)
    };
property VerticalDatumCode VerticalDatum
    {
    VerticalDatumCode get() {return static_cast<VerticalDatumCode> (m_baseGCSPeer->GetVerticalDatumCode());}
    void set (VerticalDatumCode value) { m_baseGCSPeer->SetVerticalDatumCode ((BGC::VertDatumCode) value);}
    }
property String^ EllipsoidName
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetEllipsoidName());}
    }
property int EllipsoidCode
    {
    int get() {return m_baseGCSPeer->GetEllipsoidCode ();}
    void set (int value) { m_baseGCSPeer->SetEllipsoidCode (value); }
    }
property String^ EllipsoidDescription
    {
    String^ get() {return gcnew String (m_baseGCSPeer->GetEllipsoidDescription());}
    }
property String^ EllipsoidSource
    {
    String^ get() {WString buffer; return gcnew String (m_baseGCSPeer->GetEllipsoidSource(buffer)); }
    }
property double OriginLatitude
    {
    double get() {return m_baseGCSPeer->GetOriginLatitude();}
    void set (double value) { m_baseGCSPeer->SetOriginLatitude (value); }
    }
property double OriginLongitude
    {
    double get() {return m_baseGCSPeer->GetOriginLongitude();}
    void set (double value) { m_baseGCSPeer->SetOriginLongitude (value); }
    }
property double FalseEasting
    {
    double get() {return m_baseGCSPeer->GetFalseEasting();}
    void set (double value) { m_baseGCSPeer->SetFalseEasting (value); }
    }
property double FalseNorthing
    {
    double get() {return m_baseGCSPeer->GetFalseNorthing();}
    void set (double value) { m_baseGCSPeer->SetFalseNorthing (value); }
    }
property double ScaleReduction
    {
    double get() {return m_baseGCSPeer->GetScaleReduction();}
    void set (double value) { m_baseGCSPeer->SetScaleReduction (value); }
    }
property double EllipsoidPolarRadius
    {
    double get() {return m_baseGCSPeer->GetEllipsoidPolarRadius();}
    }
property double EllipsoidEquatorialRadius
    {
    double get() {return m_baseGCSPeer->GetEllipsoidEquatorialRadius();}
    }
property double EllipsoidEccentricity
    {
    double get() {return m_baseGCSPeer->GetEllipsoidEccentricity();}
    }
property double MinimumLongitude
    {
    double get() {return m_baseGCSPeer->GetMinimumLongitude();}
    void set (double value) { m_baseGCSPeer->SetMinimumLongitude (value); }
    }
property double MaximumLongitude
    {
    double get() {return m_baseGCSPeer->GetMaximumLongitude();}
    void set (double value) { m_baseGCSPeer->SetMaximumLongitude (value); }
    }
property double MinimumLatitude
    {
    double get() {return m_baseGCSPeer->GetMinimumLatitude();}
    void set (double value) { m_baseGCSPeer->SetMinimumLatitude (value); }
    }
property double MaximumLatitude
    {
    double get() {return m_baseGCSPeer->GetMaximumLatitude();}
    void set (double value) { m_baseGCSPeer->SetMaximumLatitude (value); }
    }

property double MinimumUsefulLongitude
    {
    double get() {return m_baseGCSPeer->GetMinimumUsefulLongitude();}
    }
property double MaximumUsefulLongitude
    {
    double get() {return m_baseGCSPeer->GetMaximumUsefulLongitude();}
    }
property double MinimumUsefulLatitude
    {
    double get() {return m_baseGCSPeer->GetMinimumUsefulLatitude();}
    }
property double MaximumUsefulLatitude
    {
    double get() {return m_baseGCSPeer->GetMaximumUsefulLatitude();}
    }

property double StandardParallel1
    {
    double get() {return m_baseGCSPeer->GetStandardParallel1();}
    void set (double value) { m_baseGCSPeer->SetStandardParallel1 (value); }
    }
property double StandardParallel2
    {
    double get() {return m_baseGCSPeer->GetStandardParallel2();}
    void set (double value) { m_baseGCSPeer->SetStandardParallel2(value); }
    }
property double CentralMeridian
    {
    double get() {return m_baseGCSPeer->GetCentralMeridian();}
    void set (double value) { m_baseGCSPeer->SetCentralMeridian (value); }
    }
property double EasternMeridian
    {
    double get() {return m_baseGCSPeer->GetEasternMeridian();}
    void set (double value) { m_baseGCSPeer->SetEasternMeridian (value); }
    }
property double CentralPointLongitude
    {
    double get() {return m_baseGCSPeer->GetCentralPointLongitude();}
    void set (double value) { m_baseGCSPeer->SetCentralPointLongitude (value); }
    }
property double CentralPointLatitude
    {
    double get() {return m_baseGCSPeer->GetCentralPointLatitude();}
    void set (double value) { m_baseGCSPeer->SetCentralPointLatitude (value); }
    }
property double Azimuth
    {
    double get() {return m_baseGCSPeer->GetAzimuth();}
    void set (double value) { m_baseGCSPeer->SetAzimuth (value); }
    }
property double GeoidSeparation
    {
    double get() {return m_baseGCSPeer->GetGeoidSeparation();}
    void set (double value) { m_baseGCSPeer->SetGeoidSeparation (value); }
    }
property double ElevationAboveGeoid
    {
    double get() {return m_baseGCSPeer->GetElevationAboveGeoid();}
    void set (double value) { m_baseGCSPeer->SetElevationAboveGeoid (value); }
    }
property int    UTMZone
    {
    int get() {return m_baseGCSPeer->GetUTMZone();}
    void set (int value) { m_baseGCSPeer->SetUTMZone (value); }
    }
property int    Hemisphere
    {
    int get() {return m_baseGCSPeer->GetHemisphere();}
    void set (int value) { m_baseGCSPeer->SetHemisphere (value); }
    }
property int    Quadrant
    {
    int get() {return m_baseGCSPeer->GetQuadrant();}
    void set (int value) { m_baseGCSPeer->SetQuadrant (static_cast<short>(value)); } // NEEDSWORK - is cast correct?  Should this function take a short?
    }
property int    DanishSys34Region
    {
    int get() {return m_baseGCSPeer->GetDanishSys34Region();}
    void set (int value) { m_baseGCSPeer->SetDanishSys34Region (value); }
    }
property double Point1Longitude
    {
    double get() {return m_baseGCSPeer->GetPoint1Longitude();}
    void set (double value) { m_baseGCSPeer->SetPoint1Longitude (value); }
    }
property double Point1Latitude
    {
    double get() {return m_baseGCSPeer->GetPoint1Latitude();}
    void set (double value) { m_baseGCSPeer->SetPoint1Latitude (value); }
    }
property double Point2Longitude
    {
    double get() {return m_baseGCSPeer->GetPoint2Longitude();}
    void set (double value) { m_baseGCSPeer->SetPoint2Longitude (value); }
    }
property double Point2Latitude
    {
    double get() {return m_baseGCSPeer->GetPoint2Latitude();}
    void set (double value) { m_baseGCSPeer->SetPoint2Longitude (value); }
    }

property double AffineA0
    {
    double get() {return m_baseGCSPeer->GetAffineA0();}
    void set (double value) { m_baseGCSPeer->SetAffineA0(value); }
    }
property double AffineA1
    {
    double get() {return m_baseGCSPeer->GetAffineA1();}
    void set (double value) { m_baseGCSPeer->SetAffineA1(value); }
    }
property double AffineA2
    {
    double get() {return m_baseGCSPeer->GetAffineA2();}
    void set (double value) { m_baseGCSPeer->SetAffineA2(value); }
    }
property double AffineB0
    {
    double get() {return m_baseGCSPeer->GetAffineB0();}
    void set (double value) { m_baseGCSPeer->SetAffineB0(value); }
    }
property double AffineB1
    {
    double get() {return m_baseGCSPeer->GetAffineB1();}
    void set (double value) { m_baseGCSPeer->SetAffineB1(value); }
    }
property double AffineB2
    {
    double get() {return m_baseGCSPeer->GetAffineB2();}
    void set (double value) { m_baseGCSPeer->SetAffineB2(value); }
    }

property bool   ReprojectElevation
    {
    bool get() {return m_baseGCSPeer->GetReprojectElevation();}
    void set (bool value) { m_baseGCSPeer->SetReprojectElevation (value); }
    }

property bool   CanEdit
    {
    bool get() {return m_baseGCSPeer->GetCanEdit();}
    void set (bool value) { m_baseGCSPeer->SetCanEdit (value); }
    }
property bool   IsNAD27
    {
    bool get() { return m_baseGCSPeer->IsNAD27();}
    }
property bool   IsNAD83
    {
    bool get() { return m_baseGCSPeer->IsNAD83();}
    }

internal: property BGC::LibraryP   SourceLibrary
    {
    BGC::LibraryP get() { return m_baseGCSPeer->GetSourceLibrary(); }
    }

public:          
property int    LocalTransformType
    {
    int get ()
        {
        BGC::LocalTransformer* transformer = m_baseGCSPeer->GetLocalTransformer();
        if (NULL == transformer)
            return BGC::TRANSFORM_None;

        if (NULL != dynamic_cast <BGC::HelmertLocalTransformer*> (transformer))
            return BGC::TRANSFORM_Helmert;

    #if defined (SecondOrderConformal)
        if (NULL != dynamic_cast <BGC::SecondOrderConformalTransformer*> (transformer))
            return BGC::TRANSFORM_SecondOrderConformal2;
    #endif

        return 0;
        }
    void set (int value)
        {
        if (BGC::TRANSFORM_None == value)
            m_baseGCSPeer->SetLocalTransformer (NULL);
        else if (BGC::TRANSFORM_Helmert == value)
            m_baseGCSPeer->SetLocalTransformer (BGC::HelmertLocalTransformer::Create (1.0, 0.0, 0.0, 0.0, 0.0));
    #if defined (SecondOrderConformal)
        else if (BGC::TRANSFORM_SecondOrderConformal == value)
            m_baseGCSPeer->SetLocalTransformer (BGC::SecondOrderConrformalTransformer::Create (1.0, 0.0, 0.0, 0.0, 0.0));
    #endif
        }
    }

};

/*=================================================================================**//**
* Ellipsoid
* @bsiclass                                                     Barry.Bentley   03/08
+===============+===============+===============+===============+===============+======*/
public ref class   Ellipsoid
{
private:
BGC::EllipsoidP     m_ellipsoid;
bool                m_canEdit;

public:
/*---------------------------------------------------------------------------------**//**
* Ellipsoid constructor
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid (String^ keyName)
    {
    BI::ScopedString name (keyName);
    WCharCP nameCP = name.Uni();
    m_ellipsoid = const_cast <BGC::EllipsoidP> (BGC::Ellipsoid::CreateEllipsoid (nameCP));
    if (!m_ellipsoid->IsValid())
        m_ellipsoid = NULL;
    m_canEdit = false;
    }

/*---------------------------------------------------------------------------------**//**
* Ellipsoid constructor
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
Ellipsoid (CSEllipsoidDef const& ellipsoidDef, BGC::LibraryP library)
    {
    m_ellipsoid = const_cast <BGC::EllipsoidP> (BGC::Ellipsoid::CreateEllipsoid (ellipsoidDef, library));
    m_canEdit = false;
    }

public:
/*---------------------------------------------------------------------------------**//**
* Ellipsoid constructor
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid (Ellipsoid^ sourceEllipsoid)
    {
    m_ellipsoid = sourceEllipsoid->m_ellipsoid->Clone();
    m_canEdit   = false;
    }

/*---------------------------------------------------------------------------------**//**
* Returns whether the Ellipsoid is valid or not.
* @return   True if the Ellipsoid is valid, False otherwise. @see #GetError
* @remarks  If the Ellipsoid does not correspond to a Ellipsoid in the Ellipsoid library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property bool Valid { bool get() { return nullptr != m_ellipsoid; } }

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property int Error { int get () { return m_ellipsoid->GetError(); } }

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ ErrorMessage 
    { 
    String^ get() { WString buffer; return gcnew String (m_ellipsoid->GetErrorMessage(buffer)); } 
    }

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Ellipsoid
* @return   The name of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Name 
    { 
    String^ get() {return gcnew String (m_ellipsoid->GetName()); }
    void    set (String^ value)
        {
        BI::ScopedString name (value);
        m_ellipsoid->SetName (name.Uni());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Ellipsoid.
* @return   The description of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Description 
    { 
    String^ get() {return gcnew String (m_ellipsoid->GetDescription());}
    void    set (String^ value) 
        {
        BI::ScopedString description (value);
        m_ellipsoid->SetDescription (description.Uni());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Ellipsoid
* @return   The source of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Source 
    { 
    String^ get() { WString buffer; return gcnew String (m_ellipsoid->GetSource(buffer)); }
    void    set (String^ value) 
        {
        BI::ScopedString source (value);
        m_ellipsoid->SetSource (source.Uni());
        }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the polar radius of the Ellipsoid.
* @return   The polar radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property double PolarRadius 
    { 
    double get() {return m_ellipsoid->GetPolarRadius(); }
    void   set(double value) {m_ellipsoid->SetPolarRadius (value); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the equatorial radius of the Ellipsoid.
* @return   The equatorial radius of the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property double EquatorialRadius 
    { 
    double get() {return m_ellipsoid->GetEquatorialRadius(); }
    void   set(double value) {m_ellipsoid->SetEquatorialRadius (value); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the eccentricity value for the Ellipsoid.
* @return   The eccentricity value for the Ellipsoid.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property double Eccentricity { double get() {return m_ellipsoid->GetEccentricity(); } }

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Ellipsoid, if known.
* @return   The EPSG code.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property int   EPSGCode { int get() { return m_ellipsoid->GetEPSGCode(); } }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
property bool   CanEdit
    {
    bool get() { return m_canEdit; }
    void set (bool value) { m_canEdit = value; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool    NameUnique ([SRI::Out] bool% inSystemLibrary)
    {
    bool    systemLibrary;
    bool    returnVal =  m_ellipsoid->NameUnique (systemLibrary);
    inSystemLibrary = systemLibrary;
    return returnVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ReplaceInLibrary (Ellipsoid^ replacement)
    {
    return m_ellipsoid->ReplaceInLibrary (replacement->m_ellipsoid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AddToLibrary ()
    {
    return m_ellipsoid->AddToLibrary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
~Ellipsoid()
    {
    // Note, this method is internally renamed to Dispose, and does the "SuppressFinalization" for us.
    m_ellipsoid->Destroy();
    }

}; // end of Ellipsoid class

/*=================================================================================**//**
* Datum managed class
* @bsiclass                                                     Barry.Bentley   03/08
+===============+===============+===============+===============+===============+======*/
public ref class    Datum
{
private:
BGC::DatumP         m_datum;
bool                m_canEdit;

internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (CSDatumDef const& datumDef, BGC::LibraryP library)
    {
    m_datum   = const_cast <BGC::DatumP> (BGC::Datum::CreateDatum (datumDef, library));
    m_canEdit = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (BGC::DatumP datum)
    {
    m_datum     = datum;
    m_canEdit   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (Datum^ sourceDatum)
    {
    m_datum     = sourceDatum->m_datum->Clone();
    m_canEdit   = false;
    }

public:
/*---------------------------------------------------------------------------------**//**
* Datum constructor
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Datum (String^ keyName)
    {
    BI::ScopedString name (keyName);
    WCharCP nameCP = name.Uni();
    m_datum = const_cast <BGC::DatumP> (BGC::Datum::CreateDatum (nameCP));
    if (!m_datum->IsValid())
        m_datum = NULL;
    m_canEdit = false;
    }

/*---------------------------------------------------------------------------------**//**
* Returns whether the Datum is valid or not.
* @return   True if the Datum is valid, False otherwise. @see #GetError
* @remarks  If the Datum does not correspond to a Datum in the Datum library, IsValid is false, and GetError and
*           GetErrorMessage can be used to obtain more details.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property bool Valid { bool get() { return nullptr != m_datum; } }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
property bool CanEdit
    {
    bool get() { return (NULL != m_datum) && m_canEdit; }
    void set (bool value) { m_canEdit = value; }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the error code associated with constructor failure if IsValid is false.
* @return   The CSMap error code. @see #GetErrorMessage
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property int Error { int get () { return m_datum->GetError(); } }

/*---------------------------------------------------------------------------------**//**
* Gets the error message associated with constructor failure if IsValid is false.
* @return   The CSMap error message. @see #GetError
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ ErrorMessage 
    { 
    String^ get() { WString buffer; return gcnew String (m_datum->GetErrorMessage(buffer)); } 
    }

/*---------------------------------------------------------------------------------**//**
* Gets the name of the Datum
* @return   The name of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Name
    {
    String^ get() {return gcnew String (m_datum->GetName()); }
    void set (String^ value) { BI::ScopedString tempString(value); m_datum->SetName (tempString.Uni()); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Datum.
* @return   The description of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Description
    {
    String^ get() {return gcnew String (m_datum->GetDescription());}
    void set (String^ value) { BI::ScopedString tempString(value); m_datum->SetDescription (tempString.Uni()); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Datum
* @return   The source of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Source
    {
    String^ get() {WString buffer; return gcnew String (m_datum->GetSource(buffer));} 
    void set (String^ value) { BI::ScopedString tempString(value); m_datum->SetSource (tempString.Uni()); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets/Sets the Ellipsoid code
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property int EllipsoidCode
    {
    int get() {return m_datum->GetEllipsoidCode();}
    void set (int value) { m_datum->SetEllipsoidCode (value); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the description of the Ellipsoid of the Datum.
* @return   The description of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ EllipsoidDescription
    {
    String^ get() {return gcnew String (m_datum->GetEllipsoidDescription ()); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the source of the Ellipsoid of the Datum
* @return   The source of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ EllipsoidSource
    {
    String^ get() {WString buffer; return gcnew String (m_datum->GetEllipsoidSource(buffer));}
    }

/*---------------------------------------------------------------------------------**//**
* Gets the method of converting to WGS84
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property GeoDatumToWGS84Method ConvertToWGS84Method
    {
    GeoDatumToWGS84Method  get() {return (GeoDatumToWGS84Method) m_datum->GetConvertToWGS84MethodCode(); }
    void set (GeoDatumToWGS84Method value) {m_datum->SetConvertToWGS84MethodCode ((BGC::WGS84ConvertCode)value); }
    }

/*---------------------------------------------------------------------------------**//**
* Gets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @return   The equatorial radius of the Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetDelta ([SRI::Out] GEOM::DPoint3d% outDelta)
    {
    DPoint3d delta;
    m_datum->GetDelta (delta);
    outDelta.X = delta.x;
    outDelta.Y = delta.y;
    outDelta.Z = delta.z;
    }

/*---------------------------------------------------------------------------------**//**
* Sets the vector from the geocenter of the WGS84 Datum to the geocenter of this Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetDelta (GEOM::DPoint3d inDelta)
    {
    DPoint3d delta;
    delta.x = inDelta.X;
    delta.y = inDelta.Y;
    delta.z = inDelta.Z;
    m_datum->SetDelta (delta);
    }

/*---------------------------------------------------------------------------------**//**
* Gets the angles from the WGS84 x, y, and z axes to those of this Datum
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetRotation (GEOM::DPoint3d% outRotation)
    {
    DPoint3d rotation;
    m_datum->GetRotation (rotation);
    outRotation.X = rotation.x;
    outRotation.Y = rotation.y;
    outRotation.Z = rotation.z;
    }

/*---------------------------------------------------------------------------------**//**
* Sets the angles from the WGS84 x, y, and z axes to those of this Datum
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetRotation (GEOM::DPoint3d inRotation)
    {
    DPoint3d rotation;
    rotation.x = inRotation.X;
    rotation.y = inRotation.Y;
    rotation.z = inRotation.Z;
    m_datum->SetRotation (rotation);
    }

/*---------------------------------------------------------------------------------**//**
* Gets the datum transformation scale factor (in parts per million) for this Datum, if known.
* @return   The scale in ppm.
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
 property double Scale
    {
    double get() {return m_datum->GetScale();}
    void set (double value) {m_datum->SetScale (value);}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ParametersValid ([SRI::Out] bool% outDeltaValid, [SRI::Out] bool% outRotationValid, [SRI::Out] bool% outScaleValid)
    {
    bool    deltaValid, rotationValid, scaleValid;
    m_datum->ParametersValid (deltaValid, rotationValid, scaleValid);
    outDeltaValid       = deltaValid;
    outRotationValid    = rotationValid;
    outScaleValid       = scaleValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool    NameUnique ([SRI::Out] bool% inSystemLibrary)
    {
    bool    systemLibrary;
    bool    returnVal =  m_datum->NameUnique (systemLibrary);
    inSystemLibrary = systemLibrary;
    return returnVal;
    }

/*---------------------------------------------------------------------------------**//**
* Gets the EPSG code for this Datum, if known.
* @return   The EPSG code.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property int   EPSGCode { int get() { return m_datum->GetEPSGCode(); } }

internal: property BGC::LibraryP   SourceLibrary
    {
    BGC::LibraryP get() { return m_datum->GetSourceLibrary(); }
    }

public:          
property String^ EllipsoidName
    {
    String^ get() {return gcnew String (m_datum->GetEllipsoidName());}
    }
property double EllipsoidPolarRadius
    {
    double get() {return m_datum->GetEllipsoidPolarRadius();}
    }
property double EllipsoidEquatorialRadius
    {
    double get() {return m_datum->GetEllipsoidEquatorialRadius();}
    }
property double EllipsoidEccentricity
    {
    double get() {return m_datum->GetEllipsoidEccentricity();}
    }

/*---------------------------------------------------------------------------------**//**
* Gets the Ellipsoid for this Datum.
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
property Bentley::GeoCoordinatesNET::Ellipsoid^  Ellipsoid 
    { 
    Bentley::GeoCoordinatesNET::Ellipsoid^ get() { return gcnew Bentley::GeoCoordinatesNET::Ellipsoid (gcnew String (m_datum->GetEllipsoidName())); } 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
~Datum ()
    {
    // Note, this method is internally renamed to Dispose, and does the "SuppressFinalization" for us.
    m_datum->Destroy();
    }

internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       AddToLibrary ()
    {
    return m_datum->AddToLibrary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ReplaceInLibrary (Datum^ replacement)
    {
    return m_datum->ReplaceInLibrary (replacement->m_datum);
    }

}; // End of Datum class



public delegate bool    SearchProgressDelegate (unsigned int percentComplete, System::Object^ userObject);

/*=================================================================================**//**
*
* This is a managed class that represents a Coordinate System Group
*
+===============+===============+===============+===============+===============+======*/
public ref class     LibrarySearcher
{
private:
SCG::List<BaseGCS^>^    m_allCoordinateSystems;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
SCG::List<BaseGCS^>^    GetAllCoordinateSystems
(
SearchProgressDelegate^     progressIndicator,
System::Object^             progressArg
)
    {
    if (nullptr != m_allCoordinateSystems)
        return m_allCoordinateSystems;

    m_allCoordinateSystems = gcnew SCG::List<BaseGCS^>(2000);

    // add all of the coordinate systems from all of our libraries.
    BGC::LibraryManager*    libManager      = BGC::LibraryManager::Instance();
    size_t                  libraryCount    = libManager->GetLibraryCount();
    for (size_t libIndex=0; libIndex < libraryCount; libIndex++)
        {
        BGC::LibraryP    library;

        if (NULL != (library = libManager->GetLibrary (libIndex)))
            {
            size_t          csCount  = library->GetCSCount();
            size_t          interval = 1 + (csCount / 20);
            size_t          csIndex  = 0;
            SCG::IEnumerator<BaseGCS^>^ memberEnumerator = gcnew GCSMemberEnumerator (library);
            while (memberEnumerator->MoveNext())
                {
                m_allCoordinateSystems->Add (memberEnumerator->Current);
                if ( (nullptr != progressIndicator) && (0 == (csIndex % interval)) )
                    {
                    if (progressIndicator ( static_cast<int>((100 * csIndex) / csCount), progressArg))
                        {
                        // if told to abort the mission, free what we've got so far.
                        FreeAllCoordinateSystems();
                        return nullptr;
                        }
                    }
                csIndex++;
                }
            }
        }

    if (nullptr != progressIndicator)
        progressIndicator (100, progressArg);

    return m_allCoordinateSystems;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
~LibrarySearcher
(
)
    {
    FreeAllCoordinateSystems();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            FreeAllCoordinateSystems ()
    {
    if (nullptr == m_allCoordinateSystems)
        return;

    for each (BaseGCS^ member in m_allCoordinateSystems)
        delete member;

    m_allCoordinateSystems = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
SCG::List<BaseGCS^>^    FindInLibrary
(
array<String^>^             matchStrings,
bool                        anyWord,
SearchProgressDelegate^     progressIndicator,
System::Object^             progressArg
)
    {
    int     numStrings = matchStrings->Length;
    SCG::List<String^>^ original = gcnew SCG::List<String^> (numStrings);
    SCG::List<String^>^ upper    = gcnew SCG::List<String^> (numStrings);
    for (int iString=0; iString < numStrings; iString++)
        {
        if (0 != matchStrings[iString]->Length)
            original->Add (matchStrings[iString]);
        }
    // get upper case versions
    for (int iString=0; iString < numStrings; iString++)
        {
        if (0 != matchStrings[iString]->Length)
            {
            String^ upperCase = matchStrings[iString]->ToUpper();
                upper->Add (upperCase);
            }
        }

    SCG::List<ScoreMember^>^ scoredResults = gcnew SCG::List<ScoreMember^>();
    SCG::List<BaseGCS^>^     allCoordinateSystems = GetAllCoordinateSystems (progressIndicator, progressArg);

    if (nullptr == allCoordinateSystems)
        return  nullptr;

    unsigned int libraryCount = allCoordinateSystems->Count;
    unsigned int interval     = libraryCount/20;
    unsigned int index        = 0;
    for each (BaseGCS^ member in allCoordinateSystems)
        {
        int score;
        if (0 < (score = member->Matches (original, upper, anyWord)))
            {
            ScoreMember^ scoreMember = gcnew ScoreMember (score, member);
            scoredResults->Add (scoreMember);
            }
        if ( (nullptr != progressIndicator) && (0 == (index % interval)) )
            {
            if (progressIndicator ( ((100 * index) / libraryCount), progressArg))
                break;
            }

        index++;
        }

    scoredResults->Sort(gcnew ScoreComparer);
    SCG::List<BaseGCS^>^ results = gcnew SCG::List<BaseGCS^> (scoredResults->Count);
    for each (ScoreMember^ scoredResult in scoredResults)
        {
        // never give out the original in the list, make a copy. That's so we can dispose our members without problems.
        results->Add (gcnew BaseGCS (scoredResult->m_member->Name));
        }

    if (nullptr != progressIndicator)
        progressIndicator (100, progressArg);

    return results;
    }

ref class ScoreMember
{
public:
int         m_score;
BaseGCS^    m_member;

ScoreMember (int score, BaseGCS^ member) {m_score = score; m_member = member;};

};

ref class ScoreComparer : SCG::IComparer<ScoreMember^>
{
public:
virtual int Compare (ScoreMember ^first, ScoreMember^ second)
    {
    return (second->m_score - first->m_score);
    }
};

};

/*=================================================================================**//**
*
* This is a managed class that represents a Coordinate System Group
*
+===============+===============+===============+===============+===============+======*/
public ref class GCSGroup : public SCG::IEnumerable<BaseGCS^>
{
private:
String^     m_name;
String^     m_description;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
GCSGroup
(
String^     name,
String^     description
)
    {
    m_name = name;
    m_description = description;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Name
    {
    String^ get() {return m_name;}
    }
property String^ Description
    {
    String^ get() {return m_description;}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SCG::IEnumerator<BaseGCS^>^  GetEnumerator()
    {
    return gcnew GCSMemberEnumerator(this);
    }

// Here is an example of explicit override of an interface method.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SC::IEnumerator^  GetNonGenericEnumerator () = SC::IEnumerable::GetEnumerator
    {
    return GetEnumerator();
    }

};  // GSCGroup

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSMemberEnumerator::MoveNext
(
)
    {
    for (;;)
        {
        m_currentIndex++;
        try
            {
            if (nullptr != m_group)
                {
                BI::ScopedString    groupName (m_group->Name);
                CSGroupList         csGroupList;
                if (0 < BGC::CSMap::CS_csEnumByGroup (m_currentIndex, groupName.Ansi(), &csGroupList))
                    {
                    // found a coordinate system.
                    m_current = gcnew BaseGCS (WString(csGroupList.key_nm).c_str());
                    return true;
                    }
                return false;
                }
            else if (NULL != m_library)
                {
                if (m_currentIndex < (int) m_library->GetCSCount())
                    {
                    CSParameters *params;
                    if (NULL == (params = m_library->GetCS (m_currentIndex)))
                        continue;

                    BGC::BaseGCSPtr gcs = BGC::BaseGCS::CreateGCS (*params, COORDSYS_KEYNM, m_library);
                    BGC::CSMap::CS_free (params);

                    m_current = gcnew BaseGCS (gcs.get());
                    return true;
                    }
                return false;
                }
            else
                {
                // enumerating all member of all groups.
                char        csKeyName[1024];
                if (0 < BGC::CSMap::CS_csEnum (m_currentIndex, csKeyName, sizeof(csKeyName)))
                    {
                    m_current = gcnew BaseGCS (WString(csKeyName).c_str());
                    return true;
                    }
                return false;
                }
            }
        // discard any for which we get a constructor failure.
        catch (GeoCoordinateException::ConstructorFailure^)
            {
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSNameEnumerator::MoveNext
(
)
    {
    m_currentIndex++;

    if (nullptr != m_group)
        {
        BI::ScopedString    groupName (m_group->Name);
        CSGroupList         csGroupList;
        if (0 < BGC::CSMap::CS_csEnumByGroup (m_currentIndex, groupName.Ansi(), &csGroupList))
            {
            // found a coordinate system.
            m_current = gcnew String (csGroupList.key_nm);
            return true;
            }
        }
    else if (NULL != m_library)
        {
        if (m_currentIndex < (int) m_library->GetCSCount())
            {
            WString csName;
            m_library->GetCSName (m_currentIndex, csName);
            m_current = gcnew String (csName.c_str());
            return true;
            }
        return false;
        }
    else
        {
        // enumerating all members.
        char        csKeyName[1024];
        if (0 < BGC::CSMap::CS_csEnum (m_currentIndex, csKeyName, sizeof(csKeyName)))
            {
            m_current = gcnew String (WString (csKeyName).c_str());
            return true;
            }
        }
    return false;
    }


/*=================================================================================**//**
*
* This is a managed class that enumerates the groups within the coordinate system dictionary
*
+===============+===============+===============+===============+===============+======*/
public ref class GCSGroupEnumerator : SCG::IEnumerator<GCSGroup^>
{
private:

int         m_currentIndex;
GCSGroup^   m_current;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
GCSGroupEnumerator
(
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property GCSGroup^ Current
    {
    GCSGroup^ get()
        {
        return m_current;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property System::Object^ NonGenericCurrent
    {
    System::Object^ get() = SC::IEnumerator::Current::get
        {
        return m_current;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool MoveNext
(
)
    {
    m_currentIndex++;
    char        groupName[1024];
    char        groupDescription[2048];

    if (0 < BGC::CSMap::CS_csGrpEnum (m_currentIndex, groupName, sizeof(groupName), groupDescription, sizeof groupDescription))
        {
        String^ name        = SRI::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(groupName));
        String^ description = SRI::Marshal::PtrToStringAnsi(static_cast<System::IntPtr>(groupDescription));
        m_current = gcnew GCSGroup (name, description);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void Reset
(
)
    {
    m_currentIndex  = -1;
    m_current       = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
~GCSGroupEnumerator
(
)
    {
    // clean up code to release managed resource
    // (none)
    // to avoid code duplication
    // call finalizer to release unmanaged resources
    this->!GCSGroupEnumerator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod (Dispose method)                                   Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
!GCSGroupEnumerator
(
)
    {
    // finalizer cleans up unmanaged resources
    // destructor or garbage collector will
    // clean up managed resources
    // clean up code to release unmanaged resource
    // (we don't really have any in this class, but need to implement Dispose to satisfy interface.
    }

};


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    GCSAngleTypeConverter : public ECUI::ECCustomFormatTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Overrides ConvertTo to extract the DisplayString from the structVal</summary>
/// <author>Barry.Bentley</author>                              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public:
virtual String^          ConvertToString
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor,
int                                     componentIndex,
SG::CultureInfo^                        culture,
System::Object^                         value
) override
    {
    String^     stringValue;
    bool        negative = false;

    if (nullptr != dynamic_cast <System::Double^> (value))
        {
        double doubleVal = (double) value;
        if (doubleVal < 0)
            {
            doubleVal = -doubleVal;
            negative  = true;
            }

        double seconds   = doubleVal * 3600.0;

        // divide into degrees, minutes, seconds
        int degrees = (int) (seconds / 3600.0);
        seconds = seconds - 3600.0 * degrees;

        int minutes = (int) (seconds / 60.0);
        seconds = seconds - 60.0 * minutes;
        String^ formatString;
        if (negative)
            formatString = "-{0:00}\xb0{1:00}'{2:00.0000}\"";
        else
            formatString = "{0:00}\xb0{1:00}'{2:00.0000}\"";
        return String::Format (formatString, degrees, minutes, seconds);
        }
    else if (nullptr != (stringValue = dynamic_cast <String^> (value)))
        {
        return stringValue;
        }

    return nullptr;
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Overrides ConvertFrom to return the right type</summary>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
virtual System::Object^  ConvertFromString
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor,
int                                     componentIndex,
SG::CultureInfo^                        culture,
String^                                 value
) override
    {
    // we want to handle the MicroStation style keyins:
    // 30^20'10", or 30:20:10, or 30d20m10s or 30(0xb0)20'10".
    // If there's a trailing W or S, the value is negative.
    // The trailing " or s is not necessary.
    // 30^10" is the same as 30^0'10"
    // Any of the degrees, minutes, seconds can have decimal within, although of course
    //  it is not recommended to put in something like 30.5^30'10" - that results in 31^0'10"

    // white space has no significance, remove it.
    value = value->Trim();

    // if the string ends with W or S, treat as negative.
    bool    negative            = false;
    if (value->EndsWith ("W", System::StringComparison::OrdinalIgnoreCase) || value->EndsWith ("S", System::StringComparison::OrdinalIgnoreCase) )
        {
        negative = true;
        value    = value->Substring (0, value->Length-1);
        }
    else if (value->EndsWith ("E", System::StringComparison::OrdinalIgnoreCase) || value->EndsWith ("N", System::StringComparison::OrdinalIgnoreCase) )
        {
        value    = value->Substring (0, value->Length-1);
        }

    // Find the delimiters and get the starting and ending positions of the degrees, minutes, and seconds strings.
    int     degreesStringEnd    = -1;
    int     minutesStringStart  = 0;
    int     minutesStringEnd    = -1;
    int     secondsStringStart  = 0;
    int     secondsStringEnd    = -1;
    array<wchar_t>^ degreeDelimiters =  {'^', 0xb0, ':', 'd', 'D'};
    array<wchar_t>^ minutesDelimiters = {'\'', ':', 'm', 'M'};
    array<wchar_t>^ secondsDelimiters = {'"', ':', 's', 'S'};

    if (-1 != (degreesStringEnd = value->IndexOfAny (degreeDelimiters)))
        {
        minutesStringStart  = degreesStringEnd+1;
        secondsStringStart  = degreesStringEnd+1;
        }
    if (-1 != (minutesStringEnd = value->IndexOfAny (minutesDelimiters, minutesStringStart)))
        {
        secondsStringStart  = minutesStringEnd+1;
        }
    secondsStringEnd = value->IndexOfAny (secondsDelimiters, secondsStringStart);

    // if no delimiters (degreesStringEnd, minutesStringEnd and secondsStringEnd all 0), treat whole thing as just degrees.
    if ( (-1 == degreesStringEnd) && (-1 == minutesStringEnd) && (-1 == secondsStringEnd) )
        {
        degreesStringEnd   = value->Length;
        minutesStringStart = secondsStringStart = degreesStringEnd+1;
        }
    // if minutesStringStart > 0, we found a delimiter after the degrees.
    else if ( (0 != minutesStringStart) && (-1 == minutesStringEnd) && (-1 == secondsStringEnd) )
        {
        minutesStringEnd    = value->Length;
        secondsStringStart  = minutesStringEnd+1;
        }
    else if ( (0 != secondsStringStart) && (-1 == secondsStringEnd) )
        {
        secondsStringEnd    = value->Length;
        }

    double  degrees = 0.0;
    double  minutes = 0.0;
    double  seconds = 0.0;

    if (degreesStringEnd > 0)
        degrees = System::Double::Parse (value->Substring (0, degreesStringEnd));
    if (minutesStringEnd > minutesStringStart)
        minutes = System::Double::Parse (value->Substring (minutesStringStart, minutesStringEnd - minutesStringStart));
    if (secondsStringEnd > secondsStringStart)
        seconds = System::Double::Parse (value->Substring (secondsStringStart, secondsStringEnd - secondsStringStart));

    degrees += minutes / 60.0;
    degrees += seconds / 3600.0;

    if (negative)
        degrees = -1.0 * degrees;

    return degrees;
    }

};


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
ref class   GCSLatitudeTypeConverter : public GCSAngleTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Overrides ConvertTo to extract the DisplayString from the structVal</summary>
/// <author>Barry.Bentley</author>                              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public:
virtual String^          ConvertToString
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor,
int                                     componentIndex,
SG::CultureInfo^                        culture,
System::Object^                         value
) override
    {
    String^     stringValue;
    bool        negative = false;

    if (nullptr != dynamic_cast <System::Double^> (value))
        {
        double doubleVal = (double) value;
        if (doubleVal < 0)
            {
            doubleVal = -doubleVal;
            negative  = true;
            }

        double seconds   = doubleVal * 3600.0;

        // divide into degrees, minutes, seconds
        int degrees = (int) (seconds / 3600.0);
        seconds = seconds - 3600.0 * degrees;

        int minutes = (int) (seconds / 60.0);
        seconds = seconds - 60.0 * minutes;

        //  Don't let String::Format round up to values like 35°59'60.0000N"
        if (seconds >= 59.99995)
            {
            seconds = 0;
            minutes++;
            if (minutes >= 60)
                {
                minutes = 0;
                degrees++;
                }
            }

        String^ formatString;
        if (negative)
            formatString = "{0:00}\xb0{1:00}'{2:00.0000}\"S";
        else
            formatString = "{0:00}\xb0{1:00}'{2:00.0000}\"N";
        return String::Format (formatString, degrees, minutes, seconds);
        }
    else if (nullptr != (stringValue = dynamic_cast <String^> (value)))
        {
        return stringValue;
        }

    return nullptr;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
ref class   GCSLongitudeTypeConverter : public GCSAngleTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Overrides ConvertTo to extract the DisplayString from the structVal</summary>
/// <author>Barry.Bentley</author>                              <date>08/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
public:
virtual String^          ConvertToString
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor,
int                                     componentIndex,
SG::CultureInfo^                        culture,
System::Object^                         value
) override
    {
    String^     stringValue;
    bool        negative = false;

    if (nullptr != dynamic_cast <System::Double^> (value))
        {
        double doubleVal = (double) value;
        if (doubleVal < 0)
            {
            doubleVal = -doubleVal;
            negative  = true;
            }

        double seconds   = doubleVal * 3600.0;

        // divide into degrees, minutes, seconds
        int degrees = (int) (seconds / 3600.0);
        seconds = seconds - 3600.0 * degrees;

        int minutes = (int) (seconds / 60.0);
        seconds = seconds - 60.0 * minutes;

        //  Don't let String::Format round up to values like 35°59'60.0000N"
        if (seconds >= 59.99995)
            {
            seconds = 0;
            minutes++;
            if (minutes >= 60)
                {
                minutes = 0;
                degrees++;
                }
            }

        String^ formatString;
        if (negative)
            formatString = "{0:00}\xb0{1:00}'{2:00.0000}\"W";
        else
            formatString = "{0:00}\xb0{1:00}'{2:00.0000}\"E";
        return String::Format (formatString, degrees, minutes, seconds);
        }
    else if (nullptr != (stringValue = dynamic_cast <String^> (value)))
        {
        return stringValue;
        }

    return nullptr;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   07/07
+===============+===============+===============+===============+===============+======*/
ref class    GCSUnitsTypeConverter : public ECUI::ECSimpleListTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Gets the StandardValueMembers that make up the choices presented to the user.</summary>
/// <remarks>This is typically the only method that the derived class overrides.</remarks>
/// <returns>List of ECStandardValueMember objects.</returns>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
protected:
virtual ECUI::ECStandardValueMemberArray^ GetStandardValueMemberArray
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor
) override
    {
    ECUI::ECStandardValueMemberArray^ memberArray = gcnew ECUI::ECStandardValueMemberArray ();
    T_WStringVector*            unitNames         = BGC::BaseGCS::GetUnitNames ();
    T_WStringVector::iterator   unitsIterator;
    int                         iUnit;
    for (iUnit=0, unitsIterator = unitNames->begin(); unitsIterator != unitNames->end(); unitsIterator++, iUnit++)
        {
        String^ csMapUnitName = gcnew String ((*unitsIterator).data());
        String^ localizedUnitName;
        if (nullptr != (localizedUnitName = GeoCoordinateLocalization::GetLocalizedStringNoSubst (csMapUnitName)))
            csMapUnitName = localizedUnitName;

        memberArray->Add (gcnew ECUI::ECStandardValueMember (csMapUnitName, iUnit));
        }
    delete unitNames;
    return memberArray;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   07/07
+===============+===============+===============+===============+===============+======*/
ref class    GCSDatumCodeTypeConverter : public ECUI::ECSimpleListTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Gets the StandardValueMembers that make up the choices presented to the user.</summary>
/// <remarks>This is typically the only method that the derived class overrides.</remarks>
/// <returns>List of ECStandardValueMember objects.</returns>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
protected:
virtual ECUI::ECStandardValueMemberArray^ GetStandardValueMemberArray
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor
) override
    {
    ECUI::ECStandardValueMemberArray^   memberArray = gcnew ECUI::ECStandardValueMemberArray ();

    // The instance should be a GCS.
    BaseGCS^    gcs = dynamic_cast<BaseGCS^>(propVal->Instance->ReferenceObject);
    assert (nullptr != gcs);

    BGC::LibraryP   sourceLibrary;
    String^         ourDatumName      = nullptr;
    bool            foundOurDatumName = false;
    if ( (nullptr != gcs) && (NULL != (sourceLibrary = gcs->SourceLibrary)) && sourceLibrary->IsUserLibrary() )
        {
        UInt32 datumCount   = (UInt32) sourceLibrary->GetDatumCount();
        ourDatumName = gcs->DatumName;
        for (UInt32 iDatum=0; iDatum < datumCount; iDatum++)
            {
            WString datumName;
            sourceLibrary->GetDatumName (iDatum, datumName);
            String^ datumNameString = gcnew String (datumName.c_str());
            if (datumNameString->Equals (ourDatumName, System::StringComparison::OrdinalIgnoreCase))
                foundOurDatumName = true;
            memberArray->Add (gcnew ECUI::ECStandardValueMember (datumNameString, 1000000 + iDatum));
            }
        // poor man's separator.
        memberArray->Add (gcnew ECUI::ECStandardValueMember ("_________________", 2000000));
        }

    T_WStringVector*            datumNames            = BGC::BaseGCS::GetDatumNames ();
    T_WStringVector::iterator   datumIterator;
    int                         iDatum;
    for (iDatum=0, datumIterator = datumNames->begin(); datumIterator != datumNames->end(); datumIterator++, iDatum++)
        {
        String^     thisDatumName = gcnew String ((*datumIterator).data());
        if ( (nullptr  != ourDatumName) && thisDatumName->Equals (ourDatumName, System::StringComparison::OrdinalIgnoreCase) )
            foundOurDatumName = true;
        memberArray->Add (gcnew ECUI::ECStandardValueMember (thisDatumName, iDatum));
        }
    delete datumNames;

    // if we have a datum name, but didn't find it in the list, that is presumably because it's from a user library that we don't have. Put the datumName in as the "NotFound" item.
    if ( !foundOurDatumName && (nullptr != ourDatumName) && (ourDatumName->Length > 0))
        {
        assert (!gcs->CanEdit);
        memberArray->Add (gcnew ECUI::ECStandardValueMember (gcnew String (ourDatumName), -1));
        }
    else
        {
        memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("None"), -1));
        }

    return memberArray;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   05/10
+===============+===============+===============+===============+===============+======*/
ref class    DatumTypeTypeConverter : public ECUI::ECSimpleListTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Gets the StandardValueMembers that make up the array</summary>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
protected:
virtual ECUI::ECStandardValueMemberArray^ GetStandardValueMemberArray
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor
) override
    {
    ECUI::ECStandardValueMemberArray^   memberArray = gcnew ECUI::ECStandardValueMemberArray();

    // if it's not a datum, it's a GCS, which is also OK.
    Datum^          datum = dynamic_cast<Datum^>(propVal->Instance->ReferenceObject);

    array<int>^     convertTypeValues  = {  BGC::ConvertType_NONE,       BGC::ConvertType_MOLO,       BGC::ConvertType_MREG,       BGC::ConvertType_BURS,
                                            BGC::ConvertType_NAD27,      BGC::ConvertType_NAD83,      BGC::ConvertType_WGS84,      BGC::ConvertType_WGS72,
                                            BGC::ConvertType_HPGN,       BGC::ConvertType_7PARM,      BGC::ConvertType_AGD66,      BGC::ConvertType_3PARM,
                                            BGC::ConvertType_6PARM,      BGC::ConvertType_4PARM,      BGC::ConvertType_AGD84,      BGC::ConvertType_NZGD4,
                                            BGC::ConvertType_ATS77,      BGC::ConvertType_GDA94,      BGC::ConvertType_NZGD2K,     BGC::ConvertType_CSRS,
                                            BGC::ConvertType_TOKYO,      BGC::ConvertType_RGF93,      BGC::ConvertType_ED50,       BGC::ConvertType_DHDN,
                                            BGC::ConvertType_ETRF89,     BGC::ConvertType_GEOCTR,     BGC::ConvertType_CHENYX, 
#ifdef GEOCOORD_ENHANCEMENT
                                            BGC::ConvertType_GENGRID,
#endif
                                            };

    array<String^>^ convertTypeStrings = {  "NONE",       "MOLO",       "MREG",       "BURS",
                                            "NAD27",      "NAD83",      "WGS84",      "WGS72",
                                            "HPGN",       "7PARM",      "AGD66",      "3PARM",
                                            "6PARM",      "4PARM",      "AGD84",      "NZGD4",
                                            "ATS77",      "GDA94",      "NZGD2K",     "CSRS",
                                            "TOKYO",      "RGF93",      "ED50",       "DHDN",
                                            "ETRF89",     "GEOCTR",     "CHENYX", 
#ifdef GEOCOORD_ENHANCEMENT
                                            "GENGRID",
#endif
                                            };
    array<bool>^    canUseWhenEditing  = {  false,      true,           false,        true, 
                                            false,      false,          false,        false,
                                            false,      true,           false,        true,
                                            true,       true,           false,        false,
                                            false,      false,          false,        false,
                                            false,      false,          false,        false,
                                            false,      true,           false, 
#ifdef GEOCOORD_ENHANCEMENT
                                            false,
#endif
                                            };

    bool    canEdit = (nullptr != datum) && datum->CanEdit;
    for (int iString = 0; iString < convertTypeStrings->Length; iString++)
        {
        if (canEdit && !canUseWhenEditing[iString])
            continue;

        // leave "NONE" blank.
        if (BGC::ConvertType_NONE == convertTypeValues[iString])
            memberArray->Add (gcnew ECUI::ECStandardValueMember ("", BGC::ConvertType_NONE));
        else
            memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString (convertTypeStrings[iString]), convertTypeValues[iString]));
        }
  
    return memberArray;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   09/09
+===============+===============+===============+===============+===============+======*/
ref class    GCSVerticalDatumTypeConverter : public ECUI::ECSimpleListTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Gets the StandardValueMembers that make up the choices presented to the user.</summary>
/// <remarks>This is typically the only method that the derived class overrides.</remarks>
/// <returns>List of ECStandardValueMember objects.</returns>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
protected:
virtual ECUI::ECStandardValueMemberArray^ GetStandardValueMemberArray
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor
) override
    {
    ECUI::ECStandardValueMemberArray^           memberArray = gcnew ECUI::ECStandardValueMemberArray ();
    SCG::IEnumerator<ECI::IECPropertyValue^>^   enumerator  = propertyDescriptor->GetEnumerator();
    ECI::IECPropertyValue^                      propValue;
    ECL::LightweightInstance^                   instance;
    BaseGCS^                                    coordSys;

    enumerator->MoveNext();
    if ( (nullptr != (propValue = enumerator->Current)) && (nullptr != (instance = dynamic_cast <ECL::LightweightInstance^>(propValue->Instance))) &&
         (nullptr != (coordSys  = dynamic_cast<BaseGCS^>(instance->InstanceData))) &&
         (coordSys->IsNAD27 || coordSys->IsNAD83) )
        {
        if (coordSys->IsNAD27)
            {
            memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("NGVD29"), BGC::vdcFromDatum));
            memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("NAVD88"), BGC::vdcNAVD88));
            }
        else
            {
            memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("NAVD88"), BGC::vdcFromDatum));
            memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("NGVD29"), BGC::vdcNGVD29));
            }
        }
    else
        {
        memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("FromDatum"), BGC::vdcFromDatum));
        memberArray->Add (gcnew ECUI::ECStandardValueMember (GeoCoordinateLocalization::GetLocalizedString ("Geoid"), BGC::vdcGeoid));
        }

    return memberArray;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   07/07
+===============+===============+===============+===============+===============+======*/
ref class    GCSEllipsoidCodeTypeConverter : public ECUI::ECSimpleListTypeConverter
{
/*------------------------------------------------------------------------------------**/
/// <summary>Gets the StandardValueMembers that make up the choices presented to the user.</summary>
/// <remarks>This is typically the only method that the derived class overrides.</remarks>
/// <returns>List of ECStandardValueMember objects.</returns>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
protected:
virtual ECUI::ECStandardValueMemberArray^ GetStandardValueMemberArray
(
ECI::IECPropertyValue^                  propVal,
ECUI::ECEnumerablePropertyDescriptor^   propertyDescriptor
) override
    {
    ECUI::ECStandardValueMemberArray^   memberArray = gcnew ECUI::ECStandardValueMemberArray ();

    // The instance can either be a GCS or a Datum. In either case we need the library that it came from.
    BaseGCS^        gcs;
    Datum^          datum;
    BGC::LibraryP   sourceLibrary = NULL;
    String^         ourEllipsoidName      = nullptr;

    if (nullptr != (gcs = dynamic_cast<BaseGCS^>(propVal->Instance->ReferenceObject)))
        {
        sourceLibrary    = gcs->SourceLibrary;
        ourEllipsoidName = gcs->EllipsoidName;
        }
    else if (nullptr != (datum = dynamic_cast<Datum^>(propVal->Instance->ReferenceObject)))
        {
        sourceLibrary    = datum->SourceLibrary;
        ourEllipsoidName = datum->EllipsoidName;
        }

    bool        foundOurEllipsoidName = false;
    if ( (NULL != sourceLibrary) && sourceLibrary->IsUserLibrary() )
        {
        UInt32      ellipsoidCount = (UInt32) sourceLibrary->GetEllipsoidCount();
        for (UInt32 iEllipsoid=0; iEllipsoid < ellipsoidCount; iEllipsoid++)
            {
            WString ellipsoidName;
            sourceLibrary->GetEllipsoidName (iEllipsoid, ellipsoidName);
            String^ ellipsoidNameString = gcnew String (ellipsoidName.c_str());
            if (ellipsoidNameString->Equals (ourEllipsoidName, System::StringComparison::OrdinalIgnoreCase))
                foundOurEllipsoidName = true;
            memberArray->Add (gcnew ECUI::ECStandardValueMember (ellipsoidNameString, 1000000 + iEllipsoid));
            }
        // poor man's separator.
        memberArray->Add (gcnew ECUI::ECStandardValueMember ("_________________", 2000000));
        }

    T_WStringVector*            ellipsoidNames            = BGC::BaseGCS::GetEllipsoidNames ();
    T_WStringVector::iterator   ellipsoidIterator;
    int                         iEllipsoid;
    for (iEllipsoid=0, ellipsoidIterator = ellipsoidNames->begin(); ellipsoidIterator != ellipsoidNames->end(); ellipsoidIterator++, iEllipsoid++)
        {
        String^     thisEllipsoidName = gcnew String ((*ellipsoidIterator).data());
        if ( (nullptr  != ourEllipsoidName) && thisEllipsoidName->Equals (ourEllipsoidName, System::StringComparison::OrdinalIgnoreCase) )
            foundOurEllipsoidName = true;
        memberArray->Add (gcnew ECUI::ECStandardValueMember (thisEllipsoidName, iEllipsoid));
        }
    delete ellipsoidNames;

    // if we have a ellipsoid name, but didn't find it in the list, that is presumably because it's from a user library that we don't have. Put the ellipsoidName in as the "NotFound" item.
    if ( !foundOurEllipsoidName && (nullptr != ourEllipsoidName) && (ourEllipsoidName->Length > 0))
        {
        assert (!gcs->CanEdit);
        memberArray->Add (gcnew ECUI::ECStandardValueMember (gcnew String (ourEllipsoidName), -1));
        }

    return memberArray;
    }
};

/*=================================================================================**//**
*
* This is a managed class that provides the EC Object model for a Coordinate System Group
*
+===============+===============+===============+===============+===============+======*/
public ref class GCSECObjectModel
{
private:
ECS::ECSchema^              m_schema;
ECL::LightweightStringType^ m_stringType;
ECL::LightweightDoubleType^ m_doubleType;
ECL::LightweightInt32Type^  m_int32Type;
ECL::ReadOnlyDelegate^      m_classReadOnlyDelegate;
ECL::LightweightClass^      m_CSBaseClass;
ECL::LightweightClass^      m_projectedCSBaseClass;
ECL::LightweightClass^      m_geographicCSBaseClass;
ECL::LightweightClass^      m_latLongOriginClass;
ECL::LightweightClass^      m_longOriginClass;
ECL::LightweightClass^      m_centMerLatOriginClass;
ECL::LightweightClass^      m_krovakClass;
ECL::LightweightClass^      m_originScaleReductionClass;
ECL::LightweightClass^      m_minnesotaConicClass;
ECL::LightweightClass^      m_wisconsinConicClass;

ECL::LightweightClass^      m_transverseMercatorClass;
ECL::LightweightClass^      m_transverseMercatorWithAffineClass;
ECL::LightweightClass^      m_wisconsinTransMerClass;
ECL::LightweightClass^      m_minnesotaTransMerClass;
ECL::LightweightClass^      m_universalTransMerClass;
ECL::LightweightClass^      m_conicClass;
ECL::LightweightClass^      m_conicClassWithAffine;
ECL::LightweightClass^      m_cylindricalClass;
ECL::LightweightClass^      m_winkelTripelClass;
ECL::LightweightClass^      m_azimuthalClass;
ECL::LightweightClass^      m_azimuthalElevatedEllipsoidClass;
ECL::LightweightClass^      m_danishSystem34Class;
ECL::LightweightClass^      m_centralPointAzimuthClass;
ECL::LightweightClass^      m_minnesotaObliqueMerClass;
ECL::LightweightClass^      m_twoPointOriginLatClass;
ECL::LightweightClass^      m_centralMeridianClass;
ECL::LightweightClass^      m_centralMerStandardParClass;
ECL::LightweightClass^      m_centMerScaleRedClass;
ECL::LightweightClass^      m_modifiedPolyconicClass;
ECL::LightweightClass^      m_obliqueCylindricalClass;
ECL::LightweightClass^      m_originAzimuthClass;

ECL::LightweightClass^      m_datumIndexClass;
ECL::LightweightClass^      m_datumNameClass;
ECL::LightweightClass^      m_baseDatumClass;
ECL::LightweightClass^      m_datumDeltaClass;
ECL::LightweightClass^      m_datumRotationClass;
ECL::LightweightClass^      m_datumScaleClass;
ECL::LightweightClass^      m_ellipsoidIndexClass;
ECL::LightweightClass^      m_ellipsoidNameClass;
ECL::LightweightClass^      m_baseEllipsoidClass;
ECL::LightweightClass^      m_verticalDatumClass;
ECL::LightweightClass^      m_localTransformClass;
ECL::LightweightClass^      m_helmertTransformClass;
ECS::ECClass^               m_csGroupClass;
ECS::ECClass^               m_organizationClass;
ECS::ECClass^               m_groupNodeStruct;
ECS::ECClass^               m_memberNodeStruct;
ECI::IECInstance^           m_csCategory;
ECI::IECInstance^           m_datumCategory;
ECI::IECInstance^           m_ellipsoidCategory;
ECI::IECInstance^           m_nonCSMapCategory;

ECI::IECInstance^           m_angleET;
ECI::IECInstance^           m_parallelET;
ECI::IECInstance^           m_meridianET;
ECI::IECInstance^           m_unitsET;
ECI::IECInstance^           m_datumCodeET;
ECI::IECInstance^           m_ellipsoidCodeET;
ECI::IECInstance^           m_verticalDatumET;
ECI::IECInstance^           m_datumTypeET;
bool                        m_showNonCSMapData;
bool                        m_editNonCSMapData;

enum class Priority
    {
    Name                    = 9100000,
    Description             = 9000000,
    Projection              = 8900000,
    Group                   = 8800000,
    Location                = 8700000,
    Country                 = 8600000,
    Source                  = 8500000,
    Units                   = 8400000,

    OriginLongitude         = 7300000,
    OriginLatitude          = 7290000,

    ScaleReduction          = 6300000,
    FalseEasting            = 6200000,
    FalseNorthing           = 6190000,

    AffineA0                = 5800000,
    AffineB0                = 5790000,
    AffineA1                = 5780000,
    AffineA2                = 5770000,
    AffineB1                = 5760000,
    AffineB2                = 5750000,

    CentralMeridian         = 7300000,
    EasternMeridian         = 7290000,
    UTMZone                 = 6300000,
    Hemisphere              = 6290000,
    Quadrant                = 6000000,

    CentralPointLongitude   = 7600000,
    CentralPointLatitude    = 7590000,

    Point1Longitude         = 7600000,
    Point1Latitude          = 7590000,
    Point2Longitude         = 7580000,
    Point2Latitude          = 7470000,

    DanishSys34Region       = 6300000,

    Azimuth                 = 7200000,

    StandardParallel1       = 7500000,
    StandardParallel2       = 7490000,

    GeoidSeparation         = 7400000,
    ElevationAboveGeoid     = 7390000,

    MinimumLongitude        = 5900000,
    MaximumLongitude        = 5890000,
    MinimumLatitude         = 5880000,
    MaximumLatitude         = 5870000,

    Ellipsoid               = 8900000,
    Conversion              = 8800000,

    EquatorRadius           = 8900000,
    PolarRadius             = 8890000,
    Eccentricity            = 8880000,

    VerticalDatum           = 4000000,
    LocalTransform          = 3900000,

    HelmertTrans_A          = 3800000,
    HelmertTrans_B          = 3700000,
    HelmertTrans_C          = 3600000,
    HelmertTrans_D          = 3500000,
    HelmertTrans_E          = 3400000,

    DatumConvType           = 7200000,
    DatumDeltaX             = 7100100,
    DatumDeltaY             = 7100090,
    DatumDeltaZ             = 7100080,

    DatumRotationX          = 7000100,
    DatumRotationY          = 7000090,
    DatumRotationZ          = 7000080,

    DatumScale              = 6900000,
    };

enum class PropIndex
    {
    Name                    = 0,
    Description             = 1,
    Projection              = 2,
    Source                  = 6,
    UnitCode                = 7,
    OriginLatitude          = 8,
    OriginLongitude         = 9,
    FalseEasting            = 10,
    FalseNorthing           = 11,
    ScaleReduction          = 12,
    MinimumLongitude        = 13,
    MaximumLongitude        = 14,
    MinimumLatitude         = 15,
    MaximumLatitude         = 16,
    CentralMeridian         = 17,
    EasternMeridian         = 18,
    GeoidSeparation         = 19,
    ElevationAboveGeoid     = 20,
    Quadrant                = 21,

    StandardParallel1       = 22,
    StandardParallel2       = 23,
    Azimuth                 = 24,
    CentralPointLongitude   = 25,
    CentralPointLatitude    = 26,
    Point1Longitude         = 27,
    Point1Latitude          = 28,
    Point2Longitude         = 29,
    Point2Latitude          = 30,
    DanishSys34Region       = 31,
    UTMZone                 = 32,
    Hemisphere              = 33,
    LocalTransform          = 34,

    DatumCode               = 40,
    DatumName               = 41,
    DatumDescription        = 42,
    DatumSource             = 43,
    VerticalDatumCode       = 44,
    DatumConvType           = 45,
    DatumDeltaX             = 46,
    DatumDeltaY             = 47,
    DatumDeltaZ             = 48,
    DatumRotationX          = 49,
    DatumRotationY          = 50,
    DatumRotationZ          = 51,
    DatumScale              = 52,

    EllipsoidCode           = 60,
    EllipsoidName           = 61,
    Conversion              = 62,
    EllipsoidDescription    = 63,
    EllipsoidSource         = 64,

    EquatorRadius           = 100,
    PolarRadius             = 101,
    Eccentricity            = 102,

    AffineA0                = 110,
    AffineA1                = 111,
    AffineA2                = 112,
    AffineB0                = 113,
    AffineB1                = 114,
    AffineB2                = 115,

    HelmertTrans_A          = 200,
    HelmertTrans_B          = 201,
    HelmertTrans_C          = 202,
    HelmertTrans_D          = 203,
    HelmertTrans_E          = 204,


    };


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
GCSECObjectModel (bool showNonCSMapData, bool editNonCSMapData)
    {
    m_showNonCSMapData      = showNonCSMapData;
    m_editNonCSMapData      = editNonCSMapData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
GCSECObjectModel ()
    {
    m_showNonCSMapData      = false;
    m_editNonCSMapData      = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static ECL::LightweightProperty^    AddProperty
(
ECL::LightweightClass^          targetClass,
String^                         name,
ECI::IECInstance^               extendedType,
Priority                        priority,
ECI::IECInstance^               category,
PropIndex                       index,
bool                            readOnly,
ECL::IECLightweightType^        lwType
)
    {
    ECL::LightweightProperty^ property = gcnew ECL::LightweightProperty (name, lwType, index);

    if (nullptr != extendedType)
        ECUI::ECPropertyPane::SetExtendedType (property, extendedType);

    // see if we can find a localized display name from the displayLabelKey
    String^  displayLabel;
    if (nullptr != (displayLabel = GeoCoordinateLocalization::GetLocalizedString (name)))
        property->DisplayLabel = displayLabel;

    ECUI::ECPropertyPane::SetCategory (property, category);
    ECUI::ECPropertyPane::SetPriority (property, (int)priority);
    property->IsReadOnly = readOnly;

    targetClass->Add (property);

    return property;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
static double           GetCSDoubleDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    // the propertyData should be a boxed integer.
    int         propertyIndex = safe_cast<int>(propertyData);
    BaseGCS^    coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        {
        Datum^       datum;
        Ellipsoid^   ellipsoid;
        if (nullptr != (datum = dynamic_cast<Datum^>(instanceData)))
            {
            GEOM::DPoint3d    delta;
            GEOM::DPoint3d    rotation;

            switch (propertyIndex)
                {
                case PropIndex::DatumDeltaX:
                    datum->GetDelta (delta);
                    return delta.X;
                case PropIndex::DatumDeltaY:
                    datum->GetDelta (delta);
                    return delta.Y;
                case PropIndex::DatumDeltaZ:
                    datum->GetDelta (delta);
                    return delta.Z;
                case PropIndex::DatumRotationX:
                    datum->GetRotation (rotation);
                    return rotation.X;
                case PropIndex::DatumRotationY:
                    datum->GetRotation (rotation);
                    return rotation.Y;
                case PropIndex::DatumRotationZ:
                    datum->GetRotation (rotation);
                    return rotation.Z;
                case PropIndex::DatumScale:
                    return datum->Scale;
                case PropIndex::EquatorRadius:
                    return datum->EllipsoidEquatorialRadius;
                case PropIndex::PolarRadius:
                    return datum->EllipsoidPolarRadius;
                case PropIndex::Eccentricity:
                    return datum->EllipsoidEccentricity;
                }
            }
        else if (nullptr != (ellipsoid = dynamic_cast <Ellipsoid^> (instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::EquatorRadius:
                    return ellipsoid->EquatorialRadius;
                case PropIndex::PolarRadius:
                    return ellipsoid->PolarRadius;
                case PropIndex::Eccentricity:
                    return ellipsoid->Eccentricity;
                }
            }
        return -1.0;
        }
    else
        {
        GEOM::DPoint3d    delta;
        GEOM::DPoint3d    rotation;
        switch (propertyIndex)
            {
            case PropIndex::OriginLatitude:
                return coordSys->OriginLatitude;
            case PropIndex::OriginLongitude:
                return coordSys->OriginLongitude;
            case PropIndex::FalseEasting:
                return coordSys->FalseEasting;
            case PropIndex::FalseNorthing:
                return coordSys->FalseNorthing;
            case PropIndex::ScaleReduction:
                return coordSys->ScaleReduction;
            case PropIndex::MinimumLongitude:
                return coordSys->MinimumLongitude;
            case PropIndex::MaximumLongitude:
                return coordSys->MaximumLongitude;
            case PropIndex::MinimumLatitude:
                return coordSys->MinimumLatitude;
            case PropIndex::MaximumLatitude:
                return coordSys->MaximumLatitude;
            case PropIndex::CentralMeridian:
                return coordSys->CentralMeridian;
            case PropIndex::EasternMeridian:
                return coordSys->EasternMeridian;
            case PropIndex::CentralPointLongitude:
                return coordSys->CentralPointLongitude;
            case PropIndex::CentralPointLatitude:
                return coordSys->CentralPointLatitude;
            case PropIndex::GeoidSeparation:
                return coordSys->GeoidSeparation;
            case PropIndex::ElevationAboveGeoid:
                return coordSys->ElevationAboveGeoid;
            case PropIndex::StandardParallel1:
                return coordSys->StandardParallel1;
            case PropIndex::StandardParallel2:
                return coordSys->StandardParallel2;
            case PropIndex::Azimuth:
                return coordSys->Azimuth;
            case PropIndex::Point1Longitude:
                return coordSys->Point1Longitude;
            case PropIndex::Point1Latitude:
                return coordSys->Point1Latitude;
            case PropIndex::Point2Longitude:
                return coordSys->Point2Longitude;
            case PropIndex::Point2Latitude:
                return coordSys->Point2Latitude;

            case PropIndex::AffineA0:
                return coordSys->AffineA0;
            case PropIndex::AffineA1:
                return coordSys->AffineA1;
            case PropIndex::AffineA2:
                return coordSys->AffineA2;
            case PropIndex::AffineB0:
                return coordSys->AffineB0;
            case PropIndex::AffineB1:
                return coordSys->AffineB1;
            case PropIndex::AffineB2:
                return coordSys->AffineB2;

            case PropIndex::DatumDeltaX:
                coordSys->GetDatumDelta (delta);
                return delta.X;
            case PropIndex::DatumDeltaY:
                coordSys->GetDatumDelta (delta);
                return delta.Y;
            case PropIndex::DatumDeltaZ:
                coordSys->GetDatumDelta (delta);
                return delta.Z;
            case PropIndex::DatumRotationX:
                coordSys->GetDatumRotation (rotation);
                return rotation.X;
            case PropIndex::DatumRotationY:
                coordSys->GetDatumRotation (rotation);
                return rotation.Y;
            case PropIndex::DatumRotationZ:
                coordSys->GetDatumRotation (rotation);
                return rotation.Z;
            case PropIndex::DatumScale:
                return coordSys->DatumScale;

            case PropIndex::EquatorRadius:
                return coordSys->EllipsoidEquatorialRadius;
            case PropIndex::PolarRadius:
                return coordSys->EllipsoidPolarRadius;
            case PropIndex::Eccentricity:
                return coordSys->EllipsoidEccentricity;

            }
        }
    return -1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
static void             SetCSDoubleDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal,
double                  value
)
    {
    // the propertyData should be a boxed integer.
    int             propertyIndex = safe_cast<int>(propertyData);
    BaseGCS^        coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        {
        Datum^       datum;
        Ellipsoid^   ellipsoid;
        if (nullptr != (datum = dynamic_cast<Datum^>(instanceData)))
            {
            GEOM::DPoint3d    delta;
            GEOM::DPoint3d    rotation;

            switch (propertyIndex)
                {
                case PropIndex::DatumDeltaX:
                    datum->GetDelta (delta);
                    delta.X = value;
                    datum->SetDelta (delta);
                    break;
                case PropIndex::DatumDeltaY:
                    datum->GetDelta (delta);
                    delta.Y = value;
                    datum->SetDelta (delta);
                    break;
                case PropIndex::DatumDeltaZ:
                    datum->GetDelta (delta);
                    delta.Z = value;
                    datum->SetDelta (delta);
                    break;
                case PropIndex::DatumRotationX:
                    datum->GetRotation (rotation);
                    rotation.X = value;
                    datum->SetRotation (rotation);
                    break;
                case PropIndex::DatumRotationY:
                    datum->GetRotation (rotation);
                    rotation.Y = value;
                    datum->SetRotation (rotation);
                    break;
                case PropIndex::DatumRotationZ:
                    datum->GetRotation (rotation);
                    rotation.Z = value;
                    datum->SetRotation (rotation);
                    break;
                case PropIndex::DatumScale:
                    return datum->Scale = value;
                    break;
                }
            }
        else if (nullptr != (ellipsoid = dynamic_cast <Ellipsoid^> (instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::EquatorRadius:
                    ellipsoid->EquatorialRadius = value;
                    break;
                case PropIndex::PolarRadius:
                    ellipsoid->PolarRadius = value;
                    break;
                }
            }
        return;
        }
    else
        {
        switch (propertyIndex)
            {
            case PropIndex::OriginLatitude:
                coordSys->OriginLatitude = value;
                break;
            case PropIndex::OriginLongitude:
                coordSys->OriginLongitude = value;
                break;
            case PropIndex::FalseEasting:
                coordSys->FalseEasting = value;
                break;
            case PropIndex::FalseNorthing:
                coordSys->FalseNorthing = value;
                break;
            case PropIndex::ScaleReduction:
                coordSys->ScaleReduction = value;
                break;
            case PropIndex::MinimumLongitude:
                coordSys->MinimumLongitude = value;
                break;
            case PropIndex::MaximumLongitude:
                coordSys->MaximumLongitude = value;
                break;
            case PropIndex::MinimumLatitude:
                coordSys->MinimumLatitude = value;
                break;
            case PropIndex::MaximumLatitude:
                coordSys->MaximumLatitude = value;
                break;
            case PropIndex::CentralMeridian:
                coordSys->CentralMeridian = value;
                break;
            case PropIndex::EasternMeridian:
                coordSys->EasternMeridian = value;
                break;
            case PropIndex::CentralPointLongitude:
                coordSys->CentralPointLongitude = value;
                break;
            case PropIndex::CentralPointLatitude:
                coordSys->CentralPointLatitude = value;
                break;
            case PropIndex::GeoidSeparation:
                coordSys->GeoidSeparation = value;
                break;
            case PropIndex::ElevationAboveGeoid:
                coordSys->ElevationAboveGeoid = value;
                break;
            case PropIndex::StandardParallel1:
                coordSys->StandardParallel1 = value;
                break;
            case PropIndex::StandardParallel2:
                coordSys->StandardParallel2 = value;
                break;
            case PropIndex::Azimuth:
                coordSys->Azimuth = value;
                break;
            case PropIndex::Point1Longitude:
                coordSys->Point1Longitude = value;
                break;
            case PropIndex::Point1Latitude:
                coordSys->Point1Latitude = value;
                break;
            case PropIndex::Point2Longitude:
                coordSys->Point2Longitude = value;
                break;
            case PropIndex::Point2Latitude:
                coordSys->Point2Latitude = value;
                break;
            case PropIndex::AffineA0:
                coordSys->AffineA0 = value;
                break;
            case PropIndex::AffineA1:
                coordSys->AffineA1 = value;
                break;
            case PropIndex::AffineA2:
                coordSys->AffineA2 = value;
                break;
            case PropIndex::AffineB0:
                coordSys->AffineB0 = value;
                break;
            case PropIndex::AffineB1:
                coordSys->AffineB1 = value;
                break;
            case PropIndex::AffineB2:
                coordSys->AffineB2 = value;
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static String^  GetCSStringDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    // the propertyData should be a boxed integer.
    int         propertyIndex = safe_cast<int>(propertyData);
    // the instanceData should be an BaseGCS or a Datum
    BaseGCS^    coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        {
        Datum^       datum;
        Ellipsoid^   ellipsoid;
        if (nullptr != (datum = dynamic_cast<Datum^>(instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::DatumName:
                    return datum->Name;
                case PropIndex::DatumDescription:
                    return datum->Description;
                case PropIndex::DatumSource:
                    return datum->Source;
                case PropIndex::EllipsoidDescription:
                    return datum->EllipsoidDescription;
                case PropIndex::EllipsoidSource:
                    return datum->EllipsoidSource;
                }
            }
        else if (nullptr != (ellipsoid = dynamic_cast <Ellipsoid^> (instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::EllipsoidName:
                    return ellipsoid->Name;
                case PropIndex::EllipsoidDescription:
                    return ellipsoid->Description;
                case PropIndex::EllipsoidSource:
                    return ellipsoid->Source;
                }
            }
        return "error";
        }
    else
        {
        switch (propertyIndex)
            {
            case PropIndex::Name:
                return coordSys->Name;
            case PropIndex::Description:
                return coordSys->Description;
            case PropIndex::Source:
                return coordSys->Source;
            case PropIndex::DatumDescription:
                return coordSys->DatumDescription;
            case PropIndex::DatumSource:
                return coordSys->DatumSource;
            case PropIndex::EllipsoidDescription:
                return coordSys->EllipsoidDescription;
            case PropIndex::EllipsoidSource:
                return coordSys->EllipsoidSource;
            default:
                return "string";
            }
        }
    return "string";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void     SetCSStringDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal,
String^                 value
)
    {
    // the propertyData should be a boxed integer.
    int         propertyIndex = safe_cast<int>(propertyData);
    // the instanceData should be an BaseGCS or a Datum
    BaseGCS^    coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        {
        Datum^       datum;
        Ellipsoid^   ellipsoid;
        if (nullptr != (datum = dynamic_cast<Datum^>(instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::DatumName:
                    datum->Name = value;
                    break;
                case PropIndex::DatumDescription:
                    datum->Description = value;
                    break;
                case PropIndex::DatumSource:
                    datum->Source = value;
                    break;
                }
            }
        else if (nullptr != (ellipsoid = dynamic_cast <Ellipsoid^> (instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::EllipsoidName:
                    ellipsoid->Name = value;
                    break;
                case PropIndex::EllipsoidDescription:
                    ellipsoid->Description = value;
                    break;
                case PropIndex::EllipsoidSource:
                    ellipsoid->Source = value;
                    break;
                }
            }
        return;
        }
    else
        {
        switch (propertyIndex)
            {
            case PropIndex::Name:
                coordSys->Name = value;
                break;
            case PropIndex::Description:
                coordSys->Description = value;
                break;
            case PropIndex::Source:
                coordSys->Source = value;
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static int      GetCSIntegerDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    // the propertyData should be a boxed integer.
    int         propertyIndex = safe_cast<int>(propertyData);
    BaseGCS^    coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        {
        Datum^    datum;
        if (nullptr != (datum = dynamic_cast<Datum^>(instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::DatumConvType:
                    return (int) datum->ConvertToWGS84Method;
                case PropIndex::EllipsoidCode:
                    return datum->EllipsoidCode;
                }
            }
        }
    else
        {
        switch (propertyIndex)
            {
            case PropIndex::UTMZone:
                return coordSys->UTMZone;
            case PropIndex::Hemisphere:
                return coordSys->Hemisphere;
            case PropIndex::Quadrant:
                return coordSys->Quadrant;
            case PropIndex::DanishSys34Region:
                return coordSys->DanishSys34Region;
            case PropIndex::Projection:
                return (int) coordSys->ProjectionCode;
            case PropIndex::UnitCode:
                return coordSys->UnitCode;
            case PropIndex::EllipsoidCode:
                return coordSys->EllipsoidCode;
            case PropIndex::DatumCode:
                return coordSys->DatumCode;
            case PropIndex::VerticalDatumCode:
                return (int)coordSys->VerticalDatum;
            case PropIndex::LocalTransform:
                return (int)coordSys->LocalTransformType;
            case PropIndex::DatumConvType:
                return (int)coordSys->DatumConvertMethod;
            }
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
static void     SetCSIntegerDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal,
int                     value
)
    {
    int         propertyIndex = safe_cast<int>(propertyData);
    BaseGCS^    coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        {
        Datum^    datum;
        if (nullptr != (datum = dynamic_cast<Datum^>(instanceData)))
            {
            switch (propertyIndex)
                {
                case PropIndex::EllipsoidCode:
                    datum->EllipsoidCode = value;
                    break;
                case PropIndex::DatumConvType:
                    datum->ConvertToWGS84Method = (GeoDatumToWGS84Method) value;
                }
            }
        return;
        }
    else
        {
        // the propertyData should be a boxed integer.
        int     propertyIndex = safe_cast<int>(propertyData);
        switch (propertyIndex)
            {
            case PropIndex::UTMZone:
                coordSys->UTMZone = value;
                break;
            case PropIndex::Hemisphere:
                coordSys->Hemisphere = value;
                break;
            case PropIndex::Quadrant:
                coordSys->Quadrant = value;
                break;
            case PropIndex::DanishSys34Region:
                coordSys->DanishSys34Region = value;
                break;
            case PropIndex::Projection:
                coordSys->ProjectionCode = (BaseGCS::ProjectionCodeValue) value;
                break;
            case PropIndex::UnitCode:
                coordSys->UnitCode = value;
                break;
            case PropIndex::EllipsoidCode:
                coordSys->EllipsoidCode = value;
                break;
            case PropIndex::DatumCode:
                coordSys->DatumCode = value;
                break;
            case PropIndex::VerticalDatumCode:
                coordSys->VerticalDatum = (BaseGCS::VerticalDatumCode) value;
                break;
            case PropIndex::LocalTransform:
                coordSys->LocalTransformType = value;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     CSIsReadOnlyDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    BaseGCS^ coordSys;
    if (nullptr != (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        return !coordSys->CanEdit;

    Datum^   geoDatum;
    if (nullptr != (geoDatum = dynamic_cast<Datum^>(instanceData)))
        return !geoDatum->CanEdit;

    Ellipsoid^   geoEllipsoid;
    if (nullptr != (geoEllipsoid = dynamic_cast<Ellipsoid^>(instanceData)))
        return !geoEllipsoid->CanEdit;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightStringType^        CSStringType
    {
    ECL::LightweightStringType^ get ()
        {
        if (nullptr == m_stringType)
            m_stringType = gcnew ECL::LightweightStringType (gcnew ECL::LightweightStringType::ValueDelegate (&GetCSStringDelegate),
                                                             gcnew ECL::LightweightStringType::SetDelegate (&SetCSStringDelegate));

        return m_stringType;
        }
    }

property ECL::LightweightDoubleType^        CSDoubleType
    {
    ECL::LightweightDoubleType^ get ()
        {
        if (nullptr == m_doubleType)
            m_doubleType = gcnew ECL::LightweightDoubleType (gcnew ECL::LightweightDoubleType::ValueDelegate (&GetCSDoubleDelegate),
                                                             gcnew ECL::LightweightDoubleType::SetDelegate (&SetCSDoubleDelegate));

        return m_doubleType;
        }
    }

property ECL::LightweightInt32Type^        CSInt32Type
    {
    ECL::LightweightInt32Type^ get ()
        {
        if (nullptr == m_int32Type)
            m_int32Type = gcnew ECL::LightweightInt32Type (gcnew ECL::LightweightInt32Type::ValueDelegate (&GetCSIntegerDelegate),
                                                           gcnew ECL::LightweightInt32Type::SetDelegate (&SetCSIntegerDelegate));

        return m_int32Type;
        }
    }

property ECL::LightweightClass^     CSBaseClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_CSBaseClass)
            {
            ECL::LightweightStringType^ stringType = CSStringType;
            ECL::LightweightDoubleType^ doubleType = CSDoubleType;
            ECL::LightweightInt32Type^  int32Type  = CSInt32Type;
            ECS::IECProperty^           projProperty;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CSBaseClass");
                            AddProperty (ecClass, "Name",             nullptr,          Priority::Name,                m_csCategory, PropIndex::Name,              false, stringType);
                            AddProperty (ecClass, "Description",      nullptr,          Priority::Description,         m_csCategory, PropIndex::Description,       false, stringType);
//                          AddProperty (ecClass, "Group",            nullptr,          Priority::Group,               m_csCategory, PropIndex::Group,             true, stringType);
//                          AddProperty (ecClass, "Location",         nullptr,          Priority::Location,            m_csCategory, PropIndex::Location,          true, stringType);
//                          AddProperty (ecClass, "Country",          nullptr,          Priority::Country,             m_csCategory, PropIndex::Country,           true, stringType);
                            AddProperty (ecClass, "Source",           nullptr,          Priority::Source,              m_csCategory, PropIndex::Source,            false, stringType);
                            AddProperty (ecClass, "Units",            m_unitsET,        Priority::Units,               m_csCategory, PropIndex::UnitCode,          false, int32Type);
            projProperty =  AddProperty (ecClass, "Projection",       nullptr,          Priority::Projection,          m_csCategory, PropIndex::Projection,        false, int32Type);
                            AddProperty (ecClass, "MinimumLongitude", m_meridianET,     Priority::MinimumLongitude,    m_csCategory, PropIndex::MinimumLongitude,  false, doubleType);
                            AddProperty (ecClass, "MaximumLongitude", m_meridianET,     Priority::MaximumLongitude,    m_csCategory, PropIndex::MaximumLongitude,  false, doubleType);
                            AddProperty (ecClass, "MinimumLatitude",  m_parallelET,     Priority::MinimumLatitude,     m_csCategory, PropIndex::MinimumLatitude,   false, doubleType);
                            AddProperty (ecClass, "MaximumLatitude",  m_parallelET,     Priority::MaximumLatitude,     m_csCategory, PropIndex::MaximumLatitude,   false, doubleType);

            array<int>^     projectionValues  = { cs_PRJCOD_UNITY,      cs_PRJCOD_TRMER,    cs_PRJCOD_ALBER,    cs_PRJCOD_MRCAT,
                                                  cs_PRJCOD_AZMED,      cs_PRJCOD_LMTAN,    cs_PRJCOD_PLYCN,    cs_PRJCOD_MODPC,    cs_PRJCOD_AZMEA,
                                                  cs_PRJCOD_EDCNC,      cs_PRJCOD_MILLR,    cs_PRJCOD_MSTRO,    cs_PRJCOD_NZLND,    cs_PRJCOD_SINUS,
                                                  cs_PRJCOD_ORTHO,      cs_PRJCOD_GNOMC,    cs_PRJCOD_EDCYL,    cs_PRJCOD_VDGRN,    cs_PRJCOD_CSINI,
                                                  cs_PRJCOD_ROBIN,      cs_PRJCOD_BONNE,    cs_PRJCOD_EKRT4,    cs_PRJCOD_EKRT6,    cs_PRJCOD_MOLWD,
                                                  cs_PRJCOD_HMLSN,      cs_PRJCOD_NACYL,    cs_PRJCOD_TACYL,    cs_PRJCOD_BPCNC,    cs_PRJCOD_SWISS,
                                                  cs_PRJCOD_PSTRO,      cs_PRJCOD_OSTRO,    cs_PRJCOD_SSTRO,    cs_PRJCOD_LM1SP,    cs_PRJCOD_LM2SP,
                                                  cs_PRJCOD_LMBLG,      cs_PRJCOD_WCCSL,    cs_PRJCOD_WCCST,    cs_PRJCOD_MNDOTL,   cs_PRJCOD_MNDOTT,
                                                  cs_PRJCOD_SOTRM,      cs_PRJCOD_UTM,      cs_PRJCOD_TRMRS,    cs_PRJCOD_GAUSSK,   cs_PRJCOD_KROVAK,
                                                  cs_PRJCOD_MRCATK,     cs_PRJCOD_KRVK95,
                                                  cs_PRJCOD_PSTROSL,    cs_PRJCOD_TRMERAF,  cs_PRJCOD_NERTH,    cs_PRJCOD_OBQCYL,
                                                  cs_PRJCOD_SYS34,      cs_PRJCOD_OSTN97,   cs_PRJCOD_AZEDE,    cs_PRJCOD_OSTN02,   cs_PRJCOD_SYS34_99,
                                                  cs_PRJCOD_TRMRKRG,    cs_PRJCOD_WINKL,    cs_PRJCOD_NRTHSRT,  cs_PRJCOD_LMBRTAF,  cs_PRJCOD_HOM1UV,
                                                  cs_PRJCOD_HOM1XY,     cs_PRJCOD_HOM2UV,   cs_PRJCOD_HOM2XY,   cs_PRJCOD_RSKEW,    cs_PRJCOD_RSKEWC,
                                                  cs_PRJCOD_RSKEWO,     cs_PRJCOD_UTMZNBF,  cs_PRJCOD_TRMERBF,  cs_PRJCOD_SYS34_01, cs_PRJCOD_EDCYLE,
                                                  cs_PRJCOD_PCARREE,    cs_PRJCOD_MRCATPV,  cs_PRJCOD_MNDOTOBL};

            array<String^>^ projectionStrings = { "LL",                 "TM",               "AE",               "MRCAT",
                                                  "AZMED",              "LMTAN",            "PLYCN",            "MODPC",            "AZMEA",
                                                  "EDCNC",              "MILLER",           "MSTERO",           "NZEALAND",         "SINUS",
                                                  "ORTHO",              "GNOMONIC",         "EDCYL",            "VDGRNTN",          "CASSINI",
                                                  "ROBINSON",           "BONNE",            "ECKERT4",          "ECKERT6",          "MOLLWEID",
                                                  "GOODE",              "NEACYL",           "TEACYL",           "BIPOLAR",          "SWISS",
                                                  "PSTERO",             "OSTERO",           "OSTEROUS",         "LM1SP",            "LM",
                                                  "LMBLGN",             "LM-WCCS",          "TM-WCCS",          "LM-MNDOT",         "TM-MNDOT",
                                                  "SOTM",               "UTM",              "TM-SNYDER",        "GAUSSK",           "KROVAK",
                                                  "MRCATK",             "KROVAK95",
                                                  "PSTEROSL",           "TMAF",             "NERTH",            "OBQCYL",
                                                  "SYSTM34",            "OSTN97",           "AZMED-ELEV",       "OSTN02",           "SYSTM34-99",
                                                  "TRMRKRG",            "WINKEL",           "NERTH-SRT",        "LMAF",             "HOM1UV",
                                                  "HOM1XY",             "HOM2UV",           "HOM2XY",           "RSKEW",            "RSKEWC",
                                                  "RSKEWO",             "UTMZN-BF",         "TRMER-BF",         "SYSTM34-01",       "EDCYL-E",
                                                  "PCARREE",            "MRCAT-PV",         "OBL-MNDOT" };

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            for (int iString = 0; iString < projectionStrings->Length; iString++)
                {
                WString localizedProjectionName;
                BGC::BaseGeoCoordResource::GetLocalizedProjectionName (localizedProjectionName, (BGC::BaseGCS::ProjectionCodeValue) projectionValues[iString]);
                if (!localizedProjectionName.empty())
                    projectionStrings[iString] = gcnew String (localizedProjectionName.c_str());
                }

            ECUI::ECPropertyPane::SetStandardIntValues (projProperty, true, projectionStrings, projectionValues);
            ECUI::ECPropertyPane::SetRequiresReload (projProperty);

            m_CSBaseClass = ecClass;
            }
        return m_CSBaseClass;
        }
    }

property ECL::LightweightClass^     ProjectedCSBaseClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_projectedCSBaseClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;
            ECL::LightweightInt32Type^  int32Type   = CSInt32Type;
            ECS::IECProperty^           quadProperty;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("ProjectedCSBaseClass", m_CSBaseClass);
                            AddProperty (ecClass, "FalseEasting",     nullptr,    Priority::FalseEasting,        m_csCategory, PropIndex::FalseEasting,      false, doubleType);
                            AddProperty (ecClass, "FalseNorthing",    nullptr,    Priority::FalseNorthing,       m_csCategory, PropIndex::FalseNorthing,     false, doubleType);
            quadProperty =  AddProperty (ecClass, "Quadrant",         nullptr,    Priority::Quadrant,            m_csCategory, PropIndex::Quadrant,          false, int32Type);

            array<int>^     quadrantValues  = { 1, 2, 3, 4, -1, -2, -3, -4 };

            array<String^>^ quadrantStrings = { GeoCoordinateLocalization::GetLocalizedString ("quad1"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad2"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad3"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad4"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad-1"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad-2"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad-3"),
                                                GeoCoordinateLocalization::GetLocalizedString ("quad-4"), };
            ECUI::ECPropertyPane::SetStandardIntValues (quadProperty, true, quadrantStrings, quadrantValues);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_projectedCSBaseClass = ecClass;
            }
        return m_projectedCSBaseClass;
        }
    }

property ECL::LightweightClass^     GeographicCSBaseClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_geographicCSBaseClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("GeographicCSBaseClass", m_CSBaseClass);
            AddProperty (ecClass, "OriginLongitude",  m_meridianET,  Priority::OriginLongitude,     m_csCategory, PropIndex::OriginLongitude,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_geographicCSBaseClass = ecClass;
            }
        return m_geographicCSBaseClass;
        }
    }

property ECL::LightweightClass^     OriginLongitudeClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_longOriginClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("OriginLongitudeClass", ProjectedCSBaseClass);
            AddProperty (ecClass, "OriginLongitude",  m_meridianET,  Priority::OriginLongitude,   m_csCategory, PropIndex::OriginLongitude,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_longOriginClass = ecClass;
            }
        return m_longOriginClass;
        }
    }

property ECL::LightweightClass^     OriginLongitudeLatitudeClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_latLongOriginClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("OriginLongitudeLatitudeClass", OriginLongitudeClass);
            AddProperty (ecClass, "OriginLatitude",   m_parallelET,  Priority::OriginLatitude,  m_csCategory, PropIndex::OriginLatitude,    false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_latLongOriginClass = ecClass;
            }
        return m_latLongOriginClass;
        }
    }

property ECL::LightweightClass^     CentralMeridianOriginLatitudeClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_centMerLatOriginClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CentralMeridianOriginLatitudeClass", ProjectedCSBaseClass);
            AddProperty (ecClass, "CentralMeridian",  m_meridianET,  Priority::CentralMeridian, m_csCategory, PropIndex::CentralMeridian,   false, doubleType);
            AddProperty (ecClass, "OriginLatitude",   m_parallelET,  Priority::OriginLatitude,  m_csCategory, PropIndex::OriginLatitude,    false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_centMerLatOriginClass = ecClass;
            }
        return m_centMerLatOriginClass;
        }
    }

property ECL::LightweightClass^     TransverseMercatorClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_transverseMercatorClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("TransverseMercatorClass", CentralMeridianOriginLatitudeClass);
            AddProperty (ecClass, "ScaleReduction",   nullptr, Priority::ScaleReduction,  m_csCategory, PropIndex::ScaleReduction,    false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_transverseMercatorClass = ecClass;
            }
        return m_transverseMercatorClass;
        }
    }

property ECL::LightweightClass^     WisconsinTransMerClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_wisconsinTransMerClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("WisconsinTransMerClass", TransverseMercatorClass);
            AddProperty (ecClass, "GeoidSeparation",        nullptr, Priority::GeoidSeparation,     m_csCategory, PropIndex::GeoidSeparation,       false, doubleType);
            AddProperty (ecClass, "ElevationAboveGeoid",    nullptr, Priority::ElevationAboveGeoid, m_csCategory, PropIndex::ElevationAboveGeoid,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_wisconsinTransMerClass = ecClass;
            }
        return m_wisconsinTransMerClass;
        }
    }

property ECL::LightweightClass^     MinnesotaTransMerClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_minnesotaTransMerClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("MinnesotaTransMerClass", TransverseMercatorClass);
            AddProperty (ecClass, "ElevationAboveGeoid",    nullptr, Priority::ElevationAboveGeoid, m_csCategory, PropIndex::ElevationAboveGeoid,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_minnesotaTransMerClass = ecClass;
            }
        return m_minnesotaTransMerClass;
        }
    }

property ECL::LightweightClass^     UniversalTransMerClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_universalTransMerClass)
            {
            ECL::LightweightInt32Type^  int32Type   = CSInt32Type;
            ECS::IECProperty^           hemProperty;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("UniversalTransMerClass", m_CSBaseClass);
                          AddProperty (ecClass, "UTMZone",          nullptr, Priority::UTMZone,           m_csCategory, PropIndex::UTMZone,           false, int32Type);
            hemProperty = AddProperty (ecClass, "Hemisphere",       nullptr, Priority::Hemisphere,        m_csCategory, PropIndex::Hemisphere,        false, int32Type);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_universalTransMerClass = ecClass;

            array<int>^     hemisphereValues  = { 1, -1 };
            array<String^>^ hemisphereStrings = { GeoCoordinateLocalization::GetLocalizedString ("Northern"), GeoCoordinateLocalization::GetLocalizedString ("Southern") };
            ECUI::ECPropertyPane::SetStandardIntValues (hemProperty, true, hemisphereStrings, hemisphereValues);
            }
        return m_universalTransMerClass;
        }
    }

property ECL::LightweightClass^     ConicClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_conicClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("ConicClass", OriginLongitudeLatitudeClass);
            AddProperty (ecClass, "StandardParallel1",  m_parallelET, Priority::StandardParallel1,  m_csCategory, PropIndex::StandardParallel1,  false, doubleType);
            AddProperty (ecClass, "StandardParallel2",  m_parallelET, Priority::StandardParallel2,  m_csCategory, PropIndex::StandardParallel2,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_conicClass = ecClass;
            }
        return m_conicClass;
        }
    }

property ECL::LightweightClass^     ConicClassWithAffine
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_conicClassWithAffine)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("ConicClassWithAffine", ConicClass);
            AddProperty (ecClass, "AffineA0",       nullptr,   Priority::AffineA0,              m_csCategory, PropIndex::AffineA0,      false, doubleType);
            AddProperty (ecClass, "AffineA1",       nullptr,   Priority::AffineA1,              m_csCategory, PropIndex::AffineA1,      false, doubleType);
            AddProperty (ecClass, "AffineA2",       nullptr,   Priority::AffineA2,              m_csCategory, PropIndex::AffineA2,      false, doubleType);
            AddProperty (ecClass, "AffineB0",       nullptr,   Priority::AffineB0,              m_csCategory, PropIndex::AffineB0,      false, doubleType);
            AddProperty (ecClass, "AffineB1",       nullptr,   Priority::AffineB1,              m_csCategory, PropIndex::AffineB1,      false, doubleType);
            AddProperty (ecClass, "AffineB2",       nullptr,   Priority::AffineB2,              m_csCategory, PropIndex::AffineB2,      false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_conicClassWithAffine = ecClass;
            }
        return m_conicClassWithAffine;
        }
    }

property ECL::LightweightClass^     CylindricalClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_cylindricalClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CylindricalClass", OriginLongitudeLatitudeClass);
            AddProperty (ecClass, "ReferenceParallel",  m_parallelET, Priority::StandardParallel1,  m_csCategory, PropIndex::StandardParallel1,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_cylindricalClass = ecClass;
            }
        return m_cylindricalClass;
        }
    }

property ECL::LightweightClass^     WinkelTripelClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_winkelTripelClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("WinkelTripelClass", OriginLongitudeClass);
            AddProperty (ecClass, "ReferenceParallel",  m_parallelET, Priority::StandardParallel1,  m_csCategory, PropIndex::StandardParallel1,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_winkelTripelClass = ecClass;
            }
        return m_winkelTripelClass;
        }
    }

property ECL::LightweightClass^     AzimuthalClass      // works for Azimuthal Equal Area AZMEA and Azimuthal Equidistant AZEDE
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_azimuthalClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("AzimuthalClass", OriginLongitudeLatitudeClass);
            AddProperty (ecClass, "Azimuth",  m_angleET, Priority::Azimuth, m_csCategory, PropIndex::Azimuth,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_azimuthalClass = ecClass;
            }
        return m_azimuthalClass;
        }
    }

property ECL::LightweightClass^     AzimuthalElevatedEllipsoidClass      // works for Azimuthal Equal Area AZMEA and Azimuthal Equidistant AZEDE
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_azimuthalElevatedEllipsoidClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("AzimuthalElevatedEllipsoidClass", AzimuthalClass);
            AddProperty (ecClass, "ElevationAboveGeoid",    nullptr, Priority::ElevationAboveGeoid, m_csCategory, PropIndex::ElevationAboveGeoid,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_azimuthalElevatedEllipsoidClass = ecClass;
            }
        return m_azimuthalElevatedEllipsoidClass;
        }
    }

property ECL::LightweightClass^     DanishSystem34Class      // works for Azimuthal Equal Area AZMEA and Azimuthal Equidistant AZEDE
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_danishSystem34Class)
            {
            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("DanishSystem34Class", ProjectedCSBaseClass);
            ECS::IECProperty^       regionProperty;
            regionProperty = AddProperty (ecClass, "Region",    nullptr, Priority::DanishSys34Region, m_csCategory, PropIndex::DanishSys34Region,   false, CSInt32Type);

            array<int>^     regionValues  = { 1, 2, 3 };
            array<String^>^ regionStrings = { GeoCoordinateLocalization::GetLocalizedString ("jylland"),
                                              GeoCoordinateLocalization::GetLocalizedString ("sjaelland"),
                                              GeoCoordinateLocalization::GetLocalizedString ("bornholm") };
            ECUI::ECPropertyPane::SetStandardIntValues (regionProperty, false, regionStrings, regionValues);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_danishSystem34Class = ecClass;
            }
        return m_danishSystem34Class;
        }
    }

property ECL::LightweightClass^     CentralPointAzimuthClass      // works for Azimuthal Equal Area AZMEA and Azimuthal Equidistant AZEDE
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_centralPointAzimuthClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CentralPointAzimuthClass", ProjectedCSBaseClass);
            AddProperty (ecClass, "CentralPointLongitude",  m_meridianET, Priority::CentralPointLongitude,   m_csCategory, PropIndex::CentralPointLongitude, false, doubleType);
            AddProperty (ecClass, "CentralPointLatitude",   m_parallelET, Priority::CentralPointLatitude,    m_csCategory, PropIndex::CentralPointLatitude,  false, doubleType);
            AddProperty (ecClass, "ScaleReduction",         nullptr,      Priority::ScaleReduction,          m_csCategory, PropIndex::ScaleReduction,        false, doubleType);
            AddProperty (ecClass, "Azimuth",                m_angleET,    Priority::Azimuth,                 m_csCategory, PropIndex::Azimuth,               false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_centralPointAzimuthClass = ecClass;
            }
        return m_centralPointAzimuthClass;
        }
    }

property ECL::LightweightClass^     MinnesotaObliqueMerClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_minnesotaObliqueMerClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("MinnesotaObliqueMerClass", CentralPointAzimuthClass);
            AddProperty (ecClass, "ElevationAboveGeoid",    nullptr, Priority::ElevationAboveGeoid, m_csCategory, PropIndex::ElevationAboveGeoid,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_minnesotaObliqueMerClass = ecClass;
            }
        return m_minnesotaObliqueMerClass;
        }
    }

property ECL::LightweightClass^     TwoPointOriginLatClass      // works for Azimuthal Equal Area AZMEA and Azimuthal Equidistant AZEDE
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_twoPointOriginLatClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("TwoPointOriginLatitudeClass", ProjectedCSBaseClass);
            AddProperty (ecClass, "Point1Longitude",    m_meridianET, Priority::Point1Longitude, m_csCategory, PropIndex::Point1Longitude, false, doubleType);
            AddProperty (ecClass, "Pointt1Latitude",    m_parallelET, Priority::Point1Latitude,  m_csCategory, PropIndex::Point1Latitude,  false, doubleType);
            AddProperty (ecClass, "Point2Longitude",    m_meridianET, Priority::Point2Longitude, m_csCategory, PropIndex::Point2Longitude, false, doubleType);
            AddProperty (ecClass, "Pointt2Latitude",    m_parallelET, Priority::Point2Latitude,  m_csCategory, PropIndex::Point2Latitude,  false, doubleType);
            AddProperty (ecClass, "OriginLatitude",     m_parallelET, Priority::OriginLatitude,  m_csCategory, PropIndex::OriginLatitude,  false, doubleType);
            AddProperty (ecClass, "ScaleReduction",     nullptr,      Priority::ScaleReduction,  m_csCategory, PropIndex::ScaleReduction,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_twoPointOriginLatClass = ecClass;
            }
        return m_twoPointOriginLatClass;
        }
    }
property ECL::LightweightClass^     KrovakClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_krovakClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("TwoPointOriginLatitudeClass", OriginLongitudeLatitudeClass);
            AddProperty (ecClass, "PoleLongitude",      m_meridianET, Priority::Point1Longitude,   m_csCategory, PropIndex::Point1Longitude,   false, doubleType);
            AddProperty (ecClass, "PoleLatitude",       m_parallelET, Priority::Point1Latitude,    m_csCategory, PropIndex::Point1Latitude,    false, doubleType);
            AddProperty (ecClass, "StandardParallel1",  m_parallelET, Priority::StandardParallel1, m_csCategory, PropIndex::StandardParallel1, false, doubleType);
            AddProperty (ecClass, "ScaleReduction",     nullptr,      Priority::ScaleReduction,    m_csCategory, PropIndex::ScaleReduction,    false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_krovakClass = ecClass;
            }
        return m_krovakClass;
        }
    }

property ECL::LightweightClass^     OriginScaleReductionClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_originScaleReductionClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("OriginScaleReductionClass", OriginLongitudeLatitudeClass);
            AddProperty (ecClass, "ScaleReduction",     nullptr, Priority::ScaleReduction,  m_csCategory, PropIndex::ScaleReduction,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_originScaleReductionClass = ecClass;
            }
        return m_originScaleReductionClass;
        }
    }

property ECL::LightweightClass^     WisconsinConicClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_wisconsinConicClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("WisconsinConicClass", ConicClass);
            AddProperty (ecClass, "GeoidSeparation",        nullptr, Priority::GeoidSeparation,     m_csCategory, PropIndex::GeoidSeparation,       false, doubleType);
            AddProperty (ecClass, "ElevationAboveGeoid",    nullptr, Priority::ElevationAboveGeoid, m_csCategory, PropIndex::ElevationAboveGeoid,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_wisconsinConicClass = ecClass;
            }
        return m_wisconsinConicClass;
        }
    }

property ECL::LightweightClass^     MinnesotaConicClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_minnesotaConicClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("MinnesotaConicClass", ConicClass);
            AddProperty (ecClass, "ElevationAboveGeoid",    nullptr, Priority::ElevationAboveGeoid, m_csCategory, PropIndex::ElevationAboveGeoid,   false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_minnesotaConicClass = ecClass;
            }
        return m_minnesotaConicClass;
        }
    }
property ECL::LightweightClass^     CentralMeridianClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_centralMeridianClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CentralMeridianClass", ProjectedCSBaseClass);
            AddProperty (ecClass, "CentralMeridian",    m_meridianET, Priority::CentralMeridian,    m_csCategory, PropIndex::CentralMeridian,    false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_centralMeridianClass = ecClass;
            }
        return m_centralMeridianClass;
        }
    }
property ECL::LightweightClass^     CentralMeridianStandardParallelClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_centralMerStandardParClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CentralMeridianStandardParallelClass", CentralMeridianClass);
            AddProperty (ecClass, "StandardParallel1",  m_parallelET, Priority::StandardParallel1,  m_csCategory, PropIndex::StandardParallel1,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_centralMerStandardParClass = ecClass;
            }
        return m_centralMerStandardParClass;
        }
    }
property ECL::LightweightClass^     CentMerScaleRedClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_centMerScaleRedClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("CentMerScaleRedClass", CentralMeridianClass);
            AddProperty (ecClass, "ScaleReduction",         nullptr, Priority::ScaleReduction,          m_csCategory, PropIndex::ScaleReduction,        false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_centMerScaleRedClass = ecClass;
            }
        return m_centMerScaleRedClass;
        }
    }
property ECL::LightweightClass^     ModifiedPolyconicClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_modifiedPolyconicClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("ModifiedPolyconicClass", ProjectedCSBaseClass);
            ecClass->AddBaseClass (CentralMeridianClass);
            AddProperty (ecClass, "EasternMeridian",    m_meridianET, Priority::EasternMeridian,    m_csCategory, PropIndex::EasternMeridian,    false, doubleType);
            AddProperty (ecClass, "StandardParallel1",  m_parallelET, Priority::StandardParallel1,  m_csCategory, PropIndex::StandardParallel1,  false, doubleType);
            AddProperty (ecClass, "StandardParallel2",  m_parallelET, Priority::StandardParallel2,  m_csCategory, PropIndex::StandardParallel2,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_modifiedPolyconicClass = ecClass;
            }
        return m_modifiedPolyconicClass;
        }
    }

property ECL::LightweightClass^     ObliqueCylindricalClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_obliqueCylindricalClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("ObliqueCylindricalClass", OriginScaleReductionClass);
            AddProperty (ecClass, "GaussianParallel",  m_parallelET, Priority::StandardParallel1,  m_csCategory, PropIndex::StandardParallel1,  false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_obliqueCylindricalClass = ecClass;
            }
        return m_obliqueCylindricalClass;
        }
    }

property ECL::LightweightClass^     OriginAzimuthClass      // works for Azimuthal Equal Area AZMEA and Azimuthal Equidistant AZEDE
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_originAzimuthClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("OriginAzimuthClass", OriginLongitudeLatitudeClass);
            AddProperty (ecClass, "ScaleReduction",  nullptr,   Priority::ScaleReduction,          m_csCategory, PropIndex::ScaleReduction,        false, doubleType);
            AddProperty (ecClass, "Azimuth",         m_angleET, Priority::Azimuth,                 m_csCategory, PropIndex::Azimuth,               false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_originAzimuthClass = ecClass;
            }
        return m_originAzimuthClass;
        }
    }

property ECL::LightweightClass^     TransverseMercatorWithAffineClass   // used for the transverse mercator transformations that include affine postprocessing - TRMERAF and TRMRKRG
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_transverseMercatorWithAffineClass)
            {
            ECL::LightweightDoubleType^ doubleType  = CSDoubleType;

            ECL::LightweightClass^  ecClass = gcnew ECL::LightweightClass ("TransverseMercatorWithAffineClass", TransverseMercatorClass);
            AddProperty (ecClass, "AffineA0",       nullptr,   Priority::AffineA0,              m_csCategory, PropIndex::AffineA0,      false, doubleType);
            AddProperty (ecClass, "AffineA1",       nullptr,   Priority::AffineA1,              m_csCategory, PropIndex::AffineA1,      false, doubleType);
            AddProperty (ecClass, "AffineA2",       nullptr,   Priority::AffineA2,              m_csCategory, PropIndex::AffineA2,      false, doubleType);
            AddProperty (ecClass, "AffineB0",       nullptr,   Priority::AffineB0,              m_csCategory, PropIndex::AffineB0,      false, doubleType);
            AddProperty (ecClass, "AffineB1",       nullptr,   Priority::AffineB1,              m_csCategory, PropIndex::AffineB1,      false, doubleType);
            AddProperty (ecClass, "AffineB2",       nullptr,   Priority::AffineB2,              m_csCategory, PropIndex::AffineB2,      false, doubleType);

            ecClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;

            m_transverseMercatorWithAffineClass = ecClass;
            }
        return m_transverseMercatorWithAffineClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     DatumIndexClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_datumIndexClass)
            {
            ECL::LightweightInt32Type^  int32Type   = CSInt32Type;
            ECS::IECProperty^           datumProp;

            m_datumIndexClass = gcnew ECL::LightweightClass ("DatumIndex");
            datumProp = AddProperty (m_datumIndexClass, "Name",             m_datumCodeET,   Priority::Name,            m_datumCategory, PropIndex::DatumCode,        false, int32Type);

            ECUI::ECPropertyPane::SetRequiresReload (datumProp);
            }
        m_datumIndexClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
        return m_datumIndexClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     DatumNameClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_datumNameClass)
            {
            m_datumNameClass = gcnew ECL::LightweightClass ("DatumName");
            AddProperty (m_datumNameClass, "Name",             nullptr,     Priority::Name,            m_datumCategory, PropIndex::DatumName,   false, CSStringType);
            }
        m_datumNameClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
        return m_datumNameClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     BaseDatumClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_baseDatumClass)
            {
            ECL::LightweightStringType^ stringType  = CSStringType;
            ECL::LightweightInt32Type^  int32Type   = CSInt32Type;
            ECS::IECProperty^           cTypeProp;

            m_baseDatumClass = gcnew ECL::LightweightClass ("Datum");
            cTypeProp = AddProperty (m_baseDatumClass, "ConvertType",      m_datumTypeET,   Priority::DatumConvType,   m_datumCategory, PropIndex::DatumConvType,    false,  int32Type);
                        AddProperty (m_baseDatumClass, "Description",      nullptr,         Priority::Description,     m_datumCategory, PropIndex::DatumDescription, false,  stringType);
                        AddProperty (m_baseDatumClass, "Source",           nullptr,         Priority::Source,          m_datumCategory, PropIndex::DatumSource,      false,  stringType);

            ECUI::ECPropertyPane::SetRequiresReload (cTypeProp);

            m_baseDatumClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            }
        return m_baseDatumClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     DatumDeltaClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_datumDeltaClass)
            {
            ECL::LightweightDoubleType^ doubleType = CSDoubleType;

            m_datumDeltaClass = gcnew ECL::LightweightClass ("DatumDelta");
            AddProperty (m_datumDeltaClass, "DeltaX",      nullptr,         Priority::DatumDeltaX,  m_datumCategory, PropIndex::DatumDeltaX,   false,  doubleType);
            AddProperty (m_datumDeltaClass, "DeltaY",      nullptr,         Priority::DatumDeltaY,  m_datumCategory, PropIndex::DatumDeltaY,   false,  doubleType);
            AddProperty (m_datumDeltaClass, "DeltaZ",      nullptr,         Priority::DatumDeltaZ,  m_datumCategory, PropIndex::DatumDeltaZ,   false,  doubleType);

            m_datumDeltaClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            }
        return m_datumDeltaClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     DatumRotationClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_datumRotationClass)
            {
            ECL::LightweightDoubleType^ doubleType = CSDoubleType;

            m_datumRotationClass = gcnew ECL::LightweightClass ("DatumRotation");
            AddProperty (m_datumRotationClass, "RotationX",      nullptr,         Priority::DatumRotationX,  m_datumCategory, PropIndex::DatumRotationX,   false,  doubleType);
            AddProperty (m_datumRotationClass, "RotationY",      nullptr,         Priority::DatumRotationY,  m_datumCategory, PropIndex::DatumRotationY,   false,  doubleType);
            AddProperty (m_datumRotationClass, "RotationZ",      nullptr,         Priority::DatumRotationZ,  m_datumCategory, PropIndex::DatumRotationZ,   false,  doubleType);

            m_datumRotationClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            }
        return m_datumRotationClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     DatumScaleClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_datumScaleClass)
            {
            ECL::LightweightDoubleType^ doubleType = CSDoubleType;

            m_datumScaleClass = gcnew ECL::LightweightClass ("DatumScale");
            AddProperty (m_datumScaleClass, "Scale",      nullptr,         Priority::DatumScale,  m_datumCategory, PropIndex::DatumScale,   false,  doubleType);

            m_datumScaleClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            }
        return m_datumScaleClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
static ECL::LWIsNullReturnValue VerticalDatumIsNull
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    BaseGCS^ coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        return ECL::LWIsNullReturnValue::IsNullCantSet;

    return ECL::LWIsNullReturnValue::NotNull;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     VerticalDatumClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_verticalDatumClass)
            {
            ECL::LightweightProperty^   verticalDatumProp;
            ECL::LightweightInt32Type^  int32Type = gcnew ECL::LightweightInt32Type (gcnew ECL::LightweightInt32Type::ValueDelegate (&GetCSIntegerDelegate),
                                                                        gcnew ECL::LightweightInt32Type::SetDelegate (&SetCSIntegerDelegate));
            int32Type->IsNullDelegate = gcnew ECL::IsNullDelegate (&VerticalDatumIsNull);

            m_verticalDatumClass      = gcnew ECL::LightweightClass ("VerticalDatum");

            verticalDatumProp = AddProperty (m_verticalDatumClass, "VerticalDatum", m_verticalDatumET, Priority::VerticalDatum, m_nonCSMapCategory, PropIndex::VerticalDatumCode, !m_editNonCSMapData, CSInt32Type);
            ECUI::ECPropertyPane::SetRequiresReload (verticalDatumProp);
            }
        return m_verticalDatumClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static double   GetHelmertTransformParameterDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    BaseGCS^ coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        return 0.0;

    BGC::BaseGCSP baseGCS = reinterpret_cast <BGC::BaseGCSP> (coordSys->Peer.ToPointer());
    BGC::HelmertLocalTransformer* transformer;
    if (NULL == (transformer = dynamic_cast <BGC::HelmertLocalTransformer*> (baseGCS->GetLocalTransformer())))
        return 0.0;

    // the propertyData should be a boxed integer.
    int     propertyIndex = safe_cast<int>(propertyData);
    switch (propertyIndex)
        {
        case PropIndex::HelmertTrans_A:
            return transformer->GetA();
        case PropIndex::HelmertTrans_B:
            return transformer->GetB();
        case PropIndex::HelmertTrans_C:
            return transformer->GetC();
        case PropIndex::HelmertTrans_D:
            return transformer->GetD();
        case PropIndex::HelmertTrans_E:
            return transformer->GetE();
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void             SetHelmertTransformParameterDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal,
double                  value
)
    {
    BaseGCS^ coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        return;

    BGC::BaseGCSP baseGCS = reinterpret_cast <BGC::BaseGCSP> (coordSys->Peer.ToPointer());
    BGC::HelmertLocalTransformer* transformer;
    if (NULL == (transformer = dynamic_cast <BGC::HelmertLocalTransformer*> (baseGCS->GetLocalTransformer())))
        return;

    // the propertyData should be a boxed integer.
    int     propertyIndex = safe_cast<int>(propertyData);
    switch (propertyIndex)
        {
        case PropIndex::HelmertTrans_A:
            return transformer->SetA (value);
        case PropIndex::HelmertTrans_B:
            return transformer->SetB (value);
        case PropIndex::HelmertTrans_C:
            return transformer->SetC (value);
        case PropIndex::HelmertTrans_D:
            return transformer->SetD (value);
        case PropIndex::HelmertTrans_E:
            return transformer->SetE (value);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     HelmertTransformClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_helmertTransformClass)
            {
            ECL::LightweightDoubleType^ doubleType = gcnew ECL::LightweightDoubleType (gcnew ECL::LightweightDoubleType::ValueDelegate (&GetHelmertTransformParameterDelegate),
                                                                        gcnew ECL::LightweightDoubleType::SetDelegate (&SetHelmertTransformParameterDelegate));
            m_helmertTransformClass      = gcnew ECL::LightweightClass ("HelmertTransform");

            ECS::IECProperty^       prop;
            prop = AddProperty (m_helmertTransformClass, "HelmertA", nullptr, Priority::HelmertTrans_A, m_nonCSMapCategory, PropIndex::HelmertTrans_A, !m_editNonCSMapData, doubleType);
            ECUI::ECPropertyPane::SetFormatString (prop, "{0:f8}");
            prop = AddProperty (m_helmertTransformClass, "HelmertB", nullptr, Priority::HelmertTrans_B, m_nonCSMapCategory, PropIndex::HelmertTrans_B, !m_editNonCSMapData, doubleType);
            ECUI::ECPropertyPane::SetFormatString (prop, "{0:f8}");
            prop = AddProperty (m_helmertTransformClass, "HelmertC", nullptr, Priority::HelmertTrans_C, m_nonCSMapCategory, PropIndex::HelmertTrans_C, !m_editNonCSMapData, doubleType);
            ECUI::ECPropertyPane::SetFormatString (prop, "{0:f6}");
            prop = AddProperty (m_helmertTransformClass, "HelmertD", nullptr, Priority::HelmertTrans_D, m_nonCSMapCategory, PropIndex::HelmertTrans_D, !m_editNonCSMapData, doubleType);
            ECUI::ECPropertyPane::SetFormatString (prop, "{0:f6}");
            prop = AddProperty (m_helmertTransformClass, "HelmertE", nullptr, Priority::HelmertTrans_E, m_nonCSMapCategory, PropIndex::HelmertTrans_E, !m_editNonCSMapData, doubleType);
            ECUI::ECPropertyPane::SetFormatString (prop, "{0:f6}");
            }
        return m_helmertTransformClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     LocalTransformClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_localTransformClass)
            {
            m_localTransformClass      = gcnew ECL::LightweightClass ("LocalTransform");

            ECS::IECProperty^           transProperty;
            transProperty = AddProperty (m_localTransformClass, "Transform", nullptr, Priority::LocalTransform, m_nonCSMapCategory, PropIndex::LocalTransform, !m_editNonCSMapData, CSInt32Type);

            array<int>^     transformValues  = { BGC::TRANSFORM_None, BGC::TRANSFORM_Helmert, /* TRANSFORM_SecondOrderConformal */ };
            array<String^>^ transformStrings = { "Notransform", "Helmert", /* "SecondOrderConformal" */ };
            for (int iString = 0; iString < transformStrings->Length; iString++)
                {
                transformStrings[iString] =  GeoCoordinateLocalization::GetLocalizedString (transformStrings[iString]);
                }
            ECUI::ECPropertyPane::SetStandardIntValues (transProperty, true, transformStrings, transformValues);
            ECUI::ECPropertyPane::SetRequiresReload (transProperty);
            }
        return m_localTransformClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     EllipsoidCanBeChangedReadOnlyDelegate
(
System::Object^         instanceData,
System::Object^         propertyData,
int                     arrayIndex,
ECI::IECPropertyValue^  propVal
)
    {
    BaseGCS^ coordSys;
    if (nullptr == (coordSys = dynamic_cast<BaseGCS^>(instanceData)))
        return false;

    return (-1 != coordSys->DatumCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     EllipsoidIndexClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_ellipsoidIndexClass)
            {
            ECS::IECProperty^             ellipsoidProp;
            ECL::LightweightInt32Type^    intType         = gcnew ECL::LightweightInt32Type  (gcnew ECL::LightweightInt32Type::ValueDelegate (&GetCSIntegerDelegate),
                                                                                              gcnew ECL::LightweightInt32Type::SetDelegate (&SetCSIntegerDelegate));

            intType->IsReadOnlyDelegate = gcnew ECL::ReadOnlyDelegate (&EllipsoidCanBeChangedReadOnlyDelegate);

            m_ellipsoidIndexClass = gcnew ECL::LightweightClass ("EllipsoidIndex");
            ellipsoidProp = AddProperty (m_ellipsoidIndexClass, "Name",             m_ellipsoidCodeET,   Priority::Name,            m_ellipsoidCategory, PropIndex::EllipsoidCode,        false, intType);
            m_ellipsoidIndexClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            ECUI::ECPropertyPane::SetRequiresRefresh (ellipsoidProp);
            }
        return m_ellipsoidIndexClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     EllipsoidNameClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_ellipsoidNameClass)
            {
            m_ellipsoidNameClass = gcnew ECL::LightweightClass ("EllipsoidName");
            AddProperty (m_ellipsoidNameClass, "Name",             nullptr,             Priority::Name,            m_ellipsoidCategory, PropIndex::EllipsoidName,        false, CSStringType);
            m_ellipsoidNameClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            }
        return m_ellipsoidNameClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECL::LightweightClass^     BaseEllipsoidClass
    {
    ECL::LightweightClass^ get ()
        {
        if (nullptr == m_baseEllipsoidClass)
            {
            ECS::IECProperty^             eRadiusProp;
            ECS::IECProperty^             pRadiusProp;
            ECL::LightweightStringType^   stringType      = CSStringType;
            ECL::LightweightDoubleType^   doubleType      = CSDoubleType;

            m_baseEllipsoidClass = gcnew ECL::LightweightClass ("Ellipsoid");
                            AddProperty (m_baseEllipsoidClass, "Description",      nullptr,             Priority::Description,     m_ellipsoidCategory, PropIndex::EllipsoidDescription, false, stringType);
                            AddProperty (m_baseEllipsoidClass, "Source",           nullptr,             Priority::Source,          m_ellipsoidCategory, PropIndex::EllipsoidSource,      false, stringType);
            eRadiusProp =   AddProperty (m_baseEllipsoidClass, "EquatorRadius",    nullptr,             Priority::PolarRadius,     m_ellipsoidCategory, PropIndex::EquatorRadius,        false, doubleType);
            pRadiusProp =   AddProperty (m_baseEllipsoidClass, "PolarRadius",      nullptr,             Priority::PolarRadius,     m_ellipsoidCategory, PropIndex::PolarRadius,          false, doubleType);
                            AddProperty (m_baseEllipsoidClass, "Eccentricity",     nullptr,             Priority::Eccentricity,    m_ellipsoidCategory, PropIndex::Eccentricity,         true,  doubleType);
            m_baseEllipsoidClass->IsReadOnlyDelegate = m_classReadOnlyDelegate;
            ECUI::ECPropertyPane::SetRequiresRefresh (eRadiusProp);
            ECUI::ECPropertyPane::SetRequiresRefresh (pRadiusProp);
            }
        return m_baseEllipsoidClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECS::ECClass^          CSGroupClass
    {
    ECS::ECClass^ get ()
        {
        if (nullptr == m_csGroupClass)
            {
            m_csGroupClass = gcnew ECS::ECClass ("CSGroup");
            }
        return m_csGroupClass;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECS::ECClass^          GroupNodeStruct
    {
    ECS::ECClass^ get ()
        {
        if (nullptr == m_groupNodeStruct)
            {
            m_groupNodeStruct = gcnew ECS::ECClass ("Group", true);
            ECS::IECArrayType^  memberArray  = gcnew ECS::ECArrayType (MemberNodeStruct);
            ECS::IECArrayType^  groupArray   = gcnew ECS::ECArrayType (m_groupNodeStruct);
            m_groupNodeStruct->Add (gcnew ECS::ECProperty ("Name", EC::ECObjects::StringType));
            m_groupNodeStruct->Add (gcnew ECS::ECProperty ("Description", EC::ECObjects::StringType));
            m_groupNodeStruct->Add (gcnew ECS::ECProperty ("Members",  memberArray));
            m_groupNodeStruct->Add (gcnew ECS::ECProperty ("Groups",   groupArray));
            }
        return m_groupNodeStruct;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
property ECS::ECClass^          MemberNodeStruct
    {
    ECS::ECClass^ get ()
        {
        if (nullptr == m_memberNodeStruct)
            {
            m_memberNodeStruct = gcnew ECS::ECClass ("Member", true);
            m_memberNodeStruct->Add (gcnew ECS::ECProperty ("KeyName",   EC::ECObjects::StringType));
            m_memberNodeStruct->Add (gcnew ECS::ECProperty ("SrcLib",   EC::ECObjects::StringType));
            }
        return m_memberNodeStruct;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
property ECS::ECClass^          OrganizationClass
    {
    ECS::ECClass^ get ()
        {
        if (nullptr == m_organizationClass)
            {
            m_organizationClass = gcnew ECS::ECClass ("Organization");
            m_organizationClass->Add (gcnew ECS::ECProperty ("Root", GroupNodeStruct));
            }
        return m_organizationClass;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
property ECS::ECSchema^     Schema
    {
    ECS::ECSchema^ get()
        {
        if (nullptr == m_schema)
            {
            m_schema                = gcnew ECS::ECSchema ("GeoCoordinateSystem", 1, 0, "GeoCoord");
            m_csCategory            = ECUI::ECPropertyPane::CreateCategory ("CoordinateSystem", GeoCoordinateLocalization::GetLocalizedString ("CoordinateSystem"), nullptr, ECUI::ECPropertyPane::CategorySortPriorityHigh, true);
            m_datumCategory         = ECUI::ECPropertyPane::CreateCategory ("Datum", GeoCoordinateLocalization::GetLocalizedString ("Datum"), nullptr, ECUI::ECPropertyPane::CategorySortPriorityMedium+100, true);
            m_ellipsoidCategory     = ECUI::ECPropertyPane::CreateCategory ("Ellipsoid", GeoCoordinateLocalization::GetLocalizedString ("Ellipsoid"), nullptr, ECUI::ECPropertyPane::CategorySortPriorityMedium, true);
            m_nonCSMapCategory      = ECUI::ECPropertyPane::CreateCategory ("NonCSMap", GeoCoordinateLocalization::GetLocalizedString ("NonCSMap"), nullptr, ECUI::ECPropertyPane::CategorySortPriorityMedium-100, true);
            m_angleET               = ECUI::ECPropertyPane::CreateExtendedType ("GCSAngle");
            m_parallelET            = ECUI::ECPropertyPane::CreateExtendedType ("GCSParallel");
            m_meridianET            = ECUI::ECPropertyPane::CreateExtendedType ("GCSMeridian");
            m_unitsET               = ECUI::ECPropertyPane::CreateExtendedType ("GCUnits");
            m_datumCodeET           = ECUI::ECPropertyPane::CreateExtendedType ("GCDatumCode");
            m_ellipsoidCodeET       = ECUI::ECPropertyPane::CreateExtendedType ("GCEllipsoidCode");
            m_classReadOnlyDelegate = gcnew ECL::ReadOnlyDelegate (&CSIsReadOnlyDelegate);
            m_verticalDatumET       = ECUI::ECPropertyPane::CreateExtendedType ("GCVerticalDatumCode");
            m_datumTypeET           = ECUI::ECPropertyPane::CreateExtendedType ("DatumType");

            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_angleET,          GCSAngleTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_parallelET,       GCSLatitudeTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_meridianET,       GCSLongitudeTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_unitsET,          GCSUnitsTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_datumCodeET,      GCSDatumCodeTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_ellipsoidCodeET,  GCSEllipsoidCodeTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_verticalDatumET,  GCSVerticalDatumTypeConverter::typeid);
            ECUI::ECPropertyPane::SetExtendedTypeTypeConverter (m_datumTypeET,      DatumTypeTypeConverter::typeid);

            m_schema->AddClass (CSBaseClass);
            m_schema->AddClass (ProjectedCSBaseClass);
            m_schema->AddClass (GeographicCSBaseClass);
            m_schema->AddClass (OriginLongitudeClass);
            m_schema->AddClass (OriginLongitudeLatitudeClass);

            m_schema->AddClass (TransverseMercatorClass);
            m_schema->AddClass (WisconsinTransMerClass);
            m_schema->AddClass (MinnesotaTransMerClass);
            m_schema->AddClass (UniversalTransMerClass);
            m_schema->AddClass (ConicClass);
            m_schema->AddClass (CylindricalClass);
            m_schema->AddClass (WinkelTripelClass);
            m_schema->AddClass (CentralMeridianOriginLatitudeClass);
            m_schema->AddClass (AzimuthalClass);
            m_schema->AddClass (AzimuthalElevatedEllipsoidClass);
            m_schema->AddClass (DanishSystem34Class);
            m_schema->AddClass (TwoPointOriginLatClass);
            m_schema->AddClass (CentralPointAzimuthClass);
            m_schema->AddClass (MinnesotaObliqueMerClass);
            m_schema->AddClass (KrovakClass);
            m_schema->AddClass (OriginScaleReductionClass);
            m_schema->AddClass (WisconsinConicClass);
            m_schema->AddClass (MinnesotaConicClass);
            m_schema->AddClass (CentralMeridianClass);
            m_schema->AddClass (CentralMeridianStandardParallelClass);
            m_schema->AddClass (CentMerScaleRedClass);
            m_schema->AddClass (ModifiedPolyconicClass);
            m_schema->AddClass (ObliqueCylindricalClass);
            m_schema->AddClass (OriginAzimuthClass);

            m_schema->AddClass (DatumIndexClass);
            m_schema->AddClass (DatumNameClass);
            m_schema->AddClass (BaseDatumClass);
            m_schema->AddClass (DatumDeltaClass);
            m_schema->AddClass (DatumRotationClass);
            m_schema->AddClass (DatumScaleClass);

            m_schema->AddClass (EllipsoidIndexClass);
            m_schema->AddClass (EllipsoidNameClass);
            m_schema->AddClass (BaseEllipsoidClass);
            m_schema->AddClass (CSGroupClass);
            if (m_showNonCSMapData)
                {
                m_schema->AddClass (VerticalDatumClass);
                m_schema->AddClass (LocalTransformClass);
                m_schema->AddClass (HelmertTransformClass);
                }

            // these structs and classes are used to serialize how the coordinate systems are grouped.
            m_schema->AddClass (GroupNodeStruct);
            m_schema->AddClass (MemberNodeStruct);
            m_schema->AddClass (OrganizationClass);
            }

        return m_schema;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::ECInstanceList^   GetCoordinateSystemProperties (BaseGCS^ coordSys)
    {
    ECI::ECInstanceList^    instanceList = gcnew (ECI::ECInstanceList);
    Schema;    // make sure schema is initialized.
    switch (coordSys->ProjectionCode)
        {
        case cs_PRJCOD_UNITY:
            instanceList->Add (GeographicCSBaseClass->CreateInstance (coordSys));
            break;
        case cs_PRJCOD_TRMER:
        case cs_PRJCOD_SOTRM:
        case cs_PRJCOD_TRMRS:
        case cs_PRJCOD_TRMRKRG:
#if defined (TOTAL_SPECIAL)
        /* TOTAL UTM projection, using the Bernard Flaceliere calculation. (Added by BJB 3/2007). */
        case cs_PRJCOD_TRMERBF:
#endif
            instanceList->Add (TransverseMercatorClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_TRMERAF:     // Transvers mercator with affine postprocessing
            instanceList->Add (TransverseMercatorWithAffineClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_WCCST:
            instanceList->Add (WisconsinTransMerClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MNDOTT:
            instanceList->Add (MinnesotaTransMerClass->CreateInstance (coordSys));
            break;

#if defined (TOTAL_SPECIAL)
        /* TOTAL UTM projection, using the Bernard Flaceliere calculation. (Added by BJB 3/2007). */
        case cs_PRJCOD_UTMZNBF:
#endif
        case cs_PRJCOD_UTM:
            instanceList->Add (UniversalTransMerClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_ALBER:
        case cs_PRJCOD_EDCNC:
            instanceList->Add (ConicClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_PLYCN:
        case cs_PRJCOD_GAUSSK:
            instanceList->Add (CentralMeridianOriginLatitudeClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_AZMEA:
        case cs_PRJCOD_AZMED:
            instanceList->Add (AzimuthalClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_AZEDE:
            instanceList->Add (AzimuthalElevatedEllipsoidClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_BONNE:
        case cs_PRJCOD_GNOMC:
        case cs_PRJCOD_SWISS:
        case cs_PRJCOD_ORTHO:
        case cs_PRJCOD_PCARREE:
        case cs_PRJCOD_PSTROSL: // same as cs_PRJCOD_PSTRO, except scale reduction computed rather than specified.
            instanceList->Add (OriginLongitudeLatitudeClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_CSINI:
            instanceList->Add (CentralMeridianOriginLatitudeClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_SYS34:
        case cs_PRJCOD_SYS34_99:
        case cs_PRJCOD_SYS34_01:
            instanceList->Add (DanishSystem34Class->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_EDCYL:
        case cs_PRJCOD_EDCYLE:
            instanceList->Add (CylindricalClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_WINKL:
            instanceList->Add (WinkelTripelClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_EKRT4:
        case cs_PRJCOD_EKRT6:
        case cs_PRJCOD_HMLSN:
        case cs_PRJCOD_ROBIN:
            instanceList->Add (OriginLongitudeClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_OBLQM:   // never appears, it's the "base" for HOM1UV, HOM1XY, HOM2UV, HOM2XY, RSKEW, RSKEWC, and RSKEWO.
            break;

        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            instanceList->Add (CentralPointAzimuthClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MNDOTOBL:
            instanceList->Add (MinnesotaObliqueMerClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
            instanceList->Add (TwoPointOriginLatClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_KROVAK:  // without 1995 correction
        case cs_PRJCOD_KROVK1:  // Krovk1 is obsolete, should not be encountered.
        case cs_PRJCOD_KRVK95:  // with 1995 correction
        case cs_PRJCOD_KRVK951: // Krvk951 is obsolete, should not be encountered.
            instanceList->Add (KrovakClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_LM1SP:
        case cs_PRJCOD_LMTAN:
        case cs_PRJCOD_NZLND:
        case cs_PRJCOD_OSTRO:
        case cs_PRJCOD_PSTRO:
        case cs_PRJCOD_TACYL:
            instanceList->Add (OriginScaleReductionClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_LM2SP:
        case cs_PRJCOD_LMBLG:
            instanceList->Add (ConicClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_LMBRTAF:         // Conic with Affine parameters.
            instanceList->Add (ConicClassWithAffine->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_WCCSL:
            instanceList->Add (WisconsinConicClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MNDOTL:
            instanceList->Add (MinnesotaConicClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MRCAT:
        case cs_PRJCOD_NACYL:
            instanceList->Add (CentralMeridianStandardParallelClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MILLR:
        case cs_PRJCOD_MRCATPV:
            instanceList->Add (CentralMeridianClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MODPC:
            instanceList->Add (ModifiedPolyconicClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MOLWD:   // NEEDSWORK: Support Zones
        case cs_PRJCOD_SINUS:   // NEEDSWORK: Support Zones
        case cs_PRJCOD_VDGRN:
            instanceList->Add (OriginLongitudeClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MSTRO:   // NEEDSWORK:  Support power series coefficients
            instanceList->Add (OriginScaleReductionClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_OSTN97:  // all parameters are hardcoded.
        case cs_PRJCOD_OSTN02:
            instanceList->Add (CSBaseClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_NERTH:
        case cs_PRJCOD_NRTHSRT: // We don't really support this one.
        case cs_PRJCOD_OCCNC:   // It doesn't look like CSMap really supports this.
        case cs_PRJCOD_BPCNC:   // NEEDSWORK (sort of) Only used for North/South America, parameters really are fixed.
            instanceList->Add (ProjectedCSBaseClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_OBQCYL:
            instanceList->Add (ObliqueCylindricalClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_SSTRO:
            instanceList->Add (OriginAzimuthClass->CreateInstance (coordSys));
            break;

        case cs_PRJCOD_MRCATK:
            instanceList->Add (CentMerScaleRedClass->CreateInstance (coordSys));
            break;


        default:
            instanceList->Add (ProjectedCSBaseClass->CreateInstance (coordSys));
        }
    instanceList->Add (DatumIndexClass->CreateInstance (coordSys));
    instanceList->Add (BaseDatumClass->CreateInstance (coordSys));
    bool    deltaValid, rotationValid, scaleValid;
    coordSys->DatumParametersValid (deltaValid, rotationValid, scaleValid);
    if (deltaValid)
        instanceList->Add (DatumDeltaClass->CreateInstance (coordSys));
    if (rotationValid)
        instanceList->Add (DatumRotationClass->CreateInstance (coordSys));
    if (scaleValid)
        instanceList->Add (DatumScaleClass->CreateInstance (coordSys));

    instanceList->Add (EllipsoidIndexClass->CreateInstance (coordSys));
    instanceList->Add (BaseEllipsoidClass->CreateInstance (coordSys));

    if (m_showNonCSMapData)
        {
        instanceList->Add (VerticalDatumClass->CreateInstance (coordSys));
        instanceList->Add (LocalTransformClass->CreateInstance (coordSys));
        if (BGC::TRANSFORM_Helmert == coordSys->LocalTransformType)
            instanceList->Add (HelmertTransformClass->CreateInstance (coordSys));
        }

    for each (ECI::IECInstance^ instance in instanceList)
        instance->ReferenceObject = coordSys;

    return instanceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::ECInstanceList^   GetDatumProperties (Datum^ datum)
    {
    ECI::ECInstanceList^    instanceList = gcnew (ECI::ECInstanceList);
    Schema;     // make sure schema is initialized.

    instanceList->Add (DatumNameClass->CreateInstance (datum));
    instanceList->Add (BaseDatumClass->CreateInstance (datum));

    bool    deltaValid, rotationValid, scaleValid;
    datum->ParametersValid (deltaValid, rotationValid, scaleValid);
    if (deltaValid)
        instanceList->Add (DatumDeltaClass->CreateInstance (datum));
    if (rotationValid)
        instanceList->Add (DatumRotationClass->CreateInstance (datum));
    if (scaleValid)
        instanceList->Add (DatumScaleClass->CreateInstance (datum));

    instanceList->Add (EllipsoidIndexClass->CreateInstance (datum));
    instanceList->Add (BaseEllipsoidClass->CreateInstance (datum));
    for each (ECI::IECInstance^ instance in instanceList)
        instance->ReferenceObject = datum;

    return instanceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECI::ECInstanceList^   GetEllipsoidProperties (Ellipsoid^ ellipsoid)
    {
    ECI::ECInstanceList^    instanceList = gcnew (ECI::ECInstanceList);
    Schema;     // make sure schema is initialized.

    instanceList->Add (EllipsoidNameClass->CreateInstance (ellipsoid));
    instanceList->Add (BaseEllipsoidClass->CreateInstance (ellipsoid));
    for each (ECI::IECInstance^ instance in instanceList)
        instance->ReferenceObject = ellipsoid;

    return instanceList;
    }

};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
ref class    GCSStatusPanel : SWF::Panel
{
private:
array<SWF::Label^>^     m_lines;
SD::Font^               m_font;
SD::Size                m_size;
SD::Color               m_bgColor;
BaseGCS^                m_coordSys;
ECI::ECInstanceList^    m_instanceList;
SWF::Button^            m_okButton;

static const int       LINE_X     = 2;
static const int       LINE_H     = 14;
static const int       LINE_W     = 400;
static const int       LINE_Y     = 0;
static const int       NUMLINES   = 4;

/*------------------------------------------------------------------------------------**/
/// <author>Barry.Bentley</author>                              <date>04/2007</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
SWF::Label^      CreateLine
(
int             yPosition
)
    {
    SWF::Label^ line = gcnew SWF::Label();
    line->Font       = m_font;
    line->Size       = m_size;
    line->Location   = SD::Point (LINE_X, yPosition);
    line->BackColor  = m_bgColor;
    line->Anchor     = SWF::AnchorStyles::Left | SWF::AnchorStyles::Top | SWF::AnchorStyles::Right;
    return line;
    }

/*---------------------------------------------------------------------------------**//**
/// <author>Barry.Bentley</author>                              <date>04/2007</date>
+---------------+---------------+---------------+---------------+---------------+------*/
public:
GCSStatusPanel
(
BaseGCS^        coordSys,
SWF::Button^    okButton
)
    {
    m_coordSys          = coordSys;
    m_instanceList      = nullptr;
    m_okButton          = okButton;

    SuspendLayout();
    Dock                = SWF::DockStyle::Fill;
    Size                = SD::Size (400,50);
    int currentY        = LINE_Y;
    m_size              = SD::Size (LINE_W, LINE_H);
    m_font              = gcnew SD::Font (gcnew String ("Microsoft Sans Serif"), (float)12, (SD::FontStyle)0, SD::GraphicsUnit::Pixel, 0);
    m_bgColor           = SD::Color::FromArgb (245,245,245);
    m_lines             = gcnew array<SWF::Label^>(NUMLINES);
    for (int iLine=0; iLine < NUMLINES; iLine++)
        {
        m_lines[iLine] = CreateLine (currentY);
        currentY += LINE_H;
        }
    Controls->AddRange (m_lines);
    ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetListeners
(
ECI::ECInstanceList^    instanceList
)
    {
    for each (ECI::IECInstance^ instance in instanceList)
        {
        instance->ECPropertyValueChanged += gcnew ECI::ECPropertyValueChangedHandler (this, &GCSStatusPanel::PropertyValueChangedHandler);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveListeners
(
ECI::ECInstanceList^    instanceList
)
    {
    for each (ECI::IECInstance^ instance in instanceList)
        {
        instance->ECPropertyValueChanged -= gcnew ECI::ECPropertyValueChangedHandler (this, &GCSStatusPanel::PropertyValueChangedHandler);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetInstanceList
(
ECI::ECInstanceList^    instanceList
)
    {
    if (nullptr != m_instanceList)
        RemoveListeners (m_instanceList);

    if (nullptr != instanceList)
        SetListeners (instanceList);

    m_instanceList = instanceList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyValueChangedHandler
(
System::Object^                         sender,
ECI::ECPropertyValueChangedEventArgs^   args
)
    {
    // every time any property changes, validate, and set enabled state of OK button.
    array<String^>^ errors;
    bool    isOK = m_coordSys->Validate (errors);

    for (int iString=0; iString < NUMLINES; iString++)
        {
        if (isOK || (iString >= errors->Length))
            m_lines[iString]->Text = String::Empty;
        else
            m_lines[iString]->Text = errors[iString];
        }

    if (nullptr != m_okButton)
        m_okButton->Enabled = isOK;
    }
};

/*=================================================================================**//**
* We subclass ECPropertyPane so we can override ReloadInstanceLists, which is called when
*  an IECProperty with the "RequiresReload" custom attribute is changed. In this case,
*  it's called when we change either the projection or the datum.
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
ref class    GCSPropertyPane : ECUI::ECPropertyPane
{
BaseGCS^            m_coordSys;
GCSECObjectModel^   m_gcsECObjectModel;
GCSStatusPanel^     m_statusPanel;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
GCSPropertyPane
(
BaseGCS^                coordSys,
GCSECObjectModel^       gcsECObjectModel,
String^                 stateFileName
) : ECUI::ECPropertyPane (nullptr, nullptr, nullptr, L"CSProps", false, false, true, stateFileName, 1000)
    {
    m_coordSys          = coordSys;
    m_gcsECObjectModel  = gcsECObjectModel;
    m_statusPanel       = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
public:
virtual void         ReloadInstanceLists () override
    {
    if (m_coordSys == nullptr)
        return;

    ECI::ECInstanceListSet^ instanceListSet = gcnew ECI::ECInstanceListSet();
    ECI::ECInstanceList^    instanceList = m_gcsECObjectModel->GetCoordinateSystemProperties (m_coordSys);
    instanceListSet->Add (instanceList);

    // When this is called, it's because a property is changed that requires us to reload the elements.
    SetInstanceListSet (instanceListSet);

    // make sure our status panel knows of any changes to the instance list.
    if (nullptr != m_statusPanel)
        m_statusPanel->SetInstanceList (instanceList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
property BaseGCS^   CoordSys
    {
    BaseGCS^    get () { return m_coordSys; }
    void        set (BaseGCS^ value) { m_coordSys = value; ReloadInstanceLists(); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/07
+---------------+---------------+---------------+---------------+---------------+------*/
property GCSStatusPanel^   StatusPanel
    {
    GCSStatusPanel^     get () {return m_statusPanel;}
    void                set (GCSStatusPanel^ value) {m_statusPanel = value;}
    }
};



/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    PropertiesPanel : SWF::Panel
{
private:
GCSPropertyPane^    m_propertyPane;
GCSStatusPanel^     m_statusPanel;

static const int        TOPPANEL_W  = 400;
static const int        TOPPANEL_H  = 500;
static const int        STATUS_H    = 60;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesPanel
(
BaseGCS^        coordSys,
bool            forEditing,
SWF::Button^    okButton
)
    {
    Init (coordSys, forEditing, true, false, okButton);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesPanel
(
BaseGCS^        coordSys,
bool            editCSMapData,
bool            showNonCSMapData,
bool            editNonCSMapData,
SWF::Button^    okButton
)
    {
    Init (coordSys, editCSMapData, showNonCSMapData, editNonCSMapData, okButton);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    Init
(
BaseGCS^        coordSys,
bool            editCSMapData,
bool            showNonCSMapData,
bool            editNonCSMapData,
SWF::Button^    okButton
)
    {
    GCSECObjectModel^ gcsECObjectModel      = gcnew GCSECObjectModel (showNonCSMapData, editNonCSMapData);
    m_propertyPane                          = gcnew GCSPropertyPane (nullptr, gcsECObjectModel, "GCSProperty");
    m_propertyPane->LayoutPreset            = BUCWG::GroupLayoutPreset::StrictTopDown;

    if (!editCSMapData)
        m_propertyPane->Dock                = SWF::DockStyle::Fill;
    else
        {
        Size                        = SD::Size (TOPPANEL_W, TOPPANEL_H);
        m_propertyPane->Size        = SD::Size (TOPPANEL_W, TOPPANEL_H - STATUS_H);
        m_propertyPane->Location    = SD::Point (0, 0);
        m_propertyPane->Anchor      = SWF::AnchorStyles::Right | SWF::AnchorStyles::Top | SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom;
        m_statusPanel               = gcnew GCSStatusPanel (coordSys, okButton);
        m_statusPanel->Location     = SD::Point (0, TOPPANEL_H - STATUS_H);
        m_statusPanel->Size         = SD::Size (TOPPANEL_W, STATUS_H);
        m_statusPanel->Anchor       = SWF::AnchorStyles::Right | SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom;
        m_propertyPane->StatusPanel = m_statusPanel;
        }
    m_propertyPane->CoordSys        = coordSys;

    SuspendLayout();
    Controls->Add (m_propertyPane);
    if (editCSMapData)
        Controls->Add (m_statusPanel);
    ResumeLayout();
    }

};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    PropertiesFormContents
{
private:

SWF::Button^        m_okButton;
SWF::Button^        m_cancelButton;
SWF::Panel^         m_bottomPanel;
PropertiesPanel^    m_propertiesPanel;

static const int    BTN_Y       = 12;
static const int    PANELHEIGHT = 40;
static const int    PANELWIDTH  = 300;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesFormContents
(
SWF::Form^      form,
BaseGCS^        coordSys,
bool            forEditing
)
    {
    InitializeComponent (form, coordSys, forEditing, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesFormContents
(
SWF::Form^      form,
BaseGCS^        coordSys,
bool            editCSMapData,
bool            showNonCSMapData,
bool            editNonCSMapData
)
    {
    InitializeComponent (form, coordSys, editCSMapData, showNonCSMapData, editNonCSMapData);
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
SWF::Form^      form,
BaseGCS^        coordSys,
bool            editCSMapData,
bool            showNonCSMapData,
bool            editNonCSMapData
)
    {
    m_okButton                  = gcnew SWF::Button();

    m_propertiesPanel           = gcnew PropertiesPanel (coordSys, editCSMapData, showNonCSMapData, editNonCSMapData, m_okButton);
    m_propertiesPanel->Dock     = SWF::DockStyle::Fill;

    m_okButton->DialogResult    = SWF::DialogResult::OK;
    m_okButton->Location        = SD::Point(20, BTN_Y);
    m_okButton->Size            = SD::Size(96, 24);
    m_okButton->TabIndex        = 10;
    m_okButton->Text            = GeoCoordinateLocalization::GetLocalizedString ("Ok");

    if (editCSMapData || editNonCSMapData)
        {
        m_cancelButton          = gcnew SWF::Button();
        m_cancelButton->DialogResult = SWF::DialogResult::Cancel;
        m_cancelButton->Location    = SD::Point(130, BTN_Y);
        m_cancelButton->Size        = SD::Size(96, 24);
        m_cancelButton->TabIndex    = 11;
        m_cancelButton->Text        = GeoCoordinateLocalization::GetLocalizedString ("Cancel");
        }

    m_bottomPanel               = gcnew SWF::Panel();
    m_bottomPanel->Size         = SD::Size(PANELWIDTH, PANELHEIGHT);
    m_bottomPanel->Dock         = SWF::DockStyle::Bottom;

    m_bottomPanel->SuspendLayout();
    m_bottomPanel->Controls->Add (m_okButton);
    if (editCSMapData || editNonCSMapData)
        m_bottomPanel->Controls->Add (m_cancelButton);
    m_bottomPanel->ResumeLayout();

    form->SuspendLayout();
    form->Controls->Add (m_propertiesPanel);
    form->Controls->Add (m_bottomPanel);
    form->Size = SD::Size (420,620);
    form->ResumeLayout();
    }


};


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    GroupPropertiesFormContents
{
private:

SWF::Panel^                 m_topPanel;
SWF::Panel^                 m_bottomPanel;
SWF::Button^                m_okButton;
SWF::Button^                m_cancelButton;
SWF::TextBox^               m_nameField;
SWF::TextBox^               m_descrField;
SWF::ToolTip^               m_toolTip;
SWF::Label^                 m_nameLabel;
SWF::Label^                 m_descrLabel;

static const int        BTN_Y       = 12;
static const int        PANELHEIGHT = 40;
static const int        PANELWIDTH  = 300;

static const int        SPACING_Y   = 2;
static const int        SPACING_X   = 10;
static const int        LABEL_X     = 20;
static const int        LABEL_H     = 20;
static const int        LABEL_W     = 120;
static const int        FIELD_X     = LABEL_X + LABEL_W + SPACING_X;
static const int        FIELD_H     = 20;
static const int        FIELD_W     = 280;

static const int        NAME_Y      = 10;
static const int        DESCR_Y     = NAME_Y + FIELD_H + SPACING_Y;
static const int        RIGHT_PAD   = 10;
static const int        TOPPANEL_W  = FIELD_X + FIELD_W + RIGHT_PAD;
static const int        TOPPANEL_H  = DESCR_Y + FIELD_H + SPACING_Y;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
GroupPropertiesFormContents
(
SWF::Form^                  form
)
    {
    InitializeComponent (form);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextChanged
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    // could use further work to see if name is unique within group.
    bool    notEmpty     = m_nameField->Text->Length > 0;
    bool    unique    = true;
    m_okButton->Enabled = notEmpty && unique;

    String^  toolTip = nullptr;
    if (!notEmpty)
        toolTip = GeoCoordinateLocalization::GetLocalizedString ("NeedGroupName");
    else if (!unique)
        toolTip = GeoCoordinateLocalization::GetLocalizedString ("NeedUniqueGroupName");

    // set it on the panels so it shows up as on the disabled button.
    m_toolTip->SetToolTip (m_topPanel, toolTip);
    m_toolTip->SetToolTip (m_bottomPanel, toolTip);
    m_toolTip->SetToolTip (m_nameLabel, toolTip);
    m_toolTip->SetToolTip (m_nameField, toolTip);
    m_toolTip->SetToolTip (m_descrLabel, toolTip);
    m_toolTip->SetToolTip (m_descrField, toolTip);
    m_toolTip->SetToolTip (m_cancelButton, toolTip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
SWF::Form^                  form
)
    {
    m_toolTip                   = gcnew SWF::ToolTip();
    m_toolTip->AutoPopDelay     = 5000;
    m_toolTip->InitialDelay     = 1000;
    m_toolTip->ReshowDelay      = 500;
    m_toolTip->ShowAlways       = true;

    m_nameLabel                 = gcnew SWF::Label();
    m_nameLabel->Text           = GeoCoordinateLocalization::GetLocalizedString ("GroupName");
    m_nameLabel->TextAlign      = SD::ContentAlignment::MiddleRight;
    m_nameLabel->Size           = SD::Size (LABEL_W, LABEL_H);
    m_nameLabel->Location       = SD::Point (LABEL_X, NAME_Y);
    m_nameLabel->TabIndex       = 1;

    m_descrLabel                = gcnew SWF::Label();
    m_descrLabel->Text          = GeoCoordinateLocalization::GetLocalizedString ("GroupDescr");
    m_descrLabel->TextAlign     = SD::ContentAlignment::MiddleRight;
    m_descrLabel->Size          = SD::Size (LABEL_W, LABEL_H);
    m_descrLabel->Location      = SD::Point (LABEL_X, DESCR_Y);
    m_descrLabel->TabIndex      = 3;

    m_nameField                 = gcnew SWF::TextBox();
    m_nameField->TextChanged    += gcnew System::EventHandler (this, &GroupPropertiesFormContents::TextChanged);
    m_nameField->Size           = SD::Size (FIELD_W, FIELD_H);
    m_nameField->Location       = SD::Point (FIELD_X, NAME_Y);
    m_nameField->TabIndex       = 2;
    m_nameField->Anchor         = (SWF::AnchorStyles::Right | SWF::AnchorStyles::Top | SWF::AnchorStyles::Left);

    m_descrField                = gcnew SWF::TextBox();
    m_descrField->Size          = SD::Size (FIELD_W, FIELD_H);
    m_descrField->Location      = SD::Point (FIELD_X, DESCR_Y);
    m_descrField->TabIndex      = 4;
    m_descrField->Anchor        = (SWF::AnchorStyles::Right | SWF::AnchorStyles::Top | SWF::AnchorStyles::Left);

    m_topPanel                  = gcnew SWF::Panel();
    m_topPanel->Size            = SD::Size (TOPPANEL_W, TOPPANEL_H);
    m_topPanel->Dock            = SWF::DockStyle::Fill;
    m_topPanel->SuspendLayout();
    m_topPanel->Controls->Add (m_nameLabel);
    m_topPanel->Controls->Add (m_nameField);
    m_topPanel->Controls->Add (m_descrLabel);
    m_topPanel->Controls->Add (m_descrField);
    m_topPanel->ResumeLayout();

    m_okButton                  = gcnew SWF::Button();
    m_cancelButton              = gcnew SWF::Button();

    m_okButton->DialogResult    = SWF::DialogResult::OK;
    m_okButton->Location        = SD::Point(20, BTN_Y);
    m_okButton->Size            = SD::Size(96, 24);
    m_okButton->TabIndex        = 10;
    m_okButton->Text            = GeoCoordinateLocalization::GetLocalizedString ("Ok");

    m_cancelButton->DialogResult = SWF::DialogResult::Cancel;
    m_cancelButton->Location    = SD::Point(130, BTN_Y);
    m_cancelButton->Size        = SD::Size(96, 24);
    m_cancelButton->TabIndex    = 11;
    m_cancelButton->Text        = GeoCoordinateLocalization::GetLocalizedString ("Cancel");

    m_bottomPanel               = gcnew SWF::Panel();
    m_bottomPanel->Size         = SD::Size(PANELWIDTH, PANELHEIGHT);
    m_bottomPanel->Dock         = SWF::DockStyle::Bottom;

    m_bottomPanel->SuspendLayout();
    m_bottomPanel->Controls->Add (m_okButton);
    m_bottomPanel->Controls->Add (m_cancelButton);
    m_bottomPanel->ResumeLayout();

    form->AcceptButton          = m_okButton;
    form->CancelButton          = m_cancelButton;

    form->SuspendLayout();
    form->Controls->Add (m_topPanel);
    form->Controls->Add (m_bottomPanel);
    form->Size = SD::Size (400,130);
    form->ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    Name
    {
    String^ get()
        {
        return m_nameField->Text;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    Description
    {
    String^ get()
        {
        return m_descrField->Text;
        }
    }

};

/*=================================================================================**//**
* Serializable info needed to recreate a MemberTreeNode. I couldn't figure out how to
* make MemberTreeNode serializable because TreeNode implement ISerialiable, but the
* syntax to call super->GetObjectData is unknown, and I think they seal GetObjectData also.
* @bsiclass                                                     Barry.Bentley   03/07
+===============+===============+===============+===============+===============+======*/
[System::Serializable]
ref class MemberNodeInfo
{
internal:
String^     m_gcsKeyName;

MemberNodeInfo
(
String^ keyName
)
    {
    m_gcsKeyName = keyName;
    }
};


ref class MemberTreeNode;
ref class GroupTreeNode;
ref class FileTreeNode;
/*=================================================================================**//**
* Subclass of TreeNode used in our Selection Panel
* @bsiclass                                                     Barry.Bentley   07/07
+===============+===============+===============+===============+===============+======*/
ref class SelectionPanelTreeNode : public SWF::TreeNode
{
internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPanelTreeNode
(
SWF::ContextMenu^   contextMenu
)
    {
    ContextMenu = contextMenu;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
SelectionPanelTreeNode
(
String^             name,
SWF::ContextMenu^   contextMenu
) : SWF::TreeNode (name)
    {
    ContextMenu = contextMenu;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property FileTreeNode^   FileNode
    {
    FileTreeNode^ get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool IsGroupNode
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanRename
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromGroup
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromLibrary
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanCreateSubgroupOf
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanPasteFromClipboard
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanCopyToClipboard
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanAddToFavorites
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanEdit
    {
    bool get () { return false; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanEditDatums
    {
    bool get () { return false; }
    }

};

delegate bool   MemberTraverseDelegate (String^ keyname, MemberTreeNode^ memberNode, GroupTreeNode^ groupNode, Object^ userArg);

public delegate bool   GCSFilterDelegate (BaseGCS^ gcs);

/*=================================================================================**//**
* Subclass of TreeNode representing a GeoCoordinate System.
* @bsiclass                                                     Barry.Bentley   03/07
+===============+===============+===============+===============+===============+======*/
ref class MemberTreeNode : public SelectionPanelTreeNode
{
private:
BaseGCS^    m_gcs;

internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
MemberTreeNode
(
BaseGCS^    gcs,
SWF::ContextMenu^           contextMenu
) : SelectionPanelTreeNode (contextMenu)
    {
    m_gcs             = gcs;
    SetName();
    InitializeImageKeys();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
~MemberTreeNode
(
)
    {
    if (nullptr != m_gcs)
        {
        delete m_gcs;
        m_gcs = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetName ()
    {
    if (m_gcs->Description->Length > 0)
        this->Text    = String::Format ("{0} - {1}", m_gcs->Name, m_gcs->Description);
    else
        this->Text    = m_gcs->Name;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    KeyName
    {
    String^ get()
        {
        return (nullptr != m_gcs) ? m_gcs->Name : nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property BaseGCS^    GCS
    {
    BaseGCS^ get()
        {
        return m_gcs;
        }
    void set (BaseGCS^ value)
        {
        if (nullptr != m_gcs)
            delete m_gcs;
        m_gcs = value;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromGroup
    {
    bool get () override;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromLibrary
    {
    bool get () override;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanCopyToClipboard
    {
    bool get () override { return true; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanEdit
    {
    bool get () override;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanAddToFavorites
    {
    bool get () override { return true; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            RefreshGCS ()
    {
    // this is called for all user library GCS's whenever a Datum or Ellipsoid in that library is editted,
    //  since otherwise we might be showing stale data.
    BI::ScopedString gcsName(m_gcs->Name);
    BGC::BaseGCSPtr     newGCS = BGC::BaseGCS::CreateGCS (gcsName.Uni());
    if (newGCS->IsValid())
        m_gcs->Peer = static_cast<System::IntPtr>(newGCS.get());
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeImageKeys
(
);


};


/*=================================================================================**//**
* This is a subclass of TreeNode representing a Coordinate System Group.
* @bsiclass                                                     Barry.Bentley   03/07
+===============+===============+===============+===============+===============+======*/
ref class GroupTreeNode : public SelectionPanelTreeNode
{
public:
String^                 m_name;
bool                    m_internationalizable;
String^                 m_description;
bool                    m_membersExpanded;
ECI::IECStructValue^    m_structValue;
GCSGroup^               m_gcsGroup;
GCSMemberEnumerator^    m_memberEnumerator;
GCSFilterDelegate^      m_gcsFilter;


internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
GroupTreeNode
(
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter
) : SelectionPanelTreeNode (contextMenu)
    {
    Initialize (gcsFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
* Used when creating subgroups from library file.
+---------------+---------------+---------------+---------------+---------------+------*/
GroupTreeNode
(
ECI::IECStructValue^    structValue,
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter,
bool                    internationalizable
) : SelectionPanelTreeNode (contextMenu)
    {
    Initialize (gcsFilter);
    m_internationalizable = internationalizable;
    InitializeStructValue (structValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
* Used when initializing the system coordinate library with no organization file.
+---------------+---------------+---------------+---------------+---------------+------*/
GroupTreeNode
(
GCSGroup^               gcsGroup,
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter
) : SelectionPanelTreeNode (contextMenu)
    {
    Initialize (gcsFilter);
    m_gcsGroup      = gcsGroup;
    m_name          = gcsGroup->Name;
    m_description   = gcsGroup->Description;
    if (gcsGroup->Description->Length > 0)
        Text        = String::Format ("{0} - {1}", gcsGroup->Name, gcsGroup->Description);
    else
        Text        = gcsGroup->Name;

    // add an empty node, which is a placeholder until we expand the node.
    Nodes->Add (gcnew SWF::TreeNode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
* Used when initializing a user library with no organization file.
+---------------+---------------+---------------+---------------+---------------+------*/
GroupTreeNode
(
GCSMemberEnumerator^    memberEnumerator,
String^                 name,
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter
) : SelectionPanelTreeNode (contextMenu)
    {
    Initialize (gcsFilter);
    m_name              = name;
    m_memberEnumerator  = memberEnumerator;
    Text                = m_name;

    // add an empty node, which is a placeholder until we expand the node.
    Nodes->Add (gcnew SWF::TreeNode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
* Used from drag/drop.
+---------------+---------------+---------------+---------------+---------------+------*/
GroupTreeNode
(
GroupTreeNode^          sourceNode,
GCSFilterDelegate^      gcsFilter
) : SelectionPanelTreeNode (sourceNode->ContextMenu)
    {
    Initialize (gcsFilter);

    // A group node inherits from parent's internationalizable setting
    m_internationalizable = sourceNode->m_internationalizable;

    if (nullptr != sourceNode->m_structValue)
        InitializeStructValue (sourceNode->m_structValue);
    else if (nullptr != sourceNode->m_gcsGroup)
        m_gcsGroup = sourceNode->m_gcsGroup;

    // add an empty node, which is a placeholder until we expand the node.
    Nodes->Add (gcnew SWF::TreeNode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
* Used when creating new group.
+---------------+---------------+---------------+---------------+---------------+------*/
GroupTreeNode
(
String^                 name,
String^                 description,
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter
) : SelectionPanelTreeNode (contextMenu)
    {
    Initialize (gcsFilter);
    m_name          = name;
    m_description   = description;

    // Normally this will not be executed as the default for internationalizable is false as set in Intitialize()
    if ((m_internationalizable) && (nullptr != m_name))
        {
        String^ localizedName = GeoCoordinateLocalization::GetLocalizedStringNoSubst (m_name);
        if (nullptr != localizedName)
            m_name = localizedName;
        }

    String^ nodeName;
    if ( (nullptr != m_name) && (nullptr != m_description)  && (description->Length > 0) )
        nodeName = String::Format ("{0} - {1}", m_name, m_description);
    else if (nullptr == m_name)
        nodeName = m_description;
    else
        nodeName = m_name;

    assert (nullptr != nodeName);

    // can't use an unnamed node.
    if (nullptr == nodeName)
        nodeName = "BADNODE";

    Text = nodeName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Initialize
(
GCSFilterDelegate^      gcsFilter
)
    {
    m_name                  = nullptr;
    m_internationalizable   = false;  // Default is non-internationalizable. Will be set only when expanding system tree hierarchy.
    m_description           = nullptr;
    m_membersExpanded       = false;
    m_structValue           = nullptr;
    m_gcsGroup              = nullptr;
    m_gcsFilter             = gcsFilter;
    m_memberEnumerator      = nullptr;
    InitializeImageKeys();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InitializeImageKeys
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeStructValue
(
ECI::IECStructValue^  structValue
)
    {
    String^             name        = structValue["Name"]->IsNull ? nullptr :  structValue["Name"]->StringValue;
    String^             description = structValue["Description"]->IsNull ? nullptr : structValue["Description"]->StringValue;
    String^             nodeName    = nullptr;

    if ((m_internationalizable) && (nullptr != name))
        {
        String^ localizedName = GeoCoordinateLocalization::GetLocalizedStringNoSubst (name);
        if (nullptr != localizedName)
            name = localizedName;
        }

    // Note that for the moment the description string is not internationalized. 
    // Normally the description field is not used for group nodes.

    if ( (nullptr != name) && (nullptr != description) && (description->Length > 0))
        nodeName = String::Format ("{0} - {1}", name, description);
    else if (nullptr == name)
        nodeName = description;
    else
        nodeName = name;

    assert (nullptr != nodeName);

    // can't use an unnamed node.
    if (nullptr == nodeName)
        nodeName = "BADNODE";

    m_structValue       = structValue;
    m_name              = name;
    m_description       = description;
    Text                = nodeName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
~GroupTreeNode
(
)
    {
    if (nullptr != m_gcsGroup)
        {
        m_gcsGroup = nullptr;
        delete m_gcsGroup;
        }

    for each (SWF::TreeNode^ childNode in this->Nodes)
        {
        GroupTreeNode^      groupNode;
        MemberTreeNode^     memberNode;
        if (nullptr != (groupNode = dynamic_cast<GroupTreeNode^>(childNode)))
            delete groupNode;
        else if (nullptr != (memberNode = dynamic_cast<MemberTreeNode^>(childNode)))
            delete memberNode;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool CanModifyFileNodeOrganization
    {
    bool get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool CanModifyFileNodeLibrary
    {
    bool get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool FileNodeIsFavorites
    {
    bool get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool IsGroupNode
    {
    bool get () override { return true; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanDeleteFromGroup
    {
    bool get () override;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanCreateSubgroupOf
    {
    bool get () override { return CanModifyFileNodeOrganization; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool CanPasteFromClipboard
    {
    bool get () override
        {
        if (!CanModifyFileNodeOrganization && !CanModifyFileNodeLibrary)
            return false;

        // the only thing we can paste is a data object.
        MemberNodeInfo^   memberNodeInfo = nullptr;
        SWF::IDataObject^ dataObject;
        if ( (nullptr != (dataObject = SWF::Clipboard::GetDataObject())) && dataObject->GetDataPresent (MemberNodeInfo::typeid) )
            memberNodeInfo = dynamic_cast<MemberNodeInfo^>(dataObject->GetData (MemberNodeInfo::typeid));

        if (nullptr == memberNodeInfo)
            return false;

        if (CanModifyFileNodeOrganization)
            {
            // if it's already in our list of children, don't want to paste it.
            if (ContainsGCS (memberNodeInfo->m_gcsKeyName, false))
                return false;

            // if this is node in a Favorites tree, we can paste it.
            // Otherwise, the GCS has to be from the same library.
            if (FileNodeIsFavorites || FromSameLibrary (memberNodeInfo->m_gcsKeyName))
                return true;
            }
        if (CanModifyFileNodeLibrary)
            {
            // it must come from a different library.
            return true; // !FromSameLibrary (memberNodeInfo->m_gcsKeyName);
            }

        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DoPaste
(
SWF::IDataObject^ dataObject
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            FromSameLibrary
(
String^ gcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    Name
    {
    String^ get() new
        {
        if (nullptr != m_name)
            return m_name;
        else if (nullptr != m_gcsGroup)
            return m_gcsGroup->Name;
        else
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    Description
    {
    String^ get()
        {
        if (nullptr != m_description)
            return m_description;
        else if (nullptr != m_gcsGroup)
            return m_gcsGroup->Description;
        else
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property MemberTreeNode^    FirstMemberNode
    {
    MemberTreeNode^ get()
        {
        // make sure we have the member nodes.
        Populate ();

        for each (SWF::TreeNode^ childNode in this->Nodes)
            {
            MemberTreeNode^ memberNode;
            if (nullptr != (memberNode = dynamic_cast<MemberTreeNode^>(childNode)))
                return memberNode;
            }
        return nullptr;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            InsertGroupNode
(
GroupTreeNode^  groupNode
)
    {
    // put it in front of the first child node
    int     insertIndex = 0;
    for each (SWF::TreeNode^ childNode in this->Nodes)
        {
        if (nullptr != dynamic_cast<MemberTreeNode^>(childNode))
            {
            Nodes->Insert (insertIndex, groupNode);
            return;
            }
        insertIndex++;
        }
    // didn't find a child node, add to end.
    Nodes->Add (groupNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopulateGCSGroupNode
(
)
    {
    if (m_membersExpanded)
        return;

    m_membersExpanded = true;

    SWF::TreeNode^ firstNode;

    // if there is no first node, we've been through here and didn't find any children. (shouldn't happen)
    if (nullptr == (firstNode = this->FirstNode))
        return;

    // if the firstNode is not a MemberTreeNode, it's our placeholder and we want to remove it.
    if (nullptr != dynamic_cast<MemberTreeNode^>(firstNode))
        return;

    // remove the placeholder node and add the real ones.
    this->Nodes->Remove (firstNode);

    SCG::IEnumerator<BaseGCS^>^ memberEnumerator = m_gcsGroup->GetEnumerator();
    while (memberEnumerator->MoveNext())
        {
        BaseGCS^ member      = memberEnumerator->Current;
        if ( (nullptr == m_gcsFilter) || m_gcsFilter (member) )
            {
            MemberTreeNode^          memberNode  = gcnew MemberTreeNode (member, ContextMenu);
            this->Nodes->Add (memberNode);
            }
        else
            {
            delete member;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopulateMemberEnumeratorNode
(
)
    {
    if (m_membersExpanded)
        return;

    m_membersExpanded = true;

    SWF::TreeNode^ firstNode;

    // if there is no first node, we've been through here and didn't find any children. (shouldn't happen)
    if (nullptr == (firstNode = this->FirstNode))
        return;

    // if the firstNode is not a MemberTreeNode, it's our placeholder and we want to remove it.
    if (nullptr != dynamic_cast<MemberTreeNode^>(firstNode))
        return;

    // remove the placeholder node and add the real ones.
    this->Nodes->Remove (firstNode);

    m_memberEnumerator->Reset();
    while (m_memberEnumerator->MoveNext())
        {
        BaseGCS^ member      = m_memberEnumerator->Current;
        if ( (nullptr == m_gcsFilter) || m_gcsFilter (member) )
            {
            MemberTreeNode^          memberNode  = gcnew MemberTreeNode (member, ContextMenu);
            this->Nodes->Add (memberNode);
            }
        else
            {
            delete member;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopulateStructGroupNode
(
)
    {
    if (m_membersExpanded)
        return;

    m_membersExpanded = true;

    SWF::TreeNode^ firstNode;

    // if there is no first node, we've been through here and didn't find any children. (shouldn't happen)
    if (nullptr == (firstNode = this->FirstNode))
        return;

    // if the firstnode has an empty Text, that's our dummy member, remove it.
    if (0 == firstNode->Text->Length)
        this->Nodes->Remove (firstNode);

    // if we have no structValue, it's empty.
    if (nullptr == m_structValue)
        return;

    ECI::IECArrayValue^         memberArrayVal;
    if ( (nullptr != (memberArrayVal = dynamic_cast<ECI::IECArrayValue^> (m_structValue["Members"]))) || memberArrayVal->IsNull)
        {
        for (int iMember=0; iMember < memberArrayVal->Count; iMember++)
            {
            ECI::IECStructValue^    memberStructVal;
            if (nullptr == (memberStructVal = dynamic_cast<ECI::IECStructValue^> (memberArrayVal[iMember])))
                continue;

            String^         keyName;
            if (nullptr == (keyName = memberStructVal["KeyName"]->StringValue))
                continue;

            try
                {
                SWF::TreeNode^  memberNode;
                BaseGCS^ gcs = gcnew BaseGCS (keyName);
                if ( (nullptr == m_gcsFilter) || m_gcsFilter (gcs) )
                    {
                    memberNode = gcnew MemberTreeNode (gcs, ContextMenu);
                    this->Nodes->Add (memberNode);
                    }
                else
                    {
                    delete gcs;
                    }
                }
            catch (GeoCoordinateException::ConstructorFailure^)
                {
                // ignore any that we can't construct.
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Populate
(
)
    {
    if (nullptr != m_gcsGroup)
        PopulateGCSGroupNode ();
    else if (nullptr != m_memberEnumerator)
        PopulateMemberEnumeratorNode ();
    else
        PopulateStructGroupNode ();
    }

ref class ContainsArg
{
internal:
String^ m_searchName;
bool    m_contains;
ContainsArg (String^ searchName)
    {
    m_searchName = searchName;
    m_contains  = false;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ContainsMemberDelegate
(
String^         keyName,
MemberTreeNode^ memberNode,
GroupTreeNode^  groupNode,
Object^         userArg
)
    {
    ContainsArg^    containsArg = dynamic_cast<ContainsArg^>(userArg);
    if (containsArg->m_searchName->Equals (keyName))
        containsArg->m_contains = true;

    return containsArg->m_contains;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ContainsGCS
(
String^     searchName,
bool        checkSubGroups
)
    {
    ContainsArg^    containsArg = gcnew ContainsArg (searchName);
    TraverseMembers (gcnew MemberTraverseDelegate (this, &GroupTreeNode::ContainsMemberDelegate), containsArg, checkSubGroups);
    return containsArg->m_contains;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ContainsGCS
(
BaseGCS^        searchGCS,
bool            checkSubGroups
)
    {
    String^     searchName;
    if (nullptr == (searchName = searchGCS->Name))
        return false;

    return ContainsGCS (searchName, checkSubGroups);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ContainsGroup
(
GroupTreeNode^      searchGroup
)
    {
    // should be already expanded, but make sure
    String^ searchGroupName;
    if (nullptr == (searchGroupName = searchGroup->m_name))
        return false;

    for each (SWF::TreeNode^ childNode in this->Nodes)
        {
        GroupTreeNode^ childGroupNode;
        if ( (nullptr != (childGroupNode = dynamic_cast<GroupTreeNode^>(childNode))) && searchGroupName->Equals(childGroupNode->m_name) )
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ExtractSubGroups
(
)
    {
    // Extract the subgroups from m_structVal, creates member GroupTreeNodes.
    ECI::IECArrayValue^  groupArrayVal;
    if ( (nullptr == m_structValue) || (nullptr == (groupArrayVal = dynamic_cast <ECI::IECArrayValue^>(m_structValue["Groups"]))) )
        return;

    for (int iGroup=0; iGroup < groupArrayVal->Count; iGroup++)
        {
        ECI::IECStructValue^    nodeStructVal;
        if (nullptr == (nodeStructVal = dynamic_cast<ECI::IECStructValue^> (groupArrayVal[iGroup])))
            continue;

        // create a new TreeNode (recursively) and add it to the root.
        GroupTreeNode^ subNode = gcnew GroupTreeNode (nodeStructVal, ContextMenu, m_gcsFilter, m_internationalizable);
        subNode->ExtractSubGroups ();

        this->Nodes->Add (subNode);
        }

    // if we don't have any nodes, add an empty node, which is a placeholder until we expand the node.
    if (0 == Nodes->Count)
        Nodes->Add (gcnew SWF::TreeNode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TraverseMembers
(
MemberTraverseDelegate^  callback,
Object^                  userArg,
bool                     doSubGroups
)
    {
    if (!m_membersExpanded)
        {
        // go through all the child nodes (they'll be GroupTreeNodes) and check them, because we always put them first.
        for each (SWF::TreeNode^ childNode in this->Nodes)
            {
            GroupTreeNode^              subGroupNode;
            if (doSubGroups && (nullptr != (subGroupNode = dynamic_cast<GroupTreeNode^>(childNode))))
                {
                if (subGroupNode->TraverseMembers (callback, userArg, true))
                    return true;
                }
            }
        if (nullptr != m_gcsGroup)
            {
            GCSNameEnumerator ^memberEnumerator = gcnew GCSNameEnumerator (m_gcsGroup);
            while (memberEnumerator->MoveNext())
                {
                if (callback (memberEnumerator->Current, nullptr, this, userArg))
                    return true;
                }
            }
        else if (nullptr != m_memberEnumerator)
            {
            m_memberEnumerator->Reset();
            while (m_memberEnumerator->MoveNext())
                {
                BaseGCS^ member      = m_memberEnumerator->Current;
                if (callback (member->Name, nullptr, this, userArg))
                    return true;
                }
            }
        if (nullptr != m_structValue)
            {
            ECI::IECArrayValue^  memberArrayVal;
            if ( (nullptr != (memberArrayVal = dynamic_cast<ECI::IECArrayValue^> (m_structValue["Members"]))) && !memberArrayVal->IsNull)
                {
                for (int iMember=0; iMember < memberArrayVal->Count; iMember++)
                    {
                    ECI::IECStructValue^    memberStructVal;
                    if (nullptr == (memberStructVal = dynamic_cast<ECI::IECStructValue^> (memberArrayVal[iMember])))
                        continue;

                    String^         keyName;
                    if (nullptr == (keyName = memberStructVal["KeyName"]->StringValue))
                        continue;
                    if (callback (keyName, nullptr, this, userArg))
                        return true;
                    }
                }
            }
        }
    else
        {
        for each (SWF::TreeNode^ childNode in this->Nodes)
            {
            MemberTreeNode^             memberNode;
            GroupTreeNode^              subGroupNode;
            String^                     memberKeyName;

            if ( (nullptr != (memberNode = dynamic_cast<MemberTreeNode^>(childNode))) && (nullptr != (memberKeyName = memberNode->KeyName)) )
                {
                if (callback (memberKeyName, memberNode, this, userArg))
                    return true;
                }
            else if (doSubGroups && (nullptr != (subGroupNode = dynamic_cast<GroupTreeNode^>(childNode))))
                {
                if (subGroupNode->TraverseMembers (callback, userArg, true))
                    return true;
                }
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     AddMembersToGroupStructDelegate
(
String^             memberKeyName,
MemberTreeNode^     memberNode,
GroupTreeNode^      groupNode,
Object^             userObj
)
    {
    ECI::IECStructValue^    groupStruct = dynamic_cast<ECI::IECStructValue^>(userObj);
    ECI::IECArrayValue^     memberArray;

    if (nullptr != (memberArray = dynamic_cast<ECI::IECArrayValue^>(groupStruct["Members"])))
        {
        ECI::IECStructValue^ memberStruct = dynamic_cast<ECI::IECStructValue^>(memberArray->Add());
        memberStruct["KeyName"]->StringValue = memberKeyName;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            CreateGroupStruct
(
ECI::IECStructValue^    groupStruct
)
    {
    ECI::IECArrayValue^  subGroupArray;

    // save name and description
    groupStruct["Name"]->StringValue  = m_name;
    if (nullptr != m_description)
        groupStruct["Description"]->StringValue  = m_description;

    // if the members haven't been expanded, we need to add the GCS members from either the group or the array.
    TraverseMembers (gcnew MemberTraverseDelegate(&GroupTreeNode::AddMembersToGroupStructDelegate), groupStruct, false);

    // go through all the subgroups and put them into the Groups array.
    for each (SWF::TreeNode^ childNode in this->Nodes)
        {
        GroupTreeNode^              groupNode;

        // it's either a member or a group.
        if (nullptr != (groupNode = dynamic_cast<GroupTreeNode^> (childNode)))
            {
            if (nullptr == subGroupArray)
                subGroupArray = dynamic_cast<ECI::IECArrayValue^>(groupStruct["Groups"]);
            if (nullptr != subGroupArray)
                {
                ECI::IECStructValue^ subGroupsStruct = dynamic_cast<ECI::IECStructValue^>(subGroupArray->Add());
                groupNode->CreateGroupStruct (subGroupsStruct);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            RefreshAllGCS ()
    {
    // this is called only for group nodes that represent GCS's in user libraries, and only when Datums or Ellipsoids have been
    //  edited. We need this because otherwise, the parameters that are shown for Datum and Ellipsoid sections of the GCS can be stale.

    for each (SWF::TreeNode^ node in this->Nodes)
        {
        GroupTreeNode^  groupNode;
        MemberTreeNode^ memberNode;
        if (nullptr != (groupNode = dynamic_cast<GroupTreeNode^>(node)))
            groupNode->RefreshAllGCS ();
        else if (nullptr != (memberNode = dynamic_cast <MemberTreeNode^>(node)))
            memberNode->RefreshGCS ();
        }
    }
};

/*=================================================================================**//**
* This is a subclass of TreeNode representing a File containing either favorites or the library.
* @bsiclass                                                     Barry.Bentley   03/07
+===============+===============+===============+===============+===============+======*/
ref class    UncategorizedTreeNode : public SelectionPanelTreeNode
{
BGC::LibraryP           m_library;
GCSFilterDelegate^      m_gcsFilter;

internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
UncategorizedTreeNode
(
BGC::LibraryP           library,
String^                 name,
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter
) : SelectionPanelTreeNode (name, contextMenu)
    {
    m_library           = library;
    m_gcsFilter         = gcsFilter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AddMemberToDictionaryDelegate
(
String^         keyName,
MemberTreeNode^ memberNode,
GroupTreeNode^  groupNode,
Object^         userArg
)
    {
    SCG::Dictionary<String^,String^>^    foundGCS = dynamic_cast<SCG::Dictionary<String^, String^>^>(userArg);
    if (!foundGCS->ContainsKey (keyName))
        foundGCS->Add (keyName, keyName);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            Populate
(
)
    {
    SWF::TreeNode^ firstNode;

    // if there is no first node, we've been through here and didn't find any children.
    if (nullptr == (firstNode = FirstNode))
        return;

    // if the firstNode is not a MemberTreeNode, it's our placeholder and we want to remove it.
    if (nullptr != dynamic_cast<MemberTreeNode^>(firstNode))
        return;

    // remove the placeholder node and add the real ones.
    Nodes->Remove (firstNode);

    // create a dictionary for efficient lookup.
    SCG::Dictionary<String^,String^>^ foundGCS = gcnew SCG::Dictionary<String^,String^>();

    // this nodes Parent is always a FileTreeNode. Go through all children, enumerating all of the coordinate systems we
    // already have in the tree, put them in a map so the search is quick.
    GroupTreeNode^   fileNode;
    if (nullptr == (fileNode = dynamic_cast <GroupTreeNode^>(this->Parent)))
        return;

    fileNode->TraverseMembers (gcnew MemberTraverseDelegate (this, &UncategorizedTreeNode::AddMemberToDictionaryDelegate), foundGCS, true);
    SCG::IEnumerator<String^>^ memberEnumerator = gcnew GCSNameEnumerator (m_library);
    while (memberEnumerator->MoveNext())
        {
        if (!foundGCS->ContainsKey (memberEnumerator->Current))
            {
            try
                {
                BaseGCS^ gcs       = gcnew BaseGCS (memberEnumerator->Current);
                if ( (nullptr == m_gcsFilter) || m_gcsFilter (gcs) )
                    {
                    MemberTreeNode^          memberNode = gcnew MemberTreeNode (gcs, ContextMenu);
                    Nodes->Add (memberNode);
                    }
                else
                    {
                    delete gcs;
                    }
                }
            catch (GeoCoordinateException::ConstructorFailure^)
                {
                // ignore any we can't construct. Examples are the coordinates that use the OSTN97.txt and OSTN02.txt files if those
                //  aren't available. Even though they're in the dictionary, CSMap returns an error when we try to construct them.
                }
            }
        }
    }

};

/*=================================================================================**//**
* This is a subclass of TreeNode representing a File containing either favorites or the library.
* @bsiclass                                                     Barry.Bentley   03/07
+===============+===============+===============+===============+===============+======*/
ref class    FileTreeNode : public GroupTreeNode
{
private:
String^                 m_orgFileName;
bool                    m_organizationChanged;
UncategorizedTreeNode^  m_uncategorizedNode;
BGC::LibraryP           m_library;
bool                    m_canModifyOrganization;
bool                    m_canModifyLibrary;


internal:
/*---------------------------------------------------------------------------------**//**
* Constructor use for library with organization file.
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
FileTreeNode
(
String^                 orgFileName,
BGC::LibraryP           library,
ECS::IECSchema^         schema,
SWF::ContextMenu^       contextMenu,
bool                    canModifyOrganization,
bool                    canModifyUserLibrary,
GCSFilterDelegate^      gcsFilter
) : GroupTreeNode (contextMenu, gcsFilter)
    {
    m_orgFileName           = orgFileName;
    m_canModifyOrganization = canModifyOrganization;
    m_library               = library;

    // Set file tree node to internationalizable if the library is system (not user library) and the organization cannot be modified.
    // Notice that 'Favorites' organisation files do not refer directly to a specific library and thus
    // Favorite files will not have their node names internationalizable.
    if (m_library != nullptr && !m_library->IsUserLibrary() && !m_canModifyOrganization)
        m_internationalizable = true;

    m_organizationChanged   = false;
    m_canModifyLibrary      = (NULL == library) ? false : (!library->IsReadOnly() && canModifyUserLibrary);

    // try to deserialize.
    ECXML::ECInstanceXmlReader^ instanceReader      = gcnew ECXML::ECInstanceXmlReader (schema);
    ECI::IECInstance^           organizationClass;
    SIO::FileStream^            instanceInputFile = nullptr;
    try
        {
        instanceInputFile   = gcnew SIO::FileStream (orgFileName, SIO::FileMode::Open, SIO::FileAccess::Read, SIO::FileShare::Read);
        organizationClass   = instanceReader->Deserialize (instanceInputFile);
        }
    catch (System::Exception^ e)
        {
        // rethrow e
        throw e;
        }
    finally
        {
        if (nullptr != instanceInputFile)
            instanceInputFile->Close();
        }

    // go through the organizationClass and put each node into a separate TreeNode.
    if ( (nullptr == (m_structValue = dynamic_cast<ECI::IECStructValue^> (organizationClass["Root"]))) || m_structValue->IsNull)
        throw gcnew System::Exception ("No root");

    InitializeStructValue (m_structValue);



    ExtractSubGroups ();

    m_uncategorizedNode = nullptr;
    InitializeUncategorizedNode ();
    }

/*---------------------------------------------------------------------------------**//**
* Constructor use for CSMap library (with grouping) with no organization file.
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
FileTreeNode
(
GCSGroupEnumerator^     grpEnumerator,
BGC::LibraryP           library,
SWF::ContextMenu^       contextMenu,
GCSFilterDelegate^      gcsFilter
) : GroupTreeNode (GeoCoordinateLocalization::GetLocalizedString ("Library"), nullptr, contextMenu, gcsFilter)
    {
    m_orgFileName           = "c:\\tmp\\testfile.xml";
    m_library               = library;
    m_canModifyOrganization = false;
    m_canModifyLibrary      = (NULL == library) ? false : !library->IsReadOnly();

    // this is here to bootstrap us in the case where we don't have an organization file. Organized by groups.
    while (grpEnumerator->MoveNext())
        {
        GCSGroup^       group       = grpEnumerator->Current;
        SWF::TreeNode^  groupNode   = gcnew GroupTreeNode (group, contextMenu, m_gcsFilter);
        this->Nodes->Add (groupNode);
        }


    // get it to write the ECInstance XML file.
    m_organizationChanged = true;

    InitializeUncategorizedNode ();
    }

/*---------------------------------------------------------------------------------**//**
* Constructor use for user library with no organization file.
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
FileTreeNode
(
GCSMemberEnumerator^    memberEnumerator,
BGC::LibraryP           library,
SWF::ContextMenu^       contextMenu,
bool                    canModifyLibrary,
GCSFilterDelegate^      gcsFilter
) : GroupTreeNode (memberEnumerator, gcnew String (library->GetGUIName().data()), contextMenu, gcsFilter)
    {
    m_library               = library;
    m_canModifyOrganization = false;
    m_canModifyLibrary      = (NULL == library) ? false : (!library->IsReadOnly() && canModifyLibrary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeUncategorizedNode
(
)
    {
    if (NULL == m_library)
        return;

    m_uncategorizedNode = gcnew UncategorizedTreeNode (m_library, GeoCoordinateLocalization::GetLocalizedString ("Uncategorized"), ContextMenu, m_gcsFilter);
    // add a dummy node so it will be expandable.
    m_uncategorizedNode->Nodes->Add (gcnew SWF::TreeNode (""));
    this->Nodes->Add (m_uncategorizedNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ResetUncategorizedNode
(
)
    {
    if (nullptr != m_uncategorizedNode)
        {
        bool isExpanded = m_uncategorizedNode->IsExpanded;
        m_uncategorizedNode->Remove();
        InitializeUncategorizedNode();
        if (isExpanded)
            m_uncategorizedNode->Expand();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GCSInLibrary
(
String^     csName
)
    {
    if (NULL == m_library)
        return false;

    BI::ScopedString gcsName(csName);
    return (NULL != m_library->GetCS (gcsName.Uni()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CantDragFrom
(
FileTreeNode^   sourceNode
)
    {
    // if this FileTreeNode doesn't represent a library, then it's a favorites, and we can drag from the source node here.
    if (NULL == m_library)
        return false;

    // this is a library FileTreeNode. We can only drag from the sourceNode if it's the same library.
    return (m_library != sourceNode->m_library);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetOrganizationChanged
(
)
    {
    m_organizationChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool   CanModifyOrganization
    {
    bool    get()
        {
        return m_canModifyOrganization;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool   CanModifyLibrary
    {
    bool    get()
        {
        return m_canModifyLibrary;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool   IsLibraryFile
    {
    bool    get()
        {
        return (NULL != m_library);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
property bool   IsFavorites
    {
    bool    get()
        {
        return ( (NULL == m_library) && (nullptr != m_orgFileName) );
        }
    }

property BGC::LibraryP UserLibrary
    {
    BGC::LibraryP get () { return m_library->IsUserLibrary() ? m_library : NULL; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DeleteGCS
(
MemberTreeNode^ memberNode
)
    {
    if ( (NULL == m_library) || m_library->IsReadOnly())
        return ERROR;

    BaseGCS^ gcs;
    if (nullptr == (gcs = memberNode->GCS))
        return ERROR;

    return m_library->DeleteCS ((BGC::BaseGCSP) gcs->Peer.ToPointer());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS^        CreateNewGCS
(
String^     sourceGCSName
)
    {
    if (NULL == m_library)
        return nullptr;

    WString outName;

    BI::ScopedString sourceName(sourceGCSName);
    if (SUCCESS == m_library->CreateNewCS (outName, sourceName.Uni()))
        return gcnew BaseGCS (outName.c_str());

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ReplaceInLibrary
(
MemberTreeNode^     source,
BaseGCS^            replacement
)
    {
    // should never happen
    if (NULL == m_library)
        return -1;

    return m_library->ReplaceCS ((BGC::BaseGCSP) source->GCS->Peer.ToPointer(), (BGC::BaseGCSP) replacement->Peer.ToPointer());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            WriteOrganizationFile
(
ECS::IECClass^  organizationClass
)
    {
    // only write if there's an organization file, and it has been changed somehow.
    if (!m_organizationChanged || (nullptr == m_orgFileName) )
        return;

    try
        {
        ECI::IECInstance^       instance = organizationClass->CreateInstance();
        ECI::IECStructValue^    root     = dynamic_cast<ECI::IECStructValue^>(instance["Root"]);

        // call recursive function that populates the GroupNodeStruct.
        CreateGroupStruct (root);

        SIO::FileStream^                    instanceFile      = gcnew SIO::FileStream (m_orgFileName, SIO::FileMode::Create);
        ECXML::ECInstanceXmlStreamWriter^   instanceWriter    = gcnew ECXML::ECInstanceXmlStreamWriter (instanceFile, System::Text::Encoding::UTF8, true);
        instanceWriter->Unformatted = false;
        instanceWriter->Indent      = 2;
        instanceWriter->Serialize (instance);
        delete instanceWriter;
        }
    catch (System::Exception^)
        {
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual property bool   CanEditDatums
    {
    bool    get() override
        {
        return m_canModifyLibrary;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    InitializeImageKeys
(
) override;

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
FileTreeNode^ SelectionPanelTreeNode::FileNode::get ()
    {
    SWF::TreeNode^ node;
    for (node = this; nullptr != node; node = node->Parent)
        {
        FileTreeNode^   fileNode;
        if (nullptr != (fileNode = dynamic_cast<FileTreeNode^>(node)))
            return fileNode;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GroupTreeNode::CanDeleteFromGroup::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    // can't delete the fileNode from any group.
    if (fileTreeNode == this)
        return false;

    return fileTreeNode->CanModifyOrganization;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GroupTreeNode::CanModifyFileNodeOrganization::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->CanModifyOrganization;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GroupTreeNode::CanModifyFileNodeLibrary::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->CanModifyLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GroupTreeNode::FileNodeIsFavorites::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->IsFavorites;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GroupTreeNode::FromSameLibrary
(
String^     gcsName
)
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->GCSInLibrary(gcsName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GroupTreeNode::DoPaste
(
SWF::IDataObject^ dataObject
)
    {
    MemberNodeInfo^   memberNodeInfo = nullptr;
    if ( (nullptr != dataObject) && dataObject->GetDataPresent (MemberNodeInfo::typeid) )
            memberNodeInfo = dynamic_cast<MemberNodeInfo^>(dataObject->GetData (MemberNodeInfo::typeid));

    if (nullptr == memberNodeInfo)
        return false;

    FileTreeNode^   fileNode;
    if (nullptr == (fileNode = FileNode))
        return false;

    BaseGCS^    gcsToInsert = nullptr;
    // first choice is just to modify the organization
    if (CanModifyFileNodeOrganization)
        {
        if (!ContainsGCS (memberNodeInfo->m_gcsKeyName, false) && (FileNodeIsFavorites || FromSameLibrary (memberNodeInfo->m_gcsKeyName)))
            {
            // make sure the existing members are in the target node before we add the new one - otherwise it ends up at the beginning, not the end.
            Populate();
            gcsToInsert = gcnew BaseGCS (memberNodeInfo->m_gcsKeyName);
            }
        }

    // second choice is to modify the library
    if (CanModifyFileNodeLibrary) //  && !FromSameLibrary (memberNodeInfo->m_gcsKeyName))
        {
        // make sure the existing members are in the target node before we add the new one - otherwise it ends up at the beginning, not the end.
        Populate();
        gcsToInsert = fileNode->CreateNewGCS (memberNodeInfo->m_gcsKeyName);
        }

    if (nullptr != gcsToInsert)
        {
        // copy the group, add it to the targetNode.
        MemberTreeNode^ nodeToInsert = gcnew MemberTreeNode (gcsToInsert, ContextMenu);
        Nodes->Add (nodeToInsert);
        fileNode->SetOrganizationChanged();
        TreeView->SelectedNode = nodeToInsert;
        nodeToInsert->EnsureVisible();
        return true;
        }

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MemberTreeNode::CanDeleteFromGroup::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->CanModifyOrganization;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MemberTreeNode::CanDeleteFromLibrary::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->CanModifyLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MemberTreeNode::CanEdit::get ()
    {
    FileTreeNode^   fileTreeNode;
    if (nullptr == (fileTreeNode = FileNode))
        return false;

    return fileTreeNode->CanModifyLibrary;
    }


/*=================================================================================**//**
* IHostFormSupplier defines an interface that is implemented by users of GCSSelectionPanel
*  to provide their own host forms for dialog boxes that GCSSelectionPanel opens.
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public interface class IHostFormProvider
{
public:
SWF::Form^          GetEditPropertiesForm();
SWF::Form^          GetShowPropertiesForm();
SWF::Form^          GetGroupPropertiesForm();
SWF::DialogResult   ShowModal (SWF::Form^ form);
};

public interface class IHostFormProvider2 : IHostFormProvider
{
SWF::Form^          GetEditDatumsForm();
SWF::Form^          GetEditDatumForm();
SWF::Form^          GetEditEllipsoidsForm();
SWF::Form^          GetEditEllipsoidForm();
};


public interface class ISearchSettingsSaver
{
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
* Called before any of the "Get" methods are called to allow the SearchSettings to be
*  retrieved from permanent storage.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            ReadState ();
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
* Called after all of the "Save" methods are called to allow the SearchSettings to be
*  saved to permanant storage.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            SaveState ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
* Should return an array of 7 Int32, containing one instance each of 0 through 6.
* The columnOrder[0] is the position of the Key Name column.
* The columnOrder[1] is the position of the Description Name column.
* The columnOrder[2] is the position of the Units column.
* The columnOrder[3] is the position of the Projection column.
* The columnOrder[4] is the position of the EPSG column.
* The columnOrder[5] is the position of the Datum column.
* The columnOrder[6] is the position of the Ellipsoid Name column.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual array<Int32>^   GetColumnOrder();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
* Should return an array of 7 Int32, containing the widths of each column.
* The columnOrder[0] is the width of the Key Name column.
* The columnOrder[1] is the width of the Description Name column.
* The columnOrder[2] is the width of the Units column.
* The columnOrder[3] is the width of the Projection column.
* The columnOrder[4] is the width of the EPSG column.
* The columnOrder[5] is the width of the Datum column.
* The columnOrder[6] is the width of the Ellipsoid Name column.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual array<Int32>^   GetColumnWidths();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
* Should save the array passed to it to the internal state.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        SaveColumnOrder (array<Int32>^ columnOrder);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
* Should save the array passed to it to the internal state.
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        SaveColumnWidths (array<Int32>^ columnWidths);
};

public ref class DefaultHostFormProvider : IHostFormProvider2
{
public:
virtual SWF::Form^          GetEditPropertiesForm() {return gcnew SWF::Form();}
virtual SWF::Form^          GetShowPropertiesForm() {return gcnew SWF::Form();}
virtual SWF::Form^          GetGroupPropertiesForm() {return gcnew SWF::Form();}
virtual SWF::Form^          GetEditDatumsForm() {return gcnew SWF::Form();}
virtual SWF::Form^          GetEditDatumForm() {return gcnew SWF::Form();}
virtual SWF::Form^          GetEditEllipsoidsForm() {return gcnew SWF::Form();}
virtual SWF::Form^          GetEditEllipsoidForm() {return gcnew SWF::Form();}
virtual SWF::DialogResult   ShowModal (SWF::Form^ form) { return form->ShowDialog();}
};


ref class LibrarySelectionControl;
ref class SearchSelectionControl;
/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    GCSSelectionPanel : SWF::Panel
{
private:
SWF::Form^                  m_hostForm;
SWF::TabControl^            m_tabControl;
SWF::TabPage^               m_libraryPage;
SWF::TabPage^               m_searchPage;
SWF::Button^                m_searchPageAcceptButton;
SWF::Button^                m_okButton;
SWF::Button^                m_cancelButton;
SWF::Panel^                 m_bottomPanel;
IHostFormProvider^          m_hostFormProvider;

#if defined (MAPSELECT)
SWF::MainMenu^              m_mainMenu;
SWF::TabPage^               m_graphicsSelectPage;
#endif
LibrarySelectionControl^    m_librarySelectionControl;
SearchSelectionControl^     m_searchSelectionControl;
BaseGCS^                    m_selectedGCS;
BaseGCS^                    m_libraryPageSelectedGCS;
BaseGCS^                    m_searchPageSelectedGCS;
String^                     m_userOrganizationName;

public:
static String^        LibraryImageKey     = "Library";
static String^        FavoritesImageKey   = "Favorites";
static String^        GroupImageKey       = "Group";
static String^        GCSImageKey         = "GCS";


private:
static const int    BTN_Y           = 12;
static const int    PANELHEIGHT     = 40;
static const int    PANELWIDTH      = 300;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
GCSSelectionPanel
(
BaseGCS^                initialSelection,
String^                 favoritesFileList,       // files that contain OrganizationClass serialized ECXML instances.
SWF::Form^              hostForm,
SWF::ImageList^         imageList,
bool                    createFavoritesIfNeeded,
bool                    allowOrganizationModification,
bool                    allowUserLibraryModification,
IHostFormProvider^      hostFormProvider,
GCSFilterDelegate^      gcsFilter
)
    {
    m_hostForm          = hostForm;
    m_hostFormProvider  = (nullptr == hostFormProvider) ? gcnew DefaultHostFormProvider() : hostFormProvider;
    if (nullptr == imageList)
        imageList = GetDefaultImageList ();
    InitializeComponent (initialSelection, favoritesFileList, imageList, createFavoritesIfNeeded, allowOrganizationModification, allowUserLibraryModification, gcsFilter, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
GCSSelectionPanel
(
BaseGCS^                initialSelection,
String^                 favoritesFileList,       // files that contain OrganizationClass serialized ECXML instances.
SWF::Form^              hostForm,
SWF::ImageList^         imageList,
bool                    createFavoritesIfNeeded,
bool                    allowOrganizationModification,
bool                    allowUserLibraryModification,
IHostFormProvider^      hostFormProvider,
GCSFilterDelegate^      gcsFilter,
ISearchSettingsSaver^   searchSettingsSaver
)
    {
    m_hostForm          = hostForm;
    m_hostFormProvider  = (nullptr == hostFormProvider) ? gcnew DefaultHostFormProvider() : hostFormProvider;
    if (nullptr == imageList)
        imageList = GetDefaultImageList ();
    InitializeComponent (initialSelection, favoritesFileList, imageList, createFavoritesIfNeeded, allowOrganizationModification, allowUserLibraryModification, gcsFilter, searchSettingsSaver);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
~GCSSelectionPanel
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
property BaseGCS^        SelectedGCS
    {
    BaseGCS ^get ()
        {
        // duplicate the GCS, because we'll delete it when we delete this panel.
        if (nullptr == m_selectedGCS)
            return m_selectedGCS;

        return gcnew BaseGCS (m_selectedGCS->Peer);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
SWF::ImageList^ GetDefaultImageList ()
    {
    try
        {
        SWF::ImageList^ imageList   = gcnew SWF::ImageList();
        SD::Size        size        = imageList->ImageSize;

        imageList->ColorDepth       = SWF::ColorDepth::Depth32Bit;

        // get the Icon representing a closed folder and use it for the library, favorites, and group.
        SREF::Assembly^ assembly = SREF::Assembly::GetExecutingAssembly ();
        SD::Icon^       folderClosed = nullptr;
        SD::Icon^       geoCoord     = nullptr;
        SIO::Stream^    stream;

        stream       = assembly->GetManifestResourceStream ("FolderClosed.ico");
        folderClosed = gcnew SD::Icon (stream);

        stream       = assembly->GetManifestResourceStream ("GeoCoord.ico");
        geoCoord     = gcnew SD::Icon (stream);

        SWF::ImageList::ImageCollection^    images = imageList->Images;
        images->Add (LibraryImageKey,       folderClosed);
        images->Add (FavoritesImageKey,     folderClosed);
        images->Add (GroupImageKey,         folderClosed);
        images->Add (GCSImageKey,           geoCoord);
        return imageList;
        }
    catch (System::Exception^)
        {
        return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
property String^    UserOrganizationName
    {
    String^ get () { return m_userOrganizationName; }
    void set (String^ value) { m_userOrganizationName = value; }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            WriteOrganizationFiles
(
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
internal:
void            SetSelectedGCS
(
BaseGCS^            gcs,
System::Object^     sender
)
    {
    if (nullptr != gcs)
        {
        // share the same peer between the selected and source GCS so the Vertical Datum is shared.
        m_selectedGCS = gcnew BaseGCS (gcs->Peer);

        if (sender == m_librarySelectionControl)
            {
            m_libraryPageSelectedGCS = m_selectedGCS;
            }
        else if (sender == m_searchSelectionControl)
            {
            m_searchPageSelectedGCS = m_selectedGCS;
            }
        }
    else
        {
        m_selectedGCS = nullptr;
        }

    m_okButton->Enabled = (nullptr != m_selectedGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddGCSToFavorites
(
BaseGCS^        gcs
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
property IHostFormProvider^     HostFormProvider
{
IHostFormProvider^ get() {return m_hostFormProvider;}
}

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            TabPageChanged
(
System::Object^             sender,
SWF::TabControlEventArgs^   eventArgs
)
    {
    if (eventArgs->Action == SWF::TabControlAction::Selected)
        {
        if (eventArgs->TabPage == m_searchPage)
            {
            m_hostForm->AcceptButton = m_searchPageAcceptButton;
            m_selectedGCS            = m_searchPageSelectedGCS;
            }
        else
            {
            m_hostForm->AcceptButton = m_okButton;
            m_selectedGCS            = m_libraryPageSelectedGCS;
            }
        }
    m_okButton->Enabled = (nullptr != m_selectedGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
BaseGCS^                initialSelection,
String^                 favoritesFileList,       // files that contain OrganizationClass serialized ECXML instances.
SWF::ImageList^         imageList,
bool                    createFavoritesIfNeeded,
bool                    allowOrganizationModification,
bool                    allowUserLibraryModification,
GCSFilterDelegate^      gcsFilter,
ISearchSettingsSaver^   searchSettingsSaver
);


};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    LibrarySelectionControl : SWF::Panel
{
private:
SWF::TreeView^              m_treeView;
GCSPropertyPane^            m_propertyPane;
GCSECObjectModel^           m_ecObjectModel;
String^                     m_favoritesFileList;
SWF::ContextMenu^           m_contextMenu;
GCSSelectionPanel^          m_gcsSelectionPanel;
GCSFilterDelegate^          m_gcsFilter;

SWF::MenuItem^              m_createSubgroupMenuItem;
SWF::MenuItem^              m_deleteGroupMenuItem;
SWF::MenuItem^              m_cutMenuItem;
SWF::MenuItem^              m_copyMenuItem;
SWF::MenuItem^              m_pasteMenuItem;
SWF::MenuItem^              m_addToFavoritesMenuItem;
SWF::MenuItem^              m_deleteMemberMenuItem;
SWF::MenuItem^              m_deleteFromLibraryMenuItem;
SWF::MenuItem^              m_editMenuItem;
SWF::MenuItem^              m_editDatumsItem;
SWF::MenuItem^              m_editEllipsoidsItem;

// drag drop stoff
SelectionPanelTreeNode^     m_dragNode;
SD::Rectangle               m_dragRectangle;

static const int    SCROLLREGION    = 20;
static const int    SHIFT_KEY       = 4;    // from DragEventsArg documentation.
static const int    CTRL_KEY        = 8;
static const int    ALT_KEY         = 32;
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
LibrarySelectionControl
(
BaseGCS^                initialSelection,
String^                 favoritesFileList,       // files that contain OrganizationClass serialized ECXML instances.
SWF::ImageList^         imageList,
bool                    createFavoritesIfNeeded,
bool                    allowOrganizationModification,
bool                    allowUserLibraryModification,
GCSSelectionPanel^      selectionPanel,
GCSFilterDelegate^      gcsFilter
)
    {
    m_ecObjectModel         = gcnew GCSECObjectModel (false, false);
    m_favoritesFileList     = favoritesFileList;
    m_gcsSelectionPanel     = selectionPanel;
    m_gcsFilter             = gcsFilter;
    InitializeComponent (favoritesFileList, imageList, createFavoritesIfNeeded, allowOrganizationModification, allowUserLibraryModification);
    SetInitialSelection (initialSelection);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
~LibrarySelectionControl
(
)
    {
    // gets changed by compiler to Dispose method.
    // go through the treeview nodes and dispose of the BaseGCSs within.
    for each (SWF::TreeNode^ node in m_treeView->Nodes)
        {
        GroupTreeNode^  groupNode;
        if (nullptr != (groupNode = dynamic_cast<GroupTreeNode^>(node)))
            delete groupNode;
        }
    }


private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            HandleBeforeExpandEvent
(
System::Object^,
SWF::TreeViewCancelEventArgs^ eventArgs
)
    {
    SWF::TreeNode^          treeNode = eventArgs->Node;
    GroupTreeNode^          groupNode;
    UncategorizedTreeNode^  uncategorizedNode;

    if (nullptr != (groupNode = dynamic_cast<GroupTreeNode^>(treeNode)))
        groupNode->Populate ();
    else if (nullptr != (uncategorizedNode = dynamic_cast<UncategorizedTreeNode^>(treeNode)))
        uncategorizedNode->Populate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    NodeSelected
(
System::Object^,
SWF::TreeViewEventArgs^   eventArgs
)
    {
    MemberTreeNode^ memberNode;
    if (nullptr != (memberNode = dynamic_cast<MemberTreeNode^> (eventArgs->Node)))
        {
        m_propertyPane->CoordSys = memberNode->GCS;
        m_gcsSelectionPanel->SetSelectedGCS (memberNode->GCS, this);
        }
    else
        m_gcsSelectionPanel->SetSelectedGCS (nullptr, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    NodeDoubleClicked
(
System::Object^,
SWF::TreeNodeMouseClickEventArgs^   eventArgs
)
    {
    MemberTreeNode^ memberNode;
    if (nullptr != (memberNode = dynamic_cast<MemberTreeNode^> (eventArgs->Node)))
        {
        m_gcsSelectionPanel->SetSelectedGCS (memberNode->GCS, this);
        SWF::Form^  form;
        if (nullptr != (form = FindForm()))
            form->DialogResult  = SWF::DialogResult::OK;
        }
    else
        m_gcsSelectionPanel->SetSelectedGCS (nullptr, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    NodeClicked
(
System::Object^,
SWF::TreeNodeMouseClickEventArgs^   eventArgs
)
    {
    // if it's a right click, we need to set the node selection, as it doesn't get set on right click otherwise.
    if (SWF::MouseButtons::Right == eventArgs->Button)
        m_treeView->SelectedNode = eventArgs->Node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InitialSelectionCallback
(
String^         keyName,
MemberTreeNode^ memberNode,
GroupTreeNode^  groupNode,
Object^         userArg
)
    {
    BaseGCS^    initialSelection = safe_cast<BaseGCS^>(userArg);

    if ( (nullptr != keyName) && keyName->Equals (initialSelection->Name))
        {
        if (nullptr == memberNode)
            {
            // if the member node is null, then populate the group node and go traverse the group again.
            if (nullptr == groupNode)
                return false;

            groupNode->Populate();
            return (groupNode->TraverseMembers (gcnew MemberTraverseDelegate (this, &LibrarySelectionControl::InitialSelectionCallback), initialSelection, true));
            }
        else
            {
            // if memberNode not null, then simple make it the selected node.
            memberNode->EnsureVisible();

            // Unfortunately, the vertical Datum setting must be copied from the initial selection GCS to the library GCS, otherwise just going in and looking will change it.
            memberNode->GCS->VerticalDatum = initialSelection->VerticalDatum;
            m_treeView->SelectedNode = memberNode;
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetInitialSelection
(
BaseGCS^    initialSelection
)
    {
    if (nullptr == initialSelection)
        return;

    String^ initialGroup = initialSelection->Group;
    if (0 == initialGroup->Length)
        return;

    // search for the group.
    if (nullptr == m_treeView)
        return;

    SWF::TreeNode^      rootNode = m_treeView->Nodes[0];
    SC::IEnumerator^    nodeEnum;
    if ( (nullptr == rootNode) || (nullptr == rootNode->Nodes) || (nullptr == (nodeEnum = rootNode->Nodes->GetEnumerator())) )
        return;

    // go through the tree nodes looking for the initial selection gcs by key name.
    for each (SWF::TreeNode^ childNode in m_treeView->Nodes)
        {
        FileTreeNode^   fileNode;
        if (nullptr == (fileNode = dynamic_cast<FileTreeNode^>(childNode)))
            continue;
        if (fileNode->TraverseMembers (gcnew MemberTraverseDelegate (this, &LibrarySelectionControl::InitialSelectionCallback), initialSelection, true  ))
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CreateFavoritesFile
(
String^     favoriteFileName
)
    {
    // create an OrganizationClass instance.
    ECI::IECInstance^       instance = m_ecObjectModel->OrganizationClass->CreateInstance();
    ECI::IECStructValue^    root     = dynamic_cast<ECI::IECStructValue^>(instance["Root"]);

    // set up the name
    root["Name"]->StringValue = GeoCoordinateLocalization::GetLocalizedString ("Favorites");
    try
        {
        SIO::FileStream^                    instanceFile      = gcnew SIO::FileStream (favoriteFileName, SIO::FileMode::Create);
        ECXML::ECInstanceXmlStreamWriter^   instanceWriter    = gcnew ECXML::ECInstanceXmlStreamWriter (instanceFile, System::Text::Encoding::UTF8, true);
        instanceWriter->Unformatted = false;
        instanceWriter->Indent      = 2;
        instanceWriter->Serialize (instance);
        delete instanceWriter;
        }
    catch (System::Exception^)
        {
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MenuPopupEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // Here are the possibilities:
    // 1) GroupNode that can't be editted.
    // 2) Group Node that can be editted.
    //   (All group nodes in the favorites file are editable.
    //   For other Group Nodes, it depends on the "allowOrganizationModification" argument to the constructor, and also whether the .xml file is R/W or not)
    // 3) Favorites Member
    // 4) Library Member

    SWF::ContextMenu^   menu;

    // sender should be the menu item.
    if (nullptr == (menu = dynamic_cast<SWF::ContextMenu^> (sender)) )
        return;

    if (nullptr == m_treeView)
        return;

    // find the delete item.
    SelectionPanelTreeNode^       treeNode;
    if (nullptr == (treeNode = dynamic_cast <SelectionPanelTreeNode^> (m_treeView->SelectedNode)))
        return;

    SWF::Menu::MenuItemCollection^    menuItems = menu->MenuItems;
    menuItems->Clear();

    if (treeNode->CanCopyToClipboard)
        {
        if (treeNode->CanDeleteFromGroup)
            menuItems->Add (m_cutMenuItem);
        menuItems->Add (m_copyMenuItem);
        }

    if (treeNode->CanPasteFromClipboard)
        menuItems->Add (m_pasteMenuItem);

    if (treeNode->CanDeleteFromGroup)
        {
        if (treeNode->IsGroupNode)
            menuItems->Add (m_deleteGroupMenuItem);
        else
            menuItems->Add (m_deleteMemberMenuItem);
        }

    if (treeNode->CanDeleteFromLibrary)
        menuItems->Add (m_deleteFromLibraryMenuItem);

    if (treeNode->CanAddToFavorites)
        menuItems->Add (m_addToFavoritesMenuItem);

    if (treeNode->CanCreateSubgroupOf)
        menuItems->Add (m_createSubgroupMenuItem);

    if (treeNode->CanEdit)
        menuItems->Add (m_editMenuItem);

    if (treeNode->CanEditDatums)
        {
        menuItems->Add (m_editDatumsItem);
        menuItems->Add (m_editEllipsoidsItem);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DeleteGroupClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    if (nullptr == m_treeView)
        return;

    GroupTreeNode^      selectedGroup;
    if (nullptr == (selectedGroup = dynamic_cast<GroupTreeNode^>(m_treeView->SelectedNode)))
        return;

    SelectionPanelTreeNode^      parentNode;
    if (nullptr == (parentNode = dynamic_cast<SelectionPanelTreeNode^>(selectedGroup->Parent)))
        return;

    m_treeView->BeginUpdate();

    selectedGroup->Remove();

    FileTreeNode^   fileNode;
    if (nullptr != (fileNode = parentNode->FileNode))
        {
        fileNode->ResetUncategorizedNode();
        fileNode->SetOrganizationChanged();
        }

    m_treeView->EndUpdate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            NewGroupClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    GroupTreeNode^         selectedGroup;
    if ( (nullptr != m_treeView) && (nullptr != (selectedGroup = dynamic_cast<GroupTreeNode^>(m_treeView->SelectedNode))) )
        {
        // bring up a modal dialog box for the user to specify the new group name and description.
        SWF::Form^                      form     = m_gcsSelectionPanel->HostFormProvider->GetGroupPropertiesForm();
        GroupPropertiesFormContents^    contents = gcnew GroupPropertiesFormContents (form);
        SWF::DialogResult               result   = m_gcsSelectionPanel->HostFormProvider->ShowModal (form);
        if (SWF::DialogResult::OK == result)
            {
            // create a new node and add it to the selected group.
            selectedGroup->InsertGroupNode (gcnew GroupTreeNode (contents->Name, contents->Description, m_contextMenu, m_gcsFilter));
            SetOrganizationChanged (selectedGroup);
            }
        delete form;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PasteClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    GroupTreeNode^         selectedGroup;
    if ( (nullptr != m_treeView) && (nullptr != (selectedGroup = dynamic_cast<GroupTreeNode^>(m_treeView->SelectedNode))) )
        selectedGroup->DoPaste (SWF::Clipboard::GetDataObject());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            CopyMemberClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    MemberTreeNode^         selectedMember;
    if ( (nullptr != m_treeView) && (nullptr != (selectedMember = dynamic_cast<MemberTreeNode^>(m_treeView->SelectedNode))) )
        {
        SWF::Clipboard::SetDataObject (gcnew SWF::DataObject (gcnew MemberNodeInfo(selectedMember->KeyName)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            CutMemberClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    MemberTreeNode^         selectedMember;
    if ( (nullptr != m_treeView) && (nullptr != (selectedMember = dynamic_cast<MemberTreeNode^>(m_treeView->SelectedNode))) )
        {
        SWF::Clipboard::SetDataObject (gcnew SWF::DataObject (gcnew MemberNodeInfo(selectedMember->KeyName)));
        // remove from the tree
        GroupTreeNode^  parentNode;
        if (nullptr != (parentNode = dynamic_cast<GroupTreeNode^>(selectedMember->Parent)))
            {
            selectedMember->Remove();
            SetOrganizationChanged (parentNode);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DeleteMemberClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    MemberTreeNode^         selectedMember;
    if ( (nullptr != m_treeView) && (nullptr != (selectedMember = dynamic_cast<MemberTreeNode^>(m_treeView->SelectedNode))) )
        {
        GroupTreeNode^  parentNode;
        if (nullptr != (parentNode = dynamic_cast<GroupTreeNode^>(selectedMember->Parent)))
            {
            selectedMember->Remove();
            SetOrganizationChanged (parentNode);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DeleteFromLibraryClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    MemberTreeNode^         selectedMember;
    if ( (nullptr != m_treeView) && (nullptr != (selectedMember = dynamic_cast<MemberTreeNode^>(m_treeView->SelectedNode))) )
        {
        FileTreeNode^   fileNode;
        GroupTreeNode^  parentNode;

        // delete it from the file.
        if (nullptr != (fileNode = selectedMember->FileNode))
            {
            if (SUCCESS ==  fileNode->DeleteGCS (selectedMember))
                {
                // delete it from the tree
                if (nullptr != (parentNode = dynamic_cast<GroupTreeNode^>(selectedMember->Parent)))
                    {
                    selectedMember->Remove();
                    SetOrganizationChanged (parentNode);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EditPropertiesClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    MemberTreeNode^     selectedMember;
    FileTreeNode^       fileNode;
    if ( (nullptr != m_treeView) && (nullptr != (selectedMember = dynamic_cast<MemberTreeNode^>(m_treeView->SelectedNode))) && (nullptr != (fileNode = selectedMember->FileNode)) )
        {
        // clone the coordinate system and edit it.
        BaseGCS^        clone     = gcnew BaseGCS (selectedMember->GCS->Name);
        clone->CanEdit            = true;
        SWF::Form^      form      = m_gcsSelectionPanel->HostFormProvider->GetEditPropertiesForm();
        gcnew PropertiesFormContents (form, clone, true, false, false);
        SWF::DialogResult         result    = m_gcsSelectionPanel->HostFormProvider->ShowModal (form);
        // put it back to uneditable so it doesn't show up as editable in the selection dialog.
        clone->CanEdit                      = false;

        // if the user hits OK, replace the old GCS with the new one.
        if (SWF::DialogResult::OK == result)
            {
            // validate the parameters in the GCS
            array<String^>^ errors;
            if (clone->Validate (errors))
                {
                if (SUCCESS == fileNode->ReplaceInLibrary (selectedMember, clone))
                    {
                    selectedMember->GCS = clone;
                    ECI::ECInstanceListSet^ instanceListSet = gcnew ECI::ECInstanceListSet();
                    instanceListSet->Add (m_ecObjectModel->GetCoordinateSystemProperties (selectedMember->GCS));
                    m_propertyPane->SetInstanceListSet (instanceListSet);
                    m_gcsSelectionPanel->SetSelectedGCS (selectedMember->GCS, this);
                    selectedMember->SetName();
                    }
                }
            else
                {
                // show the errors.
                }
            }
        delete form;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            EditDatumsClickEvent (System::Object^ sender, System::EventArgs^ eventArgs);
void            EditEllipsoidsClickEvent (System::Object^ sender, System::EventArgs^ eventArgs);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddToFavoritesClickEvent
(
System::Object^     sender,
System::EventArgs^  eventArgs
)
    {
    // get node and make sure it's a group (it should be)
    MemberTreeNode^         selectedMember;
    if ( (nullptr != m_treeView) && (nullptr != (selectedMember = dynamic_cast<MemberTreeNode^>(m_treeView->SelectedNode))) )
        {
        AddGCSToFavorites (selectedMember->GCS);
        }
    }

internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddGCSToFavorites
(
BaseGCS^    gcsToAdd
)
    {
    FileTreeNode^  favoritesNode;
    if (nullptr == (favoritesNode = GetFavoritesNode()))
        return;

    favoritesNode->Populate();

    // if it's already in the favorites, don't add it again.
    for each (SWF::TreeNode^ treeNode in favoritesNode->Nodes)
        {
        MemberTreeNode^ memberNode;
        if ( (nullptr != (memberNode = dynamic_cast <MemberTreeNode^>(treeNode))) && (nullptr != memberNode->KeyName) && (memberNode->KeyName->Equals (gcsToAdd->Name)) )
            return;
        }

    BaseGCS^            newGCS     = gcnew BaseGCS (gcsToAdd->Name);
    MemberTreeNode^     newMember  = gcnew MemberTreeNode (newGCS, m_contextMenu);
    favoritesNode->Nodes->Add (newMember);
    SetOrganizationChanged (favoritesNode);
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            CreateMenuItems ()
    {
    m_cutMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("Cut"));
    m_cutMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::CutMemberClickEvent);

    m_copyMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("Copy"));
    m_copyMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::CopyMemberClickEvent);

    m_pasteMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("Paste"));
    m_pasteMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::PasteClickEvent);

    m_createSubgroupMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("NewGroup"));
    m_createSubgroupMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::NewGroupClickEvent);

    m_deleteGroupMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("DeleteGroup"));
    m_deleteGroupMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::DeleteGroupClickEvent);

    m_deleteMemberMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("DeleteMember"));
    m_deleteMemberMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::DeleteMemberClickEvent);

    m_addToFavoritesMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("AddToFavorites"));
    m_addToFavoritesMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::AddToFavoritesClickEvent);

    m_deleteFromLibraryMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("DeleteFromLibrary"));
    m_deleteFromLibraryMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::DeleteFromLibraryClickEvent);

    m_editMenuItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("EditProperties"));
    m_editMenuItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::EditPropertiesClickEvent);

    m_editDatumsItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("EditDatums"));
    m_editDatumsItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::EditDatumsClickEvent);

    m_editEllipsoidsItem = gcnew SWF::MenuItem (GeoCoordinateLocalization::GetLocalizedString ("EditEllipsoids"));
    m_editEllipsoidsItem->Click += gcnew System::EventHandler (this, &LibrarySelectionControl::EditEllipsoidsClickEvent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReadOrganizationFiles
(
String^         favoritesFileList,    // files that contain OrganizationClass serialized ECXML instances.
bool            createFavoritesIfNeeded,
bool            allowOrganizationModification,
bool            allowUserLibraryModification
)
    {
    array<System::Char>^    separators          = {';'};
    array<String^>^         favoriteFileNames   = favoritesFileList->Split (separators);
    bool                    foundFavorites      = false;

    // create an empty context menu. It is populated in the Popup event handler.
    m_contextMenu = gcnew SWF::ContextMenu();
    m_contextMenu->Popup += gcnew System::EventHandler (this, &LibrarySelectionControl::MenuPopupEvent);
    CreateMenuItems();

    for each (String^ fileName in favoriteFileNames)
        {
        try
            {
            FileTreeNode^   fileTreeNode = gcnew FileTreeNode (fileName, NULL, m_ecObjectModel->Schema, m_contextMenu, true, false, m_gcsFilter);
            m_treeView->Nodes->Add (fileTreeNode);
            foundFavorites = true;
            }
        catch (System::Exception^)
            {
            }
        }

    // if we didn't find a favorites file, and we're told to create one if needed, try to do that.
    if (!foundFavorites && createFavoritesIfNeeded && (favoriteFileNames->Length > 0))
        {
        if (CreateFavoritesFile (favoriteFileNames[0]))
            m_treeView->Nodes->Add (gcnew FileTreeNode (favoriteFileNames[0], NULL, m_ecObjectModel->Schema, m_contextMenu, true, false, m_gcsFilter));
        }

    BGC::LibraryManager* libManager = BGC::LibraryManager::Instance();
    size_t               libraryCount = libManager->GetLibraryCount();
    for (size_t index=0; index < libraryCount; index++)
        {
        BGC::LibraryP    library;
        bool        foundLibrary = false;

        if (NULL != (library = libManager->GetLibrary (index)))
            {
            // try to find accompanying library organization file.
            String^     orgFileName = gcnew String (library->GetOrganizationFileName());
            try
                {
                m_treeView->Nodes->Add (gcnew FileTreeNode (orgFileName, library, m_ecObjectModel->Schema, m_contextMenu, allowOrganizationModification, allowUserLibraryModification, m_gcsFilter));
                foundLibrary = true;
                }
            catch (System::Exception^)
                {
                }
            if (!foundLibrary)
                {
                if (library->IsUserLibrary())
                    m_treeView->Nodes->Add (gcnew FileTreeNode (gcnew GCSMemberEnumerator(library), library, m_contextMenu, allowUserLibraryModification, m_gcsFilter));
                else
                    m_treeView->Nodes->Add (gcnew FileTreeNode (gcnew GCSGroupEnumerator(), library, m_contextMenu, m_gcsFilter));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
FileTreeNode^   GetFavoritesNode
(
)
    {
    FileTreeNode^   favoritesNode;
    if ( (nullptr != (favoritesNode = dynamic_cast<FileTreeNode^>(m_treeView->Nodes[0]))) && !favoritesNode->IsLibraryFile)
        return favoritesNode;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetOrganizationChanged
(
SelectionPanelTreeNode^  node
)
    {
    FileTreeNode^   fileNode;
    if (nullptr != (fileNode = node->FileNode))
        fileNode->SetOrganizationChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DragOver
(
System::Object^         sender,
SWF::DragEventArgs^     eventArgs
)
    {
    // set effect so we can return on any problem.
    eventArgs->Effect = SWF::DragDropEffects::None;

    // if we're right at the bottom or the top, try to do a scroll.
    SD::Point       targetPoint = m_treeView->PointToClient (SD::Point(eventArgs->X,eventArgs->Y));
    // Note: I tried giving better feedback by putting SWF::DragDropEffects::Scroll into the output, but it did nothing, so I took it out.
    if (targetPoint.Y < m_treeView->Top + SCROLLREGION)
        ::SendMessage ((HWND)(m_treeView->Handle.ToPointer()), WM_VSCROLL, SB_LINEUP, 0);
    else if (targetPoint.Y > (m_treeView->Height - SCROLLREGION))
        ::SendMessage ((HWND)(m_treeView->Handle.ToPointer()), WM_VSCROLL, SB_LINEDOWN, 0);

    // if it's not one of our tree nodes getting dropped, forget it.
    // NOTE: You can't look for the base class with GetDataPresent, you have to look for the subclasses.
    // if it's not one of our tree nodes getting dropped, forget it.
    SelectionPanelTreeNode^     sourceNode       = nullptr;
    GroupTreeNode^              sourceGroupNode  = nullptr;
    MemberTreeNode^             sourceMemberNode = nullptr;
    SWF::IDataObject^           dataObject       = eventArgs->Data;
    if (dataObject->GetDataPresent (GroupTreeNode::typeid))
        {
        sourceGroupNode = dynamic_cast<GroupTreeNode^>(dataObject->GetData (GroupTreeNode::typeid));
        sourceNode      = sourceGroupNode;
        }
    else if (dataObject->GetDataPresent (MemberTreeNode::typeid))
        {
        sourceMemberNode = dynamic_cast<MemberTreeNode^>(dataObject->GetData (MemberTreeNode::typeid));
        sourceNode      = sourceMemberNode;
        }

    if (nullptr == sourceNode)
        return;

    // we can drop into a group node or a member node.
    SelectionPanelTreeNode^  targetNode;
    if (nullptr == (targetNode = dynamic_cast <SelectionPanelTreeNode^> (m_treeView->GetNodeAt (targetPoint))))
        return;

    if (nullptr != dynamic_cast<MemberTreeNode^>(targetNode))
        targetNode = dynamic_cast<SelectionPanelTreeNode^>(targetNode->Parent);

    // by this time the target node must be a group Node.
    GroupTreeNode^   targetGroupNode;
    if ( (nullptr == targetNode) || (nullptr == (targetGroupNode = dynamic_cast<GroupTreeNode^>(targetNode))) )
        return;

    FileTreeNode^   targetFileNode;
    if (nullptr == (targetFileNode = targetGroupNode->FileNode))
        return;

    FileTreeNode^   sourceFileNode;
    if (nullptr == (sourceFileNode = sourceNode->FileNode))
        return;

    bool                cantCopy = false;
    if (nullptr != sourceGroupNode)
        {
        cantCopy = (sourceGroupNode->Parent == targetNode);

        if (targetFileNode->CantDragFrom (sourceFileNode))
            return;

        // if the sourceNode is not the targetNode, and the sourceGroup is already in target group, then can't copy.
        if ( !cantCopy && (targetGroupNode->ContainsGroup (sourceGroupNode)) )
            return;

        // can't make self a parent.
        if (sourceGroupNode == targetGroupNode)
            return;
        }
    else if (nullptr != sourceMemberNode)
        {
        // if the sourceNode is not the targetNode, and the sourceGCS is already in target group, can't move or copy to the destination.
        cantCopy = (sourceMemberNode->Parent == targetNode);

        if (targetFileNode->CantDragFrom (sourceFileNode))
            return;

        if ( !cantCopy && (targetGroupNode->ContainsGCS (sourceMemberNode->GCS, false)) )
            return;
        }
    else
        {
        return;
        }

    // if we can't change the target, then we can't do anything.
    if (!targetFileNode->CanModifyOrganization)
        return;

    // if we can't change the source, must copy. If we cant copy, can't do anything.
    if (cantCopy && !sourceFileNode->CanModifyOrganization)
        return;

    // if target's root is different than source's root, can only copy.
    bool    mustCopy = false;
    if (!cantCopy && (sourceFileNode != targetFileNode))
        mustCopy = true;

    // Drop target is appropriate, set the effect based upon the KeyState.

    // if SHIFT, always do move.
    if (!mustCopy && (0 != (eventArgs->KeyState & SHIFT_KEY)) && (SWF::DragDropEffects::Move == (eventArgs->AllowedEffect & SWF::DragDropEffects::Move)) )
        eventArgs->Effect = SWF::DragDropEffects::Move;
    // if CTRL, do copy.
    else if ( mustCopy || (!cantCopy && (0 != (eventArgs->KeyState & CTRL_KEY)) && (SWF::DragDropEffects::Copy == (eventArgs->AllowedEffect & SWF::DragDropEffects::Copy))) )
        eventArgs->Effect = SWF::DragDropEffects::Copy;
    // By default, the drop action should be move, if allowed.
    else if (SWF::DragDropEffects::Move == (eventArgs->AllowedEffect & SWF::DragDropEffects::Move))
        eventArgs->Effect = SWF::DragDropEffects::Move;
    else
        eventArgs->Effect = SWF::DragDropEffects::None;

    // Note: I tried adding DragDropEffects::Link if the ALT key is down, but it did not result in a different cursor, so I removed it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DragDrop
(
System::Object^         sender,
SWF::DragEventArgs^     eventArgs
)
    {
    // this is the event we get when the user drops the node.
    SWF::DragDropEffects    desiredEffect = eventArgs->Effect;

    // set effect so we can return on any problem.
    eventArgs->Effect = SWF::DragDropEffects::None;

    // if it's not one of our tree nodes getting dropped, forget it.
    SWF::TreeNode^      sourceNode       = nullptr;
    GroupTreeNode^      sourceGroupNode  = nullptr;
    MemberTreeNode^     sourceMemberNode = nullptr;
    SWF::IDataObject^   dataObject = eventArgs->Data;
    if (dataObject->GetDataPresent (GroupTreeNode::typeid))
        {
        sourceGroupNode = dynamic_cast<GroupTreeNode^>(dataObject->GetData (GroupTreeNode::typeid));
        sourceNode      = sourceGroupNode;
        }
    else if (dataObject->GetDataPresent (MemberTreeNode::typeid))
        {
        sourceMemberNode = dynamic_cast<MemberTreeNode^>(dataObject->GetData (MemberTreeNode::typeid));
        sourceNode      = sourceMemberNode;
        }

    if (nullptr == sourceNode)
        return;

    // we can drop into a group node, or a member node. If we drop over a member node, we insert the new member before it.
    SelectionPanelTreeNode^  targetNode;
    SD::Point       targetPoint = m_treeView->PointToClient (SD::Point(eventArgs->X,eventArgs->Y));
    if (nullptr == (targetNode = dynamic_cast <SelectionPanelTreeNode^>(m_treeView->GetNodeAt (targetPoint))))
        return;

    // keep track of the node we want to insert before.
    SelectionPanelTreeNode^  insertBeforeNode = targetNode;
    if (nullptr != dynamic_cast<MemberTreeNode^>(targetNode))
        targetNode = dynamic_cast <SelectionPanelTreeNode^>(targetNode->Parent);

    // if the sourceNode is a group, and the parent is the targetNode's parent, then we are rearranging groups within the the parent, not making it more deeply nested.
    if ( (nullptr != sourceGroupNode) && (sourceGroupNode->Parent == insertBeforeNode->Parent) )
        targetNode = dynamic_cast <SelectionPanelTreeNode^>(targetNode->Parent);

    // if they're the same, don't do anything.
    if (sourceNode == insertBeforeNode)
        return;

    GroupTreeNode^  targetGroupNode;
    if ( (nullptr == targetNode) || (nullptr == (targetGroupNode = dynamic_cast<GroupTreeNode^>(targetNode))) )
        return;

    // if the source is a group, and insertBeforeNode is not another group within the same parent, we want to insert before the first member node of the target.
    if ( (nullptr != sourceGroupNode) && ( (nullptr == dynamic_cast<GroupTreeNode^>(insertBeforeNode)) || (sourceNode->Parent != insertBeforeNode->Parent) ) )
        insertBeforeNode = targetGroupNode->FirstMemberNode;

    // if user holds alt key down, we put him back to where he started. This is mostly a convenience for
    //   us when we do big reorganizations.
    SWF::TreeNode^  returnToPreviousSelectNode = nullptr;

    // stop updating while we fool around with the tree.
    m_treeView->BeginUpdate();

    // have source and target node.
    SWF::TreeNode^  nodeToInsert = nullptr;
    if (SWF::DragDropEffects::Copy == desiredEffect)
        {
        if (0 != (eventArgs->KeyState & ALT_KEY))
            returnToPreviousSelectNode = sourceNode;

        if (nullptr != sourceGroupNode)
            {
            // copy the group, add it to the targetNode.
            nodeToInsert = gcnew GroupTreeNode (sourceGroupNode, m_gcsFilter);
            }
        else if (nullptr != sourceMemberNode)
            {
            // we don't want to just use another reference to sourceGCS because that would complicate our Disposal.
            BaseGCS^ newGCS = gcnew BaseGCS (sourceMemberNode->KeyName);
            nodeToInsert    = gcnew MemberTreeNode (newGCS, m_contextMenu);
            }
        }
    else if (SWF::DragDropEffects::Move == desiredEffect)
        {
        if (0 != (eventArgs->KeyState & ALT_KEY))
            returnToPreviousSelectNode = sourceNode->NextVisibleNode;

        // reparent the node. To avoid confusion, deselect.
        m_treeView->SelectedNode = nullptr;

        SelectionPanelTreeNode^  parentNode;
        if (nullptr != (parentNode = dynamic_cast <SelectionPanelTreeNode^>(sourceNode->Parent)))
            {
            parentNode->Nodes->Remove (sourceNode);
            SetOrganizationChanged(parentNode);
            }

        nodeToInsert = sourceNode;
        }

    // here we have the node to insert, and we know where to insert it.
    if (nullptr != nodeToInsert)
        {
        // make sure the existing members are in the target node before we add the new one - otherwise it ends up at the beginning, not the end.
        targetGroupNode->Populate();

        if (!targetNode->IsExpanded)
            targetNode->Expand();

        SWF::TreeNodeCollection^    nodes = targetNode->Nodes;
        int                         insertBeforeIndex;
        if ( (nullptr == insertBeforeNode) || (-1 == (insertBeforeIndex = nodes->IndexOf (insertBeforeNode))) )
            nodes->Add (nodeToInsert);
        else
            nodes->Insert (insertBeforeIndex, nodeToInsert);

        SetOrganizationChanged (targetNode);

        if (nullptr != returnToPreviousSelectNode)
            {
            returnToPreviousSelectNode->EnsureVisible();
            m_treeView->SelectedNode = returnToPreviousSelectNode;
            }
        else
            {
            if (!targetNode->IsExpanded)
                targetNode->Expand();

            // select it and make sure it's visible.
            nodeToInsert->EnsureVisible();
            m_treeView->SelectedNode = nodeToInsert;
            }
        }

    m_treeView->EndUpdate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MouseDown
(
System::Object^          sender,
SWF::MouseEventArgs^     eventArgs
)
    {
    if (sender != m_treeView)
        return;

    SelectionPanelTreeNode^ sourceNode;
    if (nullptr != (sourceNode = dynamic_cast<SelectionPanelTreeNode^> (m_treeView->GetNodeAt (eventArgs->X, eventArgs->Y))))
        {
        m_dragNode          = sourceNode;
        SD::Size dragSize   = SWF::SystemInformation::DragSize;
        m_dragRectangle     = SD::Rectangle (SD::Point(eventArgs->X - (dragSize.Width /2), eventArgs->Y - (dragSize.Height /2)), dragSize);
        }
    else
        {
        m_dragNode      = nullptr;
        m_dragRectangle = SD::Rectangle::Empty;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MouseUp
(
System::Object^         sender,
SWF::MouseEventArgs^     eventArgs
)
    {
    if (sender != m_treeView)
        return;

    m_dragRectangle = SD::Rectangle::Empty;
    m_dragNode      = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MouseMove
(
System::Object^          sender,
SWF::MouseEventArgs^     eventArgs
)
    {
    if (sender != m_treeView)
        return;

    if ( (nullptr != m_dragNode) && (m_dragRectangle != SD::Rectangle::Empty) && !m_dragRectangle.Contains (eventArgs->X, eventArgs->Y))
        {
        // select the node, because otherwise the highlight moves to the previously selected node, which I don't like.
        m_treeView->SelectedNode = m_dragNode;

        // Proceed with the drag-and-drop, passing the node.
        /*SWF::DragDropEffects dropEffect =*/ m_treeView->DoDragDrop (m_dragNode, SWF::DragDropEffects::All);

        m_dragNode = nullptr;
        m_dragRectangle = SD::Rectangle::Empty;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
String^                     favoritesFileList,      // files that contain OrganizationClass serialized ECXML instances representing favorites.
SWF::ImageList^             imageList,
bool                        createFavoritesIfNeeded,
bool                        allowOrganizationModification,
bool                        allowUserLibraryModification
)
    {
    // Set up the "Library" Tab page.
    m_treeView = gcnew SWF::TreeView ();
    m_treeView->HideSelection   = false;      // want selection hilited even when this treeView doesn't have focus.
    m_treeView->ImageList       = imageList;
    m_treeView->Dock = SWF::DockStyle::Fill;
    m_treeView->BeforeExpand    += gcnew SWF::TreeViewCancelEventHandler (this, &LibrarySelectionControl::HandleBeforeExpandEvent);
    m_treeView->AfterSelect     += gcnew SWF::TreeViewEventHandler (this, &LibrarySelectionControl::NodeSelected);
    m_treeView->NodeMouseDoubleClick += gcnew SWF::TreeNodeMouseClickEventHandler (this, &LibrarySelectionControl::NodeDoubleClicked);
    m_treeView->NodeMouseClick       += gcnew SWF::TreeNodeMouseClickEventHandler (this, &LibrarySelectionControl::NodeClicked);

    // enable the Drag/Drop events we need to allow the user to reorganize the grouping.
    m_treeView->AllowDrop       = true;
    m_treeView->DragDrop        += gcnew SWF::DragEventHandler (this, &LibrarySelectionControl::DragDrop);
    m_treeView->DragOver        += gcnew SWF::DragEventHandler (this, &LibrarySelectionControl::DragOver);
    m_treeView->MouseDown       += gcnew SWF::MouseEventHandler (this, &LibrarySelectionControl::MouseDown);
    m_treeView->MouseUp         += gcnew SWF::MouseEventHandler (this, &LibrarySelectionControl::MouseUp);
    m_treeView->MouseMove       += gcnew SWF::MouseEventHandler (this, &LibrarySelectionControl::MouseMove);

    ReadOrganizationFiles (favoritesFileList, createFavoritesIfNeeded, allowOrganizationModification, allowUserLibraryModification);
    m_treeView->SelectedNode    = m_treeView->Nodes[0];
    m_treeView->Nodes[0]->Expand();

    m_propertyPane                          = gcnew GCSPropertyPane (nullptr, m_ecObjectModel, "GCSLibrary");
    m_propertyPane->LayoutPreset            = BUCWG::GroupLayoutPreset::StrictTopDown;
    m_propertyPane->Dock                    = SWF::DockStyle::Fill;

    SWF::SplitContainer^  splitContainer = gcnew SWF::SplitContainer();
    splitContainer->Panel1->Controls->Add (m_treeView);
    splitContainer->Panel2->Controls->Add (m_propertyPane);
    splitContainer->Dock                = SWF::DockStyle::Fill;
    splitContainer->Size                = SD::Size(750,500);
    splitContainer->Panel1MinSize       = 150;
    splitContainer->Panel2MinSize       = 150;

    // don't do the layout until we're done adding stuff.
    SuspendLayout();
    Controls->Add (splitContainer);
    ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
public:
void            WriteOrganizationFiles
(
)
    {
    for each (SWF::TreeNode^ firstLevelNode in m_treeView->Nodes)
        {
        FileTreeNode^   fileNode;

        if (nullptr != (fileNode = dynamic_cast<FileTreeNode^> (firstLevelNode)))
            fileNode->WriteOrganizationFile (m_ecObjectModel->OrganizationClass);
        }
    }

};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   06/09
+===============+===============+===============+===============+===============+======*/
public ref class    ListViewSorter : SC::IComparer
{
private:
Int32                   m_column;
SWF::SortOrder          m_sortOrder;
SWF::ListView^          m_list;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
public:
ListViewSorter (SWF::ListView^ list)
    {
    m_column        = -1;
    m_sortOrder     = SWF::SortOrder::None;

    m_list = list;

    list->ColumnClick += gcnew SWF::ColumnClickEventHandler (this, &ListViewSorter::ColumnClick);
    }

property Int32          Column
    { Int32 get() { return m_column; } }

property SWF::SortOrder SortOrder
    { SWF::SortOrder get() { return m_sortOrder; } }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual int     Compare (Object^ lhs, Object^ rhs)
    {
    SWF::ListViewItem^  lhsLvi;
    SWF::ListViewItem^  rhsLvi;
    if (nullptr == (lhsLvi = dynamic_cast <SWF::ListViewItem ^> (lhs)))
        return 0;

    if (nullptr == (rhsLvi = dynamic_cast <SWF::ListViewItem ^> (rhs)))
        return 0;

    SWF::ListViewItem::ListViewSubItemCollection^ lhsItems = lhsLvi->SubItems;
    SWF::ListViewItem::ListViewSubItemCollection^ rhsItems = rhsLvi->SubItems;

    String^ lhsText = (lhsItems->Count > m_column) ? lhsItems[m_column]->Text : String::Empty;
    String^ rhsText = (rhsItems->Count > m_column) ? rhsItems[m_column]->Text : String::Empty;

    int result = 0;
    if (lhsText->Length == 0 || rhsText->Length == 0)
        result = lhsText->CompareTo (rhsText);
    else
        {
        if (m_column == 4)
            // this is the EPSG column.
            result = Int32::Parse (lhsText, SG::NumberStyles::Number) - Int32::Parse (rhsText, SG::NumberStyles::Number);
        else
            result = String::Compare (lhsText, rhsText, true);
        }

    return (SWF::SortOrder::Ascending == m_sortOrder) ? result : -result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                Sort (Int32 column)
    {
    SWF::SortOrder order = SWF::SortOrder::Ascending;

    if (column == m_column)
        order = (m_sortOrder == SWF::SortOrder::Ascending) ? SWF::SortOrder::Descending : SWF::SortOrder::Ascending;

    Sort (column, order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                Sort (Int32 column, SWF::SortOrder order)
    {
    if (column != m_column)
        {
        ShowHeaderIcon (m_column, SWF::SortOrder::None);
        m_column = column;
        }

    ShowHeaderIcon (m_column, order);
    m_sortOrder = order;

    if (m_sortOrder != SWF::SortOrder::None)
        {
        if (nullptr == m_list->ListViewItemSorter)
            m_list->ListViewItemSorter = this;
        else
            m_list->Sort();
        }
    else
        m_list->ListViewItemSorter = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void                ColumnClick (Object^ sender, SWF::ColumnClickEventArgs^ e)
    {
    this->Sort (e->Column);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                    ShowHeaderIcon (int columnIndex, SWF::SortOrder sortOrder)
    {
    if (columnIndex < 0 || columnIndex >= m_list->Columns->Count)
        return;

    HWND                        hHeader = (HWND) ::SendMessage ((HWND)(m_list->Handle.ToPointer()), LVM_GETHEADER, 0, 0);

    SWF::ColumnHeader^          colHdr  = m_list->Columns[columnIndex];

    HDITEM                      hd;
    memset (&hd, 0, sizeof (hd));
    hd.mask = HDI_FORMAT;

    SWF::HorizontalAlignment    align = colHdr->TextAlign;

    if (align == SWF::HorizontalAlignment::Left)
        hd.fmt = HDF_LEFT | HDF_STRING | HDF_BITMAP_ON_RIGHT;
    else if (align == SWF::HorizontalAlignment::Center)
        hd.fmt = HDF_CENTER | HDF_STRING | HDF_BITMAP_ON_RIGHT;
    else    // HorizontalAlignment.Right
        hd.fmt = HDF_RIGHT | HDF_STRING;

    if (sortOrder == SWF::SortOrder::Ascending)
        hd.fmt |= HDF_SORTUP;
    else if (sortOrder == SWF::SortOrder::Descending)
        hd.fmt |= HDF_SORTDOWN;

    ::SendMessage (hHeader, HDM_SETITEM, columnIndex, (LPARAM) &hd);
    }
};


/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   03/07
+===============+===============+===============+===============+===============+======*/
public ref class    SearchSelectionControl : SWF::Panel
{
private:
SWF::TextBox^               m_searchTextField;
SWF::Label^                 m_searchLabel;
SWF::Button^                m_searchButton;
SWF::Button^                m_stopSearchButton;
SWF::ListView^              m_listView;
ListViewSorter^             m_listViewSorter;
SWF::Button^                m_addFavoriteButton;
SWF::Button^                m_propertiesButton;
GCSSelectionPanel^          m_gcsSelectionPanel;
SWF::RadioButton^           m_anyWordButton;
SWF::RadioButton^           m_allWordsButton;
SWF::ProgressBar^           m_progressBar;
bool                        m_anyWord;

SCM::BackgroundWorker^      m_backgroundWorker;
String^                     m_lastSearchText;
SWF::Timer^                 m_selectTimer;
LibrarySearcher^            m_librarySearcher;
GCSFilterDelegate^          m_gcsFilter;
ISearchSettingsSaver^       m_settingsSaver;

static const int        SPACING_Y       = 2;
static const int        SPACING_X       = 8;

static const int        LABEL_X         = 10;
static const int        LABEL_H         = 20;
static const int        LABEL_W         = 80;
static const int        FIELD_X         = LABEL_X + LABEL_W + SPACING_X;
static const int        FIELD_H         = 20;
static const int        FIELD_W         = 340;
static const int        SEARCHBUTTON_X  = FIELD_X + FIELD_W + SPACING_X;
static const int        SEARCHBUTTON_W  = 90;
static const int        TOPBUTTON_H     = 20;
static const int        STOPBUTTON_X    = SEARCHBUTTON_X + SEARCHBUTTON_W + SPACING_X;
static const int        STOPBUTTON_W    = 70;
static const int        SEARCH_Y        = 10;

static const int        RADIOBUTTON_Y   = SEARCH_Y + BUTTON_H;
static const int        ANYWORD_X       = FIELD_X;
static const int        RADIOBUTTON_H   = 20;
static const int        RADIOBUTTON_W   = 120;
static const int        ALLWORDS_X      = ANYWORD_X + RADIOBUTTON_W;

static const int        LISTVIEW_Y      = RADIOBUTTON_Y + RADIOBUTTON_H + SPACING_Y;
static const int        LISTVIEW_X      = 10;
static const int        LISTVIEW_W      = 600;
static const int        LISTVIEW_H      = 500;

static const int        ADDFAVORITE_X   = 20;
static const int        BUTTON_Y        = LISTVIEW_Y + LISTVIEW_H + SPACING_Y;
static const int        BUTTON_W        = 130;
static const int        BUTTON_H        = 24;

static const int        PROPERTIES_X    = ADDFAVORITE_X + BUTTON_W + SPACING_X;

static const int        RIGHT_PAD       = 10;
static const int        PANEL_W         = LISTVIEW_X + LISTVIEW_W + RIGHT_PAD;
static const int        PANEL_H         = BUTTON_Y   + BUTTON_H + SPACING_Y;

static const int        PROGRESSBAR_X   = PROPERTIES_X + BUTTON_W + SPACING_X;
static const int        PROGRESSBAR_Y   = BUTTON_Y + 3;
static const int        PROGRESSBAR_W   = PANEL_W - PROGRESSBAR_X - SPACING_X;
static const int        PROGRESSBAR_H   = 18;


internal:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
SearchSelectionControl
(
SWF::Button^            searchButton,
GCSSelectionPanel^      selectionPanel,
GCSFilterDelegate^      gcsFilter,
ISearchSettingsSaver^   settingsSaver
)
    {
    m_gcsSelectionPanel     = selectionPanel;
    m_lastSearchText        = nullptr;
    m_selectTimer           = nullptr;
    m_librarySearcher       = nullptr;
    m_anyWord               = true;
    m_gcsFilter             = gcsFilter;
    m_settingsSaver         = settingsSaver;
    InitializeComponent (searchButton);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
~SearchSelectionControl
(
)
    {
    if (nullptr != m_librarySearcher)
        {
        delete m_librarySearcher;
        m_librarySearcher   = nullptr;
        }
    ClearResults();
    m_listView = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextChanged
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    // when the text is not empty, emable find now button.
    EnableItems();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SearchButtonClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    StartSearch();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            StopSearchButtonClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    if (m_backgroundWorker->IsBusy)
        m_backgroundWorker->CancelAsync();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ResultDoubleClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    // when the text is changed, we do the search.
    BaseGCS^    selectedGCS;
    if (nullptr != (selectedGCS = GetSelectedGCS()))
        {
        m_gcsSelectionPanel->SetSelectedGCS (selectedGCS, this);
        SWF::Form^  form;
        if (nullptr != (form = FindForm()))
            form->DialogResult  = SWF::DialogResult::OK;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS^    GetSelectedGCS
(
)
    {
    SWF::ListView::SelectedListViewItemCollection^  selectedItems;
    int                                             selectedCount = 0;
    if ( (nullptr == (selectedItems = m_listView->SelectedItems)) || (0 == (selectedCount = selectedItems->Count)) )
        return nullptr;

    SWF::ListViewItem^          selected = selectedItems[0];
    return dynamic_cast<BaseGCS^>(selected->Tag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertiesButtonClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    BaseGCS^    gcs;
    if (nullptr != (gcs = GetSelectedGCS()))
        {
        SWF::Form^ propertiesForm = m_gcsSelectionPanel->HostFormProvider->GetShowPropertiesForm();
        gcnew PropertiesFormContents (propertiesForm, gcs, false, false, false);
        m_gcsSelectionPanel->HostFormProvider->ShowModal (propertiesForm);
        delete propertiesForm;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddFavoriteButtonClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    SWF::ListView::SelectedListViewItemCollection^  selectedItems;
    if (nullptr == (selectedItems = m_listView->SelectedItems))
        return;

    for each (SWF::ListViewItem^ selectedItem in selectedItems)
        {
        BaseGCS^    gcs;
        if (nullptr != (gcs = dynamic_cast<BaseGCS^>(selectedItem->Tag)))
            m_gcsSelectionPanel->AddGCSToFavorites (gcs);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ListViewSelectionChanged
(
System::Object^                                 sender,
SWF::ListViewItemSelectionChangedEventArgs^     eventArgs
)
    {
    SWF::ListView::SelectedListViewItemCollection^  selectedItems;
    int                                             selectedCount = 0;
    if ( (nullptr == (selectedItems = m_listView->SelectedItems)) || (0 == (selectedCount = selectedItems->Count)) )
        {
        m_propertiesButton->Enabled     = false;
        m_addFavoriteButton->Enabled    = false;
        }
    else
        {
        m_addFavoriteButton->Enabled = true;
        m_propertiesButton->Enabled  = (1 == selectedCount);
        }
    BaseGCS^    selectedGCS;

    if (nullptr != (selectedGCS = GetSelectedGCS()))
        m_gcsSelectionPanel->SetSelectedGCS (selectedGCS, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SelectTimerTimeout
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    m_selectTimer->Stop();
    m_selectTimer = nullptr;
    m_searchTextField->Select();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextFieldVisibleChanged
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    if (Visible)
        {
        EnableItems();
        // for some reason, setting the text field selected in line does not work, it has to be done on a timer.
        m_selectTimer = gcnew SWF::Timer ();
        m_selectTimer->Interval = 10;
        m_selectTimer->Tick += gcnew System::EventHandler (this, &SearchSelectionControl::SelectTimerTimeout);
        m_selectTimer->Start();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            RadioButtonClicked
(
System::Object^         sender,
System::EventArgs^      eventArgs
)
    {
    SWF::RadioButton^ rb = dynamic_cast <SWF::RadioButton^>(sender);
    if (m_anyWordButton == rb)
        m_anyWord = m_anyWordButton->Checked;
    else if (m_allWordsButton == rb)
        m_anyWord = !m_allWordsButton->Checked;

    // re-search on change, but only if there's more than one word to search for.
    String^ searchString                    = m_searchTextField->Text;
    if (0 == searchString->Length)
        return;

    array<System::Char>^    separators      = {' '};
    array<String^>^         searchStrings   = searchString->Split (separators);
    if (searchStrings->Length > 1)
        StartSearch();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetSettings ([SRI::Out] array<Int32>^% columnOrder, [SRI::Out] array<Int32>^% columnWidths, int expectedDimension)
    {
    columnOrder     = nullptr;
    columnWidths    = nullptr;

    if (nullptr == m_settingsSaver)
        return;

    m_settingsSaver->ReadState();
    columnOrder  = m_settingsSaver->GetColumnOrder();
    columnWidths = m_settingsSaver->GetColumnWidths();

    if ( (nullptr != columnOrder) && (columnOrder->Length != expectedDimension) )
        columnOrder = nullptr;
    if ( (nullptr != columnWidths) && (columnWidths->Length != expectedDimension) )
        columnWidths = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveSettings ()
    {
    if (nullptr == m_settingsSaver)
        return;

    SWF::ListView::ColumnHeaderCollection^ listColumns = m_listView->Columns;

    int     numColumns = listColumns->Count;

    array<Int32>^ columnOrder  = gcnew array<Int32> (numColumns);
    array<Int32>^ columnWidths = gcnew array<Int32> (numColumns);

    for (int iColumn = 0; iColumn < numColumns; iColumn++)
        {
        LVCOLUMN    columnInfo;
        memset (&columnInfo, 0, sizeof (columnInfo));
        columnOrder[iColumn]  = listColumns[iColumn]->DisplayIndex;
        columnWidths[iColumn] = listColumns[iColumn]->Width;
        }

    m_settingsSaver->SaveColumnOrder (columnOrder);
    m_settingsSaver->SaveColumnWidths (columnWidths);
    m_settingsSaver->SaveState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            InitializeComponent
(
SWF::Button^    searchButton
)
    {
    System::ComponentModel::ComponentResourceManager ^resources = gcnew System::ComponentModel::ComponentResourceManager (SearchSelectionControl::typeid);

    // localizer can set the Location and Size of the Search label.
    m_searchLabel                           = gcnew SWF::Label();
    m_searchLabel->Text                     = GeoCoordinateLocalization::GetLocalizedString ("SearchText");
    m_searchLabel->TextAlign                = SD::ContentAlignment::MiddleRight;
    m_searchLabel->Size                     = SD::Size (LABEL_W, LABEL_H);
    m_searchLabel->Location                 = SD::Point (LABEL_X, SEARCH_Y);
    m_searchLabel->TabIndex                 = 0;
    // the size is adjusted by the SearchSelectionControl.resx resource file.
    resources->ApplyResources (m_searchLabel, "SearchLabel");

    SD::Point       searchLabelLocation     = m_searchLabel->Location;
    SD::Size        searchLabelSize         = m_searchLabel->Size;

    // localizer can set the Location and Size of the Search button.
    m_searchButton                          = searchButton;
    m_searchButton->Text                    = GeoCoordinateLocalization::GetLocalizedString ("FindNow");
    m_searchButton->Size                    = SD::Size (SEARCHBUTTON_W, TOPBUTTON_H);
    m_searchButton->Location                = SD::Point (SEARCHBUTTON_X, SEARCH_Y);
    m_searchButton->Anchor                  = (SWF::AnchorStyles::Right | SWF::AnchorStyles::Top);
    m_searchButton->TabIndex                = 3;
    m_searchButton->Click                   += gcnew System::EventHandler (this, &SearchSelectionControl::SearchButtonClicked);
    // the location of the search button will be calculated below, but the size is adjusted by the SearchSelectionControl.resx resource file.
    resources->ApplyResources (m_searchButton, "SearchButton");

    // NOTE: without this StopSearch button, the Cancel button in the GCSSelectionPanel gets focus when we disable the Search button while searching.
    // Then if the user hits Return while the search is still going, the dialog exits. Since the search is still going, we get a crash when it finishes.
    // That's why I added the StopSearch button, the Cancel support for the BackgroundWorker, and the progress bar (TR#230515).

    m_stopSearchButton                      = gcnew SWF::Button();
    m_stopSearchButton->Text                = GeoCoordinateLocalization::GetLocalizedString ("StopSearch");
    m_stopSearchButton->Size                = SD::Size (STOPBUTTON_W, TOPBUTTON_H);
    m_stopSearchButton->Location            = SD::Point (STOPBUTTON_X, SEARCH_Y);
    m_stopSearchButton->Anchor              = (SWF::AnchorStyles::Right | SWF::AnchorStyles::Top);
    m_stopSearchButton->Enabled             = false;
    m_stopSearchButton->TabIndex            = 4;
    m_stopSearchButton->Click               += gcnew System::EventHandler (this, &SearchSelectionControl::StopSearchButtonClicked);
    // the location of the stop search button will be calculated below, but the size is adjusted by the SearchSelectionControl.resx resource file.
    resources->ApplyResources (m_stopSearchButton, "StopSearchButton");

    SD::Size        searchButtonSize        = m_searchButton->Size;
    SD::Size        stopButtonSize          = m_stopSearchButton->Size;

    // calculate X. Y is fixed
    SD::Point       stopButtonLocation      = m_stopSearchButton->Location;
    stopButtonLocation.X                    = PANEL_W - (stopButtonSize.Width + SPACING_X);

    SD::Point       searchButtonLocation    = m_searchButton->Location;
    searchButtonLocation.X                  = stopButtonLocation.X - (searchButtonSize.Width + SPACING_X);

    m_searchButton->Location                = searchButtonLocation;
    m_stopSearchButton->Location            = stopButtonLocation;

    // Search Text field is positioned between the search Label and the search button.
    SD::Point       searchTextLocation;
    searchTextLocation.X                    = searchLabelLocation.X + searchLabelSize.Width + SPACING_X;
    searchTextLocation.Y                    = searchLabelLocation.Y;

    SD::Size        searchTextSize;
    searchTextSize.Width                    = searchButtonLocation.X - (searchTextLocation.X + SPACING_X);
    searchTextSize.Height                   = FIELD_H;

    m_searchTextField                       = gcnew SWF::TextBox();
    m_searchTextField->TextChanged          += gcnew System::EventHandler (this, &SearchSelectionControl::TextChanged);
    m_searchTextField->Size                 = searchTextSize;
    m_searchTextField->Location             = searchTextLocation;
    m_searchTextField->TabIndex             = 2;
    m_searchTextField->Anchor               = (SWF::AnchorStyles::Right | SWF::AnchorStyles::Top | SWF::AnchorStyles::Left);
    m_searchTextField->VisibleChanged       += gcnew System::EventHandler (this, &SearchSelectionControl::TextFieldVisibleChanged);

    m_anyWordButton                         = gcnew SWF::RadioButton();
    m_anyWordButton->Text                   = GeoCoordinateLocalization::GetLocalizedString ("AnyWord");
    m_anyWordButton->Size                   = SD::Size (RADIOBUTTON_W, RADIOBUTTON_H);
    m_anyWordButton->Location               = SD::Point (ANYWORD_X, RADIOBUTTON_Y);
    m_anyWordButton->Anchor                 = (SWF::AnchorStyles::Top | SWF::AnchorStyles::Left);
    m_anyWordButton->Checked                = m_anyWord;
    m_anyWordButton->Click                  += gcnew System::EventHandler (this, &SearchSelectionControl::RadioButtonClicked);
    resources->ApplyResources (m_anyWordButton, "AnyWord");

    SD::Point   anyWordLocation             = m_anyWordButton->Location;
    SD::Size    anyWordSize                 = m_anyWordButton->Size;
    SD::Point   allWordsLocation            = anyWordLocation;
    allWordsLocation.X                      = anyWordLocation.X + anyWordSize.Width + SPACING_X;

    m_allWordsButton                        = gcnew SWF::RadioButton();
    m_allWordsButton->Text                  = GeoCoordinateLocalization::GetLocalizedString ("AllWords");
    m_allWordsButton->Size                  = SD::Size (RADIOBUTTON_W, RADIOBUTTON_H);
    m_allWordsButton->Location              = allWordsLocation;
    m_allWordsButton->Anchor                = (SWF::AnchorStyles::Top | SWF::AnchorStyles::Left);
    m_allWordsButton->Checked               = !m_anyWord;
    m_allWordsButton->Click                 += gcnew System::EventHandler (this, &SearchSelectionControl::RadioButtonClicked);
    resources->ApplyResources (m_allWordsButton, "AnyWord");

    m_addFavoriteButton                     = gcnew SWF::Button ();
    m_addFavoriteButton->Text               = GeoCoordinateLocalization::GetLocalizedString ("AddToFavorites");
    m_addFavoriteButton->Size               = SD::Size (BUTTON_W, BUTTON_H);
    m_addFavoriteButton->Location           = SD::Point (ADDFAVORITE_X, BUTTON_Y);
    m_addFavoriteButton->Anchor             = (SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom);
    m_addFavoriteButton->Click              += gcnew System::EventHandler (this, &SearchSelectionControl::AddFavoriteButtonClicked);
    m_addFavoriteButton->TabIndex           = 5;
    m_addFavoriteButton->Enabled            = false;
    resources->ApplyResources (m_addFavoriteButton, "AddToFavoritesButton");

    SD::Point       addFavoriteLocation     = m_addFavoriteButton->Location;
    SD::Size        addFavoriteSize         = m_addFavoriteButton->Size;
    SD::Point       propertiesLocation      = addFavoriteLocation;
    propertiesLocation.X                    = addFavoriteLocation.X + addFavoriteSize.Width + SPACING_X;

    m_propertiesButton                      = gcnew SWF::Button ();
    m_propertiesButton->Text                = GeoCoordinateLocalization::GetLocalizedString ("Properties");
    m_propertiesButton->Size                = SD::Size (BUTTON_W, BUTTON_H);
    m_propertiesButton->Location            = propertiesLocation;
    m_propertiesButton->Anchor              = (SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom);
    m_propertiesButton->Click              += gcnew System::EventHandler (this, &SearchSelectionControl::PropertiesButtonClicked);
    m_propertiesButton->TabIndex            = 6;
    m_propertiesButton->Enabled             = false;
    resources->ApplyResources (m_propertiesButton, "PropertiesButton");

    propertiesLocation                      = m_propertiesButton->Location;
    SD::Size propertiesSize                 = m_propertiesButton->Size;

    // progress bar position calculated from properties button location.
    SD::Point       progressLocation;
    progressLocation.X                      = propertiesLocation.X + propertiesSize.Width + SPACING_X;
    progressLocation.Y                      = propertiesLocation.Y;
    SD::Size        progressSize;
    progressSize.Width                      = PANEL_W - (progressLocation.X + SPACING_X);
    progressSize.Height                     = 18;

    if (progressSize.Width < 4)
        progressSize.Width = 4;

    m_progressBar                           = gcnew SWF::ProgressBar ();
    m_progressBar->Size                     = progressSize;
    m_progressBar->Location                 = progressLocation;
    m_progressBar->Anchor                   = (SWF::AnchorStyles::Left | SWF::AnchorStyles::Right | SWF::AnchorStyles::Bottom);
    m_progressBar->Visible                  = false;

    m_listView                              = gcnew SWF::ListView();
    m_listView->Dock                        = SWF::DockStyle::Fill;
    m_listView->View                        = SWF::View::Details;
    m_listView->Location                    = SD::Point (LISTVIEW_X, LISTVIEW_Y);
    m_listView->Size                        = SD::Size (LISTVIEW_W, LISTVIEW_H);
    m_listView->Anchor                      = (SWF::AnchorStyles::Right | SWF::AnchorStyles::Top | SWF::AnchorStyles::Left | SWF::AnchorStyles::Bottom);
    m_listView->TabIndex                    = 4;
    m_listView->ItemSelectionChanged        += gcnew SWF::ListViewItemSelectionChangedEventHandler (this, &SearchSelectionControl::ListViewSelectionChanged);
    m_listView->HideSelection               = false;      // want selection hilited even when this treeView doesn't have focus.
    m_listView->DoubleClick                 += gcnew System::EventHandler (this, &SearchSelectionControl::ResultDoubleClicked);
    m_listView->View                        = SWF::View::Details;
    m_listView->AllowColumnReorder          = true;
    m_listViewSorter                        = gcnew ListViewSorter (m_listView);

    m_backgroundWorker                      = gcnew SCM::BackgroundWorker();
    m_backgroundWorker->DoWork              += gcnew SCM::DoWorkEventHandler (this, &SearchSelectionControl::DoSearchInBackground);
    m_backgroundWorker->RunWorkerCompleted  += gcnew SCM::RunWorkerCompletedEventHandler (this, &SearchSelectionControl::SearchCompleted);

    // set background worker to allow cancellation.
    m_backgroundWorker->WorkerSupportsCancellation = true;


    array<Int32>^   columnOrder;
    array<Int32>^   columnWidths;
    GetSettings (columnOrder, columnWidths, 7);

    SWF::ListView::ColumnHeaderCollection^ columns = m_listView->Columns;

    array<SWF::ColumnHeader^>^ listColumns = gcnew array<SWF::ColumnHeader^>(7);
    listColumns[0] = gcnew SWF::ColumnHeader ();
    listColumns[0]->Text  = GeoCoordinateLocalization::GetLocalizedString ("KeyName");
    listColumns[0]->Width = (nullptr != columnWidths) ? columnWidths[0] : 130;

    listColumns[1] = gcnew SWF::ColumnHeader ();
    listColumns[1]->Text = GeoCoordinateLocalization::GetLocalizedString ("Description");
    listColumns[1]->Width = (nullptr != columnWidths) ? columnWidths[1] : 200;

    listColumns[2] = gcnew SWF::ColumnHeader ();
    listColumns[2]->Text = GeoCoordinateLocalization::GetLocalizedString ("Units");
    listColumns[2]->Width = (nullptr != columnWidths) ? columnWidths[2] : 60;

    listColumns[3] = gcnew SWF::ColumnHeader ();
    listColumns[3]->Text = GeoCoordinateLocalization::GetLocalizedString ("Projection");
    listColumns[3]->Width = (nullptr != columnWidths) ? columnWidths[3] : 80;

    listColumns[4] = gcnew SWF::ColumnHeader ();
    listColumns[4]->Text = GeoCoordinateLocalization::GetLocalizedString ("EPSG");
    listColumns[4]->Width = (nullptr != columnWidths) ? columnWidths[4] : 80;

    listColumns[5] = gcnew SWF::ColumnHeader ();
    listColumns[5]->Text = GeoCoordinateLocalization::GetLocalizedString ("Datum");
    listColumns[5]->Width = (nullptr != columnWidths) ? columnWidths[5] : 80;

    listColumns[6] = gcnew SWF::ColumnHeader ();
    listColumns[6]->Text = GeoCoordinateLocalization::GetLocalizedString ("Ellipsoid");
    listColumns[6]->Width = (nullptr != columnWidths) ? columnWidths[6] : 80;

    if (nullptr != columnOrder)
        {
        for (int iColumn = 0; iColumn < listColumns->Length; iColumn++)
            listColumns[iColumn]->DisplayIndex = columnOrder[iColumn];
        }

    columns->AddRange (listColumns);

    // when we become visible, focus into text item.
    // VisibleChanged                          += gcnew System::EventHandler (this, &SearchSelectionControl::HandleVisibleChanged);

    SuspendLayout();
    Dock    = SWF::DockStyle::Fill;
    Size    = SD::Size (PANEL_W, PANEL_H);
    Controls->Add (m_listView);
    Controls->Add (m_propertiesButton);
    Controls->Add (m_addFavoriteButton);
    Controls->Add (m_allWordsButton);
    Controls->Add (m_anyWordButton);
    Controls->Add (m_searchLabel);
    Controls->Add (m_searchTextField);
    Controls->Add (m_searchButton);
    Controls->Add (m_stopSearchButton);
    Controls->Add (m_progressBar);
    ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            EnableItems
(
)
    {
    String^     contents;
    m_searchButton->Enabled = (nullptr != (contents = m_searchTextField->Text)) && (contents->Length > 0) && !contents->Equals(m_lastSearchText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            StartSearch
(
)
    {
    // just ignore if already busy. Don't want the exception telling us that the background worker is busy.
    if (m_backgroundWorker->IsBusy)
        return;

    // do the search in the backgroundWorker thread object.
    m_lastSearchText = m_searchTextField->Text;

    // if there's nothing to search for, just clear the list view
    if ( (nullptr == m_lastSearchText) || (0 == m_lastSearchText->Length) )
        {
        m_listView->Items->Clear();
        return;
        }

    // set up the wait cursor
    m_stopSearchButton->Enabled         = true;
    m_stopSearchButton->Focus();
    m_progressBar->Visible              = true;

    SWF::Cursor::Current                = SWF::Cursors::WaitCursor;
    m_searchTextField->Cursor           = SWF::Cursors::WaitCursor;
    m_anyWordButton->Enabled            = false;
    m_allWordsButton->Enabled           = false;
    m_searchButton->Enabled             = false;

    m_backgroundWorker->RunWorkerAsync (m_lastSearchText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ShowProgress
(
unsigned int        percentComplete,
System::Object^     arg
)
    {
    m_progressBar->Value = percentComplete;
    }

delegate void   ShowProgressDelegate (unsigned int percentComplete, System::Object^ userObject);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ReportProgress
(
unsigned int        percentComplete,
System::Object^     arg
)
    {
    if (m_progressBar->InvokeRequired)
        {
        m_progressBar->Invoke (gcnew ShowProgressDelegate (this, &SearchSelectionControl::ShowProgress), gcnew array<System::Object^> { percentComplete, arg });
        }
    else
        ShowProgress (percentComplete, arg);

    return m_backgroundWorker->CancellationPending;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DoSearchInBackground
(
System::Object^         sender,
SCM::DoWorkEventArgs^   args
)
    {
    if (nullptr == m_librarySearcher)
        m_librarySearcher = gcnew LibrarySearcher();

    String^                 searchString    = safe_cast<String^>(args->Argument);
    array<System::Char>^    separators      = {' '};
    array<String^>^         searchStrings   = searchString->Split (separators);
    SCG::List<BaseGCS^>^    foundGCSs = m_librarySearcher->FindInLibrary (searchStrings, m_anyWord, gcnew SearchProgressDelegate (this, &SearchSelectionControl::ReportProgress), nullptr);
    args->Result = foundGCSs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClearResults
(
)
    {
    if (nullptr == m_listView)
        return;

    for each (SWF::ListViewItem^ item in m_listView->Items)
        {
        BaseGCS^ gcs;
        if (nullptr != (gcs = dynamic_cast <BaseGCS^>(item->Tag)))
            {
            delete gcs;
            item->Tag = nullptr;
            }
        }

    m_listView->Items->Clear();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SearchCompleted
(
System::Object^                     sender,
SCM::RunWorkerCompletedEventArgs^   args
)
    {
    if ( (!args->Cancelled) && (nullptr != args->Result) )
        {
        // the array of found GCSs is in args->Result
        SCG::List<BaseGCS^>^ foundGCSs = safe_cast<SCG::List<BaseGCS^>^>(args->Result);

        // NOTE: I spent quite a while trying to get a WaitCursor to come up while in this loop. I tried using
        // setting SWF::Cursor::Current to SWF::Cursors::WaitCursor, setting FindForm()->Cursor to SWF::Cursors::WaitCursor
        // and a few other things. I found that if I set m_listView->Cursor to SWF::Cursors::WaitCursor, and didn't set it
        // back, it would change to the wait cursor AFTER the loop was over. That doesn't do much good. I think the problem is
        // probably that I am handling an event, so I'm in the Windows thread. You probably have to start another thread to do
        // the work, but the documentation is not very useful in explaining how to do that.

        // Update:  I changed to using a "BackgroundWorker" object to do the actual search (this function is now the handler
        //  for the RunWorkerCompleted event of that background worker). That sort of works, although as soon as you move
        //  the cursor it reverts to the standard cursor.

        // add all of the foundGCSs to the ListView
        // m_listView->Cursor = SWF::Cursors::WaitCursor;
        m_listView->BeginUpdate();
        ClearResults();

        for each (BaseGCS^ gcs in foundGCSs)
            {
            if ( (nullptr != m_gcsFilter) && !m_gcsFilter (gcs) )
                continue;

            int     epsgCode = gcs->GetEPSGCode(true);

            SWF::ListViewItem^          item = gcnew SWF::ListViewItem (gcs->Name);
            array<SWF::ListViewItem::ListViewSubItem^>^  itemArray = gcnew array<SWF::ListViewItem::ListViewSubItem^>(6);
            itemArray[0] = gcnew SWF::ListViewItem::ListViewSubItem (item, gcs->Description);
            itemArray[1] = gcnew SWF::ListViewItem::ListViewSubItem (item, gcs->Units);
            itemArray[2] = gcnew SWF::ListViewItem::ListViewSubItem (item, gcs->Projection);
            itemArray[3] = gcnew SWF::ListViewItem::ListViewSubItem (item, (0 != epsgCode) ? String::Format ("{0}", epsgCode) : String::Empty);
            itemArray[4] = gcnew SWF::ListViewItem::ListViewSubItem (item, gcs->DatumName);
            itemArray[5] = gcnew SWF::ListViewItem::ListViewSubItem (item, gcs->EllipsoidName);
            item->Tag    = gcs;
            item->SubItems->AddRange (itemArray);
            m_listView->Items->Add (item);
            }
        m_listView->EndUpdate();
        }

    SWF::Cursor::Current                = SWF::Cursors::Default;
    m_searchTextField->Cursor           = SWF::Cursors::Default;

    m_anyWordButton->Enabled            = true;
    m_allWordsButton->Enabled           = true;
    m_searchButton->Enabled             = true;
    m_searchButton->Focus();
    m_stopSearchButton->Enabled         = false;

    // hide the progress bar.
    m_progressBar->Visible      = false;

    EnableItems();
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
GCSSelectionPanel::~GCSSelectionPanel
(
)
    {
    if (nullptr != m_searchSelectionControl)
        m_searchSelectionControl->SaveSettings ();

    if (nullptr != m_libraryPage)
        {
        delete m_libraryPage;
        m_libraryPage = nullptr;
        }
    if (nullptr != m_searchPage)
        {
        delete m_searchPage;
        m_searchPage = nullptr;
        }
    if (nullptr != m_tabControl)
        {
        delete m_tabControl;
        m_tabControl = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GCSSelectionPanel::InitializeComponent
(
BaseGCS^                initialSelection,
String^                 favoritesFileList,       // files that contain OrganizationClass serialized ECXML instances.
SWF::ImageList^         imageList,
bool                    createFavoritesIfNeeded,
bool                    allowOrganizationModification,
bool                    allowUserLibraryModification,
GCSFilterDelegate^      gcsFilter,
ISearchSettingsSaver^   searchSettingsSaver
)
    {
    m_okButton                      = gcnew SWF::Button();
    m_cancelButton                  = gcnew SWF::Button();

    m_okButton->DialogResult        = SWF::DialogResult::OK;
    m_okButton->Location            = SD::Point(20, BTN_Y);
    m_okButton->Size                = SD::Size(96, 24);
    m_okButton->TabIndex            = 10;
    m_okButton->Text                = GeoCoordinateLocalization::GetLocalizedString ("Ok");

    m_cancelButton->DialogResult    = SWF::DialogResult::Cancel;
    m_cancelButton->Location        = SD::Point(130, BTN_Y);
    m_cancelButton->Size            = SD::Size(96, 24);
    m_cancelButton->TabIndex        = 11;
    m_cancelButton->Text            = GeoCoordinateLocalization::GetLocalizedString ("Cancel");

    m_hostForm->AcceptButton        = m_okButton;
    m_hostForm->CancelButton        = m_cancelButton;

    m_bottomPanel                   = gcnew SWF::Panel();
    m_bottomPanel->Size             = SD::Size(PANELWIDTH, PANELHEIGHT);
    m_bottomPanel->Dock             = SWF::DockStyle::Bottom;

    m_bottomPanel->SuspendLayout();
    m_bottomPanel->Controls->Add (m_okButton);
    m_bottomPanel->Controls->Add (m_cancelButton);
    m_bottomPanel->ResumeLayout();

    m_libraryPage                   = gcnew SWF::TabPage();
    m_libraryPage->Text             = GeoCoordinateLocalization::GetLocalizedString ("Library");
    m_libraryPage->Dock             = SWF::DockStyle::Fill;
    m_libraryPage->TabIndex         = 0;
    m_librarySelectionControl       = gcnew LibrarySelectionControl (initialSelection, favoritesFileList, imageList, createFavoritesIfNeeded, allowOrganizationModification, allowUserLibraryModification, this, gcsFilter);
    m_librarySelectionControl->Dock = SWF::DockStyle::Fill;
    m_libraryPage->Controls->Add (m_librarySelectionControl);

    m_searchPageAcceptButton        = gcnew SWF::Button();
    m_searchPage                    = gcnew SWF::TabPage();
    m_searchPage->Text              = GeoCoordinateLocalization::GetLocalizedString ("Search");
    m_searchPage->Dock              = SWF::DockStyle::Fill;
    m_searchPage->TabIndex          = 2;
    m_searchSelectionControl        = gcnew SearchSelectionControl (m_searchPageAcceptButton, this, gcsFilter, searchSettingsSaver);
    m_searchPage->Controls->Add (m_searchSelectionControl);

    m_tabControl                    = gcnew SWF::TabControl();
    m_tabControl->Location          = SD::Point (0, 0);
    m_tabControl->SelectedIndex     = 0;
    m_tabControl->TabIndex          = 0;
    m_tabControl->Dock              = SWF::DockStyle::Fill;
    m_tabControl->Selected          += gcnew SWF::TabControlEventHandler (this, &GCSSelectionPanel::TabPageChanged);
    m_tabControl->SuspendLayout();
    m_tabControl->Controls->Add (m_libraryPage);
    m_tabControl->Controls->Add (m_searchPage);
    m_tabControl->ResumeLayout();

    SuspendLayout();
    Controls->Add (m_tabControl);
    Controls->Add (m_bottomPanel);
    ResumeLayout();
    Size = SD::Size (750,600);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GCSSelectionPanel::WriteOrganizationFiles
(
)
    {
    m_librarySelectionControl->WriteOrganizationFiles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GCSSelectionPanel::AddGCSToFavorites
(
BaseGCS^    gcs
)
    {
    m_librarySelectionControl->AddGCSToFavorites (gcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemberTreeNode::InitializeImageKeys
(
)
    {
    ImageKey            = GCSSelectionPanel::GCSImageKey;
    SelectedImageKey    = GCSSelectionPanel::GCSImageKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GroupTreeNode::InitializeImageKeys
(
)
    {
    ImageKey            = GCSSelectionPanel::GroupImageKey;
    SelectedImageKey    = GCSSelectionPanel::GroupImageKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            FileTreeNode::InitializeImageKeys
(
)
    {
    String^ key = (NULL == m_library) ? GCSSelectionPanel::LibraryImageKey : GCSSelectionPanel::FavoritesImageKey;

    ImageKey            = key;
    SelectedImageKey    = key;
    }


public enum class  UnitSystem
    {
    None    = 0,
    English = 1,
    Metric  = 2,
    };

public enum class  UnitBase
    {
    None    = 0,
    Meter   = 1,
    Degree  = 2,
    };

/*=================================================================================**//**
* Unit managed class
* @bsiclass                                                     Barry.Bentley   03/08
+===============+===============+===============+===============+===============+======*/
public ref class Unit
{
internal:

CSUnitInfo const*   m_csUnit;

// constructor is private. Use FindUnit method
Unit (int index)
    {
    m_csUnit = BGC::CSMap::GetCSUnitInfo (index);
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
static Unit^ FindUnit (String^ unitName)
    {
    const struct cs_Unittab_   *pUnit;
    int                         index;
    BI::ScopedString name (unitName);
    char* const unitNameCP = name.Ansi();
    for (index=0, pUnit = BGC::CSMap::GetCSUnitInfo (0); cs_UTYP_END != pUnit->type; pUnit++, index++)
        {
        if (0 == strcmpi (unitNameCP, pUnit->name))
            return gcnew Unit (index);
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Name { String^ get() {return gcnew String (m_csUnit->name);} }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ PluralName { String^ get() {return gcnew String (m_csUnit->pluralName);} }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property String^ Abbreviation { String^ get() {return gcnew String (m_csUnit->abrv);} }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property UnitSystem System
{
UnitSystem get()
    {
    if (cs_USYS_Metric == m_csUnit->system)
        return UnitSystem::Metric;
    else if (cs_USYS_English == m_csUnit->system)
        return UnitSystem::English;
    return UnitSystem::None;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property UnitBase   Base
{
UnitBase get ()
    {
    if (cs_UTYP_LEN == m_csUnit->type)
        return UnitBase::Meter;
    else if (cs_UTYP_ANG == m_csUnit->type)
        return UnitBase::Degree;
    return UnitBase::None;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property int   EPSGCode { int get() { return m_csUnit->epsgCode; } }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property double ConversionFactor { double get() { return m_csUnit->factor; } }

}; // end of Unit

/*=================================================================================**//**
* Unit enumeration class.
+===============+===============+===============+===============+===============+======*/
public ref class   UnitEnumerator
{
private:
int     m_currentIndex;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
UnitEnumerator () { m_currentIndex = -1; }


/*---------------------------------------------------------------------------------**//**
* Moves to the next unit
* @return   true if successful in moving to the next Unit, false if there are no more.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MoveNext ()
    {
    if (m_currentIndex < -1)
        return false;

    m_currentIndex++;
    CSUnitInfo const* unitInfo = BGC::CSMap::GetCSUnitInfo(m_currentIndex);
    if ( (cs_UTYP_END != unitInfo->type) && (cs_UTYP_OFF != unitInfo->type) )
        return true;

    // set up for repeated failures.
    m_currentIndex = -2;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property Unit^   Current
{
Unit^ get ()
    {
    if (m_currentIndex < 0)
        return nullptr;

    return gcnew Unit(m_currentIndex);
    }
}

}; // end of UnitEnumerator class



/*=================================================================================**//**
* Ellipsoid enumeration class.
+===============+===============+===============+===============+===============+======*/
public ref class   EllipsoidEnumerator
{
private:
int         m_currentIndex;
String^     m_currentEllipsoidName;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
public:
EllipsoidEnumerator ()
    {
    m_currentIndex = -1;
    }

/*---------------------------------------------------------------------------------**//**
* Moves to the next Ellipsoid
* @return   true if successful in moving to the next Ellipsoid, false if there are no more.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool              MoveNext()
    {
    if (m_currentIndex < -1)
        return false;

    m_currentIndex++;
    char    currentEllipsoidName[64];
    if (1 == BGC::CSMap::CS_elEnum (m_currentIndex, currentEllipsoidName, _countof (currentEllipsoidName)))
        {
        m_currentEllipsoidName = gcnew String (currentEllipsoidName);
        return true;
        }

    // set up for repeated failures.
    m_currentIndex = -2;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Gets the current Ellipsoid.
* @return   the current Ellipsoid.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property Ellipsoid^ Current {Ellipsoid^ get() { return gcnew Ellipsoid (m_currentEllipsoidName); } }

};


/*=================================================================================**//**
* Datum enumeration class.
+===============+===============+===============+===============+===============+======*/
public ref class   DatumEnumerator
{
private:
int         m_currentIndex;
String^     m_currentDatumName;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
public:
DatumEnumerator ()
    {
    m_currentIndex = -1;
    }

/*---------------------------------------------------------------------------------**//**
* Moves to the next Datum
* @return   true if successful in moving to the next Datum, false if there are no more.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool              MoveNext()
    {
    if (m_currentIndex < -1)
        return false;

    m_currentIndex++;
    char    currentDatumName[64];
    if (1 == BGC::CSMap::CS_dtEnum (m_currentIndex, currentDatumName, _countof (currentDatumName)))
        {
        m_currentDatumName = gcnew String (currentDatumName);
        return true;
        }

    // set up for repeated failures.
    m_currentIndex = -2;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Gets the current Datum.
* @return   the current Datum.
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
property Datum^ Current {Datum^ get() { return gcnew Datum (m_currentDatumName); } }

};

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   12/06
+===============+===============+===============+===============+===============+======*/
public ref class    BaseSelectorForm : IHostFormProvider2
{
private:
GCSSelectionPanel^      m_selectionPanel;
SWF::Form^              m_form;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
BaseSelectorForm
(
String^     title,
BaseGCS^    initialCoordSys,
String^     favoritesFileList       // files that contain OrganizationClass serialized ECXML instances.
)
    {
    m_form = gcnew SWF::Form();

    m_selectionPanel = gcnew GCSSelectionPanel (initialCoordSys, favoritesFileList, m_form, nullptr, true, false, false, this, nullptr);
    m_selectionPanel->Dock = SWF::DockStyle::Fill;

    m_form->Name    = "GCSDialog";
    m_form->Text    = title;
    m_form->SuspendLayout();
    m_form->Controls->Add (m_selectionPanel);
    m_form->Size = SD::Size (750,700);
    m_form->ResumeLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
property BaseGCS^        SelectedGCS
    {
    BaseGCS^ get() {return m_selectionPanel->SelectedGCS;}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveFavorites()
    {
    // write the group organization (if it has changed).
    m_selectionPanel->WriteOrganizationFiles();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/07
+---------------+---------------+---------------+---------------+---------------+------*/
SWF::DialogResult   ShowDialog()
    {
    SWF::DialogResult result = m_form->ShowDialog();
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
~BaseSelectorForm ()
    {
    delete m_form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetEditPropertiesForm ()
    {
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetShowPropertiesForm ()
    {
    SWF::Form^ form  = gcnew SWF::Form();
    form->Name          = "GCSProperties";
    form->Text          = GeoCoordinateLocalization::GetLocalizedString ("GCSProperties");
    return form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetGroupPropertiesForm ()
    {
    SWF::Form^ form  = gcnew SWF::Form();
    form->Name          = "GCSGroupProperties";
    form->Text          = GeoCoordinateLocalization::GetLocalizedString ("GCSGroupProperties");
    return form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetEditDatumsForm ()
    {
    SWF::Form^ form  = gcnew SWF::Form();
    form->Name       = "EditDatums";
    form->Text       = GeoCoordinateLocalization::GetLocalizedString ("EditDatums");
    return form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetEditDatumForm ()
    {
    SWF::Form^ form  = gcnew SWF::Form();
    form->Name       = "EditDatum";
    form->Text       = GeoCoordinateLocalization::GetLocalizedString ("EditDatums");
    return form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetEditEllipsoidsForm ()
    {
    SWF::Form^ form  = gcnew SWF::Form();
    form->Name       = "EditEllipsoids";
    form->Text       = GeoCoordinateLocalization::GetLocalizedString ("EditEllipsoids");
    return form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::Form^  GetEditEllipsoidForm ()
    {
    SWF::Form^ form  = gcnew SWF::Form();
    form->Name       = "EditEllipsoid";
    form->Text       = GeoCoordinateLocalization::GetLocalizedString ("EditEllipsoid");
    return form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual SWF::DialogResult   ShowModal (SWF::Form^ form)
    {
    return form->ShowDialog();
    }

};

#include "DatumEditingPanel.cpp"
#include "EllipsoidEditingPanel.cpp"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            LibrarySelectionControl::EditDatumsClickEvent (System::Object^ sender, System::EventArgs^ eventArgs)
    {
    // get node and make sure it's a user library (it should be)
    IHostFormProvider2^ host;
    if (nullptr == (host = dynamic_cast <IHostFormProvider2^>(m_gcsSelectionPanel->HostFormProvider)))
        return;

    if (nullptr == m_treeView)
        return;

    FileTreeNode^       fileNode;
    if (nullptr == (fileNode = dynamic_cast<FileTreeNode^>(m_treeView->SelectedNode)))
        return;

    BGC::LibraryP       library;
    if (NULL == (library = fileNode->UserLibrary))
        return;

    SWF::Form^          form      = host->GetEditDatumsForm();
    DatumEditingPanel^  panel     = gcnew DatumEditingPanel (form, host, nullptr, library, m_gcsSelectionPanel->UserOrganizationName);
    form->SuspendLayout();
    form->Controls->Add (panel);
    form->ResumeLayout();
    host->ShowModal (form);

    // if we have edited any of the Datums or Ellipsoids, we need to refresh all the GCS's in our tree, because it is possible that the Datum or Ellipsoid parameters are stale.
    if (panel->NeedGCSReload)
        {
        fileNode->RefreshAllGCS();
        m_propertyPane->ReloadInstanceLists();
        }

    delete form;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            LibrarySelectionControl::EditEllipsoidsClickEvent (System::Object^ sender, System::EventArgs^ eventArgs)
    {
    // get node and make sure it's a user library (it should be)
    IHostFormProvider2^ host;
    if (nullptr == (host = dynamic_cast <IHostFormProvider2^>(m_gcsSelectionPanel->HostFormProvider)))
        return;

    if (nullptr == m_treeView)
        return;

    FileTreeNode^       fileNode;
    if (nullptr == (fileNode = dynamic_cast<FileTreeNode^>(m_treeView->SelectedNode)))
        return;

    BGC::LibraryP   library;
    if (NULL == (library = fileNode->UserLibrary))
        return;

    SWF::Form^              form      = host->GetEditEllipsoidsForm();
    EllipsoidEditingPanel^  panel     = gcnew EllipsoidEditingPanel (form, host, nullptr, library, m_gcsSelectionPanel->UserOrganizationName);
    form->SuspendLayout();
    form->Controls->Add (panel);
    form->ResumeLayout();
    host->ShowModal (form);

    if (panel->NeedGCSReload)
        {
        fileNode->RefreshAllGCS();
        m_propertyPane->ReloadInstanceLists();
        }

    delete form;
    }

}   // End of GeocoordinatesNET Namespace

END_BENTLEY_NAMESPACE

namespace BGC   = Bentley::GeoCoordinates;
namespace BGCN  = Bentley::GeoCoordinatesNET;
namespace SWF   = System::Windows::Forms;

/*=================================================================================**//**
* This is an unmanaged class compiled using the CLR, designed to give native apps access
*  the GCS Selection GUI.
+===============+===============+===============+===============+===============+======*/
BGC::BaseGCSP   BGC::GCSSelectFromLibrary::OpenSelectorDialog (BGC::BaseGCSP initialGCSP, WCharCP dialogTitle, WCharCP favoritesFileList)
    {
    BGCN::BaseGCS^          initialCoordSys = (NULL == initialGCSP) ? nullptr : gcnew BGCN::BaseGCS (initialGCSP);
    BGCN::BaseSelectorForm^ form            = gcnew BGCN::BaseSelectorForm (gcnew System::String (dialogTitle), initialCoordSys, gcnew System::String(favoritesFileList));
    SWF::DialogResult       result          = form->ShowDialog();

    // save the group favorites in the file, in case the user has changed it.
    form->SaveFavorites();

    // make an additional reference to form->SelectedGCS so it doesn't get GC'ed when we delete the form.
    BGCN::BaseGCS^    selected = form->SelectedGCS;

    // dispose of the form.
    delete form;

    System::IntPtr      selectedGCS (NULL);
    if ( (SWF::DialogResult::OK == result) && (nullptr != selected))
        selectedGCS = selected->Peer;

    return (BGC::BaseGCSP) selectedGCS.ToPointer();
    }



