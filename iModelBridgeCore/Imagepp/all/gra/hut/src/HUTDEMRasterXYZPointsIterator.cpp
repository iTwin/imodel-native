//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTDEMRasterXYZPointsIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCPGCoordModel.h>

#include <Imagepp/all/h/HFCException.h>

#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HUTDEMRasterXYZPointsExtractor.h>
#include <Imagepp/all/h/HUTDEMRasterXYZPointsIterator.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRFException.h>


#define STRIP_HEIGHT 256




namespace { // BEGIN unnamed namespace

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct XYZPoint
    {
    double x, y, z;
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct PixelToPointConverter
    {
    virtual                     ~PixelToPointConverter     () = 0; 

    virtual void                _Execute                   (XYZPoint*                       po_pXYZPoints,
                                                            const void*                     pi_pPixelValues,
                                                            uint64_t                       pi_Width,
                                                            uint64_t                       pi_Height,
                                                            const HGF2DTransfoModel&        pi_rTransfoModel) const = 0;
    };
PixelToPointConverter::~PixelToPointConverter() {}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PixelT>
struct PixToPtConverter : PixelToPointConverter
    {
    virtual void                _Execute                   (XYZPoint*                       po_pXYZPoint,
                                                            const void*                     pi_pPixelValues,
                                                            uint64_t                       pi_Width,
                                                            uint64_t                       pi_Height,
                                                            const HGF2DTransfoModel&        pi_rTransfoModel) const override;
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PixelT>
struct FastPixToPtConverter : PixelToPointConverter
    {
    virtual void                _Execute                   (XYZPoint*                       po_pXYZPoint,
                                                            const void*                     pi_pPixelValues,
                                                            uint64_t                       pi_Width,
                                                            uint64_t                       pi_Height,
                                                            const HGF2DTransfoModel&        pi_rTransfoModel) const override;
    };
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointDiscriminator
    {
    virtual                     ~PointDiscriminator                    () = 0;

    virtual size_t              _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const = 0;
    };
    PointDiscriminator::~PointDiscriminator () {}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct EmptyPointDiscriminator : PointDiscriminator
    {
    virtual size_t              _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const override;
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct NoDataPointDiscriminator : PointDiscriminator
    {
private:
    double                      m_NoDataValue;
public:
    explicit                    NoDataPointDiscriminator               (double                         pi_NoDataValue);

    virtual size_t              _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const override;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointModifier
    {
    virtual                     ~PointModifier                         () = 0;

    virtual void                _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const = 0;
    };
PointModifier::~PointModifier () {}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct EmptyPointModifier : PointModifier
    {
    virtual void                _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const override;
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScaleZToMeterPointModifier : PointModifier
    {
private:
    double                      m_FactorToMeterForZ;

public:
    explicit                    ScaleZToMeterPointModifier             (double                         pi_FactorToMeterForZ);

    virtual void                _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const override;
    };

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsiclass                                                  Mathieu.St-Pierre  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScaleZToMeterButNoDataValuePointModifier : PointModifier
{
private:
    double                      m_FactorToMeterForZ;
    double                      m_NoDataValue;

public:
    explicit                    ScaleZToMeterButNoDataValuePointModifier(double pi_FactorToMeterForZ, 
                                                                         double pi_NoDataValue);

    virtual void                _Execute                               (XYZPoint*                       pio_pXYZPoints,
                                                                        size_t                          pi_PointCount) const override;
};

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtractTransfoModelParameters (double&                    OriX,
                                    double&                    OriY,
                                    double&                    DeltaX,
                                    double&                    DeltaY,
                                    const HGF2DTransfoModel&    pi_TransfoModel)
    {
    if (pi_TransfoModel.IsCompatibleWith(HGF2DIdentity::CLASS_ID))
        {
        OriX = 0.0;
        OriY = 0.0;
        DeltaX = 1.0;
        DeltaY = 1.0;
        }
    else if (pi_TransfoModel.IsCompatibleWith(HGF2DStretch::CLASS_ID))
        {
        const HGF2DStretch& rStretchModel = static_cast<const HGF2DStretch&>(pi_TransfoModel);
        OriX = rStretchModel.GetTranslation().GetDeltaX();
        OriY = rStretchModel.GetTranslation().GetDeltaY();
        DeltaX = rStretchModel.GetXScaling();
        DeltaY = rStretchModel.GetYScaling();
        }
    else if (pi_TransfoModel.IsCompatibleWith(HGF2DTranslation::CLASS_ID))
        {
        const HGF2DTranslation& rTranslationModel = static_cast<const HGF2DTranslation&>(pi_TransfoModel);
        OriX = rTranslationModel.GetTranslation().GetDeltaX();
        OriY = rTranslationModel.GetTranslation().GetDeltaY();
        DeltaX = 1.0;
        DeltaY = 1.0;
        }
    else
        {
        HASSERT(!"Unsupported case!");
        OriX = 0.0;
        OriY = 0.0;
        DeltaX = 1.0;
        DeltaY = 1.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PixelT>
void PixToPtConverter<PixelT>::_Execute(XYZPoint*                   po_pXYZPoint,
                                        const void*                 pi_pPixelValues,
                                        uint64_t                   pi_Width,
                                        uint64_t                   pi_Height,
                                        const HGF2DTransfoModel&    pi_rTransfoModel) const
    {
    const PixelT* pPixelValue = static_cast<const PixelT*>(pi_pPixelValues);

    for (uint32_t RowInd = 0; RowInd < pi_Height; RowInd++)
        {
        for (uint32_t ColInd = 0; ColInd < pi_Width; ColInd++)
            {
            pi_rTransfoModel.ConvertDirect(ColInd, RowInd, &po_pXYZPoint->x, &po_pXYZPoint->y);
            po_pXYZPoint->z = (double)pPixelValue[RowInd * pi_Width + ColInd];
            ++po_pXYZPoint;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PixelT>
void FastPixToPtConverter<PixelT>::_Execute(XYZPoint*                   po_pXYZPoint,
                                            const void*                 pi_pPixelValues,
                                            uint64_t                   pi_Width,
                                            uint64_t                   pi_Height,
                                            const HGF2DTransfoModel&    pi_rTransfoModel) const
    {
    const PixelT* pPixelValue = static_cast<const PixelT*>(pi_pPixelValues);

    double OriX;
    double OriY;
    double DeltaX;
    double DeltaY;
    ExtractTransfoModelParameters(OriX, OriY, DeltaX, DeltaY, pi_rTransfoModel);


    double PosX;
    double PosY = OriY;

    for (uint32_t RowInd = 0; RowInd < pi_Height; RowInd++)
        {
        PosX = OriX;

        for (uint32_t ColInd = 0; ColInd < pi_Width; ColInd++)
            {
            po_pXYZPoint->x   = PosX;
            po_pXYZPoint->y   = PosY;
            po_pXYZPoint->z   = (double)pPixelValue[RowInd * pi_Width + ColInd];

            ++po_pXYZPoint;
            PosX += DeltaX;
            }

        PosY += DeltaY;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PixelToPointConverter* CreatePixelToPointConverter   (const HRABitmap&                    pi_rBitmap,
                                                            const HFCPtr<HGF2DTransfoModel>&    pi_rpReprojectionTransfoModel)
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_rBitmap.GetTransfoModel());

    //Make the x-y coordinates correspond to the center of the destination pixels.
    HGF2DStretch stretchModel;

    stretchModel.SetXScaling(1);
    stretchModel.SetYScaling(1);    
                                          
    HGF2DDisplacement translation(0.5, 0.5);
                                          
    stretchModel.SetTranslation(translation);        

    pTransfoModel = stretchModel.ComposeInverseWithDirectOf(*pTransfoModel);

    HFCPtr<HGF2DTransfoModel> pSimplifiedTransfoModel(pTransfoModel->CreateSimplifiedModel());

    if (pSimplifiedTransfoModel != 0)
        pTransfoModel = pSimplifiedTransfoModel;

    //Optimized cases
    if ((pi_rpReprojectionTransfoModel == 0) &&
        ((pTransfoModel->IsCompatibleWith(HGF2DIdentity::CLASS_ID) == true) ||
         (pTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID) == true)  ||
         (pTransfoModel->IsCompatibleWith(HGF2DTranslation::CLASS_ID) == true)))
        {
        //The following is really awfully designed but acceptable given the time constraint.

        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
            return new FastPixToPtConverter<uint8_t>();

        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID))
            return new FastPixToPtConverter<short>();

        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
            return new FastPixToPtConverter<unsigned short>();

        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID))
            return new FastPixToPtConverter<float>();

        HASSERT(!"Unsupported pixel type!");
        return 0;
        }

    if (pi_rpReprojectionTransfoModel != 0)
        pTransfoModel = pTransfoModel->ComposeInverseWithDirectOf(*pi_rpReprojectionTransfoModel);

    //The following is really awfully designed but acceptable given the time constraint.
    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
        return new PixToPtConverter<uint8_t>();

    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID))
        return new PixToPtConverter<short>();
    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
        return new PixToPtConverter<unsigned short>();

    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID))
        return new PixToPtConverter<float>();

    HASSERT(!"Unsupported pixel type!");
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description  Deprecated. Remove when possible.
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PixelToPointConverter* CreateLegacyPixelToPointConverter (const HRABitmap&                    pi_rBitmap,
                                                                const HFCPtr<HGF2DTransfoModel>&    pi_rpReprojectionTransfoModel)
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_rBitmap.GetTransfoModel());
        
    //Make the x-y coordinates correspond to the center of the destination pixels.
    HGF2DStretch stretchModel;

    stretchModel.SetXScaling(1);
    stretchModel.SetYScaling(1);    
                                          
    HGF2DDisplacement translation(0.5, 0.5);
                                          
    stretchModel.SetTranslation(translation);        
 
    pTransfoModel = stretchModel.ComposeInverseWithDirectOf(*pTransfoModel);

    HFCPtr<HGF2DTransfoModel> pSimplifiedTransfoModel(pTransfoModel->CreateSimplifiedModel());

    if (pSimplifiedTransfoModel != 0)
        pTransfoModel = pSimplifiedTransfoModel;

    //Optimized cases
    if ((pi_rpReprojectionTransfoModel == 0) &&
        ((pTransfoModel->IsCompatibleWith(HGF2DIdentity::CLASS_ID) == true) ||
         (pTransfoModel->IsCompatibleWith(HGF2DStretch::CLASS_ID) == true)  ||
         (pTransfoModel->IsCompatibleWith(HGF2DTranslation::CLASS_ID) == true)))
    {                   
        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID))
            return new FastPixToPtConverter<short>();

        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
            return new FastPixToPtConverter<unsigned short>();

        if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID))
            return new FastPixToPtConverter<float>();

        HASSERT(!"Unsupported pixel type!");
        return 0;
        }

    if (pi_rpReprojectionTransfoModel != 0)
        pTransfoModel = pTransfoModel->ComposeInverseWithDirectOf(*pi_rpReprojectionTransfoModel);
    
    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID))
        return new PixToPtConverter<short>();
    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
        return new PixToPtConverter<unsigned short>();

    if (pi_rBitmap.GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID))
        return new PixToPtConverter<float>();

    HASSERT(!"Unsupported pixel type!");
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t EmptyPointDiscriminator::_Execute   (XYZPoint*   pio_pXYZPoints,
                                            size_t      pi_PointCount) const
    {
    // Do nothing
    return pi_PointCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @param        pi_NoDataValue              Value stored as coordinates for a point when
*                                           this point is unspecified.
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
NoDataPointDiscriminator::NoDataPointDiscriminator (double pi_NoDataValue)
    :   m_NoDataValue(pi_NoDataValue)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  Remove all points that are set to no data value (mean that the point
*               is unspecified)
* @param        pio_pPtsBuffer
* @param        pi_Size                     Size of inputted buffers
* @return       New size of inputted buffers
* @bsimethod                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NoDataPointDiscriminator::_Execute  (XYZPoint*   pio_pXYZPoints,
                                            size_t      pi_PointCount) const
    {
    struct ShouldPtBeRemoved : public unary_function<XYZPoint, bool>
        {
        explicit ShouldPtBeRemoved(double pi_NoDataValue) : m_NoDataValue(pi_NoDataValue) {}

        bool operator () (const XYZPoint& pi_rPt) const
            {
            //return HNumeric<double>::EQUAL_EPSILON(m_NoDataValue, pi_rPt.z);
            return m_NoDataValue == pi_rPt.z;
            }
    private:
        const double m_NoDataValue;
        };

    return distance(pio_pXYZPoints,
                    std::remove_if(pio_pXYZPoints, pio_pXYZPoints + pi_PointCount, ShouldPtBeRemoved(m_NoDataValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PointDiscriminator* CreatePointDiscriminator (const HUTDEMRasterXYZPointsExtractor& pi_rExtractor)
    {
    const double* pNoDataValue = pi_rExtractor.GetNoDataValue();
    if (0 == pNoDataValue)
        return new EmptyPointDiscriminator();

    return new NoDataPointDiscriminator(*pNoDataValue);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void EmptyPointModifier::_Execute  (XYZPoint*   pio_pXYZPoints,
                                    size_t      pi_PointCount) const
    {
// Do nothing
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleZToMeterPointModifier::ScaleZToMeterPointModifier (double pi_FactorToMeterForZ)
    :   m_FactorToMeterForZ(pi_FactorToMeterForZ)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ScaleZToMeterPointModifier::_Execute  (XYZPoint*   pio_pXYZPoints,
                                            size_t      pi_PointCount) const
    {
    struct ScaleZ : public unary_function<XYZPoint, void>
        {
        explicit ScaleZ(double pi_FactorToMeter) : m_FactorToMeter(pi_FactorToMeter) {}

        void operator () (XYZPoint& pi_rPt) const
            {
            pi_rPt.z *= m_FactorToMeter;
            }
    private:
        const double m_FactorToMeter;
        };

    std::for_each(pio_pXYZPoints, pio_pXYZPoints + pi_PointCount, ScaleZ(m_FactorToMeterForZ));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ScaleZToMeterButNoDataValuePointModifier::ScaleZToMeterButNoDataValuePointModifier (double pi_FactorToMeterForZ, 
                                                                                    double pi_NoDataValue)
: m_FactorToMeterForZ(pi_FactorToMeterForZ), 
  m_NoDataValue(pi_NoDataValue)
{
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ScaleZToMeterButNoDataValuePointModifier::_Execute  (XYZPoint*   pio_pXYZPoints,
                                                          size_t      pi_PointCount) const
{
    struct ScaleZ : public unary_function<XYZPoint, void>
    {
        explicit ScaleZ(double pi_FactorToMeter, 
                        double pi_NoDataValue) 
            : m_FactorToMeter(pi_FactorToMeter), 
              m_NoDataValue(pi_NoDataValue)
            {
            }

        void operator () (XYZPoint& pi_rPt) const
        {
            if (pi_rPt.z != m_NoDataValue)
            {
                pi_rPt.z *= m_FactorToMeter;
            }
        }
    private:
        const double m_FactorToMeter;
        const double m_NoDataValue;
    };

    std::for_each(pio_pXYZPoints, pio_pXYZPoints + pi_PointCount, ScaleZ(m_FactorToMeterForZ, m_NoDataValue));
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PointModifier* CreatePointModifier (double pi_FactoryToMeterForZ)
    {
    if (1.0 == pi_FactoryToMeterForZ)
        return new EmptyPointModifier();

    return new ScaleZToMeterPointModifier(pi_FactoryToMeterForZ);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PointModifier* CreateLegacyPointModifier (double        pi_FactoryToMeterForZ, 
                                                const double* pi_pNoDataValue)
{
    if (1.0 == pi_FactoryToMeterForZ)
        return new EmptyPointModifier();
    else
    if (pi_pNoDataValue == 0)
        return new ScaleZToMeterPointModifier(pi_FactoryToMeterForZ);
    else
        return new ScaleZToMeterButNoDataValuePointModifier(pi_FactoryToMeterForZ, *pi_pNoDataValue);        
}

} // END unnamed namespace



/** ---------------------------------------------------------------------------
    Public
    GetMaxXYZPointQtyForIteratorWith
    ---------------------------------------------------------------------------
*/
size_t HUTDEMRasterXYZPointsExtractor::GetMaxPointQtyForXYZPointsIterator (double  pi_ScaleFactor)
    {
    return static_cast<uint32_t>(ceil(m_WidthInPixels * pi_ScaleFactor)) * STRIP_HEIGHT;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct HUTDEMRasterXYZPointsIterator::Impl
    {
    HUTDEMRasterXYZPointsExtractor&         m_rRasterPointExtractor;

    unique_ptr<const PixelToPointConverter>   m_pPixelToPointConverter;
    unique_ptr<const PointDiscriminator>      m_pPointDiscriminator;
    unique_ptr<const PointModifier>           m_pPointModifier;

    explicit                                Impl                           (HUTDEMRasterXYZPointsExtractor&    pi_rRasterPointExtractor)
        :   m_rRasterPointExtractor(pi_rRasterPointExtractor)
        {

        }

    };


/** ---------------------------------------------------------------------------
    Public
    Create
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsIterator* HUTDEMRasterXYZPointsIterator::CreateFor(HUTDEMRasterXYZPointsExtractor&    pi_rExtractor,
                                                                        const WString&                     pi_rDestCoordSysKeyName,
                                                                        double                             pi_ScaleFactor)
    {
    return new HUTDEMRasterXYZPointsIterator(new Impl(pi_rExtractor),
                                             pi_rDestCoordSysKeyName,
                                             pi_ScaleFactor);
    }

/** ---------------------------------------------------------------------------
    Public
    Constructor
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsIterator::HUTDEMRasterXYZPointsIterator   (Impl*          pi_pImpl,
                                                                const WString& pi_rDestCoordSysKeyName,
                                                                double         pi_ScaleFactor)
    :   m_pImpl(pi_pImpl),
        m_CurrentStripPosInSourceRasterPhyCS(0),
        m_IsDestCoordSysCreationFailed(false)
    {
    Init(m_pImpl->m_rRasterPointExtractor, pi_rDestCoordSysKeyName, pi_ScaleFactor);

    // Create a point converter that will convert pixels to 3d points
    m_pImpl->m_pPixelToPointConverter.reset(CreatePixelToPointConverter(*m_pStrip, m_pReprojectionModel));

    // Create a discriminator that will remove all no data points
    m_pImpl->m_pPointDiscriminator.reset(CreatePointDiscriminator(m_pImpl->m_rRasterPointExtractor));

    // Create a modifier that will scale z
    m_pImpl->m_pPointModifier.reset(CreatePointModifier(m_pImpl->m_rRasterPointExtractor.GetFactorToMeterForZ()));
    }

/** ---------------------------------------------------------------------------
    Public
    Constructor
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsIterator::HUTDEMRasterXYZPointsIterator(HUTDEMRasterXYZPointsExtractor* pi_pRasterPointExtractor,
                                                             const WString&                  pi_rDestCoordSysKeyName,
                                                             double                          pi_ScaleFactor)
    :   m_pImpl(new Impl(*pi_pRasterPointExtractor)),
        m_CurrentStripPosInSourceRasterPhyCS(0),
        m_IsDestCoordSysCreationFailed(false)
    {
    Init(*pi_pRasterPointExtractor, pi_rDestCoordSysKeyName, pi_ScaleFactor);
    
    m_pImpl->m_pPixelToPointConverter.reset(CreateLegacyPixelToPointConverter(*m_pStrip,
                                                                              m_pReprojectionModel));

    m_pImpl->m_pPointDiscriminator.reset(new EmptyPointDiscriminator());

    // As no data values weren't removed, will need to make sure not to scale these
    m_pImpl->m_pPointModifier.reset(CreateLegacyPointModifier(pi_pRasterPointExtractor->GetFactorToMeterForZ(), 
                                                              pi_pRasterPointExtractor->GetNoDataValue()));    
    }


/** ---------------------------------------------------------------------------
    Public
    Init
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsIterator::Init   (HUTDEMRasterXYZPointsExtractor&    pi_rExtractor,
                                            const WString&                     pi_rDestCoordSysKeyName,
                                            double                             pi_ScaleFactor)
{ 
    HPRECONDITION(pi_rExtractor.m_pRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID));
    HPRECONDITION(pi_ScaleFactor <= 1.0);


    HFCPtr<HRFPageDescriptor>       pPageDescriptor(pi_rExtractor.
                                                    m_pRasterFile->
                                                    GetPageDescriptor(0));

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor(pPageDescriptor->
                                                          GetResolutionDescriptor(0));

    HFCPtr<HRAStoredRaster> pStoredRaster((HFCPtr<HRAStoredRaster>&)pi_rExtractor.
                                          m_pRaster);

    HGF2DStretch StretchModel;

    m_StripWidthInPixels     = (uint32_t)ceil(pi_rExtractor.m_WidthInPixels * pi_ScaleFactor);
    m_FilteredHeightInPixels = (uint32_t)ceil(pi_rExtractor.m_HeightInPixels * pi_ScaleFactor);
    
    if (pi_ScaleFactor == 1.0)
        {
        m_ScaleFactorX = 1.0;
        m_ScaleFactorY = 1.0;
        }
    else
        {
        //Ensure that the number pixels on the border of the images are aligned with pixels
        //on the border of the filtered images.
        HASSERT((pi_rExtractor.m_WidthInPixels - 1 > 0) && (pi_rExtractor.m_HeightInPixels - 1 > 0));

        m_ScaleFactorX = (double)(m_StripWidthInPixels - 1) / (pi_rExtractor.m_WidthInPixels - 1);
        m_ScaleFactorY = (double)(m_FilteredHeightInPixels - 1) / (pi_rExtractor.m_HeightInPixels - 1);
        }

    StretchModel.SetXScaling(1 / m_ScaleFactorX);
    StretchModel.SetYScaling(1 / m_ScaleFactorY);
                                                                                                       
    HFCPtr<HGF2DTransfoModel> pTransfoModel(pStoredRaster->
                                            GetPhysicalCoordSys()->
                                            GetTransfoModelTo(pi_rExtractor.GetXYCoordSystP()));
               
    //The value of this translation ensure that the center of both the top left corner pixels of the 
    //source image and the destination strip are aligned.
	HASSERT((m_ScaleFactorX > 0) && (m_ScaleFactorY > 0));
			
    double translationX = -((1 / m_ScaleFactorX) - 1) / 2;
    double translationY = -((1 / m_ScaleFactorY) - 1) / 2;

    HGF2DDisplacement translation(translationX, translationY);
                                          
    StretchModel.SetTranslation(translation);     

    pTransfoModel = StretchModel.ComposeInverseWithDirectOf(*pTransfoModel);

    m_StripHeightInSourceRasterPhyCS = (STRIP_HEIGHT / m_ScaleFactorY);

    m_pStrip = HRABitmap::Create((uint32_t)m_StripWidthInPixels,
                             (uint32_t)STRIP_HEIGHT,
                             //pi_pModelCSp_CSl
                             pTransfoModel,
                             (HFCPtr<HGF2DCoordSys>&)pi_rExtractor.GetXYCoordSystP(),
                             pStoredRaster->GetPixelType());

    if (pi_rDestCoordSysKeyName != L"")
        {
        try
            {
            m_pDestCoorSys = GeoCoordinates::BaseGCS::CreateGCS(pi_rDestCoordSysKeyName.c_str());
            }
        catch (HFCException&)
            {
            m_IsDestCoordSysCreationFailed = true;
            }

        if (m_pDestCoorSys != 0)
            {
            try
                {
                if (pi_rExtractor.m_pRasterFile->GetPageDescriptor(0)->GetGeocodingCP() != 0)
                {     
					//MST : The computation of the grid model is currently just used to determine if the conversion 
					//      from the source to the destination GCS is possible (i.e. : validity domain verification).
					//      Eventually it might be removed when HCPGCoordModel or HUTDEMRasterXYZPointsIterator provide such verification. 					
                    m_pReprojectionModel = HCPGeoTiffKeys::GetTransfoModelForReprojection(pi_rExtractor.m_pRasterFile,
                                                                          0,
                                                                          m_pDestCoorSys.get(),
                                                                          (HFCPtr<HGF2DWorldCluster>&)(HUTDEMRasterXYZPointsExtractor::GetHMRWorldCluster()),
                                                                          NULL);
                }

               
                if (m_pReprojectionModel == 0)
                {
                    m_IsDestCoordSysCreationFailed = true;
                }
                else
                {
                    GeoCoordinates::BaseGCSCP pSrcFileGeocoding = pi_rExtractor.m_pRasterFile->GetPageDescriptor(0)->GetGeocodingCP();

                    if (pSrcFileGeocoding != NULL && pSrcFileGeocoding->IsValid())
                        {
                        m_pReprojectionModel = new HCPGCoordModel(*pSrcFileGeocoding, *m_pDestCoorSys);
                        }
					
					HASSERT(m_pReprojectionModel != 0);
                }
            }
            catch (HFCException&)
            {            
                m_IsDestCoordSysCreationFailed = TRUE;
            }          
        }
    } 
}

/** ---------------------------------------------------------------------------
    Public
    Destructor
    ---------------------------------------------------------------------------
*/
HUTDEMRasterXYZPointsIterator::~HUTDEMRasterXYZPointsIterator()
    {
    }

/** ---------------------------------------------------------------------------
    Public
    GetMaxXYZPointQty
    ---------------------------------------------------------------------------
*/
size_t HUTDEMRasterXYZPointsIterator::GetMaxXYZPointQty () const
    {
    // TDORAY: Precompute this value in cstor if it slow down the process
    return static_cast<size_t>(STRIP_HEIGHT * m_StripWidthInPixels);
    }


/** ---------------------------------------------------------------------------
    Public
    ComputeStripHeight
    ---------------------------------------------------------------------------
*/
uint32_t HUTDEMRasterXYZPointsIterator::ComputeStripHeight () const
    {
    uint32_t Height;
    if (m_CurrentStripPosInSourceRasterPhyCS + m_StripHeightInSourceRasterPhyCS > m_pImpl->m_rRasterPointExtractor.m_HeightInPixels)
        {
        Height = (uint32_t)(m_FilteredHeightInPixels -
                          (uint32_t)round(m_CurrentStripPosInSourceRasterPhyCS * m_ScaleFactorY));

        HASSERT((uint32_t)(m_FilteredHeightInPixels / STRIP_HEIGHT) * STRIP_HEIGHT  + Height == m_FilteredHeightInPixels);
        }
    else
        {
        Height = STRIP_HEIGHT;
        }
    return Height;
    }

/** ---------------------------------------------------------------------------
    Public
    InitStripForDebug
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsIterator::InitStripForDebug ()
{
    HRAClearOptions ClearOptions;
    HAutoPtr<Byte> pRawDataValue;
    
    if (m_pStrip->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
    {
        pRawDataValue = new Byte[sizeof(uint8_t)];
        *((uint8_t*)pRawDataValue.get()) = (numeric_limits<uint8_t>::max)();            
    }
    else
    if (m_pStrip->GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
    {
        pRawDataValue = new Byte[sizeof(unsigned short)];
        *((unsigned short*)pRawDataValue.get()) = 65535;            
    }
    else
    if (m_pStrip->GetPixelType()->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID))
    {            
        pRawDataValue = new Byte[sizeof(short)];
        *((short*)pRawDataValue.get()) = 32765;            
    }
    else
    if (m_pStrip->GetPixelType()->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID))
    {
        pRawDataValue = new Byte[sizeof(float)];
        *((float*)pRawDataValue.get()) = (numeric_limits<float>::max)();        
    }

    ClearOptions.SetRawDataValue(pRawDataValue.get());

    m_pStrip->Clear(ClearOptions);
    }

/** ---------------------------------------------------------------------------
    Public
    ComputeTransfoModel
    ---------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HUTDEMRasterXYZPointsIterator::ComputeTransfoModel () const
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel(m_pStrip->GetTransfoModel());

    //Make the x-y coordinates correspond to the center of the destination pixels.
    HGF2DStretch StretchModel;

    StretchModel.SetXScaling(1);
    StretchModel.SetYScaling(1);    
                                          
    HGF2DDisplacement translation(0.5, 0.5);
                                          
    StretchModel.SetTranslation(translation);        

    pTransfoModel = StretchModel.ComposeInverseWithDirectOf(*pTransfoModel);

    HFCPtr<HGF2DTransfoModel> pSimplifiedTransfoModel(pTransfoModel->CreateSimplifiedModel());

    if (0 != pSimplifiedTransfoModel)
        pTransfoModel = pSimplifiedTransfoModel;
                 
     if (0 != m_pReprojectionModel)
        pTransfoModel = pTransfoModel->ComposeInverseWithDirectOf(*m_pReprojectionModel);
      
    return pTransfoModel;
    }


/** ---------------------------------------------------------------------------
    Public
    GetXYZPointsImpl
    ---------------------------------------------------------------------------
*/
size_t HUTDEMRasterXYZPointsIterator::GetXYZPointsImpl(void*    po_pPointsBuffer,
                                                       size_t   pi_CapacityInPoints) const
    {
    HPRECONDITION(po_pPointsBuffer != 0);
    HPRECONDITION(pi_CapacityInPoints >= GetMaxXYZPointQty());

    if (m_CurrentStripPosInSourceRasterPhyCS >= m_pImpl->m_rRasterPointExtractor.m_HeightInPixels)
        return 0;

    const uint32_t Height = ComputeStripHeight();

    HDEBUGCODE(const_cast<HUTDEMRasterXYZPointsIterator&>(*this).InitStripForDebug());

    m_pStrip->CopyFrom(*m_pImpl->m_rRasterPointExtractor.m_pRaster);


    HFCPtr<HGF2DTransfoModel> pTransfoModel(ComputeTransfoModel());

    XYZPoint* pXYZPoints = static_cast<XYZPoint*>(po_pPointsBuffer);
    m_pImpl->m_pPixelToPointConverter->_Execute(pXYZPoints,
                                                m_pStrip->GetPacket()->GetBufferAddress(),
                                                m_StripWidthInPixels,
                                                Height,
                                                *pTransfoModel);

    size_t pointQty = static_cast<size_t>(m_StripWidthInPixels * Height);

    pointQty = m_pImpl->m_pPointDiscriminator->_Execute(pXYZPoints, pointQty);
    m_pImpl->m_pPointModifier->_Execute(pXYZPoints, pointQty);


    return pointQty;

    }

/** ---------------------------------------------------------------------------
    Public
    GetXYZPoints
    ---------------------------------------------------------------------------
*/
const double* HUTDEMRasterXYZPointsIterator::GetXYZPoints(uint32_t* po_pNumberPoints) const
    {
    HPRECONDITION(po_pNumberPoints != 0);

    if (0 == m_pXYZPoints)
        m_pXYZPoints = new double[GetMaxXYZPointQty() * 3];

    *po_pNumberPoints = static_cast<uint32_t>(GetXYZPointsImpl(m_pXYZPoints.get(), GetMaxXYZPointQty()));

    return (0 == *po_pNumberPoints) ? 0 : m_pXYZPoints.get();
    }

/** ---------------------------------------------------------------------------
    Public
    NextBlock
    ---------------------------------------------------------------------------
*/
bool HUTDEMRasterXYZPointsIterator::NextBlock()
    {
    HFCPtr<HGF2DTranslation> pTranslation(new HGF2DTranslation(HGF2DDisplacement(0, STRIP_HEIGHT)));

    m_pStrip->SetTransfoModel(*pTranslation->ComposeInverseWithDirectOf(*m_pStrip->GetTransfoModel()));
    m_CurrentStripPosInSourceRasterPhyCS += m_StripHeightInSourceRasterPhyCS;

    return m_CurrentStripPosInSourceRasterPhyCS < m_pImpl->m_rRasterPointExtractor.m_HeightInPixels;
    }

/** ---------------------------------------------------------------------------
    Public
    Next
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsIterator::Next()
    {
    NextBlock();
    }

/** ---------------------------------------------------------------------------
    Public
    Reset
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsIterator::Reset()
    {
    HFCPtr<HRAStoredRaster> pStoredRaster((HFCPtr<HRAStoredRaster>&)m_pImpl->m_rRasterPointExtractor.
                                          m_pRaster);

    HFCPtr<HGF2DTransfoModel> pTranfoModel(pStoredRaster->
                                           GetPhysicalCoordSys()->
                                           GetTransfoModelTo(m_pImpl->m_rRasterPointExtractor.GetXYCoordSystP()));

    m_pStrip->SetTransfoModel(*pTranfoModel);
    m_CurrentStripPosInSourceRasterPhyCS = 0;
    }

/** ---------------------------------------------------------------------------
    Public
    GetNumberOfFilteredPoints
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsIterator::GetNumberOfFilteredPoints(uint64_t* po_pNumberOfPoints) const
    {
    *po_pNumberOfPoints = m_StripWidthInPixels * m_FilteredHeightInPixels;
    }

/** ---------------------------------------------------------------------------
    Public
    GetFilteredDimensionInPixels
    ---------------------------------------------------------------------------
*/
void HUTDEMRasterXYZPointsIterator::GetFilteredDimensionInPixels(uint64_t* po_pWidthInPixels,
                                                                 uint64_t* po_pHeightInPixels) const
    {
    *po_pWidthInPixels  = m_StripWidthInPixels;
    *po_pHeightInPixels = m_FilteredHeightInPixels;
    }

/** ---------------------------------------------------------------------------
    Public
    IsDestCoordSysCreationFailed
    ---------------------------------------------------------------------------
*/
bool HUTDEMRasterXYZPointsIterator::IsDestCoordSysCreationFailed() const
    {
    return m_IsDestCoordSysCreationFailed;
    }
