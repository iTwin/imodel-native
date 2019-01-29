/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshDgn/ElementType.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#if defined(BEGIN_BENTLEY_NAMESPACE)
    #error This should be the first code that includes BeAssert.h
#endif
#pragma warning(disable:4312)
#pragma warning(disable:4091)
#pragma warning(disable:4273)
//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyG06".
#define NO_USING_NAMESPACE_BENTLEY 1

#include <DgnV8Api/Bentley/BeAssert.h>
#undef BeAssert
#undef BeDataAssert
#undef BeAssertOnce
#undef BeDataAssertOnce
#include <Bentley/BeAssert.h>

#define DGNV8_WSTRING_LEGACY_SUPPORT
#include <DgnV8Api/DgnPlatform/DgnFile.h>
#include <DgnV8Api/DgnPlatform/DgnPlatformLib.h>
#include <DgnV8Api/DgnPlatform/DgnFileIO/BentleyDgn.h>
#include <DgnV8Api/DgnPlatform/DgnECManager.h>
#include <DgnV8Api/DgnPlatform/DelegatedElementECEnabler.h>
#include <DgnV8Api/DgnPlatform\ElementGeometry.h>
#include <DgnV8Api/DgnPlatform\MeshHeaderHandler.h>

#include <vector>
#include <functional>
#include <numeric>

//From TerrainModel.h
enum class DTMFeatureType : uint32_t
    {
    None = 0,
    RandomSpots = 0,
    GroupSpots = 1,
    Spots = 2,
    DatPoint = 2, // Backwards Compatiability With XM
    TriangleIndex = 3,
    TriangleInfo = 4,
    FeatureSpot = 5,
    FlowArrow = 6,
    TinPoint = 15,
    TinLine = 16,
    DtmPoint = 17,
    Breakline = 10,
    SoftBreakline = 11,
    DrapeLine = 12,
    GraphicBreak = 13,
    ContourLine = 14,
    Void = 20,
    VoidLine = 30,
    Island = 21,
    Hole = 22,
    HoleLine = 32,
    BreakVoid = 25,
    DrapeVoid = 26,
    Hull = 23,
    HullLine = 24,
    TinHull = 27,
    DrapeHull = 28,
    Triangle = 101,
    TriangleEdge = 102,
    Lattice = 103,
    LatticeEdge = 104,
    LatticeXLine = 105,
    LatticeYLine = 106,
    LatticePoint = 107,
    Contour = 110,
    ZeroSlopeLine = 112,
    ZeroSlopePolygon = 113,
    ZeroSlopeTriangle = 114,
    ISOLine = 200,
    ISOCell = 201,
    Theme = 210,
    SlopeToe = 220,
    LowPoint = 400,
    HighPoint = 401,
    SumpLine = 402,
    RidgeLine = 403,
    DescentTrace = 404,
    AscentTrace = 405,
    Catchment = 406,
    CrossLine = 407,
    LowPointPond = 408,
    PondIsland = 409,
    ClkFlowLine = 410,
    FlowLine = 411,
    VisiblePoint = 415,
    InvisiblePoint = 416,
    VisibleLine = 420,
    InvisibleLine = 421,
    SlopeLine = 422,
    Polygon = 430,
    Region = 431,
    GradeSlope = 440,
    //PolyMeshCounts = 500, // Not Used
    //PolyMeshVertices = 501, // Not Used
    //PolyMeshFaces = 502, // Not Used
    TriangleMesh = 503,
    //LatticeMesh = 504, // Not Used
    CheckStop = 505,
    TriangleShadeMesh = 506,
    InroadsRectangle = 507,
    TriangleHillShadeMesh = 508,  // Should this juse be TriangleShadeMesh?
    };

namespace DgnV8Api = Bentley::DgnPlatform;

namespace Bentley
    {

    namespace DgnPlatform
        {
        typedef MSElementDescr&          MSElementDescrR;
        typedef MSElementDescr const&    MSElementDescrCR;
        typedef MSElementDescr*          MSElementDescrP;
        typedef MSElementDescr const*    MSElementDescrCP;
        }
    }
#include "ElementType.h"

namespace {

/*---------------------------------------------------------------------------------**//**
* @description  Functor transforming a 2D point to a valid 3D point
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct Transform2DTo3DPointFn : std::unary_function<DPoint3d, DPoint3d>
    {
    static DPoint3d Do (const DPoint2d& pt)
        {
        const DPoint3d outPt = {pt.x, pt.y, 0.0};
        return outPt;
        }

    DPoint3d operator () (const DPoint2d& pt)  const
        {
        return Do(pt);
        }
    };

// NTERAY: See FoundationsPrivateTools.h.
template<typename T>
bool                                    EqEps                          (T                               lhs,
                                                                        T                               rhs)
    { return fabs(lhs-rhs) < 10e-8; }



/*---------------------------------------------------------------------------------**//**
* @description  LINE_ELM
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElemUtils
    {
    static bool Is3d(DgnV8Api::MSElementDescrCP elmDescP) { return elmDescP->el.hdr.dhdr.props.b.is3d; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  LINE_ELM
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LineElemUtils
    {
    static bool IsSinglePoint(const DgnV8Api::Line_2d& line);
    static bool IsSinglePoint(const DgnV8Api::Line_3d& line);
    static bool IsSinglePoint(DgnV8Api::MSElementDescrCP elmDesc);

    static uint32_t CountPoints(DgnV8Api::MSElementDescrCP elmDesc);

    static DPoint3d* Copy(const DgnV8Api::Line_2d& line, DPoint3d* outIt);
    static DPoint3d* Copy(const DgnV8Api::Line_3d& line, DPoint3d* outIt);
    static DPoint3d* Copy(DgnV8Api::MSElementDescrCP elmDescP, DPoint3d* outIt);
    };

/*---------------------------------------------------------------------------------**//**
* @description  LINE_STRING_ELM
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LineStringElemUtils
    {
    static uint32_t CountPoints (MSElementDescrCP elmDesc);

    static DPoint3d* Copy(DgnV8Api::MSElementDescrCP elmDescP, DPoint3d* outIt);
    static DPoint3d* Copy(const DgnV8Api::Line_String_2d& lineString, DPoint3d* outIt);
    static DPoint3d* Copy(const DgnV8Api::Line_String_3d& lineString, DPoint3d* outIt);
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool LineElemUtils::IsSinglePoint(const DgnV8Api::Line_3d& line)
    {
    return EqEps(line.start.x, line.end.x) &&
           EqEps(line.start.y, line.end.y) &&
           EqEps(line.start.z, line.end.z);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool LineElemUtils::IsSinglePoint(const DgnV8Api::Line_2d& line)
    {
    return EqEps(line.start.x, line.end.x) &&
           EqEps(line.start.y, line.end.y);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool LineElemUtils::IsSinglePoint(DgnV8Api::MSElementDescrCP elmDescP)
    {
    return ElemUtils::Is3d(elmDescP) ?
                IsSinglePoint(elmDescP->el.line_3d) :
                IsSinglePoint(elmDescP->el.line_2d);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t LineElemUtils::CountPoints(DgnV8Api::MSElementDescrCP elmDesc)
    {
    return IsSinglePoint(elmDesc) ? 1 : 2;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineElemUtils::Copy(DgnV8Api::MSElementDescrCP elmDescP, DPoint3d* outIt)
    {
    return ElemUtils::Is3d(elmDescP) ?
                Copy(elmDescP->el.line_3d, outIt) :
                Copy(elmDescP->el.line_2d, outIt);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineElemUtils::Copy(const DgnV8Api::Line_3d& line, DPoint3d* outIt)
    {
    *outIt++ = line.start;
    if (!IsSinglePoint(line))
        *outIt++ = line.end;
    return outIt;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineElemUtils::Copy(const DgnV8Api::Line_2d& line, DPoint3d* outIt)
    {
    *outIt++ = Transform2DTo3DPointFn::Do(line.start);
    if (!IsSinglePoint(line))
        *outIt++ = Transform2DTo3DPointFn::Do(line.end);
    return outIt;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t LineStringElemUtils::CountPoints(DgnV8Api::MSElementDescrCP elmDescP)
    {
    return elmDescP->el.line_string_3d.numverts;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineStringElemUtils::Copy(DgnV8Api::MSElementDescrCP elmDescP, DPoint3d* outIt)
    {
    return ElemUtils::Is3d(elmDescP) ?
                Copy(elmDescP->el.line_string_3d, outIt) :
                Copy(elmDescP->el.line_string_2d, outIt);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineStringElemUtils::Copy(const DgnV8Api::Line_String_2d& lineString, DPoint3d* outIt)
    {
    return std::transform(lineString.vertice,
                          lineString.vertice + lineString.numverts,
                          outIt,
                          Transform2DTo3DPointFn());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineStringElemUtils::Copy(const DgnV8Api::Line_String_3d& lineString, DPoint3d* outIt)
    {
    return std::copy(lineString.vertice,
                     lineString.vertice + lineString.numverts,
                     outIt);
    }


/*---------------------------------------------------------------------------------**//**
* @description  For all unsupported element types
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class UnsupportedElemPtExtractor : public ElementPointExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDesc, ElementPointStats& stats) const override { /* Do nothing */ }
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<DPoint3d>& pointArray) const override { return SUCCESS; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  For all unsupported element types
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class UnsupportedElemLinExtractor : public ElementLinearExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDesc, ElementLinearStats& stats) const override { /* Do nothing */ }
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDesc, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, unsigned int featureType, size_t capacity) const override { return SUCCESS; }
    };


/*---------------------------------------------------------------------------------**//**
* @description  For all unsupported element types
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class UnsupportedElemMeshExtractor : public ElementMeshExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDesc, ElementMeshStats& stats) const override { /* Do nothing */ }
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDesc, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, size_t capacity) const override { return SUCCESS; }
    };



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElemPtExtractor : public ElementPointExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDescP, ElementPointStats& stats) const override
        {
        if (!LineElemUtils::IsSinglePoint(elmDescP))
            return;

        ++stats.m_pointCount;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem3dPtExtractor : public LineElemPtExtractor
    {
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<DPoint3d>& pointArray) const override
        {
        if(!LineElemUtils::IsSinglePoint(elmDescP->el.line_3d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if(pointArray.size() + 1 > pointArray.capacity())
            return ERROR;

        const DPoint3d point(elmDescP->el.line_3d.start);
        pointArray.insert(pointArray.end(),&point, &point + 1);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem2dPtExtractor : public LineElemPtExtractor
    {
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<DPoint3d>& pointArray) const override
        {
        if(!LineElemUtils::IsSinglePoint(elmDescP->el.line_2d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if(pointArray.size() + 2 > pointArray.capacity())
            return ERROR;

        const DPoint3d point(Transform2DTo3DPointFn::Do(elmDescP->el.line_2d.start));
        pointArray.insert(pointArray.end(), &point,&point + 1);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElemLinExtractor : public ElementLinearExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDescP, ElementLinearStats& stats) const override
        {
        if (LineElemUtils::IsSinglePoint(elmDescP))
            return;

        stats.SetMaxPointQty(2);
        stats.m_pointCount += 2;
        ++stats.m_featureCount;
        }
    };

size_t GetTotalPointQty(bvector<bvector<DPoint3d>>& featureArray)
    {
    return std::accumulate(featureArray.begin(), featureArray.end(), (size_t)0, [] (const size_t& sum, const bvector<DPoint3d>& element) { return sum + element.size(); });
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem3dLinExtractor : public LineElemLinExtractor
    {
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, unsigned int featureType, size_t capacity) const override
        {
        if(LineElemUtils::IsSinglePoint(elmDescP->el.line_3d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.size() >= featureArray.capacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if (GetTotalPointQty(featureArray) + 2 > capacity)
            return ERROR;

        const DPoint3d points[] = {elmDescP->el.line_3d.start, elmDescP->el.line_3d.end};
        bvector<DPoint3d> pts(2);
        pts.push_back(points[0]);
        pts.push_back(points[1]);
        featureArray.push_back(pts);
        typeArray.push_back(featureType);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem2dLinExtractor : public LineElemLinExtractor
    {
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, unsigned int featureType, size_t capacity) const override
        {
        if(LineElemUtils::IsSinglePoint(elmDescP->el.line_2d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.size() >= featureArray.capacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if (GetTotalPointQty(featureArray) + 2 > capacity)
            return ERROR;

        const DPoint3d points[] = {Transform2DTo3DPointFn::Do(elmDescP->el.line_2d.start),
                                   Transform2DTo3DPointFn::Do(elmDescP->el.line_2d.end)};
        bvector<DPoint3d> pts(2);
        pts.push_back(points[0]);
        pts.push_back(points[1]);
        featureArray.push_back(pts);
        typeArray.push_back(featureType);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineStringElemLinExtractor : public ElementLinearExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDescP, ElementLinearStats& stats) const override
        {
        uint32_t nbPts = LineStringElemUtils::CountPoints(elmDescP);

        stats.SetMaxPointQty(nbPts);
        stats.m_pointCount += nbPts;
        ++stats.m_featureCount;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineStringElem3dLinExtractor : public LineStringElemLinExtractor
    {
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, unsigned int featureType, size_t capacity) const override
        {
        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.size() >= featureArray.capacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        size_t nbPts = elmDescP->el.line_string_3d.numverts;
        if (GetTotalPointQty(featureArray) + nbPts > capacity)
            return ERROR;


        bvector<DPoint3d> pts(elmDescP->el.line_string_3d.numverts);
        pts.assign(elmDescP->el.line_string_3d.vertice, elmDescP->el.line_string_3d.vertice + elmDescP->el.line_string_3d.numverts);
        featureArray.push_back(pts);
        typeArray.push_back(featureType);
        
        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineStringElem2dLinExtractor : public LineStringElemLinExtractor
    {
    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, unsigned int featureType, size_t capacity) const override
        {
        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.size() >= featureArray.capacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        size_t nbPts = elmDescP->el.line_string_2d.numverts;
        if (GetTotalPointQty(featureArray) + nbPts > capacity)
            return ERROR;

        bvector<DPoint3d> points(LineStringElemUtils::CountPoints(elmDescP));
        LineStringElemUtils::Copy(elmDescP->el.line_string_2d, &points[0]);


        featureArray.push_back(points);
        typeArray.push_back(featureType);


        return SUCCESS;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ComplexElemLinExtractor : public ElementLinearExtractor
    {
    static uint32_t CountPoints(DgnV8Api::MSElementDescrCP elmDescP)
        {
        uint32_t pointCount = 0;

        for (DgnV8Api::MSElementDescrCP childCP = elmDescP->h.firstElem; NULL != childCP; childCP = childCP->h.next)
            {
            const int elemType = childCP->el.ehdr.type;
            if (DgnV8Api::LINE_ELM == elemType)
                pointCount += LineElemUtils::CountPoints(childCP);
            else if (DgnV8Api::LINE_STRING_ELM == elemType)
                pointCount += LineStringElemUtils::CountPoints(childCP);
            else if (DgnV8Api::SHAPE_ELM == elemType)
                pointCount += LineStringElemUtils::CountPoints(childCP);
            }
        return pointCount;
        }

    static bool IsSupported(DgnV8Api::MSElementDescrCP elmDescCP)
        {
        for(MSElementDescrCP pChild = elmDescCP->h.firstElem; NULL != pChild; pChild = pChild->h.next)
            {
            // Extract element type
            int elemType = pChild->el.ehdr.type;
            // Make sure element type is different from line or line WString (only linear types are supported for now)
            if ((elemType != DgnV8Api::LINE_ELM) && (elemType != DgnV8Api::LINE_STRING_ELM) && (elemType != DgnV8Api::SHAPE_ELM))
                return false;
            }
        return true;
        }

    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDescP, ElementLinearStats& stats) const override
        {
        if(!IsSupported(elmDescP))
            return;

        uint32_t nbPts = CountPoints(elmDescP);

        stats.SetMaxPointQty(nbPts);
        stats.m_pointCount += nbPts;
        ++stats.m_featureCount;
        }

    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, unsigned int featureType, size_t capacity) const override
        {
        // Verify if the complex shape contain only linear elements
        if(!IsSupported(elmDescP))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.size() >= featureArray.capacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        const uint32_t nbPts = CountPoints(elmDescP);
        if (GetTotalPointQty(featureArray) + nbPts > capacity)
            return ERROR;

        bvector<DPoint3d> points(nbPts);

        DPoint3d* ptOutIt(&points[0]);
        for (DgnV8Api::MSElementDescrCP childCP = elmDescP->h.firstElem; NULL != childCP; childCP = childCP->h.next)
            {
            if (childCP->el.ehdr.type == DgnV8Api::LINE_ELM)
                ptOutIt = LineElemUtils::Copy(childCP, ptOutIt);
            else if (childCP->el.ehdr.type == DgnV8Api::LINE_STRING_ELM)
                ptOutIt = LineStringElemUtils::Copy(childCP, ptOutIt);
            else if (childCP->el.ehdr.type == DgnV8Api::SHAPE_ELM)
                ptOutIt = LineStringElemUtils::Copy(childCP, ptOutIt);
            }


        featureArray.push_back(points);
        typeArray.push_back(featureType);


        return SUCCESS;
        }
    };
 

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshElemMeshExtractor : public ElementMeshExtractor
    {
    virtual void ComputeStats(DgnV8Api::MSElementDescrCP elmDescP, ElementMeshStats& stats) const override
        {
        size_t numPts = 0, numFacets = 0;
        PolyfaceHeaderPtr polyface;

        DgnV8Api::ElementHandle eh(elmDescP, false);

        if (SUCCESS == DgnV8Api::MeshHeaderHandler::PolyfaceFromElement(polyface, eh))
            {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*polyface, false);     // we just want points.
            visitor->SetNumWrap (1);     // so each face gets a wraparound (closure) point.
            for (visitor->Reset (); visitor->AdvanceToNextFace ();)
                {
                numFacets++;
                numPts += visitor->Point ().size ();
                }
            }
        // Some points can be duplicated if used in more than one shape, so this is not the exact count.
        stats.SetMaxPointQty(numPts * 2);
        stats.m_pointCount += numPts * 2;
        stats.m_featureCount += numFacets * 2;
        }

    virtual StatusInt Scan(DgnV8Api::MSElementDescrCP elmDescP, bvector<bvector<DPoint3d>>& featureArray, bvector<unsigned int>& typeArray, size_t capacity) const override
        {
        DgnV8Api::ElementHandle eh(elmDescP, false);

        PolyfaceHeaderPtr polyface;
        if (SUCCESS == DgnV8Api::MeshHeaderHandler::PolyfaceFromElement(polyface, eh))
            {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*polyface, false);     // we just want points.
            bvector<DPoint3d> &facetPoints = visitor->Point ();      // this array will be loaded up facet by facet
            visitor->SetNumWrap (1);     // so each face gets a wraparound (closure) point.
            for (visitor->Reset (); visitor->AdvanceToNextFace ();)
                {
                // Make sure that we will not exceed the allocated packet capacity for points.
                if (GetTotalPointQty(featureArray) + (int)facetPoints.size() > capacity)
                    return ERROR;                

                featureArray.push_back(facetPoints);
                typeArray.push_back((unsigned int)DTMFeatureType::TinPoint);

                featureArray.push_back(facetPoints);
                typeArray.push_back((unsigned int)DTMFeatureType::TinLine);                                
                }
            }
        return SUCCESS;
        }
    };
} // END unnamed namespace


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const ElementPointExtractor& ElementPointExtractor::GetFor(DgnV8Api::MSElementDescrCP elmDescP)
    {
    const int type = elmDescP->el.ehdr.type;
    if (DgnV8Api::LINE_ELM == type)
        {
        if (ElemUtils::Is3d(elmDescP))
            {
            static const LineElem3dPtExtractor LINE_3D_INSTANCE;
            return LINE_3D_INSTANCE;
            }

        static const LineElem2dPtExtractor LINE_2D_INSTANCE;
        return LINE_2D_INSTANCE;
        }

    static const UnsupportedElemPtExtractor UNSUPPORTED_INTANCE;
    return UNSUPPORTED_INTANCE;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const ElementLinearExtractor& ElementLinearExtractor::GetFor(DgnV8Api::MSElementDescrCP elmDescP)
    {
    const int type = elmDescP->el.ehdr.type;

    if (DgnV8Api::LINE_ELM == type)
        {
        if (ElemUtils::Is3d(elmDescP))
            {
            static const LineElem3dLinExtractor LINE_3D_INSTANCE;
            return LINE_3D_INSTANCE;
            }

        static const LineElem2dLinExtractor LINE_2D_INSTANCE;
        return LINE_2D_INSTANCE;
        }
    if (DgnV8Api::LINE_STRING_ELM == type || DgnV8Api::SHAPE_ELM == type)
        {
        if (ElemUtils::Is3d(elmDescP))
            {
            static const LineStringElem3dLinExtractor LINE_STRING_3D_INSTANCE;
            return LINE_STRING_3D_INSTANCE;
            }

        static const LineStringElem2dLinExtractor LINE_STRING_2D_INSTANCE;
        return LINE_STRING_2D_INSTANCE;
        }
    if (DgnV8Api::CMPLX_SHAPE_ELM == type || DgnV8Api::CMPLX_STRING_ELM == type)
        {
        static const ComplexElemLinExtractor COMPLEX_INSTANCE;
        return COMPLEX_INSTANCE;
        }

    static const UnsupportedElemLinExtractor UNSUPPORTED_INTANCE;
    return UNSUPPORTED_INTANCE;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const ElementMeshExtractor& ElementMeshExtractor::GetFor(DgnV8Api::MSElementDescrCP elmDescP)
    {
    const int type = elmDescP->el.ehdr.type;

    if (DgnV8Api::MESH_HEADER_ELM == type)
        {
        static const MeshElemMeshExtractor MESH_INSTANCE;
        return MESH_INSTANCE;
        }

    static const UnsupportedElemMeshExtractor UNSUPPORTED_INTANCE;
    return UNSUPPORTED_INTANCE;
    }

