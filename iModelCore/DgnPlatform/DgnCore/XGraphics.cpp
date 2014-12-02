/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/XGraphics.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Mtg/decimatexyz.fdf>
#include <Bentley/BeConsole.h>
#include <Logging/bentleylogging.h>

#define GET_AND_INCREMENT_DATA(value, pData) memcpy (&value, pData, sizeof (value)); pData += sizeof (value);

#define GET_AND_INCREMENT(value)        GET_AND_INCREMENT_DATA (value, pData)
#define SET_AND_INCREMENT(value)        memcpy (pData, &value,  sizeof (value)); pData += sizeof (value);
#define GET_AND_INCREMENT_BOOL(value)   {bool boolVal; memcpy (&boolVal, pData, sizeof (boolVal)); pData += sizeof (boolVal); value = boolVal ? 1 : 0;}

#define XMATSYMB_LineColor              (0x0001 << 0)
#define XMATSYMB_LineColorIndex         (0x0001 << 1)
#define XMATSYMB_FillColor              (0x0001 << 2)
#define XMATSYMB_FillColorIndex         (0x0001 << 3)
#define XMATSYMB_RasterPattern          (0x0001 << 4)
#define XMATSYMB_RasterWidth            (0x0001 << 5)
#define XMATSYMB_FillDisplay            (0x0001 << 6)
#define XMATSYMB_ElemColor              (0x0001 << 7)
#define XMATSYMB_ElemFillColor          (0x0001 << 8)
#define XMATSYMB_ElemWeight             (0x0001 << 9)
#define XMATSYMB_ElemStyle              (0x0001 << 10)
#define XMATSYMB_ElemTransparency       (0x0001 << 11)
#define XMATSYMB_SubElemIndex           (0x0001 << 12)
#define XMATSYMB_Material               (0x0001 << 13)
#define XMATSYMB_ElemColorRGB           (0x0001 << 14)
#define XMATSYMB_StyleParams            (0x0001 << 15)

#define XMATSYMB2_GradientFill          (0x0001 << 0)

#define LOG (*LoggingManager::GetLogger (L"XGraphics"))

static double   s_directionTolerance     = 1.0E-8;
static double   s_distanceTolerance      = 1.0E-6;
static double   s_basisLength            = 1.0E4;           // Needs work. Should be UORs/meter or something of that ilk.

static XGraphicsOperations* s_XGraphicsOperations;

XGraphicsOperationP XGraphicsOperations::s_xGraphicOps[MAX_XGraphicsOpCode];

inline byte*    nextOp (byte* thisOp, UInt32 opSize) { return thisOp + opSize - sizeof (opSize); }

static ElementRefAppData::Key s_brepWireGraphicsCacheKey;

typedef bmap <ptrdiff_t, XGraphicsContainerP> T_BRepWireGraphicsMap;

#if defined(DEBUG_FIND_TEST_DATA)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2014
//---------------------------------------------------------------------------------------
static bvector<byte>*initTestData()
    {
    bvector<byte>* pTestData = new bvector<byte>();
    //  add data here
    return pTestData;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2014
//---------------------------------------------------------------------------------------
byte const*GetTestData(UInt32& testDataSize, bool includeHeader)
    {
    static bvector<byte>*pTestData;
    if (NULL == pTestData)
        pTestData = initTestData();

    testDataSize = (UInt32)pTestData->size();
    byte*retval = &(*pTestData)[0];
    if (!includeHeader)
        {
        testDataSize -= 2;
        retval += 2;
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2014
//---------------------------------------------------------------------------------------
static void findTestDataInContainer(byte const*xGraphicsData, UInt32 containerSize)
    {
    if (containerSize <= 200000000)
        return;

    //  skip the header
    containerSize -= 2;
    xGraphicsData += 2;

    UInt32 testDataSize;
    byte const*testData = GetTestData(testDataSize, false);
    if (testDataSize > containerSize)
        return;

    byte const*lastChecked = xGraphicsData + containerSize - testDataSize;
    for (byte const*curr = xGraphicsData; curr <= lastChecked;)
        {
        if (!memcmp(curr, testData, testDataSize))
            {
            printf("found the test data");
            return;
            }

        UInt32 size = *(UInt32*)(curr + 2) + 2;  //  size include "opSize" but not "opCode"
        curr += size;
        }
    }

static bool hasXGraphics (ElementHandleCR element)
    {
    ElementHandle::XAttributeIter iterator (element, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0);
    return iterator.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2014
//---------------------------------------------------------------------------------------
DGNPLATFORM_EXPORT void FindTestDataInElement(ElementHandleCR eh)
    {
    if (hasXGraphics(eh))
        {
        XGraphicsContainer temp;
        temp.ExtractFromElement(eh);
        findTestDataInContainer(temp.GetData(), (UInt32)temp.GetDataSize());
        }
    }
#endif

//=======================================================================================
//! The ARM architecture requires that all doubles are 4-byte aligned in memory.
//! You can use this helper to efficiently allocate as needed when reading XGraphics data, and before passing double-based pointers around.
// @bsiclass                                                    Jeff.Marker     03/2012
//=======================================================================================
struct XGScopedArray : NonCopyableClass
    {
private: 
    double  m_stackData[300];   // Arbitrary amount to prevent mallocs.
    double const*  m_data;

#ifdef DEBUG
    bool m_wasAcquired;
#endif // DEBUG

public: 
    XGScopedArray () : m_data (NULL)
#ifdef DEBUG
        ,m_wasAcquired (false)
#endif // DEBUG
        {
        }

    virtual ~XGScopedArray ()
        {
        if ((NULL == m_data) || (m_data == m_stackData))
            return;

        delete m_data;
        }

    //---------------------------------------------------------------------------------------
    // This can only be called once, and may return the original pointer, our stack-based pointer, or a pointer into malloced memory (which this instance will clean up).
    // @bsimethod                                                   Jeff.Marker     03/2012
    //---------------------------------------------------------------------------------------
    ByteCP Acquire (ByteCP pData, size_t sizeInBytes)
        {
#ifdef DEBUG
        if (m_wasAcquired)
            BeAssert (false);

        m_wasAcquired = true;
#endif // DEBUG

#if defined (WIN32)
        return pData;
#else

        // Already 4-byte aligned?
        if (0 == ((size_t)pData & 0x3))
            return pData;

        // Will it fit on the stack?
        if (sizeInBytes <= sizeof (m_stackData))
            {
            memcpy (m_stackData, pData, sizeInBytes);
            m_data = m_stackData;
            }
        else
            {
            // Otherwise we have to malloc.
            m_data = (double const*)malloc (sizeInBytes);
            memcpy (const_cast<double*>(m_data), pData, sizeInBytes);
            }

        return (ByteCP)m_data;
#endif
        }
};

//=======================================================================================
//! Like XGScopedArray, but it also copies the result out.
// @bsiclass                                                    John.Gooding    05/2013
//=======================================================================================
struct XGScopedArrayRW : private XGScopedArray
{
private:
    size_t  m_sizeInBytes;
    byte*   m_original;
    byte*   m_acquired;
public:
    XGScopedArrayRW() : m_sizeInBytes(0), m_original(NULL), m_acquired(NULL) {}

    ~XGScopedArrayRW()
        {
        if (m_original == m_acquired)
            return;

        memcpy(m_original, m_acquired, m_sizeInBytes);
        }

    byte* AcquireRW(byte* pData, size_t sizeInBytes)
        {
        m_original = pData;
        m_acquired = (byte*)Acquire(pData, sizeInBytes);
        m_sizeInBytes = sizeInBytes;
        return m_acquired; 
        }
};

//=======================================================================================
// @bsiclass                                                    Brien.Bastings  02/12
//=======================================================================================
struct BRepWireGraphicsAppData : ElementRefAppData
{
protected:

virtual WCharCP _GetName () override {return L"BRepWireGraphicsAppData";}
virtual bool    _OnElemChanged (ElementRefP host, bool qvCacheDeleted, ElemRefChangeReason reason) override {return (ELEMREF_CHANGE_REASON_ClearQVData != reason);}
virtual void    _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
    {
    // Free XGraphicsContainers in map...
    for (T_BRepWireGraphicsMap::iterator iter = m_wireMap.begin (); iter != m_wireMap.end (); ++iter)
        DELETE_AND_CLEAR (iter->second)

    m_wireMap.clear (); // Empty map...
    
    if (!unloadingCache)
        zone.Free (this, sizeof *this);
    }
public:

T_BRepWireGraphicsMap m_wireMap;

BRepWireGraphicsAppData () {}

static void ClearWireGeomCache (ElementRefP elementRef) { if (NULL != elementRef) elementRef->DropAppData (s_brepWireGraphicsCacheKey); }
static void CreateWireGeomCache (XGraphicsOperationContextR opContext, ViewContextR context, ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments);
static BentleyStatus DrawWireGeomCache (XGraphicsOperationContextR opContext, ViewContextR context);

}; // BRepWireGraphicsAppData

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool isEqual (double value, double rhsValue, double tolerance)
    {
    return fabs (value - rhsValue) < tolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void transformPoints (DPoint3dP points, int nPoints, TransformCR transform)
    {
    for (int i=0; i<nPoints; i++, points++)
        if (!points->isDisconnect())
            transform.multiply (points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void transformDPoint2dArray (DPoint2dP pPoint, double depth, int nPoints, Transform const* pTransform)
    {
    for (int i=0; i<nPoints; i++, pPoint++)
        {
        DPoint3d    tmpPoint = { pPoint->x, pPoint->y, depth };

        if (!tmpPoint.isDisconnect())
            pTransform->multiply (&tmpPoint);

        pPoint->x = tmpPoint.x;
        pPoint->y = tmpPoint.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void transform2dArc (DPoint2dR origin, double& orientation, double& r0, double& r1, double* start, double* sweep, double& depth, TransformCP pTransform)
    {
    DEllipse3d  dEllipse;

    dEllipse.initFromXYMajorMinor (origin.x, origin.y, depth, r0, r1, orientation, (start ? *start : 0.0), (sweep ? *sweep : msGeomConst_2pi));
    dEllipse.productOf (pTransform, &dEllipse);

    DPoint3d    origin3d;
    RotMatrix   matrix;

    dEllipse.getScaledRotMatrix(&origin3d, &matrix, &r0, &r1, start, sweep);

    origin.setComponents (origin3d.x, origin3d.y);
    depth = origin3d.z;
    orientation = bsiTrig_atan2 (matrix.getComponentByRowAndColumn (1,0), matrix.getComponentByRowAndColumn (0,0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void transform3dArc (DPoint3dR origin, DPoint4dR quaternion, double& r0, double& r1, double* start, double* sweep, TransformCP pTransform)
    {
    DEllipse3d  dEllipse;

    static double s_axisRatioTolerance = 1.0e-6;

    dEllipse.center = origin;
    dEllipse.start = start ? *start : 0.0;
    dEllipse.sweep = sweep ? *sweep : msGeomConst_piOver2;

    RotMatrix   rMatrix;

    rMatrix.initFromQuaternion (&quaternion.x);
    rMatrix.getRows (&dEllipse.vector0, &dEllipse.vector90, NULL);

    dEllipse.vector0.scale (r0);
    dEllipse.vector90.scale (r1);

    dEllipse.productOf (pTransform, &dEllipse);
    origin = dEllipse.center;

    if (NULL != start)
        *start = dEllipse.start;

    if (NULL != sweep)
        *sweep = dEllipse.sweep;

    DVec3d      xAxis, yAxis, zAxis;

    r0  = xAxis.normalize((DVec3d const*) &dEllipse.vector0);
    r1  = yAxis.normalize((DVec3d const*)  &dEllipse.vector90);

    if (r0 < s_axisRatioTolerance * r1)
        {
        // x axis is garbage, y is good ...
        yAxis.getNormalizedTriad (&zAxis, &xAxis, &yAxis);
        r0 = 0.0;
        }
    else if (r1 < s_axisRatioTolerance * r0)
        {
        // x axis is garbage, y is good ...
        xAxis.getNormalizedTriad (&yAxis, &zAxis, &xAxis);
        r1 = 0.0;
        }
    else
        {
        // The usual case.  x,y are perpendicular unit vectors ....
        zAxis.normalizedCrossProduct(&xAxis, &yAxis);
        }

    rMatrix.initFromColumnVectors (&xAxis, &yAxis, &zAxis);
    rMatrix.squareAndNormalizeColumns (&rMatrix, 0, 1);
    rMatrix.getQuaternion (&quaternion.x, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isTransformOrthogonalAndUniformScale (TransformCR transform)
    {
    RotMatrix   rMatrix;
    double      axisRatio;

    transform.getMatrix (&rMatrix);

    return rMatrix.isOrthonormal (NULL, NULL, &axisRatio) && isEqual (axisRatio, 1.0, s_directionTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     areQuaternionsEqual (DPoint4dCR quat0, DPoint4dCR quat1, double tolerance)
    {
    RotMatrix   rMatrix0, rMatrix1;

    rMatrix0.initFromQuaternion (&quat0.x);
    rMatrix1.initFromQuaternion (&quat1.x);

    return false != rMatrix0.isEqual (&rMatrix1, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     arePointsEqual (DPoint3dCP points0, int numPoints0, DPoint3dCP points1, int numPoints1, double tolerance)
    {
    if (numPoints0 != numPoints1)
        return false;

    for (int i=0; i<numPoints0; i++)
        if (!points0[i].isEqual (&points1[i], tolerance))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    getBasisTransformFromGPA (TransformR transform, GPArrayR gpa)
    {
    DPoint3d    origin;
    DVec3d      tangent;
    RotMatrix   rMatrix;

    if (!gpa.GetPrimitiveFractionPointAndTangent (origin, tangent, 0, 0.0))
        return ERROR;

    tangent.normalize ();

    DPlane3d    plane;

    if (gpa.GetPlane (plane))
        {
        DVec3d  yVector;

        yVector.normalizedCrossProduct (&tangent, &plane.normal);
        rMatrix.initFromColumnVectors (&tangent, &yVector, &plane.normal);
        }
    else
        {
        rMatrix.initFrom1Vector (&tangent, 0, false);
        }

    transform.initFrom (&rMatrix, &origin);

    return SUCCESS;
    }

static void getUnweightedPoint (DPoint3dCP points, double *weights, int i, DPoint3dR xyz)
    {
    static double s_divTol = 1.0e-12;
    xyz = points[i];
    if (NULL != weights  && weights[i] > s_divTol)
        {
        double a = 1.0 / weights[i];
        xyz.Scale (a);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    getBasisTransformFromPoints (TransformR transform, DPoint3dCP points, double *weights, int nPoints, double samePointTolerance = s_distanceTolerance)
    {
    if (0 == nPoints)
        return ERROR;

    int         i;
    DPoint3d    origin;
    getUnweightedPoint (points, weights, 0, origin);
    DPoint3d    xyz;
    for (i=1; i<nPoints; i++)
        {
        getUnweightedPoint (points, weights, i, xyz);
        if (!origin.isEqual (&xyz, samePointTolerance))
            break;
        }

    if (i == nPoints)
        return ERROR;

    DVec3d      xNormal, yNormal, zNormal;

    xNormal.normalizedDifference (&xyz, &origin);

    for (++i; i<nPoints; i++)
        {
        getUnweightedPoint (points, weights, i, xyz);        
        yNormal.differenceOf (&xyz, &origin);
        if (zNormal.normalizedCrossProduct (&xNormal, &yNormal) > samePointTolerance)
            break;
        }

    RotMatrix   rMatrix;

    if (i == nPoints)
        {
        rMatrix.initFrom1Vector (&xNormal, 0, false);
        }
    else
        {
        rMatrix.initFromColumnVectors (&xNormal, &yNormal, &zNormal);
        rMatrix.squareAndNormalizeColumns (&rMatrix, 0, 1);
        }

    transform.initFrom (&rMatrix, &origin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32   packTransform (double packedTransform[12], TransformCR transform)
    {
    RotMatrix   matrix;

    transform.getMatrix (&matrix);

    if (matrix.isRigid ())
        {
        DPoint3d    translation;

        transform.getTranslation (&translation);
        memcpy (packedTransform, &translation, sizeof (translation));

        if (!matrix.isIdentity ())
            {
            matrix.getQuaternion (&packedTransform[3], false);

            return sizeof (DPoint3d) + 4 * sizeof (double);
            }
        else
            {
            return sizeof (DPoint3d);
            }
        }
    else
        {
        memcpy (packedTransform, &transform, sizeof (transform));

        return sizeof (Transform);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     dumpTransform (TransformR transform)
    {
    for (int i=0; i<3; i++)
        printf ("\t\t [%f,\t%f, \t%f, \t%f]\n", transform.form3d[i][0], transform.form3d[i][1], transform.form3d[i][2], transform.form3d[i][3]);
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsWriter
{
XGraphicsContainerP m_container;

XGraphicsWriter (XGraphicsContainer* container)                 { m_container = container; }

void            ReplaceHeader (XGraphicsHeader const& header)   { m_container->ReplaceHeader (header); }
void            SetUseCache (bool useCache)                     { m_container->SetUseCache (useCache); }
void            SetBRepsPresent (bool hasBrep)                  { m_container->SetBRepsPresent (hasBrep); }
void            SetUseForStencil (bool useForStencil)           { m_container->SetUseForStencil (useForStencil); }
void            SetValidForStencil (bool validForStencil)       { m_container->SetValidForStencil (validForStencil); }
void            SetIsRenderable (bool isRenderable)             { m_container->SetIsRenderable (isRenderable); }
void            SetUnsupportedPrimitivePresent ()               { m_container->SetUnsupportedPrimitivePresent(); }
bool            GetUnsupportedPrimitivePresent () const         { return m_container->GetUnsupportedPrimitivePresent(); }

protected:

bvector<UInt32> m_beginOperation;
bvector<size_t> m_ifConditionalDraws;

public:

size_t          GetDataSize ()                                  { return m_container->GetDataSize(); }
void            WriteData (void const *pData, size_t dataSize)  { m_container->Write (pData, dataSize); }
void            WriteOpCode (XGraphicsOpCodes opCode)           { WriteUInt16 ((UInt16) opCode); }
void            WriteUInt16 (UInt16 value)                      { WriteData (&value, sizeof (value)); }
void            WriteUInt32 (UInt32 value)                      { WriteData (&value, sizeof (value)); }
void            WriteInt32 (Int32 value)                        { WriteData (&value, sizeof (value)); }
void            WriteElementId (ElementId value)                { WriteData (&value, sizeof (value)); }
void            WriteDouble (double value)                      { WriteData (&value, sizeof (value)); }
void            WriteBool (bool value)                          { WriteData (&value, sizeof (value)); }
void            WriteValues (double const* values, int n)       { WriteData ((void *) values, n * sizeof (*values)); }
void            WriteValues (int const* values, int n)          { WriteData ((void *) values, n * sizeof (*values)); }
void            WriteDistance (double value)                    { WriteData (&value, sizeof (value)); }
void            WriteScale (double value)                       { WriteData (&value, sizeof (value)); }
void            WriteDepth (double value)                       { WriteData (&value, sizeof (value)); }
void            WriteAngle (double value)                       { WriteData (&value, sizeof (value)); }
void            WritePoint (DPoint2dCP point)                   { WriteData ((void *) point, sizeof (*point)); }
void            WritePoint (DPoint3dCP  point)                  { WriteData ((void *) point, sizeof (*point)); }
void            WriteVector (DPoint2dCP bvector)                { WriteData ((void *) bvector, sizeof (*bvector)); }
void            WriteVector (DPoint3dCP bvector)                { WriteData ((void *) bvector, sizeof (*bvector)); }
void            WriteTransform (TransformCP transform)          { WriteData ((void *) transform, sizeof (*transform)); }
void            WriteRotMatrix (RotMatrixCP rotMatrix)          { WriteData ((void *) rotMatrix, sizeof (*rotMatrix)); }
void            WriteQuaternion (double const* quaternion)      { WriteData ((void *) quaternion, 4 * sizeof (*quaternion)); }
void            WriteEnableCap (bool enableCap)                 { WriteBool (enableCap); }
void            WriteVectors (int n, DPoint3dCP vectors)        { WritePoints (n, vectors); }
void            WriteParams (int n, DPoint2dCP params)          { WritePoints (n, params); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
~XGraphicsWriter ()
    {
    BeAssert (m_ifConditionalDraws.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AppendContainer (XGraphicsContainerR container)
    {
    WriteData (container.GetData() + sizeof (XGraphicsHeader), container.GetDataSize() - sizeof (XGraphicsHeader));

    BeAssert (0 == m_container->GetSymbolCount() || 0 == container.GetSymbolCount());       // If this is happening then we need to increment DrawSymbol opcodes.

    for (size_t i=0; i<container.GetSymbolCount(); i++)
        m_container->AddSymbol (container.GetSymbolId (i));
        
    if (container.UseCache())
        SetUseCache (true);

    if (container.GetBRepsPresent ())
        SetBRepsPresent (true);

    if (container.IsRenderable())
        SetIsRenderable (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeginOperation (XGraphicsOpCodes opCode)
    {
    WriteOpCode (opCode);

    m_beginOperation.push_back ((UInt32) GetDataSize());

    WriteUInt32 ((UInt32) 0);         // Words to follow - filled in later.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    EndOperation ()
    {
    if (m_beginOperation.empty())
        {
        BeAssert (false);
        return;
        }
    UInt32      beginOperation = m_beginOperation.back();
    m_beginOperation.pop_back();
    WriteUInt32AtLocation (beginOperation, (UInt32) GetDataSize() - beginOperation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    WriteDisplayFilterHandlerId (DisplayFilterHandlerId id)
    {

    WriteUInt16 (0) ;
    WriteUInt16 (HandlerId_DisplayFilter);
    WriteUInt16 ((UInt16) id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    WriteUInt32AtLocation (UInt32 location, UInt32 value)
    {
    memcpy (m_container->GetData() + location, &value, sizeof (value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteSymbol (BeRepositoryBasedId symbolId, TransformCR transform)
    {
    BeginOperation (XGRAPHIC_OpCode_DrawSymbol);
    WriteUInt32 ((UInt32) m_container->GetSymbolCount());
    WritePackedTransform (transform);
    EndOperation ();
    m_container->AddSymbol (symbolId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    BeginConditionalDraw ()
    {
    m_ifConditionalDraws.push_back (GetDataSize());
    WriteUInt32 ((UInt32) 0);
    m_container->SetFilterPresent ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    EndConditionalDraw ()
    {
    if (m_ifConditionalDraws.empty())
        return false;
               
    UInt32      blockSize = (UInt32)(GetDataSize() - m_ifConditionalDraws.back());

    memcpy (m_container->GetData() + m_ifConditionalDraws.back(), &blockSize, sizeof (blockSize));

    m_ifConditionalDraws.pop_back ();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            WriteStyleParams (LineStyleParamsCP styleParams)
    {
    int         paramBytes;
    byte        paramData[sizeof(LineStyleParams)];

    if ((paramBytes = LineStyleLinkageUtil::AppendModifiers (paramData, styleParams, true)) > 0)
        {
        WriteUInt16 ((UInt16) paramBytes);
        WriteData (paramData, paramBytes);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteGradientSymb (GradientSymbCP gradient)
    {
    if (NULL == gradient)
        {
        WriteUInt16 (0);
        return;
        }

    Display_attribute_gradient gradientAttr;

    if (SUCCESS != gradient->ToDisplayAttribute (gradientAttr))
        return;

    int         paramBytes = (sizeof (gradientAttr) - (MAX_GRADIENT_KEYS - gradientAttr.nKeys) * sizeof (GradientKey));

    WriteUInt16 ((UInt16) paramBytes);
    WriteData ((byte *) &gradientAttr, paramBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteFilled (bool value)
    {
    WriteData (&value, sizeof (value));

    if (value)
        SetValidForStencil (true); /* Can use for cut section pattern */

    SetIsRenderable (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteString (WChar const* pString)
    {
    if (NULL == pString)
        {
        WriteUInt32 ((UInt32) 0);
        }
    else
        {
        UInt32  nChars = (UInt32)(1 + wcslen (pString));

        WriteUInt32 (nChars);
        WriteData ((void*) pString, nChars * sizeof (*pString));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteRGBs (int n, FloatRgb const* pRgbs)
    {
    if (NULL == pRgbs)
        {
        WriteUInt32 ((UInt32) 0);
        }
    else
        {
        WriteUInt32 ((UInt32) n);
        WriteData ((void *) pRgbs, n * 3 * sizeof (float));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteIndices (int nIndices, int const* pIndices)
    {
    if (NULL == pIndices)
        {
        WriteUInt32 ((UInt32) 0);
        }
    else
        {
        WriteUInt32 ((UInt32) nIndices);
        WriteData ((void *) pIndices, nIndices * sizeof (*pIndices));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06                                                                                                    o
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteQuaternion (DVec3dCP primary, DVec3dCP secondary)
    {
    RotMatrix   rmatrix;
    double      quat[4];

    if (NULL == primary || NULL == secondary)
        {
        rmatrix.initIdentity ();
        }
    else
        {
        DVec3d  zVec;

        zVec.crossProduct (primary, secondary);
        rmatrix.initFromRowVectors (primary, secondary, &zVec);
        }

    rmatrix.getQuaternion (quat, false);

    WriteQuaternion (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteOperation (XGraphicsOpCodes opCode)
    {
    BeginOperation (opCode);
    EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WritePoints (int numPoints, DPoint3dCP points)
    {
    if (NULL == points)
        {
        WriteUInt32 ((UInt32) 0);
        }
    else
        {
        WriteUInt32 ((UInt32) numPoints);
        WriteData ((void *) points, numPoints * sizeof (*points));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WritePoints (int numPoints, DPoint2d const* points)
    {
    if (NULL == points)
        {
        WriteUInt32 ((UInt32) 0);
        }
    else
        {
        WriteUInt32 ((UInt32) numPoints);
        WriteData ((void *) points, numPoints * sizeof (*points));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteInts (int numInts, int const* values)
    {
    WriteUInt32 ((UInt32) numInts);
    WriteData ((void *) values, numInts * sizeof (int));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteClip (ClipPlaneSetCP clip)
    {
    if (NULL != clip)
        SetUnsupportedPrimitivePresent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBSplineParam (BsplineParamCR params)
    {
    WriteInt32 (params.order);
    WriteBool (0 != params.closed);
    WriteInt32 (params.numPoles);
    WriteInt32 (params.numKnots);
    WriteInt32 (params.numRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBSplineDisplay (BsplineDisplayCR display)
    {
    WriteBool (0 != display.polygonDisplay);
    WriteBool (0 != display.curveDisplay);
    WriteBool (0 != display.rulesByLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBSplineKnots (double const* knots, BsplineParam const* params)
    {
    WriteValues (knots, bspknot_numberKnots (params->numPoles, params->order, params->closed));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/12
//---------------------------------------------------------------------------------------
void WriteMatSymb (UInt16 mask, UInt32 maskExtended, UInt32 subElemIndex, bool writeSubMaterials, ElemMatSymbR matSymb, ElemDisplayParamsR displayParams, bool useMaterialId, Int64 materialId)
    {
    // Write incomplete/extra symbology first so Cook/Activate can be done once...
    if (0 != maskExtended)
        {
        BeginOperation (XGRAPHIC_OpCode_MatSymb2);
        WriteUInt32 (maskExtended);

       if (0 != (maskExtended & XMATSYMB2_GradientFill))
            WriteGradientSymb (displayParams.GetGradient ());

        EndOperation ();
        }

    if (0 == mask && 0 == maskExtended && (0 == subElemIndex || !writeSubMaterials))
        return;

    // Note - The order here is important. - The bit mask implies the values that should be read, but the
    // order they are read must match the order they were written.
    BeginOperation(XGRAPHIC_OpCode_MatSymb);
    WriteUInt16 (mask);

    if (0 != (mask & XMATSYMB_LineColorIndex))
        WriteInt32 (matSymb.GetLineColorIndex());

    if (0 != (mask & XMATSYMB_LineColor))
        WriteUInt32 (matSymb.GetLineColorTBGR());

    if (0 != (mask & XMATSYMB_FillColorIndex))
        WriteInt32 (matSymb.GetFillColorIndex());

    if (0 != (mask & XMATSYMB_FillColor))
        WriteUInt32 (matSymb.GetFillColorTBGR());

    if (0 != (mask & XMATSYMB_RasterWidth))
        WriteUInt32 (matSymb.GetWidth());

    if (0 != (mask & XMATSYMB_RasterPattern))
        WriteUInt32 (matSymb.GetRasterPattern());

    if (0 != (mask & XMATSYMB_FillDisplay))
        {
        if (FillDisplay::Blanking == displayParams.GetFillDisplay ())
            SetUseCache (true); // Blanking fill requires a QvElem...

        WriteInt32(static_cast<Int32>(displayParams.GetFillDisplay ()));
        }

    if (0 != (mask & XMATSYMB_ElemColor))
        WriteUInt32 (displayParams.GetLineColor ());

    if (0 != (mask & XMATSYMB_ElemFillColor))
        WriteUInt32 (displayParams.GetFillColor ());

    if (0 != (mask & XMATSYMB_ElemWeight))
        WriteUInt32 (displayParams.GetWeight ());

    if (0 != (mask & XMATSYMB_ElemTransparency))
        WriteDouble (displayParams.GetTransparency ());

    if (0 != (mask & XMATSYMB_SubElemIndex))
        WriteUInt32 (subElemIndex);

    if (0 != (mask & XMATSYMB_Material))
        {
        if (useMaterialId)
            WriteElementId(ElementId(materialId));
        else
            {
            MaterialCP  material = displayParams.GetMaterial ();
            WriteElementId (NULL == material ? ElementId() : ElementId(material->GetId().GetValue()));
            }
        }

    if (0 != (mask & XMATSYMB_ElemStyle))
        WriteInt32 (displayParams.GetLineStyle ());

    if (0 != (mask & XMATSYMB_ElemColorRGB))
        WriteUInt32 (displayParams.GetLineColorTBGR ());

    if (0 != (mask & XMATSYMB_StyleParams))
        WriteStyleParams (displayParams.GetLineStyleParams ());

    EndOperation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            WriteSolidKernelEntity (ISolidKernelEntityCR entity)
    {
    WriteTransform (&entity.GetEntityTransform ());
    WriteUInt32 ((UInt32) ISolidKernelEntity::SolidKernel_PSolid);

    UInt32      bufferSize  = 0;
    void*       buffer      = NULL;

    if (SUCCESS == T_HOST.GetSolidsKernelAdmin()._SaveEntityToMemory (&buffer, bufferSize, entity))
        {
        // NOTE: Don't need to free buffer - it is owned by entity and will be freed when it goes out of scope...
        WriteUInt32 (bufferSize);
        WriteData (buffer, bufferSize);
        }

    /* NOTE: Tell xgraphics reader that this XGContainer cannot be used *directly* to
             create a qvElem for stenciling or is a Triforma sheet body w/hidden
             edges needs to be re-stroked w/DRAW_OPTION_ClipStencil... */
    SetUseForStencil (T_HOST.GetSolidsKernelAdmin()._QueryEntityData (entity, ISolidKernelEntity::EntityQuery_HasHiddenEdge) && ISolidKernelEntity::EntityType_Sheet == entity.GetEntityType ());
    }

public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     12/12
//---------------------------------------------------------------------------------------
void WriteNoOp ()
    {
    BeginOperation (XGRAPHIC_OpCode_NoOp);
    EndOperation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void WritePolyface (PolyfaceQueryCR facetSet, bool triangulate)
    {
    if (0 == facetSet.GetPointIndexCount ())
        {
        BeAssert (false);

        return;
        }

    PolyfaceHeaderPtr   triangulatedFacets;
    PolyfaceQueryCP     effectiveFacets = &facetSet;

    if (triangulate && !facetSet.IsTriangulated ())
        {
        triangulatedFacets = PolyfaceHeader::CreateVariableSizeIndexed ();
        triangulatedFacets->CopyFrom (facetSet);
        triangulatedFacets->Triangulate ();
        effectiveFacets = triangulatedFacets.get ();
        }

    // Some of the facet builders build populate normals without indices (if not normals not requested).
    DPoint3dCP      normals = (NULL == effectiveFacets->GetNormalIndexCP()) ? NULL : effectiveFacets->GetNormalCP();
    DPoint2dCP      params  = (NULL == effectiveFacets->GetParamIndexCP())  ? NULL : effectiveFacets->GetParamCP();
    FloatRgb const* color   = (NULL == effectiveFacets->GetFloatColorCP())  ? NULL : effectiveFacets->GetFloatColorCP();  

    size_t          normalCount = NULL == normals ? 0 : effectiveFacets->GetNormalCount ();
    size_t          paramCount  = NULL == params  ? 0 : effectiveFacets->GetParamCount ();
    size_t          colorCount  = NULL == color   ? 0 : effectiveFacets->GetColorCount ();

    SetUseCache (true);
    SetIsRenderable (true);

    BeginOperation (XGRAPHIC_OpCode_AddIndexPolys);
    WriteUInt32 ((UInt32) effectiveFacets->GetNumPerFace ()); // PolySize.
    WriteUInt32 ((UInt32) effectiveFacets->GetPointIndexCount ());
    WriteIndices ((int) effectiveFacets->GetPointIndexCount (), effectiveFacets->GetPointIndexCP ());
    WriteIndices (NULL == effectiveFacets->GetNormalIndexCP () ? 0 : (int) effectiveFacets->GetPointIndexCount (), effectiveFacets->GetNormalIndexCP ());
    WriteIndices (NULL == effectiveFacets->GetParamIndexCP () ? 0 : (int) effectiveFacets->GetPointIndexCount (), effectiveFacets->GetParamIndexCP ());
    WritePoints ((int) effectiveFacets->GetPointCount (), effectiveFacets->GetPointCP ());
    WritePoints ((int) normalCount, normals);
    WritePoints ((int) paramCount, params);
    WriteRGBs ((int) colorCount, effectiveFacets->GetFloatColorCP ());
    WriteIndices (NULL == effectiveFacets->GetColorIndexCP () ? 0 : (int) effectiveFacets->GetPointIndexCount (), effectiveFacets->GetColorIndexCP ());
    WriteString (effectiveFacets->GetIlluminationNameCP ());
    EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBsplineCurve (MSBsplineCurveCR bCurve, bool filled)
    {
    BeginOperation (XGRAPHIC_OpCode_DrawBSplineCurve);
    WriteBool (filled);
    WriteInt32 (bCurve.type);
    WriteBool (0 != bCurve.rational);
    WriteBSplineDisplay (bCurve.display);
    WriteBSplineParam (bCurve.params);

    WriteData ((void *) bCurve.poles, bCurve.params.numPoles * sizeof (DPoint3d));
    WriteBSplineKnots (bCurve.knots, &bCurve.params);

    if (bCurve.rational)
        WriteValues (bCurve.weights, bCurve.params.numPoles);

    EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteBsplineSurface (MSBsplineSurfaceCR bSurface)
    {
    SetIsRenderable (true);
    BeginOperation (XGRAPHIC_OpCode_DrawBSplineSurface);
    WriteInt32 (bSurface.type);
    WriteBool (0 != bSurface.rational);
    WriteBSplineDisplay (bSurface.display);
    WriteBSplineParam (bSurface.uParams);
    WriteBSplineParam (bSurface.vParams);

    int         nPoles = bSurface.uParams.numPoles * bSurface.vParams.numPoles;

    WriteData ((void *) bSurface.poles, nPoles * sizeof (DPoint3d));
    WriteBSplineKnots (bSurface.uKnots, &bSurface.uParams);
    WriteBSplineKnots (bSurface.vKnots, &bSurface.vParams);

    if (bSurface.rational)
        WriteValues (bSurface.weights, nPoles);

    WriteBool (0 != bSurface.holeOrigin);
    WriteInt32 (bSurface.GetIntNumBounds ());

    for (int i=0; i<bSurface.GetIntNumBounds (); i++)
        WritePoints (bSurface.GetIntNumPointsInBoundary (i), bSurface.GetBoundaryUVCP (i));

    EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteCone (DVec3dCP primary, DVec3dCP secondary, DPoint3dCP center0, DPoint3dCP center1, double r0, double r1, bool cap)
    {
    SetIsRenderable (true);
    BeginOperation (XGRAPHIC_OpCode_DrawCone);
    WriteVector (primary);
    WriteVector (secondary);
    WritePoint (center0);
    WritePoint (center1);
    WriteDistance (r0);
    WriteDistance (r1);
    WriteBool (cap);
    EndOperation ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/12
//---------------------------------------------------------------------------------------
void WriteCone (DgnConeDetailCR cone)
    {
    WriteCone (&cone.m_vector0, &cone.m_vector90, &cone.m_centerA, &cone.m_centerB, cone.m_radiusA, cone.m_radiusB, cone.m_capped);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/12
//---------------------------------------------------------------------------------------
void WriteSphere (DgnSphereDetailCR sphere)
    {
    double      rXY, rZ;
    DVec3d      unitX, unitY, unitZ;
    DPoint3d    center;
    RotMatrix   axes;
    bool        isTrueSphere = sphere.IsTrueSphere (center, axes, rZ);

    if (!sphere.IsTrueRotationAroundZ (center, unitX, unitY, unitZ, rXY, rZ))
        return;

    SetIsRenderable (true);
    BeginOperation (XGRAPHIC_OpCode_DrawSphere);
    WriteVector (&unitX);
    WriteVector (&unitY); // NOTE: Original IViewDraw::DrawSphere input was X vec, Y vec, center, XY radius, Z radius, start, sweep...
    WritePoint (&center);
    WriteDistance (rXY);
    WriteDistance (rZ);
    WriteAngle (isTrueSphere ? -msGeomConst_piOver2 : sphere.m_startLatitude);
    WriteAngle (isTrueSphere ? msGeomConst_pi : sphere.m_latitudeSweep);
    EndOperation ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/12
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
void WriteTorus (DgnTorusPipeDetailCR torus)
    {
    SetIsRenderable (true);
    BeginOperation (XGRAPHIC_OpCode_DrawTorus);
    WriteVector (&torus.m_vectorX);
    WriteVector (&torus.m_vectorY);
    WritePoint (&torus.m_center);
    WriteDistance (torus.m_majorRadius);
    WriteDistance (torus.m_minorRadius);
    WriteAngle (torus.m_sweepAngle);
    WriteBool (torus.m_capped);
    EndOperation ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/12
//---------------------------------------------------------------------------------------
void WriteBox (DgnBoxDetailCR box)
    {
    DVec3dCP    primary     = &box.m_vectorX;
    DVec3dCP    secondary   = &box.m_vectorY;
    DPoint3dCP  baseOrigin  = &box.m_baseOrigin;
    DPoint3dCP  topOrigin   = &box.m_topOrigin;
    double      baseWidth   = box.m_baseX;
    double      baseLength  = box.m_baseY;
    double      topWidth    = box.m_topX;
    double      topLength   = box.m_topY;
    bool        cap         = box.m_capped;
    
    DPoint3d    baseCenter, topCenter;

    // NOTE: Need to store centers not origins for compatibility...
    baseCenter = DPoint3d::FromSumOf (*baseOrigin, *primary, baseWidth * 0.5);
    baseCenter = DPoint3d::FromSumOf (baseCenter, *secondary, baseLength * 0.5);
          
    topCenter = DPoint3d::FromSumOf (*topOrigin, *primary, topWidth * 0.5);
    topCenter = DPoint3d::FromSumOf (topCenter, *secondary, topLength * 0.5);

    SetIsRenderable (true);
    BeginOperation (XGRAPHIC_OpCode_DrawBox);
    WriteVector (primary);
    WriteVector (secondary);
    WritePoint (&baseCenter);
    WritePoint (&topCenter);
    WriteDistance (baseWidth);
    WriteDistance (baseLength);
    WriteDistance (topWidth);
    WriteDistance (topLength);
    WriteBool (cap);
    EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            WritePackedTransform (TransformCR transform)
    {
    double      packedTransform[12];
    int         packedTransformSize = packTransform (packedTransform, transform);

    m_container->Write (packedTransform, packedTransformSize);
    }
}; // XGraphicsWriter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetXGraphicsFacetOptions (IFacetOptionsR options, bool wantNormals = false, bool wantParams = false)
    {
    options.SetMaxPerFace (3);
    options.SetChordTolerance (0.0);
    options.SetAngleTolerance (0.2);
    options.SetNormalsRequired(wantNormals);
    options.SetParamsRequired(wantParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/12
//---------------------------------------------------------------------------------------
static StatusInt facetTopologyTableFromBody (IFacetTopologyTablePtr& table, TransformR transform, byte const* pData, UInt32 opSize, byte const* pEnd, bool planarOnly)
    {
    UInt32 kernelType, bufferSize;

    GET_AND_INCREMENT (transform);
    GET_AND_INCREMENT (kernelType);
    GET_AND_INCREMENT (bufferSize);

    ISolidKernelEntityPtr   entityPtr;
    StatusInt               status;

    if (SUCCESS != (status = T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory (entityPtr, pData, bufferSize, (ISolidKernelEntity::SolidKernelType) kernelType, transform)))
        return status;

    if (planarOnly && T_HOST.GetSolidsKernelAdmin()._QueryEntityData (*entityPtr.get (), DgnPlatform::ISolidKernelEntity::EntityQuery_HasCurvedFaceOrEdge))
        return ERROR;

    IFacetOptionsPtr options = IFacetOptions::New ();
    SetXGraphicsFacetOptions (*options, true);

    if (SUCCESS != (status = T_HOST.GetSolidsKernelAdmin()._FacetBody (table, *entityPtr.get (), *options, NULL)))
        return status;

#if !defined(NDEBUG)
    //  I don't understand why the Graphite05 version did transform = entityPtr->GetEntityTransform(); here
    Transform temp = entityPtr->GetEntityTransform();
    BeAssert(!memcmp(&transform, &temp, sizeof temp));
#endif

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
XGraphicsOperationContext::XGraphicsOperationContext (XGraphicsHeader* hdr, DgnProjectR project, T_XGraphicsSymbolIds& symbolIds, ElementHandleCP element, XGraphicsContainer::DrawOptions drawOptions)
 : m_dgnProject (project), m_symbolIds (symbolIds), m_element (element)
    {
    m_header = hdr;
    m_brepOffset = 0;
    m_baseElemId = 0;
    m_inMultiSymbBody = false;
    m_drawOptions = drawOptions;
    m_sizeDependentGeometryExists = false;
    m_filled = false;
    m_is3d = true;
    m_zDepth = 0.0;
    m_solidPrimitiveType = SolidPrimitiveType_None;

    m_sourceElement = NULL;
    m_projection = NULL;
    m_extrusion = NULL;
    m_revolution = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/13
//---------------------------------------------------------------------------------------
XGraphicsOperationContext::~XGraphicsOperationContext()
    {
    delete m_projection;
    delete m_extrusion;
    delete m_revolution;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::AllowDrawStyled (ViewContextR context, bool isRenderable)
    {
    // Display of custom linestyles is only supported for immediate mode calls in wireframe views...
    return (NULL != context.GetCurrLineStyle (NULL) && !context.CheckICachedDraw () && 0 != (context.GetDisplayInfo (isRenderable) & DISPLAY_INFO_Edge));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::AllowDrawFilled (ViewContextR context, bool isFilled)
    {
    // Display of fill needs to check fill display state of current display params...
    // NOTE: When not m_useCache and fill display is off, fill boundary can be redundant with outline that is sometimes explictly stored...yuck.
    return (isFilled && (context.GetDisplayInfo (true) & (DISPLAY_INFO_Fill | DISPLAY_INFO_Surface)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::UseUnCachedDraw (ViewContextR context, bool isFilled)
    {
    // When linestyles are possible (including overrides!) any un-filled open/close path can't be draw cached...
    return (NULL != context.GetElemMatSymb ()->GetLineStyleSymbR ().GetILineStyle ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::DrawStyledCurveVector (ViewContextR context, CurveVectorCR curves, bool isFilled, bool is3d, double zDepth)
    {
    bool    displayFilled = AllowDrawFilled (context, isFilled);

    if (!displayFilled && AllowDrawStyled (context, curves.IsAnyRegionType ()))
        {
        if (is3d)
            context.DrawStyledCurveVector3d (curves);
        else
            context.DrawStyledCurveVector2d (curves, zDepth);

        return;
        }

    if (!isFilled && context.GetIViewDraw ().IsOutputQuickVision ())
        {
        // Never filled, output open geometry that won't render as a surface when added to QvElem...
        if (is3d)
            WireframeGeomUtil::DrawOutline (curves, context.GetIDrawGeom ());
        else
            WireframeGeomUtil::DrawOutline2d (curves, context.GetIDrawGeom (), zDepth);
        }
    else
        {
        if (is3d)
            context.GetIDrawGeom ().DrawCurveVector (curves, displayFilled);
        else
            context.GetIDrawGeom ().DrawCurveVector2d (curves, displayFilled, zDepth);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::BeginComplex (bool isClosed, bool filled)
    {
    BeAssert (!m_curve.IsValid ());
    m_filled = filled;
    m_is3d = true;
    m_zDepth = 0.0;

    m_curve = CurveVector::Create (isClosed ? (0 != m_loops.size () ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer) : CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::IsComplexComponent ()
    {
    return (SolidPrimitiveType_None != m_solidPrimitiveType || m_curve.IsValid () || 0 != m_loops.size ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::AddComplexComponent (ICurvePrimitivePtr& curvePrimitive, bool is3d, double zDepth)
    {
    if (!m_curve.IsValid ()) // Sweeps don't always need to call BeginComplex do they?
        {
        switch (m_solidPrimitiveType)
            {
            case SolidPrimitiveType_DgnExtrusion:
                BeginComplex (m_extrusion->m_capped, false);
                break;

            case SolidPrimitiveType_DgnRotationalSweep:
                BeginComplex (m_revolution->m_capped, false);
                break;

            case SolidPrimitiveType_DgnRuledSweep:
                BeginComplex (m_projection->m_capped, false);
                break;

            default:
                {
                // Save zDepth for EndComplex. NOTE: Bspline curve xgraphics always 3d, that's why is3d is initialized to true and only ever set to false here...
                if (!is3d)
                    {
                    m_is3d = false;
                    m_zDepth = zDepth;
                    }

                if (0 != m_loops.size ())
                    BeginComplex (true, m_filled); // Start of inner loop...
                break;
                }
            }
        }

    BeAssert (m_curve.IsValid ());
    m_curve->push_back (curvePrimitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::AddDisconnect ()
    {
    BeAssert (m_curve.IsValid ());
    m_loops.push_back (m_curve);
    m_curve = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::EndComplex (ViewContextR context)
    {
    //  #ifdef NEEDS_WORK_TopazMerge_ Why did Matt have to add this
    BeAssert(m_curve.IsValid());
    if (!m_curve.IsValid())
        {
        m_loops.clear();
        m_curve = NULL;
        m_solidPrimitiveType = SolidPrimitiveType_None;
        return;
        }

    if (0 != m_loops.size ())
        {
        m_loops.push_back (m_curve);
        m_curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);

        for (CurveVectorPtr loopCurve: m_loops)
            m_curve->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*loopCurve));

        m_loops.clear ();
        }

    switch (m_solidPrimitiveType)
        {
        case SolidPrimitiveType_DgnExtrusion:
            m_extrusion->m_baseCurve = m_curve;
            break;

        case SolidPrimitiveType_DgnRotationalSweep:
            m_revolution->m_baseCurve = m_curve;
            break;

        case SolidPrimitiveType_DgnRuledSweep:
            m_projection->m_sectionCurves.push_back (m_curve);
            break;

        default:
            DrawStyledCurveVector (context, *m_curve, m_filled, m_is3d, m_zDepth);
            break;
        }

    m_curve = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::BeginSweepProject (bool capped)
    {
    BeAssert (SolidPrimitiveType_None == m_solidPrimitiveType);
    m_solidPrimitiveType = SolidPrimitiveType_DgnRuledSweep;
    if (!m_projection)
        m_projection = new DgnRuledSweepDetail();
    m_projection->m_capped = capped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::BeginSweepExtrude (bool capped, DVec3dCR extrusionVector)
    {
    BeAssert (SolidPrimitiveType_None == m_solidPrimitiveType);
    m_solidPrimitiveType = SolidPrimitiveType_DgnExtrusion;
    if (!m_extrusion)
        m_extrusion = new DgnExtrusionDetail();
    m_extrusion->m_capped = capped;
    m_extrusion->m_extrusionVector = extrusionVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::BeginSweepRevolve (bool capped, DRay3dCR axisOfRotation, double sweepAngle)
    {
    BeAssert (SolidPrimitiveType_None == m_solidPrimitiveType);
    m_solidPrimitiveType = SolidPrimitiveType_DgnRotationalSweep;
    if (!m_revolution)
        m_revolution = new DgnRotationalSweepDetail();
    m_revolution->m_capped = capped;
    m_revolution->m_axisOfRotation = axisOfRotation;
    m_revolution->m_sweepAngle = sweepAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::EndSweep (ViewContextR context)
    {
    BeAssert (SolidPrimitiveType_None != m_solidPrimitiveType);

    if (m_curve.IsValid ())
        EndComplex (context); // Add final profile curve if not handled by EndComplex...

    ISolidPrimitivePtr  primitive;

    switch (m_solidPrimitiveType)
        {
        case SolidPrimitiveType_DgnExtrusion:
            if (m_extrusion->m_baseCurve.IsValid())
                primitive = ISolidPrimitive::CreateDgnExtrusion (*m_extrusion);
            m_extrusion->m_baseCurve = NULL;
            break;

        case SolidPrimitiveType_DgnRotationalSweep:
            if (m_revolution->m_baseCurve.IsValid())
                primitive = ISolidPrimitive::CreateDgnRotationalSweep (*m_revolution);
            m_revolution->m_baseCurve = NULL;
            break;

        case SolidPrimitiveType_DgnRuledSweep:
            if (!m_projection->m_sectionCurves.empty())
                primitive = ISolidPrimitive::CreateDgnRuledSweep (*m_projection);
            m_projection->m_sectionCurves.clear ();
            break;
        }

    if (primitive.IsValid ())
        context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);

    m_solidPrimitiveType = SolidPrimitiveType_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
double XGraphicsOperationContext::ComputeSolidPixelSize (DRange3dCR bodyRange, ViewContextR context, TransformCR transform)
    {
    DPoint3d    center;
    DRange3d    transformedRange;

    transform.multiply (&transformedRange, &bodyRange);
    center.interpolate (&transformedRange.low, 0.5, &transformedRange.high);

    return context.GetPixelSizeAtPoint (&center); // pixelSize in UORs
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::AllowDrawWireframe (ViewContextR context)
    {
    if (context.CheckICachedDraw () || context.GetIViewDraw ().IsOutputQuickVision ())
        return false; // Wireframe will be cached in QVElem...

    // Display of wireframe is only needed for immediate mode calls in wireframe views...
    return (0 != (context.GetDisplayInfo (true) & DISPLAY_INFO_Edge));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::DrawBodyAndCacheEdges (ViewContextR context, ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments)
    {
    double  pixelSize = 0.0;

    if (T_HOST.GetSolidsKernelAdmin ()._QueryEntityData (entity, DgnPlatform::ISolidKernelEntity::EntityQuery_HasCurvedFaceOrEdge))
        {
        DRange3d    bodyRange;

        // compute pixelSize (in UORs) to avoid coarse tristripping
        if (SUCCESS == T_HOST.GetSolidsKernelAdmin ()._GetEntityRange (bodyRange, entity))
            pixelSize = ComputeSolidPixelSize (bodyRange, context, entity.GetEntityTransform ());

        m_sizeDependentGeometryExists = true;
        }

    // Create wireframe geometry cache for locate/snapping...
    BRepWireGraphicsAppData::CreateWireGeomCache (*this, context, entity, attachments);

    if (!AllowDrawWireframe (context) ||
        SUCCESS != BRepWireGraphicsAppData::DrawWireGeomCache (*this, context))
        context.GetIDrawGeom ().DrawBody (entity, attachments, m_sizeDependentGeometryExists ? pixelSize : 0.0);
        
    if (NULL == attachments || attachments->_GetFaceToSubElemIdMap ().empty ())
        return;

    for (T_FaceToSubElemIdMap::const_iterator curr = attachments->_GetFaceToSubElemIdMap ().begin (); curr != attachments->_GetFaceToSubElemIdMap ().end (); ++curr)
        {
        if (curr->second > m_baseElemId)
            m_baseElemId = curr->second;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::WantMultiSymbologyBody ()
    {
    return (0 == (m_drawOptions & XGraphicsContainer::DRAW_OPTION_ClipStencil));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::IsMultiSymbologyBodyValid ()
    {
    return (m_inMultiSymbBody && m_brepEntity.IsValid () && m_faceAttachments.IsValid ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::BeginMultiSymbologyBody ()
    {
    if (!WantMultiSymbologyBody ())
        return;

    BeAssert (!IsMultiSymbologyBodyValid ());

    m_inMultiSymbBody = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::EndMultiSymbologyBody (ViewContextR context)
    {
    if (!WantMultiSymbologyBody ())
        return;

#if defined (NEEDS_WORK_DGNITEM)
    BeAssert (!context.CheckICachedDraw () || IsMultiSymbologyBodyValid ()); // Only assert on cacheable pass...not wireframe pass...
#endif

    if (IsMultiSymbologyBodyValid ())
        DrawBodyAndCacheEdges (context, *m_brepEntity, m_faceAttachments.get ());

    m_brepEntity = NULL;
    m_faceAttachments = NULL;
    m_inMultiSymbBody = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperationContext::IsCurvePrimitiveRequired ()
    {
    return (IsComplexComponent () || m_curvePrimitiveId.IsValid ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::DrawOrAddLineString (DPoint3dCP points, size_t nPoints, ViewContextR context, bool isClosed, bool isFilled, bool is3d, double zDepth)
    {
    bvector <UInt32>    associationIds;
    int                 topologyIdType;

    if (DrawPurpose::Pick == context.GetDrawPurpose() &&
        m_curvePrimitiveId.IsValid () &&
        SUCCESS == m_curvePrimitiveId->GetLineStringAssociationIds (topologyIdType, associationIds, nPoints))
        {
        CurveVectorPtr  curves = IsComplexComponent() ? CurveVectorPtr() : CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

        for (size_t i=0; i<nPoints-1; i++)
            {
            ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLineString (&points[i], 2);
            CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create (m_curvePrimitiveId->GetType(), CurveTopologyId ((CurveTopologyId::Type) topologyIdType, &associationIds[i], 2), m_curvePrimitiveId->GetCompoundDrawState().get());

            primitive->SetId (newId.get());

            if (IsComplexComponent())
                AddComplexComponent (primitive, is3d, zDepth);
            else
                curves->push_back (primitive);
            }

        if (curves.IsValid ())
            DrawStyledCurveVector (context, *curves, isFilled, is3d, zDepth);
        }
    else
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLineString (points, nPoints);

        DrawOrAddComplexComponent (primitive, context, isClosed, isFilled, is3d, zDepth);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperationContext::DrawOrAddComplexComponent (ICurvePrimitivePtr& primitive, ViewContextR context, bool isClosed, bool isFilled, bool is3d, double zDepth)
    {
    primitive->SetId (m_curvePrimitiveId.get());
    m_curvePrimitiveId = NULL;

    if (IsComplexComponent ())
        {
        AddComplexComponent (primitive, is3d, zDepth);
        return;
        }

    // Shouldn't need to worry about linestyles w/edge id...
    CurveVectorPtr  curve = CurveVector::Create (isClosed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, primitive);

    DrawStyledCurveVector (context, *curve, isFilled, is3d, zDepth);
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsDumpOperator : XGraphicsOperator
{
    XGraphicsOperationContextR  m_opContext;
    byte const*                 m_pBase;

XGraphicsDumpOperator (XGraphicsOperationContextR opContext, byte const* pBase) : m_opContext (opContext), m_pBase(pBase) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _DoOperation (XGraphicsOperationR operation, byte* pData, UInt32 size, byte* pEnd, XGraphicsOpCodes opCode) override
    {
    printf ("Location: %d, OpCode: %d, DataSize: %d\t", (int)(pData - m_pBase), (int)opCode, (int)size);
    operation._Dump (pData, size, m_opContext);

    return SUCCESS;
    }
}; // XGraphicsDumpOperator

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          AppendComplexComponentsToGpaOperator : XGraphicsOperator
{
GPArrayR        m_gpa;

AppendComplexComponentsToGpaOperator (GPArrayR gpa) : m_gpa (gpa) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _DoOperation (struct XGraphicsOperation& operation, byte* pData, UInt32 size, byte* pEnd, XGraphicsOpCodes opCode) override
    {
    return (XGRAPHIC_OpCode_EndComplex == opCode) ? ERROR : operation._AppendToGPA (pData, size, m_gpa);
    }

}; // AppendComplexComponentsToGpaOperator

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      04/2008
+===============+===============+===============+===============+===============+======*/
struct          XGraphicAnnotationData : XGraphicsOperation
{

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("AnnotationData - %d bytes: {", (int)dataSize);
    byte const* pX = pData + dataSize;
    for (byte const* p = pData; p < pX; ++p)
        printf ("%x ", *p);

    printf ("}\n");

    XGraphicsAnnotationData::Signature sig = (XGraphicsAnnotationData::Signature) *(UInt32*) pData;

    if (XGraphicsAnnotationData::SIGNATURE_EdgeId == sig)
        {
        void*                   curvePrimitiveIdData = (void *) (pData + sizeof (sig));
        size_t                  curvePrimitiveIdSize = (dataSize - sizeof (sig));
        CurvePrimitiveIdPtr     curvePrimitiveId = CurvePrimitiveId::Create (curvePrimitiveIdData, curvePrimitiveIdSize);

        printf (" %S", curvePrimitiveId->GetDebugString().c_str());
        }
    printf ("\n");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    BeAssert (false); // NEEDS WORK - unimplemented
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 opSize, XGraphicsOperationContextR opContext) override
    {
    // The primitive ID is only used for picking.... and creating it can take a significant amount of processing time....
    if (DrawPurpose::ExportVisibleEdges != context.GetDrawPurpose() &&
        DrawPurpose::Pick != context.GetDrawPurpose())
        return SUCCESS;

    XGraphicsAnnotationData::Signature sig = (XGraphicsAnnotationData::Signature) *(UInt32*) pData;

    if (sig != XGraphicsAnnotationData::SIGNATURE_EdgeId)
        return SUCCESS;

    void*       curvePrimitiveIdData = (void *) (pData + sizeof (sig));
    size_t      curvePrimitiveIdSize = (opSize - sizeof (sig));

    opContext.m_curvePrimitiveId = CurvePrimitiveId::Create (curvePrimitiveIdData, curvePrimitiveIdSize);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt  _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    }

}; // XGraphicAnnotationData

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawUnimplemented : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
   printf ("Unimplemented Draw Function\n");
    return ERROR;
    }

}; // XGraphicsDrawUnimplemented

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawLineString3d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;

    GET_AND_INCREMENT (numPoints);
    DPoint3dCP  points = (DPoint3dCP) pData;

    printf ("DrawLineString3d - %d Points:\n", numPoints);
    for (int i=0; i<numPoints; ++i)
        printf ("\t%lf,%lf,%lf\n", points[i].x, points[i].y, points[i].z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.UseUnCachedDraw (context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;

    GET_AND_INCREMENT (numPoints)

    if (opContext.IsCurvePrimitiveRequired ())
        {
        opContext.DrawOrAddLineString ((DPoint3dCP) pData, numPoints, context, false, false, true, 0.0);

        return SUCCESS;
        }

    XGScopedArray ptBuffer;
    ByteCP pts = ptBuffer.Acquire (pData, numPoints * sizeof (DPoint3d));

    if (opContext.AllowDrawStyled (context, false))
        context.DrawStyledLineString3d (numPoints, (DPoint3dCP) pts, NULL);
    else
        context.GetIDrawGeom ().DrawLineString3d (numPoints, (DPoint3dCP) pts, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    byte*       pData = &data[0];

    GET_AND_INCREMENT (numPoints)
    transformPoints ((DPoint3dP) pData, numPoints, *transform.GetTransform());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    int         numPoints;

    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptsBuffer;
    ByteCP pts = ptsBuffer.Acquire (pData, (numPoints * sizeof (DPoint3d)));

    return getBasisTransformFromPoints (transform, (DPoint3dCP)pts, NULL, numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    int         thisNumPoints, rhsNumPoints;

    GET_AND_INCREMENT_DATA (thisNumPoints, data);
    GET_AND_INCREMENT_DATA (rhsNumPoints, rhsData);

    XGScopedArray thisPointsBuffer, rhsPointsBuffer;
    ByteCP thisPoints = thisPointsBuffer.Acquire (  data, (thisNumPoints * sizeof (DPoint3d)));
    ByteCP rhsPoints  =  rhsPointsBuffer.Acquire (rhsData, (rhsNumPoints * sizeof (DPoint3d)));

    return arePointsEqual ((DPoint3dCP) thisPoints, thisNumPoints, (DPoint3dCP) rhsPoints,  rhsNumPoints, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _AppendToGPA (byte const* pData, UInt32 dataSize, GPArrayR gpa) override
    {
    int         numPoints;

    GET_AND_INCREMENT (numPoints);
    gpa.Add ((DPoint3dCP) pData, numPoints);

    return SUCCESS;
    }

}; // XGraphicsDrawLineString3d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawLineString2d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;
    double      depth;

    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    printf ("DrawLineString2d - %d Points\n", numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.UseUnCachedDraw (context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {

    int         numPoints;
    double      depth;

    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptBuffer;
    DPoint2dCP pts = (DPoint2dCP) ptBuffer.Acquire (pData, numPoints * sizeof (DPoint3d));

    if (opContext.IsCurvePrimitiveRequired ())
        {
        std::valarray<DPoint3d>  localPointsBuf (numPoints);

        for (int iPt = 0; iPt < numPoints; ++iPt)
            localPointsBuf[iPt].Init ((pts+iPt)->x, (pts+iPt)->y, depth);

        opContext.DrawOrAddLineString (&localPointsBuf[0], numPoints, context, false, false, false, depth);

        return SUCCESS;
        }

    if (opContext.AllowDrawStyled (context, false))
        context.DrawStyledLineString2d (numPoints, pts, depth, NULL);
    else
        context.GetIDrawGeom ().DrawLineString2d (numPoints, pts, depth, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    double      depth;
    byte*       pData = &data[0];

    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);
    transformDPoint2dArray ((DPoint2dP) pData, depth, numPoints, transform.GetTransform());

    return SUCCESS;
    }

}; // XGraphicsDrawLineString2d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawShape2d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;
    double      depth;
    int         numPoints;

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    printf ("DrawShape2d: Filled: %d, NumPoints: %d\n", filled, numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;

    GET_AND_INCREMENT (filled);

    return opContext.UseUnCachedDraw (context, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;
    double      depth;
    int         numPoints;

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray           ptBuffer;
    DPoint2dCP              pts = (DPoint2dCP) ptBuffer.Acquire (pData, numPoints * sizeof (DPoint2d));

    if (opContext.IsCurvePrimitiveRequired ())
        {
        std::valarray<DPoint3d>  localPointsBuf (numPoints);

        for (int iPt = 0; iPt < numPoints; ++iPt)
            localPointsBuf[iPt].Init ((pts+iPt)->x, (pts+iPt)->y, depth);

        opContext.DrawOrAddLineString (&localPointsBuf[0], numPoints, context, true, filled, false, depth);

        return SUCCESS;
        }

    bool    displayFilled = opContext.AllowDrawFilled (context, filled);

    if (!displayFilled && opContext.AllowDrawStyled (context, true))
        context.DrawStyledLineString2d (numPoints, pts, depth, NULL, true);
    else if (!filled && context.GetIViewDraw ().IsOutputQuickVision ()) // Never filled, output open geometry that won't render as a surface when added to QvElem...
        context.GetIDrawGeom ().DrawLineString2d (numPoints, pts, depth, NULL);
    else
        context.GetIDrawGeom ().DrawShape2d (numPoints, pts, displayFilled, depth, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    double      depth;
    byte*       pData = &data[0];

    pData += sizeof (bool);     // Fill.
    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);
    transformDPoint2dArray ((DPoint2dP) pData, depth, numPoints, transform.GetTransform());

    return SUCCESS;
    }

}; // XGraphicsDrawShape2d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawShape3d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;
    int         numPoints;

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (numPoints);

    printf ("DrawShape3d: Filled: %d, NumPoints: %d\n", filled, numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;

    GET_AND_INCREMENT (filled);

    return opContext.UseUnCachedDraw (context, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;
    int         numPoints;

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptsBuffer;
    ByteCP pts = ptsBuffer.Acquire (pData, (numPoints * sizeof (DPoint3d)));

    if (opContext.IsCurvePrimitiveRequired ())
        {
        opContext.DrawOrAddLineString ((DPoint3dCP)pts, numPoints, context, true, filled, true, 0.0);

        return SUCCESS;
        }

    bool    displayFilled = opContext.AllowDrawFilled (context, filled);

    if (!displayFilled && opContext.AllowDrawStyled (context, true))
        context.DrawStyledLineString3d (numPoints, (DPoint3dCP)pts, NULL, true);
    else if (!filled && context.GetIViewDraw ().IsOutputQuickVision ()) // Never filled, output open geometry that won't render as a surface when added to QvElem...
        context.GetIDrawGeom ().DrawLineString3d (numPoints, (DPoint3dCP)pts, NULL);
    else
        context.GetIDrawGeom ().DrawShape3d (numPoints, (DPoint3dCP)pts, displayFilled, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    byte*       pData = &data[0];

    pData += sizeof (bool);     // Fill.
    GET_AND_INCREMENT (numPoints);
    transformPoints ((DPoint3dP) pData, numPoints, *transform.GetTransform());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    bool        filled;
    int         numPoints;

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptsBuffer;
    ByteCP pts = ptsBuffer.Acquire (pData, (numPoints * sizeof (DPoint3d)));

    return getBasisTransformFromPoints (transform, (DPoint3dCP)pts, NULL, numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    bool        thisFilled, rhsFilled;
    int         thisNumPoints, rhsNumPoints;

    GET_AND_INCREMENT_DATA (thisFilled, data);
    GET_AND_INCREMENT_DATA (thisNumPoints, data);

    GET_AND_INCREMENT_DATA (rhsFilled, rhsData);
    GET_AND_INCREMENT_DATA (rhsNumPoints, rhsData);

    XGScopedArray thisPointsBuffer, rhsPointsBuffer;
    ByteCP thisPoints = thisPointsBuffer.Acquire (  data, (thisNumPoints * sizeof (DPoint3d)));
    ByteCP rhsPoints  =  rhsPointsBuffer.Acquire (rhsData, (rhsNumPoints * sizeof (DPoint3d)));

    return thisFilled == rhsFilled && arePointsEqual ((DPoint3dCP) thisPoints, thisNumPoints, (DPoint3dCP) rhsPoints,  rhsNumPoints, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _AppendToGPA (byte const* pData, UInt32 dataSize, GPArrayR gpa) override
    {
    bool        filled;
    int         numPoints;

    if (0 != gpa.GetCount())
        gpa.MarkMajorBreak();

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (numPoints);
    gpa.Add ((DPoint3dCP) pData, numPoints);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     AddPolygon (PolyfaceCoordinateMap& facetCache, byte* pData, UInt32 dataSize)
    {
    bool        filled;
    int         numPoints;

    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (numPoints);

    DPoint3dCP      points = (DPoint3dCP) pData;
    facetCache.AddPolygon (numPoints, points);
    }

}; // XGraphicsDrawShape3d

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/08
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawTriStrip2d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("DrawTriStrip2d\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;
    double      depth;
    Int32       usageFlags;

    GET_AND_INCREMENT (usageFlags);
    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptBuffer;
    ByteCP pts = ptBuffer.Acquire (pData, numPoints * sizeof (DPoint2d));

    context.GetIDrawGeom().DrawTriStrip2d (numPoints, (DPoint2dCP) pts, usageFlags, depth, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    double      depth;
    byte*       pData = &data[0];

    pData += sizeof (Int32); // usageFlags
    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);
    //  NEEDS WORK unaligned output?
    transformDPoint2dArray ((DPoint2dP) pData, depth, numPoints, transform.GetTransform());

    return SUCCESS;
    }

}; // XGraphicsDrawTriStrip2d

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/08
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawTriStrip3d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("DrawTriStrip3d\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;
    Int32       usageFlags;

    GET_AND_INCREMENT (usageFlags);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptBuffer;
    ByteCP pts = ptBuffer.Acquire (pData, numPoints * sizeof (DPoint3d));

    context.GetIDrawGeom().DrawTriStrip3d (numPoints, (DPoint3dCP) pts, usageFlags, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    byte*       pData = &data[0];

    pData += sizeof (Int32); // usageFlags
    GET_AND_INCREMENT (numPoints);
    transform.GetTransform()->multiply ((DPoint3dP) pData, numPoints);

    return SUCCESS;
    }

}; // XGraphicsDrawTriStrip3d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawPointString2d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;
    double      depth;

    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    printf ("DrawLinePointString2D - Depth: %f, %d Points\n", depth, numPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;
    double      depth;

    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);

    XGScopedArray ptBuffer;
    ByteCP pts = ptBuffer.Acquire (pData, numPoints * sizeof (DPoint2d));

    context.GetIDrawGeom().DrawPointString2d (numPoints, (DPoint2dCP) pts, depth, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    double      depth;
    byte*       pData = &data[0];

    GET_AND_INCREMENT (depth);
    GET_AND_INCREMENT (numPoints);
    //  NEEDS WORK -- unaligned output
    transformDPoint2dArray ((DPoint2dP) pData, depth, numPoints, transform.GetTransform());

    return SUCCESS;
    }

}; // XGraphicsDrawPointString2d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawPointString3d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    int         numPoints;

    GET_AND_INCREMENT (numPoints);

    XGScopedArray   pointBuffer;
    DPoint3dCP      pointP      = (DPoint3dCP)pointBuffer.Acquire (pData, sizeof (DPoint3d) * numPoints);

    printf ("DrawPointString3D -  %d Points - (%g,%g,%g)...\n", numPoints, pointP->x, pointP->y, pointP->z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize , XGraphicsOperationContextR opContext) override
    {
    int         numPoints;

    GET_AND_INCREMENT (numPoints);

    XGScopedArray   pointBuffer;
    DPoint3dCP      pointP          = (DPoint3dCP)pointBuffer.Acquire (pData, sizeof (DPoint3d) * numPoints);

    context.GetIDrawGeom().DrawPointString3d (numPoints, pointP, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    int         numPoints;
    byte*       pData = &data[0];

    GET_AND_INCREMENT (numPoints);

    XGScopedArrayRW   pointBuffer;
    DPoint3dP         pointP          = (DPoint3dP)pointBuffer.AcquireRW (pData, sizeof (DPoint3d) * numPoints);

    transform.GetTransform()->multiply (pointP, numPoints);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    int         thisNumPoints, rhsNumPoints;

    GET_AND_INCREMENT_DATA (thisNumPoints, data);
    GET_AND_INCREMENT_DATA (rhsNumPoints, rhsData);

    XGScopedArray   pointBuffer;
    XGScopedArray   rhsPointBuffer;
    DPoint3dCP      pointP          = (DPoint3dCP)pointBuffer.Acquire (data, sizeof (DPoint3d) * thisNumPoints);
    DPoint3dCP      rhsPointP       = (DPoint3dCP)rhsPointBuffer.Acquire (rhsData, sizeof (DPoint3d) * rhsNumPoints);

    return arePointsEqual (pointP, thisNumPoints, rhsPointP,  rhsNumPoints, distanceTolerance);
    }

}; // XGraphicsDrawPointString3d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawArc2d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint2d    origin;
    double      r0, r1, start, sweep, orientation, depth;

    Get (origin, orientation, r0, r1, start, sweep, depth, pData);

    printf ("DrawArc2d: R0: %g R1: %g, start: %f, sweep: %f Center: (%g,%g)\n", r0, r1, start, sweep, origin.x, origin.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.UseUnCachedDraw (context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint2d    origin;
    double      r0, r1, start, sweep, orientation, depth;
    DEllipse3d  ellipse;

    Get (origin, orientation, r0, r1, start, sweep, depth, pData);
    ellipse.InitFromDGNFields2d (origin, orientation, r0, r1, start, sweep, 0.0);

    if (opContext.IsCurvePrimitiveRequired ())
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);

        opContext.DrawOrAddComplexComponent (primitive, context, false, false, false, depth);

        return SUCCESS;
        }

    if (opContext.AllowDrawStyled (context, false))
        context.DrawStyledArc2d (ellipse, false, depth, NULL);
    else
        context.GetIDrawGeom ().DrawArc2d (ellipse, false, false, depth, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DPoint2d    origin;
    double      r0, r1, start, sweep, orientation, depth;
    byte*       pData = &data[0];

    Get (origin, orientation, r0, r1, start, sweep, depth, pData);
    transform2dArc (origin, orientation, r0, r1, &start, &sweep, depth, transform.GetTransform());

    SET_AND_INCREMENT (origin);
    SET_AND_INCREMENT (orientation);
    SET_AND_INCREMENT (r0);
    SET_AND_INCREMENT (r1);
    SET_AND_INCREMENT (start);
    SET_AND_INCREMENT (sweep);
    SET_AND_INCREMENT (depth);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DPoint2d& origin, double& orientation, double& r0, double& r1, double& start, double& sweep, double& depth, byte const* pData)
    {
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (orientation);
    GET_AND_INCREMENT (r0);
    GET_AND_INCREMENT (r1);
    GET_AND_INCREMENT (start);
    GET_AND_INCREMENT (sweep);
    GET_AND_INCREMENT (depth);
    }
}; // XGraphicsDrawArc2d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawEllipse2d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint2d    origin;
    double      r0, r1,orientation, depth;
    bool        filled;

    Get (origin, orientation, r0, r1, filled, depth, pData);

    printf ("DrawEllipse2d: R0: %f, R1: %f, Depth: %f, filled: %d\n", r0, r1, depth, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint2d    origin;
    double      r0, r1, orientation, depth;
    bool        filled;

    Get (origin, orientation, r0, r1, filled, depth, pData);

    return opContext.UseUnCachedDraw (context, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint2d    origin;
    double      r0, r1, orientation, depth;
    bool        filled;
    DEllipse3d  ellipse;

    Get (origin, orientation, r0, r1, filled, depth, pData);
    ellipse.InitFromDGNFields2d (origin, orientation, r0, r1, 0.0, msGeomConst_2pi, 0.0);

    if (opContext.IsCurvePrimitiveRequired ())
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse); 
        opContext.DrawOrAddComplexComponent (primitive, context, true, filled, false, depth);
        return SUCCESS;
        }

    bool    displayFilled = opContext.AllowDrawFilled (context, filled);

    if (!displayFilled && opContext.AllowDrawStyled (context, true))
        context.DrawStyledArc2d (ellipse, true, depth, NULL);
    else if (!filled && context.GetIViewDraw ().IsOutputQuickVision ()) // Never filled, output open geometry that won't render as a surface when added to QvElem...
        context.GetIDrawGeom ().DrawArc2d (ellipse, false, false, depth, NULL);
    else
        context.GetIDrawGeom ().DrawArc2d (ellipse, true, displayFilled, depth, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DPoint2d    origin;
    double      r0, r1,orientation, depth;
    bool        filled;
    byte*       pData = &data[0];

    Get (origin, orientation, r0, r1, filled, depth, pData);
    transform2dArc (origin, orientation, r0, r1,  NULL, NULL, depth, transform.GetTransform());

    SET_AND_INCREMENT (origin);
    SET_AND_INCREMENT (orientation);
    SET_AND_INCREMENT (r0);
    SET_AND_INCREMENT (r1);
    SET_AND_INCREMENT (filled);
    SET_AND_INCREMENT (depth);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DPoint2d& origin, double& orientation, double& r0, double& r1, bool& filled, double& depth, byte const* pData)
    {
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (orientation);
    GET_AND_INCREMENT (r0);
    GET_AND_INCREMENT (r1);
    GET_AND_INCREMENT (filled);
    GET_AND_INCREMENT (depth);
    }
}; // XGraphicsDrawEllipse2d


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawArc3d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d    origin;
    DPoint4d    quat;
    double      r0, r1, start, sweep;

    Get (origin, quat, r0, r1, start, sweep, pData);

    printf ("Arc3D - Origin: (%g,%g,%g), quat: (%f,%f,%f,%f), r0: %g, r1: %g, start: %f, sweep: %f\n",
            origin.x, origin.y, origin.z, quat.x, quat.y, quat.z, quat.w, r0, r1, start, sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.UseUnCachedDraw (context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1, start, sweep;
    DEllipse3d  ellipse;

    Get (origin, quaternion, r0, r1, start, sweep, pData);
    ellipse.InitFromDGNFields3d (origin, &quaternion.x, r0, r1, start, sweep);

    if (opContext.IsCurvePrimitiveRequired ())
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);

        opContext.DrawOrAddComplexComponent (primitive, context, false, false, true, 0.0);

        return SUCCESS;
        }

    if (opContext.AllowDrawStyled (context, false))
        context.DrawStyledArc3d (ellipse, false, NULL);
    else
        context.GetIDrawGeom ().DrawArc3d (ellipse, false, false, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1, start, sweep;
    byte*       pData = &data[0];

    Get (origin, quaternion, r0, r1, start, sweep, pData);

    transform3dArc (origin, quaternion, r0, r1, &start, &sweep, transform.GetTransform());

    SET_AND_INCREMENT (origin);
    SET_AND_INCREMENT (quaternion);
    SET_AND_INCREMENT (r0);
    SET_AND_INCREMENT (r1);
    SET_AND_INCREMENT (start);
    SET_AND_INCREMENT (sweep);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;

    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (quaternion);

    RotMatrix   rMatrix;

    rMatrix.initFromQuaternion (&quaternion.x);
    transform.initFrom (&rMatrix, &origin);

    return SUCCESS;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _TestTransform (TransformCR transform, byte* pData, UInt32 dataSize) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1, start, sweep;

    Get (origin, quaternion, r0, r1, start, sweep, pData);

    RotMatrix       rMatrix;
    DVec3d          xVec, yVec;

    rMatrix.initFromQuaternion (&quaternion.x);

    rMatrix.getColumn (&xVec, 0);
    rMatrix.getColumn (&yVec, 1);

    xVec.scale (r0);
    yVec.scale (r1);

    transform.multiplyMatrixOnly (&xVec);
    transform.multiplyMatrixOnly (&yVec);

    double  xMagnitude = xVec.magnitude(), yMagnitude = yVec.magnitude();
    return (isEqual (r0/r1, xMagnitude/yMagnitude, 1.0E-8)  && xVec.isPerpendicularTo (&yVec)) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DPoint3d    thisOrigin, rhsOrigin;
    DPoint4d    thisQuaternion, rhsQuaternion;
    double      thisR0, thisR1, thisStart, thisSweep;
    double      rhsR0, rhsR1, rhsStart, rhsSweep;

    Get (thisOrigin, thisQuaternion, thisR0, thisR1, thisStart, thisSweep, data);
    Get (rhsOrigin, rhsQuaternion, rhsR0, rhsR1, rhsStart, rhsSweep, rhsData);

    return thisOrigin.isEqual (&rhsOrigin, distanceTolerance) &&
           areQuaternionsEqual (thisQuaternion, rhsQuaternion, s_directionTolerance) &&
           isEqual (thisR0, rhsR0, s_distanceTolerance) &&
           isEqual (thisR1, rhsR1, s_distanceTolerance) &&
           isEqual (thisStart, rhsStart, s_directionTolerance) &&
           isEqual (thisSweep, rhsSweep, s_directionTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _AppendToGPA (byte const* pData, UInt32 dataSize, GPArrayR gpa) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1, start, sweep;

    Get (origin, quaternion, r0, r1, start, sweep, pData);

    DEllipse3d  dEllipse;

    dEllipse.center = origin;
    dEllipse.start = start;
    dEllipse.sweep = sweep;

    RotMatrix   rMatrix;

    rMatrix.initFromQuaternion (&quaternion.x);
    rMatrix.getRows (&dEllipse.vector0, &dEllipse.vector90, NULL);

    dEllipse.vector0.scale (r0);
    dEllipse.vector90.scale (r1);

    gpa.Add (dEllipse);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DPoint3d& origin, DPoint4d& quaternion, double& r0, double& r1, double& start, double& sweep, byte const* pData)
    {
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (quaternion);
    GET_AND_INCREMENT (r0);
    GET_AND_INCREMENT (r1);
    GET_AND_INCREMENT (start);
    GET_AND_INCREMENT (sweep);
    }
}; // XGraphicsDrawArc3d


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsDrawEllipse3d : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d    origin;
    DPoint4d    quat;
    double      r0, r1;
    bool        filled;

    Get (origin, quat, r0, r1, filled, pData);

    printf ("Ellipse3d Filled: %d - Origin: (%g,%g,%g), quat: (%f,%f,%f,%f), r0: %g, r1: %g\n",  filled, origin.x, origin.y, origin.z, quat.x, quat.y, quat.z, quat.w, r0, r1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1;
    bool        filled;

    Get (origin, quaternion, r0, r1, filled, pData);

    return opContext.UseUnCachedDraw (context, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1;
    bool        filled;
    DEllipse3d  ellipse;

    Get (origin, quaternion, r0, r1, filled, pData);
    ellipse.InitFromDGNFields3d (origin, &quaternion.x, r0, r1, 0.0, msGeomConst_2pi);

    if (opContext.IsCurvePrimitiveRequired ())
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);

        opContext.DrawOrAddComplexComponent (primitive, context, true, filled, true, 0.0);

        return SUCCESS;
        }

    bool    displayFilled = opContext.AllowDrawFilled (context, filled);

    if (!displayFilled && opContext.AllowDrawStyled (context, true))
        context.DrawStyledArc3d (ellipse, true, NULL);
    else if (!filled && context.GetIViewDraw ().IsOutputQuickVision ()) // Never filled, output open geometry that won't render as a surface when added to QvElem...
        context.GetIDrawGeom ().DrawArc3d (ellipse, false, false, NULL);
    else
        context.GetIDrawGeom ().DrawArc3d (ellipse, true, displayFilled, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1;
    bool        filled;
    byte*       pData = &data[0];

    Get (origin, quaternion, r0, r1, filled, pData);
    transform3dArc (origin, quaternion, r0, r1, NULL, NULL, transform.GetTransform());

    SET_AND_INCREMENT (origin);
    SET_AND_INCREMENT (quaternion);
    SET_AND_INCREMENT (r0);
    SET_AND_INCREMENT (r1);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;

    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (quaternion);

    RotMatrix   rMatrix;

    rMatrix.initFromQuaternion (&quaternion.x);
    transform.initFrom (&rMatrix, &origin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DPoint3d    thisOrigin, rhsOrigin;
    DPoint4d    thisQuaternion, rhsQuaternion;
    double      thisR0, thisR1, rhsR0, rhsR1;
    bool        thisFilled, rhsFilled;

    Get (thisOrigin, thisQuaternion, thisR0, thisR1, thisFilled, data);
    Get (rhsOrigin, rhsQuaternion, rhsR0, rhsR1, rhsFilled, rhsData);

    return thisOrigin.isEqual (&rhsOrigin, distanceTolerance) &&
           areQuaternionsEqual (thisQuaternion, rhsQuaternion, s_directionTolerance) &&
           isEqual (thisR0, rhsR0, s_distanceTolerance) &&
           isEqual (thisR1, rhsR1, s_distanceTolerance) &&
           thisFilled == rhsFilled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _AppendToGPA (byte const* pData, UInt32 dataSize, GPArrayR gpa) override
    {
    DPoint3d    origin;
    DPoint4d    quaternion;
    double      r0, r1;
    bool        filled;

    if (0 != gpa.GetCount())
        gpa.MarkMajorBreak();

    Get (origin, quaternion, r0, r1, filled, pData);

    DEllipse3d  dEllipse;

    dEllipse.center = origin;
    dEllipse.start = 0.0;
    dEllipse.makeFullSweep();

    RotMatrix   rMatrix;

    rMatrix.initFromQuaternion (&quaternion.x);
    rMatrix.getRows (&dEllipse.vector0, &dEllipse.vector90, NULL);

    dEllipse.vector0.scale (r0);
    dEllipse.vector90.scale (r1);

    gpa.Add (dEllipse);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DPoint3d& origin, DPoint4d& quaternion, double& r0, double& r1, bool& filled, byte const* pData)
    {
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (quaternion);
    GET_AND_INCREMENT (r0);
    GET_AND_INCREMENT (r1);
    GET_AND_INCREMENT (filled);
    }
}; // XGraphicsDrawEllipse3d

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawSphere : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DVec3d      primary, secondary;
    DPoint3d    center;
    double      primaryRadius, secondaryRadius, start, sweep;

    Get (primary, secondary, center, primaryRadius, secondaryRadius, start, sweep, pData);

    printf ("DrawSphere - Primary Radius: %g, Secondary Radius: %g, Start: %f, Sweep; %f\n", primaryRadius, secondaryRadius, start, sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d center;
    DVec3d vectorX;
    DVec3d vectorY;
    DVec3d vectorZ;
    double radiusXY;
    double radiusZ;
    double startLatitude;
    double latitudeSweep; // NOTE: SS3 checked for Angle::Pi to denote full sphere...
    bool capped = true; // ?????
    Get (vectorX, vectorY, center, radiusXY, radiusZ, startLatitude, latitudeSweep, pData);
    vectorZ.CrossProduct (vectorX, vectorY); // NOTE: Original IViewDraw::DrawSphere input was X vec, Y vec, center, XY radius, Z radius, start, sweep...
    DgnSphereDetail detail (center, vectorX, vectorZ, radiusXY, radiusZ, Angle::Pi () == latitudeSweep ? -Angle::PiOver2 () : startLatitude, latitudeSweep, capped);
    ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnSphere (detail);

    context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DVec3d      primary, secondary;
    DPoint3d    center;
    double      primaryRadius, secondaryRadius, start, sweep;
    byte*       pData = &data[0];

    Get (primary, secondary, center, primaryRadius, secondaryRadius, start, sweep, pData);

    primary.scale (primaryRadius);
    secondary.scale (secondaryRadius);

    transform.GetTransform()->multiply (&center);
    transform.GetTransform()->multiplyMatrixOnly (&primary);
    transform.GetTransform()->multiplyMatrixOnly (&secondary);
    primaryRadius = primary.normalize();
    secondaryRadius = secondary.normalize();

    SET_AND_INCREMENT (primary);
    SET_AND_INCREMENT (secondary);
    SET_AND_INCREMENT (center);
    SET_AND_INCREMENT (primaryRadius);
    SET_AND_INCREMENT (secondaryRadius);
    SET_AND_INCREMENT (start);
    SET_AND_INCREMENT (sweep);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DVec3d      primary, secondary;
    DPoint3d    center;
    double      primaryRadius, secondaryRadius, start, sweep;

    Get (primary, secondary, center, primaryRadius, secondaryRadius, start, sweep, pData);

    RotMatrix   rMatrix;
    rMatrix.initFrom2Vectors (&primary, &secondary);
    transform.initFrom (&rMatrix, &center);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DVec3d      primary, secondary;
    DPoint3d    center;
    double      primaryRadius, secondaryRadius, start, sweep;

    Get (primary, secondary, center, primaryRadius, secondaryRadius, start, sweep, data);

    DVec3d      primaryRHS, secondaryRHS;
    DPoint3d    centerRHS;
    double      primaryRadiusRHS, secondaryRadiusRHS, startRHS, sweepRHS;

    Get (primaryRHS, secondaryRHS, centerRHS, primaryRadiusRHS, secondaryRadiusRHS, startRHS, sweepRHS, rhsData);

    return primary.isEqual (&primaryRHS, s_directionTolerance) &&
           secondary.isEqual (&secondaryRHS, s_directionTolerance) &&
           center.isEqual (&centerRHS, distanceTolerance) &&
           isEqual (primaryRadius, primaryRadiusRHS, distanceTolerance) &&
           isEqual (secondaryRadius, secondaryRadiusRHS, distanceTolerance) &&
           isEqual (start, startRHS, s_directionTolerance) &&
           isEqual (sweep, sweepRHS, s_directionTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DVec3dR primary, DVec3dR secondary, DPoint3dR center, double& primaryRadius, double& secondaryRadius, double& start, double& sweep, byte const* pData)
    {
    GET_AND_INCREMENT (primary);
    GET_AND_INCREMENT (secondary);
    GET_AND_INCREMENT (center);
    GET_AND_INCREMENT (primaryRadius);
    GET_AND_INCREMENT (secondaryRadius);
    GET_AND_INCREMENT (start);
    GET_AND_INCREMENT (sweep);
    }
}; // XGraphicsDrawSphere

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawBox : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DVec3d      primary, secondary;
    DPoint3d    basePoint, topPoint;
    double      baseWidth, baseLength, topWidth, topLength;
    bool        cap;

    Get (primary, secondary, basePoint, topPoint, baseWidth, baseLength, topWidth, topLength, cap, pData);
    printf ("DrawBox - (%g, %g) X (%g, %g) \n", baseWidth, baseLength, topWidth, topLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        cap;
    double      baseX, baseY, topX, topY;
    DVec3d      vectorX, vectorY;
    DPoint3d    baseCenter, topCenter;

    // NOTE: Stored centers not origins...
    Get (vectorX, vectorY, baseCenter, topCenter, baseX, baseY, topX, topY, cap, pData);

    DgnBoxDetail        detail = DgnBoxDetail::InitFromCenters (baseCenter, topCenter, vectorX, vectorY, baseX, baseY, topX, topY, cap);
    ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnBox (detail);

    context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DVec3d      primary, secondary;
    DPoint3d    basePoint, topPoint;
    double      baseWidth, baseLength, topWidth, topLength;
    bool        cap;
    byte*       pData = &data[0];

    Get (primary, secondary, basePoint, topPoint, baseWidth, baseLength, topWidth, topLength, cap, pData);

    transform.GetTransform()->multiply (&basePoint);
    transform.GetTransform()->multiply (&topPoint);
    transform.GetTransform()->multiplyMatrixOnly (&primary);
    transform.GetTransform()->multiplyMatrixOnly (&secondary);
    double  xScale = primary.normalize();
    double  yScale = secondary.normalize();

    baseWidth  *= xScale;
    topWidth   *= xScale;
    baseLength  *= yScale;
    topLength  *= yScale;

    SET_AND_INCREMENT (primary);
    SET_AND_INCREMENT (secondary);
    SET_AND_INCREMENT (basePoint);
    SET_AND_INCREMENT (topPoint);
    SET_AND_INCREMENT (baseWidth);
    SET_AND_INCREMENT (baseLength);
    SET_AND_INCREMENT (topWidth);
    SET_AND_INCREMENT (topLength);
    SET_AND_INCREMENT (cap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DVec3d      primary, secondary;
    DPoint3d    basePoint, topPoint;
    double      baseWidth, baseLength, topWidth, topLength;
    bool        cap;

    Get (primary, secondary, basePoint, topPoint, baseWidth, baseLength, topWidth, topLength, cap, pData);

    RotMatrix   rMatrix;
    rMatrix.initFrom2Vectors (&primary, &secondary);
    transform.initFrom (&rMatrix, &basePoint);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DVec3d      primary, secondary;
    DPoint3d    basePoint, topPoint;
    double      baseWidth, baseLength, topWidth, topLength;
    bool        cap;

    Get (primary, secondary, basePoint, topPoint, baseWidth, baseLength, topWidth, topLength, cap, data);

    DVec3d      rhsPrimary, rhsSecondary;
    DPoint3d    rhsBasePoint, rhsTopPoint;
    double      rhsBaseWidth, rhsBaseLength, rhsTopWidth, rhsTopLength;
    bool        rhsCap;

    Get (rhsPrimary, rhsSecondary, rhsBasePoint, rhsTopPoint, rhsBaseWidth, rhsBaseLength, rhsTopWidth, rhsTopLength, rhsCap, rhsData);

    return primary.isEqual (&rhsPrimary, s_directionTolerance) &&
           secondary.isEqual (&rhsSecondary, s_directionTolerance) &&
           basePoint.isEqual (&rhsBasePoint, distanceTolerance) &&
           topPoint.isEqual (&rhsTopPoint, distanceTolerance) &&
           isEqual (baseWidth,  rhsBaseWidth,  distanceTolerance) &&
           isEqual (baseLength, rhsBaseLength, distanceTolerance) &&
           isEqual (topWidth,   rhsTopWidth,   distanceTolerance) &&
           isEqual (topLength , rhsTopLength,  distanceTolerance) &&
           cap == rhsCap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DVec3dR primary, DVec3dR secondary, DPoint3dR basePoint, DPoint3dR topPoint, double& baseWidth, double& baseLength, double& topWidth, double& topLength, bool& cap, byte const* pData)
    {
    GET_AND_INCREMENT (primary);
    GET_AND_INCREMENT (secondary);
    GET_AND_INCREMENT (basePoint);
    GET_AND_INCREMENT (topPoint);
    GET_AND_INCREMENT (baseWidth);
    GET_AND_INCREMENT (baseLength);
    GET_AND_INCREMENT (topWidth);
    GET_AND_INCREMENT (topLength);
    GET_AND_INCREMENT (cap);
    }
}; // XGraphicsDrawBox

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawCone : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DVec3d      xVec, yVec;
    DPoint3d    center0, center1;
    double      r0, r1;
    bool        cap;

    Get (xVec, yVec, center0, center1, r0, r1, cap, pData);
    printf ("DrawCone: C0: (%g,%g,%g) R0: %f, C1: (%g,%g,%g) R1: %f, cap: %d\n", center0.x, center0.y, center0.z, r0, center1.x, center1.y, center1.z, r1, cap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DgnConeDetail  detail;

    Get (detail.m_vector0, detail.m_vector90, detail.m_centerA, detail.m_centerB, detail.m_radiusA, detail.m_radiusB, detail.m_capped, pData);
    ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnCone (detail);
    context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DVec3d      xVec, yVec, tmpVec;
    DPoint3d    center0, center1;
    double      r0, r1;
    bool        cap;
    byte*       pData = &data[0];

    Get (xVec, yVec, center0, center1, r0, r1, cap, pData);

    if (0.0 != r0)
        {
        tmpVec.scale (&xVec, r0);
        transform.GetTransform()->multiplyMatrixOnly (&tmpVec);
        r0 = tmpVec.magnitude();
        }

    if (0.0 != r1)
        {
        tmpVec.scale (&xVec, r1);
        transform.GetTransform()->multiplyMatrixOnly (&tmpVec);
        r1 = tmpVec.magnitude();
        }

    transform.GetTransform()->multiply (&center0);
    transform.GetTransform()->multiply (&center1);
    transform.GetTransform()->multiplyMatrixOnly (&xVec);
    transform.GetTransform()->multiplyMatrixOnly (&yVec);
    xVec.normalize();
    yVec.normalize();

    SET_AND_INCREMENT (xVec);
    SET_AND_INCREMENT (yVec);
    SET_AND_INCREMENT (center0);
    SET_AND_INCREMENT (center1);
    SET_AND_INCREMENT (r0);
    SET_AND_INCREMENT (r1);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _TestTransform (TransformCR transform, byte* pData, UInt32 dataSize) override
    {
    DVec3d      xVec, yVec;
    DPoint3d    center0, center1;
    double      r0, r1;
    bool        cap;

    Get (xVec, yVec, center0, center1, r0, r1, cap, pData);

    transform.multiplyMatrixOnly (&xVec);
    transform.multiplyMatrixOnly (&yVec);

    double      xMagnitude = xVec.magnitude(), yMagnitude = yVec.magnitude();

    return (isEqual (xMagnitude, yMagnitude, 1.0E-8)  && xVec.isPerpendicularTo (&yVec)) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DVec3d      xVec, yVec;
    DPoint3d    center0, center1;
    double      r0, r1;
    bool        cap;

    Get (xVec, yVec, center0, center1, r0, r1, cap, pData);

    RotMatrix   rMatrix;
    rMatrix.initFrom2Vectors (&xVec, &yVec);
    transform.initFrom (&rMatrix, &center0);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DVec3d      xVec, yVec;
    DPoint3d    center0, center1;
    double      r0, r1;
    bool        cap;

    Get (xVec, yVec, center0, center1, r0, r1, cap, data);

    DVec3d      rhsXVec, rhsYVec;
    DPoint3d    rhsCenter0, rhsCenter1;
    double      rhsR0, rhsR1;
    bool        rhsCap;

    Get (rhsXVec, rhsYVec, rhsCenter0, rhsCenter1, rhsR0, rhsR1, rhsCap, rhsData);

    return xVec.isEqual (&rhsXVec, s_directionTolerance) &&
           yVec.isEqual (&rhsYVec, s_directionTolerance) &&
           center0.isEqual (&rhsCenter0, distanceTolerance) &&
           center1.isEqual (&rhsCenter1, distanceTolerance) &&
           isEqual (r0, rhsR0, distanceTolerance) &&
           isEqual (r1, rhsR1, distanceTolerance) &&
           cap == rhsCap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DVec3d& xVec, DVec3d& yVec, DPoint3d& center0, DPoint3d& center1, double& r0, double& r1, bool& cap, byte const* pData)
    {
    GET_AND_INCREMENT (xVec);
    GET_AND_INCREMENT (yVec);
    GET_AND_INCREMENT (center0);
    GET_AND_INCREMENT (center1);
    GET_AND_INCREMENT (r0);
    GET_AND_INCREMENT (r1);
    GET_AND_INCREMENT (cap);
    }
}; // XGraphicsDrawCone

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawTorus : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DPoint3d    origin;
    DVec3d      primary, secondary;
    double      radius, tubeRadius, sweep;
    bool        cap;

    Get (primary, secondary, origin, radius, tubeRadius, sweep, cap, pData);
    printf ("DrawTorus, Radius: %g, Tube Radius: %g, Sweep: %f\n", radius, tubeRadius, sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DgnTorusPipeDetail  detail;

    Get (detail.m_vectorX, detail.m_vectorY, detail.m_center, detail.m_majorRadius, detail.m_minorRadius, detail.m_sweepAngle, detail.m_capped, pData);
    ISolidPrimitivePtr  primitive = ISolidPrimitive::CreateDgnTorusPipe (detail);
    context.GetIDrawGeom ().DrawSolidPrimitive (*primitive);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DPoint3d    origin;
    DVec3d      primary, secondary, tubePrimary;
    double      radius, tubeRadius, sweep;
    bool        cap;
    byte*       pData = &data[0];

    Get (primary, secondary, origin, radius, tubeRadius, sweep, cap, pData);

    tubePrimary.scale (&primary, tubeRadius);
    primary.scale (radius);

    TransformCP pTransform = transform.GetTransform();

    pTransform->multiply (&origin);
    pTransform->multiplyMatrixOnly (&primary);
    pTransform->multiplyMatrixOnly (&secondary);
    pTransform->multiplyMatrixOnly (&tubePrimary);

    radius = primary.normalize ();
    secondary.normalize ();
    tubeRadius = tubePrimary.magnitude();

    SET_AND_INCREMENT (primary);
    SET_AND_INCREMENT (secondary);
    SET_AND_INCREMENT (origin);
    SET_AND_INCREMENT (radius);
    SET_AND_INCREMENT (tubeRadius);

    SET_AND_INCREMENT (sweep);
    SET_AND_INCREMENT (cap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _TestTransform (TransformCR transform, byte* pData, UInt32 dataSize) override
    {
    return isTransformOrthogonalAndUniformScale (transform) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DPoint3d    origin;
    DVec3d      primary, secondary;
    double      radius, tubeRadius, sweep;
    bool        cap;

    Get (primary, secondary, origin, radius, tubeRadius, sweep, cap, pData);

    RotMatrix   rMatrix;
    rMatrix.initFrom2Vectors (&primary, &secondary);
    transform.initFrom (&rMatrix, &origin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DPoint3d    thisOrigin, rhsOrigin;
    DVec3d      thisPrimary, thisSecondary, rhsPrimary, rhsSecondary;
    double      thisRadius, thisTubeRadius, thisSweep, rhsRadius, rhsTubeRadius, rhsSweep;
    bool        thisCap, rhsCap;

    Get (thisPrimary, thisSecondary, thisOrigin, thisRadius, thisTubeRadius, thisSweep, thisCap, data);
    Get (rhsPrimary, rhsSecondary, rhsOrigin, rhsRadius, rhsTubeRadius, rhsSweep, rhsCap, rhsData);

    return thisPrimary.isEqual (&rhsPrimary, s_directionTolerance) &&
           thisSecondary.isEqual (&rhsSecondary, s_directionTolerance) &&
           thisOrigin.isEqual (&rhsOrigin, distanceTolerance) &&
           isEqual (thisRadius, rhsRadius, distanceTolerance) &&
           isEqual (thisTubeRadius, rhsTubeRadius, distanceTolerance) &&
           isEqual (thisSweep,  rhsSweep, s_directionTolerance) &&
           thisCap == rhsCap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DVec3d& primary, DVec3d& secondary, DPoint3d& origin, double& radius, double& tubeRadius, double& sweep, bool &cap, byte const* pData)
    {
    GET_AND_INCREMENT (primary);
    GET_AND_INCREMENT (secondary);
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (radius);
    GET_AND_INCREMENT (tubeRadius);
    GET_AND_INCREMENT (sweep);
    GET_AND_INCREMENT (cap);
    }
}; // XGraphicsDrawTorus

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsBeginSweepProject : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("BeginSweepProject\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        enableCap;

    GET_AND_INCREMENT (enableCap);
    opContext.BeginSweepProject (enableCap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    byte *pSweep0, *pEnd0, *pSweep1, *pEnd1;

    pData += dataSize;

    if (SUCCESS == XGraphicsOperations::FindBoundaries (&pSweep0, &pEnd0, &pSweep1, &pEnd1, pData, pEnd))
        {
        Transform   transform0, transform1;

        if (SUCCESS == XGraphicsOperations::GetBasisTransform (transform0, pSweep0, pEnd0) &&
            SUCCESS == XGraphicsOperations::GetBasisTransform (transform1, pSweep1, pEnd1))
            {
            DPoint3d    origin0, origin1;
            DVec3d      xColumn, yColumn, zColumn;

            transform0.getTranslation (&origin0);
            transform1.getTranslation (&origin1);

            zColumn.differenceOf (&origin1, &origin0);

            if (zColumn.magnitude() < 1.0E-6)
                {
                transform = transform0;

                return SUCCESS;
                }

            transform0.getMatrixColumn (&xColumn, 0);
            yColumn.crossProduct (&zColumn, &xColumn);
            xColumn.crossProduct (&yColumn, &zColumn);
            xColumn.normalize();
            yColumn.normalize();
            zColumn.scale (1.0 / s_basisLength);

            transform.initFromOriginAndVectors (&origin0, &xColumn, &yColumn, &zColumn);

            return SUCCESS;
            }
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    UInt16      opCode;
    UInt32      opSize;

    for (; pData < pEnd; pData += opSize - sizeof (opSize))
        {
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        XGraphicsOperations::Draw (opCode, &context, pData, opSize - sizeof (opSize), opContext);
        size += opSize + sizeof (opCode);

        if (XGRAPHIC_OpCode_EndSweep == opCode)
            return SUCCESS;
        }

    BeAssert (false); // No EndSweep found
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pOptimizeData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override
    {
    bool    meshFromPlanarSweep     = 0 != (context.m_options & (XGRAPHIC_OptimizeOptions_MeshFromPlanarSweep)),
            conesFromProjections    = 0 != (context.m_options & (XGRAPHIC_OptimizeOptions_ConesFromProjections));

    if (!meshFromPlanarSweep && !conesFromProjections)
        return ERROR;

    byte    *pSweep0, *pEnd0, *pSweep1, *pEnd1;
    byte    *pData = pOptimizeData;
    bool    enableCap;

    GET_AND_INCREMENT (enableCap);

    if (SUCCESS != XGraphicsOperations::FindBoundaries (&pSweep0, &pEnd0, &pSweep1, &pEnd1, pData, pEnd))
        return ERROR;

    UInt16  opCode;

    pData = pEnd1;

    if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd) || opCode != XGRAPHIC_OpCode_EndSweep)
        return ERROR;

    GPArraySmartP   gpa[2];

    AppendComplexComponentsToGpaOperator appendToGpa0 (*gpa[0]);
    XGraphicsOperations::Traverse (pSweep0, pEnd0, appendToGpa0);
    AppendComplexComponentsToGpaOperator appendToGpa1 (*gpa[1]);
    XGraphicsOperations::Traverse (pSweep1, pEnd1, appendToGpa1);

    if (0 == gpa[0]->GetCount() || 0 == gpa[1]->GetCount())
        return ERROR;

    if (!gpa[0]->ContainsCurves() && !gpa[1]->ContainsCurves () && gpa[0]->GetCount () == gpa[1]->GetCount ())
        {
        if (!meshFromPlanarSweep)
            return ERROR;

        bvector<GraphicsPointArrayP> contours;
        contours.push_back (gpa[0]);
        contours.push_back (gpa[1]);

        IFacetOptionsPtr options = IFacetOptions::New ();
        SetXGraphicsFacetOptions (*options);
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New (*options);
        builder->AddRuledBetweenCorespondingCurves (contours, enableCap);

        optimizedData.BeginDraw ();
        XGraphicsWriter (&optimizedData).WritePolyface (builder->GetClientMeshR (), true);
        pOptimizeData = nextOp (pData, opSize);

        return SUCCESS;
        }

    if (!conesFromProjections)
        return ERROR;

    int         index0 = 0, index1 = 0;
    DEllipse3d  ellipse0, ellipse1;

    if (SUCCESS == gpa[0]->GetEllipse (&index0, &ellipse0) && index0 == gpa[0]->GetCount() &&
        SUCCESS == gpa[1]->GetEllipse (&index1, &ellipse1) && index1 == gpa[1]->GetCount() &&
        ellipse0.isFullEllipse () && ellipse1.isFullEllipse () &&
        ellipse0.isCircular() && ellipse1.isCircular())
        {
        double          radius0, radius1;
        DPoint3d        center0, center1;
        RotMatrix       matrix0, matrix1;

        ellipse0.getScaledRotMatrix(&center0, &matrix0, &radius0, NULL, NULL, NULL);
        ellipse1.getScaledRotMatrix (&center1, &matrix1, &radius1, NULL, NULL, NULL);
        DVec3d xVec0, yVec0, zVec0, xVec1, yVec1, zVec1;
        matrix0.getColumns (&xVec0, &yVec0, &zVec0);
        matrix1.getColumns (&xVec1, &yVec1, &zVec1);

        if (zVec0.isParallelTo (&zVec1))
            {
            optimizedData.BeginDraw ();
            XGraphicsWriter (&optimizedData).WriteCone (&xVec0, &yVec1, &center0, &center1, radius0, radius1, enableCap);
            pOptimizeData = nextOp (pData, opSize);

            return SUCCESS;
            }
        }

    return ERROR;
    }
}; // XGraphicsBeginSweepProject

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsBeginSweepExtrude : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        enableCap;
    DVec3d      extrusion;

    Get (enableCap, extrusion, pData);

    printf ("BeginSweepExtrude: Cap: %d, Extrude: (%f, %f, %f)\n", enableCap, extrusion.x, extrusion.y, extrusion.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        enableCap;
    DVec3d      extrusion;

    Get (enableCap, extrusion, pData);
    opContext.BeginSweepExtrude (enableCap, extrusion);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    bool        enableCap;
    DVec3d      extrusion;
    byte*       pData = &data[0];

    Get (enableCap, extrusion, pData);

    transform.GetTransform()->multiplyMatrixOnly (&extrusion);

    SET_AND_INCREMENT (extrusion);
    SET_AND_INCREMENT (enableCap);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    byte*       pNext = pData + dataSize, *pBoundary, *pEndBoundary;
    bool        cap;
    DVec3d      extrusion;

    GET_AND_INCREMENT (extrusion);
    GET_AND_INCREMENT (cap);

    if (SUCCESS == XGraphicsOperations::FindComplex (&pBoundary, &pEndBoundary, &pNext, pEnd, XGRAPHIC_OpCode_BeginComplexString) ||
        SUCCESS == XGraphicsOperations::FindComplex (&pBoundary, &pEndBoundary, &pNext, pEnd, XGRAPHIC_OpCode_BeginComplexShape))
        {
        if (SUCCESS == XGraphicsOperations::GetBasisTransform (transform, pBoundary, pEndBoundary))
            {
            transform.scaleMatrixColumns (&transform, 1.0, 1.0, extrusion.magnitude() / s_basisLength);

            return SUCCESS;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    bool        cap, rhsCap;
    DVec3d      extrusion, rhsExtrusion;

    Get (cap, extrusion, data);
    Get (rhsCap, rhsExtrusion, rhsData);

    return  cap == rhsCap && extrusion.isEqual (&rhsExtrusion, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (bool& enableCap, DVec3d& extrusion, byte const* pData)
    {
    GET_AND_INCREMENT (extrusion);
    GET_AND_INCREMENT (enableCap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    UInt16      opCode;
    UInt32      opSize;

    for (; pData < pEnd; pData += opSize - sizeof (opSize))
        {
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        XGraphicsOperations::Draw (opCode, &context, pData, opSize - sizeof (opSize), opContext);
        size += opSize + sizeof (opCode);

        if (XGRAPHIC_OpCode_EndSweep == opCode)
            return SUCCESS;
        }

    BeAssert (false); // No EndSweep found
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pOptimizeData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override
    {
    bool    doPlanarSweeps  = 0 != (XGRAPHIC_OptimizeOptions_MeshFromPlanarSweep & context.m_options);
    bool    doCones         = 0 != (XGRAPHIC_OptimizeOptions_ConesFromProjections & context.m_options);

    if (!doPlanarSweeps && !doCones)
        return ERROR;

    byte        *pData = pOptimizeData;
    byte        *pAfterBeginSweep = nextOp (pData, opSize), *pAfterEndSweep = pAfterBeginSweep, *pBoundary, *pBoundaryEnd, *pSweepEnd;
    bool        enableCap;
    DVec3d      extrusion;
    StatusInt   status;

    GET_AND_INCREMENT (extrusion);
    GET_AND_INCREMENT (enableCap);

    if (SUCCESS != (status = XGraphicsOperations::FindOperation (&pSweepEnd, NULL, &pAfterEndSweep, pEnd, XGRAPHIC_OpCode_EndSweep)) ||
        SUCCESS != (status = XGraphicsOperations::FindBoundary (&pBoundary, &pBoundaryEnd, pAfterBeginSweep, pSweepEnd)))
        return status;

    GPArraySmartP   gpa;

    AppendComplexComponentsToGpaOperator appendToGpa (gpa);
    XGraphicsOperations::Traverse (pBoundary, pBoundaryEnd, appendToGpa);

    if (0 == gpa->GetCount())
        return ERROR;

    if (enableCap)
        gpa->MarkMajorBreak();

    if (doCones)
        {
        int                 index0 = 0;
        DEllipse3d          ellipse0;

        if (SUCCESS == gpa->GetEllipse (&index0, &ellipse0) && index0 == gpa->GetCount() &&
            ellipse0.isFullEllipse () &&
            ellipse0.isCircular())
            {
            double          radius0;
            DPoint3d        center0, center1;
            DMatrix3d       matrix0;


            ellipse0.getScaledDMatrix3d (&center0, &matrix0, &radius0, NULL, NULL, NULL);
            center1.SumOf (center0, extrusion);

            optimizedData.BeginDraw ();
            XGraphicsWriter (&optimizedData).WriteCone (&matrix0.column[0], &matrix0.column[1], &center0, &center1, radius0, radius0, enableCap);
            pOptimizeData = pAfterEndSweep;

            return SUCCESS;
            }
        }

    if (doPlanarSweeps && !gpa->ContainsCurves())
        {
        bvector<GraphicsPointArrayP>    contours;
        GPArraySmartP                   gpa1;
        Transform                       transform = Transform::From (extrusion);

        gpa1->CopyContentsOf (gpa);
        gpa1->Transform (&transform);

        contours.push_back (gpa);
        contours.push_back (gpa1);
        IFacetOptionsPtr options = IFacetOptions::New ();
        SetXGraphicsFacetOptions (*options);
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New (*options);
        builder->AddRuledBetweenCorespondingCurves (contours, enableCap);
        optimizedData.BeginDraw ();
        XGraphicsWriter (&optimizedData).WritePolyface (builder->GetClientMeshR (), true);
        pOptimizeData = pAfterEndSweep;

        return SUCCESS;
        }

    return ERROR;
    }
}; // XGraphicsBeginSweepExtrude

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsBeginSweepRevolve : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        enableCap;
    double      r0, r1, sweep;
    DPoint3d    origin;
    DVec3d      xVec, yVec, revAxis;

    Get (xVec, yVec, origin, r0, r1, revAxis, sweep, enableCap, pData);

    printf ("BeginSweepRevolve: Cap: %d\n", enableCap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        enableCap;
    double      rPrimary, rSecondary, sweepAngle;
    DVec3d      primary, secondary, revAxis;
    DRay3d      axisOfRotation;

    Get (primary, secondary, axisOfRotation.origin, rPrimary, rSecondary, axisOfRotation.direction, sweepAngle, enableCap, pData);
    revAxis.NormalizedCrossProduct (primary, secondary);
    if (revAxis.DotProduct (axisOfRotation.direction) < 0.0)
        axisOfRotation.direction.Negate ();

    opContext.BeginSweepRevolve (enableCap, axisOfRotation, sweepAngle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    bool        enableCap;
    double      r0, r1, sweep;
    DPoint3d    origin;
    DVec3d      xVec, yVec, revAxis;
    byte*       pData = &data[0];

    Get (xVec, yVec, origin, r0, r1, revAxis, sweep, enableCap, pData);

    xVec.scale (r0);
    yVec.scale (r1);

    transform.GetTransform()->multiply (&origin);
    transform.GetTransform()->multiplyMatrixOnly (&xVec);
    transform.GetTransform()->multiplyMatrixOnly (&yVec);
    transform.GetTransform()->multiplyMatrixOnly (&revAxis);
    r0 = xVec.normalize();
    r1 = yVec.normalize();
    revAxis.normalize();

    SET_AND_INCREMENT (xVec);
    SET_AND_INCREMENT (yVec);
    SET_AND_INCREMENT (origin);
    SET_AND_INCREMENT (r0);
    SET_AND_INCREMENT (r1);
    SET_AND_INCREMENT (revAxis);
    SET_AND_INCREMENT (sweep);

    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _TestTransform (TransformCR transform, byte* pData, UInt32 dataSize) override
    {
    return isTransformOrthogonalAndUniformScale (transform) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    bool        enableCap;
    double      r0, r1, sweep;
    DPoint3d    origin;
    DVec3d      xVec, yVec, revAxis;
    RotMatrix   rMatrix;

    Get (xVec, yVec, origin, r0, r1, revAxis, sweep, enableCap, pData);
    rMatrix.initFrom2Vectors (&xVec, &yVec);
    transform.initFrom (&rMatrix, &origin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    bool        thisEnableCap;
    double      thisR0, thisR1, thisSweep;
    DPoint3d    thisOrigin;
    DVec3d      thisXVec, thisYVec, thisRevAxis;

    Get (thisXVec, thisYVec, thisOrigin, thisR0, thisR1, thisRevAxis, thisSweep, thisEnableCap, data);

    bool        rhsEnableCap;
    double      rhsR0, rhsR1, rhsSweep;
    DPoint3d    rhsOrigin;
    DVec3d      rhsXVec, rhsYVec, rhsRevAxis;

    Get (rhsXVec, rhsYVec, rhsOrigin, rhsR0, rhsR1, rhsRevAxis, rhsSweep, rhsEnableCap, data);

    return thisXVec.isEqual (&rhsXVec, s_directionTolerance) &&
           thisYVec.isEqual (&rhsYVec, s_directionTolerance) &&
           thisOrigin.isEqual (&rhsOrigin, distanceTolerance) &&
           isEqual (thisR0, rhsR0, distanceTolerance) &&
           isEqual (thisR1, rhsR1, distanceTolerance) &&
           thisRevAxis.isEqual (&rhsRevAxis, s_directionTolerance) &&
           isEqual (thisSweep, rhsSweep, s_directionTolerance) &&
           thisEnableCap == rhsEnableCap;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    UInt16      opCode;
    UInt32      opSize;

    for (; pData < pEnd; pData += opSize - sizeof (opSize))
        {
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        XGraphicsOperations::Draw (opCode, &context, pData, opSize - sizeof (opSize), opContext);
        size += opSize + sizeof (opCode);

        if (XGRAPHIC_OpCode_EndSweep == opCode)
            return SUCCESS;
        }

    BeAssert (false); // No EndSweep found
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pOptimizeData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override
    {
    if (0 == (context.m_options & XGRAPHIC_OptimizeOptions_MeshFromRevolvedSweeps))
        return ERROR;

    bool        enableCap;
    double      r0, r1, sweep;
    DPoint3d    origin;
    DVec3d      xVec, yVec, revAxis, axis;
    byte*       pData = pOptimizeData;

    Get (xVec, yVec, origin, r0, r1, revAxis, sweep, enableCap, pData);

    byte        *pAfterBeginSweep = nextOp (pData, opSize), *pAfterEndSweep = pAfterBeginSweep, *pBoundary, *pBoundaryEnd, *pSweepEnd;
    StatusInt   status;

    if (SUCCESS != (status = XGraphicsOperations::FindOperation (&pSweepEnd, NULL, &pAfterEndSweep, pEnd, XGRAPHIC_OpCode_EndSweep)) ||
        SUCCESS != (status = XGraphicsOperations::FindBoundary (&pBoundary, &pBoundaryEnd, pAfterBeginSweep, pSweepEnd)))
        return status;

    GPArraySmartP   gpa;

    AppendComplexComponentsToGpaOperator appendToGpa (*gpa);
    XGraphicsOperations::Traverse (pBoundary, pBoundaryEnd, appendToGpa);

    optimizedData.BeginDraw ();

    axis.normalizedCrossProduct (&xVec, &yVec);

    IFacetOptionsPtr options = IFacetOptions::New ();
    SetXGraphicsFacetOptions (*options, true);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New (*options);
    builder->AddRotationalSweep (*gpa, origin, revAxis, sweep, enableCap);
    optimizedData.BeginDraw ();
    XGraphicsWriter (&optimizedData).WritePolyface (builder->GetClientMeshR (), true);
    pOptimizeData = pAfterEndSweep;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Get (DVec3d& xVec, DVec3d& yVec, DPoint3d& origin, double& r0, double& r1, DVec3d& revAxis, double& sweep, bool& enableCap, byte const* pData)
    {
    GET_AND_INCREMENT (xVec);
    GET_AND_INCREMENT (yVec);
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (r0);
    GET_AND_INCREMENT (r1);
    GET_AND_INCREMENT (revAxis);
    GET_AND_INCREMENT (sweep);
    GET_AND_INCREMENT (enableCap);
    }
}; // XGraphicsBeginSweepRevolve

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsEndSweep : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("EndSweep\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.EndSweep (context);
    return SUCCESS;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Should be included in other operation that's performing _CalculateRange - this function should not be called.
    BeAssert (false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    };
}; // XGraphicsEndSweep

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsBeginComplexString : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("BeginComplexString\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.UseUnCachedDraw (context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.BeginComplex (false, false);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    GPArraySmartP   gpa;

    AppendComplexComponentsToGpaOperator appendToGpa (*gpa);
    XGraphicsOperations::Traverse (pData, pEnd, appendToGpa);

    return getBasisTransformFromGPA (transform, *gpa);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _AppendToGPA (byte const* data, UInt32 dataSize, GPArrayR gpa) override
    {
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    UInt16      opCode;
    UInt32      opSize;

    for (; pData < pEnd; pData += opSize - sizeof (opSize))
        {
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        XGraphicsOperations::Draw (opCode, &context, pData, opSize - sizeof (opSize), opContext);
        size += opSize + sizeof (opCode);

        if (XGRAPHIC_OpCode_EndComplex == opCode)
            return SUCCESS;
        }

    BeAssert (false); // No EndComplex found
    return ERROR;
    }
}; // XGraphicsBeginComplexString

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsBeginComplexShape : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    { 
    bool        filled;

    GET_AND_INCREMENT (filled);

    printf ("BeginComplexShape, Filled: %d\n", filled); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;

    GET_AND_INCREMENT (filled);

    return opContext.UseUnCachedDraw (context, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;

    GET_AND_INCREMENT (filled);
    opContext.BeginComplex (true, filled);

    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    bool        filled;

    GET_AND_INCREMENT (filled);

    GPArraySmartP   gpa;

    AppendComplexComponentsToGpaOperator appendToGpa (*gpa);
    XGraphicsOperations::Traverse (pData, pEnd, appendToGpa);

    return getBasisTransformFromGPA (transform, *gpa);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    UInt16      opCode;
    UInt32      opSize;

    for (; pData < pEnd; pData += opSize - sizeof (opSize))
        {
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        XGraphicsOperations::Draw (opCode, &context, pData, opSize - sizeof (opSize), opContext);
        size += opSize + sizeof (opCode);

        if (XGRAPHIC_OpCode_EndComplex == opCode)
            return SUCCESS;
        }

    BeAssert (false); // No EndComplex found
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _AppendToGPA (byte const* data, UInt32 dataSize, GPArrayR gpa) override
    {
    return SUCCESS;
    }
}; // XGraphicsBeginComplexShape

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsEndComplex : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("EndComplex\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.EndComplex (context);
    return SUCCESS;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Should be included in other operation that's performing _CalculateRange - this function should not be called.
    BeAssert (false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    };
}; // XGraphicsEndComplex

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsAddDisconnect : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("AddDisconnect\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.AddDisconnect ();
    return SUCCESS;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Should be included in other operation that's performing _CalculateRange - this function should not be called.
    BeAssert (false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _AppendToGPA (byte const* data, UInt32 dataSize, GPArrayR gpa) override
    {
    gpa.MarkMajorBreak();
    return SUCCESS;
    }
}; // XGraphicsAddDisconnect


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsPushRenderOverrides : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) 
    { 
    ViewFlagsCP     viewFlags = (ViewFlagsCP) pData;

    pData += sizeof (ViewFlags);
    printf ("Push Render Overrides, RenderMode: %d\n", viewFlags->renderMode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize , XGraphicsOperationContext& opContext) override 
    {
    ViewFlagsCP viewFlags = (ViewFlagsCP) pData;

    pData += sizeof (ViewFlags);
    context.GetIViewDraw().PushRenderOverrides (*viewFlags, NULL);
    return SUCCESS;
    };

StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override {return SUCCESS;}
}; // XGraphicsPushRenderOverrides


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2011
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsPopRenderOverrides : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }
void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) { printf ("Pop RenderOverides \n"); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize , XGraphicsOperationContext& opContext) override 
    {
    context.GetIViewDraw().PopRenderOverrides ();
    return SUCCESS;
    };

StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override {return SUCCESS;}
}; // XGraphicsPopRenderOverrides

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsPushTransClip : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    TransformP  pTransform = (Transform *) pData;

    printf ("PushTransClip: \n");
    dumpTransform (*pTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    Transform   transform;

    // NEEDS_WORK - Clip.
    GET_AND_INCREMENT (transform);
    context.PushTransform (transform);

    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    Transform*  pTransform = (Transform *) &data[0];

    pTransform->productOf (transform.GetTransform(), pTransform);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    DPoint3d    vectors[4], rhsVectors[4];

    ((TransformCP) data)->getOriginAndVectors (&vectors[0], &vectors[1], &vectors[2], &vectors[3]);
    ((TransformCP) rhsData)->getOriginAndVectors (&rhsVectors[0], &rhsVectors[1], &rhsVectors[2], &rhsVectors[3]);

    if (!vectors[0].isEqual (&rhsVectors[0], distanceTolerance))
        return false;

    for (int i=1; i<4; i++)
        if (!vectors[i].isEqual (&rhsVectors[i], s_directionTolerance))
            return false;

    return true;
    }
}; // XGraphicsPushTransClip

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsPopTransClip : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("PopTransClip\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    context.PopTransformClip ();
    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override {return SUCCESS;}
}; // XGraphicsPopTransClip


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsPopAll : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("PopAll\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    context.GetTransformClipStack ().PopAll (context);

    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override {return SUCCESS;}

}; // XGraphicsPopAll

/*=================================================================================**//**
* @bsiclass                                                     JohnGooding     12/14
+===============+===============+===============+===============+===============+======*/
struct ParsedMatSymb
{
    UInt16              m_mask;
    ElemDisplayParams   m_params;
    Int64               m_materialId;
    ParsedMatSymb() : m_mask(0) {}
    void Clear() { m_mask = 0; }
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsMatSymb : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt16      matSymbMask;

    GET_AND_INCREMENT (matSymbMask);

    printf ("MatSymb, Mask: %x\n", matSymbMask);

    if (0 != (matSymbMask & XMATSYMB_LineColor))
        {
        UInt32  lineColor;

        GET_AND_INCREMENT (lineColor);
        printf ("Line Color: %x\n", (int)lineColor);
        }

    if (0 != (matSymbMask & XMATSYMB_LineColorIndex))
        {
        int     lineColorIndex;

        GET_AND_INCREMENT (lineColorIndex);
        printf ("Line Color Index: %d\n", lineColorIndex);
        }

    if (0 != (matSymbMask & XMATSYMB_FillColor))
        {
        UInt32  fillColor;

        GET_AND_INCREMENT (fillColor);
        printf ("Fill Color: %d\n", (int)fillColor);
        }

    if (0 != (matSymbMask & XMATSYMB_FillColorIndex))
        {
        int     fillColorIndex;

        GET_AND_INCREMENT (fillColorIndex);
        printf ("Fill Color Index %d\n", (int)fillColorIndex);
        }

    if (0 != (matSymbMask & XMATSYMB_RasterWidth))
        {
        UInt32  rasterWidth;

        GET_AND_INCREMENT (rasterWidth);
        printf ("Raster Width: %d\n", (int)rasterWidth);
        }

    if (0 != (matSymbMask & XMATSYMB_RasterPattern))
        {
        UInt32  rasterPattern;

        GET_AND_INCREMENT (rasterPattern);
        printf ("Raster Pattern: %x\n", (int)rasterPattern);
        }

    if (0 != (matSymbMask & XMATSYMB_FillDisplay))
        {
        FillDisplay fillDisplay;

        GET_AND_INCREMENT (fillDisplay);
        printf ("fillDisplay: %d\n", (int)fillDisplay);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColor))
        {
        UInt32  color;

        GET_AND_INCREMENT (color);
        printf ("Element Color: %d\n", (int)color);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
        {
        UInt32  color;

        GET_AND_INCREMENT (color);
        printf (" Element Fill Color: %d\n", (int)color);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemWeight))
        {
        UInt32  weight;

        GET_AND_INCREMENT (weight);
        printf (" Element Weight: %d\n", (int)weight);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
        {
        double  transparency;

        GET_AND_INCREMENT (transparency);
        printf (" Element Transparency: %f\n", transparency);
        }

    if (0 != (matSymbMask & XMATSYMB_SubElemIndex))
        {
        UInt32  subElemIndex;

        GET_AND_INCREMENT (subElemIndex);
        printf (" SubElemIndex: %d\n", (int)subElemIndex);
        }

    if (0 != (matSymbMask & XMATSYMB_Material))
        {
        ElementId materialId;

        GET_AND_INCREMENT (materialId);
        printf (" MaterialID: %lld\n", materialId.GetValue());
        }

    if (0 != (matSymbMask & XMATSYMB_ElemStyle))
        {
        Int32   style;

        GET_AND_INCREMENT (style);
        printf (" Element Style: %d\n", style);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColorRGB))
        {
        UInt32  colorRGB;

        GET_AND_INCREMENT (colorRGB);
        printf (" Element Color RGB: %x\n", (int)colorRGB);
        }

    if (0 != (matSymbMask & XMATSYMB_StyleParams))
        {
        UInt16  styleParamSize;

        GET_AND_INCREMENT (styleParamSize);
        printf ("  Style Params, Size: %d\n", styleParamSize);

        BeAssert (styleParamSize < sizeof (LineStyleParams));
        pData += styleParamSize;
        }
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt16          matSymbMask;
    ElemMatSymbP    elemMatSymb = context.GetElemMatSymb();

    GET_AND_INCREMENT (matSymbMask);

    if (0 != (matSymbMask & XMATSYMB_LineColor))
        {
        UInt32          lineColor;

        GET_AND_INCREMENT (lineColor);
        elemMatSymb->SetLineColorTBGR (lineColor);
        }

    if (0 != (matSymbMask & XMATSYMB_LineColorIndex))
        {
        int             lineColorIndex;

        GET_AND_INCREMENT (lineColorIndex);
        elemMatSymb->SetIndexedLineColorTBGR (lineColorIndex, context.GetIndexedColor (lineColorIndex));
        }

    if (0 != (matSymbMask & XMATSYMB_FillColor))
        {
        UInt32          fillColor;

        GET_AND_INCREMENT (fillColor);
        elemMatSymb->SetFillColorTBGR (fillColor);
        }

    if (0 != (matSymbMask & XMATSYMB_FillColorIndex))
        {
        int             fillColorIndex;

        GET_AND_INCREMENT (fillColorIndex);
        elemMatSymb->SetIndexedFillColorTBGR (fillColorIndex, context.GetIndexedColor (fillColorIndex));
        }

    if (0 != (matSymbMask & XMATSYMB_RasterWidth))
        {
        UInt32          rasterWidth;

        GET_AND_INCREMENT (rasterWidth);
        elemMatSymb->SetWidth (rasterWidth);
        }

    if (0 != (matSymbMask & XMATSYMB_RasterPattern))
        {
        UInt32          rasterPattern;

        GET_AND_INCREMENT (rasterPattern);
        elemMatSymb->SetRasterPattern (rasterPattern);
        }

    ElemDisplayParamsP  displayParams;
    UInt32              subElemIndex = 0;
    bool                doRecook = false;

    if (NULL != (displayParams = context.GetCurrentDisplayParams()))
        {
        if (0 != (matSymbMask & XMATSYMB_FillDisplay))
            {
            FillDisplay fillDisplay;
            
            GET_AND_INCREMENT (fillDisplay);
            displayParams->SetFillDisplay (fillDisplay);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_ElemColor))
            {
            UInt32  color;

            GET_AND_INCREMENT (color);
            displayParams->SetLineColor (color);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
            {
            UInt32  fillColor;

            GET_AND_INCREMENT (fillColor);
            displayParams->SetFillColor (fillColor);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_ElemWeight))
            {
            UInt32  weight;

            GET_AND_INCREMENT (weight);
            displayParams->SetWeight (weight);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
            {
            double  transparency;

            GET_AND_INCREMENT (transparency);
            displayParams->SetTransparency (transparency);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_SubElemIndex))
            {
            GET_AND_INCREMENT (subElemIndex);
            subElemIndex += opContext.m_baseElemId;
            }

        if (0 != (matSymbMask & XMATSYMB_Material))
            {
            ElementId   materialId;
            MaterialCP  material = NULL;

            GET_AND_INCREMENT (materialId);

#ifdef WIP_XGRAPHICS_MERGE // material
            if (0 != materialId )
                material = MaterialManager::GetManagerR ().FindMaterial (NULL, MaterialId (materialId), opContext.m_dgnProject, *opContext.m_model, false);
#endif

            displayParams->SetMaterial (material);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_ElemStyle))
            {
            Int32   style;

            GET_AND_INCREMENT (style);
            displayParams->SetLineStyle (style);            
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_ElemColorRGB))
            {
            UInt32  colorTBGR;

            GET_AND_INCREMENT (colorTBGR);
            displayParams->SetLineColorTBGR (colorTBGR);
            doRecook = true;
            }

        if (0 != (matSymbMask & XMATSYMB_StyleParams))
            {
            UInt16          styleParamSize;
            LineStyleParams styleParams;
            
            GET_AND_INCREMENT (styleParamSize);
            
            memset (&styleParams, 0, sizeof (styleParams));
            if (0 != LineStyleLinkageUtil::ExtractModifiers (&styleParams, const_cast <byte*> (pData), true))
                {
                displayParams->SetLineStyle (displayParams->GetLineStyle (), &styleParams);
                doRecook = true;
                }

            pData += styleParamSize;
            }
        }

    if (subElemIndex)
        {
        if (!opContext.WantMultiSymbologyBody ())
            return SUCCESS;

        if (opContext.IsMultiSymbologyBodyValid ())
            {
            T_FaceToSubElemIdMap& faceToSubElemIdMap = opContext.m_faceAttachments->_GetFaceToSubElemIdMapR ();
            T_FaceAttachmentsMap& faceAttachmentsMap = opContext.m_faceAttachments->_GetFaceAttachmentsMapR ();

            bool    foundSubElemId = false;

            for (T_FaceToSubElemIdMap::iterator curr = faceToSubElemIdMap.begin (); curr != faceToSubElemIdMap.end (); ++curr)
                {
                if (curr->second != subElemIndex)
                    continue;

                T_FaceAttachmentsMap::iterator found = faceAttachmentsMap.find (curr->first);

                if (found == faceAttachmentsMap.end ())
                    break;

                found->second = FaceAttachment (*displayParams, context);
                foundSubElemId = true;
                break;
                }

            BeAssert (foundSubElemId);
            }

        return SUCCESS;
        }

    if (doRecook)
        context.CookDisplayParams ();
    else
        context.GetIDrawGeom().ActivateMatSymb (elemMatSymb);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override {return SUCCESS;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
static bool lineStyleParamsEqual(LineStyleParamsCP left, LineStyleParamsCP right)
    {
    if (left == right)
        return true;

    if (NULL == left || NULL == right)
        return false;

    return !memcmp(left, right, sizeof(*left));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
static bool HasElemMatSymbParams(UInt16 mask)
    {
    // ElemMatSymb params - only set in legacy elements. Don't try to compress in this case.
    UInt16 allElemMatSymb = XMATSYMB_LineColor | XMATSYMB_LineColor | XMATSYMB_LineColorIndex | XMATSYMB_FillColor | XMATSYMB_FillColorIndex | XMATSYMB_RasterWidth | XMATSYMB_RasterPattern;
    return (mask & allElemMatSymb) != 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
static bool HasOnlyElemMatSymbParams(UInt16 mask)
    {
    // ElemMatSymb params - only set in legacy elements. Don't try to compress in this case.
    UInt16 allElemMatSymb = XMATSYMB_LineColor | XMATSYMB_LineColor | XMATSYMB_LineColorIndex | XMATSYMB_FillColor | XMATSYMB_FillColorIndex | XMATSYMB_RasterWidth | XMATSYMB_RasterPattern;
    return (mask & ~allElemMatSymb) == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
static bool CompareDisplayParamsToHeader (byte const* pData, ElementHandleCR eh)
    {
    DisplayHandlerP dh;
    DgnProjectP        dgnProject;
    if (NULL == (dh = eh.GetDisplayHandler()) || NULL == (dgnProject = eh.GetDgnProject()))
        return false;

    ElemDisplayParams dpFromElement; dpFromElement.Init();
    dh->GetElemDisplayParams (eh, dpFromElement, true);

    UInt16              matSymbMask;
    ElemDisplayParams   dpFromMatSymb;

    GET_AND_INCREMENT (matSymbMask);

    if (HasElemMatSymbParams(matSymbMask))
        return false;

    if (0 != (matSymbMask & XMATSYMB_FillDisplay))
        {
        FillDisplay  fillDisplay;
        BeAssert(sizeof(fillDisplay) == 4);
        GET_AND_INCREMENT (fillDisplay);
        dpFromMatSymb.SetFillDisplay(fillDisplay);
        if (dpFromElement.GetFillDisplay() != dpFromMatSymb.GetFillDisplay())
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColor))
        {
        UInt32 color;
        GET_AND_INCREMENT (color);
        dpFromMatSymb.SetLineColor(color);
        if (dpFromElement.GetLineColor() != color)
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
        {
        UInt32 fillColor;
        GET_AND_INCREMENT (fillColor);
        dpFromMatSymb.SetFillColor(fillColor);
        if (dpFromElement.GetFillColor() != fillColor)
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemWeight))
        {
        UInt32  weight;
        GET_AND_INCREMENT (weight);
        dpFromMatSymb.SetWeight(weight);
        if (dpFromElement.GetWeight() != weight)
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
        {
        double transparency;
        GET_AND_INCREMENT (transparency);
        dpFromMatSymb.SetTransparency(transparency);
        if (abs (transparency - dpFromElement.GetTransparency()) > Angle::SmallAngle())
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_SubElemIndex))
        return false;

    if (0 != (matSymbMask & XMATSYMB_Material))
        {
        Int64 rawMaterialId;
        GET_AND_INCREMENT (rawMaterialId);

#ifdef NEEDS_WORK_TopazMerge_XMATSYMB_Material
        if (0 != dpFromElement.m_rendMatID)
            {
            MaterialCP castMaterial = reinterpret_cast <MaterialCP> (dpFromElement.m_rendMatID);
            if (castMaterial->GetId().GetValue() != rawMaterialId)
                return false;
            }
        else if (rawMaterialId != 0)
            return false;
#endif
        }

    if (0 != (matSymbMask & XMATSYMB_ElemStyle))
        {
        Int32 lineStyle;
        GET_AND_INCREMENT (lineStyle);
        dpFromMatSymb.SetLineStyle(lineStyle);
        if (lineStyle != dpFromElement.GetLineStyle())
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColorRGB))
        {
        UInt32  colorTBGR;
        GET_AND_INCREMENT (colorTBGR);
        dpFromMatSymb.SetLineColorTBGR(colorTBGR);
        if (dpFromMatSymb.IsLineColorTBGR() != dpFromElement.IsLineColorTBGR() || 
            dpFromMatSymb.GetLineColorTBGR() != dpFromElement.GetLineColorTBGR())
            return false;
        }

    if (0 != (matSymbMask & XMATSYMB_StyleParams))
        {
        UInt16 styleParamSize;

        GET_AND_INCREMENT (styleParamSize);

        LineStyleParams lineStyleParams;
        if (0 != LineStyleLinkageUtil::ExtractModifiers (&lineStyleParams, const_cast <byte*> (pData), true))
            {
            dpFromMatSymb.SetLineStyle(dpFromMatSymb.GetLineStyle(), &lineStyleParams);
            }

        pData += styleParamSize;
        LineStyleParamsCP fromElement = dpFromElement.GetLineStyleParams();
        LineStyleParamsCP fromMatSym = dpFromMatSymb.GetLineStyleParams();
        if (!lineStyleParamsEqual(fromElement, fromMatSym))
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
static StatusInt CombineDisplayParamsWithParsed (ParsedMatSymb& parsed, byte const* pData)
    {
    UInt16              matSymbMask;

    GET_AND_INCREMENT (matSymbMask);
    if (HasElemMatSymbParams(matSymbMask))
        {
        //  If this has both ElemMatSymbParams and ElemDisplayParams the ElemDisplayParams would
        //  cause a recook overwriting the ElemMatSymbParams.  If we have both then we should just
        //  ignore the ElemMatSymbParams.  For now I am assuming that no operation will have both.
        return BSIERROR;
        }

    if (0 != (matSymbMask & XMATSYMB_SubElemIndex))
        {
        //  Don't know what to do with this
        BeAssert(0 == (matSymbMask & XMATSYMB_SubElemIndex));
        return BSIERROR;
        }

    if (0 != (matSymbMask & XMATSYMB_FillDisplay))
        {
        Int32 fillDisplay;
        GET_AND_INCREMENT(fillDisplay);
        parsed.m_params.SetFillDisplay(FillDisplay(fillDisplay));
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColor))
        {
        UInt32 color;
        GET_AND_INCREMENT (color);
        parsed.m_params.SetLineColor(color);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
        {
        UInt32 fillColor;
        GET_AND_INCREMENT (fillColor);
        parsed.m_params.SetFillColor(fillColor);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemWeight))
        {
        UInt32 weight;
        GET_AND_INCREMENT (weight);
        parsed.m_params.SetWeight(weight);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
        {
        double transparency;
        GET_AND_INCREMENT (transparency);
        parsed.m_params.SetTransparency(transparency);
        }

    if (0 != (matSymbMask & XMATSYMB_Material))
        {
        Int64 materialId;
        GET_AND_INCREMENT (materialId)
        //  We don't want to access the material object here so we don't use ElemDisplayParamsm_material.
        parsed.m_materialId = materialId;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemStyle))
        {
        Int32 style;
        GET_AND_INCREMENT (style);
        parsed.m_params.SetLineStyle(style, NULL);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColorRGB))
        {
        //  We don't care about parsed.m_params.m_useElemColorRGB here. We are just using this data to merge streams.
        //  If we use this to draw then we do need to set m_useElemColorRGB correctly.
        UInt32 colorRGB;
        GET_AND_INCREMENT (colorRGB);
        parsed.m_params.SetLineColorTBGR(colorRGB);
        }

    if (0 != (matSymbMask & XMATSYMB_StyleParams))
        {
        UInt16 styleParamSize;

        GET_AND_INCREMENT (styleParamSize);
        LineStyleParams lineStyleParams;

        if (0 != LineStyleLinkageUtil::ExtractModifiers (&lineStyleParams, const_cast <byte*> (pData), true))
            {
            parsed.m_params.SetLineStyle(parsed.m_params.GetLineStyle(), &lineStyleParams);
            }

        pData += styleParamSize;
        }

    parsed.m_mask |= matSymbMask;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
static bool ExtractDisplayParamsToHeader (byte const* pData, EditElementHandleR eeh)
    {
    DisplayHandlerP dh;
    DgnProjectP        project;
    if (NULL == (dh = eeh.GetDisplayHandler()) || NULL == (project = eeh.GetDgnProject()))
        return false;

    ElemDisplayParams dpFromElement; dpFromElement.Init();
    dh->GetElemDisplayParams (eeh, dpFromElement, true);

    UInt16              matSymbMask;
    ElemDisplayParams   dpFromMatSymb;
    bool                needToSetFillLinkage = false, isModified = false;

    GET_AND_INCREMENT (matSymbMask);

    // ElemMatSymb params - ignore. If display params are valid, these are thrown out anyway.
    if (0 != (matSymbMask & XMATSYMB_LineColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_LineColorIndex))
        pData += sizeof (int);

    if (0 != (matSymbMask & XMATSYMB_FillColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_FillColorIndex))
        pData += sizeof (int);

    if (0 != (matSymbMask & XMATSYMB_RasterWidth))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_RasterPattern))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_FillDisplay))
        {
        FillDisplay fillDisplay;
        GET_AND_INCREMENT (fillDisplay);
        dpFromMatSymb.SetFillDisplay(fillDisplay);
        if (dpFromElement.GetFillDisplay() != dpFromMatSymb.GetFillDisplay())
            {
            if (FillDisplay::Never == dpFromMatSymb.GetFillDisplay())
                {
                isModified = true;
                mdlElement_displayAttributeRemove (eeh.GetElementP (), FILL_ATTRIBUTE);
                mdlElement_displayAttributeRemove (eeh.GetElementP (), GRADIENT_ATTRIBUTE);
                }
            else
                needToSetFillLinkage = true;    // Have to wait to get color in order to set this properly.
            }
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColor))
        {
        UInt32 color;
        GET_AND_INCREMENT (color);
        dpFromMatSymb.SetLineColor(color);
        if (dpFromElement.GetLineColor() != color)
            {
            isModified = true;
            eeh.GetElementP()->GetSymbologyR().color = color;
            }
        }

    if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
        {
        UInt32 fillColor;

        GET_AND_INCREMENT (fillColor);
        dpFromMatSymb.SetFillColor(fillColor);

        // If the element will end up with a non-never fill linkage and the XG stream specifies fill color, we need
        // to update the fill linkage.
        needToSetFillLinkage = needToSetFillLinkage || FillDisplay::Never != dpFromElement.GetFillDisplay();
        }

    if (needToSetFillLinkage)
        {
        bool    alwaysFilled;
        UInt32  fillColor;

        if (0 != (matSymbMask & XMATSYMB_FillDisplay))
            alwaysFilled = FillDisplay::Always == dpFromMatSymb.GetFillDisplay();
        else
            alwaysFilled = FillDisplay::Always == dpFromElement.GetFillDisplay();

        if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
            fillColor = dpFromMatSymb.GetFillColor();
        else
            fillColor = dpFromElement.GetFillColor();

        IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetHandler ());

        if (areaObj)
            areaObj->AddSolidFill (eeh, &fillColor, &alwaysFilled);

        isModified = true;
        }

    if (0 != (matSymbMask & XMATSYMB_ElemWeight))
        {
        UInt32 weight;
        GET_AND_INCREMENT (weight);
        dpFromMatSymb.SetWeight(weight);
        if (dpFromElement.GetWeight() != weight)
            {
            isModified = true;
            eeh.GetElementP()->GetSymbologyR().weight = weight;
            }
        }

    if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
        {
        double transparency;
        GET_AND_INCREMENT (transparency);
        dpFromMatSymb.SetTransparency(transparency);
        if (abs (transparency - dpFromElement.GetTransparency()) > Angle::SmallAngle())
            {
            isModified = true;
            if (transparency < Angle::SmallAngle())
                mdlElement_displayAttributeRemove (eeh.GetElementP(), TRANSPARENCY_ATTRIBUTE);
            else
                mdlElement_addTransparencyDisplayAttribute (eeh, transparency);
            }
        }

    if (0 != (matSymbMask & XMATSYMB_SubElemIndex))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_Material))
        {
        Int64 rawMaterialId;

        GET_AND_INCREMENT (rawMaterialId);

#ifdef NEEDS_WORK_TopazMerge_Materials
        DgnMaterialId materialId (rawMaterialId);
        if (materialId.IsValid() && NULL != MaterialManager::GetManagerR().FindMaterial (materialId, project))
            {
            isModified = true;
            MaterialManager::GetManagerR().SetMaterialAttachmentId (eeh, materialId);
            }
#endif
        }

    if (0 != (matSymbMask & XMATSYMB_ElemStyle))
        {
        Int32 style;

        GET_AND_INCREMENT (style);
        dpFromMatSymb.SetLineStyle(style);
        if (style != dpFromElement.GetLineStyle())
            {
            isModified = true;
            eeh.GetElementP()->GetSymbologyR().style = style;
            }
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColorRGB))
        {
        UInt32 elemColorRGB;
        GET_AND_INCREMENT (elemColorRGB);
        dpFromMatSymb.SetLineColorTBGR(elemColorRGB);
        BeAssert(dpFromMatSymb.IsLineColorTBGR());
        if (!dpFromElement.IsLineColorTBGR() || elemColorRGB != dpFromElement.GetLineColorTBGR())
            {
            isModified = true;
            eeh.GetElementP()->GetSymbologyR().color = project->Colors().CreateElementColor (elemColorRGB, NULL, NULL);
            }
        }

    if (0 != (matSymbMask & XMATSYMB_StyleParams))
        {
        UInt16 styleParamSize;

        GET_AND_INCREMENT (styleParamSize);

        LineStyleParams params;
        if (0 != LineStyleLinkageUtil::ExtractModifiers (&params, const_cast <byte*> (pData), true))
            {
            dpFromMatSymb.SetLineStyle(dpFromMatSymb.GetLineStyle(), &params);
            }

        pData += styleParamSize;

        LineStyleParamsCP paramsFromElement = dpFromElement.GetLineStyleParams();
        LineStyleParamsCP paramsFromMatSymb = dpFromMatSymb.GetLineStyleParams();
        if (!lineStyleParamsEqual(paramsFromElement, paramsFromMatSymb))
            {
            LineStyle_setElementStyle(eeh, dpFromMatSymb.GetLineStyle(), (LineStyleParamsP)paramsFromMatSymb);
            isModified = true;
            }
        }

    return isModified;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     11/12
//---------------------------------------------------------------------------------------
static bool ExtractAndRemoveSubElementIndex (UInt32& subElementIndex, UInt16& matSymbMask, bvector <byte>& data)
    {
    byte*       pData = &data.front();
    memcpy (&matSymbMask, pData, sizeof (matSymbMask));

    // No sub element index, nothing to be done.
    if (0 == (matSymbMask & XMATSYMB_SubElemIndex))
        return false;

    // Switch off this operation.
    matSymbMask &= ~(XMATSYMB_SubElemIndex);
    memcpy (pData, &matSymbMask, sizeof (matSymbMask));
    pData += sizeof (matSymbMask);

    // NOTE: Stored are differences from the header, so don't announce as base ids.
    if (0 != (matSymbMask & XMATSYMB_LineColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_LineColorIndex))
        pData += sizeof (int);

    if (0 != (matSymbMask & XMATSYMB_FillColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_FillColorIndex))
        pData += sizeof (int);

    if (0 != (matSymbMask & XMATSYMB_RasterWidth))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_RasterPattern))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_FillDisplay))
        pData += sizeof (FillDisplay);

    if (0 != (matSymbMask & XMATSYMB_ElemColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_ElemWeight))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
        pData += sizeof (double);

    memcpy (&subElementIndex, pData, sizeof (subElementIndex));

    byte*       pEnd = pData + data.size();
    byte*       pRemaining = pData + sizeof (subElementIndex);

    if (pRemaining < pEnd)
        {
        ptrdiff_t   bytesRemaining = pEnd - pRemaining;

        memcpy (pData, pRemaining, bytesRemaining);
        data.resize (data.size() - sizeof (subElementIndex));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _ProcessProperties (byte* pData, UInt32 dataSize, PropertyContextR context) override
    {
    UInt16      matSymbMask;

    GET_AND_INCREMENT (matSymbMask);

    // NOTE: Stored are differences from the header, so don't announce as base ids.
    if (0 != (matSymbMask & XMATSYMB_LineColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_LineColorIndex))
        pData += sizeof (int);

    if (0 != (matSymbMask & XMATSYMB_FillColor))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_FillColorIndex))
        pData += sizeof (int);

    if (0 != (matSymbMask & XMATSYMB_RasterWidth))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_RasterPattern))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_FillDisplay))
        pData += sizeof (FillDisplay);

    if (0 != (matSymbMask & XMATSYMB_ElemColor))
        {
        XGScopedArrayRW alignedData;
        UInt32* colorP = (UInt32*) alignedData.AcquireRW (pData, sizeof (UInt32));

        if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
            context.DoColorCallback (colorP, EachColorArg (*colorP, PROPSCALLBACK_FLAGS_NoFlagsSet, context));

        pData += sizeof (UInt32);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
        {
        XGScopedArrayRW alignedData;
        UInt32* colorP = (UInt32*) alignedData.AcquireRW (pData, sizeof (UInt32));

        if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
            context.DoColorCallback (colorP, EachColorArg (*colorP, PROPSCALLBACK_FLAGS_IsBackgroundID, context));

        pData += sizeof (UInt32);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemWeight))
        {
        XGScopedArrayRW alignedData;
        UInt32* weightP = (UInt32*) alignedData.AcquireRW (pData, sizeof (UInt32));

        if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
            context.DoWeightCallback (weightP, EachWeightArg (*weightP, PROPSCALLBACK_FLAGS_NoFlagsSet, context));

        pData += sizeof (UInt32);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemTransparency))
        {
        XGScopedArrayRW alignedData;
        double* transparencyP = (double*) alignedData.AcquireRW (pData, sizeof (double));

        if (0 != (ELEMENT_PROPERTY_Transparency & context.GetElementPropertiesMask ()))
            context.DoTransparencyCallback (transparencyP, EachTransparencyArg (*transparencyP, PROPSCALLBACK_FLAGS_NoFlagsSet, context));

        pData += sizeof (double);
        }

    if (0 != (matSymbMask & XMATSYMB_SubElemIndex))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_Material))
        {
        //  #ifdef NEEDS_WORK_TopazMerge_ -- compare changes for materials
        XGScopedArrayRW alignedData;
        DgnMaterialId* elemIdP = (DgnMaterialId*)alignedData.AcquireRW (pData, sizeof (DgnMaterialId));
        DgnMaterialId newId = *elemIdP;

        if (0 != (ELEMENT_PROPERTY_Material & context.GetElementPropertiesMask ()))
            {
            if (context.DoMaterialCallback (&newId, EachMaterialArg (newId, PROPSCALLBACK_FLAGS_NoFlagsSet, context)))
                *elemIdP = newId;
            }

        pData += sizeof (ElementId);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemStyle))
        {
        XGScopedArrayRW alignedData;
        Int32* styleP = (Int32*) alignedData.AcquireRW (pData, sizeof (Int32));

        if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
            context.DoLineStyleCallback (styleP, EachLineStyleArg (*styleP, NULL, PROPSCALLBACK_FLAGS_NoFlagsSet, context));

        pData += sizeof (Int32);
        }

    if (0 != (matSymbMask & XMATSYMB_ElemColorRGB))
        pData += sizeof (UInt32);

    if (0 != (matSymbMask & XMATSYMB_StyleParams))
        {
        UInt16  styleParamSize;

        // NOTE: Should include modifiers when announcing style id...but then we'd have to keep track of the current style...
        GET_AND_INCREMENT (styleParamSize);
        pData += styleParamSize;
        }
    }

}; // XGraphicsMatSymb

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct XGraphicsMatSymb2 : XGraphicsOperation
{
    virtual bool      _IsStateChange () override { return true; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt32      matSymbMask;

    GET_AND_INCREMENT (matSymbMask);

    printf ("MatSymb2, Mask: %x\n", (int)matSymbMask);

    if (0 != (matSymbMask & XMATSYMB2_GradientFill))
        {
        UInt16  gradientSize;

        GET_AND_INCREMENT (gradientSize);
        printf ("  Gradient Fill, Size: %d\n", gradientSize);

        BeAssert (gradientSize < sizeof (Display_attribute_gradient));
        pData += gradientSize;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override {return SUCCESS;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContext& opContext) override
    {
    ElemMatSymbP       elemMatSymb = context.GetElemMatSymb ();
    ElemDisplayParamsP displayParams = context.GetCurrentDisplayParams ();

    if (NULL == elemMatSymb || NULL == displayParams)
        return SUCCESS;

    UInt32 matSymbMask;

    GET_AND_INCREMENT (matSymbMask);

    if (0 != (matSymbMask & XMATSYMB2_GradientFill))
        {
        UInt16  gradientSize;

        GET_AND_INCREMENT (gradientSize);

        if (0 == gradientSize)
            {
            displayParams->SetGradient (NULL);
            }
        else
            {
            Display_attribute_gradient gradientAttr;

            memset (&gradientAttr, 0, sizeof (gradientAttr));
            memcpy (&gradientAttr, pData, gradientSize);

            GradientSymbPtr     gradient = GradientSymb::Create();

            gradient->FromDisplayAttribute (&gradientAttr);

            // NOTE: XGraphicsMatSymb will either re-cook or activate, so setup both ElemMatSymb and ElemDisplayParams...
            displayParams->SetGradient (gradient.get());
            elemMatSymb->SetGradient (gradient.get());

            pData += gradientSize;
            }
        }

    return SUCCESS;
    }
}; // XGraphicsMatSym2

//=======================================================================================
// @bsiclass                                                    MattGooding     12/12
//=======================================================================================
struct XGraphicsNoOp : XGraphicsOperation
{
    virtual void _Dump (byte const*, UInt32, XGraphicsOperationContextR) override { printf ("NoOp\n"); }
    virtual StatusInt _Draw (ViewContextR, byte const*, UInt32, XGraphicsOperationContextR) override { return SUCCESS; }
    virtual StatusInt _OnTransform (TransformInfoCR, XGraphicsDataR) override { return SUCCESS; }
    virtual bool _IsEqual (byte const*, byte const*, UInt32, double) override { return true; }
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsDrawBody : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    Transform   *pTransform = (Transform*) pData;

    printf ("DrawBody: Transform: \n (%f,%f,%f,%f)\n(%f,%f,%f,%f)\n(%f,%f,%f,%f)\n",
            pTransform->form3d[0][0], pTransform->form3d[0][1], pTransform->form3d[0][2], pTransform->form3d[0][3],
            pTransform->form3d[1][0], pTransform->form3d[1][1], pTransform->form3d[1][2], pTransform->form3d[1][3],
            pTransform->form3d[2][0], pTransform->form3d[2][1], pTransform->form3d[2][2], pTransform->form3d[2][3]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsCachable (ViewContextR viewContext, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return true; // Always need QvElem to display surface...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.AllowDrawWireframe (context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.m_brepOffset = (pData - (const byte*) opContext.m_header);

    // Check for existing edge geometry cache tp display before restoring entity...
    if (opContext.AllowDrawWireframe (context))
        {
        BentleyStatus status = BRepWireGraphicsAppData::DrawWireGeomCache (opContext, context);

        if (SUCCESS == status)
            return SUCCESS;
        }

    Transform   transform;
    UInt32      kernelType, bufferSize;

    GET_AND_INCREMENT (transform);
    GET_AND_INCREMENT (kernelType);
    GET_AND_INCREMENT (bufferSize);

    ISolidKernelEntityPtr   entityPtr;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory (entityPtr, pData, bufferSize, (ISolidKernelEntity::SolidKernelType) kernelType, transform))
        return SUCCESS;

    if (0 != (opContext.m_drawOptions & XGraphicsContainer::DRAW_OPTION_ClipStencil))
        {
        // Output planar bodies as shapes/complex shapes when used as clip stencil...
        T_HOST.GetSolidsKernelAdmin()._OutputBodyAsSurfaces (*entityPtr, context);
        }
    else if (opContext.m_inMultiSymbBody)
        {
        // Populate attachment info for all faces using the current display params and don't check for face material attributes...
        opContext.m_faceAttachments = T_HOST.GetSolidsKernelAdmin()._GetFaceMaterialAttachments (*entityPtr, context, *context.GetCurrentDisplayParams (), opContext.m_baseElemId, NULL, false);
        opContext.m_brepEntity = entityPtr;
        }
    else
        {
        opContext.DrawBodyAndCacheEdges (context, *entityPtr);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    Transform   *pTransform = (Transform*) &data[0];

    pTransform->productOf (transform.GetTransform(), pTransform);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       _TestTransform (TransformCR transform, byte* pData, UInt32 dataSize) override
    {
    return  isTransformOrthogonalAndUniformScale (transform) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    UInt32      kernelType, bufferSize;

    GET_AND_INCREMENT (transform);
    GET_AND_INCREMENT (kernelType);
    GET_AND_INCREMENT (bufferSize);

    ISolidKernelEntityPtr   entityPtr;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory (entityPtr, pData, bufferSize, (ISolidKernelEntity::SolidKernelType) kernelType, transform))
        return ERROR;

    T_HOST.GetSolidsKernelAdmin()._GetEntityBasisTransform (transform, *entityPtr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    Transform   transform, rhsTransform;
    UInt32      kernelType, rhsKernelType, bufferSize, rhsBufferSize;

    GET_AND_INCREMENT_DATA (transform, data);
    GET_AND_INCREMENT_DATA (kernelType, data);
    GET_AND_INCREMENT_DATA (bufferSize, data);

    GET_AND_INCREMENT_DATA (rhsTransform, rhsData);
    GET_AND_INCREMENT_DATA (rhsKernelType, rhsData);
    GET_AND_INCREMENT_DATA (rhsBufferSize, rhsData);

    if (kernelType != rhsKernelType || bufferSize != rhsBufferSize)
        return false;

    if (transform.IsIdentity () && rhsTransform.IsIdentity () && 0 == memcmp (data, rhsData, bufferSize))
        return true;

    ISolidKernelEntityPtr   entityPtr1;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory (entityPtr1, data, bufferSize, (ISolidKernelEntity::SolidKernelType) kernelType, transform))
        return false;

    ISolidKernelEntityPtr   entityPtr2;

    if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._RestoreEntityFromMemory (entityPtr2, rhsData, rhsBufferSize, (ISolidKernelEntity::SolidKernelType) rhsKernelType, rhsTransform))
        return false;

    return T_HOST.GetSolidsKernelAdmin()._AreEntitiesEqual (*entityPtr1, *entityPtr2, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override
    {
    bool        meshPlanar = 0 != (context.m_options & XGRAPHIC_OptimizeOptions_MeshFromPlanarBRep);
    bool        meshAll    = 0 != (context.m_options & XGRAPHIC_OptimizeOptions_MeshParasolid);

    if (!meshAll && !meshPlanar)
        return ERROR;

    StatusInt       status;

    if (SUCCESS == (status = FacetsFromBody (optimizedData, pData, opSize, pEnd, !meshAll)))
        pData += (opSize - sizeof (opSize));

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FacetsFromBody (XGraphicsContainerR facetData, byte const* pData, UInt32 opSize, byte const* pEnd, bool planarOnly)
    {
    StatusInt               status;
    Transform               transform;
    IFacetTopologyTablePtr  facetsPtr;
    if (SUCCESS != (status = facetTopologyTableFromBody (facetsPtr, transform, pData, opSize, pEnd, planarOnly)))
        return status;

    PolyfaceHeaderPtr polyface = PolyfaceHeader::New ();
    IFacetOptionsPtr    options     = IFacetOptions::New ();
    SetXGraphicsFacetOptions(*options.get(), !planarOnly);

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface (*polyface, *facetsPtr, *options.get()))
        return ERROR;

    polyface->Transform (transform, true);

    facetData.BeginDraw ();
    XGraphicsWriter (&facetData).WritePolyface (*polyface, true);

    return SUCCESS;
    }

}; // XGraphicsDrawBody

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawBSplineCurve : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("DrawBsplineCurve\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    bool        filled;

    GET_AND_INCREMENT (filled);

    return opContext.UseUnCachedDraw (context, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    //  #ifdef NEEDS_WORK_TopazMerge_ -- check for alignment issues?
    bool                filled;
    MSBsplineCurvePtr   curve = MSBsplineCurve::CreatePtr();

    GET_AND_INCREMENT (filled);
    Get (*curve, pData);

    if (opContext.IsCurvePrimitiveRequired ())
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateBsplineCurve (curve);

        opContext.DrawOrAddComplexComponent (primitive, context, TO_BOOL (curve->params.closed), filled, true, 0.0);

        return SUCCESS;
        }

    bool    displayFilled = opContext.AllowDrawFilled (context, filled);

    if (!displayFilled && opContext.AllowDrawStyled (context, TO_BOOL (curve->params.closed)))
        {
        context.DrawStyledBSplineCurve3d (*curve);
        }
    else if (curve->params.closed && filled && context.GetIViewDraw ().IsOutputQuickVision ())
        {
        // NOTE: Unlike other region types (ellipse/shape/complex shape) a closed bspline renders as an outline unless filled.
        //       We need to create a complex shape to have a single cached representation that behaves correctly in wireframe and shaded views...
        CurveVectorPtr  tmpCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

        tmpCurve->push_back (ICurvePrimitive::CreateBsplineCurve (*curve));
        context.GetIDrawGeom ().DrawCurveVector (*tmpCurve, displayFilled);
        }
    else
        {
        // NOTE: Don't need "never filled" case since a closed bcurve won't render as a surface when not part of a complex shape...
        context.GetIDrawGeom ().DrawBSplineCurve (*curve, displayFilled);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    //  #ifdef NEEDS_WORK_TopazMerge_ -- look for alignment issues.
    bool                filled;
    MSBsplineCurvePtr   curve = MSBsplineCurve::CreatePtr();
    byte*               pData = &data[0];

    GET_AND_INCREMENT (filled);
    Get (*curve, pData);
    curve->TransformCurve (*transform.GetTransform ());

    Int32               type, rational;
    BsplineDisplay      display;
    BsplineParam        params;

    // Get (primarily just to increment pData appropriately).
    GET_AND_INCREMENT (type);
    GET_AND_INCREMENT_BOOL (rational);

    GET_AND_INCREMENT_BOOL (display.polygonDisplay);
    GET_AND_INCREMENT_BOOL (display.curveDisplay);
    GET_AND_INCREMENT_BOOL (display.rulesByLength);

    GET_AND_INCREMENT (params.order);
    GET_AND_INCREMENT_BOOL (params.closed);
    GET_AND_INCREMENT (params.numPoles);
    GET_AND_INCREMENT (params.numKnots);
    GET_AND_INCREMENT (params.numRules);

    if (params.numPoles != curve->params.numPoles)
        {
        BeAssert (false);       // Should never happen.
        return ERROR;
        }
    memcpy (pData, curve->poles, params.numPoles * sizeof (DPoint3d));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    bool            filled;
    MSBsplineCurvePtr   curve = MSBsplineCurve::CreatePtr();

    GET_AND_INCREMENT (filled);
    Get (*curve, pData);

    StatusInt status = getBasisTransformFromPoints (transform, curve->poles, curve->rational ? curve->weights : NULL, curve->params.numPoles);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    bool            filled, rhsFilled;
    MSBsplineCurvePtr   curve = MSBsplineCurve::CreatePtr(), rhsCurve = MSBsplineCurve::CreatePtr();

    GET_AND_INCREMENT_DATA (filled, data);
    Get (*curve, data);

    GET_AND_INCREMENT_DATA (rhsFilled, rhsData);
    Get (*rhsCurve, rhsData);

    if (curve->rational               != rhsCurve->rational ||
        curve->params.numPoles        != rhsCurve->params.numPoles ||
        curve->params.numKnots        != rhsCurve->params.numKnots ||
        curve->params.closed          != rhsCurve->params.closed ||
        curve->display.polygonDisplay != rhsCurve->display.polygonDisplay ||
        curve->display.curveDisplay   != rhsCurve->display.curveDisplay)
        return false;

    return arePointsEqual (curve->poles, curve->params.numPoles, rhsCurve->poles, rhsCurve->params.numPoles, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _AppendToGPA (byte const* pData, UInt32 dataSize, GPArrayR gpa) override
    {
    bool                filled;
    MSBsplineCurvePtr   curve = MSBsplineCurve::CreatePtr();

    GET_AND_INCREMENT (filled);
    Get (*curve, pData);

    gpa.Add (*curve);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Get (MSBsplineCurve& curve, byte const* pData)
    {
    curve.Zero();
    GET_AND_INCREMENT (curve.type);
    GET_AND_INCREMENT_BOOL (curve.rational);

    GET_AND_INCREMENT_BOOL (curve.display.polygonDisplay);
    GET_AND_INCREMENT_BOOL (curve.display.curveDisplay);
    GET_AND_INCREMENT_BOOL (curve.display.rulesByLength);

    GET_AND_INCREMENT (curve.params.order);
    GET_AND_INCREMENT_BOOL (curve.params.closed);
    GET_AND_INCREMENT (curve.params.numPoles);
    GET_AND_INCREMENT (curve.params.numKnots);
    GET_AND_INCREMENT (curve.params.numRules);

    curve.Allocate ();
    size_t      poleSize = curve.params.numPoles * sizeof (DPoint3d);

    memcpy (curve.poles, pData, poleSize);
    pData += poleSize;

    size_t      knotSize = curve.NumberAllocatedKnots () * sizeof (double);

    if (0 != knotSize)
        {
        memcpy (curve.knots, pData, knotSize);
        pData += knotSize;
        }

    if (curve.rational)
        {
        memcpy (curve.weights, pData, curve.params.numPoles * sizeof (double));
        // No need to increment pData (it is input only).
        }
    }

}; // XGraphicsDrawBSplineCurve

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawBSplineSurface : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("DrawBsplineSurface\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    MSBsplineSurfacePtr     surface = MSBsplineSurface::CreatePtr();

    Get (*surface, pData, true);

    context.GetIDrawGeom().DrawBSplineSurface (*surface);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    MSBsplineSurfacePtr     surface = MSBsplineSurface::CreatePtr();
    byte*                   pData = &data[0];

    Get (*surface, pData, true);

    surface->TransformSurface (transform.GetTransform ());

    Int32           type, rational;
    BsplineDisplay  display;
    BsplineParam    uParams, vParams;


    // Primarily to increment pointer appropriately.
    GET_AND_INCREMENT (type);
    GET_AND_INCREMENT_BOOL (rational);

    GET_AND_INCREMENT_BOOL (display.polygonDisplay);
    GET_AND_INCREMENT_BOOL (display.curveDisplay);
    GET_AND_INCREMENT_BOOL (display.rulesByLength);

    GET_AND_INCREMENT (uParams.order);
    GET_AND_INCREMENT_BOOL (uParams.closed);
    GET_AND_INCREMENT (uParams.numPoles);
    GET_AND_INCREMENT (uParams.numKnots);
    GET_AND_INCREMENT (uParams.numRules);

    GET_AND_INCREMENT (vParams.order);
    GET_AND_INCREMENT_BOOL (vParams.closed);
    GET_AND_INCREMENT (vParams.numPoles);
    GET_AND_INCREMENT (vParams.numKnots);
    GET_AND_INCREMENT (vParams.numRules);

    if (vParams.numPoles != surface->vParams.numPoles ||
        uParams.numPoles != surface->uParams.numPoles)
        {
        BeAssert (false);       // Should never happen but test to be safe..
        return ERROR;
        }
    memcpy (pData, surface->poles, uParams.numPoles * vParams.numPoles * sizeof (DPoint3d));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    MSBsplineSurfacePtr     surface = MSBsplineSurface::CreatePtr();

    Get (*surface, pData, true);

    int         numPoles = surface->uParams.numPoles * surface->vParams.numPoles;

    StatusInt status = getBasisTransformFromPoints (transform, surface->poles, surface->rational ? surface->weights : NULL,
            numPoles);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    MSBsplineSurfacePtr     surface     = MSBsplineSurface::CreatePtr ();
    MSBsplineSurfacePtr     rhsSurface  = MSBsplineSurface::CreatePtr ();

    // Note - we don't bother checking boundaries.  But its a pretty safe assumption that
    // if the poles coincide that the bounds will match.
    Get (*surface, data, false);
    Get (*rhsSurface, rhsData, false);

    if (surface->rational != rhsSurface->rational ||
        0 != memcmp (&surface->uParams, &rhsSurface->uParams, sizeof (surface->uParams)) ||
        0 != memcmp (&surface->vParams, &rhsSurface->vParams, sizeof (surface->vParams)))
        return false;


    int         numPoles = surface->uParams.numPoles * surface->vParams.numPoles;

    return arePointsEqual (surface->poles, numPoles, rhsSurface->poles, numPoles, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void Get (MSBsplineSurface& surface, byte const* pData, bool getBoundaries)
    {
    surface.Zero();

    GET_AND_INCREMENT (surface.type);
    GET_AND_INCREMENT_BOOL (surface.rational);

    GET_AND_INCREMENT_BOOL (surface.display.polygonDisplay);
    GET_AND_INCREMENT_BOOL (surface.display.curveDisplay);
    GET_AND_INCREMENT_BOOL (surface.display.rulesByLength);

    GET_AND_INCREMENT (surface.uParams.order);
    GET_AND_INCREMENT_BOOL (surface.uParams.closed);
    GET_AND_INCREMENT (surface.uParams.numPoles);
    GET_AND_INCREMENT (surface.uParams.numKnots);
    GET_AND_INCREMENT (surface.uParams.numRules);
    int numURules = surface.uParams.numRules;

    GET_AND_INCREMENT (surface.vParams.order);
    GET_AND_INCREMENT_BOOL (surface.vParams.closed);
    GET_AND_INCREMENT (surface.vParams.numPoles);
    GET_AND_INCREMENT (surface.vParams.numKnots);
    GET_AND_INCREMENT (surface.vParams.numRules);
    int numVRules = surface.vParams.numRules;

    surface.Allocate ();
    if (numURules > 0)
        surface.uParams.numRules = numURules;
    if (numVRules > 0)
        surface.vParams.numRules = numVRules;
    size_t      nPoles = surface.uParams.numPoles * surface.vParams.numPoles;
    size_t      poleSize = nPoles * sizeof (DPoint3d);

    memcpy (surface.poles, pData, poleSize);
    pData += poleSize;

    size_t      uKnotSize = bspknot_numberKnots (surface.uParams.numPoles, surface.uParams.order, surface.uParams.closed) * sizeof (double);

    if (0 != uKnotSize)
        {
        memcpy (surface.uKnots, pData, uKnotSize);
        pData += uKnotSize;
        }
        
    size_t      vKnotSize = bspknot_numberKnots (surface.vParams.numPoles, surface.vParams.order, surface.vParams.closed) * sizeof (double);

    if (0 != vKnotSize)
        {
        memcpy (surface.vKnots, pData, vKnotSize);
        pData += vKnotSize;
        }

    if (surface.rational)
        {
        size_t      weightSize = nPoles * sizeof (double);

        memcpy (surface.weights, pData, weightSize);
        pData += weightSize;
        }

    GET_AND_INCREMENT_BOOL (surface.holeOrigin);

    if (getBoundaries)                  // NEEDS_WORK.... Memory management.
        {
        Int32       numBounds;

        GET_AND_INCREMENT (numBounds);

        if (0 != numBounds)
            {
            for (int i=0; i<numBounds; i++)
                {
                Int32       numPoints;
                GET_AND_INCREMENT (numPoints);

                size_t              boundaryPointSize = numPoints * sizeof (DPoint2d);
                bvector <DPoint2d>  boundaryPoints (numPoints);

                memcpy (&boundaryPoints.front(), pData, boundaryPointSize);
                pData += boundaryPointSize;

                surface.AddTrimBoundary (boundaryPoints);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override
    {
    bool    meshFromBSurf = 0 != (context.m_options & XGRAPHIC_OptimizeOptions_MeshFromBilinearBSurf);

    if (!meshFromBSurf)
        return ERROR;

    MSBsplineSurfacePtr     surface = MSBsplineSurface::CreatePtr();
    byte*                   opStart = pData;

    Get (*surface, pData, true);

    if (!surface->IsPlanarBilinear(1.0e-5))
        return ERROR;

    IFacetOptionsPtr            options = IFacetOptions::New ();
    SetXGraphicsFacetOptions (*options);

    IPolyfaceConstructionPtr    builder = IPolyfaceConstruction::New (*options);

    builder->Add (*surface);

    optimizedData.BeginDraw ();
    XGraphicsWriter (&optimizedData).WritePolyface (builder->GetClientMeshR(), true);
    pData = opStart + opSize - sizeof (opSize);

    return SUCCESS;
    }


}; // XGraphicsDrawBSplineSurface

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct   XGraphicsAddIndexPolys : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) 
    {
    //  #ifdef NEEDS_WORK_TopazMerge_ - printf/tools...printf
    Int32 polySize, numIndices;
    UInt32 nValues, nVerts, nVertIndices;
    int const* vertIndex;
    int const* normIndex;
    int const* uvColorIndex;
    DPoint3dCP verts;
    
    GET_AND_INCREMENT (polySize);
    GET_AND_INCREMENT (numIndices);

    printf ("AddIndexPoly, Polysize: %d, NumIndices: %d\n", polySize, numIndices);
    if (0 == numIndices)
        return;

    XGScopedArray   vertIndexBuffer, normIndexBuffer, uvColorIndexBuffer, vertsBuffer;
    GetPointer ((void **) &vertIndex, nVertIndices, sizeof (int), (byte const**) &pData, vertIndexBuffer);
    GetPointer ((void **) &normIndex, nValues, sizeof (int), (byte const**) &pData, normIndexBuffer);
    GetPointer ((void **) &uvColorIndex, nValues, sizeof (int),  (byte const**) &pData, uvColorIndexBuffer);
    GetPointer ((void **) &verts, nVerts, sizeof (DPoint3d), (byte const**) &pData, vertsBuffer);
    
#ifdef DUMP_VERBOSE
    printf ("\n\tIndex Count: %d", nVertIndices);
    UInt32 ii_ind=0;
    while (ii_ind < nVertIndices)
        {
        if (0 == ii_ind % polySize)
            printf ("\n\tPoly: %d, Vertex Indices: ", (int) (ii_ind/polySize) + 1);
        printf ("%d ", vertIndex[ii_ind]);
        ii_ind++;
        }
        
    printf ("\n\tVertex Count: %d\n", nVerts);
    for (UInt32 ii=0; ii<nVerts; ii++)
        {
        printf ("\tIndex %d : %f, %f, %f\n", ii + 1, verts[ii].x, verts[ii].y, verts[ii].z);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    Int32       polySize;
    int*        vertIndex;
    int*        normIndex;
    int*        uvColorIndex;
    DPoint3dP   verts;
    DVec3dP     normals;
    DPoint2d*   params;
    FloatRgb*   colors;
    int *       illuminatedColorIndex;
    WChar *   illuminatedTextureName;
    UInt32      pointCount, normalCount, pointIndexCount, paramCount, colorCount, normalIndexCount, paramIndexCount, colorIndexCount, textureNameCharCount;
    XGScopedArray   vertIndexBuffer, normIndexBuffer, uvColorIndexBuffer, vertsBuffer, normalsBuffer, paramsBuffer, colorsBuffer, illuminatedColorIndexBuffer, illuminatedTextureNameBuffer;

    GET_AND_INCREMENT (polySize);
    GET_AND_INCREMENT (pointIndexCount);

    if (0 == pointIndexCount)
        {
        LOG.error (L"AddIndexPolys with no indices.");
        return ERROR;
        }

    GetPointer ((void **) &vertIndex, pointIndexCount, sizeof (int), &pData, vertIndexBuffer);
    GetPointer ((void **) &normIndex, normalIndexCount, sizeof (int), &pData, normIndexBuffer);
    GetPointer ((void **) &uvColorIndex, paramIndexCount, sizeof (int), &pData, uvColorIndexBuffer);
    GetPointer ((void **) &verts, pointCount, sizeof (DPoint3d), &pData, vertsBuffer);
    GetPointer ((void **) &normals, normalCount, sizeof (DPoint3d), &pData, normalsBuffer);
    GetPointer ((void **) &params, paramCount, sizeof (DPoint2d), &pData, paramsBuffer);
    GetPointer ((void **) &colors, colorCount, 3 * sizeof (float), &pData, colorsBuffer);
    GetPointer ((void **) &illuminatedColorIndex, colorIndexCount, sizeof (int), &pData, illuminatedColorIndexBuffer);
    GetPointer ((void **) &illuminatedTextureName, textureNameCharCount, sizeof (WChar), &pData, illuminatedTextureNameBuffer);

    PolyfaceQueryCarrier meshData
            (
            polySize, true, pointIndexCount,
            pointCount, verts, vertIndex,
            normalCount, normals, normIndex,
            paramCount,  params, params ? uvColorIndex : NULL,
            colorCount, colors ? uvColorIndex : NULL, colors, NULL, NULL, NULL,
            illuminatedTextureName
            );

    // Define texture for illuminated mesh...
    meshData.SetTextureId (context.GetIViewDraw().DefineQVTexture (illuminatedTextureName, &context.GetDgnProject ()));
    context.GetIDrawGeom().DrawPolyface (meshData);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/13
//---------------------------------------------------------------------------------------
static PolyfaceHeaderPtr Get (byte const* pData, UInt32 dataSize)
    {
    Int32       polySize;
    int*        vertIndex;
    int*        normIndex;
    int*        uvColorIndex;
    DPoint3dP   verts;
    DVec3dP     normals;
    DPoint2d*   params;
    FloatRgb*   colors;
    int *       illuminatedColorIndex;
    WChar *   illuminatedTextureName;
    UInt32      pointCount, normalCount, pointIndexCount, paramCount, colorCount, normalIndexCount, paramIndexCount, colorIndexCount, textureNameCharCount;
    XGScopedArray   vertIndexBuffer, normIndexBuffer, uvColorIndexBuffer, vertsBuffer, normalsBuffer, paramsBuffer, colorsBuffer, illuminatedColorIndexBuffer, illuminatedTextureNameBuffer;

    GET_AND_INCREMENT (polySize);
    GET_AND_INCREMENT (pointIndexCount);

    if (0 == pointIndexCount)
        return NULL;

    GetPointer ((void **) &vertIndex, pointIndexCount, sizeof (int), &pData, vertIndexBuffer);
    GetPointer ((void **) &normIndex, normalIndexCount, sizeof (int), &pData, normIndexBuffer);
    GetPointer ((void **) &uvColorIndex, paramIndexCount, sizeof (int), &pData, uvColorIndexBuffer);
    GetPointer ((void **) &verts, pointCount, sizeof (DPoint3d), &pData, vertsBuffer);
    GetPointer ((void **) &normals, normalCount, sizeof (DPoint3d), &pData, normalsBuffer);
    GetPointer ((void **) &params, paramCount, sizeof (DPoint2d), &pData, paramsBuffer);
    GetPointer ((void **) &colors, colorCount, 3 * sizeof (float), &pData, colorsBuffer);
    GetPointer ((void **) &illuminatedColorIndex, colorIndexCount, sizeof (int), &pData, illuminatedColorIndexBuffer);
    GetPointer ((void **) &illuminatedTextureName, textureNameCharCount, sizeof (WChar), &pData, illuminatedTextureNameBuffer);

    PolyfaceQueryCarrier meshData
            (
            polySize, true, pointIndexCount,
            pointCount, verts, vertIndex,
            normalCount, normals, normIndex,
            paramCount,  params, params ? uvColorIndex : NULL,
            colorCount, colors ? uvColorIndex : NULL, colors, NULL, NULL, NULL,
            illuminatedTextureName
            );

    PolyfaceHeaderPtr result = PolyfaceHeader::CreateVariableSizeIndexed();
    result->CopyFrom (meshData);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    Int32       polySize, numIndices;
    UInt32      nValues, nVerts, nNorms;
    int const*  vertIndex;
    int const*  normIndex;
    int const*  uvColorIndex;
    DPoint3d*   verts;
    DVec3d*     norms;
    byte*       pData = &data[0];
    XGScopedArray   vertIndexBuffer, normIndexBuffer, uvColorIndexBuffer, vertsBuffer;
    XGScopedArrayRW normalsBuffer;

    GET_AND_INCREMENT (polySize);
    GET_AND_INCREMENT (numIndices);
    GetPointer ((void **) &vertIndex, nValues, sizeof (int), (byte const**) &pData, vertIndexBuffer);
    GetPointer ((void **) &normIndex, nValues, sizeof (int), (byte const**) &pData, normIndexBuffer);
    GetPointer ((void **) &uvColorIndex, nValues, sizeof (int),  (byte const**) &pData, uvColorIndexBuffer);
    GetPointer ((void **) &verts, nVerts, sizeof (DPoint3d), (byte const**) &pData, vertsBuffer);
    GetPointer ((void **) &norms, nNorms, sizeof (DPoint3d), (byte**) &pData, normalsBuffer);

    Transform const*    pTransform = transform.GetTransform();

    pTransform->multiply (verts, nVerts);

    for (UInt32 i=0; i<nNorms; i++)
        {
        pTransform->multiplyMatrixOnly (&norms[i]);
        norms[i].normalize ();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    Int32       polySize, numIndices;
    UInt32      nValues, nVerts;
    int const*  vertIndex;
    int const*  normIndex;
    int const*  uvColorIndex;
    DPoint3dCP  verts;
    XGScopedArray   vertIndexBuffer, normIndexBuffer, uvColorIndexBuffer, vertsBuffer;

    GET_AND_INCREMENT (polySize);
    GET_AND_INCREMENT (numIndices);
    GetPointer ((void **) &vertIndex, nValues, sizeof (int), (byte const**) &pData, vertIndexBuffer);
    GetPointer ((void **) &normIndex, nValues, sizeof (int), (byte const**) &pData, normIndexBuffer);
    GetPointer ((void **) &uvColorIndex, nValues, sizeof (int),  (byte const**) &pData, uvColorIndexBuffer);
    GetPointer ((void **) &verts, nVerts, sizeof (DPoint3d), (byte const**) &pData, vertsBuffer);

    return getBasisTransformFromPoints (transform, verts, NULL, nVerts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    Int32       polySize, numIndices;
    Int32       rhsPolySize, rhsNumIndices;
    UInt32      nVerts, rhsNVerts;
    DPoint3dP   verts, rhsVerts;
    XGScopedArray   vertsBuffer, rhsVertsBuffer, normalsBuffer, rhsNormalsBuffer;

    GET_AND_INCREMENT_DATA (polySize, data);
    GET_AND_INCREMENT_DATA (numIndices, data);

    GET_AND_INCREMENT_DATA (rhsPolySize, rhsData);
    GET_AND_INCREMENT_DATA (rhsNumIndices, rhsData);

    if (polySize != rhsPolySize || numIndices != rhsNumIndices)
        return false;

    // We don't actually test the indices. - The triangulation
    // at different rotations can produce different indices.
    // We'll just assume that testing the actual points is sufficient.
    TestValuesEqual (sizeof (int), data, rhsData);              // vertex indices
    TestValuesEqual (sizeof (int), data, rhsData);              // normal indices
    TestValuesEqual (sizeof (int), data, rhsData);              // uv Indices

    GetPointer ((void **) &verts, nVerts, sizeof (DPoint3d), &data, vertsBuffer);
    GetPointer ((void **) &rhsVerts, rhsNVerts, sizeof (DPoint3d), &rhsData, rhsVertsBuffer);

    if (!arePointsEqual (verts, nVerts, rhsVerts, rhsNVerts, distanceTolerance))
        return false;

    DPoint3dP   normals, rhsNormals;
    UInt32      nNormals, rhsNNormals;

    GetPointer ((void **) &normals, nNormals, sizeof (DPoint3d), &data, normalsBuffer);
    GetPointer ((void **) &rhsNormals, rhsNNormals, sizeof (DPoint3d), &rhsData, rhsNormalsBuffer);

    if (!arePointsEqual (normals, nNormals, rhsNormals, rhsNNormals, s_directionTolerance))
        return false;

    return TestValuesEqual (sizeof (DPoint2d),  data, rhsData) &&
           TestValuesEqual (sizeof (float),     data, rhsData) &&
           TestValuesEqual (sizeof (int),       data, rhsData) &&
           TestValuesEqual (sizeof (WChar),   data, rhsData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetPointer (void** ppPointer, UInt32& nValues, int dataSize, byte const** ppData, XGScopedArray& buffer)
    {
    byte const* pData = *ppData;

    GET_AND_INCREMENT (nValues);

    if (0 == nValues)
        {
        *ppPointer = NULL;
        *ppData = pData;
        }
    else
        {
        size_t dataSizeInBytes = (nValues * dataSize);
        *ppPointer = (void*)buffer.Acquire (pData, dataSizeInBytes);
        *ppData = pData + dataSizeInBytes;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2014
//---------------------------------------------------------------------------------------
static void GetPointer (void** ppPointer, UInt32& nValues, int dataSize, byte** ppData, XGScopedArrayRW& buffer)
    {
    byte* pData = *ppData;

    GET_AND_INCREMENT (nValues);

    if (0 == nValues)
        {
        *ppPointer = NULL;
        *ppData = pData;
        }
    else
        {
        size_t dataSizeInBytes = (nValues * dataSize);
        *ppPointer = (void*)buffer.AcquireRW (pData, dataSizeInBytes);
        *ppData = pData + dataSizeInBytes;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestValuesEqual (Int32 valueSize, byte const*& data, byte const*& rhsData)
    {
    UInt32      nValues, rhsNValues;

    GET_AND_INCREMENT_DATA (nValues, data);
    GET_AND_INCREMENT_DATA (rhsNValues, rhsData);

    if (nValues != rhsNValues)
        return false;

    if (0 == nValues)
        return true;

    UInt32      dataSize = nValues * valueSize;
    bool        isEqual  = 0 == memcmp (data, rhsData, dataSize);

    data    += dataSize;
    rhsData += dataSize;

    return isEqual;
    }

}; // XGraphicsAddIndexPolys

/*=================================================================================**//**
* @bsiclass                                                     Jeff.Marker     02/09
+===============+===============+===============+===============+===============+======*/
struct DgnPlatform::PersistableTextStringHelper
    {
    /* File Format, Version 1.0

        For minor upgrades, the version number can be bumped, new bits from PersistableDataIndicators can be used,
            and the new data can be written into the optional section (provided that StringOffset is correct).
            In implementation, the version number is more superficial than useful, especially as existing/old versions
            of MicroStation will NOT reject if they encounter a newer version. If the structure must change more than
            this, a new opcode will have to be introduced.

            Also see comments in XGraphicsDrawTextString::OnTransform; depending how this is implemented in the future,
            this may introduce additional restrictions to what you can sneak into the structure in the future.

        -- Data that is always written. This data is always meaningful, and not worth attempting to compress out.

        UInt8                           Version
        UInt32                          StringOffset (offset, in bytes, into the structure for the string)
        UInt32                          NumBytesInString
        DPoint3d                        Origin
        double[4]                       Orientation (quaternion)
        DPoint2d                        FontSize
        UInt32                          Color
        UInt32                          Font
        PersistableTextStringFlags      Flags
        PersistableDataIndicators       DataIndicators

        -- Optional data as determined by DataIndicators.

        UInt32                          UnderlineColor
        Int32                           UnderlineStyle
        UInt32                          UnderlineWeight
        double                          UnderlineOffset

        UInt32                          OverlineColor
        Int32                           OverlineStyle
        UInt32                          OverlineWeight
        double                          OverlineOffset

        UInt32                          BackgroundFillColor
        UInt32                          BackgroundColor
        Int32                           BackgroundStyle
        UInt32                          BackgroundWeight
        DPoint2d                        BackgroundBorder

        UInt8 (CharacterSpacingType)    CharacterSpacingType
        double                          CharacterSpacing

        UInt32                          ShxBigFont
        double                          Slant
        Int32                           LineStyle
        DVec2d                          RunOffset

        UInt32                          SymbLineWeight
        double                          ZDepth

        -- Variable-sized NULL-terminated string always at end.

        WChar                           UnicodeString

    */

    /*=================================================================================**//**
    * @bsiclass                                                     Jeff.Marker     02/09
    +===============+===============+===============+===============+===============+======*/
private: 
    struct PersistableTextStringFlags // : UInt32
        {
        UInt32  m_backgroundStyleFlag       :1;
        UInt32  m_underline                 :1;
        UInt32  m_underlineStyleFlag        :1;
        UInt32  m_overline                  :1;
        UInt32  m_overlineStyleFlag         :1;
        UInt32  m_bold                      :1;
        UInt32  m_slantFlag                 :1;
        UInt32  m_characterSpacingFlag      :1;
        UInt32  m_subscript                 :1;
        UInt32  m_superscript               :1;
        UInt32  m_vertical                  :1;
        UInt32  m_unused1                   :1; // was m_hasCodePage
        UInt32  m_renderAlignEdge           :1;
        UInt32  m_isField                   :1;
        UInt32  m_is3d                      :1;
        UInt32  m_stackedFractionSection    :2;
        UInt32  m_stackedFractionType       :2;
        UInt32  m_useZDepth                 :1;
        UInt32  m_unused                    :12;

        }; // PersistableTextStringFlags

    /*=================================================================================**//**
    * @bsiclass                                                     Jeff.Marker     02/09
    +===============+===============+===============+===============+===============+======*/
    enum PersistableDataIndicators ENUM_UNDERLYING_TYPE (UInt32)
        {
        DataIndicator_UnderlineColor        = 1 << 0,
        DataIndicator_UnderlineStyle        = 1 << 1,
        DataIndicator_UnderlineWeight       = 1 << 2,
        DataIndicator_UnderlineOffset       = 1 << 3,

        DataIndicator_OverlineColor         = 1 << 4,
        DataIndicator_OverlineStyle         = 1 << 5,
        DataIndicator_OverlineWeight        = 1 << 6,
        DataIndicator_OverlineOffset        = 1 << 7,

        DataIndicator_BackgroundFillColor   = 1 << 8,
        DataIndicator_BackgroundColor       = 1 << 9,
        DataIndicator_BackgroundStyle       = 1 << 10,
        DataIndicator_BackgroundWeight      = 1 << 11,
        DataIndicator_BackgroundBorder      = 1 << 12,

        DataIndicator_CharacterSpacingType  = 1 << 13,
        DataIndicator_CharacterSpacing      = 1 << 14,

        DataIndicator_ShxBigFont            = 1 << 15,
        DataIndicator_Slant                 = 1 << 16,
        DataIndicator_Unused1               = 1 << 17, // Was line style ID in 08.11
        DataIndicator_RunOffset             = 1 << 18,

        DataIndicator_SymbLineWeight        = 1 << 19,
        DataIndicator_ZDepth                = 1 << 20
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static size_t ComputeRequiredBufferSize (TextStringCR textString)
        {
        // Compute the upper bound (e.g. as if we needed to write every piece of data).

        return    2 * sizeof (UInt8)    // Version, CharacterSpacingType
                + sizeof (DPoint3d)     // Origin
                + 9 * sizeof (double)   // Orientation[4], UnderlineOffset, OverlineOffset, CharacterSpacing, Slant, ZDepth
                + 3 * sizeof (DPoint2d) // FontSize, BackgroundBorder, RunOffset
                + 17 * sizeof (UInt32)  // StringOffset, Color, Font, NumBytesInString, UnderlineColor, UnderlineStyle, UnderlineWeight, OverlineColor, OverlineStyle, OverlineWeight
                                        //  BackgroundFillColor, BackgroundColor, BackgroundStyle, BackgroundWeight, ShxBigFont, LineStyle, SymbLineWeight,
                + sizeof (PersistableTextStringFlags)
                + sizeof (PersistableDataIndicators)
                + sizeof (WChar) * (textString.GetNumFontChars () + 1);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    private: static void SetIndicator (PersistableDataIndicators& dataIndicators, PersistableDataIndicators indicatorToSet)
        {
        dataIndicators = (PersistableDataIndicators)(dataIndicators | indicatorToSet);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    private: static bool IsIndicated (PersistableDataIndicators dataIndicators, PersistableDataIndicators indicatorToTest)
        {
        return (indicatorToTest == (dataIndicators & indicatorToTest));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static size_t AppendToBuffer (byte* pData, TextStringCR textString, double const * zDepth, DgnProjectR dgnProject)
        {
        // Some local utility variables.
                byte*                       bufferStartAddr         = pData;
                TextStringPropertiesCR      props                   = textString.m_props;
                WString                     unicodeString           = textString.GetString ();
                byte*                       stringOffsetAddr        = NULL;
                byte*                       dataIndicatorsAddr      = NULL;

        if (unicodeString.length() == 0)
            {
            BeDataAssert(false);
            return 0;
            }

        // Platform-independent string encoding.
        Utf16Buffer utf16String;
        BeStringUtilities::WCharToUtf16(utf16String, unicodeString.c_str());
        
        // Set aside strongly typed variables for use with SET_AND_INCREMENT.
        const   UInt8                       version                 = 1;
                UInt32                      stringOffset            = 0;
                UInt32                      numBytesInString        = (UInt32)(sizeof (Utf16Char) * utf16String.size ());
                DPoint4d                    orientationQuat;        textString.m_rMatrix.getQuaternion (&orientationQuat, false);
                PersistableTextStringFlags  flags;                  memset (&flags, 0, sizeof (flags));
                PersistableDataIndicators   dataIndicators;         memset (&dataIndicators, 0, sizeof (dataIndicators));
                UInt32                      fontNumber              = 0;
                UInt32                      shxBigFontNumber        = 0;
                UInt8                       characterSpacingType    = (UInt8)props.m_characterSpacingType;

        // Configure the flags structure from the TextStringProperties.
        flags.m_backgroundStyleFlag     = props.m_shouldUseBackground;
        flags.m_underline               = props.m_isUnderlined;
        flags.m_underlineStyleFlag      = props.m_shouldUseUnderlineStyle;
        flags.m_overline                = props.m_isOverlined;
        flags.m_overlineStyleFlag       = props.m_shouldUseOverlineStyle;
        flags.m_bold                    = props.m_isBold;
        flags.m_slantFlag               = props.m_isItalic;
        flags.m_characterSpacingFlag    = (0.0 != props.m_characterSpacingValue);
        flags.m_subscript               = props.m_isSubScript;
        flags.m_superscript             = props.m_isSuperScript;
        flags.m_vertical                = props.IsVertical ();
        flags.m_unused1                 = false;
        flags.m_renderAlignEdge         = props.m_shouldIgnoreLSB;
        flags.m_isField                 = props.m_isPartOfField;
        flags.m_is3d                    = props.m_is3d;
        flags.m_stackedFractionSection  = static_cast<UInt32>(props.m_stackedFractionSection);
        flags.m_stackedFractionType     = static_cast<UInt32>(props.m_stackedFractionType);

        // Resolve the font numbers.
        // I will pass true to GetFontNumber (to create if not found) because a handler could in theory contsruct a TextString for its XGraphics by purely using Font objects, hence the font it uses is not strictly guaranteed to exist in the file yet.
        if (NULL == props.m_font)
            BeAssert (false);
        else if (SUCCESS != dgnProject.Fonts().AcquireFontNumber (fontNumber, *props.m_font))
            LOG.warningv (
                L"%hs %d - PersistableTextStringHelper::AppendToBuffer cannot acqure a font number for font '%hs' (%u). Text = '%ls'.",
                __FILE__,
                __LINE__,
                props.m_font->GetName().c_str(),
                props.m_font->GetType(),
                textString.GetString().c_str()
                );

        if (NULL != props.m_shxBigFont)
            dgnProject.Fonts().AcquireFontNumber (shxBigFontNumber, *props.m_shxBigFont);

        // Append required data to the stream.
        SET_AND_INCREMENT (version)

        // We'll compute this later, and will need to sneak it back in.
        stringOffsetAddr = pData;
        pData += sizeof (stringOffset);

        SET_AND_INCREMENT (numBytesInString)
        SET_AND_INCREMENT (textString.m_lowerLeft)
        SET_AND_INCREMENT (orientationQuat)
        SET_AND_INCREMENT (props.m_fontSize)
        SET_AND_INCREMENT (props.m_color)
        SET_AND_INCREMENT (fontNumber)
        SET_AND_INCREMENT (flags)

        // We'll compute this as we go, and will need to sneak it back in.
        dataIndicatorsAddr = pData;
        pData += sizeof (dataIndicators);

        // Write the optional data and configure dataIndicators as we go.
        // Note that we can be somewhat conservative here since we can abandom data we'll never use (since we strictly control the other end's TextString's purpose and lifetime).
        //  This includes style information for things like underline if underline is not turned on; elements would still have to write this in case underline was turned on
        //  in the future, but we don't have to handle things like that.

        bool encodeUnderlineValues      = (props.m_isUnderlined && props.m_shouldUseUnderlineStyle);
        bool encodeOverlineValues       = (props.m_isOverlined && props.m_shouldUseOverlineStyle);
        bool encodeBackgroundValues     = props.m_shouldUseBackground;
        bool encodeBackgroundBorder     = (0.0 != props.m_backgroundBorderPadding.x || 0.0 != props.m_backgroundBorderPadding.y);
        bool encodeCharSpacingValues    = (0.0 != props.m_characterSpacingValue);
        bool encodeSlantValues          = props.m_isItalic;
        bool encodeRunOffsetValues      = (0.0 != props.m_runOffset.x || 0.0 != props.m_runOffset.y);

        if (encodeUnderlineValues && 0 != props.m_underlineColor)               { SetIndicator (dataIndicators, DataIndicator_UnderlineColor);          SET_AND_INCREMENT (props.m_underlineColor)              }
        if (encodeUnderlineValues && 0 != props.m_underlineLineStyle)           { SetIndicator (dataIndicators, DataIndicator_UnderlineStyle);          SET_AND_INCREMENT (props.m_underlineLineStyle)          }
        if (encodeUnderlineValues && 0 != props.m_underlineWeight)              { SetIndicator (dataIndicators, DataIndicator_UnderlineWeight);         SET_AND_INCREMENT (props.m_underlineWeight)             }
        if (encodeUnderlineValues && 0.0 != props.m_underlineOffset)            { SetIndicator (dataIndicators, DataIndicator_UnderlineOffset);         SET_AND_INCREMENT (props.m_underlineOffset)             }

        if (encodeOverlineValues && 0 != props.m_overlineColor)                 { SetIndicator (dataIndicators, DataIndicator_OverlineColor);           SET_AND_INCREMENT (props.m_overlineColor)               }
        if (encodeOverlineValues && 0 != props.m_overlineLineStyle)             { SetIndicator (dataIndicators, DataIndicator_OverlineStyle);           SET_AND_INCREMENT (props.m_overlineLineStyle)           }
        if (encodeOverlineValues && 0 != props.m_overlineWeight)                { SetIndicator (dataIndicators, DataIndicator_OverlineWeight);          SET_AND_INCREMENT (props.m_overlineWeight)              }
        if (encodeOverlineValues && 0.0 != props.m_overlineOffset)              { SetIndicator (dataIndicators, DataIndicator_OverlineOffset);          SET_AND_INCREMENT (props.m_overlineOffset)              }

        if (encodeBackgroundValues && 0 != props.m_backgroundFillColor)         { SetIndicator (dataIndicators, DataIndicator_BackgroundFillColor);     SET_AND_INCREMENT (props.m_backgroundFillColor)         }
        if (encodeBackgroundValues && 0 != props.m_backgroundBorderColor)       { SetIndicator (dataIndicators, DataIndicator_BackgroundColor);         SET_AND_INCREMENT (props.m_backgroundBorderColor)       }
        if (encodeBackgroundValues && 0 != props.m_backgroundBorderLineStyle)   { SetIndicator (dataIndicators, DataIndicator_BackgroundStyle);         SET_AND_INCREMENT (props.m_backgroundBorderLineStyle)   }
        if (encodeBackgroundValues && 0 != props.m_backgroundBorderWeight)      { SetIndicator (dataIndicators, DataIndicator_BackgroundWeight);        SET_AND_INCREMENT (props.m_backgroundBorderWeight)      }
        if (encodeBackgroundValues && encodeBackgroundBorder)                   { SetIndicator (dataIndicators, DataIndicator_BackgroundBorder);        SET_AND_INCREMENT (props.m_backgroundBorderPadding)     }

        if (encodeCharSpacingValues && CharacterSpacingType::Absolute != props.m_characterSpacingType)       { SetIndicator (dataIndicators, DataIndicator_CharacterSpacingType);    SET_AND_INCREMENT (characterSpacingType)                }
        if (encodeCharSpacingValues && 0.0 != (int)props.m_characterSpacingValue){SetIndicator (dataIndicators, DataIndicator_CharacterSpacing);        SET_AND_INCREMENT (props.m_characterSpacingValue)       }

        if (0 != shxBigFontNumber)                                              { SetIndicator (dataIndicators, DataIndicator_ShxBigFont);              SET_AND_INCREMENT (shxBigFontNumber)                    }
        if (encodeSlantValues && 0.0 != props.m_customSlantAngle)               { SetIndicator (dataIndicators, DataIndicator_Slant);                   SET_AND_INCREMENT (props.m_customSlantAngle)            }
        if (encodeRunOffsetValues)                                              { SetIndicator (dataIndicators, DataIndicator_RunOffset);               SET_AND_INCREMENT (props.m_runOffset)                   }

        if (0 != textString.m_lineWeight)                                       { SetIndicator (dataIndicators, DataIndicator_SymbLineWeight);          SET_AND_INCREMENT (textString.m_lineWeight)             }
        if (NULL != zDepth)                                                     { SetIndicator (dataIndicators, DataIndicator_ZDepth);                  SET_AND_INCREMENT (*zDepth)                             }

        // Go back and write the offset and indicator data now that we have enough information.
        stringOffset = static_cast<UInt32>(pData - bufferStartAddr);
        *((UInt32*)stringOffsetAddr) = stringOffset;

        *((PersistableDataIndicators*)dataIndicatorsAddr) = dataIndicators;

        // Write the string.
        memcpy (pData, &utf16String[0], numBytesInString);
        pData += numBytesInString;

        // Return the number of bytes actually written.
        return (pData - bufferStartAddr);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static void ExtractTextString (TextStringR textString, double& zDepth, bool& useZDepth, byte const * pData, DgnProjectR project)
        {
        // Some local utility variables.
        TextStringProperties        props;

        // Set aside strongly typed variables for use with GET_AND_INCREMENT.
        byte const *                bufferStartAddr         = pData;
        UInt8                       version                 = 0;
        UInt32                      stringOffset            = 0;
        DPoint3d                    origin;                 origin.zero ();
        DPoint4d                    orientationQuat         = { 0.0, 0.0, 0.0, 0.0 };
        RotMatrix                   orientation;            orientation.initIdentity ();
        UInt32                      numBytesInString        = 0;
        UInt32                      fontNumber              = 0;
        UInt32                      shxBigFontNumber        = 0;
        UInt32                      symbLineWeight          = 0;
        PersistableTextStringFlags  flags;                  memset (&flags, 0, sizeof (flags));
        PersistableDataIndicators   dataIndicators;         memset (&dataIndicators, 0, sizeof (dataIndicators));
        UInt8                       characterSpacingType    = 0;

        // Extract from the stream.
        GET_AND_INCREMENT (version);
        GET_AND_INCREMENT (stringOffset);
        GET_AND_INCREMENT (numBytesInString);
        GET_AND_INCREMENT (origin);

        GET_AND_INCREMENT (orientationQuat);
        orientation.initFromQuaternion (&orientationQuat);

        GET_AND_INCREMENT (props.m_fontSize);
        GET_AND_INCREMENT (props.m_color);

        GET_AND_INCREMENT (fontNumber);
        props.m_font = DgnFontManager::ResolveFont (fontNumber, project, DGNFONTVARIANT_DontCare);

        GET_AND_INCREMENT (flags);

        props.m_shouldUseBackground     = flags.m_backgroundStyleFlag;
        props.m_isUnderlined            = flags.m_underline;
        props.m_shouldUseUnderlineStyle = flags.m_underlineStyleFlag;
        props.m_isOverlined             = flags.m_overline;
        props.m_shouldUseOverlineStyle  = flags.m_overlineStyleFlag;
        props.m_isBold                  = flags.m_bold;
        props.m_isItalic                = flags.m_slantFlag;
        props.m_isSubScript             = flags.m_subscript;
        props.m_isSuperScript           = flags.m_superscript;
        props.SetIsVertical (flags.m_vertical);
        props.m_shouldIgnoreLSB         = flags.m_renderAlignEdge;
        props.m_isPartOfField           = flags.m_isField;
        props.m_is3d                    = flags.m_is3d;
        props.m_stackedFractionSection  = (StackedFractionSection)flags.m_stackedFractionSection;
        props.m_stackedFractionType     = (StackedFractionType)flags.m_stackedFractionType;

        GET_AND_INCREMENT (dataIndicators);

        if (IsIndicated (dataIndicators, DataIndicator_UnderlineColor))         { GET_AND_INCREMENT (props.m_underlineColor)            }
        if (IsIndicated (dataIndicators, DataIndicator_UnderlineStyle))         { GET_AND_INCREMENT (props.m_underlineLineStyle)        }
        if (IsIndicated (dataIndicators, DataIndicator_UnderlineWeight))        { GET_AND_INCREMENT (props.m_underlineWeight)           }
        if (IsIndicated (dataIndicators, DataIndicator_UnderlineOffset))        { GET_AND_INCREMENT (props.m_underlineOffset)           }
        if (IsIndicated (dataIndicators, DataIndicator_OverlineColor))          { GET_AND_INCREMENT (props.m_overlineColor)             }
        if (IsIndicated (dataIndicators, DataIndicator_OverlineStyle))          { GET_AND_INCREMENT (props.m_overlineLineStyle)         }
        if (IsIndicated (dataIndicators, DataIndicator_OverlineWeight))         { GET_AND_INCREMENT (props.m_overlineWeight)            }
        if (IsIndicated (dataIndicators, DataIndicator_OverlineOffset))         { GET_AND_INCREMENT (props.m_overlineOffset)            }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundFillColor))    { GET_AND_INCREMENT (props.m_backgroundFillColor)       }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundColor))        { GET_AND_INCREMENT (props.m_backgroundBorderColor)     }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundStyle))        { GET_AND_INCREMENT (props.m_backgroundBorderLineStyle) }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundWeight))       { GET_AND_INCREMENT (props.m_backgroundBorderWeight)    }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundBorder))       { GET_AND_INCREMENT (props.m_backgroundBorderPadding)   }
        if (IsIndicated (dataIndicators, DataIndicator_CharacterSpacingType))   { GET_AND_INCREMENT (characterSpacingType)              }
        if (IsIndicated (dataIndicators, DataIndicator_CharacterSpacing))       { GET_AND_INCREMENT (props.m_characterSpacingValue)     }
        if (IsIndicated (dataIndicators, DataIndicator_ShxBigFont))             { GET_AND_INCREMENT (shxBigFontNumber)                  }
        if (IsIndicated (dataIndicators, DataIndicator_Slant))                  { GET_AND_INCREMENT (props.m_customSlantAngle)          }
        if (IsIndicated (dataIndicators, DataIndicator_Unused1))                { pData += sizeof (Int32);                              }   // Was line style ID in 08.11
        if (IsIndicated (dataIndicators, DataIndicator_RunOffset))              { GET_AND_INCREMENT (props.m_runOffset)                 }
        if (IsIndicated (dataIndicators, DataIndicator_SymbLineWeight))         { GET_AND_INCREMENT (symbLineWeight)                    }

        if (!flags.m_characterSpacingFlag)
            {
            props.m_characterSpacingValue   = 0.0;
            props.m_characterSpacingType    = CharacterSpacingType::Absolute;
            }
        else
            {
            props.m_characterSpacingType = (CharacterSpacingType)characterSpacingType;
            }

        if (IsIndicated (dataIndicators, DataIndicator_ZDepth))
            {
            GET_AND_INCREMENT (zDepth);
            useZDepth = true;
            }
        else
            {
            useZDepth = false;
            }

        if (0 != shxBigFontNumber)
            props.m_shxBigFont = DgnFontManager::ResolveFont (shxBigFontNumber, project, DGNFONTVARIANT_ShxBig);

        // Reset pData to the string location, noting that future versions may add optional data we don't understand.
        pData = (bufferStartAddr + stringOffset);

        WString platformWideChar;
        BeStringUtilities::Utf16ToWChar(platformWideChar, (Utf16CP)pData);

        textString.Clear ();
        textString.Init (platformWideChar.c_str(), origin, orientation, props, 0, NULL, symbLineWeight);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static void ExtractTransformData (DPoint3dR origin, RotMatrixR orientation, DPoint2dR fontSize, byte const * pData)
        {
        DPoint4d orientationQuat;

        pData += sizeof (UInt8);    // Version
        pData += sizeof (UInt32);   // StringOffset
        pData += sizeof (UInt32);   // NumBytesInString

        GET_AND_INCREMENT (origin);
        GET_AND_INCREMENT (orientationQuat);
        GET_AND_INCREMENT (fontSize);

        orientation.initFromQuaternion (&orientationQuat);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TransformInPlace (TransformInfoCR transform, byte* pData)
        {
        pData += sizeof (UInt8);    // Version
        pData += sizeof (UInt32);   // StringOffset
        pData += sizeof (UInt32);   // NumBytesInString

        DPoint3dP   origin          = (DPoint3dP)pData;     pData += sizeof (DPoint3d);
        DPoint4dP   orientationQuat = (DPoint4dP)pData;     pData += sizeof (DPoint4d);
        DPoint2dP   fontSize        = (DPoint2dP)pData;     pData += sizeof (DPoint2d);

        transform.GetTransform ()->multiply (origin);

        RotMatrix   unScaledTFormMtx;
        DPoint2d    textScale;

        unScaledTFormMtx.initFromQuaternion (orientationQuat);

        TextString::TransformOrientationAndGetScale (textScale, unScaledTFormMtx, NULL, *transform.GetTransform (), true);

        unScaledTFormMtx.getQuaternion (orientationQuat, false);

        fontSize->x *= textScale.x;
        fontSize->y *= textScale.y;

        pData += sizeof (UInt32);                       // Color
        pData += sizeof (UInt32);                       // Font
        pData += sizeof (PersistableTextStringFlags);   // Flags

        PersistableDataIndicators dataIndicators;
        GET_AND_INCREMENT (dataIndicators);

        if (IsIndicated (dataIndicators, DataIndicator_UnderlineColor))         { pData += sizeof (UInt32); }
        if (IsIndicated (dataIndicators, DataIndicator_UnderlineStyle))         { pData += sizeof (Int32);  }
        if (IsIndicated (dataIndicators, DataIndicator_UnderlineWeight))        { pData += sizeof (UInt32); }

        if (IsIndicated (dataIndicators, DataIndicator_UnderlineOffset))
            {
            double* underlineOffset = (double*)pData;
            *underlineOffset *= textScale.y;
            pData += sizeof (double);
            }

        if (IsIndicated (dataIndicators, DataIndicator_OverlineColor))          { pData += sizeof (UInt32); }
        if (IsIndicated (dataIndicators, DataIndicator_OverlineStyle))          { pData += sizeof (Int32);  }
        if (IsIndicated (dataIndicators, DataIndicator_OverlineWeight))         { pData += sizeof (UInt32); }

        if (IsIndicated (dataIndicators, DataIndicator_OverlineOffset))
            {
            double* overlineOffset = (double*)pData;
            *overlineOffset *= textScale.y;
            pData += sizeof (double);
            }

        if (IsIndicated (dataIndicators, DataIndicator_BackgroundFillColor))    { pData += sizeof (UInt32); }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundColor))        { pData += sizeof (UInt32); }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundStyle))        { pData += sizeof (Int32);  }
        if (IsIndicated (dataIndicators, DataIndicator_BackgroundWeight))       { pData += sizeof (UInt32); }

        if (IsIndicated (dataIndicators, DataIndicator_BackgroundBorder))
            {
            DPoint2dP backgroundBorder = (DPoint2dP)pData;
            backgroundBorder->x *= textScale.x;
            backgroundBorder->y *= textScale.y;
            pData += sizeof (DPoint2d);
            }

        if (IsIndicated (dataIndicators, DataIndicator_CharacterSpacingType))   { pData += sizeof (UInt8);  }

        if (IsIndicated (dataIndicators, DataIndicator_CharacterSpacing))
            {
            double* characterSpacing = (double*)pData;
            *characterSpacing *= textScale.x;
            pData += sizeof (double);
            }

        if (IsIndicated (dataIndicators, DataIndicator_ShxBigFont))             { pData += sizeof (UInt32); }
        if (IsIndicated (dataIndicators, DataIndicator_Slant))                  { pData += sizeof (double); }
        if (IsIndicated (dataIndicators, DataIndicator_Unused1))                { pData += sizeof (Int32);  }   // Was line style ID in 08.11

        if (IsIndicated (dataIndicators, DataIndicator_RunOffset))
            {
            DVec2dP runOffset = (DVec2dP)pData;
            runOffset->x *= textScale.x;
            runOffset->y *= textScale.y;
            pData += sizeof (DVec2d);
            }

        if (IsIndicated (dataIndicators, DataIndicator_SymbLineWeight))         { pData += sizeof (UInt32); }

        if (IsIndicated (dataIndicators, DataIndicator_ZDepth))
            {
            DPoint3d zcol;
            transform.GetTransform ()->getMatrixColumn (&zcol, 2);

            double  zScale  = zcol.magnitude ();
            double* zDepth  = (double*)pData;

            *zDepth *= zScale;

            pData += sizeof (double);
            }
        }
template <typename T>
static bool CheckAndAdvance (const byte *&lhsData, const byte *&rhsData)
    {
    if (*(T*)lhsData != *(T*)rhsData)
        return false;
    lhsData += sizeof (T);
    rhsData += sizeof (T);
    return true;
    }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     02/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static bool AreEqualWithinTolerance (byte const * lhsData, byte const * rhsData, double tolerance)
        {
#define CHECK_BASIC_DATA_TYPE(DATA_TYPE) \
if (*(DATA_TYPE const *)lhsData != *(DATA_TYPE const *)rhsData)\
return false;\
lhsData += sizeof (DATA_TYPE);\
rhsData += sizeof (DATA_TYPE);

        byte const * lhsDataStart = lhsData;
        byte const * rhsDataStart = rhsData;

        CHECK_BASIC_DATA_TYPE (UInt8)   // Version

        UInt32 lhsStringOffset = *(UInt32*)lhsData;
        UInt32 rhsStringOffset = *(UInt32*)rhsData;

        if (lhsStringOffset != rhsStringOffset)
            return false;

        lhsData += sizeof (UInt32);
        rhsData += sizeof (UInt32);

        UInt32 lhsNumBytesInString = *(UInt32*)lhsData;
        UInt32 rhsNumBytesInString = *(UInt32*)rhsData;

        if (lhsNumBytesInString != rhsNumBytesInString)
            return false;

        lhsData += sizeof (UInt32);
        rhsData += sizeof (UInt32);

        // Origin
        if (!arePointsEqual ((DPoint3dCP)lhsData, 1, (DPoint3dCP)rhsData, 1, tolerance))
            return false;

        lhsData += sizeof (DPoint3d);
        rhsData += sizeof (DPoint3d);

        // Orientation Quat
        DPoint4d lhsQuat;
        DPoint4d rhsQuat;

        lhsQuat.initFromArray ((double*)lhsData);
        rhsQuat.initFromArray ((double*)rhsData);

        if (!areQuaternionsEqual (lhsQuat, rhsQuat, tolerance))
            return false;

        lhsData += sizeof (DPoint4d);
        rhsData += sizeof (DPoint4d);

        // Scale
        DPoint2dCP lhsScale = (DPoint2dCP) lhsData;
        DPoint2dCP rhsScale = (DPoint2dCP) rhsData;

        lhsData += sizeof (DPoint2d);
        rhsData += sizeof (DPoint2d);

        RotMatrix lhsScaleMtx;
        RotMatrix rhsScaleMtx;

        lhsScaleMtx.initIdentity ();
        rhsScaleMtx.initIdentity ();

        lhsScaleMtx.scale (&lhsScaleMtx, lhsScale->x, lhsScale->y, 1.0, NULL);
        rhsScaleMtx.scale (&rhsScaleMtx, rhsScale->x, rhsScale->y, 1.0, NULL);

        if (!lhsScaleMtx.isEqual (&rhsScaleMtx, tolerance))
            return false;

        if (!CheckAndAdvance <UInt32> (lhsData, rhsData))   // Color
            return false;
        if (!CheckAndAdvance <UInt32> (lhsData, rhsData))   // Font
            return false;
        if (!CheckAndAdvance <UInt32> (lhsData, rhsData))   // PersistableTextStringFlags)
            return false;

//        CHECK_BASIC_DATA_TYPE (UInt32)  // Color
//        CHECK_BASIC_DATA_TYPE (UInt32)  // Font
//        CHECK_BASIC_DATA_TYPE (UInt32)  // Flags (PersistableTextStringFlags)

        PersistableDataIndicators const * lhsDataIndicators = (PersistableDataIndicators const *) lhsData;
        PersistableDataIndicators const * rhsDataIndicators = (PersistableDataIndicators const *) rhsData;

        if (*lhsDataIndicators != *rhsDataIndicators)
            return false;

        lhsData += sizeof (PersistableDataIndicators);
        rhsData += sizeof (PersistableDataIndicators);

        if (IsIndicated (*lhsDataIndicators, DataIndicator_UnderlineColor))     { CHECK_BASIC_DATA_TYPE (UInt32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_UnderlineStyle))     { CHECK_BASIC_DATA_TYPE (Int32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_UnderlineWeight))    { CHECK_BASIC_DATA_TYPE (UInt32) }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_UnderlineOffset))
            {
            if (!isEqual (*(double const *)lhsData, *(double const *)rhsData, tolerance))
                return false;

            lhsData += sizeof (double);
            rhsData += sizeof (double);
            }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_OverlineColor))  { CHECK_BASIC_DATA_TYPE (UInt32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_OverlineStyle))  { CHECK_BASIC_DATA_TYPE (Int32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_OverlineWeight)) { CHECK_BASIC_DATA_TYPE (UInt32) }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_OverlineOffset))
            {
            if (!isEqual (*(double const *)lhsData, *(double const *)rhsData, tolerance))
                return false;

            lhsData += sizeof (double);
            rhsData += sizeof (double);
            }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_BackgroundFillColor))    { CHECK_BASIC_DATA_TYPE (UInt32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_BackgroundColor))        { CHECK_BASIC_DATA_TYPE (UInt32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_BackgroundStyle))        { CHECK_BASIC_DATA_TYPE (Int32) }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_BackgroundWeight))       { CHECK_BASIC_DATA_TYPE (UInt32) }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_BackgroundBorder))
            {
            DPoint2dCP lhsBackgroundBorder = (DPoint2d const *)lhsData;
            DPoint2dCP rhsBackgroundBorder = (DPoint2d const *)rhsData;

            if (!lhsBackgroundBorder->isEqual (rhsBackgroundBorder, tolerance))
                return false;

            lhsData += sizeof (DPoint2d);
            rhsData += sizeof (DPoint2d);
            }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_CharacterSpacingType))   { CHECK_BASIC_DATA_TYPE (CharacterSpacingType) }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_CharacterSpacing))
            {
            if (!isEqual (*(double const *)lhsData, *(double const *)rhsData, tolerance))
                return false;

            lhsData += sizeof (double);
            rhsData += sizeof (double);
            }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_ShxBigFont))             { CHECK_BASIC_DATA_TYPE (UInt32)    }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_Slant))                  { CHECK_BASIC_DATA_TYPE (double)    }
        if (IsIndicated (*lhsDataIndicators, DataIndicator_Unused1))                { CHECK_BASIC_DATA_TYPE (Int32)     }   // Was line style ID in 08.11

        if (IsIndicated (*lhsDataIndicators, DataIndicator_RunOffset))
            {
            DVec2d lhsRunOffset = *(DVec2d const *)lhsData;
            DVec2d rhsRunOffset = *(DVec2d const *)rhsData;

            if (!lhsRunOffset.isEqual (&rhsRunOffset, tolerance))
                return false;

            lhsData += sizeof (DVec2d);
            rhsData += sizeof (DVec2d);
            }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_SymbLineWeight)) { CHECK_BASIC_DATA_TYPE (UInt32) }

        if (IsIndicated (*lhsDataIndicators, DataIndicator_ZDepth))
            {
            if (!isEqual (*(double const *)lhsData, *(double const *)rhsData, tolerance))
                return false;

            lhsData += sizeof (double);
            rhsData += sizeof (double);
            }

#undef CHECK_BASIC_DATA_TYPE

        if (0 != wcsncmp ((WCharCP)(lhsDataStart + lhsStringOffset), (WCharCP)(rhsDataStart + rhsStringOffset), (lhsNumBytesInString / sizeof (WChar))))
            return false;

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Jeff.Marker     08/10
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: static void ProcessProperties (byte* pData, UInt32 dataSize, PropertyContextR context)
        {
        pData += sizeof (UInt8);                        // Version
        pData += sizeof (UInt32);                       // StringOffset
        pData += sizeof (UInt32);                       // NumBytesInString
        pData += 3 * sizeof (double);                   // Origin
        pData += 4 * sizeof (double);                   // Orientation
        pData += 2 * sizeof (double);                   // TextSize

        if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
            {
            UInt32* pColor = (UInt32*) pData;
            context.DoColorCallback (pColor, EachColorArg (*pColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
            }

        pData += sizeof (UInt32);                       // Color

        if (0 != (ELEMENT_PROPERTY_Font & context.GetElementPropertiesMask ()))
            {
            UInt32* pFontNumber = (UInt32*) pData;
            context.DoFontCallback (pFontNumber, EachFontArg (*pFontNumber, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
            }

        pData += sizeof (UInt32);                       // Font

        pData += sizeof (PersistableTextStringFlags);   // Flags

        PersistableDataIndicators* dataIndicators = (PersistableDataIndicators*)pData;

        pData += sizeof (PersistableDataIndicators);    // DataIndicators

        if (IsIndicated (*dataIndicators, DataIndicator_UnderlineColor))
            {
            if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                {
                UInt32* pColor = (UInt32*) pData;
                context.DoColorCallback (pColor, EachColorArg (*pColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_UnderlineStyle))
            {
            if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                {
                Int32* pStyle = (Int32*) pData;
                context.DoLineStyleCallback (pStyle, EachLineStyleArg (*pStyle, NULL, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (Int32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_UnderlineWeight))
            {
            if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                {
                UInt32* pWeight = (UInt32*) pData;
                context.DoWeightCallback (pWeight, EachWeightArg (*pWeight, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_UnderlineOffset))   pData += sizeof (double);

        if (IsIndicated (*dataIndicators, DataIndicator_OverlineColor))
            {
            if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                {
                UInt32* pColor = (UInt32*) pData;
                context.DoColorCallback (pColor, EachColorArg (*pColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_OverlineStyle))
            {
            if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                {
                Int32* pStyle = (Int32*) pData;
                context.DoLineStyleCallback (pStyle, EachLineStyleArg (*pStyle, NULL, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (Int32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_OverlineWeight))
            {
            if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                {
                UInt32* pWeight = (UInt32*) pData;
                context.DoWeightCallback (pWeight, EachWeightArg (*pWeight, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_OverlineOffset))    pData += sizeof (double);

        if (IsIndicated (*dataIndicators, DataIndicator_BackgroundFillColor))
            {
            if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                {
                UInt32* pColor = (UInt32*) pData;
                context.DoColorCallback (pColor, EachColorArg (*pColor, PROPSCALLBACK_FLAGS_IsBackgroundID, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_BackgroundColor))
            {
            if (0 != (ELEMENT_PROPERTY_Color & context.GetElementPropertiesMask ()))
                {
                UInt32* pColor = (UInt32*) pData;
                context.DoColorCallback (pColor, EachColorArg (*pColor, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_BackgroundStyle))
            {
            if (0 != (ELEMENT_PROPERTY_Linestyle & context.GetElementPropertiesMask ()))
                {
                Int32* pStyle = (Int32*) pData;
                context.DoLineStyleCallback (pStyle, EachLineStyleArg (*pStyle, NULL, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (Int32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_OverlineWeight))
            {
            if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                {
                UInt32* pWeight = (UInt32*) pData;
                context.DoWeightCallback (pWeight, EachWeightArg (*pWeight, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_BackgroundBorder))      pData += sizeof (DPoint2d);
        if (IsIndicated (*dataIndicators, DataIndicator_CharacterSpacingType))  pData += sizeof (UInt8);
        if (IsIndicated (*dataIndicators, DataIndicator_CharacterSpacing))      pData += sizeof (double);

        if (IsIndicated (*dataIndicators, DataIndicator_ShxBigFont))
            {
            if (0 != (ELEMENT_PROPERTY_Font & context.GetElementPropertiesMask ()))
                {
                UInt32* pFontNumber = (UInt32*) pData;
                context.DoFontCallback (pFontNumber, EachFontArg (*pFontNumber, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        if (IsIndicated (*dataIndicators, DataIndicator_Slant))      pData += sizeof (double);
        if (IsIndicated (*dataIndicators, DataIndicator_Unused1))    pData += sizeof (Int32);
        if (IsIndicated (*dataIndicators, DataIndicator_RunOffset))  pData += sizeof (DVec2d);

        if (IsIndicated (*dataIndicators, DataIndicator_SymbLineWeight))
            {
            if (0 != (ELEMENT_PROPERTY_Weight & context.GetElementPropertiesMask ()))
                {
                UInt32* pWeight = (UInt32*) pData;
                context.DoWeightCallback (pWeight, EachWeightArg (*pWeight, PROPSCALLBACK_FLAGS_NoFlagsSet, context));
                }

            pData += sizeof (UInt32);
            }

        // No other pieces need to be announced.
        }

    }; // PersistableTextStringHelper

/*=================================================================================**//**
* @bsiclass                                                     Jeff.Marker     02/09
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsDrawTextString : XGraphicsOperation
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     02/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("DrawTextString\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     02/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    TextString      textString;
    double          zDepth;
    bool            useZDepth;

    PersistableTextStringHelper::ExtractTextString (textString, zDepth, useZDepth, pData, opContext.m_dgnProject);

    context.GetIDrawGeom().DrawTextString (textString, useZDepth ? &zDepth : NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetBasisTransform (TransformR transform, byte* pData, UInt32 dataSize, byte* pEnd) override
    {
    DPoint3d    origin;
    RotMatrix   orientation;
    DPoint2d    fontSize;

    PersistableTextStringHelper::ExtractTransformData (origin, orientation, fontSize, pData);

    orientation.scale (&orientation, fontSize.x, fontSize.y, 1.0, NULL);

    transform.initFrom (&orientation, &origin);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsEqual (byte const* lhsData, byte const* rhsData, UInt32 dataSize, double distanceTolerance) override
    {
    return PersistableTextStringHelper::AreEqualWithinTolerance (lhsData, rhsData, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    PersistableTextStringHelper::TransformInPlace (transform, &data[0]);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _ProcessProperties (byte* pData, UInt32 dataSize, PropertyContextR context) override
    {
    PersistableTextStringHelper::ProcessProperties (pData, dataSize, context);
    }

}; // XGraphicsDrawTextString

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
void XGraphicsData::Append (void const* data, size_t dataSize)
    {
    if (0 == dataSize)
        return;

    size_t currSize = size();

    resize (currSize + dataSize);
    memcpy (&(at(currSize)), data, dataSize);
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawOperator : XGraphicsOperator
{
enum  DrawCachableMode
    {
    DrawCachableOnly,
    DrawUncachableOnly,
    DrawAll
    };

    ViewContextR                m_viewContext;
    XGraphicsOperationContextR  m_operationContext;
    DrawCachableMode            m_drawCachableMode;
    DrawCachableMode            m_complexDrawMode;
    UInt32                      m_inComplex;

XGraphicsDrawOperator (ViewContextR viewContext, XGraphicsOperationContextR opContext, DrawCachableMode drawCachableMode = DrawAll) : m_viewContext (viewContext), m_operationContext (opContext), m_drawCachableMode (drawCachableMode)
    {
    m_complexDrawMode = DrawAll;
    m_inComplex = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DoOperation (XGraphicsOperationR operation, byte* pData, UInt32 dataSize, byte* pEnd, XGraphicsOpCodes opCode) override
    {
    if (m_viewContext.CheckStop ())
        return ERROR;

    if (!operation._IsStateChange () && DrawAll != m_drawCachableMode)
        {
        bool    skipDraw = false;

        // Uncachable is determined at start of a complex construct and applies until it's completed...
        if (0 == m_inComplex)
            {
            switch (m_drawCachableMode)
                {
                case DrawCachableOnly:
                    {
                    if (!operation._IsCachable (m_viewContext, pData, dataSize, m_operationContext))
                        skipDraw = true;
                    break;
                    }

                case DrawUncachableOnly:
                    {
                    if (!operation._IsUncachable (m_viewContext, pData, dataSize, m_operationContext))
                        skipDraw = true;
                    break;
                    }
                }
            }
        else
            {
            if (m_complexDrawMode != m_drawCachableMode)
                skipDraw = true;
            }

        switch (opCode)
            {
            case XGRAPHIC_OpCode_BeginSweepProject:
            case XGRAPHIC_OpCode_BeginSweepExtrude:
            case XGRAPHIC_OpCode_BeginSweepRevolve:
            case XGRAPHIC_OpCode_BeginComplexString:
            case XGRAPHIC_OpCode_BeginComplexShape:
            case XGRAPHIC_OpCode_BeginMultiSymbologyBody:
                {
                m_inComplex++;

                if (1 == m_inComplex)
                    m_complexDrawMode = (skipDraw ? (DrawCachableOnly == m_drawCachableMode ? DrawUncachableOnly : DrawCachableOnly) : m_drawCachableMode);
                break;
                }

            case XGRAPHIC_OpCode_EndSweep:
            case XGRAPHIC_OpCode_EndComplex:
            case XGRAPHIC_OpCode_EndMultiSymbologyBody:
                {
                m_inComplex--;
                break;
                }
            }

        if (skipDraw)
            return SUCCESS;
        }

    return operation._Draw (m_viewContext, pData, dataSize, m_operationContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Increment (XGraphicsOperationR operation, byte*& pData, UInt32 dataSize) override
    {
    operation._DrawBranch (pData, dataSize, m_viewContext, m_operationContext);
    }

}; // XGraphicsDrawOperator

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MatSymbStateSaver
{
ViewContextR    m_context;
ElemMatSymb     m_elmMatSymb;

MatSymbStateSaver (ViewContextR context) : m_context (context)
    {
    // NOTE: Preserve current ElemMatSymb state in case a 2nd pass to draw un-cachable geometry is required.
    //       Cached draw doesn't change OvrMatSymb (ActivateOverrideMatSymb is a no-op).
    //       ViewContext::CreateCacheElem preserves ElemDisplayParams already.
    m_elmMatSymb = *context.GetElemMatSymb ();
    }

~MatSymbStateSaver ()
    {
    // Restore and re-activate original ElemMatSymb if changed...
    if (m_elmMatSymb == *m_context.GetElemMatSymb ())
        return;

    m_context.GetIDrawGeom ().ActivateMatSymb (&m_elmMatSymb);
    *m_context.GetElemMatSymb () = m_elmMatSymb;
    }

}; // MatSymbStateSaver

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct XGraphicsCacheStroker : IStrokeForCache
{
byte const*                                 m_data;
byte const*                                 m_dataEnd;
XGraphicsOperationContextR                  m_operationContext;
bool                                        m_stroked;
XGraphicsDrawOperator::DrawCachableMode     m_cachableMode;
bool                                        m_wantLocateByQvElem;

XGraphicsCacheStroker (byte const* data, byte const* dataEnd, XGraphicsOperationContextR operationContext, XGraphicsDrawOperator::DrawCachableMode cachableMode = XGraphicsDrawOperator::DrawCachableOnly) : 
                       m_operationContext (operationContext), m_data (data), m_dataEnd (dataEnd), 
                       m_cachableMode (cachableMode), m_stroked (false), m_wantLocateByQvElem (false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _GetSizeDependentGeometryPossible () override {return true;}
virtual bool _GetSizeDependentGeometryStroked () override {return m_operationContext.m_sizeDependentGeometryExists;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantLocateByQvElem () {return m_wantLocateByQvElem;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SetWantLocateByQvElem (ViewContextR context, XGraphicsOperationContextR opContext)
    {
    // Avoid iterating over container when interior locate isn't required...
    switch (context.GetDrawPurpose ())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::FenceAccept:
            break;

        default:
            return;
        }

    // NOTE: Stroke will only output brep edge cache, need QvElem locate for brep surface/silhouette...
    m_wantLocateByQvElem = HasBrep (&opContext, m_data, m_dataEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool NeedsBRepWires (ViewContextR context, XGraphicsOperationContextR opContext, byte const* pData, byte const* pDataEnd)
    {
    if (!HasBrep (&opContext, pData, pDataEnd))
        return false;

    return opContext.AllowDrawWireframe (context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasBrep (XGraphicsOperationContextP opContext, byte const* pData, byte const* pDataEnd)
    {
    if (opContext && opContext->m_header->m_version > 1)
        return opContext->m_header->m_brepsPresent;
    byte*   start = const_cast <byte*> (pData);
    byte*   end   = const_cast <byte*> (pDataEnd);

    return (SUCCESS == XGraphicsOperations::FindOperation (NULL, NULL, &start, end, XGRAPHIC_OpCode_DrawBody));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsCachedDrawRequired (ViewContextR context, XGraphicsOperationContextR opContext, byte const* pData, byte const* pDataEnd)
    {
    if (0 != (opContext.m_drawOptions & XGraphicsContainer::DRAW_OPTION_IgnoreUseCache))
        return false;

    if (!opContext.m_header->m_useCache)
        return false;

    bool    hasOvrLS = (0 != (context.GetOverrideMatSymb ()->GetFlags () & MATSYMB_OVERRIDE_Style) && NULL != context.GetOverrideMatSymb ()->GetMatSymbR ().GetLineStyleSymbR ().GetILineStyle ());

    if (hasOvrLS && !XGraphicsCacheStroker::HasBrep (&opContext, pData, pDataEnd))
        return false; // BReps can't be drawn un-cached...ignore a custom linestyle override if container includes BReps...

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsUnCachedDrawRequired (ViewContextR context, XGraphicsOperationContextR opContext, byte const* pData, byte const* pDataEnd)
    {
    if (opContext.m_header->m_uncachablePresent)
        return true;

    if (NULL != context.GetElemMatSymb ()->GetLineStyleSymbR ().GetILineStyle ())
        return true;
    
    if (XGraphicsCacheStroker::NeedsBRepWires (context, opContext, pData, pDataEnd))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    m_stroked = true;
    
    DrawFromMemory (context, m_data, m_dataEnd, m_operationContext, m_cachableMode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt DrawFromMemory (ViewContextR context, byte const* pData, byte const* pDataEnd, XGraphicsOperationContextR opContext, XGraphicsDrawOperator::DrawCachableMode drawMode)
    {
    XGraphicsDrawOperator draw (context, opContext, drawMode);
    
    return XGraphicsOperations::Traverse (const_cast <byte*> (pData), const_cast <byte*> (pDataEnd), draw);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt DrawCachedFromMemory (ViewContextR context, CachedDrawHandleCR dh, UInt32 qvIndex, byte const* pData, byte const* pDataEnd, XGraphicsOperationContextR opContext)
    {
    MatSymbStateSaver      saveState (context);
    XGraphicsCacheStroker  stroker (pData, pDataEnd, opContext);
    
    stroker.SetWantLocateByQvElem (context, opContext); // Check for breps which require QvElem locate for surface hits...                                                                                              

    context.DrawCached (dh, stroker, qvIndex);

    return (stroker.m_stroked ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt DrawOrCacheFromMemory (ViewContextR context, CachedDrawHandleCR dh, UInt32 qvIndex, byte const* pData, byte const* pDataEnd, XGraphicsOperationContextR opContext)
    {
    // TFS# 107922 - Add context mark for drawing xGraphics containers - an early abort could leave inconsistent transform stack.
    ViewContext::ContextMark mark (&context);

    bool    requireCached = IsCachedDrawRequired (context, opContext, pData, pDataEnd);
    bool    requireUnCached = IsUnCachedDrawRequired (context, opContext, pData, pDataEnd);

    if (!requireCached)
        return DrawFromMemory (context, pData, pDataEnd, opContext, XGraphicsDrawOperator::DrawAll);

    DrawCachedFromMemory (context, dh, qvIndex, pData, pDataEnd, opContext);

    if (requireUnCached)
        DrawFromMemory (context, pData, pDataEnd, opContext, XGraphicsDrawOperator::DrawUncachableOnly);

    return SUCCESS;
    }

};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void XGraphicsSymbolCache::ExtractSymbolIds (T_XGraphicsSymbolIdsR symbolIds, XGraphicsSymbolStampCR symbolStamp)
    {
    //  if defined(SUPPORTING_NESTED_SYMBOLS_IN_STAMPS) -- Graphite does not generate nested symbols.  Leave this as a stub until it does.
    }

#define LINKAGEID_StampSymbolIds 22852
//=======================================================================================
//! Format of the linkage data that maps symbol index into symbol ID.
// @bsiclass                                                    John.Gooding    04/2014
//=======================================================================================
struct SymbolIdsLinkageData
    {
    UInt16      m_key;
    UInt16      m_padding;
    UInt32      m_count;
    UInt64      m_ids[1];

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
static size_t GetLinkageSize(T_XGraphicsSymbolIds const& symbolIds)
    {
    if (symbolIds.size() == 0)
        return 0;

    return sizeof (SymbolIdsLinkageData) + (symbolIds.size()-1) * sizeof (DgnStampId);
    }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsSymbolCache::ExtractSymbolIds (T_XGraphicsSymbolIdsR symbolIds, ElementHandleCR element)
    {
    symbolIds.clear ();

    //  Try to stamp symbol ID's.  If I thought there were many iterations I would move EndElementLinkages out of the loop. However, I expect it
    //  to be 0 or 1 calls.
    for (ConstElementLinkageIterator iter = element.BeginElementLinkages(LINKAGEID_StampSymbolIds); iter != element.EndElementLinkages(); ++iter)
        {
        SymbolIdsLinkageData    linkageData;
        SymbolIdsLinkageData*   pUnaligned = (SymbolIdsLinkageData*)iter.GetData();
        memcpy(&linkageData, pUnaligned, sizeof linkageData);

        if (STAMPID_LINKAGE_KEY_SymbolIdMap != linkageData.m_key)
            continue;

        symbolIds.resize(linkageData.m_count);
        memcpy(&symbolIds[0], pUnaligned->m_ids, linkageData.m_count * sizeof (pUnaligned->m_ids[0]));
        return;
        }

#if defined (NEEDS_WORK_DGNITEM)
    DependencyLinkageAccessor dependencyLinkage;

    if (SUCCESS == DependencyManagerLinkage::GetLinkage (&dependencyLinkage, element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_XGraphicsSymbol))
        {
        DependencyLinkage const*   aligned = dependencyLinkage.GetAlignedData();
        for (int i = 0; i < aligned->nRoots; ++i)
            symbolIds.push_back (ElementId(aligned->root.elemid[i]));
        }
#endif
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawSymbol : XGraphicsOperation
{

virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR optimizeContext) override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt32      elementIndex;
    Transform   transform;

    Get (elementIndex, transform, pData, dataSize);

    if (elementIndex + 1 < opContext.m_symbolIds.size())
        {
        printf ("Draw Symbol - Symbol Not Found\n");
        return;
        }

    printf ("Draw Symbol: %d [%lld] \n", (int)elementIndex,  opContext.m_symbolIds[elementIndex].GetValue());

    dumpTransform (transform);

#if defined(WIP_NEW_CACHE)
    ElementRefP      symbolElemRef;
    static bool     s_showSymbol = true;

    if (s_showSymbol && elementIndex < opContext.m_symbolIds.size() &&
        NULL != (symbolElemRef = opContext.m_dgnProject.Models().FindElementById (opContext.GetSymbolId (elementIndex))))
        {
        XAttributeHandle  iterator (symbolElemRef, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0);

        if (iterator.IsValid () && iterator.GetSize () > sizeof (XGraphicsHeader))
            {
            byte*                   data = (byte *) iterator.PeekData();
            EditElementHandle       symbolEeh (symbolElemRef);
            T_XGraphicsSymbolIds    symbolIds;

            XGraphicsSymbolCache::ExtractSymbolIds (symbolIds, symbolEeh);

            AutoRestore <T_XGraphicsSymbolIds>  saveSymbolIds (&opContext.m_symbolIds, symbolIds);

            printf (">>>>>>>>>>>>>>Symbol: >>>>>>>>>>>>>>>>>>>\n");
            XGraphicsDumpOperator dump (opContext, data + sizeof (XGraphicsHeader));
            XGraphicsOperations::Traverse (data + sizeof (XGraphicsHeader), data + iterator.GetSize(), dump);
            printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* data, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt32      symbolIndex;
    Transform   transform;

    Get (symbolIndex, transform, data, dataSize);

    if (symbolIndex >= opContext.m_symbolIds.size())
        return ERROR;

    AutoRestore <bool>      saveViewdependentGeometryExists (&opContext.m_sizeDependentGeometryExists, false);

    //  We assume that no project has symbols as stamps and elements. Therefore, we are not concerned that the ID
    //  spaces may intersect.
    DgnStampId  stampId(opContext.m_symbolIds[symbolIndex].GetValue());

    XGraphicsSymbolStampP symbolStamp;

    if (NULL != opContext.m_sourceElement)
        {
        //  Associate this stamp with the current element. A stamp can be associated with any number of elements.  The stamp should remain
        //  valid and QvElems should remain in the cache until all of the elements associated with the stamp are unloaded.
        symbolStamp = DgnSymbolStampPinner::GetAndPin (opContext.m_sourceElement, stampId);
        }
    else
        {
        //  We are counting on the element refs to keep the stamps in memory.  If we load the DgnSymbolStamp without
        //  adding the reference count then we need to make sure the cache does not refer to the stamp.  For now, this is an unsolved problem.  
        //  This happens during the upgrade process, but only for symbols saved as elements.
        XGraphicsSymbolStampPtr symbolStampPtr = XGraphicsSymbolStamp::Get(opContext.m_dgnProject, DgnStampId(opContext.m_symbolIds[symbolIndex].GetValue()));
        //  If valid we found the stamp but did not expect to.  We should set up a source element whenever we need to access a stamp.
        BeAssert(!symbolStampPtr.IsValid());
        symbolStamp = NULL;
        }

    if (NULL != symbolStamp)
        return XGraphicsContainer::DrawSymbolFromStamp (context, *symbolStamp, opContext, transform);

#if defined (NEEDS_WORK_DGNITEM)
    //  No stamp. Try it as an element.
    ElementId symbolElementId(opContext.m_symbolIds[symbolIndex].GetValue());
    PersistentElementRefPtr symbolElemRef;
    if (NULL != opContext.m_sourceElement)
        symbolElemRef = ElementRefPinner::GetAndPin (opContext.m_sourceElement, symbolElementId);
    else
        {
        HighPriorityOperationBlock highPriorityOperationBlock;
        symbolElemRef = opContext.m_dgnProject.Models().GetElementById (symbolElementId);
        }

    if (symbolElemRef.IsNull())
        return SUCCESS;

    return XGraphicsContainer::DrawSymbol (context, symbolElemRef.get(), opContext, transform);
#endif
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    UInt32      elementIndex;
    Transform   thisTransform;
    byte*       pData = &data[0];

    Get (elementIndex, thisTransform, pData, (UInt32) data.size());
    thisTransform.productOf (transform.GetTransform(), &thisTransform);

    double      packedTransform[12];
    int         packedTransformSize = packTransform (packedTransform, thisTransform);

    data.resize (sizeof (elementIndex));
    data.Append (packedTransform, packedTransformSize);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt Get (UInt32& elementIndex, TransformR transform, byte const* pData, UInt32 dataSize)
    {
    GET_AND_INCREMENT (elementIndex);

    UInt32      transformSize = dataSize - sizeof (elementIndex);

    if (transformSize == sizeof (transform))
        {
        GET_AND_INCREMENT (transform);
        }
    else if (transformSize == (sizeof (DPoint3d) + sizeof (DPoint4d)))
        {
        DPoint3d    translation;
        DPoint4d    quaternion;
        RotMatrix   rMatrix;

        GET_AND_INCREMENT (translation);
        GET_AND_INCREMENT (quaternion);

        rMatrix.initFromQuaternion (&quaternion.x);
        transform.initFrom (&rMatrix, &translation);
        }
    else if (transformSize == sizeof (DPoint3d))
        {
        DPoint3d    translation;

        GET_AND_INCREMENT (translation);
        transform.initFrom (&translation);
        }
    else
        {
        BeAssert (false);
        }

    return SUCCESS;
    }
}; // XGraphicsDrawSymbol

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsBeginMultiSymbologyBody : XGraphicsOperation
{
    virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override { printf ("BeginMultiSymbologyBody\n"); }
    virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override { return SUCCESS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsCachable (ViewContextR viewContext, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return true; // Always need QvElem to display surface...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return opContext.AllowDrawWireframe (context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* data, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.BeginMultiSymbologyBody ();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    UInt16      opCode;
    UInt32      opSize;

    for (; pData < pEnd; pData += opSize - sizeof (opSize))
        {
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        XGraphicsOperations::Draw (opCode, &context, pData, opSize - sizeof (opSize), opContext);
        size += opSize + sizeof (opCode);

        if (XGRAPHIC_OpCode_EndMultiSymbologyBody == opCode)
            return SUCCESS;
        }

    BeAssert (false); // No EndMultiSymbologyBody found
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override
    {
    bool    meshParasolid   = 0 != (context.m_options & XGRAPHIC_OptimizeOptions_MeshParasolid);
    bool    meshPlanar      = 0 != (context.m_options & XGRAPHIC_OptimizeOptions_MeshFromPlanarBRep);

    if (!meshPlanar && !meshParasolid)
        return ERROR;

    IFacetTopologyTablePtr          table;
    DgnBoxDetail                    box;
    Transform                       bodyTransform;
    bmap <UInt32, bvector <byte> >  subElementSymbologies;

    for (; pData < pEnd; pData = nextOp (pData, opSize))
        {
        UInt16 opCode;

        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            {
            pData = pEnd;
            return ERROR;
            }

        switch (opCode)
            {
            case XGRAPHIC_OpCode_DrawBody:
                if ((meshParasolid || meshPlanar) && SUCCESS != facetTopologyTableFromBody (table, bodyTransform, pData, opSize, pEnd, !meshParasolid))
                    {
                    LOG.error (L"Failed to create IFacetTopologyTable for multi-symbology body.");
                    pData = pEnd;
                    return ERROR;
                    }

                break;

            case XGRAPHIC_OpCode_MatSymb:
                {
                UInt32          subElementIndex;
                UInt16          matSymbMask;
                bvector <byte>  matSymbCopy;

                // Store a copy since we're going to modify the stream to remove the subElementIndex.
                matSymbCopy.resize (opSize - sizeof (opSize));
                memcpy (&matSymbCopy[0], pData, opSize - sizeof (opSize));

                if (!XGraphicsMatSymb::ExtractAndRemoveSubElementIndex (subElementIndex, matSymbMask, matSymbCopy))
                    {
                    LoggingManager::GetLogger (L"DgnCore")->message (LOG_ERROR, L"Failed to find sub-element index for ElemMatSymb in multi-symbology body.");
                    pData = pEnd;
                    return ERROR;
                    }

                if (0 != matSymbMask)
                    subElementSymbologies[subElementIndex] = matSymbCopy;

                break;
                }

            case XGRAPHIC_OpCode_EndMultiSymbologyBody:
                {
                pData = nextOp (pData, opSize);

                IFacetOptionsPtr                options = IFacetOptions::New ();
                T_FaceToSubElemIdMap const*     faceToSubElemIdMap = table->_GetFaceToSubElemIdMap();

                SetXGraphicsFacetOptions(*options.get(), true);

                if (subElementSymbologies.empty() || NULL == faceToSubElemIdMap || faceToSubElemIdMap->empty())
                    {
                    PolyfaceHeaderPtr polyface = PolyfaceHeader::New();
                    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface (*polyface.get(), *table.get(), *options.get()))
                        {
                        LoggingManager::GetLogger (L"DgnCore")->message (LOG_ERROR, L"Failed to find convert IFacetTopologyTable to single polyface.");
                        pData = pEnd;
                        return ERROR;
                        }
                    optimizedData.BeginDraw();
                    polyface->Transform (bodyTransform, true);
                    XGraphicsWriter (&optimizedData).WritePolyface (*polyface.get(), true);
                    }
                else
                    {
                    bvector <PolyfaceHeaderPtr>             polyfaces;
                    bvector <bvector<byte>* >               matSymbBlobs;
                    bmap <int, PolyfaceHeaderCP>            faceToPolyfaces;

                    polyfaces.push_back (PolyfaceHeader::New ());      // For default faces (ones without attachments).

                    PolyfaceHeaderP     currPolyface = polyfaces.front().get();

                    for (T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMap->begin(); curr != faceToSubElemIdMap->end(); curr++)
                        {
                        bmap <UInt32, bvector <byte> >::iterator found = subElementSymbologies.find (curr->second);

                        if (found == subElementSymbologies.end())
                            {
                            faceToPolyfaces[curr->first] = currPolyface;
                            }
                        else
                            {
                            PolyfaceHeaderPtr   polyface = PolyfaceHeader::New ();

                            faceToPolyfaces[curr->first] = currPolyface = polyface.get();
                            polyfaces.push_back (polyface);
                            matSymbBlobs.push_back (&found->second);
                            }
                        }

                    if (SUCCESS != IFacetTopologyTable::ConvertToPolyfaces (polyfaces, faceToPolyfaces, *table.get(), *options.get()))
                        {
                        LoggingManager::GetLogger (L"DgnCore")->message (LOG_ERROR, L"Failed to find convert IFacetTopologyTable to multiple polyfaces.");
                        pData = pEnd;
                        return ERROR;
                        }

                    optimizedData.BeginDraw();
                    XGraphicsWriter writer (&optimizedData);

                    for (size_t i=0; i<polyfaces.size(); i++)
                        {
                        if (0 != polyfaces[i]->GetPointIndexCount())
                            {
                            if (i)
                                {
                                bvector <byte>* matSymbBlob = matSymbBlobs[i-1];

                                writer.WriteOpCode (XGRAPHIC_OpCode_MatSymb);
                                writer.WriteUInt32 ((UInt32) matSymbBlob->size() + sizeof (opSize));
                                optimizedData.Write (&matSymbBlob->front(), matSymbBlob->size());
                                }
                            polyfaces[i]->Transform (bodyTransform, true);  
                            writer.WritePolyface (*polyfaces[i], true);
                            }
                        }
                    }

                return SUCCESS;
                }
            }
        }

    pData = pEnd;
    LoggingManager::GetLogger (L"DgnCore")->message (LOG_ERROR, L"Failed to find EndMultiSymbologyBody after BeginMultiSymbologyBody in XGraphics.");
    return ERROR;
    }

}; // XGraphicsBeginMultiSymbologyBody

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsEndMultiSymbologyBody : XGraphicsOperation
{
    virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override { printf ("EndMultiSymbologyBody\n"); }
    virtual StatusInt _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override { return SUCCESS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* data, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    opContext.EndMultiSymbologyBody (context);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Should be included in other operation that's performing _CalculateRange - this function should not be called.
    BeAssert (false);
    return ERROR;
    }
}; // XGraphicsEndMultiSymbologyBody

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/
struct XGraphicsConditionalDrawBase : XGraphicsOperation
{
    virtual bool        _IsStateChange () override { return true; }
    virtual StatusInt   _Draw (ViewContextR context, byte const* data, UInt32 dataSize, XGraphicsOperationContextR opContext) override { return SUCCESS; }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    getDisplayFilterHandlerId (DisplayFilterHandlerId& id, byte*& pData)
    {
    UInt16      majorId, minorId, handlerId;

    GET_AND_INCREMENT (minorId);
    GET_AND_INCREMENT (majorId);
    GET_AND_INCREMENT (handlerId);

    id = (DisplayFilterHandlerId) handlerId; 
    }

void    getDisplayFilterHandlerId (DisplayFilterHandlerId& id, byte const*& pData) { return getDisplayFilterHandlerId (id, (byte*&) pData); }

/*=================================================================================**//**
* @bsiclass                                                     Paul.Connelly   07/2013
+===============+===============+===============+===============+===============+======*/
struct          XGraphicsFilteredConditionalDrawBase : XGraphicsConditionalDrawBase
    {
    struct FilterData
        {
        byte*                   data;
        byte const*             pEnd;
        DisplayFilterHandlerP   handler;

        bool Extract (byte* data_, UInt32 dataSize_)
            {
            data = data_;
            pEnd = data + dataSize_;

            UInt32 endSize;
            DisplayFilterHandlerId filterId;

            GET_AND_INCREMENT_DATA (endSize, data);
            getDisplayFilterHandlerId (filterId, data);

            handler = DisplayFilterHandlerManager::GetManager().GetHandler (filterId);
            BeAssert (NULL != handler);
            return NULL != handler;
            }
        };

// /*---------------------------------------------------------------------------------**//**
// * @bsimethod                                                    Paul.Connelly   07/13
// +---------------+---------------+---------------+---------------+---------------+------*/
// virtual bool _DeepCopyRoots (byte* data, UInt32 dataSize, CopyContextR context) override
//     {
//     if (context.IsSameFile ())
//         return false;
// 
//     FilterData fd;
// 
//     if (!fd.Extract (data, dataSize))
//         return false;
// 
//     fd.handler->DoClone (fd.data, fd.pEnd - fd.data, context);
// 
//     return true;
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnWriteToElement (byte* data, UInt32 dataSize, ElementHandleCR eh) override
    {
    FilterData fd;
    if (fd.Extract (data, dataSize))
        return fd.handler->OnWriteToElement (fd.data, fd.pEnd - fd.data, eh);
    else
        return ERROR;
    }
}; // XGraphicsFilteredConditionalDrawBase

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/
struct XGraphicsIfConditionalDraw : XGraphicsFilteredConditionalDrawBase
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override 
    {
    byte*                   pData = &data[0];
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;
    DisplayFilterHandlerP   handler;
   
    GET_AND_INCREMENT (endSize);
    getDisplayFilterHandlerId (filterId, pData);

    if (NULL == (handler = DisplayFilterHandlerManager::GetManager().GetHandler (filterId)))
        {
        //BeAssert (false); WIP_VANCOUVER_MERGE displayfilter
        return ERROR;
        }

    return handler->OnTransform (transform, pData, &data[data.size()] - pData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool      _IsEqual (byte const* data, byte const* rhsData, UInt32 dataSize, double distanceTolerance)  override
    {
    byte const*             pEnd = data + dataSize;
    UInt32                  endSize, rhsEndSize;
    DisplayFilterHandlerId  filterId, rhsFilterId;
    DisplayFilterHandlerP   handler;
   
    GET_AND_INCREMENT_DATA (endSize, data);
    getDisplayFilterHandlerId (filterId, data);

    GET_AND_INCREMENT_DATA (rhsEndSize, rhsData);
    getDisplayFilterHandlerId (rhsFilterId, rhsData);

    if (endSize != rhsEndSize || filterId  != rhsFilterId)
        return false;

    if (NULL == (handler = DisplayFilterHandlerManager::GetManager().GetHandler (filterId)))
        {
        //BeAssert (false); WIP_VANCOUVER_MERGE displayfilter
        return false;
        }

    return  handler->IsEqual (data, rhsData, pEnd - data, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override 
    { 
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;


    GET_AND_INCREMENT (endSize);
    getDisplayFilterHandlerId (filterId, pData);

    context.PushBranch (endSize);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _DrawBranch (byte*& pIncrement, UInt32 dataSize, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    byte*                   pData = pIncrement;
    byte*                   pEndOperation = pData + dataSize;
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;
   
    GET_AND_INCREMENT (endSize);
    byte*                   pEndBlock = pData + endSize - sizeof (endSize);
    getDisplayFilterHandlerId (filterId, pData);
    
    if (0 != endSize && !context.IfConditionalDraw (filterId, opContext.m_element, pData, pEndOperation - pData))
        {
        pIncrement = pEndBlock;
        return true;
        }

    pIncrement = pEndOperation;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    // #ifdef NEEDS_WORK_TopazMerge_ - alignment adjustment?
    byte const*             pEndOperation = pData + dataSize;
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;
   
    GET_AND_INCREMENT (endSize);
    getDisplayFilterHandlerId (filterId, pData);

    printf ("IfConditionalDraw, Handler ID: (%d), Branch by: %d\n", filterId, (int)endSize);
    
    DisplayFilterHandlerP   handler;

    if (NULL != (handler = DisplayFilterHandlerManager::GetManager().GetHandler (filterId)))
        printf ("%ls\n", handler->GetDumpString (pData, pEndOperation - pData, opContext.m_dgnProject).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Punt on conditional draw for now.
    return ERROR;
    }
}; // XGraphicsIfConditionalDraw


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/
struct XGraphicsElseIfConditionalDraw : XGraphicsFilteredConditionalDrawBase
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override 
    {
    byte*                   pData = &data[0];
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;
    DisplayFilterHandlerP   handler;
   
    GET_AND_INCREMENT (endSize);
    getDisplayFilterHandlerId (filterId, pData);

    if (NULL == (handler = DisplayFilterHandlerManager::GetManager().GetHandler (filterId)))
        return ERROR;

    return handler->OnTransform (transform, pData, &data[data.size()] - pData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override 
    { 
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;

    GET_AND_INCREMENT (endSize);
    getDisplayFilterHandlerId (filterId, pData);

    context.PushBranch (endSize);
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Punt on conditional draw for now.
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _DrawBranch (byte*& pIncrement, UInt32 dataSize, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    byte*                   pData = pIncrement;
    byte*                   pEndOperation = pData + dataSize;
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;
   
    GET_AND_INCREMENT (endSize);
    byte*                   pEndBlock = pData + endSize - sizeof (endSize);
    getDisplayFilterHandlerId (filterId, pData);
    
    if (0 != endSize && !context.ElseIfConditionalDraw (filterId, opContext.m_element, pData, pEndOperation - pData))
        {
        pIncrement = pEndBlock;
        return true;
        }
    pIncrement = pEndOperation;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    byte const*             pEndOperation = pData + dataSize;
    UInt32                  endSize;
    DisplayFilterHandlerId  filterId;
   
    GET_AND_INCREMENT (endSize);
    getDisplayFilterHandlerId (filterId, pData);

    printf ("ElseIfConditionalDraw, Handler ID: (%d), Branch By: %d\n", filterId, (int)endSize); 
    
    DisplayFilterHandlerP   handler;

    if (NULL != (handler = DisplayFilterHandlerManager::GetManager().GetHandler (filterId)))
        printf ("%ls\n", handler->GetDumpString (pData, pEndOperation - pData, opContext.m_dgnProject).c_str());
    }

}; // XGraphicsElseIfConditionalDraw

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/
struct XGraphicsElseConditionalDraw : XGraphicsConditionalDrawBase
{
virtual StatusInt       _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override { return SUCCESS; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context) override 
    { 
    UInt32 endSize;
    GET_AND_INCREMENT (endSize);

    context.PushBranch (endSize);
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Punt on conditional draw for now.
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _DrawBranch (byte*& pIncrement, UInt32 dataSize, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    byte*  pData = pIncrement;
    byte*  pEndOperation = pData + dataSize;
    UInt32 endSize;
   
    GET_AND_INCREMENT (endSize);
    byte*  pEndBlock = pData + endSize - sizeof (endSize);
   
    if (0 != endSize && !context.ElseConditionalDraw ())
        {
        pIncrement = pEndBlock;
        return true;
        }

    pIncrement = pEndOperation;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    UInt32 endSize;
    GET_AND_INCREMENT (endSize);

    printf ("ElseConditionalDraw, Branch by: %d\n", (int)endSize);
    }
}; // XGraphicsElseConditionalDraw

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsEndConditionalDraw : XGraphicsConditionalDrawBase
{
virtual StatusInt       _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override { return SUCCESS; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _Draw (ViewContextR context, byte const* data, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    context.EndConditionalDraw ();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
virtual BentleyStatus _CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext) override
    {
    // Punt on conditional draw for now.
    return ERROR;
    }

    virtual void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override {printf ("EndConditionalDraw\n");}
}; // XGraphicsEndConditionalDraw

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  transformXGraphicsData (XGraphicsData& transformedData, byte const* pDataIn, byte const* pEnd, TransformInfoCR transform) 
    {
    XGraphicsData   transformedOperationData;
    int             transformDepth = 0;


    for (byte* pData = const_cast <byte*> (pDataIn); pData < pEnd; )
        {
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (SUCCESS != (status = XGraphicsOperations::GetOperation (opCode, opSize, pData, const_cast <byte*> (pEnd))))
            return status;

        transformedOperationData.clear ();
        transformedOperationData.Append (pData, opSize - sizeof (opSize));

        if (0 == transformDepth &&
            SUCCESS != (status = XGraphicsOperations::OnTransform (opCode, transform, transformedOperationData)))
            return status;

        UInt32  newOpSize = (UInt32)(transformedOperationData.size() + sizeof (newOpSize));

        transformedData.Append (&opCode, sizeof (opCode));
        transformedData.Append (&newOpSize, sizeof (newOpSize));
        transformedData.Append (&transformedOperationData[0], transformedOperationData.size());

        switch (opCode)
            {
            case XGRAPHIC_OpCode_PushTransClip:
                transformDepth++;
                break;

            case XGRAPHIC_OpCode_PopTransClip:
                transformDepth--;
                break;

            case XGRAPHIC_OpCode_PopAll:
                transformDepth = 0;
                break;
            }

        pData += (opSize - sizeof (opSize));
        }
    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2012
+===============+===============+===============+===============+===============+======*/
struct DrawAlignedStroker : ViewContext::IStrokeAligned
{
byte const*                     m_data;
byte const*                     m_dataEnd;
XGraphicsOperationContextR      m_operationContext;

DrawAlignedStroker (byte const* data, byte const* dataEnd, XGraphicsOperationContextR operationContext) : m_data (data), m_dataEnd (dataEnd), m_operationContext (operationContext) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeAligned (ViewContextR context) override
    {
    XGraphicsCacheStroker::DrawFromMemory (context, m_data, m_dataEnd, m_operationContext, XGraphicsDrawOperator::DrawAll);
    }

}; // DrawAlignedStroker

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2012
+===============+===============+===============+===============+===============+======*/                                                                                           
struct XGraphicsDrawAligned : XGraphicsOperation
{
// Data Layout....
//      Origin
//      Range
//      Stroke Data count
//      Stroke Data (XGraphicsHeader  + XGraphics)...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    printf ("DrawAligned: \n"); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool            _IsUncachable (ViewContextR, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR) override { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    DVec3d          axis;
    DPoint3d        origin;
    UInt32          alignmentMode;
    UInt32          strokeSize;

    GET_AND_INCREMENT (axis);
    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (alignmentMode);
    GET_AND_INCREMENT (strokeSize);

    DrawAlignedStroker  stroker (pData, pData + strokeSize, opContext);
    
    context.DrawAligned (axis, origin, (ViewContext::AlignmentMode) alignmentMode, stroker);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
    DVec3dP     axis   = (DVec3dP) &data[0];
    DPoint3dP   origin = (DPoint3dP) (axis+1);

    transform.GetTransform()->MultiplyMatrixOnly (*axis);
    axis->Normalize ();
    transform.GetTransform()->Multiply (*origin);
    return SUCCESS;
    }

};  // XGraphicsDrawAligned


/*=================================================================================**//**
* @bsiclass                                                     RayBentley      11/2012
+===============+===============+===============+===============+===============+======*/                                                                                           
struct XGraphicsSetLocatePriority : XGraphicsOperation
{
    virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override { return SUCCESS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    UInt32          priority;

    GET_AND_INCREMENT (priority);

    printf ("SetLocatePriority: %d\n", (int)priority); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt32          priority;

    GET_AND_INCREMENT (priority);
    context.SetLocatePriority (priority);

    return SUCCESS;
    }

};  // XGraphicsSetLocatePriority

/*=================================================================================**//**                                                                                        
* @bsiclass                                                     RayBentley      11/2012
+===============+===============+===============+===============+===============+======*/                                                                                           
struct XGraphicsSetNonSnappable : XGraphicsOperation
{
    virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override { return SUCCESS; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    UInt32          nonSnappable;

    GET_AND_INCREMENT (nonSnappable);

    printf ("SetNonSnappable: %d\n", (int)nonSnappable); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    UInt32          nonSnappable;

    GET_AND_INCREMENT (nonSnappable);
    context.SetNonSnappable (0 != nonSnappable);

    return SUCCESS;
    }

};  // XGraphicsSetNonSnappable

//=======================================================================================
// @bsiclass                                                    MattGooding     10/13
//=======================================================================================
struct XGraphicsOBSOLETEDrawAreaPattern : XGraphicsOperation
{
void _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    printf ("Obsolete DrawAreaPattern - early Topaz demo file. Should be republished.\n");
    }

StatusInt _Draw (ViewContextR, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
    return SUCCESS;
    }

StatusInt _OnTransform (TransformInfoCR, XGraphicsDataR data) override
    {
    return SUCCESS;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct XGraphicsDrawAreaPattern : XGraphicsOperation
{
// Data Layout....
//      Origin
//      Range
//      Parameter byte count.
//      Parameter data
//      Boundary byte count
//      Boundary Data (XGraphicsHeader  + XGraphics)...

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    Extract 
(
DRange3dR           range,
PatternParamsR      params, 
DwgHatchDefLine     hatchLines[MAX_DWG_EXPANDEDHATCH_LINES], 
byte const*&        pStartBoundary,
UInt32&             boundarySize,
byte const*         pData,
byte const*         pEnd
)
    {
#if defined (NEEDS_WORK_DGNITEM)
    DPoint3d        origin;
    UInt32          paramSize;

    GET_AND_INCREMENT (origin);
    GET_AND_INCREMENT (range);
    GET_AND_INCREMENT (paramSize);

    if (pData + paramSize > pEnd)
        {
        BeAssert (false);
        return ERROR;
        }

    StatusInt       status;
    if (SUCCESS != (status = PatternLinkageUtil::Extract (params, hatchLines, MAX_DWG_EXPANDEDHATCH_LINES, pData, true)))
        return status;

    params.SetOrigin (origin);

    pData += paramSize;
    GET_AND_INCREMENT (params.cellId);
    GET_AND_INCREMENT (boundarySize);
    if (pData + boundarySize > pEnd)
        {
        BeAssert (false);
        return ERROR;
        }
    
    pStartBoundary = pData;
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void            _Dump (byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override 
    { 
    printf ("DrawAreaPattern: \n"); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IsUncachable (ViewContextR, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR) override { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       _Draw (ViewContextR context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext) override
    {
#if defined (NEEDS_WORK_DGNITEM)
    DRange3d                range;
    PatternParams           params;
    DwgHatchDefLine         hatchLines[MAX_DWG_EXPANDEDHATCH_LINES]; // huge local!
    byte const*             pBoundary;
    UInt32                  boundarySize;
    StatusInt               status;

    if (SUCCESS != (status = Extract (range, params, hatchLines, pBoundary, boundarySize, pData, pData + dataSize)))
        return status;

    ViewContext::PatternParamSource     source (&params, hatchLines);
    XGraphicsCacheStroker               stroker (pBoundary, pBoundary + boundarySize, opContext, XGraphicsDrawOperator::DrawAll);
    ViewContext::ClipStencil            boundary (stroker, 0, false);

    if (DrawPurpose::RangeCalculation == context.GetDrawPurpose())
        {
        context.DrawCached (CachedDrawHandle(opContext.m_element), stroker, 0);
        return SUCCESS;
        }

    EditElementHandle   tmpEeh;

    // NEEDSWORK_V10: This is horrible...
    ExtendedElementHandler::InitializeElement (tmpEeh, NULL, *opContext.m_dgnProject.Models ().GetModelById (opContext.m_dgnProject.Models ().GetFirstModelId ()), true);
    tmpEeh.GetElementP ()->GetRangeR () = range;

    context.DrawAreaPattern (tmpEeh, boundary, source);
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   _OnTransform (TransformInfoCR transform, XGraphicsDataR data) override
    {
#if defined (NEEDS_WORK_DGNITEM)
    DRange3d                range;
    PatternParams           params;
    DwgHatchDefLine         hatchLines[MAX_DWG_EXPANDEDHATCH_LINES]; // huge local!
    UInt32                  boundarySize;
    byte const*             pBoundary;
    StatusInt               status;
    HatchLinkage            linkage;

    if (SUCCESS != (status = Extract (range, params, hatchLines, pBoundary, boundarySize, &data.front(), &data.front() + data.size())))
        return status;

    transform.GetTransform()->Multiply (params.origin);
    transform.GetTransform()->Multiply (range, range);
    PatternLinkageUtil::Transform (params, hatchLines, *transform.GetTransform(), false, true);

    UInt32              paramSize = PatternLinkageUtil::Create (linkage, params, hatchLines, true);
    XGraphicsData       transformedBoundaries;
    
    if (SUCCESS != (status = transformXGraphicsData (transformedBoundaries, pBoundary, pBoundary + boundarySize, transform)))
        return status;

    boundarySize = (UInt32) transformedBoundaries.size();

    data.clear();
    data.Append (&params.origin, sizeof (params.origin));
    data.Append (&range, sizeof (range));
    data.Append (&paramSize, sizeof (paramSize));
    data.Append (&linkage.modifiers, paramSize);
    data.Append (&params.cellId, sizeof (params.cellId));
    data.Append (&boundarySize, sizeof (boundarySize));
    data.Append (&transformedBoundaries.front(), transformedBoundaries.size());
#endif
    
    return SUCCESS;
    }

};  // XGraphicsDrawAreaPattern

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct XGraphicsGetBasisOperator : XGraphicsOperator
{
    bool                        m_basisFound;
    TransformR                  m_basisTransform;
    bvector<Transform>          m_transformStack;

    XGraphicsGetBasisOperator (TransformR transform) : m_basisTransform (transform) { m_basisFound = false; }
    StatusInt GetStatus () { return m_basisFound ? SUCCESS : ERROR; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DoOperation (XGraphicsOperationR operation, byte* pData, UInt32 size, byte* pEnd, XGraphicsOpCodes opCode) override
    {
    switch (opCode)
        {
        case XGRAPHIC_OpCode_PushTransClip:
            {
            TransformCP         transform = (TransformCP) pData;

            if (m_transformStack.empty())
                {
                m_transformStack.push_back (*transform);
                }
            else
                {
                Transform       composite;

                composite.productOf (transform, &m_transformStack.back());
                m_transformStack.push_back (composite);
                }

            break;
            }

        case XGRAPHIC_OpCode_PopTransClip:
            if (!m_transformStack.empty())
            m_transformStack.pop_back();
            break;

        case XGRAPHIC_OpCode_PopAll:
            m_transformStack.clear ();
            break;
        }

    // Needs work... handle transform push/pops.
    if (!m_basisFound && SUCCESS == operation._GetBasisTransform (m_basisTransform, pData, size, pEnd))
        {
        if (!m_transformStack.empty())
            m_basisTransform.productOf (&m_transformStack.back(), &m_basisTransform);

        m_basisFound = true;
        }

    return SUCCESS;
    }

}; // XGraphicsGetBasisOperator

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct XGraphicsTestTransformOperator : XGraphicsOperator
{
    TransformCR     m_transform;
    XGraphicsTestTransformOperator (TransformCR transform) : m_transform (transform) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DoOperation (XGraphicsOperationR operation, byte* pData, UInt32 size, byte* pEnd, XGraphicsOpCodes opCode) override
    {
    return operation._TestTransform (m_transform, pData, size);
    }
}; // XGraphicsTestTransformOperator

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
XGraphicsOperations::XGraphicsOperations ()
    {
    s_xGraphicOps[XGRAPHIC_OpCode_DrawLineString3d        ] = new XGraphicsDrawLineString3d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawLineString2d        ] = new XGraphicsDrawLineString2d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawPointString3d       ] = new XGraphicsDrawPointString3d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawPointString2d       ] = new XGraphicsDrawPointString2d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawArc3d               ] = new XGraphicsDrawArc3d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawArc2d               ] = new XGraphicsDrawArc2d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawEllipse3d           ] = new XGraphicsDrawEllipse3d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawEllipse2d           ] = new XGraphicsDrawEllipse2d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawShape3d             ] = new XGraphicsDrawShape3d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawShape2d             ] = new XGraphicsDrawShape2d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawCone                ] = new XGraphicsDrawCone ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawTorus               ] = new XGraphicsDrawTorus ();
    s_xGraphicOps[XGRAPHIC_OpCode_PushTransClip           ] = new XGraphicsPushTransClip ();
    s_xGraphicOps[XGRAPHIC_OpCode_PopTransClip            ] = new XGraphicsPopTransClip ();
    s_xGraphicOps[XGRAPHIC_OpCode_PopAll                  ] = new XGraphicsPopAll ();
    s_xGraphicOps[XGRAPHIC_OpCode_BeginSweepProject       ] = new XGraphicsBeginSweepProject ();
    s_xGraphicOps[XGRAPHIC_OpCode_BeginSweepExtrude       ] = new XGraphicsBeginSweepExtrude ();
    s_xGraphicOps[XGRAPHIC_OpCode_BeginSweepRevolve       ] = new XGraphicsBeginSweepRevolve ();
    s_xGraphicOps[XGRAPHIC_OpCode_EndSweep                ] = new XGraphicsEndSweep ();
    s_xGraphicOps[XGRAPHIC_OpCode_BeginComplexString      ] = new XGraphicsBeginComplexString ();
    s_xGraphicOps[XGRAPHIC_OpCode_BeginComplexShape       ] = new XGraphicsBeginComplexShape ();
    s_xGraphicOps[XGRAPHIC_OpCode_EndComplex              ] = new XGraphicsEndComplex ();
    s_xGraphicOps[XGRAPHIC_OpCode_AddDisconnect           ] = new XGraphicsAddDisconnect ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawBody                ] = new XGraphicsDrawBody ();
    s_xGraphicOps[XGRAPHIC_OpCode_AddIndexPolys           ] = new XGraphicsAddIndexPolys ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawBSplineCurve        ] = new XGraphicsDrawBSplineCurve ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawBSplineSurface      ] = new XGraphicsDrawBSplineSurface ();
    s_xGraphicOps[XGRAPHIC_OpCode_MatSymb                 ] = new XGraphicsMatSymb ();
    s_xGraphicOps[XGRAPHIC_OpCode_MatSymb2                ] = new XGraphicsMatSymb2 ();
    s_xGraphicOps[XGRAPHIC_OpCode_AnnotationData          ] = new XGraphicAnnotationData ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawBox                 ] = new XGraphicsDrawBox ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawSphere              ] = new XGraphicsDrawSphere ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawTriStrip3d          ] = new XGraphicsDrawTriStrip3d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawTriStrip2d          ] = new XGraphicsDrawTriStrip2d ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawSymbol              ] = new XGraphicsDrawSymbol ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawText                ] = new XGraphicsDrawTextString ();
    s_xGraphicOps[XGRAPHIC_OpCode_OBSOLETEDrawAreaPattern ] = new XGraphicsOBSOLETEDrawAreaPattern ();
    s_xGraphicOps[XGRAPHIC_OpCode_BeginMultiSymbologyBody ] = new XGraphicsBeginMultiSymbologyBody ();
    s_xGraphicOps[XGRAPHIC_OpCode_EndMultiSymbologyBody   ] = new XGraphicsEndMultiSymbologyBody ();
    s_xGraphicOps[XGRAPHIC_OpCode_IfConditionalDraw       ] = new XGraphicsIfConditionalDraw ();
    s_xGraphicOps[XGRAPHIC_OpCode_ElseConditionalDraw     ] = new XGraphicsElseConditionalDraw ();
    s_xGraphicOps[XGRAPHIC_OpCode_ElseIfConditionalDraw   ] = new XGraphicsElseIfConditionalDraw ();
    s_xGraphicOps[XGRAPHIC_OpCode_EndConditionalDraw      ] = new XGraphicsEndConditionalDraw ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawAligned             ] = new XGraphicsDrawAligned ();
    s_xGraphicOps[XGRAPHIC_OpCode_NoOp                    ] = new XGraphicsNoOp ();
    s_xGraphicOps[XGRAPHIC_OpCode_SetLocatePriority       ] = new XGraphicsSetLocatePriority ();
    s_xGraphicOps[XGRAPHIC_OpCode_SetNonSnappable         ] = new XGraphicsSetNonSnappable ();
    s_xGraphicOps[XGRAPHIC_OpCode_DrawAreaPattern         ] = new XGraphicsDrawAreaPattern ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperations::StaticInitialize ()
    {
    s_XGraphicsOperations = new XGraphicsOperations;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::AppendToGPA (UInt16 opCode, byte const* data, UInt32 size, GPArrayR gpa)
    {
    if (opCode >= MAX_XGraphicsOpCode)
        return ERROR;

    return s_xGraphicOps[opCode]->_AppendToGPA (data, size, gpa);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperations::IsEqual (UInt16 opCode, byte const* thisData, byte const* otherData, UInt32 size, double distanceTolerance)
    {
    if (opCode >= MAX_XGraphicsOpCode)
        return false;

    return s_xGraphicOps[opCode]->_IsEqual (thisData, otherData, size, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/10
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOperations::ProcessProperties (UInt16 opCode, byte* data, UInt32 size, PropertyContextR context)
    {
    if (opCode >= MAX_XGraphicsOpCode)
        return;

    s_xGraphicOps[opCode]->_ProcessProperties (data, size, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
//bool            XGraphicsOperations::DeepCopyRoots (UInt16 opCode, byte* data, UInt32 size, ElementCopyContextR context)
//    {
//    if (opCode >= MAX_XGraphicsOpCode)
//        return false;
//
//    return s_xGraphicOps[opCode]->_DeepCopyRoots (data, size, context);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsOperations::OnWriteToElement (UInt16 opCode, byte* data, UInt32 size, ElementHandleCR eh)
    {
    if (opCode < MAX_XGraphicsOpCode)
        return s_xGraphicOps[opCode]->_OnWriteToElement (data, size, eh);
    else
        return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::GetOperation (UInt16& opCode, UInt32& opSize, byte*& pData, byte const* pEnd)
    {
    GET_AND_INCREMENT (opCode);
    GET_AND_INCREMENT (opSize);

    if (opCode < 0 || opCode >= MAX_XGraphicsOpCode)
        {
        BeAssert (false && "Invalid XGraphics op code\n");

        return ERROR;
        }

    if ((opSize - sizeof(opSize)) > (UInt32)(pEnd - pData)) // op data must not exceed data remaining (note: opSize includes itself)
        {
        BeAssert (false && "embedded op size exceeds actual data size");

        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::Traverse (byte* pData, byte* pEnd, XGraphicsOperatorR xGraphicsOperator)
    {
    for (; pData < pEnd;)
        {
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (SUCCESS != (status = GetOperation (opCode, opSize, pData, pEnd)))
            return status;

        XGraphicsOperationP xGraphicsOperation = s_xGraphicOps[opCode];
        if (NULL == xGraphicsOperation)
            return ERROR;

        opSize -= sizeof (opSize);

        if (SUCCESS != (status = xGraphicsOperator._DoOperation (*xGraphicsOperation, pData, opSize, pEnd, (XGraphicsOpCodes) opCode)))
            return status;

        xGraphicsOperator._Increment (*xGraphicsOperation, pData, opSize);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsOperation::_DrawBranch (byte*& pData, UInt32 opSize, ViewContextR viewContext, XGraphicsOperationContextR opContext) { pData += opSize; return false; }
void XGraphicsOperator::_Increment (struct XGraphicsOperation& operation, byte*& pData, UInt32 opSize) { pData += opSize; }
void XGraphicsConstOperator::_Increment (struct XGraphicsOperation& operation, byte const*& pData, UInt32 opSize) { pData += opSize; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::Traverse (byte const* pData, byte const* pEnd, XGraphicsConstOperatorR xGraphicsOperator)
    {
    for (; pData < pEnd;)
        {
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (SUCCESS != (status = GetOperation (opCode, opSize, const_cast <byte*&> (pData), const_cast <byte*> (pEnd))))
            return status;

        XGraphicsOperationP xGraphicsOperation = s_xGraphicOps[opCode];
        if (NULL == xGraphicsOperation)
            return ERROR;

        opSize -= sizeof (opSize);

        if (SUCCESS != (status = xGraphicsOperator._DoOperation (*xGraphicsOperation, pData, opSize, pEnd, (XGraphicsOpCodes) opCode)))
            return status;

        xGraphicsOperator._Increment (*xGraphicsOperation, pData, opSize);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::FindOperation (byte** ppOpStart, UInt32* pOpSize, byte** ppData, byte const* pEnd, XGraphicsOpCodes opToFind)
    {
    byte*       pData = *ppData;

    for (; pData < pEnd;)
        {
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (NULL != ppOpStart)
            *ppOpStart = pData;

        if (SUCCESS != (status = GetOperation (opCode, opSize, pData, pEnd)))
            return status;

        pData = nextOp (pData, opSize);

        if (opCode == opToFind)
            {
            if (NULL != pOpSize)
                *pOpSize = opSize;

            *ppData = pData;

            return SUCCESS;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::FindComplex (byte** ppStart, byte** ppEnd, byte** ppData, byte const* pEnd, XGraphicsOpCodes opCode)
    {
    StatusInt   status;

    if (SUCCESS == (status = FindOperation (ppStart, NULL, ppData, pEnd, opCode)))
        {
        if (SUCCESS == (status = FindOperation (ppEnd, NULL, ppData, pEnd, XGRAPHIC_OpCode_EndComplex)))
            *ppEnd = *ppData;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::FindBoundary (byte** ppBoundary, byte** ppEnd, byte* pData, byte* pEnd)
    {
    UInt16          opCode;
    UInt32          opSize;
    StatusInt       status;

    *ppBoundary = pData;
    if (SUCCESS != (status = GetOperation (opCode, opSize, pData, pEnd)))
        return status;

    pData = nextOp (pData, opSize);
    if (XGRAPHIC_OpCode_BeginComplexString == opCode || XGRAPHIC_OpCode_BeginComplexShape == opCode)
        {
        byte*       pEndComplex;

        if (SUCCESS == (status = FindOperation (&pEndComplex, &opSize, &pData, pEnd, XGRAPHIC_OpCode_EndComplex)) &&
            SUCCESS == (status = GetOperation (opCode, opSize, pEndComplex, pEnd)))
            *ppEnd = nextOp (pEndComplex, opSize);
        }
    else
        {
        *ppEnd = pData;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::FindBoundaries (byte** ppBoundary0, byte** ppEnd0, byte** ppBoundary1, byte** ppEnd1, byte* pData, byte* pEnd)
    {
    StatusInt   status;
    byte        *pTempData = pData, *pEndSweep;

    if (SUCCESS != (status = FindOperation (&pEndSweep, NULL, &pTempData, pEnd, XGRAPHIC_OpCode_EndSweep)))
        return status;

    if (SUCCESS == (status = FindBoundary (ppBoundary0, ppEnd0, pData, pEnd)))
        status = FindBoundary (ppBoundary1, ppEnd1, *ppEnd0, pEnd);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::Traverse (XGraphicsDataR data, XGraphicsOperatorR xGraphicsOperator)
    {
    return Traverse (&data[0] + sizeof (XGraphicsHeader), &data[0] + data.size(), xGraphicsOperator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::Traverse (XGraphicsDataCR data, XGraphicsConstOperatorR xGraphicsOperator)
    {
    return Traverse (&data[0] + sizeof (XGraphicsHeader), &data[0] + data.size(), xGraphicsOperator);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::TestTransform (TransformCR transform, byte* pData, byte* pEnd)
    {
    XGraphicsTestTransformOperator testTransform (transform);
    return Traverse (pData, pEnd, testTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::GetBasisTransform (TransformR transform, byte* pData, byte* pEnd)
    {
    XGraphicsGetBasisOperator   getBasisOperator (transform);

    Traverse (pData, pEnd, getBasisOperator);

    StatusInt   status;

    if (SUCCESS == (status = getBasisOperator.GetStatus ()))
        {
        Transform   inverse;

        inverse.inverseOf (&transform);

        if (SUCCESS != (status = TestTransform (inverse, pData, pEnd)))
            {
            RotMatrix   rMatrix;
            DPoint3d    translation;

            transform.getMatrix (&rMatrix);
            transform.getTranslation (&translation);
            rMatrix.squareAndNormalizeColumns (&rMatrix, 0, 1);

            transform.initFrom (&rMatrix, &translation);
            status = TestTransform (transform, pData, pEnd);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::Optimize (XGraphicsContainerR optimizedData, UInt16 opCode, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR context)
    {
    return s_xGraphicOps[opCode]->_Optimize (optimizedData, pData, opSize, pEnd, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsOperations::OnTransform (UInt16 opCode, TransformInfoCR transform, XGraphicsDataR data)
    {
    return s_xGraphicOps[opCode]->_OnTransform (transform, data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem*         XGraphicsContainer::CreateQvElem (ElementHandleCR eh, ViewContextR viewContext, QvCache* qvCache, double pixelSize, DrawOptions options)
    {
    T_XGraphicsSymbolIds        nullIDs;
    XGraphicsOperationContext   opContext ((XGraphicsHeader*) GetData(), *eh.GetDgnProject(), nullIDs, &eh, options);
    XGraphicsCacheStroker       stroker (GetGraphicsData(), GetDataEnd(), opContext);

    return viewContext.CreateCacheElem (CachedDrawHandle(&eh), qvCache, stroker, NULL, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::DrawXGraphicsFromMemory (ViewContextR context, byte const* pData, size_t size, DgnModelP dgnCache, DrawOptions options)
    {
    T_XGraphicsSymbolIds        nullIDs;
    XGraphicsOperationContext   opContext ((XGraphicsHeader*) pData, dgnCache->GetDgnProject (), nullIDs, NULL, options);

    return XGraphicsCacheStroker::DrawFromMemory (context, pData + sizeof (XGraphicsHeader), pData + size, opContext, XGraphicsDrawOperator::DrawAll);
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct  XGraphicsOutput : public IViewDraw
{
    DEFINE_T_SUPER(IViewDraw)
    friend struct DgnPlatform::XGraphicsContext;

protected:
    bvector<XGraphicsContainerP>        m_parents;
    ViewFlags                           m_viewFlags;
    XGraphicsSymbolCacheP               m_symbolCache;
    ElemMatSymb                         m_currentMatSymb;
    ElemDisplayParams                   m_currentDisplayParams;
    bool                                m_currentMatSymbValid;
    UInt32                              m_optimizeOptions;
    UInt32                              m_createOptions;
    ViewContextP                        m_viewContext;
    int                                 m_complexDepth;
    bool                                m_viewDependentPresent;
    bool                                m_customLineStylePresent;
    UInt16                              m_initialMask;
    bool                                m_writeSubMaterials;
    int                                 m_subElemIndex;
    Symbology                           m_baseSymbology;
    std::stack <CookedDisplayStyleCP>   m_displayStyles;
    bool                                m_forceSymbologyInclusion;

public:
    XGraphicsWriter                 m_writer;

    void         SetUseCache (bool useCache) { m_writer.SetUseCache (useCache); }
    bool         ViewDependentPresent() const { return m_viewDependentPresent; }
    bool         CustomLineStylePresent () const { return m_customLineStylePresent; }
    void         SetViewDependentPresent () { m_viewDependentPresent = true; }
    void         SetViewContext (ViewContextP viewContext) { m_viewContext = viewContext; }
    void         SetForceSymbologyInclusion (bool force) { m_forceSymbologyInclusion = force; }
    bool         GetForceSymbologyInclusion () const { return m_forceSymbologyInclusion; }

    ViewFlags*  _GetDrawViewFlags() override { return &m_viewFlags; }
    void        _SetDrawViewFlags(ViewFlags const *flags) override { m_viewFlags = *flags; }
    RangeResult _PushBoundingRange3d (DPoint3dCP) override { return RangeResult::Overlap; }
    RangeResult _PushBoundingRange2d (DPoint2d const*, double zDepth) override { return RangeResult::Overlap; }
    StatusInt   _TestOcclusion (int numVolumes, DPoint3dP verts, int* results) override { return ERROR; }
    void        _PopBoundingRange() override {}
    void        _DrawMosaic (int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) override { BeAssert(false && "An XGraphics stream will never contain a raster image"); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsOutput (XGraphicsContainer& container, UInt32 createOptions = XGRAPHIC_CreateOptions_None, UInt32 optimizeOptions = XGRAPHIC_OptimizeOptions_None) : m_writer (&container), m_symbolCache (NULL), m_viewContext (NULL), m_viewDependentPresent (false), m_customLineStylePresent (false)
    {
    memset (&m_viewFlags, 0, sizeof (m_viewFlags));
    m_viewFlags.SetRenderMode (MSRenderMode::SmoothShade);
    m_viewFlags.inhibitLineStyles = true; // NOTE: Don't want linestyles dropped, also symbol draw not supported...
    m_viewFlags.dimens = true;
    m_currentMatSymbValid = false;
    m_createOptions = createOptions;
    m_optimizeOptions = optimizeOptions;
    m_complexDepth = 0;
    m_initialMask = 0;
    m_writeSubMaterials = false;
    m_forceSymbologyInclusion = false;
    m_subElemIndex = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnableInitialWeightMatSym ()
    {
    m_initialMask |= XMATSYMB_ElemWeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnableInitialColorMatSym ()
    {
    m_initialMask |= XMATSYMB_ElemColor | XMATSYMB_ElemFillColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnableInitialLineCodeMatSym ()
    {
    m_initialMask |= XMATSYMB_ElemStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteAnnotationData (XGraphicsAnnotationData const& annotationData)
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_AnnotationData);
    m_writer.WriteUInt32 ((UInt32)annotationData.m_sig);
    m_writer.WriteData ((void*)&annotationData.m_data[0], (UInt32)annotationData.m_data.size());
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginSymbol()
    {
    m_parents.push_back (m_writer.m_container);
    if (1 == m_parents.size())                           // Disable nesting.
        {
        m_writer.m_container = new XGraphicsContainer ();
        m_writer.m_container->BeginDraw ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    GeorgeDulchinos 09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EndSymbolDiscard ()
    {
    XGraphicsContainerP symbol = m_writer.m_container;
    delete symbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EndSymbol (DgnModelP modelRef)
    {
    BeAssert(NULL != modelRef);
    if (m_parents.empty())
        return;

    if (m_parents.size() > 1)                           // Disable nesting.
        {
        m_parents.pop_back();
        return;
        }

    Transform           transform;
    size_t              symbolIndex;

    XGraphicsContainerP parent = m_parents.back(), symbol = m_writer.m_container;
    static  UInt32      s_minimumSymbolSize = 400;
    static  UInt32      s_maximumSymbolSize = 1000000;

    symbol->Optimize (m_optimizeOptions);
    symbol->EndDraw ();

    m_parents.pop_back ();
    m_writer.m_container = parent;

    if (symbol->IsEmpty())
        return;

    if (symbol->IsRenderable())
        m_writer.SetIsRenderable (true);

    DgnProjectR dgnProject = modelRef->GetDgnProject();
    if ((symbol->GetDataSize() < s_minimumSymbolSize  && !symbol->UseCache ()) ||
        symbol->GetDataSize() > s_maximumSymbolSize ||
        SUCCESS != XGraphicsSymbolCache::Get(dgnProject).AddSymbol (transform, symbolIndex, *parent, *symbol, *modelRef, m_currentMatSymbValid ? &m_baseSymbology : NULL))
        {
        if (symbol->UseCache())
            m_writer.SetUseCache (true);

        m_writer.AppendContainer (*symbol);
        delete symbol;
        }
    else
        {
        m_writer.BeginOperation (XGRAPHIC_OpCode_DrawSymbol);
        m_writer.WriteUInt32 ((UInt32) symbolIndex);
        m_writer.WritePackedTransform (transform);

        m_writer.EndOperation();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void SetElemDisplayParams (ElemDisplayParamsCP elemDisplayParams) {}
void _ActivateOverrideMatSymb (OvrMatSymbCP ovrMatSymb) override {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _ActivateMatSymb (ElemMatSymbCP matSymb) override
    {
    // Can't change matsymb when nested construct incomplete
    if (0 != m_complexDepth)
        return;

    if (NULL == m_viewContext->GetCurrentDisplayParams())
        return;

    ElemDisplayParams   displayParams;

    displayParams =  *m_viewContext->GetCurrentDisplayParams();
    displayParams.Resolve (*m_viewContext);

    if (NULL != matSymb->GetLineStyleSymb ().GetILineStyle ())
        m_customLineStylePresent = true;

    UInt16      mask = m_initialMask;
    bool        settingInitialValues = m_initialMask != 0;

    // Setting initial mask is used for line styles.
    m_initialMask = 0;

    if (!m_currentMatSymbValid)
        {
        m_currentMatSymb = *matSymb;
        m_currentDisplayParams = displayParams;
        m_currentMatSymbValid = true;

        m_baseSymbology.style   = displayParams.GetLineStyle ();
        m_baseSymbology.weight  = displayParams.GetWeight ();
        m_baseSymbology.color   = displayParams.GetLineColor ();

        if (!m_forceSymbologyInclusion && 0 == mask)
            return;
        }

    UInt32      maskExtended = 0;

    // Original version would store changes to active matSymb. - This had several problems associated with keeping in synch
    // with symbology changes so I switched to looking at the element based currentDisplayParams below.
    // The MatSymb based opcodes are still supported (for beta files) but are not created.

    bool    currentUseTBGR = m_currentDisplayParams.IsLineColorTBGR (); // invalid index signifies a true rgb (ex. IsoContour display for Insolation elements)
    bool    displayUseTBGR = displayParams.IsLineColorTBGR ();
    UInt32  currentColor   = m_currentDisplayParams.GetLineColor ();
    UInt32  displayColor   = displayParams.GetLineColor ();
    UInt32  currentTBGR    = currentUseTBGR ? m_currentDisplayParams.GetLineColorTBGR () : 0;
    UInt32  displayTBGR    = displayUseTBGR ? displayParams.GetLineColorTBGR () : 0;
    bool    writeNonIndexedRGB = false;

    if (displayUseTBGR)
        {
        if (m_forceSymbologyInclusion || !currentUseTBGR || (displayTBGR != currentTBGR))
            writeNonIndexedRGB = true;
        }
    else if (m_forceSymbologyInclusion || (displayColor != currentColor))
        {
        m_currentDisplayParams.SetLineColor (displayColor);
        mask |= XMATSYMB_ElemColor;
        }


    if (FillDisplay::Never != displayParams.GetFillDisplay ())
        {
        if (((NULL == displayParams.GetGradient ()) != (NULL == m_currentDisplayParams.GetGradient())) ||
            m_forceSymbologyInclusion
             || 
            (NULL != m_currentDisplayParams.GetGradient () && !(*m_currentDisplayParams.GetGradient () == *displayParams.GetGradient ()))
            )
            {
            if (NULL == displayParams.GetGradient())
                {
                m_currentDisplayParams.SetGradient (NULL);
                }
            else
                {
                GradientSymbPtr     gradient = GradientSymb::Create();

                gradient->CopyFrom (*displayParams.GetGradient());
                m_currentDisplayParams.SetGradient (gradient.get());
                }
            maskExtended |= XMATSYMB2_GradientFill;
            }

        // NEEDSWORK: What about RGB fill? (XMATSYMB2_FillColorRGB)
        if (m_currentDisplayParams.GetFillColor () != displayParams.GetFillColor ())
            {
            m_currentDisplayParams.SetFillColor (displayParams.GetFillColor ());
            mask |= XMATSYMB_ElemFillColor;
            }
        }

    if (m_forceSymbologyInclusion || m_currentDisplayParams.GetWeight () != displayParams.GetWeight ())
        {
        m_currentDisplayParams.SetWeight (displayParams.GetWeight ());
        mask |= XMATSYMB_ElemWeight;
        }

    if (m_forceSymbologyInclusion || m_currentDisplayParams.GetTransparency () != displayParams.GetTransparency ())
        {
        m_currentDisplayParams.SetTransparency (displayParams.GetTransparency ());
        mask |= XMATSYMB_ElemTransparency;
        }

    if (m_forceSymbologyInclusion || settingInitialValues || (m_currentDisplayParams.GetFillDisplay () != displayParams.GetFillDisplay ()))
        {
        m_currentDisplayParams.SetFillDisplay (displayParams.GetFillDisplay ());
        mask |= XMATSYMB_FillDisplay;
        }

    if (m_forceSymbologyInclusion || m_currentDisplayParams.GetLineStyle () != displayParams.GetLineStyle ())
        {
        m_currentDisplayParams.SetLineStyle (displayParams.GetLineStyle ());
        mask |= XMATSYMB_ElemStyle;

        if (displayParams.GetLineStyle () < 0 || displayParams.GetLineStyle () > 7)
            m_writer.m_container->SetUncachablePresent ();
        } 
    else if (settingInitialValues && 0 != (mask & XMATSYMB_ElemStyle))
        {
        // Only hardware 0-7 line codes are valid for initial mask...
        if (!IS_LINECODE (displayParams.GetLineStyle ()))
            mask &= ~XMATSYMB_ElemStyle;
        }

    if (writeNonIndexedRGB)
        {
        m_currentDisplayParams.SetLineColorTBGR (displayTBGR);
        mask |= XMATSYMB_ElemColorRGB;
        }

    LineStyleParamsCP   currentStyleParams = m_currentDisplayParams.GetLineStyleParams ();
    LineStyleParamsCP   displayStyleParams = displayParams.GetLineStyleParams ();

    if (((NULL == currentStyleParams) != (NULL == displayStyleParams)) || (NULL != currentStyleParams && 0 != memcmp (currentStyleParams, displayStyleParams, offsetof (LineStyleParams, normal))))
        {
        m_currentDisplayParams.SetLineStyle (m_currentDisplayParams.GetLineStyle (), displayStyleParams);
        mask |= XMATSYMB_StyleParams;
        }

    if (m_forceSymbologyInclusion || m_currentDisplayParams.GetMaterial () != displayParams.GetMaterial ())
        {
        // BEIJING_DGNPLATFORM_WIP_RENDLIB - NEEDS WORK - Old code actually writes material into file if it's not there already.
        m_currentDisplayParams.SetMaterial (displayParams.GetMaterial ());
        mask |= XMATSYMB_Material;
        }

    if (0 != mask || 0 != maskExtended || (0 != m_subElemIndex && m_writeSubMaterials))
        {
        if (0 != m_subElemIndex)
            {
            mask |= XMATSYMB_SubElemIndex;
            m_writeSubMaterials = true; // Once we write one we need to write all face materials.
            }

        m_writer.WriteMatSymb (mask, maskExtended, m_subElemIndex, m_writeSubMaterials, m_currentMatSymb, m_currentDisplayParams, false, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawLineString3d);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawLineString2d (int numPoints, DPoint2d const* points, double zDepth, DPoint2d const* range) override
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawLineString2d);
    m_writer.WriteDepth (zDepth);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    john.gooding    03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawPointCloud (IPointCloudDrawParams* drawParams) override
    {
    DPoint3dCP  dPoints   = drawParams->GetDPoints ();
    DPoint3dCP  pRange    = NULL;       //  ignoring range
    UInt32      numPoints = drawParams->GetNumPoints ();

    DPoint3d    offsets;
    offsets.init (0, 0, 0);

// Need the GetOrigin call to fill in offsets, but we only need the return value in an assert, so need to wrap to avoid unused variable warnings.
#if !defined (NDEBUG)
    bool        haveOffsets = 
#endif
    drawParams->GetOrigin (&offsets);

    if (NULL != dPoints)
        {
        //  QVision does not support DPoint3d with offsets so QvOutput does not support it and there
        //  is no need to support it here.
        BeAssert (!haveOffsets);

        _DrawPointString3d (numPoints, dPoints, pRange);

        return;
        }

    //  Don't risk stack overflow to get points buffer
    UInt32      maxPointsPerIter = 64000/sizeof (DPoint3d);

    if (numPoints < maxPointsPerIter)
        maxPointsPerIter = numPoints;

    DPoint3dP   pointBuffer = (DPoint3dP)_alloca (maxPointsPerIter * sizeof (*pointBuffer));

    //  Convert float points to DPoints and add offset
    FPoint3dCP      fPoints = drawParams->GetFPoints ();
    FPoint3dCP      currIn = fPoints;

    while (numPoints > 0)
        {
        UInt32  pointsThisIter = numPoints > maxPointsPerIter ? maxPointsPerIter : numPoints;

        for (DPoint3dP  curr = pointBuffer; curr < pointBuffer + pointsThisIter; curr++, currIn++)
            {
            curr->x = currIn->x + offsets.x;
            curr->y = currIn->y + offsets.y;
            curr->z = currIn->z + offsets.z;
            }

        _DrawPointString3d (pointsThisIter, pointBuffer, NULL);
        numPoints -= pointsThisIter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawPointString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawPointString3d);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawPointString2d (int numPoints, DPoint2d const* points, double zDepth, DPoint2d const* range) override
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawPointString2d);
    m_writer.WriteDepth (zDepth);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawArc3d (DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) override
    {
    if (isEllipse && filled&& false)
        {
        // In SS3 the DrawEllipse3d code will not fill a non-complex 3d ellipse correctly if
        // the DrawLineStyles flag is set (it should not be testing that but it does.
        // Workaround here by generating complex with fill. (CVE cuts of cylinders)...
        BeginComplexShape (true);
        _DrawArc3d (ellipse, true, false, range);
        EndComplex();
        return;
        }
    double      r0, r1, start, sweep;
    DVec3d      primary, secondary;
    RotMatrix   rMatrix;
    DPoint3d    center;

    ellipse.GetScaledRotMatrix (center, rMatrix, r0, r1, start, sweep);
    rMatrix.getColumn (&primary, 0);
    rMatrix.getColumn (&secondary, 1);

    m_writer.BeginOperation (isEllipse ? XGRAPHIC_OpCode_DrawEllipse3d : XGRAPHIC_OpCode_DrawArc3d);
    m_writer.WritePoint (&center);
    m_writer.WriteQuaternion (&primary, &secondary);
    m_writer.WriteDistance (r0);
    m_writer.WriteDistance (r1);

    if (!isEllipse)
        {
        m_writer.WriteAngle (start);
        m_writer.WriteAngle (sweep);
        }
    else
        {
        m_writer.WriteFilled (filled);
        }

    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) override
    {
    double      r0, r1, start, sweep;
    DVec3d      primary, secondary, normal;
    RotMatrix   rMatrix;
    DPoint3d    center;

    ellipse.GetScaledRotMatrix (center, rMatrix, r0, r1, start, sweep);
    rMatrix.GetColumn (primary, 0);
    rMatrix.GetColumn (secondary, 1);
    rMatrix.GetColumn (normal, 2);

    if (normal.z < 0.0) // viewed in 2D, but normal is going down. Flip the angles.
        {
        start = -start;
        sweep = -sweep;
        }

    DPoint2d    center2d;
    DVec2d      primary2d;

    center2d.Init (center);
    primary2d.Init (primary.x, primary.y);

    double      orientation = atan2 (primary2d.y, primary2d.x);

    m_writer.BeginOperation (isEllipse ? XGRAPHIC_OpCode_DrawEllipse2d : XGRAPHIC_OpCode_DrawArc2d);
    m_writer.WritePoint (&center2d);
    m_writer.WriteAngle (orientation);
    m_writer.WriteDistance (r0);
    m_writer.WriteDistance (r1);

    if (!isEllipse)
        {
        m_writer.WriteAngle (start);
        m_writer.WriteAngle (sweep);
        }
    else
        {
        m_writer.WriteFilled (filled);
        }

    m_writer.WriteDepth (zDepth);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawShape3d);
    m_writer.WriteFilled (filled);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawShape2d (int numPoints, DPoint2d const* points, bool filled, double zDepth, DPoint2d const* range) override
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawShape2d);
    m_writer.WriteFilled (filled);
    m_writer.WriteDepth (zDepth);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawTriStrip3d (int numPoints, DPoint3dCP points, Int32 usageFlags, DPoint3dCP range) override
    {
    m_writer.SetIsRenderable (true);
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawTriStrip3d);
    m_writer.WriteInt32 (usageFlags);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawTriStrip2d (int numPoints, DPoint2dCP points, Int32 usageFlags, double zDepth, DPoint2dCP range) override
    {
    m_writer.SetIsRenderable (true);
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawTriStrip2d);
    m_writer.WriteInt32 (usageFlags);
    m_writer.WriteDepth (zDepth);
    m_writer.WritePoints (numPoints, points);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawTextString (TextStringCR text, double* zDepth = NULL) override
    {
    size_t      bufferSize   = PersistableTextStringHelper::ComputeRequiredBufferSize (text);
    byte*       buffer       = (byte*)_alloca (bufferSize);
    size_t      bytesWritten = PersistableTextStringHelper::AppendToBuffer (buffer, text, zDepth, m_viewContext->GetDgnProject ());

    m_writer.SetUseCache (true);
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawText);
    m_writer.WriteData (buffer, bytesWritten);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawBSplineCurve (MSBsplineCurveCR bCurve, bool filled) override
    {
    m_writer.WriteBsplineCurve (bCurve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            _DrawBSplineCurve2d (MSBsplineCurveCR bCurve, bool filled, double zDepth) override
    {
    if (0.0 != zDepth)
        m_writer.SetUseCache (true); // SS3 doesn't have _DrawBSplineCurve2d - Handle priority with DrawQvElem2d...

    m_writer.WriteBsplineCurve (bCurve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawBSplineSurface (MSBsplineSurfaceCR bSurface) override
    {
    m_writer.WriteBsplineSurface (bSurface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, double zDepth, DPoint2dCP range) override
    {
    m_writer.SetUnsupportedPrimitivePresent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawRaster (DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, DPoint3d const *range) override
    {
    m_writer.SetUnsupportedPrimitivePresent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawDgnOle (IDgnOleDraw*) override
    {
    m_writer.SetUnsupportedPrimitivePresent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _PushTransClip (TransformCP trans, ClipPlaneSetCP clip) override
    {
    if (NULL == trans)
        {
        Transform   idTrans = Transform::FromIdentity();

        trans = &idTrans;
        }

    m_writer.BeginOperation (XGRAPHIC_OpCode_PushTransClip);
    m_writer.WriteTransform (trans);
    m_writer.WriteClip (clip);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _PopTransClip () override
    {
    m_writer.WriteOperation (XGRAPHIC_OpCode_PopTransClip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginSweepProject (int enableCap)
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_BeginSweepProject);
    m_writer.WriteEnableCap (TO_BOOL (enableCap));
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginSweepExtrude (DVec3dCP extrusionP, int enableCap)
    {
    m_writer.SetIsRenderable (true);
    m_writer.BeginOperation (XGRAPHIC_OpCode_BeginSweepExtrude);
    m_writer.WriteVector (extrusionP);
    m_writer.WriteEnableCap (TO_BOOL (enableCap));
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginSweepRevolve (DVec3dCP rPrimary, DVec3dCP rSecondary, DPoint3dCP rOrigin, double rPrimaryRadius, double rSecondaryRadius, DVec3dCP revAxis, double rSweep, int enableCap)
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_BeginSweepRevolve);
    m_writer.WriteVector (rPrimary);
    m_writer.WriteVector (rSecondary);
    m_writer.WritePoint (rOrigin);
    m_writer.WriteDistance (rPrimaryRadius);
    m_writer.WriteDistance (rSecondaryRadius);
    m_writer.WriteVector (revAxis);
    m_writer.WriteAngle (rSweep);
    m_writer.WriteEnableCap (TO_BOOL (enableCap));
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void EndSweep ()
    {
    m_writer.SetIsRenderable (true);
    m_writer.SetUseCache (true);                // Force Cache (there is no immediate mode sweep).
    m_writer.WriteOperation (XGRAPHIC_OpCode_EndSweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginComplexString ()
    {
    m_writer.SetUseCache (true);          

    if (0 == m_complexDepth++)
        m_writer.WriteOperation (XGRAPHIC_OpCode_BeginComplexString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginComplexShape (bool filled)
    {
    if (0 == m_complexDepth++)
        {
        m_writer.SetUseCache (true);          
        m_writer.SetIsRenderable (true);
        m_writer.BeginOperation (XGRAPHIC_OpCode_BeginComplexShape);
        m_writer.WriteFilled (filled);
        m_writer.EndOperation ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void EndComplex ()
    {
    if (0 == --m_complexDepth)
        m_writer.WriteOperation (XGRAPHIC_OpCode_EndComplex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void AddDisconnect ()
    {
    m_writer.WriteOperation (XGRAPHIC_OpCode_AddDisconnect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    if (1 > curves.size ())
        return;

    if (curves.IsUnionRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (false && "Unexpected entry in union region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region)...
                }

            CurveVector const* childCurves = curve->GetChildCurveVectorCP ();

            _DrawCurveVector (*childCurves, isFilled);
            }
        }
    else if (curves.IsParityRegion ())
        {
        bool    firstLoop = true;

        BeginComplexShape (isFilled);

        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (false && "Unexpected entry in parity region.");

                return; // Each loop must be a child curve bvector (a closed loop)...
                }

            CurveVector const* childCurves = curve->GetChildCurveVectorCP ();

            if (!firstLoop)
                AddDisconnect ();

            _DrawCurveVector (*childCurves, false);

            firstLoop = false;
            }

        EndComplex ();
        }
    else
        {
        bool    isSingleEntry = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive ());
        bool    isClosed = curves.IsClosedPath ();
        bool    isOpen = curves.IsOpenPath ();
        bool    isComplex = ((isClosed || isOpen) && !isSingleEntry);

        if (isComplex)
            {
            if (isClosed)
                BeginComplexShape (isFilled);
            else
                BeginComplexString ();
            }

        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid ())
                continue;

            CurvePrimitiveIdCP curvePrimitiveId;

            if (NULL != (curvePrimitiveId = curve->GetId()))
                {
                XGraphicsAnnotationData  annotationData;

                annotationData.m_sig = XGraphicsAnnotationData::SIGNATURE_EdgeId;
                curvePrimitiveId->Store (annotationData.m_data);

                WriteAnnotationData (annotationData);
                }

            switch (curve->GetCurvePrimitiveType ())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP segment = curve->GetLineCP ();

                    _DrawLineString3d (2, segment->point, NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> const* points = curve->GetLineStringCP ();

                    if (!isComplex && isClosed)
                        _DrawShape3d ((int) points->size (), &points->front (), isFilled, NULL);
                    else
                        _DrawLineString3d ((int) points->size (), &points->front (), NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP ();

                    _DrawArc3d (*ellipse, !isComplex && isClosed && ellipse->IsFullEllipse(), isFilled, NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP ();
        
                    bool displayFilled = (!isComplex && isClosed && isFilled);
                    bool recordClosure = (displayFilled && !bcurve->params.closed); // Pole-based curve closure does not match loop closure...

                    if (recordClosure)
                        BeginComplexShape (isFilled);

                    _DrawBSplineCurve (*bcurve, displayFilled && !recordClosure);

                    if (recordClosure)
                        EndComplex ();
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                    {
                    bvector<DPoint3d> const* points = curve->GetPointStringCP ();

                    _DrawPointString3d ((int) points->size (), &points->front (), NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                    {
                    CurveVector const* childCurves = curve->GetChildCurveVectorCP ();
                    _DrawCurveVector (*childCurves, false);
                    break;
                    }
                default:
                    {
                    BeAssert (false && "Unexpected entry in CurveVector.");
                    break;
                    }
                }
            }

        if (isComplex)
            EndComplex ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawCurveVector2d (CurveVectorCR curves, bool isFilled, double zDepth) override
    {
    if (1 > curves.size ())
        return;

    if (curves.IsUnionRegion ())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (true && "Unexpected entry in union region.");

                return; // Each loop must be a child curve bvector (a closed loop or parity region)...
                }

            CurveVector const* childCurves = curve->GetChildCurveVectorCP ();

            _DrawCurveVector2d (*childCurves, isFilled, zDepth);
            }
        }
    else if (curves.IsParityRegion ())
        {
        bool    firstLoop = true;

        BeginComplexShape (isFilled);

        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull ())
                continue;

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                {
                BeAssert (true && "Unexpected entry in parity region.");

                return; // Each loop must be a child curve bvector (a closed loop)...
                }

            CurveVector const* childCurves = curve->GetChildCurveVectorCP ();

            if (!firstLoop)
                AddDisconnect ();

            _DrawCurveVector2d (*childCurves, false, zDepth);

            firstLoop = false;
            }

        EndComplex ();
        }
    else
        {
        bool    isSingleEntry = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive ());
        bool    isClosed = curves.IsClosedPath ();
        bool    isOpen = curves.IsOpenPath ();
        bool    isComplex = ((isClosed || isOpen) && !isSingleEntry);

        if (isComplex)
            {
            if (isClosed)
                BeginComplexShape (isFilled);
            else
                BeginComplexString ();
            }

        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid ())
                continue;

            switch (curve->GetCurvePrimitiveType ())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP  segment = curve->GetLineCP ();
                    DPoint2d      points[2];

                    points[0].Init (segment->point[0]);
                    points[1].Init (segment->point[1]);

                    _DrawLineString2d (2, points, zDepth, NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> const* points = curve->GetLineStringCP ();
                    int                      nPts = (int) points->size ();
                    std::valarray<DPoint2d>  localPoints2dBuf (nPts);

                    for (int iPt = 0; iPt < nPts; ++iPt)
                        {
                        DPoint3dCP  tmpPt = &points->front ()+iPt;

                        localPoints2dBuf[iPt].x = tmpPt->x;
                        localPoints2dBuf[iPt].y = tmpPt->y;
                        }

                    if (!isComplex && isClosed)
                        _DrawShape2d (nPts, &localPoints2dBuf[0], isFilled, zDepth, NULL);
                    else
                        _DrawLineString2d (nPts, &localPoints2dBuf[0], zDepth, NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP ();

                    _DrawArc2d (*ellipse, !isComplex && isClosed, isFilled, zDepth, NULL);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP ();
        
                    bool displayFilled = (!isComplex && isClosed && isFilled);
                    bool recordClosure = (displayFilled && !bcurve->params.closed); // Pole-based curve closure does not match loop closure...

                    if (recordClosure)
                        BeginComplexShape (isFilled);

                    _DrawBSplineCurve2d (*bcurve, displayFilled && !recordClosure, zDepth);

                    if (recordClosure)
                        EndComplex ();
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                    {
                    bvector<DPoint3d> const* points = curve->GetPointStringCP ();
                    int                      nPts = (int) points->size ();
                    std::valarray<DPoint2d>  localPoints2dBuf (nPts);

                    for (int iPt = 0; iPt < nPts; ++iPt)
                        {
                        DPoint3dCP  tmpPt = &points->front ()+iPt;

                        localPoints2dBuf[iPt].x = tmpPt->x;
                        localPoints2dBuf[iPt].y = tmpPt->y;
                        }

                    _DrawPointString2d (nPts, &localPoints2dBuf[0], zDepth, NULL);
                    break;
                    }

                default:
                    {
                    BeAssert (true && "Unexpected entry in CurveVector.");
                    break;
                    }
                }
            }

        if (isComplex)
            EndComplex ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawSolidPrimitive (ISolidPrimitiveCR primitive) override
    {
    switch (primitive.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail  detail;

            if (!primitive.TryGetDgnTorusPipeDetail (detail))
                return;

            m_writer.WriteTorus (detail);
            return;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail  detail;

            if (!primitive.TryGetDgnConeDetail (detail))
                return;

            m_writer.WriteCone (detail);
            return;
            }

        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail  detail;

            if (!primitive.TryGetDgnBoxDetail (detail))
                return;

            m_writer.WriteBox (detail);
            return;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail  detail;

            if (!primitive.TryGetDgnSphereDetail (detail))
                return;

            m_writer.WriteSphere (detail);
            return;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail  detail;

            if (!primitive.TryGetDgnExtrusionDetail (detail))
                return;

            BeginSweepExtrude (&detail.m_extrusionVector, detail.m_capped);
            _DrawCurveVector (*detail.m_baseCurve, false);
            EndSweep ();
            return;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail  detail;

            if (!primitive.TryGetDgnRotationalSweepDetail (detail))
                return;

            double      rPrimaryRadius = 1.0, rSecondaryRadius = 1.0;
            DVec3d      rPrimary, rSecondary;
            RotMatrix   axes;

            axes.InitFrom1Vector (detail.m_axisOfRotation.direction, 2, true);
            axes.GetColumn (rPrimary, 0);
            axes.GetColumn (rSecondary, 1);

            BeginSweepRevolve (&rPrimary, &rSecondary, &detail.m_axisOfRotation.origin, rPrimaryRadius, rSecondaryRadius, &detail.m_axisOfRotation.direction, detail.m_sweepAngle, detail.m_capped);
            _DrawCurveVector (*detail.m_baseCurve, false);
            EndSweep ();
            return;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail  detail;

            if (!primitive.TryGetDgnRuledSweepDetail (detail))
                return;

            BeginSweepProject (detail.m_capped);
            for (auto const& curves : detail.m_sectionCurves)
                {
                if (curves->HasSingleCurvePrimitive())
                    curves->IsAnyRegionType() ? BeginComplexShape (false) : BeginComplexString();

                _DrawCurveVector (*curves, false);

                if (curves->HasSingleCurvePrimitive())
                    EndComplex();
                }
            EndSweep ();
            return;
            }

        default:
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawPolyface (PolyfaceQueryCR meshData, bool filled = false) override
    {
    if (filled)
        {
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (meshData, true);

        visitor->SetNumWrap (1);

        for (visitor->Reset (); visitor->AdvanceToNextFace (); )
            _DrawShape3d ((int) visitor->Point ().size (), &visitor->Point ().front (), filled, NULL);

        return;
        }

    size_t  minLoop, maxLoop, numFace;
    bool    bHasNonPlanarFace, bHasNonConvexFace;

    meshData.InspectFaces (numFace, minLoop, maxLoop, bHasNonPlanarFace, bHasNonConvexFace);

    m_writer.WritePolyface (meshData, bHasNonPlanarFace || bHasNonConvexFace); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       _DrawBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments, double pixelSize) override
    {
    bool        isMultiSymb = (NULL != attachments && !attachments->_GetFaceToSubElemIdMap ().empty ());

    if (isMultiSymb)
        BeginMultiSymbologyBody ();
    m_writer.SetBRepsPresent (true);
    m_writer.SetUseCache (true);
    m_writer.SetIsRenderable (true);
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawBody);
    m_writer.WriteSolidKernelEntity (entity);

    m_writer.WriteUInt32 ((UInt32) 0); // 8.11.5 Compatibility (hidden edges).
    m_writer.WriteUInt32 ((UInt32) 0); // 8.11.7 Compatibility (hidden faces).

    m_writer.EndOperation ();

    if (isMultiSymb)
        EndMultiSymbologyBody (*attachments);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void BeginMultiSymbologyBody ()
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_BeginMultiSymbologyBody);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EndMultiSymbologyBody (IFaceMaterialAttachmentsCR attachments)
    {
    T_FaceToSubElemIdMap const& faceToSubElemIdMap = attachments._GetFaceToSubElemIdMap ();
    T_FaceAttachmentsMap const& faceAttachmentsMap = attachments._GetFaceAttachmentsMap ();

    for (T_FaceAttachmentsMap::const_iterator curr = faceAttachmentsMap.begin (); curr != faceAttachmentsMap.end (); ++curr)
        {
        T_FaceToSubElemIdMap::const_iterator found = faceToSubElemIdMap.find (curr->first);

        if (found == faceToSubElemIdMap.end ())
            continue;

        curr->second.ToElemDisplayParams (*m_viewContext->GetCurrentDisplayParams ());
        m_subElemIndex = found->second;
        m_viewContext->CookDisplayParams ();
        }

    m_writer.BeginOperation (XGRAPHIC_OpCode_EndMultiSymbologyBody);
    m_writer.EndOperation ();
    m_writeSubMaterials = false;
    m_subElemIndex = 0;

    return SUCCESS;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void _PushRenderOverrides (ViewFlags viewFlags, CookedDisplayStyleCP displayStyle) override
    {
#ifdef PUSH_RENDER_OVERRIDE_SUPPORT
    m_writer.BeginOperation (XGRAPHIC_OpCode_PushRenderOverrides);
    m_writer.WriteData (&viewFlags, sizeof (viewFlags));
    m_writer.EndOperation ();
    m_displayStyles.push (NULL == displayStyle ? new CookedDisplayStyle (viewFlags, m_displayStyles.empty() ? NULL : m_displayStyles.top()) : new CookedDisplayStyle (*displayStyle));
#endif
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void _PopRenderOverrides () override
    {
#ifdef PUSH_RENDER_OVERRIDE_SUPPORT
    m_writer.BeginOperation (XGRAPHIC_OpCode_PopRenderOverrides);
    m_writer.EndOperation ();

    if (m_displayStyles.empty())
        {
        BeAssert (false);
        return;
        }

    delete m_displayStyles.top();
    m_displayStyles.pop ();
#endif
    BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/04
+---------------+---------------+---------------+---------------+---------------+------*/
CookedDisplayStyleCP _GetDrawDisplayStyle () const override { return m_displayStyles.empty() ? NULL : m_displayStyles.top(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAreaPattern (ElementHandleCR thisElm, ViewContext::ClipStencil& boundary, ViewContext::PatternParamSource& source)
    {
    BeAssert(false);
#if defined (NEEDS_WORK_DGNITEM)
    int                 patternIndex;
    DPoint3d            origin;
    DRange3d            range;
    PatternParamsP      params;
    DwgHatchDefLineP    hatchLines;

    if (NULL == (params = source.GetParams (thisElm, &origin, &hatchLines, &patternIndex, m_viewContext)))
        return;

    CurveVectorPtr  curveVector = boundary.GetCurveVector (thisElm);

    if (!curveVector.IsValid ())
        return;

    curveVector->GetRange (range);

    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawAreaPattern);                                                                                                                                                                                                
    m_writer.WritePoint (&origin);
    m_writer.WritePoint (&range.low);
    m_writer.WritePoint (&range.high);

    HatchLinkage        linkage;
    UInt32 nBytes  = PatternLinkageUtil::Create (linkage, *params, hatchLines, true);
    m_writer.WriteUInt32 (nBytes);
    m_writer.WriteData (&linkage.modifiers, nBytes);
    m_writer.WriteElementId (PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::Cell) ? ElementId() : ElementId(params->cellId));

    UInt32              boundaryLocation = (UInt32) m_writer.GetDataSize();

    BeAssert(false && "conversion to CachedDrawHandle okay here?");
    m_writer.WriteUInt32 (0);
    boundary.GetStroker()._StrokeForCache (CachedDrawHandle(&thisElm), *m_viewContext, 0.0);
    m_writer.WriteUInt32AtLocation (boundaryLocation, (UInt32) m_writer.GetDataSize() - boundaryLocation - sizeof (boundaryLocation));
    m_writer.EndOperation ();

    m_writer.m_container->SetUncachablePresent();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    DrawAligned (DVec3dCR axis, DPoint3dCR origin, ViewContext::AlignmentMode alignmentMode, ViewContext::IStrokeAligned& stroker) 
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_DrawAligned);                                                                                                                                                                                                
    m_writer.WritePoint (&axis);
    m_writer.WritePoint (&origin);
    m_writer.WriteUInt32 ((UInt32) alignmentMode);

    UInt32              dataLocation = (UInt32) m_writer.GetDataSize();

    m_writer.WriteUInt32 (0);
    stroker._StrokeAligned (*m_viewContext);
    m_writer.WriteUInt32AtLocation (dataLocation, (UInt32) m_writer.GetDataSize() - dataLocation - sizeof (dataLocation));
    m_writer.EndOperation ();
    m_writer.m_container->SetUncachablePresent();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void   SetLocatePriority (int priority)          
    {
    m_writer.BeginOperation (XGRAPHIC_OpCode_SetLocatePriority);                                                                                                                                                                                                
    m_writer.WriteUInt32 ((UInt32) priority);
    m_writer.EndOperation ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void   SetNonSnappable (bool unsnappable)                
    { 
    m_writer.BeginOperation (XGRAPHIC_OpCode_SetNonSnappable);
    m_writer.WriteUInt32 (unsnappable ? 1 : 0);
    m_writer.EndOperation ();
    }

// UnsupportedPrimitive...
virtual void _SetToViewCoords (bool yesNo) override {}
virtual void _SetSymbology (UInt32 lineColorTBGR, UInt32 fillColorTBGR, int lineWidth, UInt32 linePattern) override {}
virtual void _DrawGrid (bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3d const& xVector, DVec3d const& yVector, UInt32 gridsPerRef, Point2d const& repetitions) override {}
virtual bool _DrawSprite (ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) override { return false; }
virtual void _DrawTiledRaster (ITiledRaster* tiledRaster) override { }
virtual void _DrawQvElem3d (QvElem* qvElem, int subElemIndex) override {}
virtual void _DrawQvElem2d (QvElem* qvElem, double zDepth, int subElemIndex) override {}
//virtual StatusInt   _BeginViewlet (DPoint3dCR frustum, double fraction, DPoint3dCR center, double width, double height, ClipVectorCP clips, RgbColorDef const* bgColor, DgnAttachmentCP refP) override {return SUCCESS;}
//virtual void        _EndViewlet() override {}
virtual void _ClearZ () override {}
virtual bool _IsOutputQuickVision () const override {return false;};
virtual bool _DeferShadowsToHeal () const override {return false;}
virtual bool _ApplyMonochromeOverrides (ViewFlagsCR flags) const override { return flags.renderMode > static_cast<UInt32>(MSRenderMode::SmoothShade); }
virtual void _PushClipStencil (QvElem* qvElem) override {}
virtual void _PopClipStencil () override {}
}; // XGraphicsOutput

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct          DgnPlatform::XGraphicsContext : public NullContext
{
    DEFINE_T_SUPER(NullContext)
private:

UInt32                          m_fillOutlineThreshold;
bool                            m_removeSymbols;
bvector <ElemDisplayParams>     m_branchDisplayParams;
XGraphicsOutput                 m_output; // Default output...

public:

void BeginSymbol ()                             { GetOutput ().BeginSymbol (); }
void EndSymbol (DgnModelP model)             { GetOutput ().EndSymbol (model); }
void EndSymbolDiscard ()                        { GetOutput ().EndSymbolDiscard (); }
void SetElemDisplayParams (ElemDisplayParamsCP elemDisplayParams) { GetOutput ().SetElemDisplayParams (elemDisplayParams); }

//  Used by plotting via the XGraphicsRecorder when creating the XGraphics for a LineStyle symbol.
void SetFillOutlineThreshold (UInt32 threshold) { m_fillOutlineThreshold = threshold; }
void SetRemoveSymbols (bool removeSymbols) { m_removeSymbols = removeSymbols; }
bool GetRemoveSymbols () const { return m_removeSymbols; }
bool _WantAreaPatterns () override { return true; }
bool NoDisplayFilters() { return 0 != (GetOutput ().m_createOptions & XGRAPHIC_CreateOptions_NoDisplayFilters); }

XGraphicsOutput& GetOutput () {return _GetOutput ();}

protected:

virtual XGraphicsOutput& _GetOutput () {return m_output;}
virtual void _SetupOutputs () override {SetIViewDraw (_GetOutput ());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    GeorgeDulchinos 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _VisitElemHandle (ElementHandleCR inEl, bool checkRange, bool checkScanCriteria) override
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (CellUtil::IsPointCell (inEl))
        GetOutput ().SetViewDependentPresent ();
#endif

    return  T_Super::_VisitElemHandle (inEl, checkRange, checkScanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _CheckFillOutline () override
    {
    ElemMatSymbP matSymb = GetElemMatSymb ();

    if (NULL != matSymb->GetGradientSymb())
        return (0 != (matSymb->GetGradientSymb()->GetFlags () & static_cast<int>(GradientFlags::Outline)));

    if (m_fillOutlineThreshold < matSymb->GetWidth ())
        return true;

    UInt32 lineColor = GetCurrLineColor ();
    UInt32 fillColor = GetCurrFillColor ();

    if (lineColor != fillColor)
        return true;

    // NOTE: If white, because of white-on-white reversal ignore same rgb if only one of the colors is a background or rgb...
    if ((lineColor & 0x00ffffff) != 0x00ffffff)
        return false;

    int lineColorIndex = (m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_Color) ? m_ovrMatSymb.GetLineColorIndex() : m_elemMatSymb.GetLineColorIndex();
    int fillColorIndex = (m_ovrMatSymb.GetFlags() & MATSYMB_OVERRIDE_FillColor) ? m_ovrMatSymb.GetFillColorIndex() : m_elemMatSymb.GetFillColorIndex();

    if ((DgnColorMap::INDEX_Background == lineColorIndex) != (DgnColorMap::INDEX_Background == fillColorIndex))
        return true;

    if ((DgnColorMap::INDEX_Invalid == lineColorIndex) != (DgnColorMap::INDEX_Invalid == fillColorIndex))
        return true;

    return false;
    }

virtual void _CookDisplayParams (ElemDisplayParamsR params, ElemMatSymbR matSymb) override { ViewContext::_CookDisplayParams (params, matSymb); }
virtual void _CookDisplayParamsOverrides () override { ViewContext::_CookDisplayParamsOverrides(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual UInt32 _GetDisplayInfo (bool renderable) override
    {
    bool        isCapped;

    if (NULL != GetCurrentDisplayParams ()->GetThickness (isCapped))
        return DISPLAY_INFO_Thickness;

    UInt32      info = DISPLAY_INFO_Pattern;        // Always get patterns (regardless of flags).
    bool        isFilled = FillDisplay::Never != GetCurrentDisplayParams()->GetFillDisplay ();

    if (isFilled)
        info |= DISPLAY_INFO_Fill;

    if (renderable)
        {
        info |= DISPLAY_INFO_Surface;

        if (isFilled && _CheckFillOutline ())
            info |= DISPLAY_INFO_Edge;

        return info;
        }
    else
        {
        info |= DISPLAY_INFO_Edge;
        }

    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QvElem* _DrawCached (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32 qvIndex) override
    {
#if defined (WIP_NEW_CACHE)
    //  If we need this we have some work to do.
    ElementId               symbolId;
    double                  pixelSize = 0.0;            // Needs work. Compute.

    if (qvIndex < 0 && 
        (symbolId = XGraphicsSymbolCache::GetSymbolElementId (elHandle, -qvIndex, stroker, pixelSize)).IsValid())
        {
        GetOutput ().m_writer.WriteSymbol (symbolId, Transform::FromIdentity());
        return NULL;
        }
else
    if (qvIndex < 0)
        {
        BeAssert(qvIndex >= 0);
        }
#endif

    GetOutput ().SetUseCache (true);
    stroker._StrokeForCache (dh, *this);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawCurveVector (ElementHandleCR eh, ICurvePathQueryR query, GeomRepresentations info, bool allowCachedOutline) override
    {
    if (0 == (info & DISPLAY_INFO_Fill) || NoDisplayFilters())
        return T_Super::_DrawCurveVector (eh, query, info, allowCachedOutline);

    bvector <byte>          fillOrRenderedTest, wireframeTest;
    bool                    doDisplayEdge = 0 != (info & DISPLAY_INFO_Edge);

    if (!doDisplayEdge && FillDisplay::Always == m_currDisplayParams.GetFillDisplay())
        {
        // Special case  - fill always on and no outline - no branching required.
        T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Fill, allowCachedOutline);
        }
    else
        {
        DisplayFilter::If (*this, &eh, *DisplayFilter::CreateRenderModeTest (MSRenderMode::Wireframe, DisplayFilter::TestMode_GreaterThan));
            {
            T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Fill, allowCachedOutline);
            }
        DisplayFilter::Else (*this);
            {
            if (doDisplayEdge)
                {
                DisplayFilter::If (*this, &eh, *DisplayFilter::CreateViewFlagTest (DisplayFilter::ViewFlag_Fill, true));
                    T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Fill, allowCachedOutline);
                DisplayFilter::End(*this);
                T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Edge, allowCachedOutline);
                }
            else
                {
                DisplayFilter::If (*this, &eh, *DisplayFilter::CreateViewFlagTest (DisplayFilter::ViewFlag_Fill, true));
                                T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Fill, allowCachedOutline);
                DisplayFilter::Else (*this);
                                T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Edge, allowCachedOutline);
                DisplayFilter::End(*this);
                }
            }
        DisplayFilter::End(*this);
        }

    T_Super::_DrawCurveVector (eh, query, DISPLAY_INFO_Pattern, allowCachedOutline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PushBranchDisplayParams ()
    {
    if (NULL == GetCurrentDisplayParams ())
        return;

    m_branchDisplayParams.push_back (*GetCurrentDisplayParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PopBranchDisplayParams ()
    {
    if (NULL == GetCurrentDisplayParams ())
        return;
    
    if (m_branchDisplayParams.empty())
        {
        BeAssert (false);
        return;
        }

    if (! (m_branchDisplayParams.back() == *GetCurrentDisplayParams()))
        {
        *GetCurrentDisplayParams () = m_branchDisplayParams.back();
        CookDisplayParams ();
        }
        

    m_branchDisplayParams.pop_back ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _IfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize) override
    {
    if (NoDisplayFilters())        
        return true;               // If we are omitting display filters, arbitrarily take the "if" branch.

    PushBranchDisplayParams ();

    GetOutput ().m_writer.BeginOperation (XGRAPHIC_OpCode_IfConditionalDraw);
    GetOutput ().m_writer.BeginConditionalDraw ();
    GetOutput ().m_writer.WriteDisplayFilterHandlerId (filterId);
    GetOutput ().m_writer.WriteData (data, dataSize);
    GetOutput ().m_writer.EndOperation ();

    return true;           // Never filter...always collect so we can apply conditions at display time.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ElseIfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize) override
    {
    if (NoDisplayFilters())        
        return false;               // If we are omitting display filters, arbitrarily discard the "else" branch.

    PopBranchDisplayParams ();
    PushBranchDisplayParams (); 

    GetOutput ().m_writer.EndConditionalDraw ();
    GetOutput ().m_writer.BeginOperation (XGRAPHIC_OpCode_ElseIfConditionalDraw);
    GetOutput ().m_writer.BeginConditionalDraw ();
    GetOutput ().m_writer.WriteDisplayFilterHandlerId (filterId);
    GetOutput ().m_writer.WriteData (data, dataSize);
    GetOutput ().m_writer.EndOperation ();

    return true;           // Never filter...always collect so we can apply conditions at display time.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ElseConditionalDraw () override
    {
    if (NoDisplayFilters())        
        return false;               // If we are omitting display filters, arbitrarily discard the "else" branch.

    PopBranchDisplayParams ();
    PushBranchDisplayParams (); 

    GetOutput ().m_writer.EndConditionalDraw ();
    GetOutput ().m_writer.BeginOperation (XGRAPHIC_OpCode_ElseConditionalDraw);
    GetOutput ().m_writer.BeginConditionalDraw ();
    GetOutput ().m_writer.EndOperation ();

    return true;           // Never filter...always collect so we can apply conditions at display time.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _EndConditionalDraw () override
    {
    if (NoDisplayFilters())        
        return;

    PopBranchDisplayParams ();

    GetOutput ().m_writer.EndConditionalDraw ();
    GetOutput ().m_writer.BeginOperation (XGRAPHIC_OpCode_EndConditionalDraw);
    GetOutput ().m_writer.EndOperation ();
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsContext (XGraphicsContainer& container, UInt32 createOptions = XGRAPHIC_CreateOptions_None, UInt32 optimizeOptions = XGRAPHIC_OptimizeOptions_None) : m_output (container, createOptions, optimizeOptions)
    {
    m_purpose = DrawPurpose::XGraphicsCreate;
    m_fillOutlineThreshold = 1;

    _GetOutput ().SetViewContext (this); 
    _SetupOutputs ();

    m_removeSymbols = false; // #ifdef NEEDS_WORK_TopazMerge_ -- do we still use m_removeSymbools?
    m_displayPriorityRange[0] = -MAX_HW_DISPLAYPRIORITY;
    m_displayPriorityRange[1] = MAX_HW_DISPLAYPRIORITY;
    m_levelClassMask.classMask = 0x7fff;
    m_wantMaterials = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteAnnotationData (XGraphicsAnnotationData const& annotationData)
    {
    GetOutput ().WriteAnnotationData (annotationData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnableInitialWeightMatSym ()
    {
    GetOutput ().EnableInitialWeightMatSym ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnableInitialColorMatSym ()
    {
    GetOutput ().EnableInitialColorMatSym ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnableInitialLineCodeMatSym ()
    {
    GetOutput ().EnableInitialLineCodeMatSym ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAreaPattern (ElementHandleCR thisElm, ClipStencil& boundary, PatternParamSource& source) override
    {
    if (GetOutput ().m_createOptions & XGRAPHIC_CreateOptions_StrokeAreaPatterns)
        T_Super::_DrawAreaPattern (thisElm, boundary, source);
    else 
        GetOutput ()._DrawAreaPattern (thisElm, boundary, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAligned (DVec3dCR axis, DPoint3dCR origin, AlignmentMode alignmentMode, IStrokeAligned& stroker) override
    {
    GetOutput ().DrawAligned (axis, origin, alignmentMode, stroker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _SetLocatePriority (int priority) override { GetOutput ().SetLocatePriority (priority); }        
virtual void _SetNonSnappable (bool unsnappable) override { GetOutput ().SetNonSnappable (unsnappable); }

}; // XGraphicsContext

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/13
//---------------------------------------------------------------------------------------
StatusInt XGraphicsDrawSymbol::_Optimize (XGraphicsContainerR optimizedData, byte*& pData, UInt32 opSize, byte* pEnd, XGraphicsOptimizeContextR optimizeContext)
    {
    BeAssert(sizeof(XGraphicsHeader) == 2);
    bool    removeSymbols = 0 != (optimizeContext.m_options & XGRAPHIC_OptimizeOptions_RemoveSymbols);
    if (!removeSymbols || NULL == optimizedData.GetElementRef ())
        return ERROR;

    XGraphicsHeader header = optimizedData.GetHeader();
    ElementHandle eh (optimizedData.GetElementRef());
    XGraphicsOperationContext opContext (&header, *optimizedData.GetElementRef ()->GetDgnProject (), optimizedData.GetSymbols(), &eh);

    XGraphicsContext context (optimizedData, XGRAPHIC_CreateOptions_None, optimizeContext.m_options);
    context.SetRemoveSymbols (removeSymbols);
    context.SetDgnProject (*optimizedData.GetElementRef ()->GetDgnProject ());

    optimizedData.BeginDraw();

    context.CookElemDisplayParams (eh);

    opSize -= sizeof (opSize);

    _Draw (context, pData, opSize, opContext);

    pData += opSize;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    GeorgeDulchinos 09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::ValidateBuffer ()
    {
    for (byte *pData = &m_buffer[0] + sizeof(XGraphicsHeader), *pEnd = &m_buffer[0] + m_buffer.size(); pData < pEnd;)
        {
        UInt16  opCode;
        UInt32  opSize;

        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            {
            BeAssert (false);
            return ERROR;
            }

        pData += (opSize - sizeof(opSize));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsContainer::Write (void const* data, size_t dataSize)
    {
    m_buffer.Append (data, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsContainer::BeginDraw ()
    {
    m_buffer.clear ();

    Write (&m_header, sizeof (m_header));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::EndDraw ()
    {
    if (IsEmpty ())
        return ERROR;           // Nothing saved.

    // Update header in case the flags were changed during the draw (m_useCache, m_cannotUseForStencil).
    memcpy (&m_buffer[0], &m_header, sizeof (m_header));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XGraphicsContainer::IsEmpty () const
    {
    return GetDataSize() <= sizeof(m_header);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsContainer::Dump (DgnModelP model) const
    {
    printf ("Total Size: %d\n", (int)m_buffer.size());
    XGraphicsOperationContext xcontext ((XGraphicsHeader*) &m_buffer[0], model->GetDgnProject (), const_cast <T_XGraphicsSymbolIdsR> (m_symbolIds), NULL);
    XGraphicsDumpOperator dump (xcontext, &m_buffer[0] + sizeof(XGraphicsHeader));
    XGraphicsOperations::Traverse (const_cast <XGraphicsDataR> (m_buffer), dump);
    printf ("\n*************\n");
    }

/*=================================================================================**//**
bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct XGraphicsTestGraphicsPresentOperator : XGraphicsOperator
{
    XGraphicsTestGraphicsPresentOperator() : m_graphicsPresent (false) { }

    bool m_graphicsPresent;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       _DoOperation (XGraphicsOperationR operation, byte* pData, UInt32 size, byte* pEnd, XGraphicsOpCodes opCode)
    {
    switch (opCode)
        {
        case XGRAPHIC_OpCode_PushTransClip:     
        case XGRAPHIC_OpCode_PopTransClip:      
        case XGRAPHIC_OpCode_PopAll: 
        case XGRAPHIC_OpCode_MatSymb:
        case XGRAPHIC_OpCode_MatSymb2:
        case XGRAPHIC_OpCode_AnnotationData:             
             return SUCCESS;

        default:
            m_graphicsPresent = true;
            return ERROR;
        }
    }

};  // XGraphicsTestGraphicsPresentOperator

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool    XGraphicsContainer::ContainsGraphics () 
    {
    XGraphicsTestGraphicsPresentOperator        testGraphicsOperator;

    XGraphicsOperations::Traverse (m_buffer, testGraphicsOperator);
    
    return testGraphicsOperator.m_graphicsPresent;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::OnTransform (TransformInfoCR transform)
    {
    StatusInt       status;
    XGraphicsData   transformedContainerData;

    if (SUCCESS != (status = transformXGraphicsData (transformedContainerData, &m_buffer[0] + sizeof(XGraphicsHeader), &m_buffer[0] + m_buffer.size(), transform)))
        return status;
    
    m_buffer.resize (transformedContainerData.size() + sizeof (XGraphicsHeader));
    memcpy (GetData() + sizeof (XGraphicsHeader), &transformedContainerData[0], transformedContainerData.size());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::CompressTransforms ()
    {
    bool                    compressed = false;
    bvector<Transform>      transforms;
    XGraphicsData           transformedContainerData, transformedOperationData;

    for (byte *pData = &m_buffer[0] + sizeof(XGraphicsHeader), *pEnd = &m_buffer[0] + m_buffer.size(); pData < pEnd; )
        {
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (SUCCESS != (status = XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd)))
            return status;

        switch (opCode)
            {
            case XGRAPHIC_OpCode_PushTransClip:
                {
                Transform       transform;
       
                memcpy (&transform, pData, sizeof (transform));
                if (!transforms.empty())
                    transform.InitProduct (transform, transforms.back());

                transforms.push_back (transform);
                break;
                }

            case XGRAPHIC_OpCode_PopTransClip:
                transforms.pop_back ();
                break;

            case XGRAPHIC_OpCode_PopAll:
                transforms.clear();
                break;

            default:
                {
                transformedContainerData.Append (&opCode, sizeof (opCode));
                if (transforms.empty())
                    {
                    transformedContainerData.Append (&opSize, sizeof (opSize));
                    transformedOperationData.Append (pData, opSize - sizeof (opSize));
                    }
                else
                    {
                    compressed = true;
                    transformedOperationData.clear ();
                    transformedOperationData.Append (pData, opSize - sizeof (opSize));

                    if (SUCCESS != (status = XGraphicsOperations::OnTransform (opCode, TransformInfo (transforms.back()), transformedOperationData)))
                        return status;
        
                    UInt32  newOpSize = (UInt32)(transformedOperationData.size() + sizeof (newOpSize));

                    transformedContainerData.Append (&newOpSize, sizeof (newOpSize));
                    transformedContainerData.Append (&transformedOperationData[0], transformedOperationData.size());
                    }
                break;
                }
            }

        pData += (opSize - sizeof (opSize));
        }

    if (compressed)
        {
        m_buffer.resize (transformedContainerData.size() + sizeof (XGraphicsHeader));
        memcpy (GetData() + sizeof (XGraphicsHeader), &transformedContainerData[0], transformedContainerData.size());
        }


    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::GetBasisTransform (TransformR transform)
    {
    return XGraphicsOperations::GetBasisTransform (transform, &m_buffer[0] + sizeof(XGraphicsHeader), &m_buffer[0] + m_buffer.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsContainer::IsEqual (XGraphicsContainer const& rhs, double distanceTolerance) const
    {
    if (m_buffer.size() != rhs.m_buffer.size())
        return false;

    for (byte const *thisData = &m_buffer[0] + sizeof (XGraphicsHeader), *rhsData = &rhs.m_buffer[0] + sizeof (XGraphicsHeader), *end = &m_buffer[0] + m_buffer.size(); thisData < end; )
        {
        UInt16  thisOpCode, rhsOpCode;
        UInt32  thisOpSize, rhsOpSize;

        GET_AND_INCREMENT_DATA (thisOpCode, thisData);
        GET_AND_INCREMENT_DATA (thisOpSize, thisData);

        GET_AND_INCREMENT_DATA (rhsOpCode, rhsData);
        GET_AND_INCREMENT_DATA (rhsOpSize, rhsData);

        if (thisOpCode != rhsOpCode || thisOpSize != rhsOpSize)
            return false;

        if (thisOpCode < 0 || thisOpCode >= MAX_XGraphicsOpCode ||
            (thisOpSize - sizeof(thisOpSize)) > (UInt32) (end - thisData))       // op data must not exceed data remaining (note: opSize includes itself)
            {
            BeAssert (false && "Invalid XGraphics");
            return false;
            }

        thisOpSize -= sizeof (thisOpSize);

        if (!XGraphicsOperations::IsEqual (thisOpCode, thisData, rhsData, thisOpSize, distanceTolerance))
            return false;

        thisData += thisOpSize;
        rhsData  += thisOpSize;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsContainer::ProcessProperties (PropertyContextR context)
    {
    struct Processor : IProcessOperations
        {
        PropertyContextR m_context;

        Processor (PropertyContextR context) : m_context (context) {}

        virtual void _ProcessOperation (UInt16 opcode, byte* data, UInt32 dataSize) override
            {
            XGraphicsOperations::ProcessProperties (opcode, data, dataSize, m_context);
            }
        };

    Processor processor (context);

    ProcessOperations (processor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/13
+---------------+---------------+---------------+---------------+---------------+------*/
//bool XGraphicsContainer::DeepCopyRoots (ElementCopyContextR context)
//    {
//    struct Processor : IProcessOperations
//        {
//        bool                m_changed;
//        ElementCopyContextR m_context;
//
//        Processor (ElementCopyContextR context) : m_context (context), m_changed (false) {}
//
//        virtual void _ProcessOperation (UInt16 opcode, byte* data, UInt32 dataSize) override
//            {
//            m_changed |= XGraphicsOperations::DeepCopyRoots (opcode, data, dataSize, m_context);
//            }
//        };
//
//    Processor processor (context);
//
//    ProcessOperations (processor);
//
//    return processor.m_changed;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsContainer::ProcessOperations (IProcessOperations& processor)
    {
    for (byte *thisData = &m_buffer[0] + sizeof (XGraphicsHeader), *end = &m_buffer[0] + m_buffer.size(); thisData < end; )
        {
        UInt16  thisOpCode;
        UInt32  thisOpSize;

        GET_AND_INCREMENT_DATA (thisOpCode, thisData);
        GET_AND_INCREMENT_DATA (thisOpSize, thisData);

        if (thisOpCode < 0 || thisOpCode >= MAX_XGraphicsOpCode || (thisOpSize - sizeof(thisOpSize)) > (UInt32) (end - thisData)) // op data must not exceed data remaining (note: opSize includes itself)
            {
            BeAssert (false && "Invalid XGraphics");

            return;
            }

        thisOpSize -= sizeof (thisOpSize);

        processor._ProcessOperation (thisOpCode, thisData, thisOpSize);

        thisData += thisOpSize;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     flushMeshCache (XGraphicsContainerR container, byte*& cacheStart, byte*& cacheEnd, PolyfaceCoordinateMap & facetMap)
    {
    bool            meshCreated = false;
    static size_t   s_minMeshFacetCount = 5;
    PolyfaceHeader & polyface = facetMap.GetPolyfaceHeaderR ();
    if (false != (meshCreated = polyface.GetNumFacet () > s_minMeshFacetCount))
        {
        XGraphicsContainer      tailData;
        byte*                   containerEnd = container.GetData() + container.GetDataSize();

        tailData.BeginDraw ();
        if (cacheEnd < containerEnd)
            tailData.Write (cacheEnd, containerEnd - cacheEnd);

        container.Resize (cacheStart - container.GetData());

        XGraphicsWriter         writer (&container);

        writer.WritePolyface (polyface, true);

        size_t          cacheEndIndex =  container.GetDataSize();
        writer.AppendContainer (tailData);

        cacheEnd = container.GetData() + cacheEndIndex;
        }

    cacheStart = NULL;
    facetMap.ClearData ();
    return meshCreated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::CreateMeshFromShapes ()
    {
    PolyfaceHeaderPtr headerPtr = PolyfaceHeader::New ();
    PolyfaceCoordinateMapPtr polyface = PolyfaceCoordinateMap::New (*headerPtr);

    byte*       cacheStart = NULL, *pData, *pEnd;
    bool        meshCreated = false;
    int         complexCount = 0, conditionalDrawCount = 0;
    bool        isFilled = false;

    for (pData = &m_buffer[0] + sizeof(XGraphicsHeader), pEnd = &m_buffer[0] + m_buffer.size(); pData < pEnd;)
        {
        byte*       pOpStart = pData;
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (SUCCESS != (status = XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd)))
            return status;

        switch (opCode)
            {
            case XGRAPHIC_OpCode_MatSymb:
                {
                UInt16 matSymbMask;

                memcpy (&matSymbMask, pData, sizeof (matSymbMask));

                if (0 != (matSymbMask & XMATSYMB_ElemFillColor))
                    isFilled = true;
                break;
                }

            case XGRAPHIC_OpCode_MatSymb2:
                {
                UInt32 matSymbMask;

                memcpy (&matSymbMask, pData, sizeof (matSymbMask));

                if (0 != (matSymbMask & XMATSYMB2_GradientFill))
                    isFilled = true;
                break;
                }

            case XGRAPHIC_OpCode_BeginComplexString:
            case XGRAPHIC_OpCode_BeginComplexShape:
                {
                meshCreated |= flushMeshCache (*this, cacheStart, pOpStart, *polyface);
                pData = pOpStart + sizeof (opSize) + sizeof (opCode);
                pEnd = &m_buffer[0] + m_buffer.size();
                complexCount++;
                break;
                }

            case XGRAPHIC_OpCode_EndComplex:
                {
                complexCount--;
                break;
                }

            case XGRAPHIC_OpCode_IfConditionalDraw:
                {
                meshCreated |= flushMeshCache (*this, cacheStart, pOpStart, *polyface);
                pData = pOpStart + sizeof (opSize) + sizeof (opCode);
                pEnd = &m_buffer[0] + m_buffer.size();
                complexCount++;
                conditionalDrawCount++;
                break;
                }
            
            case XGRAPHIC_OpCode_EndConditionalDraw:
                {
                conditionalDrawCount--;
                break;
                }

            case XGRAPHIC_OpCode_DrawShape3d:
                {
                if (isFilled || 0 != complexCount || 0 != conditionalDrawCount)
                    break;

                XGraphicsDrawShape3d::AddPolygon (*polyface, pData, opSize);

                if (NULL == cacheStart)
                    cacheStart = pOpStart;
                break;
                }

            default:
                {
                meshCreated |= flushMeshCache (*this, cacheStart, pOpStart, *polyface);
                pData = pOpStart + sizeof (opSize) + sizeof (opCode);
                pEnd = &m_buffer[0] + m_buffer.size();
                break;
                }
            }

        pData += (opSize - sizeof (opSize));
        }

    meshCreated |= flushMeshCache (*this, cacheStart,pEnd, *polyface);

    return meshCreated ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::Optimize (UInt32 options)
    {
#ifndef NDEBUG
    static bool     s_dump = false;

    if (s_dump)
        {
        printf ("PreOptimize:\n");
        Dump (NULL);
        }
#endif

    bool        anyOptimized = false;

    if (0 != (options & XGRAPHIC_OptimizeOptions_MeshFromShapes) && SUCCESS == CreateMeshFromShapes ())
        anyOptimized = true;

    XGraphicsData       optimizedData;
    optimizedData.reserve (GetDataSize ());

    XGraphicsOptimizeContext    context (options);
    DgnProgressMeterP           meter = T_HOST.GetProgressMeter();

    for (byte *pBase = &m_buffer[0] + sizeof(XGraphicsHeader), *pData = pBase, *pEnd = &m_buffer[0] + m_buffer.size(); pData < pEnd;)
        {
        if (meter && DgnProgressMeter::ABORT_No != meter->ShowProgress())
            return ERROR;

        byte*       pOpStart = pData;
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        context.m_inputLocation  = pData - pBase;
        context.m_outputLocation = optimizedData.size();
        context.RemapBranches (optimizedData);

        if (SUCCESS != (status = XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd)))
            return status;

        context.m_inputLocation  += sizeof (opCode) + sizeof (opSize);
        context.m_outputLocation += sizeof (opCode) + sizeof (opSize);

        XGraphicsContainer optimizedContainer;

        // Define these for _RemoveSymbols optimization.
        optimizedContainer.m_symbolIds = m_symbolIds;
        optimizedContainer.m_elementRef = m_elementRef;

        if (SUCCESS == XGraphicsOperations::Optimize (optimizedContainer, opCode, pData, opSize, pEnd, context))
            {
            optimizedData.Append (optimizedContainer.GetData() + sizeof (XGraphicsHeader), optimizedContainer.GetDataSize() - sizeof (XGraphicsHeader));
            anyOptimized = true;
            }
        else
            {
            optimizedData.Append (pOpStart, opSize + sizeof (opCode));
            pData = pOpStart + opSize  + sizeof (opCode);
            }
        }

    if (anyOptimized)
        {
        m_buffer.resize (optimizedData.size() + sizeof (XGraphicsHeader));
        memcpy (GetData() + sizeof (XGraphicsHeader), &optimizedData[0], optimizedData.size());

        if (0 != (options & XGRAPHIC_OptimizeOptions_RemoveSymbols))
            m_symbolIds.clear();
        }

#ifndef NDEBUG
    if (s_dump)
        {
        printf ("PostOptimize:\n");
        Dump (NULL);
        }
#endif

    return anyOptimized ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsOptimizeContext::RemapBranches (XGraphicsData& optimizedData)
    {
    T_InputDestToOutputBranch::iterator   found = m_branches.find (m_inputLocation);

    if (found != m_branches.end())
        {
        UInt32      outputDelta = (UInt32) (m_outputLocation - found->second);
        memcpy (&optimizedData[found->second], &outputDelta, sizeof (outputDelta));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::CreateFromElementAgenda (ElementAgendaCR agenda)
    {
    BeginDraw();
    SetUseCache (true);

    bool first = true;

    for (auto& elIter : agenda)
        {
        XGraphicsCreateOptions options = XGRAPHIC_CreateOptions_RemoveSymbols;
        if (!first)
            options = (XGraphicsCreateOptions) (options | XGRAPHIC_CreateOptions_ForceSymbologyInclusion);

        XGraphicsContainer childContainer;
//  #ifdef NEEDS_WORK_TopazMerge_XGRAPHIC_OptimizeOptions_All -- verify this selects the desired options
        if (SUCCESS != childContainer.CreateFromElement (elIter, XGRAPHIC_OptimizeOptions_Default, options))
            continue;

        if (childContainer.IsRenderable())
            SetIsRenderable (true);

        first = false;
        size_t oldSize = GetDataSize(), childSize = childContainer.GetDataSize() - sizeof (XGraphicsHeader);
        m_buffer.resize (oldSize + childSize);
        memcpy (GetData() + oldSize, childContainer.GetData() + sizeof (XGraphicsHeader), childSize);
        }

    EndDraw();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::CreateFromElement (ElementHandleCR element, UInt32 optimizeOptions, UInt32 createOptions, XGraphicsSymbolCacheP symbolCache)
    {
    bool    forceSymbologyInclusion = 0 != (createOptions & (XGRAPHIC_CreateOptions_ForceSymbologyInclusion)),
            removeSymbols           = 0 != (createOptions & (XGRAPHIC_CreateOptions_RemoveSymbols)),
            createSymbols           = 0 != (optimizeOptions & (XGRAPHIC_OptimizeOptions_CreateSymbols));

    if (!createSymbols && !removeSymbols && !forceSymbologyInclusion && SUCCESS == ExtractFromElement (element))
        {
        Optimize (optimizeOptions);
        return SUCCESS;
        }

    XGraphicsContext context (*this, createOptions, optimizeOptions);
    context.GetOutput().SetForceSymbologyInclusion (forceSymbologyInclusion);
    context.SetRemoveSymbols (removeSymbols);

    BeginDraw ();

    context.SetDgnProject (*element.GetDgnProject ());

    if (!removeSymbols && NULL != symbolCache)
       context.BeginSymbol ();

    context.VisitElemHandle (element, false, false);

    if (0 != (createOptions & XGRAPHIC_CreateOptions_ViewIndependentOnly) && context.GetOutput ().ViewDependentPresent() ||
        0 != (createOptions & XGRAPHIC_CreateOptions_NoCustomLineStyles) && context.GetOutput ().CustomLineStylePresent())
        {
        if (!removeSymbols && NULL != symbolCache)
            context.EndSymbolDiscard ();

        return ERROR;
        }

    if (!removeSymbols && NULL != symbolCache)
        context.EndSymbol (element.GetDgnModelP());

    return EndDraw ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::CreateFromStroker (IStrokeForCache& stroker, ElementHandleCR element, double pixelSize, UInt32 optimizeOptions)
    {
    XGraphicsContext context (*this, optimizeOptions);

    BeginDraw ();

    context.SetDgnProject (*element.GetDgnProject ());
    context.CookElemDisplayParams (element);

    stroker._StrokeForCache (CachedDrawHandle(&element), context, pixelSize);

    return EndDraw ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
IViewDrawP      XGraphicsContainer::ConnectToViewDraw (ViewContextR viewContext, UInt32 createOptions, UInt32 optimizeOptions)
    {
    XGraphicsOutput*    viewDraw = new XGraphicsOutput (*this, createOptions, optimizeOptions);

    viewDraw->SetViewContext (&viewContext);
    return viewDraw;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsContainer::DeleteViewDraw (IViewDrawP viewDraw)
    {
    XGraphicsOutput*        output;

    if (NULL != (output = dynamic_cast <XGraphicsOutput*> (viewDraw)))
        delete output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsRecorder::XGraphicsRecorder (DgnModelP root)
    {
    m_context = new XGraphicsContext (m_container, XGRAPHIC_CreateOptions_None, XGRAPHIC_OptimizeOptions_None);

    m_context->SetDgnProject (root->GetDgnProject ());

    Reset ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsRecorder::~XGraphicsRecorder ()
    {
    if (NULL != m_context)
        delete m_context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContextP XGraphicsRecorder::GetContext () const
    {
    return m_context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsRecorder::EnableInitialWeightMatSym ()
    {
    m_context->EnableInitialWeightMatSym ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsRecorder::EnableInitialColorMatSym ()
    {
    m_context->EnableInitialColorMatSym ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsRecorder::EnableInitialLineCodeMatSym ()
    {
    m_context->EnableInitialLineCodeMatSym ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsRecorder::Reset ()
    {
    m_container.BeginDraw ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsRecorder::SetElemDisplayParams (ElemDisplayParamsCP elemDisplayParams)
    {
    m_context->SetElemDisplayParams (elemDisplayParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2012
//--------------+------------------------------------------------------------------------
void XGraphicsRecorder::SetFillOutlineThreshold (UInt32 threshold)
    {
    m_context->SetFillOutlineThreshold (threshold);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsRecorder::WriteAnnotationData (XGraphicsAnnotationData const& annotationData)
    {
    m_context->WriteAnnotationData (annotationData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::ExtractFromElement (ElementHandleCR element, XGraphicsMinorId minorId)
    {
    ElementHandle::XAttributeIter iterator (element, XAttributeHandlerId (XATTRIBUTEID_XGraphics, (UInt16)minorId), 0);

    if (iterator.IsValid() && iterator.GetSize() > sizeof (m_header))
        {
        memcpy (&m_header, iterator.PeekData(), sizeof (m_header));

        Write (iterator.PeekData(), iterator.GetSize());
        XGraphicsSymbolCache::ExtractSymbolIds (m_symbolIds, element);

        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsContainer::ExtractSymbolIdsFromElement (ElementHandleCR element)
    {
    XGraphicsSymbolCache::ExtractSymbolIds (m_symbolIds, element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::AddToElement (EditElementHandleR eh, XGraphicsMinorId minorId)
    {
    EndDraw ();

    if (0 == GetDataSize ())
        return ERROR;

    BRepWireGraphicsAppData::ClearWireGeomCache (eh.GetElementRef());
    if (SUCCESS != eh.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_XGraphics, (UInt16)minorId), 0, GetDataSize(), &m_buffer[0]))
        return ERROR;

    if (0 == m_symbolIds.size ())
        return SUCCESS;

    return AddSymbolIdsToElement (eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::OnWriteToElement (ElementHandleCR eh)
    {
    struct Processor : IProcessOperations
        {
        ElementHandleCR         m_elem;
        StatusInt               m_result;

        Processor (ElementHandleCR eh) : m_elem(eh), m_result(SUCCESS) { }
    
        virtual void    _ProcessOperation (UInt16 opcode, byte* data, UInt32 size) override
            {
            if (SUCCESS == m_result)
                m_result = XGraphicsOperations::OnWriteToElement (opcode, data, size, m_elem);
            }
        };

    Processor processor (eh);
    ProcessOperations (processor);
    return processor.m_result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::AddSymbolIdsToElement (EditElementHandleR eh)
    {
    if (m_symbolIds.size() == 0)
        {
        DgnElementP ele = (DgnElementP)_alloca(eh.GetElementCP()->Size());
        eh.GetElementCP()->CopyTo(*ele);
        elemUtil_deleteLinkage(ele, LINKAGEID_StampSymbolIds);
        eh.ReplaceElement(ele);
        return BSISUCCESS;
        }

    size_t linkageSize = SymbolIdsLinkageData::GetLinkageSize(m_symbolIds);
    SymbolIdsLinkageData* data = (SymbolIdsLinkageData*)_alloca(linkageSize + 4);
    data->m_key = STAMPID_LINKAGE_KEY_SymbolIdMap;
    data->m_padding = 0;
    data->m_count = (UInt32)m_symbolIds.size();
    memcpy(data->m_ids, &m_symbolIds[0], sizeof data->m_ids[0] * m_symbolIds.size());

    LinkageHeader   linkHdr;

    ElementLinkageUtil::InitLinkageHeader (linkHdr, LINKAGEID_StampSymbolIds, linkageSize);  //  don't know if the size is supposed to contain the header
    size_t oldElemSize = eh.GetElementCP()->Size();
    size_t newElemSize = oldElemSize + LinkageUtil::GetWords(&linkHdr) * 2;

    DgnElementP ele = (DgnElementP)_alloca(newElemSize);
    eh.GetElementCP()->CopyTo(*ele);
    elemUtil_deleteLinkage(ele, LINKAGEID_StampSymbolIds);
    linkage_appendToElement(ele, &linkHdr, data, NULL);

    eh.ReplaceElement(ele);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::AddInstance (XGraphicsContainerR definition, EditElementHandleR eh, TransformCR transform, Int32 symbolId, XGraphicsMinorId  minorId)
    {
    BeAssert(eh.GetDgnModelP() != NULL);
    XGraphicsSymbolCacheR       symbolCache = XGraphicsSymbolCache::Get (eh.GetDgnModelP()->GetDgnProject());

    //  Topaz does not pass in the transform. It saves a pointer to the transform.  Before taking that change
    //  I have to understand the lifetime of the transform.
    XGraphicsSymbolP  symbol = new XGraphicsSymbol (definition, NULL, NULL), foundSymbol;

    if (NULL == (foundSymbol = symbolCache.FindOrAddMatchingSymbol (XGraphicsSymbolId (eh, symbolId), symbol)))
        return ERROR;

    if (foundSymbol != symbol)
        delete symbol;
    
    XGraphicsWriter (this).WriteSymbol (foundSymbol->GetParentId(), transform);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::ReplaceOnElement (EditElementHandleR element, XGraphicsMinorId minorId)
    {
    StatusInt status = RemoveFromElement (element, minorId);
    if (SUCCESS != status)
        return status;

    return AddToElement (element, minorId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::RemoveFromElement (EditElementHandleR element, XGraphicsMinorId minorId)
    {
    StatusInt retval = element.ScheduleDeleteXAttribute (XAttributeHandlerId (XATTRIBUTEID_XGraphics, (UInt16)minorId), 0);
    if (BSISUCCESS != retval)
        return retval;

    //  #ifdef NEEDS_WORK_TopazMerge_RemoveFromElement // Topaz doesn't do the DeleteLinkage
#if defined (NEEDS_WORK_DGNITEM)
    DependencyManagerLinkage::DeleteLinkage (element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_XGraphicsSymbol);
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool       XGraphicsContainer::IsPresent (ElementHandleCR element, XGraphicsMinorId minorId)
    {
    ElementHandle::XAttributeIter iterator (element, XAttributeHandlerId (XATTRIBUTEID_XGraphics, (UInt16)minorId), 0);

    return iterator.IsValid() && iterator.GetSize() > sizeof (XGraphicsHeader);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
StatusInt       XGraphicsContainer::DrawSymbolFromStamp (ViewContextR context, XGraphicsSymbolStamp& symbolStamp, XGraphicsOperationContextR opContext, TransformCR transform)
    {
    UInt32 streamSize;
    byte const*stampXGraphics = (byte const*)symbolStamp.GetXGraphicStream(streamSize);

    if (NULL == stampXGraphics || streamSize < sizeof (XGraphicsHeader))
        return ERROR;

#if defined(SUPPORTING_NESTED_SYMBOLS_IN_STAMPS)
    T_XGraphicsSymbolIds    symbolIds;

    XGraphicsSymbolCache::ExtractSymbolIds (symbolIds, symbolStamp);

    AutoRestore <T_XGraphicsSymbolIds>  saveSymbolIds (&opContext.m_symbolIds, symbolIds);
#endif

    context.PushTransform (transform);

    CachedDrawHandle dh(symbolStamp);
    XGraphicsCacheStroker::DrawOrCacheFromMemory (context, dh, 0, stampXGraphics + sizeof (XGraphicsHeader), stampXGraphics + streamSize, opContext);

    context.PopTransformClip ();

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::DrawSymbol (ViewContextR context, ElementRefP symbolElemRef, XGraphicsOperationContextR opContext, TransformCR transform)
    {
    XAttributeHandle  iterator (symbolElemRef, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0);

    if (!iterator.IsValid () || iterator.GetSize () < sizeof (XGraphicsHeader))
        return ERROR;

    XAttributeHandle        symbolIterator (symbolElemRef, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_SymbolTransform), 0);
    EditElementHandle       symbolEeh (symbolElemRef);
    T_XGraphicsSymbolIds    symbolIds;

    XGraphicsSymbolCache::ExtractSymbolIds (symbolIds, symbolEeh);

    if (symbolIterator.IsValid () && symbolIterator.GetSize () >= sizeof (Transform))
        {
        // XGraphics symbols are type 107 so in order to create size dependent QvElems
        // create a temporary type 106 using the range as extracted from the inverse transform.
        DRange3d        range;
        Transform       inverseSymbol;

        inverseSymbol.InverseOf (*((TransformCP) symbolIterator.PeekData ()));
        inverseSymbol.GetTranslation (range.low);
        range.high = range.low;

        EditElementHandle   tmpEeh;
        
        // NEEDSWORK_V10: This is horrible...
        ExtendedElementHandler::InitializeElement (tmpEeh, NULL, *symbolEeh.GetDgnModelP (), true);
        tmpEeh.GetElementP ()->GetRangeR () = range;

        MSElementDescrPtr   tmpEdPtr = tmpEeh.ExtractElementDescr ();

        if (tmpEdPtr.IsValid ())
            tmpEdPtr.get ()->SetElementRef (symbolElemRef);

        symbolEeh.SetElementDescr (tmpEdPtr.get (), true);
        }

    void const*     pData       = iterator.PeekData();

    AutoRestore <T_XGraphicsSymbolIds>  saveSymbolIds (&opContext.m_symbolIds, symbolIds);
    AutoRestore <XGraphicsHeader*>      saveHeader (&opContext.m_header);

    opContext.m_header = (XGraphicsHeader*) pData;
    context.PushTransform (transform);
    
    XGraphicsCacheStroker::DrawOrCacheFromMemory (context, CachedDrawHandle(&symbolEeh), 0, (byte const*) iterator.PeekData () + sizeof (XGraphicsHeader), (byte const*) iterator.PeekData () + iterator.GetSize (), opContext);
    context.PopTransformClip ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::Draw (ViewContextR context, DrawOptions options)
    {
    XGraphicsOperationContext xcontext ((XGraphicsHeader*) GetData(), context.GetDgnProject (), m_symbolIds, NULL, options);

    //  Hold symbol definition elements in memory on referencing elements' app data.
    //  Reduces contention between query thread and drawing thread for DgnElementPool.
    xcontext.m_sourceElement = m_elementRef;

    return XGraphicsCacheStroker::DrawFromMemory (context, GetGraphicsData(), GetDataEnd(), xcontext, XGraphicsDrawOperator::DrawAll);
    }

struct XGOperationLocation
    {
    byte const* bufferStart;
    int         bufferSize;
    void Set (byte const* start, int size) {bufferStart = start; bufferSize = size;}
    XGOperationLocation() {Set (NULL, 0);}
    XGOperationLocation (byte const* start, int size) {Set (start, size);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    David.Assaf     12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::ExtractPrimitives (bvector<XGraphicsContainer>& containerVector) const
    {
    bvector<XGOperationLocation>    transformLocations;
    XGraphicsContainer                  xg;

    UInt32      opSize;
    UInt16      opCode;
    byte const* pData = &m_buffer.front();
    byte const* pEnd  = pData + m_buffer.size();
    byte const* pNext;
    byte const* pOpData;
    int         transformStack = 0;
    int         complexStack = 0;
    int         sweepStack = 0;
    bool        atStart = true;
    bool        needNextOp = false;
    bool        foundTransform = false;

    for (pData += sizeof (XGraphicsHeader); pData < pEnd; pData = pNext)
        {
        if (atStart)
            {
            // (re)initialize container with our header
            xg.m_header = m_header;
            xg.BeginDraw();
            }

        pOpData = pData;
        GET_AND_INCREMENT (opCode);
        GET_AND_INCREMENT (opSize);
        pNext = pData + (opSize - sizeof (opSize));     // (note: opSize includes itself)

        switch (opCode)
            {
            case XGRAPHIC_OpCode_PushTransClip:
                {
                foundTransform = true;
                transformStack++;
                break;
                }

            case XGRAPHIC_OpCode_PopTransClip:
                {
                foundTransform = true;
                transformStack--;
                break;
                }

            case XGRAPHIC_OpCode_PopAll:
                {
                foundTransform = true;
                transformStack = 0;
                break;
                }

            case XGRAPHIC_OpCode_AnnotationData:
            case XGRAPHIC_OpCode_MatSymb:
            case XGRAPHIC_OpCode_MatSymb2:
            case XGRAPHIC_OpCode_AddDisconnect:
                {
                // these operations make no sense by themselves
                needNextOp = true;
                break;
                }

            case XGRAPHIC_OpCode_BeginSweepProject:
            case XGRAPHIC_OpCode_BeginSweepExtrude:
            case XGRAPHIC_OpCode_BeginSweepRevolve:
                {
                sweepStack++;
                break;
                }

            case XGRAPHIC_OpCode_EndSweep:
                {
                sweepStack--;
                break;
                }

            case XGRAPHIC_OpCode_BeginComplexShape:
            case XGRAPHIC_OpCode_BeginComplexString:
                {
                complexStack++;
                break;
                }

            case XGRAPHIC_OpCode_EndComplex:
                {
                complexStack--;
                break;
                }
            }

        // start every container with all transforms found so far
        if (atStart)
            {
            for (bvector<XGOperationLocation>::const_iterator location = transformLocations.begin(); location != transformLocations.end(); location++)
                {
                xg.Write ((*location).bufferStart, (*location).bufferSize);
                }
            atStart = false;
            }

        // copy the op
        xg.Write (pOpData, static_cast<int>(pNext - pOpData));

        if (foundTransform)
            {
            transformLocations.push_back (XGOperationLocation (pOpData, static_cast<int>(pNext - pOpData)));
            foundTransform = false;
            }
        else if (needNextOp)
            {
            needNextOp = false;
            }
        else if (complexStack > 0 || sweepStack > 0)
            {
            }
        else
            {
            int localTransformStack = transformStack;

            // pop any pushed transforms
            while (localTransformStack > 0)
                {
                XGraphicsWriter writer (&xg);
                writer.WriteOperation (XGRAPHIC_OpCode_PopTransClip);
                localTransformStack--;
                }

            // close the container
            if (!localTransformStack)
                {
                xg.EndDraw();
                containerVector.push_back (xg);
                atStart = true;
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XGraphicsContainer::IsRenderable (ElementHandleCR eh)
    {
    ElementHandle::XAttributeIter iterator (eh, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0);

    return iterator.IsValid () && iterator.GetSize () >= sizeof (XGraphicsHeader) && ((XGraphicsHeader*) iterator.PeekData())->m_isRenderable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::Draw (ViewContextR context, ElementHandleCR eh, DrawOptions options, XGraphicsMinorId minorId)
    {
    T_XGraphicsSymbolIds symbolIds;
    ElementHandle::XAttributeIter iterator (eh, XAttributeHandlerId (XATTRIBUTEID_XGraphics, (UInt16)minorId), 0);
    if (!iterator.IsValid () || iterator.GetSize () <= sizeof (XGraphicsHeader))
        return ERROR;

    XGraphicsSymbolCache::ExtractSymbolIds (symbolIds, eh);

    return Draw (context, CachedDrawHandle(&eh), &iterator, symbolIds, options);
    }

//  Not in Topaz version
//  struct XGraphicsDrawBRepFaceIsoOperator : XGraphicsOperator
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::Draw (ViewContextR context, CachedDrawHandleCR dh, ElementHandle::XAttributeIter* iterator, T_XGraphicsSymbolIdsR symbolIds, DrawOptions options)
    {
    if (DrawPurpose::ChangedPre == context.GetDrawPurpose ())
        {
        BeAssert(false && "how does CachedDrawHandleCR fit into this");
        // NOTE: Some xGraphics (breps) could be expensive to draw; display range and heal...
        //  context.DrawElementRange (eh.GetElementCP ());

        return SUCCESS;
        }

    byte*               pData = (byte*) iterator->PeekData ();
    UInt32              dataSize = iterator->GetSize ();
    XGraphicsContext*   xgContext;

    if (DrawPurpose::XGraphicsCreate == context.GetDrawPurpose() && NULL != (xgContext = dynamic_cast <XGraphicsContext*> (&context)) && symbolIds.empty())
        {
        XGraphicsHeader saveHeader;

        memcpy (&saveHeader, pData, sizeof (saveHeader));
        // If we're trying to create XGraphics and already have them, simply push the existing graphics into the container.
        xgContext->GetOutput ().m_writer.WriteData (pData + sizeof (XGraphicsHeader), dataSize - sizeof (XGraphicsHeader));
        xgContext->GetOutput ().m_writer.ReplaceHeader (saveHeader);
        return SUCCESS;
        }

    XGraphicsOperationContext   opContext ((XGraphicsHeader*) pData, dh.GetDgnModelP ()->GetDgnProject (), symbolIds, dh.GetElementHandleCP(), options);
    opContext.m_sourceElement = dh.GetElementRef();
    return XGraphicsCacheStroker::DrawOrCacheFromMemory (context, dh, 0, pData + sizeof (XGraphicsHeader), pData + dataSize, opContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::DropSymbolInstance (EditElementHandleR eh, XGraphicsSymbolR symbol)
    {
    T_XGraphicsSymbolIds::iterator    currId = m_symbolIds.begin();

    for (byte *pData = &m_buffer[0] + sizeof(XGraphicsHeader), *pEnd = &m_buffer[0] + m_buffer.size(); pData < pEnd;)
        {
        byte*       pOpStart = pData;
        UInt16      opCode;
        UInt32      opSize;
        StatusInt   status;

        if (SUCCESS != (status = XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd)))
            return status;

        if (XGRAPHIC_OpCode_DrawSymbol == opCode)
            {
            if (*currId != symbol.GetParentId())
                {
                currId++;
                continue;
                }

            UInt32      elementIndex;
            Transform   transform;

            opSize -= sizeof (opSize);
            XGraphicsDrawSymbol::Get (elementIndex, transform, pData, opSize);
            pData += opSize;

            TransformInfo   transformInfo;

            if (NULL == symbol.GetTransform())
                {
                transformInfo.GetTransformR() = transform;
                }
            else    
                {
                Transform       inverseBasis;

                inverseBasis.InverseOf (*symbol.GetTransform());
                transformInfo.GetTransformR().productOf (&transform, &inverseBasis);
                }
            symbol.OnTransform (transformInfo);

            XGraphicsContainer  tailData;

            tailData.BeginDraw ();

            if (pData < pEnd)
                tailData.Write (pData, pEnd - pData);

            Resize (pOpStart - GetData());

            XGraphicsWriter writer (this);

            writer.AppendContainer (symbol);
            writer.AppendContainer (tailData);
            m_header.m_useCache = true;
            EndDraw ();

            StatusInt   status;

            if (SUCCESS == (status = eh.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0, GetDataSize(), &m_buffer[0])))
                {
                m_symbolIds.erase (currId);
                AddSymbolIdsToElement(eh);
                }

            return status;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContextP XGraphicsPublish::CreateXGraphicsContext (XGraphicsContainer& container, UInt32 createOptions, UInt32 optimizeOptions, DgnModelR model)
    {
    XGraphicsContext* xgContext = new XGraphicsContext (container, createOptions, optimizeOptions);

    xgContext->SetDgnProject (model.GetDgnProject ());

    return xgContext;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsPublish::DeleteXGraphicsContext (ViewContextP context)
    {
    delete ((XGraphicsContext*) context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsPublish::BeginSymbol (ViewContextP context)
    {
    ((XGraphicsContext*) context)->BeginSymbol ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsPublish::EndSymbol (ViewContextP context, DgnModelP modelRef)
    {
    ((XGraphicsContext*) context)->EndSymbol (modelRef);
    }

// 8.11.9 additions....
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
*
*   Draws the container contents as a proxy to the ElementHandle (not the element XAttributes).
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XGraphicsContainer::DrawProxy (ViewContextR context, ElementHandleCR eh, UInt32 qvIndex, DrawOptions options) const
    {
    if (m_buffer.empty())
        return ERROR;

    XGraphicsOperationContext opContext ((XGraphicsHeader*) &m_buffer.front(), *eh.GetDgnProject(), const_cast <T_XGraphicsSymbolIdsR> (m_symbolIds), &eh, options);
    opContext.m_header->m_useCache = true;

    if (NULL != eh.GetElementRef())//  && ELEMENT_REF_TYPE_ProxyDisplay == eh.GetElementRef()->GetRefType())    removed in Graphite
        XGraphicsCacheStroker::DrawOrCacheFromMemory (context, CachedDrawHandle(&eh), qvIndex, GetGraphicsData(), GetDataEnd(), opContext);
    else
        XGraphicsCacheStroker::DrawFromMemory (context, GetGraphicsData(), GetDataEnd(), opContext, XGraphicsDrawOperator::DrawAll);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct ExtractProxyGPArrayOperator : XGraphicsConstOperator
{
    XGraphicsWriter                     m_output;
    ProxyHLEdgeSegmentId                m_curvePrimitiveId;
    std::vector <ProxyHLEdgeSegmentId>  m_foundIds;
    bool                                m_startFound;

    ExtractProxyGPArrayOperator (XGraphicsContainerR container, ProxyHLEdgeSegmentIdCR curvePrimitiveId) : m_output (&container), m_curvePrimitiveId (curvePrimitiveId), m_startFound (false) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DoOperation (struct XGraphicsOperation& operation, byte const* pData, UInt32 size, byte const* pEnd, XGraphicsOpCodes opCode) override
    {
    switch (opCode)
        {
        case XGRAPHIC_OpCode_AnnotationData:
            {
            if (m_startFound)           // Stop traversal.
                return ERROR;

            XGraphicsAnnotationData::Signature sig = (XGraphicsAnnotationData::Signature) *(UInt32*) pData;

            if (XGraphicsAnnotationData::SIGNATURE_EdgeId == sig)
                {
                CurvePrimitiveId::Type  edgeType = (CurvePrimitiveId::Type) *(pData + sizeof (sig));
                if (CurvePrimitiveId::Type_CachedEdge == edgeType)
                   {
                   ProxyHLEdgeSegmentId     curvePrimitiveId;
                   byte const*              pEdgeData = pData + sizeof (sig);

                   if (SUCCESS == curvePrimitiveId.Init (pEdgeData, size - sizeof (size) - sizeof (sig)) && m_curvePrimitiveId.m_edgeId == curvePrimitiveId.m_edgeId)
                        {
                        m_startFound = m_curvePrimitiveId.Equals (curvePrimitiveId);
                        m_foundIds.push_back (curvePrimitiveId);
                        }
                   }
                }
            }

        case XGRAPHIC_OpCode_BeginComplexString: 
        case XGRAPHIC_OpCode_BeginComplexShape:  
        case XGRAPHIC_OpCode_EndComplex:        
            break;

        default:
            {
            if (m_startFound)
                {
                if (m_output.m_container->IsEmpty())
                    m_output.m_container->BeginDraw ();

                m_output.WriteOpCode (opCode);
                m_output.WriteUInt32 (size);
                m_output.WriteData (pData, size - sizeof (size));
                }
            }
        }

    return SUCCESS;
    }

};  // ExtractProxyGPArrayOperator

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::ExtractProxyGPArray (XGraphicsContainerR edgeGraphics, ProxyHLEdgeSegmentIdCR curvePrimitiveId, GPArrayParamCP edgeParam) const
    {
    ExtractProxyGPArrayOperator        extractOperator (edgeGraphics, curvePrimitiveId);

    // The first traversal looks for an exact match.  This means that the edge was hidden in exactly the same manner as when the association was created.
    XGraphicsOperations::Traverse(m_buffer, extractOperator);

    if (edgeGraphics.IsEmpty() && !extractOperator.m_foundIds.empty() && NULL != edgeParam)  
        {
        // If we didn't find an exact match, we'll look for a segment that either contains the association point or is closest to that point.
        GPArrayParam        originalHitParam;                             
        int                 startIndex = (int) curvePrimitiveId.m_startParam, endIndex = (int) curvePrimitiveId.m_endParam;
        double              startParam = curvePrimitiveId.m_startParam - (double) startIndex, endParam = curvePrimitiveId.m_endParam - (double) endIndex;

        originalHitParam.m_index = (int) curvePrimitiveId.m_startParam + edgeParam->m_index;

        double              segmentStart = (edgeParam->m_index == 0) ? startParam : 0.0;
        double              segmentEnd   = (edgeParam->m_index == (endIndex - startIndex + 1)) ? endParam : 1.0;

        originalHitParam.m_param = segmentStart + edgeParam->m_param * (segmentEnd - segmentStart);
        double              searchParam = originalHitParam.m_index + originalHitParam.m_param, closestDist = 1.0E8;

        for (std::vector <ProxyHLEdgeSegmentId>::iterator curr = extractOperator.m_foundIds.begin(); curr != extractOperator.m_foundIds.end(); curr++)
            {   
            double startDist = curr->m_startParam - searchParam;
            double endDist   = searchParam - curr->m_endParam;

            if (startDist <= 0.0 && endDist <= 0.0)
                {
                extractOperator.m_curvePrimitiveId = *curr;
                break;
                }
            else
                {
                if ((startDist > 0.0 && startDist < closestDist) ||
                    (endDist > 0.0 && endDist < closestDist))
                    extractOperator.m_curvePrimitiveId = *curr;
                }
            }
        XGraphicsOperations::Traverse (m_buffer, extractOperator);
        }

    return edgeGraphics.IsEmpty() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsContainer::XGraphicsContainer (void const* data, size_t dataSize)
    {
    m_elementRef = NULL;
    m_buffer.resize(dataSize);
    memcpy (&m_buffer[0], data, dataSize);
    memcpy (&m_header, data, sizeof (m_header));
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct ExtractProxyCutEdgeOperator : XGraphicsConstOperator
{
    XGraphicsWriter                     m_writer;
    ProxyEdgeIdData                     m_curvePrimitiveId;
    bool                                m_startFound;

    ExtractProxyCutEdgeOperator (XGraphicsContainerR output, ProxyEdgeIdDataCR curvePrimitiveId) : m_writer (&output), m_curvePrimitiveId (curvePrimitiveId), m_startFound (false) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DoOperation (struct XGraphicsOperation& operation, byte const* pData, UInt32 size, byte const* pEnd, XGraphicsOpCodes opCode) override
    {
    if (XGRAPHIC_OpCode_AnnotationData == opCode)
        {
        XGraphicsAnnotationData::Signature sig = (XGraphicsAnnotationData::Signature) *(UInt32*) pData;

        if (XGraphicsAnnotationData::SIGNATURE_EdgeId == sig)
            {
            if (m_startFound)
                return ERROR;           // We're done.

            m_startFound = m_curvePrimitiveId.Matches (pData + sizeof (sig), size - sizeof (size) - sizeof (sig));
            }
        }
    else if (m_startFound && XGRAPHIC_OpCode_EndComplex == opCode)
        {
        return ERROR;                                                                                                                                                             
        }
    else
        {
        if (m_startFound || XGRAPHIC_OpCode_PushTransClip == opCode || XGRAPHIC_OpCode_PopTransClip == opCode)
            {
            if (m_writer.m_container->IsEmpty())
                m_writer.m_container->BeginDraw ();

            m_writer.WriteOpCode (opCode);
            m_writer.WriteUInt32 (size);
            m_writer.WriteData (pData, size - sizeof (size));
            }
        }

    return SUCCESS;
    }
};  // ExtractProxyCutEdgeOperator

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsContainer::ExtractProxyCutEdge (XGraphicsContainerR edgeGraphics, ProxyEdgeIdDataCR curvePrimitiveId) const
    {
    ExtractProxyCutEdgeOperator extractProxyCutEdgeOperator (edgeGraphics, curvePrimitiveId);
    XGraphicsOperations::Traverse (m_buffer, extractProxyCutEdgeOperator);

    return edgeGraphics.IsEmpty() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   XGraphicsContainer::AddCurveVector (CurveVectorCR curveVector, bool filled)
    {
    XGraphicsContext  context (*this);
                                          
    if (IsEmpty ())
        BeginDraw ();

    context.GetIDrawGeom().DrawCurveVector (curveVector, filled);

    return EndDraw ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    XGraphicsContainer::AddProxyCurve (GPArrayCR gpa, GPArrayIntervalCP interval, ProxyHLEdgeSegmentIdCR segmentId, CompoundDrawStateP cds)
    {
    bool                isCompleteSegment = segmentId.IsComplete();
    ProxyEdgeIdData     idData;
    XGraphicsContext    context (*this);
    IDrawGeomR          viewDraw = context.GetIViewDraw ();
    GPArrayInterval     subCurveInterval = segmentId.GetInterval(), completeInterval (gpa), outputInterval = (NULL == interval) ? completeInterval : *interval;

    if (IsEmpty ())
        BeginDraw ();

    if (segmentId.m_edgeId.IsAssociable ())
        {
        GPArraySmartP          outputSegment;
        GPArraySegmentLengths  lengths (gpa);
        CurveVectorPtr         multiSegmentCurve;

        if (outputInterval.GetSegmentCount () > 1)
            multiSegmentCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

        for (GPArraySegmentLengths::iterator curr = lengths.find (outputInterval.m_start.m_index); curr != lengths.end (); curr++)
            {
            GPArrayInterval  segmentInterval (GPArrayParam (curr->first, 0.0), GPArrayParam (curr->first, 1.0), NULL);

            if (segmentInterval.m_start < outputInterval.m_start)
                segmentInterval.m_start = outputInterval.m_start;
            else if (segmentInterval.m_start > outputInterval.m_end)
                break;                                                                                                                                 

            if (segmentInterval.m_end.m_index == outputInterval.m_end.m_index)
                segmentInterval.m_end.m_param = outputInterval.m_end.m_param;
               
            outputSegment->EmptyAll();

            if (outputSegment->CopyPortionOf (&gpa, segmentInterval.m_start, segmentInterval.m_end))
                {
                GPArrayInterval  localSegmentInterval (GPArrayParam (0, segmentInterval.m_start.m_param), GPArrayParam (0, segmentInterval.m_end.m_param), NULL);

                if (!isCompleteSegment)
                    localSegmentInterval = GPArrayInterval::InterpolateSubInterval (localSegmentInterval, subCurveInterval, gpa); 

                if (multiSegmentCurve.IsValid ())
                    ProxyHLEdgeSegmentId (ProxyHLEdgeId (segmentId.m_edgeId, curr->first), localSegmentInterval).Save (idData);
                else
                    ProxyHLEdgeSegmentId (segmentId.m_edgeId, localSegmentInterval).Save (idData);

                int                 index = 0;
                ICurvePrimitivePtr  primitive = outputSegment->GetCurvePrimitive (index);

                if (primitive.IsValid ())
                    {
                    CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create (idData.GetType (), (void *) idData.GetAnnounceData (), idData.GetAnnounceSize (), cds);
                    primitive->SetId (newId.get());

                    if (multiSegmentCurve.IsValid ())
                        {
                        multiSegmentCurve->push_back (primitive);
                        }
                    else
                        {
                        CurveVectorPtr  tmpCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

                        tmpCurve->push_back (primitive);
                        viewDraw.DrawCurveVector (*tmpCurve, false);
                        }
                    }
                }
            }

        if (multiSegmentCurve.IsValid ())
            viewDraw.DrawCurveVector (*multiSegmentCurve, false);
        }
    else
        {
#if defined (DGNPLATFORM_WIP_DRAWGEOM) // Verify HLine...
        // Is this still necessary?!? What does this edge id mean if it's not "associable"...
        ProxyHLEdgeSegmentId (curvePrimitiveId, outputInterval).Save (idData);
        idData.Announce (context);
#endif

        if (NULL == interval)
            {
            (const_cast <GPArrayR> (gpa)).Draw (viewDraw, false, false);
            }
        else
            {
            GPArraySmartP    outputSegment;
            GPArrayInterval  outputInterval = *interval;

            outputSegment->CopyPortionOf (&gpa, outputInterval.m_start, outputInterval.m_end);
            outputSegment->Draw (viewDraw, false, false);
            }
        }

    return EndDraw ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr XGraphicsContainer::GetCurveVector () const
    {
    GPArraySmartP       gpa;
    CurveVectorPtr      curveVector;

    AppendComplexComponentsToGpaOperator appendToGpa (*gpa);

    if (SUCCESS != XGraphicsOperations::Traverse (const_cast<XGraphicsDataR> (m_buffer), appendToGpa))
        return CurveVectorPtr();

    return gpa->ToCurveVector ();
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2012
+===============+===============+===============+===============+===============+======*/
struct XGraphicsUnifyOutput : XGraphicsOutput
{
    DEFINE_T_SUPER(XGraphicsOutput)

    bvector<ISolidKernelEntityPtr>& m_otherBodies;
    bool                            m_unified;

            XGraphicsUnifyOutput (XGraphicsContainerR unifiedXGraphics, bvector<ISolidKernelEntityPtr>& otherBodies) : XGraphicsOutput (unifiedXGraphics, XGRAPHIC_CreateOptions_None, XGRAPHIC_OptimizeOptions_None), m_otherBodies (otherBodies), m_unified (false) { }
    bool    GetUnified () { return m_unified; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ComputeUnified (ISolidKernelEntityPtr& unified, ISolidKernelEntityCR target, bvector <ISolidKernelEntityPtr>& tools)
    {
    Transform   localTransform, inverseLocalTransform;
    bool        localTransformValid = (SUCCESS == m_viewContext->GetCurrLocalToFrustumTrans (localTransform));

    if (localTransformValid)
        {
        if (!inverseLocalTransform.InverseOf (localTransform))
            return ERROR;
            
        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._CopyEntity (unified, target))
            return ERROR;

        unified->PreMultiplyEntityTransformInPlace (localTransform);
        }
    else
        {
        if (SUCCESS != T_HOST.GetSolidsKernelAdmin()._CopyEntity (unified, target))
            return ERROR;
        }

    BentleyStatus   status;

    if (SUCCESS == (status = T_HOST.GetSolidsKernelAdmin()._UnifyBody (unified, &tools.front (), tools.size ())))
        {
        m_unified = true;

        if (localTransformValid)
            unified->PreMultiplyEntityTransformInPlace (inverseLocalTransform);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DrawBody (ISolidKernelEntityCR body, IFaceMaterialAttachmentsCP attachments, double pixelSize) override
    {
    ISolidKernelEntityPtr   unified;

    if (SUCCESS == ComputeUnified (unified, body, m_otherBodies))
        return T_Super::_DrawBody (*unified, attachments, pixelSize);
    else
        return T_Super::_DrawBody (body, attachments, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawSolidPrimitive (ISolidPrimitiveCR primitive) override
    {
    ISolidKernelEntityPtr       body, unified;

    if (SUCCESS == T_HOST.GetSolidsKernelAdmin()._CreateBodyFromSolidPrimitive (body, primitive, m_viewContext->GetDgnProject ()) &&
        SUCCESS == ComputeUnified (unified, *body, m_otherBodies))
        T_Super::_DrawBody (*unified, NULL, 0.0);
    else
        T_Super::_DrawSolidPrimitive (primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawPolyface (PolyfaceQueryCR meshData, bool filled) override
    {
    ISolidKernelEntityPtr       body, unified;

    if (SUCCESS == T_HOST.GetSolidsKernelAdmin()._CreateBodyFromPolyface (body, meshData, m_viewContext->GetDgnProject ()) &&
        SUCCESS == ComputeUnified (unified, *body, m_otherBodies))
        {
        IFacetTopologyTablePtr      topologyTable;
        IFacetOptionsPtr            facetOptions = IFacetOptions::Create ();
        PolyfaceHeaderPtr           unifiedPolyface = PolyfaceHeader::New();

        if (SUCCESS == T_HOST.GetSolidsKernelAdmin()._FacetBody (topologyTable, *unified, *facetOptions, NULL) &&
            SUCCESS == IFacetTopologyTable::ConvertToPolyface (*unifiedPolyface, *topologyTable, *facetOptions))
            {
            unifiedPolyface->Transform (unified->GetEntityTransform());
            T_Super::_DrawPolyface (*unifiedPolyface, filled);
            }
        else
            {
            T_Super::_DrawBody (*unified, NULL, 0.0);
            }
        }
    else
        {
        T_Super::_DrawPolyface (meshData, filled);
        }
    }

};  // XGraphicsUnifyOutput

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/06
+===============+===============+===============+===============+===============+======*/
struct      XGraphicsUnifyContext : public XGraphicsContext
{
    XGraphicsUnifyOutput&    m_unifyOutput;
    
    XGraphicsUnifyContext (XGraphicsContainerR xGraphics, XGraphicsUnifyOutput& unifyOutput) : XGraphicsContext(xGraphics), m_unifyOutput (unifyOutput)
        {
        _GetOutput ().SetViewContext (this);
        _SetupOutputs ();
        m_purpose = DrawPurpose::NotSpecified;
        };

    virtual XGraphicsOutput& _GetOutput () override {return m_unifyOutput;}

}; // XGraphicsUnifyContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus XGraphicsContainer::Unify (XGraphicsContainerP& unifiedXGraphics, ElementHandleCR eh, bvector<ISolidKernelEntityPtr>& otherBodies)
    {
    unifiedXGraphics = new XGraphicsContainer();

    XGraphicsUnifyOutput        output (*unifiedXGraphics, otherBodies);
    XGraphicsUnifyContext       context (*unifiedXGraphics, output);

    output.SetViewContext (&context);
    context.SetDgnProject (*eh.GetDgnProject ());

    unifiedXGraphics->BeginDraw();
    context.VisitElemHandle (eh, false, false);
    unifiedXGraphics->EndDraw();

    if (!output.GetUnified())
        {
        DELETE_AND_CLEAR (unifiedXGraphics);
        return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    XGraphicsContainer::IsXGraphicsSymbol (ElementRefP elRef)
    {
    XAttributeHandle  transformIterator (elRef, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_SymbolTransform), 0);
    XAttributeHandle  idIterator (elRef, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_SymbolId), 0);

    return idIterator.IsValid() || transformIterator.IsValid();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void BRepWireGraphicsAppData::CreateWireGeomCache (XGraphicsOperationContextR opContext, ViewContextR context, ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments)
    {
    ElementRefP elemRef = (opContext.m_element ? opContext.m_element->GetElementRef () : NULL);

    if (NULL == elemRef)
        return;

    BRepWireGraphicsAppData* geomCache = (BRepWireGraphicsAppData*) elemRef->FindAppData (s_brepWireGraphicsCacheKey);

    if (NULL == geomCache)
        {
        HeapZone&  zone = elemRef->GetHeapZone ();

        geomCache = new ((BRepWireGraphicsAppData*) zone.Alloc (sizeof (BRepWireGraphicsAppData))) BRepWireGraphicsAppData ();

        if (SUCCESS != elemRef->AddAppData (s_brepWireGraphicsCacheKey, geomCache, zone))
            {
            zone.Free (geomCache, sizeof (*geomCache));

            return;
            }
        }

    T_BRepWireGraphicsMap::const_iterator found = geomCache->m_wireMap.find (opContext.m_brepOffset);

    if (found != geomCache->m_wireMap.end ())
        return;

    XGraphicsContainer* wireContainer = new (XGraphicsContainer);
    XGraphicsContext    xgContext (*wireContainer);

    wireContainer->BeginDraw ();
    xgContext.SetDgnProject (*elemRef->GetDgnProject ());
    *xgContext.GetCurrentDisplayParams () = *context.GetCurrentDisplayParams (); // Setup base symbology...
    WireframeGeomUtil::Draw (entity, xgContext, attachments);
    wireContainer->EndDraw ();

    geomCache->m_wireMap[opContext.m_brepOffset] = wireContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BRepWireGraphicsAppData::DrawWireGeomCache (XGraphicsOperationContextR opContext, ViewContextR context)
    {
    ElementRefP elemRef = (opContext.m_element ? opContext.m_element->GetElementRef () : NULL);

    if (NULL == elemRef)
        return ERROR;

    BRepWireGraphicsAppData* geomCache = (BRepWireGraphicsAppData*) elemRef->FindAppData (s_brepWireGraphicsCacheKey);

    if (NULL == geomCache)
        return ERROR;

    T_BRepWireGraphicsMap::const_iterator found = geomCache->m_wireMap.find (opContext.m_brepOffset);

    if (found == geomCache->m_wireMap.end ())
        return ERROR;

    XGraphicsCacheStroker::DrawFromMemory (context, found->second->GetGraphicsData (), found->second->GetDataEnd (), opContext, XGraphicsDrawOperator::DrawAll);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsContainer::QueryProperties (ElementHandleCR eh, PropertyContextR context) // moved here in graphite from ClongRemapper.cpp, which was deleted
    {
    // Avoid creating XGraphicsContainer for properties we never store (ex. ELEMENT_PROPERTY_Level)
    if (0 == ((ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Weight | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Transparency | ELEMENT_PROPERTY_Material | ELEMENT_PROPERTY_Font) & context.GetElementPropertiesMask ()))
        return;

    XGraphicsContainer container;

    if (SUCCESS != container.ExtractFromElement (eh))
        return;

    container.ProcessProperties (context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsContainer::EditProperties (EditElementHandleR eeh, PropertyContextR context) // moved here in graphite from ClongRemapper.cpp, which was deleted
    {
    // Avoid creating XGraphicsContainer for properties we never store (ex. ELEMENT_PROPERTY_Level)
    if (0 == ((ELEMENT_PROPERTY_Color | ELEMENT_PROPERTY_Weight | ELEMENT_PROPERTY_Linestyle | ELEMENT_PROPERTY_Transparency | ELEMENT_PROPERTY_Material | ELEMENT_PROPERTY_Font) & context.GetElementPropertiesMask ()))
        return;

    XGraphicsContainer container;

    if (SUCCESS != container.ExtractFromElement (eeh))
        return;

    container.ProcessProperties (context);

    if (!context.GetElementChanged ())
        return;

    container.AddToElement (eeh);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsOperations::CalculateRange (UInt16 opCode, byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext)
    {
    return s_xGraphicOps[opCode]->_CalculateRange (pData, size, pEnd, context, opContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
StatusInt XGraphicsOperations::Draw (UInt16 opCode, ViewContextP context, byte const* pData, UInt32 dataSize, XGraphicsOperationContextR opContext)
    {
    return s_xGraphicOps[opCode]->_Draw (*context, pData, dataSize, opContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsOperation::_CalculateRange (byte*& pData, UInt32& size, byte* pEnd, ViewContextR context, XGraphicsOperationContextR opContext)
    {
    _Draw (context, pData, size - sizeof (size), opContext);
    pData += size - sizeof (size);

    return SUCCESS;
    }

//=======================================================================================
// @bsiclass                                                    MattGooding     03/13
//=======================================================================================
struct XGraphicsRangeContext : public NullContext
{
    DEFINE_T_SUPER (NullContext)

private:
    RangeOutput     m_output;

public:
    XGraphicsRangeContext()
        {
        m_purpose = DrawPurpose::RangeCalculation;
        m_is3dView = true;

        SetBlockAsynchs (true);

        m_output.Init (this);
        _SetupOutputs();
        }

    virtual void    _SetupOutputs() override            {SetIViewDraw (m_output);}
    virtual bool    _WantUndisplayed() override         {return true;}

    ElemRangeCalc*  GetElemRange()                      {return m_output.GetElemRange();}
};

//=======================================================================================
// @bsiclass                                                    MattGooding     03/13
//=======================================================================================
struct XGraphicsOpEntry
{
public:
    byte*               m_opStart;
    UInt32              m_opSize;

    XGraphicsOpEntry (byte* opStart, UInt32 opSize) : m_opStart (opStart), m_opSize (opSize)    { }
};

//=======================================================================================
// @bsiclass                                                    MattGooding     04/13
//=======================================================================================
struct XGraphicsDrawEntry
{
public:
    XGraphicsOpEntry    m_opEntry;
    Int32               m_matSymbIndex;
    Int32               m_transformIndex;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   MattGooding     04/13
    //---------------------------------------------------------------------------------------
    XGraphicsDrawEntry (byte* opStart, UInt32 opSize, Int32 matSymbIndex, Int32 transformIndex)
        :
        m_opEntry (opStart, opSize),
        m_matSymbIndex (matSymbIndex),
        m_transformIndex (transformIndex)
        {
        }
};

//=======================================================================================
// @bsiclass                                                    MattGooding     03/13
//=======================================================================================
struct XGraphicsRangeEntry : RefCountedBase
{
    friend struct DgnPlatform::XGraphicsContainer;

public:
    XGraphicsDrawEntry  m_drawEntry;
    DRange3d            m_range;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   MattGooding     03/13
    //---------------------------------------------------------------------------------------
    XGraphicsRangeEntry (byte* opStart, UInt32 opSize, Int32 matSymbIndex, Int32 transformIndex)
        :
        m_drawEntry (opStart, opSize, matSymbIndex, transformIndex)
        {
        }
};

typedef RefCountedPtr <struct XGraphicsRangeEntry>  XGraphicsRangeEntryPtr;
typedef bvector <XGraphicsDrawEntry>                DrawEntryList;
typedef bvector <XGraphicsOpEntry>                  OpEntryList;
typedef std::list <XGraphicsRangeEntryPtr>            RangeEntryList;

//=======================================================================================
// @bsiclass                                                    MattGooding     03/13
//=======================================================================================
struct DgnPlatform::XGraphicsSplitter
{
    static BentleyStatus SplitByRange (bvector<XGraphicsContainer>& splitGraphics, DgnModelR model);
    static void WriteOpEntry (XGraphicsContainer& container, size_t& outPos, XGraphicsOpEntry const& opEntry);
    static void WriteParsedMatSymb (XGraphicsContainer& container, size_t& outPos, ParsedMatSymb& parsed);
    static void AddOpsToContainer (XGraphicsContainerR container, T_XGraphicsSymbolIdsR symbols, size_t dataSize,
                                   RangeEntryList const& ops, OpEntryList const& matSymbs, OpEntryList const& transforms);
    static BentleyStatus SplitOperations (XGraphicsContainerR container, DrawEntryList& opsToFind, XGraphicsOpCodes opCodeToFind,
                                          XGraphicsData& otherOps, OpEntryList& matSymbs, OpEntryList& transforms);
    static BentleyStatus SegmentSurface (bvector <MSBsplineSurface>& segments, MSBsplineSurfaceR surface, UInt32 maxPole);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void XGraphicsSplitter::WriteParsedMatSymb (XGraphicsContainer& container, size_t& outPos, ParsedMatSymb& parsed)
    {
    if (0 == parsed.m_mask)
        return;

    XGraphicsWriter     writer(&container);
    ElemMatSymb dummy;
    size_t startSize = container.GetDataSize();

    //  The materialId is not used unless the bit is set in the mask so it is okay to pass true for wantMaterial.
    writer.WriteMatSymb(parsed.m_mask, 0, 0, false, dummy, parsed.m_params, true, parsed.m_materialId);
    parsed.Clear();
    outPos += container.GetDataSize() - startSize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
void XGraphicsSplitter::WriteOpEntry (XGraphicsContainer& container, size_t& outPos, XGraphicsOpEntry const& opEntry)
    {
    container.m_buffer.resize (container.m_buffer.size() + opEntry.m_opSize + sizeof (UInt16));
    memcpy (&container.m_buffer[outPos], opEntry.m_opStart, opEntry.m_opSize + sizeof (UInt16));
    outPos += opEntry.m_opSize + sizeof (UInt16);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
void XGraphicsSplitter::AddOpsToContainer (XGraphicsContainerR container, T_XGraphicsSymbolIdsR symbols, size_t dataSize,
                                            RangeEntryList const& ops, OpEntryList const& matSymbs, OpEntryList const& transforms)
    {
    container.BeginDraw();
    container.SetUseCache (true);
        container.SetIsRenderable (true);
    container.m_symbolIds = symbols;

    size_t  outPos = sizeof (XGraphicsHeader);
    Int32   lastMatSymbIndex = -1, lastTransformIndex = -1;

    ParsedMatSymb   parsedMatSymb;

    for (auto& iter : ops)
        {
        parsedMatSymb.Clear();

        for (; iter->m_drawEntry.m_matSymbIndex > lastMatSymbIndex; )
            {
            ++lastMatSymbIndex;
            UInt16      opCode;
            UInt32      opSize;  //  we shouldn't have to save opSize in the OpEntryList
            byte*       data = matSymbs[lastMatSymbIndex].m_opStart;
            byte*       end =  data + matSymbs[lastMatSymbIndex].m_opSize + sizeof(opSize);

            XGraphicsOperations::GetOperation (opCode, opSize, data, end);

            if (XGRAPHIC_OpCode_MatSymb != opCode || XGraphicsMatSymb::CombineDisplayParamsWithParsed(parsedMatSymb, data) != BSISUCCESS)
                {
                //  Have a XGRAPHIC_OpCode_MatSymb or has cooked symbology.  Write what we have accumulated so far and then write this entry.
                WriteParsedMatSymb(container, outPos, parsedMatSymb);
                WriteOpEntry (container, outPos, matSymbs[lastMatSymbIndex]);
                }
            }

        WriteParsedMatSymb(container, outPos, parsedMatSymb);

        for (; iter->m_drawEntry.m_transformIndex > lastTransformIndex; )
            WriteOpEntry (container, outPos, transforms[++lastTransformIndex]);

        WriteOpEntry (container, outPos, iter->m_drawEntry.m_opEntry);
        }

    for (Int32 limit = (Int32) (transforms.size()) - 1; lastTransformIndex < limit; )
        WriteOpEntry (container, outPos, transforms[++lastTransformIndex]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsSplitter::SplitOperations (XGraphicsContainerR container, DrawEntryList& opsToFind,
                                                  XGraphicsOpCodes opCodeToFind, XGraphicsData& otherOps,
                                                  OpEntryList& matSymbs, OpEntryList& transforms)
    {
    otherOps.reserve (container.GetDataSize());

    for (byte *base = &container.m_buffer[0] + sizeof(XGraphicsHeader), *data = base, *end = &container.m_buffer[0] + container.m_buffer.size(); data < end;)
        {
        byte*       opStart = data;
        UInt16      opCode;
        UInt32      opSize;

        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, data, end))
            return ERROR;

        if (opCode == opCodeToFind)
            {
            opsToFind.push_back (XGraphicsDrawEntry (opStart, opSize, ((Int32) matSymbs.size()) - 1, ((Int32) transforms.size()) - 1));
            data += opSize - sizeof (opSize);
            continue;
            }

        switch (opCode)
            {
            case XGRAPHIC_OpCode_MatSymb:
            case XGRAPHIC_OpCode_MatSymb2:
                if (matSymbs.empty() || opSize != matSymbs.back().m_opSize || 0 != memcmp (opStart, matSymbs.back().m_opStart, opSize))
                    matSymbs.push_back (XGraphicsOpEntry (opStart, opSize));
                break;
            case XGRAPHIC_OpCode_PushTransClip:
            case XGRAPHIC_OpCode_PopTransClip:
            case XGRAPHIC_OpCode_PopAll:
                transforms.push_back (XGraphicsOpEntry (opStart, opSize));
                break;
            }

        otherOps.Append (opStart, opSize + sizeof (opCode));
        data += opSize - sizeof (opSize);
        }

    return opsToFind.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsSplitter::SegmentSurface (bvector <MSBsplineSurface>& segments, MSBsplineSurfaceR surface, UInt32 maxPole)
    {
    if (surface.uParams.numPoles <= (int) maxPole && surface.vParams.numPoles <= (int) maxPole)
        return ERROR;

    int     uSegments = int (ceil ((double) surface.uParams.numPoles / (double) maxPole)),
            vSegments = int (ceil ((double) surface.vParams.numPoles / (double) maxPole));

    double  uIncrement = 1.0 / (double) uSegments,
            vIncrement = 1.0 / (double) vSegments;

    bool    segmentFailed = false;

    segments.resize (uSegments * vSegments);

    DPoint2d segStart, segEnd;

    segStart.x  = 0.0;
    segEnd.x    = uIncrement;

    for (int u = 0, i = 0; u < uSegments; ++u)
        {
        segStart.y  = 0.0;
        segEnd.y    = vIncrement;

        for (int v = 0; v < vSegments; ++v)
            {
            if (SUCCESS != bspsurf_segmentSurface (&segments[i++], &surface, &segStart, &segEnd))
                {
                segmentFailed = true;
                break;
                }

            segStart.y  += vIncrement;
            segEnd.y    += vIncrement;
            }

        if (segmentFailed)
            break;

        segStart.x  += uIncrement;
        segEnd.x    += uIncrement;
        }

    if (segmentFailed)
        segments.clear();

    return !segments.empty() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
static bool rangeIsContained (DRange3dCR r1, DRange3dCR r2)
    {
    return  (r1.low.x >= r2.low.x
             && r1.high.x <= r2.high.x
             && r1.low.y >= r2.low.y
             && r1.high.y <= r2.high.y
             && r1.low.z >= r2.low.z
             && r1.high.z <= r2.high.z);    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     03/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::SplitByRange (std::list<XGraphicsContainer>& splitGraphics, DgnModelR model)
    {
    //  Arbitrary threshold. We claim if it is smaller than this it probably is not worth incurring the overhead of
    //  multiple elements.
    if (m_buffer.size() < 10000)
        return ERROR;

    XGraphicsRangeContext rangeContext;
    rangeContext.SetDgnProject (model.GetDgnProject ());

    XGraphicsOperationContext           opContext ((XGraphicsHeader*) &m_buffer.front(), model.GetDgnProject (), const_cast <T_XGraphicsSymbolIdsR> (m_symbolIds), NULL);
    RangeEntryList                      rangeEntries;
    OpEntryList                         matSymbs;
    OpEntryList                         transforms;

    DRange3d totalRange;
    totalRange.Init();

    for (byte *pBase = &m_buffer[0] + sizeof(XGraphicsHeader), *pData = pBase, *pEnd = &m_buffer[0] + m_buffer.size(); pData < pEnd;)
        {
        byte*       pOpStart = pData;
        UInt16      opCode;
        UInt32      opSize;

        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
            return ERROR;

        switch (opCode)
            {
            case XGRAPHIC_OpCode_MatSymb:
            case XGRAPHIC_OpCode_MatSymb2:
                if (matSymbs.empty() || opSize != matSymbs.back().m_opSize || 0 != memcmp (pOpStart, matSymbs.back().m_opStart, opSize))
                    matSymbs.push_back (XGraphicsOpEntry (pOpStart, opSize));
                pData += opSize - sizeof (opSize);
                continue;
            case XGRAPHIC_OpCode_PushTransClip:
            case XGRAPHIC_OpCode_PopTransClip:
            case XGRAPHIC_OpCode_PopAll:
                transforms.push_back (XGraphicsOpEntry (pOpStart, opSize));
                pData += opSize - sizeof (opSize);
                continue;
            }

        rangeContext.GetElemRange()->Invalidate();

        if (SUCCESS != XGraphicsOperations::CalculateRange (opCode, pData, opSize, pEnd, rangeContext, opContext))
            return ERROR;

        rangeEntries.push_back (new XGraphicsRangeEntry (pOpStart, opSize, ((Int32) matSymbs.size()) - 1, ((Int32) transforms.size()) - 1));
        rangeContext.GetElemRange()->GetRange (rangeEntries.back()->m_range);

        totalRange.Extend (rangeEntries.back()->m_range);
        }

    if (totalRange.IsNull() || 1 == rangeEntries.size())
        return ERROR;

    double totalExtent = totalRange.ExtentSquared();

    RangeEntryList                              noSplitOps;
    struct  SplitOp
        {
        DRange3d    m_range;
        double      m_minimumExtent;
        RangeEntryList m_rangeEntries;
        SplitOp(DRange3dCR range, double minimumExtent, RangeEntryList&entries) : m_range(range), m_minimumExtent(minimumExtent), m_rangeEntries(entries) {}
        };

    std::vector <SplitOp>  splitOps;

    static double s_minimumTotalRatio = 0.0625;
    static double s_minimumExtentRatio = 0.1;
    static double s_newContainerRatio = 20.0;
    static size_t s_maximumPerNode = 10;

    SplitOp*    prevSplitOp = NULL;
    for (auto& iter : rangeEntries)
        {
        double thisExtent = iter->m_range.ExtentSquared();
        if (thisExtent <= DBL_EPSILON || iter->m_range.IsNull() || (noSplitOps.size() < s_maximumPerNode && ((thisExtent / totalExtent) > s_minimumTotalRatio)))
            noSplitOps.push_back (iter);
        else
            {
            if (NULL != prevSplitOp)
                {
                //  First see if it fits in the same bucket as the previous operation.
                if (prevSplitOp->m_rangeEntries.size() < s_maximumPerNode && rangeIsContained (iter->m_range, prevSplitOp->m_range) && (thisExtent >= prevSplitOp->m_minimumExtent))
                    {
                    prevSplitOp->m_rangeEntries.push_back (iter);
                    continue;
                    }
                }

            bool foundContainer = false;
            prevSplitOp = NULL;
            
            for (auto& splitIter : splitOps)
                {
                prevSplitOp = &splitIter;
                if (splitIter.m_rangeEntries.size() < s_maximumPerNode && rangeIsContained (iter->m_range, splitIter.m_range) && (thisExtent >= splitIter.m_minimumExtent))
                    {
                    splitIter.m_rangeEntries.push_back (iter);
                    foundContainer = true;
                    break;
                    }
                }

            if (!foundContainer)
                {
                DRange3d        splitOpsRange;
                RangeEntryList  splitOpsList;
                splitOpsList.push_back (iter);
                splitOpsRange.ScaleAboutCenter (iter->m_range, s_newContainerRatio);
                splitOps.push_back (SplitOp(splitOpsRange, thisExtent * s_minimumExtentRatio, splitOpsList));
                prevSplitOp = &splitOps.back();
                }
            }
        }

    if (splitOps.empty())
        return ERROR;

    if (!noSplitOps.empty())
        {
        splitGraphics.push_back (XGraphicsContainer());
        XGraphicsSplitter::AddOpsToContainer (splitGraphics.back(), m_symbolIds, GetDataSize(), noSplitOps, matSymbs, transforms);
        }

    for (auto& iter : splitOps)
        {
        splitGraphics.push_back (XGraphicsContainer());
        XGraphicsSplitter::AddOpsToContainer (splitGraphics.back(), m_symbolIds, GetDataSize(), iter.m_rangeEntries, matSymbs, transforms);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::FlattenTransforms()
    {
    XGraphicsData flattened;
    flattened.reserve (GetDataSize());

    Transform       currentTransform;
    bvector<Transform> savedTransforms;
    TransformInfo   effectiveTransform;

    bool transformFound = false;

    for (byte *base = &m_buffer[0] + sizeof(XGraphicsHeader), *data = base, *end = &m_buffer[0] + m_buffer.size(); data < end;)
        {
        UInt16      opCode;
        UInt32      opSize;

        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, data, end))
            return ERROR;

        switch (opCode)
            {
            case XGRAPHIC_OpCode_PushTransClip:
                if (opSize - sizeof (opSize) > sizeof (Transform))
                    return ERROR;   // We don't currently make these operations with clips. If one is encountered, don't flatten.

                transformFound = true;
                savedTransforms.push_back(effectiveTransform.GetTransformR());
                memcpy (&currentTransform, data, sizeof (Transform));
                effectiveTransform.GetTransformR().InitProduct (effectiveTransform.GetTransformR(), currentTransform);
                break;
            case XGRAPHIC_OpCode_PopTransClip:
                if (savedTransforms.size() == 0)
                    return ERROR;   //  more pops than pushes

                effectiveTransform.GetTransformR() = savedTransforms.back();
                savedTransforms.pop_back();
                break;
            case XGRAPHIC_OpCode_PopAll:
                savedTransforms.clear();
                effectiveTransform.GetTransformR().InitIdentity();
                break;
            default:
                {
                XGraphicsData opData;
                opData.Append (data, opSize - sizeof (opSize));

                if (SUCCESS != XGraphicsOperations::OnTransform (opCode, effectiveTransform, opData))
                    return ERROR;

                UInt32 newOpSize = (UInt32)(opData.size() + sizeof (UInt32));

                flattened.Append (&opCode, sizeof (opCode));
                flattened.Append (&newOpSize, sizeof (newOpSize));
                flattened.Append (&opData[0], opData.size());
                break;
                }
            }
        data += opSize - sizeof (opSize);
        }

    if (!transformFound)
        return ERROR;

    BeAssert(savedTransforms.size() == 0);
    m_buffer.resize (flattened.size() + sizeof (XGraphicsHeader));
    memcpy (GetData() + sizeof (XGraphicsHeader), &flattened[0], flattened.size());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
XGraphicsContainer::ConsumeParsedResult XGraphicsContainer::ConsumeParsedDisplayParams(XGraphicsData& output, EditElementHandleR eeh, bvector <byte>& previousMatSymb, bool allowExtractDisplayParamsToHeader, bool useHeaderAllowed, UInt16 mask, ElemDisplayParamsR params, Int64 materialId)
    {
    ElemMatSymb dummy;
    XGraphicsContainer  temp;
    XGraphicsWriter     writer(&temp);
    //  The materialId is not used unless the bit is set in the mask
    writer.WriteMatSymb(mask, 0, 0, false, dummy, params, true, materialId);

    byte*tempData = &temp.m_buffer[0];
    byte*tempEnd = tempData + temp.m_buffer.size();

    UInt16      tempOpCode;
    UInt32      tempOpSize;

    XGraphicsOperations::GetOperation (tempOpCode, tempOpSize, tempData, tempEnd);

    if (!previousMatSymb.empty() && XGraphicsOperations::IsEqual (tempOpCode, tempData, &previousMatSymb[0], tempOpSize - sizeof (tempOpSize), 0.0))
        return ConsumeParsedResult::UsedCurrent;
    else if (allowExtractDisplayParamsToHeader && XGraphicsMatSymb::ExtractDisplayParamsToHeader (tempData, eeh))
        return ConsumeParsedResult::MovedToElement;
    else if (useHeaderAllowed && XGraphicsMatSymb::CompareDisplayParamsToHeader (tempData, eeh))
        return ConsumeParsedResult::UsedElement;

    previousMatSymb.resize (tempOpSize - sizeof (tempOpSize));
    memcpy (&previousMatSymb[0], tempData, tempOpSize - sizeof (tempOpSize));

    output.Append (&tempOpCode, sizeof (tempOpCode));
    output.Append (&tempOpSize, sizeof (tempOpSize));
    output.Append (tempData, tempOpSize - sizeof (tempOpSize));
    return ConsumeParsedResult::AppendedToContainer;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::RecomputeBranches (EditElementHandleR eeh)
    {
    DgnProjectP dgnProject = eeh.GetDgnProject();
    if (!dgnProject)
        return ERROR;

    bvector<Int32>     begins;
    bool updatedStream = false;

    for (byte *base = &m_buffer[0] + sizeof(XGraphicsHeader), *data = base, *end = &m_buffer[0] + m_buffer.size(); data < end;)
        {
        UInt16      opCode;
        UInt32      opSize;
        Int32       standardOffset = Int32(sizeof (opCode) + sizeof(opSize));
        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, data, end))
            return ERROR;

        bool startConditional = false;
        bool endConditional = false;
        switch(opCode)
            {
            case XGRAPHIC_OpCode_IfConditionalDraw:
                startConditional = true;
                break;

            case XGRAPHIC_OpCode_ElseIfConditionalDraw:
            case XGRAPHIC_OpCode_ElseConditionalDraw:
                startConditional = endConditional = true;
                break;

            case XGRAPHIC_OpCode_EndConditionalDraw:
                endConditional = true;
                break;
            }

        //  data points to where endSize is to be stored.  It is the 4 bytes following opSize.  The offset to be stored in 
        //  is the offset from this location to the beginning of the next thing.
        if (endConditional)
            {
            BeAssert(begins.size() > 0);
            if (begins.size() > 0)
                {
                Int32 startPoint = begins.back();
                begins.pop_back();
                Int32 newOffset = (Int32)(data - &m_buffer[0]) - startPoint - standardOffset;
                Int32 oldOffset;
                memcpy(&oldOffset, &m_buffer[startPoint], sizeof (oldOffset));
                if (newOffset != oldOffset)
                    {
                    memcpy(&m_buffer[startPoint], &newOffset, sizeof (newOffset));
                    updatedStream = true;
                    }
                }
            }

        if (startConditional)
            {
            begins.push_back(Int32(data-&m_buffer[0]));
            }

        data += opSize - sizeof (opSize);
        }

    return updatedStream ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     05/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::FlattenSymbology (EditElementHandleR eeh)
    {
    DgnProjectP dgnProject = eeh.GetDgnProject();
    if (!dgnProject)
        return ERROR;

    XGraphicsData flattened;
    flattened.reserve (GetDataSize());

    bool allowExtract = true, allowUseElemParams = true, matSymbFound = false;;

    bvector <byte> previousMatSymb;
    ParsedMatSymb   parsedMatSymb;

    for (byte *base = &m_buffer[0] + sizeof(XGraphicsHeader), *data = base, *end = &m_buffer[0] + m_buffer.size(); data < end;)
        {
        UInt16      opCode;
        UInt32      opSize;

        if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, data, end))
            return ERROR;

        if (XGRAPHIC_OpCode_MatSymb == opCode && XGraphicsMatSymb::CombineDisplayParamsWithParsed(parsedMatSymb, data) == BSISUCCESS)
            {
            matSymbFound = true;
            data += opSize - sizeof (opSize);
            continue;
            }

        if (parsedMatSymb.m_mask)
            {
            ConsumeParsedResult result = ConsumeParsedDisplayParams(flattened, eeh, previousMatSymb, allowExtract, allowUseElemParams, parsedMatSymb.m_mask, parsedMatSymb.m_params, parsedMatSymb.m_materialId);
            switch(result)
                {
                case ConsumeParsedResult::MovedToElement:
                case ConsumeParsedResult::UsedElement:
                    allowExtract = false;
                    break;

                case ConsumeParsedResult::UsedCurrent:
                    //  No change, but if there is a current then both of these values should be false;
                    BeAssert(!allowExtract);
                    BeAssert(!allowUseElemParams);
                    break;

                case ConsumeParsedResult::AppendedToContainer:
                    allowExtract = allowUseElemParams = false;
                    break;
                }
            }

        parsedMatSymb.Clear();
        allowExtract = false;

        flattened.Append (&opCode, sizeof (opCode));
        flattened.Append (&opSize, sizeof (opSize));
        flattened.Append (data, opSize - sizeof (opSize));

        data += opSize - sizeof (opSize);
        }

    if (!matSymbFound)
        return ERROR;

    m_buffer.resize (flattened.size() + sizeof (XGraphicsHeader));
    memcpy (GetData() + sizeof (XGraphicsHeader), &flattened[0], flattened.size());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::SplitSurfacesByPoleCount (std::list<XGraphicsContainer>& outContainers, UInt32 maxPole)
    {
    XGraphicsData   nonSurfaceOps;
    DrawEntryList   surfaceOps;
    OpEntryList     matSymbs;
    OpEntryList     transforms;

    if (SUCCESS != XGraphicsSplitter::SplitOperations (*this, surfaceOps, XGRAPHIC_OpCode_DrawBSplineSurface, nonSurfaceOps, matSymbs, transforms))
        return ERROR;

    bool maxPoleExceeded = false;

    for (auto& iter : surfaceOps)
        {
        MSBsplineSurface    surface;

        XGraphicsDrawBSplineSurface::Get (surface, iter.m_opEntry.m_opStart + sizeof (UInt16) + sizeof (UInt32), true);

        bvector <MSBsplineSurface> segments;

        if (SUCCESS != XGraphicsSplitter::SegmentSurface (segments, surface, maxPole))
            {
            outContainers.push_back (XGraphicsContainer());

            XGraphicsContainer& container = outContainers.back();
            container.BeginDraw();
            container.SetUseCache (true);

            size_t outPos = sizeof (XGraphicsHeader);

            for (int i = -1; i < iter.m_matSymbIndex; )
                XGraphicsSplitter::WriteOpEntry (container, outPos, matSymbs[++i]);

            for (int i = -1; i < iter.m_transformIndex; )
                XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[++i]);

            XGraphicsSplitter::WriteOpEntry (container, outPos, iter.m_opEntry);

            // Guarantee that we've written necessary transform stack pops.
            for (int i = iter.m_transformIndex + 1, limit = (int) transforms.size(); i < limit; ++i)
                XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[i]);
            }
        else
            {
            maxPoleExceeded = true;

            for (auto& surfaceIter : segments)
                {
                outContainers.push_back (XGraphicsContainer());

                XGraphicsContainer& container = outContainers.back();
                container.BeginDraw();
                container.SetUseCache (true);

                size_t outPos = sizeof (XGraphicsHeader);

                for (int i = -1; i < iter.m_matSymbIndex; )
                    XGraphicsSplitter::WriteOpEntry (container, outPos, matSymbs[++i]);

                for (int i = -1; i < iter.m_transformIndex; )
                    XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[++i]);

                XGraphicsWriter (&container).WriteBsplineSurface (surfaceIter);

                outPos = container.GetDataSize();

                // Guarantee that we've written necessary transform stack pops.
                for (int i = iter.m_transformIndex + 1, limit = (int) transforms.size(); i < limit; ++i)
                    XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[i]);
                }
            }

        }

    if (!maxPoleExceeded)
        return ERROR;

    m_buffer.resize (nonSurfaceOps.size() + sizeof (XGraphicsHeader));
    memcpy (GetData() + sizeof (XGraphicsHeader), &nonSurfaceOps[0], nonSurfaceOps.size());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::SplitMeshesByFaceCount (std::list<XGraphicsContainer>& splitMeshContainers, UInt32 maxFace)
    {
    XGraphicsData   nonMeshOps;
    DrawEntryList   meshOps;
    OpEntryList     matSymbs;
    OpEntryList     transforms;

    if (SUCCESS != XGraphicsSplitter::SplitOperations (*this, meshOps, XGRAPHIC_OpCode_AddIndexPolys, nonMeshOps, matSymbs, transforms))
        return ERROR;

    bool maxFaceExceeded = false;

    for (auto& iter : meshOps)
        {
        PolyfaceHeaderPtr mesh = XGraphicsAddIndexPolys::Get (iter.m_opEntry.m_opStart + sizeof (UInt16) + sizeof (UInt32),
                                                              iter.m_opEntry.m_opSize);

        if (!mesh.IsValid())
            continue;

        bvector <PolyfaceHeaderPtr> splitMeshes;
        if (!mesh->PartitionByXYRange (maxFace, 0, splitMeshes) || splitMeshes.size() < 2)
            {
            splitMeshContainers.push_back (XGraphicsContainer ());

            XGraphicsContainer& container = splitMeshContainers.back();
            container.BeginDraw();
            container.SetUseCache (true);
            container.SetIsRenderable (true);

            size_t outPos = sizeof (XGraphicsHeader);

            for (int i = -1; i < iter.m_matSymbIndex; )
                XGraphicsSplitter::WriteOpEntry (container, outPos, matSymbs[++i]);

            for (int i = -1; i < iter.m_transformIndex; )
                XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[++i]);

            XGraphicsSplitter::WriteOpEntry (container, outPos, iter.m_opEntry);

            // Guarantee that we've written necessary transform stack pops.
            for (int i = iter.m_transformIndex + 1, limit = (int) transforms.size(); i < limit; ++i)
                XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[i]);
            }
        else
            {
            maxFaceExceeded = true;

            for (auto& meshIter : splitMeshes)
                {
                splitMeshContainers.push_back (XGraphicsContainer ());

                XGraphicsContainer& container = splitMeshContainers.back();
                container.BeginDraw();
                container.SetUseCache (true);
                container.SetIsRenderable (true);

                size_t outPos = sizeof (XGraphicsHeader);

                for (int i = -1; i < iter.m_matSymbIndex; )
                    XGraphicsSplitter::WriteOpEntry (container, outPos, matSymbs[++i]);

                for (int i = -1; i < iter.m_transformIndex; )
                    XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[++i]);

                XGraphicsWriter (&container).WritePolyface (*meshIter.get(), true);

                outPos = container.GetDataSize();

                // Guarantee that we've written necessary transform stack pops.
                for (int i = iter.m_transformIndex + 1, limit = (int) transforms.size(); i < limit; ++i)
                    XGraphicsSplitter::WriteOpEntry (container, outPos, transforms[i]);
                }
            }
        }

    if (!maxFaceExceeded)
        return ERROR;

    m_buffer.resize (nonMeshOps.size() + sizeof (XGraphicsHeader));
    memcpy (GetData() + sizeof (XGraphicsHeader), &nonMeshOps[0], nonMeshOps.size());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsContainer::DropSymbolsToElement (EditElementHandleR eeh)
    {
    XGraphicsContainer container;
    if (SUCCESS != container.ExtractFromElement (eeh))
        return ERROR;

    container.SetElement (eeh.GetElementDescrCP()->GetElementRef());
    if (SUCCESS != container.Optimize (XGRAPHIC_OptimizeOptions_RemoveSymbols))
        return ERROR;

    container.SetUseCache (true);
    container.SetIsRenderable (true);

    return SUCCESS == container.ReplaceOnElement (eeh) ? SUCCESS : ERROR;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/14
+===============+===============+===============+===============+===============+======*/
struct XGraphicsConvertDimOutput : XGraphicsOutput
{
DEFINE_T_SUPER (XGraphicsOutput)

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurveVectorStroker : IStrokeForCache
    {
    private:

    CurveVectorCR   m_curves;
    bool            m_is3d;

    protected:

    virtual void _StrokeForCache (CachedDrawHandleCR drawHandle, ViewContextR context, double pixelSize) override
        {
        if (m_is3d)
            context.GetIDrawGeom ().DrawCurveVector (m_curves, true);
        else
            context.GetIDrawGeom ().DrawCurveVector2d (m_curves, true, 0.0);
        }

    public:

    CurveVectorStroker (CurveVectorCR curves, bool is3d) : m_curves (curves), m_is3d (is3d) {}

    }; // CurveVectorStroker

protected:

bool            m_convert2d;
Transform       m_flattenTrans;
Transform       m_flattenTransCurrent; // Adjusted for top of stack transforms...
DVec3d          m_flattenDir;
double          m_elevation;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
TransformCR GetCurrentFlattenTransform ()
    {
    TransformCP currentTrans = m_viewContext->GetCurrLocalToFrustumTransformCP ();

    if (NULL != currentTrans)
        {
        Transform   invCurrTrans;

        invCurrTrans.InverseOf (*currentTrans);
        m_flattenTransCurrent = Transform::FromProduct (invCurrTrans, m_flattenTrans, *currentTrans);
        }
    else
        {
        m_flattenTransCurrent = m_flattenTrans;
        }

    return m_flattenTransCurrent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertPoints (std::valarray<DPoint2d>& localPoints2dBuf, DPoint3dCP points, int numPoints)
    {
    for (int iPt = 0; iPt < numPoints; ++iPt)
        {
        DPoint3d    tmpPt = points[iPt];

        GetCurrentFlattenTransform ().Multiply (tmpPt);
        localPoints2dBuf[iPt].x = tmpPt.x;
        localPoints2dBuf[iPt].y = tmpPt.y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertPoints (std::valarray<DPoint3d>& localPoints3dBuf, DPoint2dCP points, int numPoints)
    {
    for (int iPt = 0; iPt < numPoints; ++iPt)
        {
        DPoint2dCP  tmpPt = &points[iPt];

        localPoints3dBuf[iPt].x = tmpPt->x;
        localPoints3dBuf[iPt].y = tmpPt->y;
        localPoints3dBuf[iPt].z = m_elevation;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool DrawFlattenedArcAsLine (DEllipse3dCR ellipse, double zDepth)
    {
    DPoint3d    origin;
    RotMatrix   rMatrix;
    double      r0, r1, start, sweep;

    ellipse.GetScaledRotMatrix (origin, rMatrix, r0, r1, start, sweep);

    // see if the transformation has reduced the arc to a line
    if (fabs (r0) < 1.0e-6 * fabs (r1))
        {
        double      min, max;
        DPoint2d    segment[2];

        min = sin (start);
        max = sin (start + sweep);

        if (min > max)
            std::swap (min, max);

        if (in_span (msGeomConst_piOver2, start, sweep))
            max = 1.0;

        if (in_span (msGeomConst_pi + msGeomConst_piOver2, start, sweep))
            min = -1.0;

        segment[0].x = origin.x + min * rMatrix.form3d[0][1] * r1;
        segment[0].y = origin.y + min * rMatrix.form3d[1][1] * r1;
        segment[1].x = origin.x + max * rMatrix.form3d[0][1] * r1;
        segment[1].y = origin.y + max * rMatrix.form3d[1][1] * r1;

        T_Super::_DrawLineString2d (2, segment, 0.0, NULL);

        return true;
        }
    else if (fabs (r1) < 1.0e-6 * fabs (r0))
        {
        double      min, max;
        DPoint2d    segment[2];

        min = cos (start);
        max = cos (start + sweep);

        if (min > max)
            std::swap (min, max);

        if (in_span (0.0, start, sweep))
            max = 1.0;

        if (in_span (msGeomConst_pi, start, sweep))
            min = -1.0;

        segment[0].x = origin.x + min * rMatrix.form3d[0][0] * r0;
        segment[0].y = origin.y + min * rMatrix.form3d[1][0] * r0;
        segment[1].x = origin.x + max * rMatrix.form3d[0][0] * r0;
        segment[1].y = origin.y + max * rMatrix.form3d[1][0] * r0;

        T_Super::_DrawLineString2d (2, segment, 0.0, NULL);

        return true;
        }
    else
        {
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawLineString3d (numPoints, points, range);

    std::valarray<DPoint2d>  localPoints2dBuf (numPoints);

    ConvertPoints (localPoints2dBuf, points, numPoints);

    T_Super::_DrawLineString2d (numPoints, &localPoints2dBuf[0], 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawLineString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override
    {
    if (IsConvert2d ())
        return T_Super::_DrawLineString2d (numPoints, points, zDepth, range);

    std::valarray<DPoint3d>  localPoints3dBuf (numPoints);

    ConvertPoints (localPoints3dBuf, points, numPoints);
        
    T_Super::_DrawLineString3d (numPoints, &localPoints3dBuf[0], NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawPointString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawPointString3d (numPoints, points, range);

    std::valarray<DPoint2d>  localPoints2dBuf (numPoints);

    ConvertPoints (localPoints2dBuf, points, numPoints);

    T_Super::_DrawPointString2d (numPoints, &localPoints2dBuf[0], 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawPointString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override
    {
    if (IsConvert2d ())
        return T_Super::_DrawPointString2d (numPoints, points, zDepth, range);

    std::valarray<DPoint3d>  localPoints3dBuf (numPoints);

    ConvertPoints (localPoints3dBuf, points, numPoints);
        
    T_Super::_DrawPointString3d (numPoints, &localPoints3dBuf[0], NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawShape3d (numPoints, points, filled, range);

    std::valarray<DPoint2d>  localPoints2dBuf (numPoints);

    ConvertPoints (localPoints2dBuf, points, numPoints);

    T_Super::_DrawShape2d (numPoints, &localPoints2dBuf[0], filled, 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawShape2d (int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) override
    {
    if (IsConvert2d ())
        return T_Super::_DrawShape2d (numPoints, points, filled, zDepth, range);

    std::valarray<DPoint3d>  localPoints3dBuf (numPoints);

    ConvertPoints (localPoints3dBuf, points, numPoints);
        
    T_Super::_DrawShape3d (numPoints, &localPoints3dBuf[0], filled, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTriStrip3d (int numPoints, DPoint3dCP points, Int32 usageFlags, DPoint3dCP range) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawTriStrip3d (numPoints, points, usageFlags, range);

    std::valarray<DPoint2d>  localPoints2dBuf (numPoints);

    ConvertPoints (localPoints2dBuf, points, numPoints);

    T_Super::_DrawTriStrip2d (numPoints, &localPoints2dBuf[0], usageFlags, 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTriStrip2d (int numPoints, DPoint2dCP points, Int32 usageFlags, double zDepth, DPoint2dCP range) override
    {
    if (IsConvert2d ())
        return T_Super::_DrawTriStrip2d (numPoints, points, usageFlags, zDepth, range);

    std::valarray<DPoint3d>  localPoints3dBuf (numPoints);

    ConvertPoints (localPoints3dBuf, points, numPoints);
        
    T_Super::_DrawTriStrip3d (numPoints, &localPoints3dBuf[0], usageFlags, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawArc3d (DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawArc3d (ellipse, isEllipse, filled, range);

    DEllipse3d  ellipse2d = ellipse;

    GetCurrentFlattenTransform ().Multiply (ellipse2d, ellipse2d);

    if (DrawFlattenedArcAsLine (ellipse2d, 0.0))
        return;

    T_Super::_DrawArc2d (ellipse2d, isEllipse, filled, 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) override
    {
    if (IsConvert2d ())
        return T_Super::_DrawArc2d (ellipse, isEllipse, filled, zDepth, range);

    DEllipse3d  ellipse3d = ellipse;

    ellipse3d.center.z = m_elevation;

    T_Super::_DrawArc3d (ellipse3d, isEllipse, filled, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawBSplineCurve (MSBsplineCurveCR curve, bool filled) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawBSplineCurve (curve, filled);

    MSBsplineCurvePtr  tmpCurve = curve.CreateCopyTransformed (GetCurrentFlattenTransform ());

    T_Super::_DrawBSplineCurve2d (*tmpCurve, filled, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawBSplineCurve2d (MSBsplineCurveCR curve, bool filled, double zDepth) override
    {
    // NOTE: I don't expect this to be called in Vancouver, couldn't add XGRAPHIC_OpCode_DrawBSplineCurve2d since it wouldn't be compatibly with SS3...
    if (IsConvert2d ())
        return T_Super::_DrawBSplineCurve2d (curve, filled, zDepth);

    if (0.0 == m_elevation)
        return T_Super::_DrawBSplineCurve (curve, filled);

    MSBsplineCurvePtr  tmpCurve = curve.CreateCopy ();

    for (int iPoint = 0; iPoint < tmpCurve->params.numPoles; ++iPoint)
        {
        DPoint3d    xyz = tmpCurve->GetPole (iPoint);

        if (tmpCurve->rational && NULL != tmpCurve->GetWeightCP ())
            xyz.z = m_elevation * tmpCurve->GetWeight (iPoint);
        else
            xyz.z = m_elevation;

        tmpCurve->SetPole (iPoint, xyz);
        }

    return T_Super::_DrawBSplineCurve (*tmpCurve, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawSolidPrimitive (ISolidPrimitiveCR primitive) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawSolidPrimitive (primitive);

    WireframeGeomUtil::Draw (primitive, *m_viewContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawBSplineSurface (MSBsplineSurfaceCR surface) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawBSplineSurface (surface);

    WireframeGeomUtil::Draw (surface, *m_viewContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _DrawBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments, double pixelSize) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawBody (entity, attachments, pixelSize);

    WireframeGeomUtil::Draw (entity, *m_viewContext, attachments);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawPolyface (PolyfaceQueryCR meshData, bool filled) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawPolyface (meshData, filled);

    PolyfaceHeaderPtr   tmpMeshData = PolyfaceHeader::New ();

    tmpMeshData->CopyFrom (meshData);
    tmpMeshData->Transform (GetCurrentFlattenTransform ());

    return T_Super::_DrawPolyface (*tmpMeshData, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTextString (TextStringCR text, double* zDepth = NULL) override
    {
    if (IsConvert3d () && !text.Is3d ())
        {
        DPoint3d                 lowerLeft = text.GetOrigin ();
        RotMatrix                rMatrix = text.GetRotMatrix ();
        TextStringPropertiesPtr  props = text.GetProperties ().Clone ();

        lowerLeft.z = m_elevation;
        props->SetIs3d (true);

        TextStringPtr   tmpText = TextString::Create (text.GetString ().c_str (), &lowerLeft, &rMatrix, *props);

        return T_Super::_DrawTextString (*tmpText, zDepth);
        }
    else if (IsConvert2d () && text.Is3d ())
        {
        DPoint2d                 scaleFactor;
        DPoint3d                 lowerLeft = text.GetOrigin ();
        RotMatrix                rMatrix = text.GetRotMatrix ();
        TextStringPropertiesPtr  props = text.GetProperties ().Clone ();

        GetCurrentFlattenTransform ().Multiply (lowerLeft);
        TextString::TransformOrientationAndGetScale (scaleFactor, rMatrix, NULL, GetCurrentFlattenTransform (), false);
        props->ApplyScale (scaleFactor);
        props->SetIs3d (false);

        TextStringPtr   tmpText = TextString::Create (text.GetString ().c_str (), &lowerLeft, &rMatrix, *props);

        return T_Super::_DrawTextString (*tmpText, zDepth);
        }
    else
        {
        T_Super::_DrawTextString (text, zDepth);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAreaPattern (ElementHandleCR thisElm, ViewContext::ClipStencil& boundary, ViewContext::PatternParamSource& source) override
    {
    if (IsConvert3d ())
        return T_Super::_DrawAreaPattern (thisElm, boundary, source);

    int                 patternIndex;
    DPoint3d            origin;
    PatternParamsP      params;
    DwgHatchDefLineP    hatchLines;

    if (NULL == (params = source.GetParams (thisElm, &origin, &hatchLines, &patternIndex, m_viewContext)))
        return;

    CurveVectorPtr      curveVector = boundary.GetCurveVector (thisElm);

    if (!curveVector.IsValid ())
        return;

    CurveVectorPtr      tmpCurveVector = curveVector->Clone ();
    PatternParamsPtr    tmpParams = PatternParams::CreateFromExisting (*params);

    tmpCurveVector->TransformInPlace (GetCurrentFlattenTransform ()); // Pre-flatten since range gets computed from curves and saved before drawing curves...
#if defined (NEEDS_WORK_DGNITEM)
    PatternLinkageUtil::Transform (*tmpParams, hatchLines, GetCurrentFlattenTransform (), true, true); // Just transform hatchLines in place, huge local in XGraphicsDrawAreaPattern::_Draw...
#endif
    GetCurrentFlattenTransform ().Multiply (origin);
    tmpParams->origin = origin;

    CurveVectorStroker               stroker (*tmpCurveVector, false);
    ViewContext::ClipStencil         tmpBoundary (stroker, 1);
    ViewContext::PatternParamSource  tmpSource (tmpParams.get (), hatchLines, patternIndex);

    return T_Super::_DrawAreaPattern (thisElm, tmpBoundary, tmpSource);
    }

public:

XGraphicsConvertDimOutput (XGraphicsContainer& container) : XGraphicsOutput (container) {}

bool IsConvert2d () {return m_convert2d;}
bool IsConvert3d () {return !m_convert2d;}

bool GetConvertTo2d (TransformR flattenTrans, DVec3dR flattenDir) {flattenTrans = m_flattenTrans; flattenDir = m_flattenDir; return IsConvert2d ();}
bool GetConvertTo3d (double& elevation) {elevation = m_elevation; return IsConvert3d ();}

void SetConvertTo2d (TransformCR flattenTrans, DVec3dCR flattenDir) {m_convert2d = true; m_flattenTrans = flattenTrans; m_flattenDir = flattenDir;}
void SetConvertTo3d (double elevation) {m_convert2d = false; m_elevation = elevation;}

}; // XGraphicsConvertDimOutput

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/14
+===============+===============+===============+===============+===============+======*/
struct XGraphicsConvertDimContext : XGraphicsContext
{
DEFINE_T_SUPER (XGraphicsContext)

protected:

XGraphicsConvertDimOutput   m_convertOutput;

virtual XGraphicsOutput& _GetOutput () override {return m_convertOutput;}

public:

XGraphicsConvertDimContext (XGraphicsContainer& container) : XGraphicsContext (container), m_convertOutput (container)
    {
    _GetOutput ().SetViewContext (this);
    _SetupOutputs ();
    }

void SetConvertTo2d (TransformCR flattenTrans, DVec3dCR flattenDir) {m_convertOutput.SetConvertTo2d (flattenTrans, flattenDir);}
void SetConvertTo3d (double elevation) {m_convertOutput.SetConvertTo3d (elevation);}

}; // XGraphicsConvertDimContext

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsContainer::ConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    XGraphicsContainer  container;

    if (SUCCESS != container.ExtractFromElement (eeh))
        return;

    XGraphicsContainer          convertedData;
    XGraphicsConvertDimContext  context (convertedData);

    context.SetConvertTo2d (flattenTrans, flattenDir);
    context.SetDgnProject (*eeh.GetDgnProject ());
    context.CookElemDisplayParams (eeh); // Establish initial symbology...

    convertedData.BeginDraw ();
    container.Draw (context, DRAW_OPTION_Default);

    convertedData.AddToElement (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsContainer::ConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    XGraphicsContainer container;

    if (SUCCESS != container.ExtractFromElement (eeh))
        return;

    XGraphicsContainer          convertedData;
    XGraphicsConvertDimContext  context (convertedData);

    context.SetConvertTo3d (elevation);
    context.SetDgnProject (*eeh.GetDgnProject ());
    context.CookElemDisplayParams (eeh); // Establish initial symbology...

    convertedData.BeginDraw ();
    container.Draw (context, DRAW_OPTION_Default);

    convertedData.AddToElement (eeh);
    }

