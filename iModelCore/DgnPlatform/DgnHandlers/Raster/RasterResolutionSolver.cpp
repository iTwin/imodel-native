/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Raster/RasterResolutionSolver.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

// Unit conversion factors.
#define MM_PER_M                            1000.0
#define MM_PER_DM                           100.0
#define MM_PER_CM                           10.0
#define MM_PER_IN                           25.4

#define HGLOBAL_EPSILON (1.0E-14)
#define HDOUBLE_EQUAL_EPSILON(v1,v2)  ((v1 <= (v2+HGLOBAL_EPSILON)) && (v1 >= (v2-HGLOBAL_EPSILON)))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RasterResolutionPtr RasterResolution::Create(DgnModelP dgnCache)
    {
    return new RasterResolution(dgnCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RasterResolutionPtr RasterResolution::CreateFromRasterAttachment(ElementHandleCR eh,UInt64 nbOfPixelX,UInt64 nbOfPixelY)
    {
    RasterFrameHandler* pQuery = dynamic_cast<RasterFrameHandler*>(&eh.GetHandler());
    if(NULL == pQuery)
        return NULL; //It is NOT a RasterAttachment!

    Transform trans;
    RasterTransformFacility::SetUV(trans,pQuery->GetU(eh),pQuery->GetV(eh));
    RasterTransformFacility::SetUV(trans,pQuery->GetU(eh),pQuery->GetV(eh));
    RasterTransformFacility::SetTranslation(trans,pQuery->GetOrigin(eh));

    RasterResolutionPtr pResolution = RasterResolution::Create(eh.GetDgnModelP());

    pResolution->SetNbOfPixelX(nbOfPixelX);
    pResolution->SetNbOfPixelY(nbOfPixelY);

    DPoint2d dpi(pQuery->GetScanningResolution(eh));
    if (dpi.x<1)
        dpi.x = 300; //default value, 0 give error when used to compute scaling
    if (dpi.y<1)
        dpi.y = 300; //default value, 0 give error when used to compute scaling
    pResolution->SetDPI(dpi);

    DPoint2d pixelSize;
    pixelSize.x = RasterTransformFacility::GetScalingX(trans);
    pixelSize.y = RasterTransformFacility::GetScalingY(trans);
    pResolution->SetPixelSize(pixelSize);

    bool aspectRatioLock(true);
    if(!HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y))
        aspectRatioLock = false;
    pResolution->SetAspectRatioLockState(aspectRatioLock);

    DPoint2d extent(pQuery->GetExtent(eh));
    pResolution->SetExtent(extent);

    IResolutionSolverStrategyPtr pStrategy = ResolutionSolverFromPixelSizeAndDPI::Create();
    pResolution->SetResolutionSolverStrategy(pStrategy);
    pResolution->ComputeResolution(IResolutionSolverStrategy::UseXAxis);

    return pResolution;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
RasterResolution::RasterResolution(DgnModelP dgnCache)
:m_aspectRatioLock(false),
m_sheetUnit(UNITS_UOR)
    {
    // Get conversion between meters and inches
    UnitDefinition  unitDefMeter = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    UnitDefinition  unitDefInches = UnitDefinition::GetStandardUnit (StandardUnit::EnglishInches);
    double InchesPerMeter;
    unitDefInches.GetConversionFactorFrom(InchesPerMeter, unitDefMeter);
    double metersPerInches (1.0/InchesPerMeter);

    //Set UOR conversion factor in s_scaleToInches 
    double UORPerMeters = 1000.;
    double UORPerInches = UORPerMeters * metersPerInches;
    m_inchesPerUOR = 1.0 / UORPerInches;

    m_nbOfPixelX = 100;
    m_nbOfPixelY = 100;
    m_DPI.x = 1.0;
    m_DPI.y = 1.0;
    m_pixelSize.x = 1.0;
    m_pixelSize.y = 1.0;

    if(!HDOUBLE_EQUAL_EPSILON(m_pixelSize.x,m_pixelSize.y))
        m_aspectRatioLock = false;

    m_extent.x = m_pixelSize.x * m_nbOfPixelX;
    m_extent.y = m_pixelSize.y * m_nbOfPixelY;

    m_scale.x = m_pixelSize.x * m_DPI.x;
    m_scale.y = m_pixelSize.y * m_DPI.y;
    m_sheetSize.x = m_nbOfPixelX / m_DPI.x;
    m_sheetSize.y = m_nbOfPixelY / m_DPI.y;
    
    //Default strategy
    IResolutionSolverStrategyPtr pStrategy = ResolutionSolverFromPixelSizeAndDPI::Create();
    SetResolutionSolverStrategy(pStrategy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
RasterResolution::~RasterResolution()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterResolution::SetInRasterAttachment(EditElementHandleR eeh) const
    {
    RasterFrameHandler* pEdit = dynamic_cast<RasterFrameHandler*>(&eeh.GetHandler());
    if(NULL == pEdit)
        return; //It is NOT a RasterAttachment!

    pEdit->SetExtent(eeh,m_extent,true);
    pEdit->SetScanningResolution(eeh,GetDPI());
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterResolution::SetResolutionSolverStrategy(IResolutionSolverStrategyPtr& value)  
    {
    m_pResolutionSolverStrategy = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
double RasterResolution::ConvertUnits   /* <= converted value */
(
double            value,                      /* => value to convert */
SheetSizeUnit     fromUnits,                  /* => value's current units (SheetSizeUnit) */
SheetSizeUnit     toUnits                     /* => desired units (SheetSizeUnit) */
) const
    {
    static double s_scaleToInches[] =
        {
        1.0,                        // in / UOR
        MM_PER_M / MM_PER_IN,       // in / m
        MM_PER_DM / MM_PER_IN,      // in / dm
        MM_PER_CM / MM_PER_IN,      // in / cm
        1 / MM_PER_IN,              // in / mm
        12.0,                       // in / ft
        1.0,                        // in / in
        };

    if (fromUnits == toUnits)
        return value;

    s_scaleToInches[UNITS_UOR] = m_inchesPerUOR;

    // Convert the input value to inches, then to the desired units.
    return (value * s_scaleToInches[fromUnits]) / s_scaleToInches[toUnits];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d RasterResolution::GetSheetSize() const                        
    {
    DPoint2d sheetSize;
    //Use sheet size units to convert from internal value which is always in UOR
    sheetSize.x = ConvertUnits(m_sheetSize.x,UNITS_UOR,m_sheetUnit);
    sheetSize.y = ConvertUnits(m_sheetSize.y,UNITS_UOR,m_sheetUnit);
    return sheetSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterResolution::SetSheetSize(const DPoint2d& value)                
    {
    DPoint2d sheetSize;
    //Use sheet size units to convert to internal value which is always in UOR
    sheetSize.x = ConvertUnits(value.x,m_sheetUnit,UNITS_UOR);
    sheetSize.y = ConvertUnits(value.y,m_sheetUnit,UNITS_UOR);

    m_sheetSize = sheetSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d RasterResolution::GetDPI() const 
    {
    DPoint2d dpi;
    //Convert from internal value which is always in UOR to inches
    dpi.x = m_DPI.x / m_inchesPerUOR;
    dpi.y = m_DPI.y / m_inchesPerUOR;

    return dpi;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterResolution::SetDPI(const DPoint2d & value)              
    {
    DPoint2d dpiUOR;
    //Convert to internal value which is always in UOR from inches
    dpiUOR.x = value.x * m_inchesPerUOR;
    dpiUOR.y = value.y * m_inchesPerUOR;

    m_DPI = dpiUOR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromPixelSizeAndDPI::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());
    if (fromAxis == UseXAxis) 
        {
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }
    
    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d scale;
    scale.x = pixelSize.x * resolution.GetInternalDPI().x;
    scale.y = pixelSize.y * resolution.GetInternalDPI().y;
    resolution.SetScale(scale);

    DPoint2d sheetSizeUor;
    sheetSizeUor.x = resolution.GetNbOfPixelX() / resolution.GetInternalDPI().x;
    sheetSizeUor.y = resolution.GetNbOfPixelY() / resolution.GetInternalDPI().y;
    resolution.SetInternalSheetSize(sheetSizeUor);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromPixelSizeAndScale::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());

    if (fromAxis == UseXAxis)
        {
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x;
        }
    else
        {
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }

    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d sheetSizeUor;
    sheetSizeUor.x = extent.x / resolution.GetScale().x;
    sheetSizeUor.y = extent.y / resolution.GetScale().y;
    resolution.SetInternalSheetSize(sheetSizeUor);

    DPoint2d dpiUOR;
    dpiUOR.x = resolution.GetNbOfPixelX() / sheetSizeUor.x;
    dpiUOR.y = resolution.GetNbOfPixelY() / sheetSizeUor.y;
    resolution.SetInternalDPI(dpiUOR);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromPixelSizeAndSheetSize::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());

    if (fromAxis == UseXAxis)
        {
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }

    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d dpiUOR;
    dpiUOR.x = resolution.GetNbOfPixelX() / resolution.GetInternalSheetSize().x;
    dpiUOR.y = resolution.GetNbOfPixelY() / resolution.GetInternalSheetSize().y;
    resolution.SetInternalDPI(dpiUOR);

    DPoint2d scale;
    scale.x = extent.x  / resolution.GetInternalSheetSize().x;
    scale.y = extent.y  / resolution.GetInternalSheetSize().y;
    resolution.SetScale(scale);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromExtentAndDPI::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());

    if (fromAxis == UseXAxis)
        {
        pixelSize.x = resolution.GetExtent().x / resolution.GetNbOfPixelX();
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        pixelSize.y = resolution.GetExtent().y / resolution.GetNbOfPixelY();
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }

    //Now recompute the world area snapped to upper pixel boundary
    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d scale;
    scale.x = pixelSize.x * resolution.GetInternalDPI().x;
    scale.y = pixelSize.y * resolution.GetInternalDPI().y;
    resolution.SetScale(scale);

    DPoint2d sheetSizeUor;
    sheetSizeUor.x = resolution.GetNbOfPixelX() / resolution.GetInternalDPI().x;
    sheetSizeUor.y = resolution.GetNbOfPixelY() / resolution.GetInternalDPI().y;
    resolution.SetInternalSheetSize(sheetSizeUor);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromExtentAndScale::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());

    if (fromAxis == UseXAxis)
        {
        pixelSize.x = resolution.GetExtent().x / resolution.GetNbOfPixelX();
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        pixelSize.y = resolution.GetExtent().y / resolution.GetNbOfPixelY();
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }
    //Now recompute the world area snapped to upper pixel boundary
    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d sheetSizeUor;
    sheetSizeUor.x = extent.x / resolution.GetScale().x;
    sheetSizeUor.y = extent.y / resolution.GetScale().y;
    resolution.SetInternalSheetSize(sheetSizeUor);

    DPoint2d dpiUOR;
    dpiUOR.x = resolution.GetNbOfPixelX() / resolution.GetInternalSheetSize().x;
    dpiUOR.y = resolution.GetNbOfPixelY() / resolution.GetInternalSheetSize().y;
    resolution.SetInternalDPI(dpiUOR);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromExtentAndSheetSize::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());

    if (fromAxis == UseXAxis)
        {
        pixelSize.x = resolution.GetExtent().x / resolution.GetNbOfPixelX();
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        pixelSize.y = resolution.GetExtent().y / resolution.GetNbOfPixelY();
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }
    //Now recompute the world area snapped to upper pixel boundary
    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d dpiUOR;
    dpiUOR.x = resolution.GetNbOfPixelX() / resolution.GetInternalSheetSize().x;
    dpiUOR.y = resolution.GetNbOfPixelY() / resolution.GetInternalSheetSize().y;
    resolution.SetInternalDPI(dpiUOR);

    DPoint2d scale;
    scale.x = extent.x / resolution.GetInternalSheetSize().x;
    scale.y = extent.y / resolution.GetInternalSheetSize().y;
    resolution.SetScale(scale);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromDPIAndScale::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());

    if (fromAxis == UseXAxis)
        {
        pixelSize.x = resolution.GetScale().x / resolution.GetInternalDPI().x; 
        if (resolution.GetAspectRatioLockState())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        pixelSize.y = resolution.GetScale().y / resolution.GetInternalDPI().y; 
        if (resolution.GetAspectRatioLockState())
            pixelSize.x = pixelSize.y; 
        }
    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d sheetSizeUor;
    sheetSizeUor.x = extent.x / resolution.GetScale().x;
    sheetSizeUor.y = extent.y / resolution.GetScale().y;
    resolution.SetInternalSheetSize(sheetSizeUor);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolutionSolverFromDPIAndSheetSize::ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)
    {
    DPoint2d pixelSize(resolution.GetPixelSize());
    DPoint2d sheetSizeUor(resolution.GetInternalSheetSize());

    //Aspect ratio is always respected and pixel are always square
    if (fromAxis == UseXAxis)
        {
        //resolution.GetInternalDPI().y = resolution.GetInternalDPI().x / GetAspectRatio();
        sheetSizeUor.y = sheetSizeUor.x / resolution.GetAspectRatio();
        pixelSize.x = resolution.GetExtent().x / ceil((resolution.GetInternalDPI().x * resolution.GetInternalSheetSize().x)-HGLOBAL_EPSILON); 
        if (resolution.GetAspectRatio())
            pixelSize.y = pixelSize.x; 
        }
    else
        {
        //resolution.GetInternalDPI().x = resolution.GetInternalDPI().y * GetAspectRatio(); 
        sheetSizeUor.x = sheetSizeUor.y * resolution.GetAspectRatio();
        pixelSize.y = resolution.GetExtent().y / ceil((resolution.GetInternalDPI().y * resolution.GetInternalSheetSize().y)-HGLOBAL_EPSILON); 
        if (resolution.GetAspectRatio())
            pixelSize.x = pixelSize.y; 
        }
    resolution.SetInternalSheetSize(sheetSizeUor);

    DPoint2d extent;
    extent.x = pixelSize.x * resolution.GetNbOfPixelX();
    extent.y = pixelSize.y * resolution.GetNbOfPixelY();
    resolution.SetExtent(extent);

    DPoint2d scale;
    scale.x = extent.x / sheetSizeUor.x;
    scale.y = extent.y / sheetSizeUor.y;
    resolution.SetScale(scale);

    BeAssert(resolution.GetAspectRatioLockState() ? HDOUBLE_EQUAL_EPSILON(pixelSize.x,pixelSize.y) : true);
    resolution.SetPixelSize(pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void RasterResolution::ComputeResolution
(
IResolutionSolverStrategy::CoordinateAxis  fromAxis
)
    {
    GetResolutionSolverStrategy()->ComputeResolution(*this,fromAxis);
    }

