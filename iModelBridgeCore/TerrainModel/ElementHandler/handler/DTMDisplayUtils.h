/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDisplayUtils.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTimeUtilities.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

bool GetViewVectorPoints (DTMDrawingInfo& drawingInfo, ViewContextR context, DTMPtr dtm, DPoint3d& startPt, DPoint3d& endPt);
bool GetUpdateRangeFromRange (ViewContextR viewContext, DTMUnitsConverter& conv, const ::DRange3d& range, ::DRange3d& updateRange, int pixelExpansion, bool allowHealingUpdate);
void DrawScanRange (ViewContextR context, ElementHandleCR el, const RefCountedPtr<DTMDataRef>& dtmDataRef);
DgnModelP GetDgnModel(ElementHandleCR elem);
DgnModelRefP GetModelRef(ElementHandleCR elem);
int elemUtil_setRange (Bentley::DgnPlatform::ScanRange* range, DRange3d const* rangeVec, bool setZ, bool is3d);
void setStorageToUORMatrix (const Transform& trsf, EditElementHandleR el);
void getStorageToUORMatrix (Transform& trsf, ElementHandleCR el);
void getStorageToUORMatrix (Transform& trsf, DgnModelRefP model, ElementHandleCR el, bool withExaggeration = true);
void RedrawElement (ElementHandleR elemHandle);

bool DoProgressiveDraw(RefCountedPtr<DTMDataRef>& ref, ViewContextP viewContext);

#ifndef MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_HIGH_QUALITY_DISPLAY 
    #define MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_HIGH_QUALITY_DISPLAY 0.25
#endif

#ifndef MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_DEFAULT_DISPLAY
    #define MRDTM_MIN_SCREEN_PIXELS_PER_DRAPE_PIXEL_IN_DEFAULT_DISPLAY 1.0
#endif

IAnnotationHandlerP GetSharedAnnotationHandler ();

// In TerrainModelElementHandler
DTMELEMENT_EXPORT DgnModelRefP GetActivatedModel (ElementHandleCR elem, ViewContextCP context);

#ifndef NDEBUG
extern bool s_logInfo;
extern bool s_logDebug;

// Logging functions
static inline bool HasInfoLog () { return s_logInfo; }
static inline bool HasDebugLog () { return s_logDebug; }

void LogInfo(const wchar_t* message);
void LogInfoV(const wchar_t* message ...);
void LogDebug(const wchar_t* message);
void LogDebugV(const wchar_t* message ...);

class LogTimeInfoHelper
    {
    WString m_message;
    UInt32 m_took;
    public:
    LogTimeInfoHelper(wchar_t* message)
        {
        m_message = message;
        m_message += L" Took(%d)";
        m_took = BeTimeUtilities::QueryMillisecondsCounterUInt32();
        }
    ~LogTimeInfoHelper()
        {
        m_took = BeTimeUtilities::QueryMillisecondsCounterUInt32() - m_took;
        LogInfoV(m_message.GetWCharCP(), m_took);
        }
    };

#define LogTimeInfo(message) LogTimeInfoHelper helper(message)

void DumpPointsInXYZfile(const DPoint3d* pointsP, int nbPoints, string& fileName, const Int64* indP);

void DumpDTMInTinFile(BcDTMP dtmP, wstring& fileName, const Int64* indP);

#else
#define LogInfo(...)
#define LogInfoV(...)
#define LogDebug(...)
#define LogDebugV(...)

#define LogTimeInfo(...)
#define HasInfoLog () false
#define HasDebugLog () false
#endif

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint2dR operator /= ( DPoint2dR pt, double d )
    {
    pt.x /= d;
    pt.y /= d;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3dR operator /= ( DPoint3dR pt, double d )
    {
    pt.x /= d;
    pt.y /= d;
    pt.z /= d;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint2dR operator *= ( DPoint2dR pt, double m )
    {
    pt.x *= m;
    pt.y *= m;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3dR operator *= ( DPoint3dR pt, double m )
    {
    pt.x *= m;
    pt.y *= m;
    pt.z *= m;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint2dR operator *= ( DPoint2dR pt, DPoint2dCR m )
    {
    pt.x *= m.x;
    pt.y *= m.y;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3dR operator += ( DPoint3dR pt, DPoint3dCR offset )
    {
    pt.x += offset.x;
    pt.y += offset.y;
    pt.z += offset.z;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3dR operator -= (DPoint3dR pt, DPoint3dCR offset)
    {
    pt.x -= offset.x;
    pt.y -= offset.y;
    pt.z -= offset.z;
    return pt;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3d operator + ( DPoint3dCR p1, DPoint3dCR p2 )
    {
    DPoint3d p = p1;
    p += p2;
    return p;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3d operator * ( DPoint3dCR p1, double m )
    {
    DPoint3d p = p1;
    p *= m;
    return p;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint2d operator / ( DPoint2dCR p1, double d )
    {
    DPoint2d p = p1;
    p /= d;
    return p;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3d operator / ( DPoint3dCR p1, double d )
    {
    DPoint3d p = p1;
    p /= d;
    return p;
    }

///// <author>Piotr.Slowinski</author>                            <date>5/2011</date>
inline DPoint3d operator / ( DPoint3dCR p1, DPoint3dCR p2 )
    {
    DPoint3d p = { p1.x / p2.x, p1.y / p2.y, p1.z / p2.z };
    return p;
    }

