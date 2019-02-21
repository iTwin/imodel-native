/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/ElementType.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include "ElementType.h"
#include <DgnPlatform\ElementGeometry.h>
#include <DgnPlatform\MeshHeaderHandler.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

namespace {

/*---------------------------------------------------------------------------------**//**
* @description  Functor transforming a 2D point to a valid 3D point
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct Transform2DTo3DPointFn : unary_function<DPoint3d, DPoint3d>
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
    { return HNumeric<T>::EQUAL_EPSILON(lhs, rhs); }



/*---------------------------------------------------------------------------------**//**
* @description  LINE_ELM
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElemUtils
    {
    static bool Is3d (MSElementDescrCP elmDescP) { return elmDescP->el.hdr.dhdr.props.b.is3d; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  LINE_ELM
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LineElemUtils
    {
    static bool IsSinglePoint(const Line_2d& line);
    static bool IsSinglePoint(const Line_3d& line);
    static bool IsSinglePoint(MSElementDescrCP elmDesc);

    static uint32_t CountPoints (MSElementDescrCP elmDesc);

    static DPoint3d* Copy (const Line_2d& line, DPoint3d* outIt);
    static DPoint3d* Copy (const Line_3d& line, DPoint3d* outIt);
    static DPoint3d* Copy (MSElementDescrCP elmDescP, DPoint3d* outIt);
    };

/*---------------------------------------------------------------------------------**//**
* @description  LINE_STRING_ELM
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct LineStringElemUtils
    {
    static uint32_t CountPoints (MSElementDescrCP elmDesc);

    static DPoint3d* Copy (MSElementDescrCP elmDescP, DPoint3d* outIt);
    static DPoint3d* Copy (const Line_String_2d& lineString, DPoint3d* outIt);
    static DPoint3d* Copy (const Line_String_3d& lineString, DPoint3d* outIt);
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool LineElemUtils::IsSinglePoint(const Line_3d& line)
    {
    return EqEps(line.start.x, line.end.x) &&
           EqEps(line.start.y, line.end.y) &&
           EqEps(line.start.z, line.end.z);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool LineElemUtils::IsSinglePoint(const Line_2d& line)
    {
    return EqEps(line.start.x, line.end.x) &&
           EqEps(line.start.y, line.end.y);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool LineElemUtils::IsSinglePoint(MSElementDescrCP elmDescP)
    {
    return ElemUtils::Is3d(elmDescP) ?
                IsSinglePoint(elmDescP->el.line_3d) :
                IsSinglePoint(elmDescP->el.line_2d);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t LineElemUtils::CountPoints (MSElementDescrCP elmDesc)
    {
    return IsSinglePoint(elmDesc) ? 1 : 2;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineElemUtils::Copy (MSElementDescrCP elmDescP, DPoint3d* outIt)
    {
    return ElemUtils::Is3d(elmDescP) ?
                Copy(elmDescP->el.line_3d, outIt) :
                Copy(elmDescP->el.line_2d, outIt);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineElemUtils::Copy (const Line_3d& line, DPoint3d* outIt)
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
DPoint3d* LineElemUtils::Copy (const Line_2d& line, DPoint3d* outIt)
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
inline uint32_t LineStringElemUtils::CountPoints (MSElementDescrCP elmDescP)
    {
    return elmDescP->el.line_string_3d.numverts;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineStringElemUtils::Copy (MSElementDescrCP elmDescP, DPoint3d* outIt)
    {
    return ElemUtils::Is3d(elmDescP) ?
                Copy(elmDescP->el.line_string_3d, outIt) :
                Copy(elmDescP->el.line_string_2d, outIt);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d* LineStringElemUtils::Copy(const Line_String_2d& lineString, DPoint3d* outIt)
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
DPoint3d* LineStringElemUtils::Copy (const Line_String_3d& lineString, DPoint3d* outIt)
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
    virtual void ComputeStats(MSElementDescrCP elmDesc, ElementPointStats& stats) const override { /* Do nothing */ }
    virtual StatusInt Scan (MSElementDescrCP elmDescP, HPU::Array<DPoint3d>& pointArray) const override { return SUCCESS; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  For all unsupported element types
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class UnsupportedElemLinExtractor : public ElementLinearExtractor
    {
    virtual void ComputeStats(MSElementDescrCP elmDesc, ElementLinearStats& stats) const override { /* Do nothing */ }
    virtual StatusInt Scan(MSElementDescrCP elmDesc, IDTMFeatureArray<DPoint3d>& featureArray, DTMFeatureType featureType) const override { return SUCCESS; }
    };

/*---------------------------------------------------------------------------------**//**
* @description  For all unsupported element types
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class UnsupportedElemMeshExtractor : public ElementMeshExtractor
    {
    virtual void ComputeStats(MSElementDescrCP elmDesc, ElementMeshStats& stats) const override { /* Do nothing */ }
    virtual StatusInt Scan(MSElementDescrCP elmDesc, IDTMFeatureArray<DPoint3d>& featureArray) const override { return SUCCESS; }
    };



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElemPtExtractor : public ElementPointExtractor
    {
    virtual void ComputeStats(MSElementDescrCP elmDescP, ElementPointStats& stats) const override
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
    virtual StatusInt Scan(MSElementDescrCP elmDescP, HPU::Array<DPoint3d>& pointArray) const override
        {
        if(!LineElemUtils::IsSinglePoint(elmDescP->el.line_3d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if(pointArray.GetSize() + 1 > pointArray.GetCapacity())
            return ERROR;

        const DPoint3d point(elmDescP->el.line_3d.start);
        pointArray.Append(&point, &point + 1);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem2dPtExtractor : public LineElemPtExtractor
    {
    virtual StatusInt Scan(MSElementDescrCP elmDescP, HPU::Array<DPoint3d>& pointArray) const override
        {
        if(!LineElemUtils::IsSinglePoint(elmDescP->el.line_2d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if(pointArray.GetSize() + 2 > pointArray.GetCapacity())
            return ERROR;

        const DPoint3d point(Transform2DTo3DPointFn::Do(elmDescP->el.line_2d.start));
        pointArray.Append(&point, &point + 1);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElemLinExtractor : public ElementLinearExtractor
    {
    virtual void ComputeStats(MSElementDescrCP elmDescP, ElementLinearStats& stats) const override
        {
        if (LineElemUtils::IsSinglePoint(elmDescP))
            return;

        stats.SetMaxPointQty(2);
        stats.m_pointCount += 2;
        ++stats.m_featureCount;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem3dLinExtractor : public LineElemLinExtractor
    {
    virtual StatusInt Scan(MSElementDescrCP elmDescP, IDTMFeatureArray<DPoint3d>& featureArray, DTMFeatureType featureType) const override
        {
        if(LineElemUtils::IsSinglePoint(elmDescP->el.line_3d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.GetSize() >= featureArray.GetCapacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if(featureArray.GetTotalPointQty() + 2 > featureArray.GetTotalPointCapacity())
            return ERROR;

        const DPoint3d points[] = {elmDescP->el.line_3d.start, elmDescP->el.line_3d.end};
        featureArray.Append((unsigned int)featureType, points, points + 2);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineElem2dLinExtractor : public LineElemLinExtractor
    {
    virtual StatusInt Scan(MSElementDescrCP elmDescP, IDTMFeatureArray<DPoint3d>& featureArray, DTMFeatureType featureType) const override
        {
        if(LineElemUtils::IsSinglePoint(elmDescP->el.line_2d))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.GetSize() >= featureArray.GetCapacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        if(featureArray.GetTotalPointQty() + 2 > featureArray.GetTotalPointCapacity())
            return ERROR;

        const DPoint3d points[] = {Transform2DTo3DPointFn::Do(elmDescP->el.line_2d.start),
                                   Transform2DTo3DPointFn::Do(elmDescP->el.line_2d.end)};
        featureArray.Append((unsigned int)featureType, points, points + 2);

        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineStringElemLinExtractor : public ElementLinearExtractor
    {
    virtual void ComputeStats(MSElementDescrCP elmDescP, ElementLinearStats& stats) const override
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
    virtual StatusInt Scan(MSElementDescrCP elmDescP, IDTMFeatureArray<DPoint3d>& featureArray, DTMFeatureType featureType) const override
        {
        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.GetSize() >= featureArray.GetCapacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        size_t nbPts = elmDescP->el.line_string_3d.numverts;
        if(featureArray.GetTotalPointQty() + nbPts > featureArray.GetTotalPointCapacity())
            return ERROR;

        featureArray.Append((unsigned int)featureType, elmDescP->el.line_string_3d.vertice,
                                             elmDescP->el.line_string_3d.vertice + elmDescP->el.line_string_3d.numverts);
        
        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class LineStringElem2dLinExtractor : public LineStringElemLinExtractor
    {
    virtual StatusInt Scan(MSElementDescrCP elmDescP, IDTMFeatureArray<DPoint3d>& featureArray, DTMFeatureType featureType) const override
        {
        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.GetSize() >= featureArray.GetCapacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        size_t nbPts = elmDescP->el.line_string_2d.numverts;
        if(featureArray.GetTotalPointQty() + nbPts > featureArray.GetTotalPointCapacity())
            return ERROR;

        vector<DPoint3d> points(LineStringElemUtils::CountPoints(elmDescP));
        LineStringElemUtils::Copy(elmDescP->el.line_string_2d, &points[0]);

        featureArray.Append((unsigned int)featureType, points.begin(), points.end());

        return SUCCESS;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class ComplexElemLinExtractor : public ElementLinearExtractor
    {
    static uint32_t CountPoints (MSElementDescrCP elmDescP)
        {
        uint32_t pointCount = 0;

        for(MSElementDescrCP childCP = elmDescP->h.firstElem; NULL != childCP; childCP = childCP->h.next)
            {
            const int elemType = childCP->el.ehdr.type;
            if (LINE_ELM == elemType)
                pointCount += LineElemUtils::CountPoints(childCP);
            else if (LINE_STRING_ELM == elemType)
                pointCount += LineStringElemUtils::CountPoints(childCP);
            else if (SHAPE_ELM == elemType)
                pointCount += LineStringElemUtils::CountPoints(childCP);
            }
        return pointCount;
        }

    static bool IsSupported(MSElementDescrCP elmDescCP)
        {
        for(MSElementDescrCP pChild = elmDescCP->h.firstElem; NULL != pChild; pChild = pChild->h.next)
            {
            // Extract element type
            int elemType = pChild->el.ehdr.type;
            // Make sure element type is different from line or line WString (only linear types are supported for now)
            if((elemType != LINE_ELM) && (elemType != LINE_STRING_ELM) && (elemType != SHAPE_ELM))
                return false;
            }
        return true;
        }

    virtual void ComputeStats (MSElementDescrCP elmDescP, ElementLinearStats& stats) const override
        {
        if(!IsSupported(elmDescP))
            return;

        uint32_t nbPts = CountPoints(elmDescP);

        stats.SetMaxPointQty(nbPts);
        stats.m_pointCount += nbPts;
        ++stats.m_featureCount;
        }

    virtual StatusInt Scan(MSElementDescrCP elmDescP, IDTMFeatureArray<DPoint3d>& featureArray, DTMFeatureType featureType) const override
        {
        // Verify if the complex shape contain only linear elements
        if(!IsSupported(elmDescP))
            return SUCCESS;

        // Make sure that we will not exceed the allocated packet capacity for headers.
        if(featureArray.GetSize() >= featureArray.GetCapacity())
            return ERROR;

        // Make sure that we will not exceed the allocated packet capacity for points.
        const uint32_t nbPts = CountPoints(elmDescP);
        if(featureArray.GetTotalPointQty() + nbPts > featureArray.GetTotalPointCapacity())
            return ERROR;

        vector<DPoint3d> points(nbPts);

        DPoint3d* ptOutIt(&points[0]);
        for(MSElementDescrCP childCP = elmDescP->h.firstElem; NULL != childCP; childCP = childCP->h.next)
            {
            if(childCP->el.ehdr.type == LINE_ELM)
                ptOutIt = LineElemUtils::Copy(childCP, ptOutIt);
            else if(childCP->el.ehdr.type == LINE_STRING_ELM)
                ptOutIt = LineStringElemUtils::Copy(childCP, ptOutIt);
            else if(childCP->el.ehdr.type == SHAPE_ELM)
                ptOutIt = LineStringElemUtils::Copy(childCP, ptOutIt);
            }

        featureArray.Append((unsigned int)featureType, points.begin(), points.end());

        return SUCCESS;
        }
    };
 

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class MeshElemMeshExtractor : public ElementMeshExtractor
    {
    virtual void ComputeStats(MSElementDescrCP elmDescP, ElementMeshStats& stats) const override
        {
        size_t numPts = 0, numFacets = 0;
        PolyfaceHeaderPtr polyface;

        ElementHandle eh(elmDescP, false);

        if (SUCCESS == MeshHeaderHandler::PolyfaceFromElement (polyface, eh))
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

    virtual StatusInt Scan(MSElementDescrCP elmDescP, IDTMFeatureArray<DPoint3d>& featureArray) const override
        {
        ElementHandle eh(elmDescP, false);

        PolyfaceHeaderPtr polyface;
        if (SUCCESS == MeshHeaderHandler::PolyfaceFromElement (polyface, eh))
            {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*polyface, false);     // we just want points.
            bvector<DPoint3d> &facetPoints = visitor->Point ();      // this array will be loaded up facet by facet
            visitor->SetNumWrap (1);     // so each face gets a wraparound (closure) point.
            for (visitor->Reset (); visitor->AdvanceToNextFace ();)
                {
                // Make sure that we will not exceed the allocated packet capacity for points.
                if(featureArray.GetTotalPointQty() + (int)facetPoints.size () > featureArray.GetTotalPointCapacity())
                    return ERROR;                
                // TIN_POINT
                featureArray.Append((unsigned int)DTMFeatureType::TinPoint, facetPoints.begin(), facetPoints.end());
                // TIN_LINE
                featureArray.Append((unsigned int)DTMFeatureType::TinLine, facetPoints.begin(), facetPoints.end());                                
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
const ElementPointExtractor& ElementPointExtractor::GetFor(MSElementDescrCP elmDescP)
    {
    const int type = elmDescP->el.ehdr.type;
    if (LINE_ELM == type)
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
const ElementLinearExtractor& ElementLinearExtractor::GetFor(MSElementDescrCP elmDescP)
    {
    const int type = elmDescP->el.ehdr.type;

    if (LINE_ELM == type)
        {
        if (ElemUtils::Is3d(elmDescP))
            {
            static const LineElem3dLinExtractor LINE_3D_INSTANCE;
            return LINE_3D_INSTANCE;
            }

        static const LineElem2dLinExtractor LINE_2D_INSTANCE;
        return LINE_2D_INSTANCE;
        }
    if (LINE_STRING_ELM == type || SHAPE_ELM == type)
        {
        if (ElemUtils::Is3d(elmDescP))
            {
            static const LineStringElem3dLinExtractor LINE_STRING_3D_INSTANCE;
            return LINE_STRING_3D_INSTANCE;
            }

        static const LineStringElem2dLinExtractor LINE_STRING_2D_INSTANCE;
        return LINE_STRING_2D_INSTANCE;
        }
    if (CMPLX_SHAPE_ELM == type || CMPLX_STRING_ELM == type)
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
const ElementMeshExtractor& ElementMeshExtractor::GetFor (MSElementDescrCP elmDescP)
    {
    const int type = elmDescP->el.ehdr.type;

    if (MESH_HEADER_ELM == type)
        {
        static const MeshElemMeshExtractor MESH_INSTANCE;
        return MESH_INSTANCE;
        }

    static const UnsupportedElemMeshExtractor UNSUPPORTED_INTANCE;
    return UNSUPPORTED_INTANCE;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
