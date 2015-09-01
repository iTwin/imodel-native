//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUtility.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFUtility implementation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>

#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFRasterFileResBooster.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRFSloStripAdapter.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFLRDFile.h>
#include <Imagepp/all/h/HRFCacheFileCreator.h>
#include <Imagepp/all/h/HRFThumbnail.h>
#include <Imagepp/all/h/HTIFFTag.h>
#include <Imagepp/all/h/HRFTiffFile.h>

#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <Imagepp/all/h/HGF2DStretch.h>

//-----------------------------------------------------------------------------
// HGS Implementation Includes for thumbnail
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRABlitter.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HRASurface.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>

#include <Imagepp/all/h/HRFDtedFile.h>

#include <Imagepp/all/h/HUTExportProgressIndicator.h>




//----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFThumbnailProgressIndicator)

//----------------------------------------------------------------------------

HRFThumbnailProgressIndicator::HRFThumbnailProgressIndicator()
    : HFCProgressIndicator()
    {
    }

//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const double s_ObjectRectangle                    = 4.0;
static const double s_ObjectSurface                      = 3.0;
static const double s_ObjectHoledPolygon                 = 2.0;
static const double s_ObjectPolygon                      = 1.0;
static const double s_ObjectSurfaceVersion1              = 1.0;
static const double s_ObjectSurfaceVersionWithRectangles = 1.1;
static const double s_ObjectHoledPolygonVersion          = 1.0;
static const double s_ObjectPolygonVersion               = 1.6;
static const double s_ObjectRectangleVersion             = 1.0;


//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

// Descriptor for a Shape Object
typedef    struct
    {
    double Type;
    double Version;
    double EmbeddedObjectCount;
    } ObjectDescriptor;



//-----------------------------------------------------------------------------
// Internal functions foward declaration
//-----------------------------------------------------------------------------
static void sfExportSurfaceHoledPolygon (const HVE2DComplexShape&     pi_rShape,
                                         HArrayAutoPtr<double>&      pio_rpBuffer,
                                         size_t&                      pio_rBufferSize);
static void sfExportNormalHoledPolygon  (const HVE2DShape&            pi_rShape,
                                         HArrayAutoPtr<double>&      pio_rpBuffer,
                                         size_t&                      pio_rBufferSize);
static void sfExportShape               (const HVE2DShape&            pi_rShape,
                                         HArrayAutoPtr<double>&      pio_rpBuffer,
                                         size_t&                      pio_rBufferSize);
static void sfExtractShapesFromComplexShape  (const HVE2DComplexShape&    pi_rShape,
                                              HVE2DShape::ShapeList* po_pShapeList);
static void sfExportHoledShape          (const HVE2DShape&            pi_rShape,
                                         HArrayAutoPtr<double>&      pio_rpBuffer,
                                         size_t&                      pio_rBufferSize);
static void sfDropShape                 (const HVE2DShape&            pi_rShape,
                                         HArrayAutoPtr<double>&      pio_rpBuffer,
                                         size_t&                      pio_rBufferSize);

static HFCPtr<HGSMemorySurfaceDescriptor> sf1BitBestQualityStretcher (HGSMemorySurfaceDescriptor const& pi_SrcSurfaceDesc, uint32_t pi_PreferedHeight);

static HFCPtr<HGSMemorySurfaceDescriptor> sfBestQualityStretcher(HGSMemorySurfaceDescriptor const& pi_SrcSurfaceDesc, uint32_t pi_PreferedHeight);

static WString sfHRFGeoKeyTagToString     (const HFCPtr<HPMGenericAttribute>& pi_rpGeoKeyTag, unsigned short pi_GeoKey);
static unsigned short sfHRFDecodeGeoKeyFromString(const HFCPtr<HPMGenericAttribute>& pi_rpTag);




//-----------------------------------------------------------------------------
// Currently, we only support one surface composed of holed polygons,
// each holed polygon being composed of polygons or rectangles, depending
// on the surface version.
//
// Shape Tag description
//
// ObjectType  Description
// 1           Polygon
// 2           Holed Polygon
// 3           Surface
// 4           Rectangle
//
// Object    Version  Description                    Supported
// Polygon   1.0      Logical  3D coordinates        false
// Polygon   1.1      Physical 2D coordinates        false
// Polygon   1.5      Logical  3D coordinates        false
// Polygon   1.6      Physical 2D coordinates        true
// Rectangle 1.0      Physical 2D coordinates        true
//
//
// Surface version    Objects inside holed polygons can be    Supported
//   1.0              Polygon                                 true
//   1.1              Polygon or Rectangle                    true
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// ImportShapeFromArrayOfDouble
// For now We support only one polygon
//-----------------------------------------------------------------------------
HRFClipShape* ImagePP::ImportShapeFromArrayOfDouble(const double* pi_pLogicalShape,
                                           size_t         pi_LogicalShapeLength)
    {
    HPRECONDITION(pi_pLogicalShape != 0);
    size_t              OffsetInArray = 0;
    ObjectDescriptor    Surface;
    ObjectDescriptor    Holed;
    ObjectDescriptor    Polygon;
    HVEShape            Result(new HGF2DCoordSys());
    HRFCoordinateType   CoordType = HRFCoordinateType::PHYSICAL;
    HFCPtr<HVEShape>    pShape;

    static uint32_t const IMPORT_SHAPE_ERROR = -1234;

    try
        {
        // verify that there is enough data in the array
        if (pi_LogicalShapeLength < OffsetInArray + 3)
            throw IMPORT_SHAPE_ERROR;

        // Extract the surface information from the array and verify
        // that we have a valid Complex Shape
        Surface.Type                = pi_pLogicalShape[OffsetInArray++];
        Surface.Version             = pi_pLogicalShape[OffsetInArray++];
        Surface.EmbeddedObjectCount = pi_pLogicalShape[OffsetInArray++];
        if ((Surface.Type != s_ObjectSurface) ||
            (Surface.Version != s_ObjectSurfaceVersion1 && Surface.Version != s_ObjectSurfaceVersionWithRectangles) ||
            (Surface.EmbeddedObjectCount == 0))
            throw IMPORT_SHAPE_ERROR;

        // Process each Embedded Holed Polygon
        for (uint32_t HoledPolygonCount = 0;
             HoledPolygonCount < Surface.EmbeddedObjectCount;
             HoledPolygonCount++)
            {
            HVE2DHoledShape HoledShape(Result.GetCoordSys());

            // verify that there is enough data in the array
            if (pi_LogicalShapeLength < OffsetInArray + 3)
                throw IMPORT_SHAPE_ERROR;

            // Extract the holed polygon information from the array and verify
            // that we have a valid holed polygon
            Holed.Type                = pi_pLogicalShape[OffsetInArray++];
            Holed.Version             = pi_pLogicalShape[OffsetInArray++];
            Holed.EmbeddedObjectCount = pi_pLogicalShape[OffsetInArray++];
            if ((Holed.Type != s_ObjectHoledPolygon) ||
                (Holed.Version != s_ObjectHoledPolygonVersion) ||
                (Holed.EmbeddedObjectCount == 0))
                throw IMPORT_SHAPE_ERROR;

            // Process each Embedded Polygon
            for (uint32_t PolygonCount = 0;
                 PolygonCount < Holed.EmbeddedObjectCount;
                 PolygonCount++)
                {
                // verify if there is enough data in the header
                if (pi_LogicalShapeLength < OffsetInArray + 3)
                    throw IMPORT_SHAPE_ERROR;

                // extract the polygon information from the array and verify
                // that we have a valid polygon
                Polygon.Type                = pi_pLogicalShape[OffsetInArray++];
                Polygon.Version             = pi_pLogicalShape[OffsetInArray++];
                Polygon.EmbeddedObjectCount = pi_pLogicalShape[OffsetInArray++];
                if (Polygon.Type == s_ObjectRectangle)
                    {
                    if ((Polygon.Version != s_ObjectRectangleVersion) ||
                        (Polygon.EmbeddedObjectCount != 4))
                        throw IMPORT_SHAPE_ERROR;

                    // verify that there is enough data for the current shape in the array
                    if (pi_LogicalShapeLength < OffsetInArray + Polygon.EmbeddedObjectCount)
                        throw IMPORT_SHAPE_ERROR;

                    pShape = new HVEShape(pi_pLogicalShape[OffsetInArray],
                                          pi_pLogicalShape[OffsetInArray + 1],
                                          pi_pLogicalShape[OffsetInArray + 2],
                                          pi_pLogicalShape[OffsetInArray + 3],
                                          HoledShape.GetCoordSys());
                    }
                else
                    {
                    if ((Polygon.Type    != s_ObjectPolygon) ||
                        (Polygon.Version != s_ObjectPolygonVersion) ||
                        ((Polygon.EmbeddedObjectCount > 0) &&
                         (Polygon.EmbeddedObjectCount < 6)) )
                        throw IMPORT_SHAPE_ERROR;

                    // if the object count is 0, then build a VOID shape
                    if (Polygon.EmbeddedObjectCount == 0)
                        {
                        pShape = new HVEShape (HVE2DVoidShape(HoledShape.GetCoordSys()));
                        }
                    else
                        {
                        // verify that there is enough data for the current shape in the array
                        if (pi_LogicalShapeLength < OffsetInArray + Polygon.EmbeddedObjectCount)
                            throw IMPORT_SHAPE_ERROR;

                        // the current polygon is from version 1.0 or 1.5, the shape is
                        // in LOGICAL coordinates
                        if ((Polygon.Version == 1.0) || (Polygon.Version == 1.5))
                            CoordType = HRFCoordinateType::LOGICAL;

                        // Create the current shape
                        size_t BufferLength = (size_t)Polygon.EmbeddedObjectCount;
                        pShape = new HVEShape (&BufferLength,
                                               (double*)pi_pLogicalShape + OffsetInArray,
                                               HoledShape.GetCoordSys());

                        // Simplify the polygon to a rectangle if possible
                        if (pShape->GetShapePtr()->GetClassID() == HVE2DPolygonOfSegments::CLASS_ID &&
                            ((HVE2DPolygonOfSegments*)pShape->GetShapePtr())->RepresentsARectangle())
                            {
                            HFCPtr<HVE2DRectangle> pRectangle(((HVE2DPolygonOfSegments*)pShape->GetShapePtr())->GenerateCorrespondingRectangle());
                            pShape = new HVEShape(*pRectangle);
                            }
                        }
                    }

                // Skip to the next polygon (If any)
                OffsetInArray += (int32_t)Polygon.EmbeddedObjectCount;

                // Skip holed stuff if there are no holes
                if (Holed.EmbeddedObjectCount > 1)
                    {
                    // a polygon has been created, unify it with the current holed shape
                    if (HoledShape.IsEmpty())
                        HoledShape.SetBaseShape((const HVE2DSimpleShape&)*pShape->GetShapePtr());
                    else
                        HoledShape.AddHole((const HVE2DSimpleShape&)*pShape->GetShapePtr());
                    }
                }

            // A holed polygon has been created, unify it with the result shape
            if (Holed.EmbeddedObjectCount == 1)
                Result.Unify(*pShape);   // Simple polygon, not holed!
            else
                Result.Unify(HVEShape(HoledShape));
            }
        }
    catch(uint32_t&)
        {
        // something happened.  Make sure the we return no shape
        Result.MakeEmpty();
        }
    catch(HFCException&)
        {
        // something happened.  Make sure the we return no shape
        Result.MakeEmpty();
        }

    // since the shape is an auto pointer, release it
    return new HRFClipShape(Result, CoordType);
    }


//-----------------------------------------------------------------------------
// public
// ExportClipShapeToArrayOfDouble
// For now We support only one surface.
//
// By default (pi_SurfaceVersionToUse parameter = 1.0), the objects used to
// represent a hold polygon will all be polygons. If the parameter is set to
// 1.1, rectangles will be used if necessary.
//-----------------------------------------------------------------------------
double* ImagePP::ExportClipShapeToArrayOfDouble(const HRFClipShape& pi_rShape,
                                        size_t*             po_pShapeLength,
                                        double             pi_SurfaceVersionToUse)
    {
    HPRECONDITION(po_pShapeLength != 0);
    HArrayAutoPtr<double>  pBuffer(new double[3]);
    size_t                  BufferSize = 0;

    // Make sure we're asked to generate a valid surface version
    HASSERT(pi_SurfaceVersionToUse == s_ObjectSurfaceVersion1 ||
            pi_SurfaceVersionToUse == s_ObjectSurfaceVersionWithRectangles);

    // prepare the surface header
    pBuffer[BufferSize++] = s_ObjectSurface;
    pBuffer[BufferSize++] = pi_SurfaceVersionToUse;
    pBuffer[BufferSize++] = 0;  // for now, will be updated by sub-routines

    // Create a version of the shape in the given coord sys
    HRFClipShape Shape(pi_rShape);

    // Export the internal shape pointer of the shape object
    const HVE2DShape* pShape = Shape.GetShapePtr();
    try
        {
        // Set the number of holed polygon in the surface
        // if the shape is a HVE2DComplexShape, the number of holed polygons
        // is the number of shaped in the complex shape.  For any other shape,
        // there is 1 holed shape.
        if (pShape->GetClassID() == HVE2DComplexShape::CLASS_ID)
            sfExportSurfaceHoledPolygon((const HVE2DComplexShape&)*pShape, pBuffer, BufferSize);
        else
            sfExportNormalHoledPolygon(*pShape, pBuffer, BufferSize);

        // set the shape lenth parameter pointer to the number
        // of elements in the double array
        *po_pShapeLength = BufferSize;
        }
    catch(HFCException&)
        {
        // something happened.  Make sure the we return no data
        pBuffer = 0;
        *po_pShapeLength = 0;
        }

    // since the array is an auto pointer, release it
    return pBuffer.release();
    }


//-----------------------------------------------------------------------------
// static
// Extracts the shapes from a complex shape.  If one of these shapes is also a
// complex shape, the method recurses to extract its shapes.
//-----------------------------------------------------------------------------
void sfExtractShapesFromComplexShape(const HVE2DComplexShape& pi_rShape,
                                     HVE2DShape::ShapeList*   po_pShapeList)
    {
    HPRECONDITION(pi_rShape.GetClassID() == HVE2DComplexShape::CLASS_ID);
    HPRECONDITION(po_pShapeList != 0);

    const HVE2DShape::ShapeList& rShapeList = pi_rShape.GetShapeList();
    for (HVE2DShape::ShapeList::const_iterator Itr = rShapeList.begin();
         Itr != rShapeList.end();
         Itr++)
        {
        // if the current shape is a complex shape, recurse
        if ((*Itr)->GetClassID() == HVE2DComplexShape::CLASS_ID)
            sfExtractShapesFromComplexShape((const HVE2DComplexShape&)** Itr, po_pShapeList);
        else
            po_pShapeList->push_back(*Itr);
        }
    }


//-----------------------------------------------------------------------------
// static
// Builds the holed polygon data for a complex sha[e (extracts all the internal shapes
// which becomes holed shapes).
//-----------------------------------------------------------------------------
void sfExportSurfaceHoledPolygon(const HVE2DComplexShape& pi_rShape,
                                 HArrayAutoPtr<double>&  pio_rpBuffer,
                                 size_t&                  pio_rBufferSize)
    {
    HPRECONDITION(pi_rShape.GetClassID() == HVE2DComplexShape::CLASS_ID);
    HPRECONDITION(pio_rpBuffer != 0);

    // Extract the shape in the complex shape.  Recurse on all internal complex shape.
    HVE2DShape::ShapeList Shapes;
    sfExtractShapesFromComplexShape(pi_rShape, &Shapes);

    // the last element in the array is the number of element
    HPRECONDITION(pio_rpBuffer[pio_rBufferSize - 1] == 0);
    pio_rpBuffer[pio_rBufferSize - 1] += Shapes.size();

    // parse each shapes in the list
    for (HVE2DShape::ShapeList::const_iterator Itr = Shapes.begin();
         Itr != Shapes.end();
         Itr++)
        {
        // There must be no complex shape in this list!
        HASSERT((*Itr)->GetClassID() != HVE2DComplexShape::CLASS_ID);

        // compute the size for the new buffer
        // previous data + holed polygon header + polygon data
        size_t NewSize = pio_rBufferSize + 3;

        // grow the buffer to accomodate the new data
        HArrayAutoPtr<double> pNew(new double[NewSize]);
        memcpy(pNew, pio_rpBuffer, pio_rBufferSize * sizeof(double));
        pio_rpBuffer = pNew;

        // Set the current HOLED polygon header
        pio_rpBuffer[pio_rBufferSize++] = s_ObjectHoledPolygon;
        pio_rpBuffer[pio_rBufferSize++] = s_ObjectHoledPolygonVersion;
        pio_rpBuffer[pio_rBufferSize++] = 0; // for now, will be updated by sub-routines

        // export the current shape
        if ((*Itr)->HasHoles())
            sfExportHoledShape(*(*Itr), pio_rpBuffer, pio_rBufferSize);
        else
            sfExportShape(*(*Itr), pio_rpBuffer, pio_rBufferSize);
        }
    }


//-----------------------------------------------------------------------------
// static
// Builds the holed polygon data for a normal shape (not a HVE2DComplexShape)
//-----------------------------------------------------------------------------
static void sfExportNormalHoledPolygon(const HVE2DShape&       pi_rShape,
                                       HArrayAutoPtr<double>& pio_rpBuffer,
                                       size_t&                 pio_rBufferSize)
    {
    HPRECONDITION(pi_rShape.GetClassID() != HVE2DComplexShape::CLASS_ID);
    HPRECONDITION(pio_rpBuffer != 0);

    // the last element in the array is the number of element
    HPRECONDITION(pio_rpBuffer[pio_rBufferSize - 1] == 0);
    pio_rpBuffer[pio_rBufferSize - 1] = 1;

    // compute the size for the new buffer
    // previous data + holed polygon header + polygon data
    size_t NewSize = pio_rBufferSize + 3;

    // grow the buffer to accomodate the new data
    HArrayAutoPtr<double> pNew(new double[NewSize]);
    memcpy(pNew, pio_rpBuffer, pio_rBufferSize * sizeof(double));
    pio_rpBuffer = pNew;

    // Set the current HOLED polygon header
    pio_rpBuffer[pio_rBufferSize++] = s_ObjectHoledPolygon;
    pio_rpBuffer[pio_rBufferSize++] = s_ObjectHoledPolygonVersion;
    pio_rpBuffer[pio_rBufferSize++] = 0; // for now, will be updated by sub-routines

    // export the current shape
    if (pi_rShape.GetClassID() == HVE2DHoledShape::CLASS_ID)
        sfExportHoledShape(pi_rShape, pio_rpBuffer, pio_rBufferSize);
    else
        sfExportShape(pi_rShape, pio_rpBuffer, pio_rBufferSize);
    }


//-----------------------------------------------------------------------------
// static
// Builds a polygon from a shape
//-----------------------------------------------------------------------------
static void sfDropShape(const HVE2DShape&       pi_rShape,
                        HArrayAutoPtr<double>& pio_rpBuffer,
                        size_t&                 pio_rBufferSize)
    {
    // The dropped shape must be simple
    HPRECONDITION(pi_rShape.IsSimple());

    // A buffer must have been provided
    HPRECONDITION(pio_rpBuffer != 0);

    // Declare collection of points
    HGF2DLocationCollection PointsCollection;

    size_t NewSize;

    // Second position in buffer holds the surface version. We only create
    // rectangles if the surface version supports them.
    if (pio_rpBuffer[1] == s_ObjectSurfaceVersionWithRectangles &&
        pi_rShape.GetClassID() == HVE2DRectangle::CLASS_ID)
        {
        NewSize = pio_rBufferSize + 7;   // header plus 4 coordinates

        // grow the buffer to accomodate the new data
        HArrayAutoPtr<double> pNew(new double[NewSize]);
        memcpy(pNew, pio_rpBuffer, pio_rBufferSize * sizeof(double));
        pio_rpBuffer = pNew;

        // Set rectangle header
        pio_rpBuffer[pio_rBufferSize++] = s_ObjectRectangle;
        pio_rpBuffer[pio_rBufferSize++] = s_ObjectRectangleVersion;
        pio_rpBuffer[pio_rBufferSize++] = 4;

        ((HVE2DRectangle&)pi_rShape).GetRectangle(&pio_rpBuffer[pio_rBufferSize],
                                                  &pio_rpBuffer[pio_rBufferSize+1],
                                                  &pio_rpBuffer[pio_rBufferSize+2],
                                                  &pio_rpBuffer[pio_rBufferSize+3]);
        pio_rBufferSize += 4;
        }
    else
        {
        // Either the version does not support rectangles or we have any of the other
        // simple shapes
        // Drop the shape in the point collection
        pi_rShape.Drop(&PointsCollection, 1.0);

        // verify that the point collection is valid
        if (PointsCollection.size() == 0)
            {
//HChkAR there is no points ... do we send void shapes this way????
            // compute the size for the new buffer
            //  previous data + current polygon header + (no data)
            NewSize = pio_rBufferSize + 3;

            // grow the buffer to accomodate the new data
            HArrayAutoPtr<double> pNew(new double[NewSize]);
            memcpy(pNew, pio_rpBuffer, pio_rBufferSize * sizeof(double));
            pio_rpBuffer = pNew;

            // Set the polygon header
            pio_rpBuffer[pio_rBufferSize++] = s_ObjectPolygon;
            pio_rpBuffer[pio_rBufferSize++] = s_ObjectPolygonVersion;
            pio_rpBuffer[pio_rBufferSize++] = 0;
            }
        else
            {
            // compute the size for the new buffer
            //  previous data + current polygon header + polygon data
            NewSize = pio_rBufferSize + 3 + (PointsCollection.size() - 1) * 2;

            // grow the buffer to accomodate the new data
            HArrayAutoPtr<double> pNew(new double[NewSize]);
            memcpy(pNew, pio_rpBuffer, pio_rBufferSize * sizeof(double));
            pio_rpBuffer = pNew;

            // Set the polygon header
            pio_rpBuffer[pio_rBufferSize++] = s_ObjectPolygon;
            pio_rpBuffer[pio_rBufferSize++] = s_ObjectPolygonVersion;
            pio_rpBuffer[pio_rBufferSize++] = (double)(PointsCollection.size() - 1) * 2;

            // set the data of the polygon
            uint32_t i = 0;
            for (HGF2DLocationCollection::const_iterator Itr = PointsCollection.begin();
                 (i < PointsCollection.size() - 1) && (Itr != PointsCollection.end());
                 Itr++, i++)
                {
                pio_rpBuffer[pio_rBufferSize++] = (*Itr).GetX();
                pio_rpBuffer[pio_rBufferSize++] = (*Itr).GetY();
                }
            }
        }

    // after all of this, the buffer size must be equal to NewSize
    HASSERT(pio_rBufferSize == NewSize);
    }


//-----------------------------------------------------------------------------
// static
// Builds a polygon from a shape
//-----------------------------------------------------------------------------
static void sfExportShape(const HVE2DShape&       pi_rShape,
                          HArrayAutoPtr<double>& pio_rpBuffer,
                          size_t&                 pio_rBufferSize)
    {
    // The dropped shape must be simple
    HPRECONDITION(pi_rShape.IsSimple());

    // Check that a recipient buffer is provided
    HPRECONDITION(pio_rpBuffer != 0);

    // The last element in the array is the number of element
    HPRECONDITION(pio_rpBuffer[pio_rBufferSize - 1] == 0);
    pio_rpBuffer[pio_rBufferSize - 1] += 1;

    // Drop the simple shape in the array
    sfDropShape(pi_rShape, pio_rpBuffer, pio_rBufferSize);
    }


//-----------------------------------------------------------------------------
// static
// Builds a polygon from a holed shape
//-----------------------------------------------------------------------------
static void sfExportHoledShape(const HVE2DShape&       pi_rShape,
                               HArrayAutoPtr<double>& pio_rpBuffer,
                               size_t&                 pio_rBufferSize)
    {
    HPRECONDITION(pi_rShape.GetClassID() == HVE2DHoledShape::CLASS_ID);
    HPRECONDITION(pio_rpBuffer != 0);

    // Cast for programmer's convenience
    const HVE2DHoledShape& rShape = (HVE2DHoledShape&)pi_rShape;

    // Compute the number of holes
    size_t HoleCount = (rShape.HasHoles() ? rShape.GetHoleList().size() : 0);

    // Indicate in the total number of shapes in this holed polygon
    // the last element in the array is the number of element
    HPRECONDITION(pio_rpBuffer[pio_rBufferSize - 1] == 0);
    pio_rpBuffer[pio_rBufferSize - 1] = (double)(HoleCount + 1); // The shape plus its holes

    // Drop the main shape if the holed polygon
    sfDropShape(rShape.GetBaseShape(), pio_rpBuffer, pio_rBufferSize);

    // drop the holes
    if (rShape.HasHoles())
        {
        // get the list of holes
        const HVE2DShape::HoleList& rHoles = rShape.GetHoleList();

        // Drop the holes in the array
        for (HVE2DShape::HoleList::const_iterator Itr = rHoles.begin();
             Itr != rHoles.end();
             Itr++)
            sfDropShape(*(*Itr), pio_rpBuffer, pio_rBufferSize);
        }
    }

//-----------------------------------------------------------------------------
// GenericImprove to redefine by your application
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile>  ImagePP::GenericImprove(HFCPtr<HRFRasterFile>       pi_rpRasterFile,
                                      const HRFCacheFileCreator*  pi_pCreator,
                                      bool                        pi_PageFileOverwrite,
                                      bool                        pi_ApplyPageFile)
    {
    return GenericImprove(pi_rpRasterFile, 
                          pi_pCreator, 
                          pi_PageFileOverwrite, 
                          pi_ApplyPageFile, 
                          1.0);
    }

//-----------------------------------------------------------------------------
// GenericImprove to redefine by your application
//-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 @return A new pointer of HRFRasterFile

 @param pi_rpForRasterFile Raster file source.
 @param pi_pCreator Cache file creator, if == 0 Don't create cache file.
 @param pi_PageFileOverwrite TRUE : Try to find an associate the sister file HGR with 
                             all types of files(ITiff, GeoTiff, etc)
                             FALSE(default) : Try to find an associate sister file HGR
                             only with the files don't support Georeference.
 @param pi_ApplyPageFile Apply the page file if present.
 @param pi_DefaultRatioToMeterForPageFile Default ratio to meter to use for the page 
                                          file if the page file has no unit information.
-----------------------------------------------------------------------------*/
HFCPtr<HRFRasterFile>  ImagePP::GenericImprove(HFCPtr<HRFRasterFile>       pi_rpRasterFile, 
                                      const HRFCacheFileCreator*  pi_pCreator,
                                      bool                        pi_PageFileOverwrite,
                                      bool                        pi_ApplyPageFile, 
                                      double                      pi_DefaultRatioToMeterForPageFile)
    {
    bool CreateBestAdapter = true;

    HFCPtr<HRFRasterFile> pRasterFile = pi_rpRasterFile;

    // Adapt Scan Line Orientation (1 bit images)
    if ((pRasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID)) ||
        (pRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID))       ||
        (pRasterFile->IsCompatibleWith(HRFLRDFile::CLASS_ID))        ||
        (pRasterFile->IsCompatibleWith(HRFDtedFile::CLASS_ID))      )
        {
        if (HRFSLOStripAdapter::NeedSLOAdapterFor(pRasterFile))
            {
            // Adapt only when the raster file has not a standard scan line orientation
            // i.e. with an upper left origin, horizontal scan line.
            pRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pRasterFile);
            CreateBestAdapter = false;
            }
        }

    // Add the Decoration HGR, TFW or ERS Page File
    if (pi_ApplyPageFile && HRFPageFileFactory::GetInstance()->HasFor(pRasterFile, pi_PageFileOverwrite))
        {        
        if (pi_DefaultRatioToMeterForPageFile == 1.0)
            {
            pRasterFile = new HRFRasterFilePageDecorator(pRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pRasterFile));
            }
        else
            {            
            const HRFPageFileCreator* pPageFileCreator(HRFPageFileFactory::GetInstance()->FindCreatorFor(pRasterFile, pi_PageFileOverwrite));

            HASSERT(pPageFileCreator != 0);

            HFCPtr<HRFPageFile> pPageFile(pPageFileCreator->CreateFor(pRasterFile));

            pPageFile->SetDefaultRatioToMeter(pi_DefaultRatioToMeterForPageFile);

            HASSERT(pPageFile != 0);
            
            pRasterFile = new HRFRasterFilePageDecorator(pRasterFile, pPageFile);
            }        
        }

    // Adapt only when the raster file has not the same storage type
    if (CreateBestAdapter)
        pRasterFile = HRFRasterFileBlockAdapter::CreateBestAdapterBasedOnCacheFor(pRasterFile, pi_pCreator);

    if (pi_pCreator != 0)
        {
        // Add the cache or res booster when necessary
        if (HRFRasterFileResBooster::NeedResBoosterFor(pRasterFile))
            pRasterFile = new HRFRasterFileResBooster(pRasterFile, pi_pCreator);
        else if (HRFRasterFileCache::NeedCacheFor(pRasterFile))
            pRasterFile = new HRFRasterFileCache(pRasterFile, pi_pCreator);
        }

    return pRasterFile;
    }

//----------------------------------------------------------------------------------------
// HRFStretcher
// the output is 8-bits alignement
//----------------------------------------------------------------------------------------
Byte* ImagePP::HRFStretcher(HFCPtr<HRFRasterFile>& pi_rpSource,
                     uint32_t               pi_Page,
                     uint32_t*                pio_pPreferedWidth,
                     uint32_t*                pio_pPreferedHeight,
                     bool                  pi_UseBestQuality)
    {
    // Init Source descriptor, editor, and buffer of pixels
    HFCPtr<HRFPageDescriptor>       pPageDescriptor = pi_rpSource->GetPageDescriptor(pi_Page);
    unsigned short                 Resolution = 0;
    double                         ResolutionFactor;
    HFCPtr<HRFResolutionDescriptor> pSrcResolutionDescriptor;
    HAutoPtr<HRFResolutionEditor>   pSrcResolutionEditor;

    if (pPageDescriptor->IsUnlimitedResolution())
        {
        pi_UseBestQuality = false;
        if (pPageDescriptor->GetResolutionDescriptor(0)->GetHeight() > pPageDescriptor->GetResolutionDescriptor(0)->GetWidth())
            ResolutionFactor = (double)*pio_pPreferedHeight / (double)pPageDescriptor->GetResolutionDescriptor(0)->GetHeight();
        else
            ResolutionFactor = (double)*pio_pPreferedWidth / (double)pPageDescriptor->GetResolutionDescriptor(0)->GetWidth();

        pSrcResolutionEditor = pi_rpSource->CreateUnlimitedResolutionEditor(pi_Page, ResolutionFactor, HFC_READ_ONLY);
        pSrcResolutionDescriptor = pSrcResolutionEditor->GetResolutionDescriptor();
        }
    else
        {
        // Move to the small resolution
        Resolution = pPageDescriptor->CountResolutions() -1;

        // Find the resolution number
        if (pi_UseBestQuality)
            {
            // Find the resolution near of 1024
            while ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetHeight() <  1024) && (Resolution > 0))
                Resolution--;
            }
        else
            {
            // Find the resolution near of the width or height
            if (pPageDescriptor->GetResolutionDescriptor(0)->GetHeight() > pPageDescriptor->GetResolutionDescriptor(0)->GetWidth())
                {
                while ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetHeight() < *pio_pPreferedHeight) && (Resolution > 0))
                    Resolution--;
                }
            else
                {
                while ((pPageDescriptor->GetResolutionDescriptor(Resolution)->GetWidth() < *pio_pPreferedWidth) && (Resolution > 0))
                    Resolution--;
                }
            }
        pSrcResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(Resolution);
        pSrcResolutionEditor = pi_rpSource->CreateResolutionEditor(pi_Page, Resolution, HFC_READ_ONLY);
        }

    HFCPtr<HCDPacket>               pSrcPacket;

    pSrcPacket = new HCDPacket(new HCDCodecIdentity(),
                               new Byte[pSrcResolutionDescriptor->GetBlockSizeInBytes()],
                               pSrcResolutionDescriptor->GetBlockSizeInBytes(),
                               pSrcResolutionDescriptor->GetBlockSizeInBytes());
    pSrcPacket->SetBufferOwnership(true);

    // Calc the Height Step incrementation and update destination dimension
    double PreferedHeightStep;
    if (pSrcResolutionDescriptor->GetWidth() > pSrcResolutionDescriptor->GetHeight())
        {
        PreferedHeightStep = (double)pSrcResolutionDescriptor->GetWidth() / (double)*pio_pPreferedWidth;
        // Adjust the height with the Aspect ratio
        *pio_pPreferedHeight = (uint32_t)((double)pSrcResolutionDescriptor->GetHeight() / PreferedHeightStep);
        }
    else
        {
        PreferedHeightStep = (double)pSrcResolutionDescriptor->GetHeight() / (double)*pio_pPreferedHeight;
        // Adjust the width with the Aspect ratio
        *pio_pPreferedWidth = (uint32_t)((double)pSrcResolutionDescriptor->GetWidth() / PreferedHeightStep);
        }

    // Calc the best width and height
    uint32_t BestWidth  = *pio_pPreferedWidth;
    uint32_t BestHeight = *pio_pPreferedHeight;

    if (pi_UseBestQuality)
        {
        do
            {
            BestWidth  = BestWidth * 2;
            BestHeight = BestHeight * 2;
            }
        while (BestHeight < MIN(1024, pSrcResolutionDescriptor->GetHeight()));

        if (pSrcResolutionDescriptor->GetWidth() > pSrcResolutionDescriptor->GetHeight())
            PreferedHeightStep = (double)pSrcResolutionDescriptor->GetWidth() / (double)BestWidth;
        else
            PreferedHeightStep = (double)pSrcResolutionDescriptor->GetHeight() / (double)BestHeight;
        }

    // Init Destination dimensions and buffer of pixels
    double             HeightStep          = PreferedHeightStep;
    uint32_t            DstWidth            = BestWidth;
    uint32_t            DstHeight           = BestHeight;
    uint32_t            DstBytesPerWidth    = ((pSrcResolutionDescriptor->GetBitsPerPixel() * DstWidth) + 7) /8;
    uint32_t            PixelsSizeDst       = DstBytesPerWidth * DstHeight;
    HFCPtr<HCDPacket>   pDstPacket;
    double             DstBlockWidth;


    pDstPacket = new HCDPacket(new HCDCodecIdentity(),      // the packet will be initialized with a clear
                               new Byte[PixelsSizeDst],
                               PixelsSizeDst,
                               PixelsSizeDst);

    if (pSrcResolutionDescriptor->GetBlockWidth() >= pSrcResolutionDescriptor->GetWidth())
        DstBlockWidth = DstWidth;
    else
        DstBlockWidth = (double)pSrcResolutionDescriptor->GetBlockWidth() / HeightStep;


    const HFCPtr<HRPPixelType>& rpPixelType = pSrcResolutionDescriptor->GetPixelType();

    // Stretch settings
    // Source
    HFCPtr<HGSMemorySurfaceDescriptor> pDescriptorSrc(new HGSMemorySurfaceDescriptor(pSrcResolutionDescriptor->GetBlockWidth(),
                                                                               pSrcResolutionDescriptor->GetBlockHeight(),
                                                                               rpPixelType,
                                                                               pSrcPacket,
                                                                               HGF_UPPER_LEFT_HORIZONTAL,
                                                                               pSrcResolutionDescriptor->GetBytesPerBlockWidth()));

    // Destination
    HFCPtr<HGSMemorySurfaceDescriptor> pDescriptorDst(new HGSMemorySurfaceDescriptor(DstWidth,
                                                                               DstHeight,
                                                                               rpPixelType,
                                                                               pDstPacket,
                                                                               HGF_UPPER_LEFT_HORIZONTAL,
                                                                               DstBytesPerWidth));


    // Init the surface
    HRASurface destSurface(pDescriptorDst.GetPtr());
    HRABlitter blitter(destSurface);
    HRAEditor editor(destSurface);
    HRASurface srcSurface(pDescriptorSrc.GetPtr());


    // Stretching Generic Algo
    uint64_t BlockPosX;
    uint64_t BlockPosY;

    // clear the destination surface
    editor.Clear(rpPixelType->GetDefaultRawData());

    double aDstExtent[8];
    HFCPtr<HGSRegion> pRegion;

    double ScaleX = (double)pSrcResolutionDescriptor->GetWidth() / (double)DstWidth;
    double ScaleY = (double)pSrcResolutionDescriptor->GetHeight() / (double)DstHeight;

    HGF2DStretch StretchModel(HGF2DDisplacement(),
                              ScaleX,
                              ScaleY);

    HFCPtr<HGF2DStretch> pBlitModel(new HGF2DStretch(StretchModel));

    double MaxDstX;
    double MaxDstY;
    StretchModel.ConvertInverse((double)pSrcResolutionDescriptor->GetWidth(),
                                (double)pSrcResolutionDescriptor->GetHeight(),
                                &MaxDstX,
                                &MaxDstY);

    double SrcPosX;
    double SrcPosY;

    set<uint64_t> NeededBlocks;

    uint64_t LastBlockIndex = 0xFFFFFFFFFFFFFFFF;
    // First step: Compute needed source blocks
    for (uint32_t DstPosX = 0; DstPosX < DstWidth ; ++DstPosX)
        {
        for (uint32_t DstPosY=0 ; DstPosY < DstHeight ; DstPosY++)
            {
            StretchModel.ConvertDirect(DstPosX + 0.5,
                                       DstPosY + 0.5,
                                       &SrcPosX,
                                       &SrcPosY);
            HPOSTCONDITION(SrcPosX >= 0.0);
            HPOSTCONDITION(SrcPosY >= 0.0);

            if (SrcPosX < (double)pSrcResolutionDescriptor->GetWidth() &&
                SrcPosY < (double)pSrcResolutionDescriptor->GetHeight())
                {
                // Calc the block position for the needed block
                uint64_t BlockIndex = pSrcResolutionDescriptor->ComputeBlockIndex((uint32_t)SrcPosX, (uint32_t)SrcPosY);
                if(LastBlockIndex != BlockIndex) // optimization, calling insert for each pixel is slow.
                    {
                    NeededBlocks.insert(BlockIndex);
                    LastBlockIndex = BlockIndex; 
                    }

                pSrcResolutionDescriptor->ComputeBlockPosition(BlockIndex, &BlockPosX, &BlockPosY);

                if (HDOUBLE_EQUAL_EPSILON(SrcPosX, (double)BlockPosX))
                    NeededBlocks.insert(pSrcResolutionDescriptor->ComputeBlockIndex((uint32_t)SrcPosX - 1, (uint32_t)SrcPosY));

                if (HDOUBLE_EQUAL_EPSILON(SrcPosY, (double)BlockPosY))
                    {
                    NeededBlocks.insert(pSrcResolutionDescriptor->ComputeBlockIndex((uint32_t)SrcPosX, (uint32_t)SrcPosY - 1));

                    if (HDOUBLE_EQUAL_EPSILON(SrcPosX, (double)BlockPosX))
                        NeededBlocks.insert(pSrcResolutionDescriptor->ComputeBlockIndex((uint32_t)SrcPosX - 1, (uint32_t)SrcPosY - 1));
                    }
                }
            }
        }

    HRFThumbnailProgressIndicator::GetInstance()->Restart(NeededBlocks.size());

    // Lock the sister file once for all the ReadBlock
    HFCLockMonitor SisterFileLock;
    if(pi_rpSource->GetSharingControl() != NULL)
        pSrcResolutionEditor->AssignRasterFileLock(pi_rpSource, SisterFileLock, true);

    // Step two: copy the blocks
    set<uint64_t>::const_iterator Itr(NeededBlocks.begin());
    while (Itr != NeededBlocks.end() && HRFThumbnailProgressIndicator::GetInstance()->ContinueIteration())
        {
        // Calc the block position
        pSrcResolutionDescriptor->ComputeBlockPosition(*Itr, &BlockPosX, &BlockPosY);
        // Read directly the required block
        pSrcResolutionEditor->ReadBlock(BlockPosX, BlockPosY, pSrcPacket->GetBufferAddress(), &SisterFileLock);

        pBlitModel = new HGF2DStretch(HGF2DDisplacement(-(double)BlockPosX, -(double)BlockPosY),
                                      ScaleX,
                                      ScaleY);

        // Compute source surface shape, in destination system.
        // To intersect with destination cliping
        double OriginX;
        double OriginY;
        double CornerX;
        double CornerY;
        pBlitModel->ConvertInverse(0.0,
                                   0.0,
                                   &OriginX,
                                   &OriginY);
        pBlitModel->ConvertInverse(srcSurface.GetSurfaceDescriptor()->GetWidth(),
                                   srcSurface.GetSurfaceDescriptor()->GetHeight(),
                                   &CornerX,
                                   &CornerY);
        if (OriginX > CornerX)
            {
            double Temp = OriginX;
            OriginX = CornerX;
            CornerX = Temp;
            }
        if (OriginY > CornerY)
            {
            double Temp = OriginY;
            OriginY = CornerY;
            CornerY = Temp;
            }

        // Stretch the current line
        aDstExtent[0] = OriginX;
        aDstExtent[1] = OriginY;
        aDstExtent[2] = OriginX;
        aDstExtent[3] = MIN(MIN(CornerY, DstHeight), MaxDstY);
        aDstExtent[4] = MIN(MIN(CornerX, DstWidth), MaxDstX);
        aDstExtent[5] = MIN(MIN(CornerY, DstHeight), MaxDstY);
        aDstExtent[6] = MIN(MIN(CornerX, DstWidth), MaxDstX);
        aDstExtent[7] = OriginY;
        pRegion = new HGSRegion(aDstExtent,
                                8);

        // To support clipping correctly, drop the shape
        // into a series of points.

        destSurface.SetRegion(pRegion);
        blitter.BlitFrom(srcSurface, *pBlitModel);

        ++Itr;
        }

    SisterFileLock.ReleaseKey();

    // Quality Down sampling to the specified size
    Byte* pOutputData;
    if (pi_UseBestQuality)
        {
        HFCPtr<HGSMemorySurfaceDescriptor> pOutputSurfaceDesc;
        if (rpPixelType->CountPixelRawDataBits() == 1)
            pOutputSurfaceDesc = sf1BitBestQualityStretcher(*pDescriptorDst, *pio_pPreferedHeight);
        else
            pOutputSurfaceDesc = sfBestQualityStretcher(*pDescriptorDst, *pio_pPreferedHeight);
        
        HFCPtr<HCDPacket> pPacket = pOutputSurfaceDesc->GetPacket();

        // uncompress data
        if (!pPacket->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
            {
            size_t DataSize = (pOutputSurfaceDesc->GetWidth() + 7) / 8; // nb bytes per row
            DataSize *= pOutputSurfaceDesc->GetHeight();
            HFCPtr<HCDPacket> pUncompressedPacket = new HCDPacket(new HCDCodecIdentity(),
                                                                  new Byte[DataSize],
                                                                  DataSize);

            pPacket->Decompress(pUncompressedPacket);

            pUncompressedPacket->SetBufferOwnership(false);
            pOutputData = pUncompressedPacket->GetBufferAddress();
            }
        else
            {
            pPacket->SetBufferOwnership(false);
            pOutputData = pPacket->GetBufferAddress();
            }
        }
    else
        pOutputData = pDstPacket->GetBufferAddress();

    return pOutputData;
    }




//----------------------------------------------------------------------------------------
// static
// sf1BitBestQualityStretcher
// the output is 8-bits alignment
//----------------------------------------------------------------------------------------
HFCPtr<HGSMemorySurfaceDescriptor> sf1BitBestQualityStretcher(HGSMemorySurfaceDescriptor const& pi_SrcSurfaceDesc, uint32_t pi_PreferedHeight)
    {
    HPRECONDITION(pi_SrcSurfaceDesc.GetPixelType() != 0);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPixelType()->CountPixelRawDataBits() == 1);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPacket() != 0);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPacket()->GetCodec() != 0);

    HFCPtr<HCDPacket> pPacketUp;
    HFCPtr<HRPPixelType> pPixelType;

    // if the source was not compressed with RLE, compressed it to use OR algo
    if (!pi_SrcSurfaceDesc.GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
        {
        HFCPtr<HCDCodec> pCodec(new HCDCodecHMRRLE1(pi_SrcSurfaceDesc.GetWidth(), pi_SrcSurfaceDesc.GetHeight()));

        // the output is 8-bits alignment
        uint32_t LinePaddingBits = 8 - (pi_SrcSurfaceDesc.GetWidth() % 8);
        if (LinePaddingBits == 8)
            LinePaddingBits = 0;

        ((HFCPtr<HCDCodecImage>&)pCodec)->SetLinePaddingBits(LinePaddingBits);
        ((HFCPtr<HCDCodecHMRRLE1>&)pCodec)->EnableLineIndexesTable(true);

        pPacketUp = new HCDPacket(pCodec, 0, 0, 0);
        pPacketUp->SetBufferOwnership(true);

        pi_SrcSurfaceDesc.GetPacket()->Compress(pPacketUp);

        HPRECONDITION(pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID) ||
                      pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8::CLASS_ID) ||
                      pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeV1Gray1::CLASS_ID) ||
                      pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeV1GrayWhite1::CLASS_ID));

        // change the source pixel type
        if (pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8::CLASS_ID))
            pPixelType = new HRPPixelTypeI1R8G8B8RLE(pi_SrcSurfaceDesc.GetPixelType()->GetPalette());
        else if (pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8::CLASS_ID))
            pPixelType = new HRPPixelTypeI1R8G8B8A8RLE(pi_SrcSurfaceDesc.GetPixelType()->GetPalette());
        else if (pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeV1Gray1::CLASS_ID))
            pPixelType = new HRPPixelTypeI1R8G8B8RLE();
        else if (pi_SrcSurfaceDesc.GetPixelType()->IsCompatibleWith(HRPPixelTypeV1GrayWhite1::CLASS_ID))
            pPixelType = new HRPPixelTypeI1R8G8B8RLE();

        }
    else
        {
        pPacketUp  = pi_SrcSurfaceDesc.GetPacket();
        pPixelType = pi_SrcSurfaceDesc.GetPixelType();
        }


    uint32_t            SubWidth  = pi_SrcSurfaceDesc.GetWidth();
    uint32_t            SubHeight = pi_SrcSurfaceDesc.GetHeight();
    HFCPtr<HCDPacket>   pPacketDown = new HCDPacket(new HCDCodecHMRRLE1(SubWidth / 2,
                                                                        SubHeight / 2),
                                                    0,
                                                    0,
                                                    0);
    pPacketDown->SetBufferOwnership(true);

    ((HFCPtr<HCDCodecHMRRLE1>&)pPacketDown->GetCodec())->EnableLineIndexesTable(true);
    uint32_t LinePaddingBits = 8 - (SubWidth % 8);
    if (LinePaddingBits == 8)
        LinePaddingBits = 0;

    HGF2DStretch StretchModel(HGF2DDisplacement(),
                              2.0,
                              2.0);

    HFCPtr<HGSMemorySurfaceDescriptor> pDescriptorUp;
    HFCPtr<HGSMemorySurfaceDescriptor> pDescriptorDown;
    do
        {
        // Stretch settings
        // Up
        pDescriptorUp = new HGSMemorySurfaceDescriptor(SubWidth,
                                                       SubHeight,
                                                       pPixelType,
                                                       pPacketUp,
                                                       HGF_UPPER_LEFT_HORIZONTAL,
                                                       0);

        // Stretch settings
        // Down
        pDescriptorDown = new HGSMemorySurfaceDescriptor(SubWidth / 2,
                                                         SubHeight / 2,
                                                         pPixelType,
                                                         pPacketDown,
                                                         HGF_UPPER_LEFT_HORIZONTAL,
                                                         0);

        // Init the down surface
        HRASurface downSurface(pDescriptorDown.GetPtr());
            {   // Editor must be destroyed to complete.
            HRAEditor editor(downSurface);
            editor.Clear(pPixelType->GetDefaultRawData());
            }

        // init the up surface
        HRASurface UpSurface(pDescriptorUp.GetPtr());

        // Stretch Down to the sub-resolution
        HRABlitter blitter(downSurface);
        blitter.SetSamplingMode(HGSResampling::AVERAGE);
        blitter.BlitFrom(UpSurface, StretchModel);

        // set the new up packet
        pPacketUp = pPacketDown;

        // set size for down resolution
        SubWidth  = SubWidth / 2;
        SubHeight = SubHeight / 2;

        // create a new down packet
        pPacketDown = new HCDPacket(new HCDCodecHMRRLE1(SubWidth / 2, SubHeight / 2), 0, 0, 0);
        pPacketDown->SetBufferOwnership(true);

        ((HFCPtr<HCDCodecHMRRLE1>&)pPacketDown->GetCodec())->EnableLineIndexesTable(true);
        uint32_t LinePaddingBits = 8 - (SubWidth % 8);
        if (LinePaddingBits == 8)
            LinePaddingBits = 0;

        ((HFCPtr<HCDCodecHMRRLE1>&)pPacketDown->GetCodec())->SetLinePaddingBits(LinePaddingBits);

        }
    while (SubHeight > pi_PreferedHeight);

    return pDescriptorDown;
    }

//----------------------------------------------------------------------------------------
// static
// sfBestQualityStretcher
// the output is 8-bits alignment
//----------------------------------------------------------------------------------------
HFCPtr<HGSMemorySurfaceDescriptor> sfBestQualityStretcher(HGSMemorySurfaceDescriptor const& pi_SrcSurfaceDesc, uint32_t pi_PreferedHeight)
    {
    HPRECONDITION(pi_SrcSurfaceDesc.GetPixelType() != 0);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPixelType()->CountPixelRawDataBits() != 1);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPacket() != 0);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPacket()->GetCodec() != 0);
    HPRECONDITION(pi_SrcSurfaceDesc.GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID));

    HFCPtr<HCDPacket> pPacketUp = pi_SrcSurfaceDesc.GetPacket();

    uint32_t                SubWidth  = pi_SrcSurfaceDesc.GetWidth();
    uint32_t                SubHeight = pi_SrcSurfaceDesc.GetHeight();
    HFCPtr<HCDPacket>       pPacketDown;
    HFCPtr<HRPPixelType>    pPixelType(pi_SrcSurfaceDesc.GetPixelType());

    HFCPtr<HGSMemorySurfaceDescriptor> pDescriptorUp;
    HFCPtr<HGSMemorySurfaceDescriptor> pDescriptorDown;

    uint32_t UpSurfaceBytesPerRow = pi_SrcSurfaceDesc.GetBytesPerRow();
    uint32_t DownSurfaceBytesPerRow;
    size_t DataSize;

    HGF2DStretch StretchModel(HGF2DDisplacement(),
                              2.0,
                              2.0);
    do
        {
        // Stretch settings
        // Up
        pDescriptorUp = new HGSMemorySurfaceDescriptor(SubWidth,
                                                       SubHeight,
                                                       pPixelType,
                                                       pPacketUp,
                                                       HGF_UPPER_LEFT_HORIZONTAL,
                                                       UpSurfaceBytesPerRow);

        // Stretch settings
        // Down
        // create a new down packet
        DownSurfaceBytesPerRow = (((SubWidth / 2) * pPixelType->CountPixelRawDataBits()) + 7) / 8;
        DataSize = DownSurfaceBytesPerRow * (SubHeight / 2);
        pPacketDown = new HCDPacket(new HCDCodecIdentity(),
                                    new Byte[DataSize],
                                    DataSize);
        pPacketDown->SetBufferOwnership(true);
        pDescriptorDown = new HGSMemorySurfaceDescriptor(SubWidth / 2,
                                                         SubHeight / 2,
                                                         pPixelType,
                                                         pPacketDown,
                                                         HGF_UPPER_LEFT_HORIZONTAL,
                                                         DownSurfaceBytesPerRow);

        // Init the down surface
        HRASurface downSurface(pDescriptorDown.GetPtr());
        HRABlitter blitter(downSurface);
        if (pPixelType->CountValueBits() != 0)
            blitter.SetSamplingMode(HGSResampling::AVERAGE);
        else
            blitter.SetSamplingMode(HGSResampling::NEAREST_NEIGHBOUR);
        
            {
            HRAEditor editor(downSurface);
            editor.Clear(pPixelType->GetDefaultRawData());
            }

        // init the up surface
        HRASurface UpSurface(pDescriptorUp.GetPtr());

        // Stretch Down to the sub-resolution
        blitter.BlitFrom(UpSurface, StretchModel);

        // set the new up packet
        pPacketUp = pPacketDown;
        UpSurfaceBytesPerRow = DownSurfaceBytesPerRow;

        // set size for down resolution
        SubWidth  = SubWidth / 2;
        SubHeight = SubHeight / 2;

        }
    while (SubHeight > pi_PreferedHeight);

    return pDescriptorDown;
    }

//-----------------------------------------------------------------------------
// HRFThumbnailMaker
//-----------------------------------------------------------------------------

HFCPtr<HRFThumbnail> ImagePP::HRFThumbnailMaker(HFCPtr<HRFRasterFile>& pi_rpSource,
                                       uint32_t               pi_Page,
                                       uint32_t*                pio_pPreferedWidth,
                                       uint32_t*                pio_pPreferedHeight,
                                       bool                  pi_UseBestQuality)
    {
    //If this page not available, don't try to get a thumbnail
    if (pi_Page < pi_rpSource->CountPages())
        {
        if (!pi_rpSource->GetPageDescriptor(pi_Page)->HasThumbnail())
            {
            HArrayAutoPtr<Byte> pPixels;
            pPixels = HRFStretcher(pi_rpSource, pi_Page, pio_pPreferedWidth, pio_pPreferedHeight, pi_UseBestQuality);

            return new HRFThumbnail(*pio_pPreferedWidth,
                                    *pio_pPreferedHeight,
                                    pi_rpSource->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(0)->GetPixelType(),
                                    pPixels,
                                    HFC_READ_ONLY,
                                    false,
                                    8); // 8-bits alignement
            }
        else
            return pi_rpSource->GetPageDescriptor(pi_Page)->GetThumbnail();
        }
    return NULL;
    }


//-----------------------------------------------------------------------------
// public
// IsValidMatrix
//
// Make sure that global scaling is different from 0.0
// Make sure that provided parameters are valid
// This implies that the upper left parameters follow certain rules:
// One of [0,0] and [0,1] must different from 0.0
// One of [0,0] and [1,0] must different from 0.0
// One of [0,1] and [1,1] must different from 0.0
// One of [1,0] and [1,1] must different from 0.0
//-----------------------------------------------------------------------------
bool ImagePP::IsValidMatrix(const HFCMatrix<3,3>& pi_rMatrix)
    {
    bool IsValid = true;

    for (size_t i = 0; i < 3 && IsValid; i++)
        for (size_t j = 0; j < 3 && IsValid; j++)
            IsValid = BeNumerical::BeFinite(pi_rMatrix[i][j]) != 0;

    if (IsValid)
        {
        if (pi_rMatrix[2][2] == 0)
            IsValid = false;

        if ((pi_rMatrix[0][0] == 0.0) && (pi_rMatrix[0][1] == 0.0))
            IsValid = false;

        if ((pi_rMatrix[0][0] == 0.0) && (pi_rMatrix[1][0] == 0.0))
            IsValid = false;

        if ((pi_rMatrix[0][1] == 0.0) && (pi_rMatrix[1][1] == 0.0))
            IsValid = false;

        if ((pi_rMatrix[1][0] == 0.0) && (pi_rMatrix[1][1] == 0.0))
            IsValid = false;
        }
    return IsValid;
    }

//-----------------------------------------------------------------------------
// WriteEmptyFile
//-----------------------------------------------------------------------------
void ImagePP::WriteEmptyFile(HFCPtr<HRFRasterFile>& pi_prFile,
                    Byte*                 pi_pRGBDefaultColor)
    {
    HPRECONDITION(pi_prFile->CountPages() == 1);

    unsigned short                     NbRes = pi_prFile->GetPageDescriptor(0)->CountResolutions();
    HFCPtr<HRFResolutionDescriptor>     pResolutionDesc = 0;
    HAutoPtr<HRFResolutionEditor>       pResolutionEditor;
    HFCPtr<HRPPixelType>                pPixelType;
    HFCPtr<HRPPixelConverter>           pConverter;
    HAutoPtr<Byte>                     pBlockEmptyData;
    uint32_t                            ByteCounts = 0;
    uint64_t                           NbBlock = 0;
    Byte                              DefaultColor[HRPPixelType::MAX_PIXEL_BYTES]; // Default color

    NbBlock = pi_prFile->GetPageDescriptor(0)->CountBlocksForAllRes();

    HUTExportProgressIndicator::GetInstance()->Restart(NbBlock);
    HUTExportProgressIndicator::GetInstance()->SetExportedFile(pi_prFile);

    for (unsigned short ResInd = 0; ResInd < NbRes; ResInd++)
        {
        pResolutionDesc = pi_prFile->GetPageDescriptor(0)->GetResolutionDescriptor(ResInd);
        pResolutionEditor = pi_prFile->CreateResolutionEditor(0, ResInd, HFC_READ_CREATE);

        pPixelType = pResolutionDesc->GetPixelType();

        // Get this converter (V24R8G8B8 -> selected pixel type)
        pConverter = HRPPixelTypeV24R8G8B8().GetConverterTo(pPixelType);

        // Convert color
        pConverter->Convert(pi_pRGBDefaultColor,DefaultColor);

        ByteCounts = (uint32_t)ceil(pPixelType->CountPixelRawDataBits() / 8.0);

        //Propagate
        if (pPixelType->CountPixelRawDataBits() < 8)
            {
            switch (pPixelType->CountPixelRawDataBits())
                {
                case 1 :
                    //Arithmetic shift
                    DefaultColor[0] = (Byte)((int8_t)DefaultColor[0] >> 7);
                    break;
                case 4 :
                    DefaultColor[0] = ((Byte)DefaultColor[0] & 0xF0) + ((Byte)DefaultColor[0] >> 4);
                    break;
                default :
                    HASSERT(!"Pixel count handler not implemented");
                }
            }

        pBlockEmptyData = new Byte[pResolutionDesc->GetBlockSizeInBytes()];

        HASSERT((ByteCounts != 0) &&
                (pResolutionDesc->GetBlockSizeInBytes() % ByteCounts == 0));

        for (uint32_t ByteInd = 0; ByteInd < pResolutionDesc->GetBlockSizeInBytes(); ByteInd += ByteCounts)
            {
            memcpy((pBlockEmptyData + ByteInd), DefaultColor, ByteCounts);
            }

        for (uint32_t RowInd = 0; RowInd < pResolutionDesc->GetBlocksPerHeight(); RowInd++)
            {
            for (uint32_t ColInd = 0;
                 ColInd < pResolutionDesc->GetBlocksPerWidth() &&
                 HUTExportProgressIndicator::GetInstance()->ContinueIteration(pi_prFile, ResInd);
                 ColInd++)
                {
                pResolutionEditor->WriteBlock(ColInd * pResolutionDesc->GetBlockWidth(),
                                              RowInd * pResolutionDesc->GetBlockHeight(),
                                              pBlockEmptyData);
                }
            }
        }

    HUTExportProgressIndicator::GetInstance()->SetExportedFile(0);
    }