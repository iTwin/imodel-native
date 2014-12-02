/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/RasterResolutionSolver.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#if defined (NEEDS_WORK_DGNITEM)
DGNPLATFORM_TYPEDEFS(RasterResolution)
DGNPLATFORM_REF_COUNTED_PTR(RasterResolution)

DGNPLATFORM_TYPEDEFS (IResolutionSolverStrategy)
DGNPLATFORM_REF_COUNTED_PTR (IResolutionSolverStrategy)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
//! Strategy interface to compute Raster Resolution.
// @bsiclass                                                       Marc.Bedard    02/2012
struct  IResolutionSolverStrategy : public IRefCounted
{
    //__PUBLISH_CLASS_VIRTUAL__
public:
    enum CoordinateAxis
        {
        UseXAxis  = 0,
        UseYAxis,
        };

    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis)=0;
};


/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromPixelSizeAndDPI : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromPixelSizeAndDPI()    {}
    ~ResolutionSolverFromPixelSizeAndDPI()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromPixelSizeAndDPI();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromPixelSizeAndDPI

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromPixelSizeAndScale : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromPixelSizeAndScale()    {}
    ~ResolutionSolverFromPixelSizeAndScale()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromPixelSizeAndScale();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromPixelSizeAndScale

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromPixelSizeAndSheetSize : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromPixelSizeAndSheetSize()    {}
    ~ResolutionSolverFromPixelSizeAndSheetSize()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromPixelSizeAndSheetSize();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromPixelSizeAndSheetSize

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromExtentAndDPI : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromExtentAndDPI()    {}
    ~ResolutionSolverFromExtentAndDPI()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromExtentAndDPI();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromExtentAndDPI

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromExtentAndScale : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromExtentAndScale()    {}
    ~ResolutionSolverFromExtentAndScale()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromExtentAndScale();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromExtentAndScale

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromExtentAndSheetSize : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromExtentAndSheetSize()    {}
    ~ResolutionSolverFromExtentAndSheetSize()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromExtentAndSheetSize();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromExtentAndSheetSize

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromDPIAndScale : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromDPIAndScale()    {}
    ~ResolutionSolverFromDPIAndScale()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromDPIAndScale();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromDPIAndScale

/*=================================================================================**//**
* @bsiclass                                                     Marc.Bedard     02/2012
+===============+===============+===============+===============+===============+======*/
struct ResolutionSolverFromDPIAndSheetSize : public RefCounted<IResolutionSolverStrategy>
{
private:
    ResolutionSolverFromDPIAndSheetSize()    {}
    ~ResolutionSolverFromDPIAndSheetSize()   {}

public:
    DGNPLATFORM_EXPORT static IResolutionSolverStrategyPtr Create() {return new ResolutionSolverFromDPIAndSheetSize();}

    //IResolutionSolverStrategy interface implementation
    virtual void ComputeResolution(RasterResolutionR resolution, CoordinateAxis fromAxis);

}; // ResolutionSolverFromDPIAndSheetSize

/*=================================================================================**//**
* @bsiclass                                                     MarcBedard    01/2012
+===============+===============+===============+===============+===============+======*/
struct RasterResolution : public RefCountedBase
    {
    public:

        //Don't change this number without editing ConvertUnits method
         enum SheetSizeUnit
            {
            UNITS_UOR = 0,
            UNITS_CM  = 3,
            UNITS_MM  = 4,
            UNITS_IN  = 6,
            };

        /*----------------------------------------------------------------------------+
        |   Public Member Functions
        +----------------------------------------------------------------------------*/
        DGNPLATFORM_EXPORT static RasterResolutionPtr Create(DgnModelP dgnCache);
        DGNPLATFORM_EXPORT static RasterResolutionPtr CreateFromRasterAttachment(ElementHandleCR eh,UInt64 nbOfPixelX,UInt64 nbOfPixelY);

        /*----------------------------------------------------------------------------+
        |       settings getter / setter
        +----------------------------------------------------------------------------*/
        DGNPLATFORM_EXPORT void SetInRasterAttachment(EditElementHandleR eeh) const;

        IResolutionSolverStrategyPtr  GetResolutionSolverStrategy() const                  {return m_pResolutionSolverStrategy;}
        DGNPLATFORM_EXPORT void                  SetResolutionSolverStrategy(IResolutionSolverStrategyPtr& value);

        DPoint2dCR     GetExtent() const                            {return m_extent;}
        void           SetExtent(const DPoint2d & value)            {m_extent = value;}

        DPoint2dCR     GetPixelSize() const                         {return m_pixelSize;}
        void           SetPixelSize(const DPoint2d & value)         {m_pixelSize = value;}

        bool           GetAspectRatioLockState() const              {return m_aspectRatioLock;}
        void           SetAspectRatioLockState(const bool value)    {m_aspectRatioLock = value;}

        const UInt64& GetNbOfPixelX() const                     {return m_nbOfPixelX;}
        const UInt64& GetNbOfPixelY() const                     {return m_nbOfPixelY;}
        void          SetNbOfPixelX(const UInt64& value)        {m_nbOfPixelX = value;}
        void          SetNbOfPixelY(const UInt64& value)        {m_nbOfPixelY = value;}

        DGNPLATFORM_EXPORT DPoint2d       GetDPI() const;
        DGNPLATFORM_EXPORT void           SetDPI(const DPoint2d & value);

        //Values are in UORs
        DPoint2dCR     GetInternalDPI() const                       {return m_DPI;}
        void           SetInternalDPI(const DPoint2d & value)       {m_DPI = value;}

        DPoint2dCR     GetScale() const                             {return m_scale;}
        void           SetScale(const DPoint2d & value)             {m_scale = value;}

        //Values are in sheet size units
        DGNPLATFORM_EXPORT DPoint2d       GetSheetSize() const;
        DGNPLATFORM_EXPORT void           SetSheetSize(const DPoint2d & value);

        //Values are in UORs
        DPoint2dCR     GetInternalSheetSize() const                 {return m_sheetSize;}
        void           SetInternalSheetSize(const DPoint2d & value) {m_sheetSize = value;}

        const SheetSizeUnit&  GetSheetUnit() const                 {return m_sheetUnit;}
        void           SetSheetUnit(SheetSizeUnit value)           {m_sheetUnit = value;}

        double         GetAspectRatio() const                      {return m_extent.x/m_extent.y;}

        //Use actual ResolutionSolverStrategy and specified axis to compute new values based on value specified by computational method.
        DGNPLATFORM_EXPORT void           ComputeResolution(IResolutionSolverStrategy::CoordinateAxis  fromAxis);

        DGNPLATFORM_EXPORT double         ConvertUnits   /* <= converted value */
                                                    (
                                                    double            value,                      /* => value to convert */
                                                    SheetSizeUnit     fromUnits,                  /* => value's current units (SheetSizeUnit) */
                                                    SheetSizeUnit     toUnits                     /* => desired units (SheetSizeUnit) */
                                                    ) const;

    private:
        RasterResolution(DgnModelP dgnCache);
        ~RasterResolution();


        /*----------------------------------------------------------------------------+
        |   Private Member variables
        +----------------------------------------------------------------------------*/
        double                      m_inchesPerUOR;
        IResolutionSolverStrategyPtr m_pResolutionSolverStrategy;
        bool                        m_aspectRatioLock; //When aspect is locked, pixel size X must equal pixel size Y
        DPoint2d                    m_extent;  //Internal value is always in UOR
        DPoint2d                    m_pixelSize; //Internal value is always in UOR
        UInt64                      m_nbOfPixelX;
        UInt64                      m_nbOfPixelY;
        DPoint2d                    m_DPI;       //Internal value is always in UOR, value is converted to inches on Get/Set
        DPoint2d                    m_scale;
        DPoint2d                    m_sheetSize; //Internal value is always in UOR, value is converted to m_sheetUnit on Get/Set
        SheetSizeUnit               m_sheetUnit;
    };


END_BENTLEY_DGNPLATFORM_NAMESPACE
#endif
