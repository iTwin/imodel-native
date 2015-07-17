/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMUnitsConverter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

    // ==================================================================================================
    // 
    // IMPORTANT NOTE (Sylvain.Pucci 01/06):
    // 
    // The same factor must be used to convert back and forth from Storage to uors.
    // Calculating a reverse factor i.e. m_dMpU = 1/m_dUpM, and use it in reverse conversion will make the 
    // conversion to be not reflexive (Pt != MpU(UpM(Pt))), and this will cause damages in dependency 
    // management.
    // So this is why the DTMUnitsConverter class has been changed and the m_dMpU field has been eliminated.
    //
    // ==================================================================================================

    //=======================================================================================
    // @bsiclass                                           Piotr M. Slowinski      10/2004
    //+===============+===============+===============+===============+===============+======
class DTMUnitsConverter
    {
    private:

        Transform m_trfs;
        Transform m_trfsInv;

    public:

        DTMUnitsConverter();
        DTMUnitsConverter(Transform& trfs);
        DTMUnitsConverter(DgnModelRefP hUStnModel);
        DTMUnitsConverter(DgnModelP dgnCache);
        DTMUnitsConverter(const DTMUnitsConverter &rSrc);
        DTMUnitsConverter& operator=(const DTMUnitsConverter &rSrc);

        // 1) FULL transformation
        // a) Master to UOR
        void FullStorageToUors(DPoint3d &rPoint) const;
        void FullStorageToUors(DPoint3d aPoints[], unsigned int nPoints) const;
        void FullStorageToUors(DPoint2d &rPoint) const;
        void FullStorageToUors(DPoint2d aPoints[], unsigned int nPoints) const;

        // b) UOR to Master
        void FullUorsToStorage(DPoint3d &rPoint) const;
        void FullUorsToStorage(DPoint3d aPoints[], unsigned int nPoints) const;
        template<size_t _TAB_SIZE_> void FullUorsToStorage ( DPoint3d (&points)[_TAB_SIZE_] ) const
            {
            return FullUorsToStorage ( points, _TAB_SIZE_ );
            }
        void FullUorsToStorage(DPoint2d &rPoint) const;
        void FullUorsToStorage(DPoint2d aPoints[], unsigned int nPoints) const;

        // 2) SCALING only
        // a) Master to UOR
        void ScaleStorageToUors(DPoint3d &rPoint) const;
        void ScaleStorageToUors(DPoint3d aPoints[], unsigned int nPoints) const;
        void ScaleStorageToUors(DPoint2d &rPoint) const;
        void ScaleStorageToUors(DPoint2d aPoints[], unsigned int nPoints) const;
        double ScaleStorageToUors(double dValue) const;
        double ScaleStorageToUorsZ(double dValue) const;

        // b) UOR to Master
        void ScaleUorsToStorage(DPoint3d &rPoint) const;
        void ScaleUorsToStorage(DPoint3d aPoints[], unsigned int nPoints) const;
        void ScaleUorsToStorage(DPoint2d &rPoint) const;
        void ScaleUorsToStorage(DPoint2d aPoints[], unsigned int nPoints) const;
        double ScaleUorsToStorage(double dValue) const;
        double ScaleUorsToStorageZ(double dValue) const;

        TransformCP GetStorageToUORTransformation() const;
        TransformCP GetUORToStorageTransformation() const;
    }; // class DTMUnitsConverter


//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline DTMUnitsConverter::DTMUnitsConverter()
    {
    m_trfs.initIdentity();
    m_trfsInv.initIdentity();
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline DTMUnitsConverter::DTMUnitsConverter(Transform& trfs)
    {
    m_trfs = trfs;
    m_trfsInv.inverseOf(&trfs);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline DTMUnitsConverter::DTMUnitsConverter(DgnModelRefP hUStnModel)
    {
    double dUpM = dgnModel_getUorPerMeter(hUStnModel->AsDgnModelCP());
    DPoint3d ptGO;
    dgnModel_getGlobalOrigin(hUStnModel->AsDgnModelCP(), &ptGO);
    bool is3d = hUStnModel->Is3d();

    m_trfs.setTranslation (&ptGO);
    m_trfs.ScaleMatrixColumns (dUpM, dUpM, is3d ? dUpM : 0); // If the file is not a 3d file we zeroes the Z (problem with picking if elevation is not zeroed)

    m_trfsInv.inverseOf(&m_trfs);
    }

//=======================================================================================
// @bsimethod                                           Sylvain.Pucci      11/2007
//+===============+===============+===============+===============+===============+======
__forceinline DTMUnitsConverter::DTMUnitsConverter(DgnModelP dgnCache)
    {
    double dUpM = dgnModel_getUorPerMeter(dgnCache);
    DPoint3d ptGO;
    dgnModel_getGlobalOrigin(dgnCache, &ptGO);
    bool is3d = dgnCache->Is3d();

    m_trfs.InitIdentity ();
    m_trfs.setTranslation (&ptGO);
    m_trfs.ScaleMatrixColumns (dUpM, dUpM, is3d ? dUpM : 0); // If the file is not a 3d file we zeroes the Z (problem with picking if elevation is not zeroed)

    m_trfsInv.inverseOf(&m_trfs);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline DTMUnitsConverter::DTMUnitsConverter(const DTMUnitsConverter &rSrc)
    {
    m_trfs = rSrc.m_trfs;
    m_trfsInv = rSrc.m_trfsInv;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline DTMUnitsConverter& DTMUnitsConverter::operator=(const DTMUnitsConverter &rSrc)
    {
    if (this != &rSrc)
        {
        m_trfs = rSrc.m_trfs;
        m_trfsInv = rSrc.m_trfsInv;
        }
    return *this;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullStorageToUors(DPoint3d &rPoint) const
    {
    m_trfs.multiply(&rPoint);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullStorageToUors(DPoint3d aPoints[], unsigned int nPoints) const
    {
    for(DPoint3d *p = &aPoints[nPoints];--p >= aPoints;)
        FullStorageToUors(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullStorageToUors(DPoint2d &rPoint) const
    {
    m_trfs.multiply(&rPoint, &rPoint, 1);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullStorageToUors(DPoint2d aPoints[], unsigned int nPoints) const
    {
    for(DPoint2d *p = &aPoints[nPoints]; --p >= aPoints;)
        FullStorageToUors(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullUorsToStorage(DPoint3d &rPoint) const
    {
    m_trfsInv.multiply(&rPoint);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullUorsToStorage(DPoint3d aPoints[], unsigned int nPoints) const
    {
    for(DPoint3d *p = &aPoints[nPoints]; --p >= aPoints;)
        FullUorsToStorage(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullUorsToStorage(DPoint2d &rPoint) const
    {
    m_trfsInv.multiply(&rPoint, &rPoint, 1);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::FullUorsToStorage(DPoint2d aPoints[], unsigned int nPoints) const
    {
    for(DPoint2d *p = &aPoints[nPoints]; --p >= aPoints;)
        FullUorsToStorage(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleStorageToUors(DPoint3d &rPoint) const
    {
    m_trfs.multiplyMatrixOnly(&rPoint);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleStorageToUors(DPoint3d aPoints[], unsigned int nPoints) const
    {
    for(DPoint3d *p = &aPoints[nPoints]; --p >= aPoints;)
        ScaleStorageToUors(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleStorageToUors(DPoint2d &rPoint) const
    {
    DPoint3d rPt;
    rPt.x = rPoint.x;
    rPt.y = rPoint.y;
    rPt.z = 0;
    m_trfs.multiplyMatrixOnly(&rPt);
    rPoint.x = rPt.x;
    rPoint.y = rPt.y;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleStorageToUors(DPoint2d aPoints[], unsigned int nPoints) const
    {
    for(DPoint2d *p = &aPoints[nPoints]; --p >= aPoints;)
        ScaleStorageToUors(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline double DTMUnitsConverter::ScaleStorageToUors(double dValue) const
    {
    DPoint3d p;
    p.x = dValue;
    p.y = p.z = 0;
    ScaleStorageToUors(p);
    return p.x;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline double DTMUnitsConverter::ScaleStorageToUorsZ(double dValue) const
    {
    DPoint3d p;
    p.x = p.y = 0;
    p.z = dValue;
    ScaleStorageToUors(p);
    return p.z;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleUorsToStorage(DPoint3d &rPoint) const
    {
    m_trfsInv.multiplyMatrixOnly (&rPoint);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleUorsToStorage(DPoint3d aPoints[], unsigned int nPoints) const
    {
    for(DPoint3d *p = &aPoints[nPoints]; --p >= aPoints;)
        ScaleUorsToStorage(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleUorsToStorage(DPoint2d &rPoint) const
    {
    DPoint3d rPt;
    rPt.x = rPoint.x;
    rPt.y = rPoint.y;
    rPt.z = 0;
    m_trfsInv.multiplyMatrixOnly(&rPt);
    rPoint.x = rPt.x;
    rPoint.y = rPt.y;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline void DTMUnitsConverter::ScaleUorsToStorage(DPoint2d aPoints[], unsigned int nPoints) const
    {
    for(DPoint2d *p = &aPoints[nPoints]; --p >= aPoints;)
        ScaleUorsToStorage(*p);
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline double DTMUnitsConverter::ScaleUorsToStorage(double dValue) const
    {
    DPoint3d p;
    p.x = dValue;
    p.y = p.z = 0;
    ScaleUorsToStorage(p);
    return p.x;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline double DTMUnitsConverter::ScaleUorsToStorageZ(double dValue) const
    {
    DPoint3d p;
    p.x = p.y = 0;
    p.z = dValue;
    ScaleUorsToStorage(p);
    return p.z;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline TransformCP DTMUnitsConverter::GetStorageToUORTransformation() const
    {
    return &m_trfs;
    }

//=======================================================================================
// @bsimethod                                           Piotr M. Slowinski      10/2004
//+===============+===============+===============+===============+===============+======
__forceinline TransformCP DTMUnitsConverter::GetUORToStorageTransformation() const
    {
    return &m_trfsInv;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
