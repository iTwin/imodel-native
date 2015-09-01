//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DCoordSys.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DCoordSys
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HGF2DCoordSys.h>

#include <Imagepp/all/h/HFCMonitor.h>

HPM_REGISTER_CLASS(HGF2DCoordSys, HPMPersistentObject)


/** -----------------------------------------------------------------------------
    Default Constructor

    Sets units to meters for both X and Y dimensions.
    -----------------------------------------------------------------------------
*/
HGF2DCoordSysImpl::HGF2DCoordSysImpl()
    {
    // Indicate that system has no reference
    m_HasReference = false;

    // Indicate that there is no last used transformation to other system
    m_pLastSystem   = 0;
    m_pLastModel    = 0;
    m_pTransfoModel = 0;
    m_LastDirection = DIRECT;
    }





/** -----------------------------------------------------------------------------
    Construtor

    Creates a coordinate system based upon another coordinate system.
    A transformation model is provided, and the coordinate system units are set
    equal to the direct dimension of this transformation model.

    @param pi_rModel IN A constant reference to a reference model, that will
                        expressed the direct relation between the current coordinate
                        system and the other specified coordinate system, which
                        becomes the reference.

    @param pi_rpCoordSys IN Reference to a smart pointer to the coordinate system
                            which is linked to the present one through the
                            given transformation model.

    @code
        HGF2DCoordSys*      pRefSystem = new (HGF2DCoordSys) ();

        // A translation is a kind of transformation model
        HGF2DTranslation    TransModel (5.0, 60.0);
        HGF2DCoordSys       ImageASys (TransModel,
                                    HFCPtr<HGF2DCoordSys> (pRefSystem));
    @end

    -----------------------------------------------------------------------------
*/
HGF2DCoordSysImpl::HGF2DCoordSysImpl(const HGF2DTransfoModel& pi_rModel,
                                     const HFCPtr<HGF2DCoordSysImpl>&    pi_rpCoordSys)
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    // Copy model
    HAutoPtr<HGF2DTransfoModel> pTempModel(pi_rModel.Clone());

    // Copy smart pointer to reference system
    m_pRefCoordSys = pi_rpCoordSys;

    // Indicate that there is no last used transformation to other system
    m_pLastSystem   = 0;
    m_pLastModel    = 0;
    m_LastDirection = DIRECT;

    // Advise reference coordinate system it is ref
    pi_rpCoordSys->AddInverseRef(pTempModel, this);

    // Indicate system has a reference and release from auto ptr
    m_pTransfoModel = pTempModel.release();
    m_HasReference = true;
    }



/** -----------------------------------------------------------------------------
    Copy Construtor

    This copy is only partial, since coordinate system serve as labels inside
    a network of coordinate systems. Only the transformation model to the reference
    and the smart pointer to this reference are copied.
    -----------------------------------------------------------------------------
*/
HGF2DCoordSysImpl::HGF2DCoordSysImpl(const HGF2DCoordSysImpl& pi_rObj)
    {
    // Check if reference parameters must be copied
    if (pi_rObj.m_HasReference)
        {
        // Duplicate model to reference
        HAutoPtr<HGF2DTransfoModel> pTempModel(pi_rObj.m_pTransfoModel->Clone());

        // Duplicate smart pointer to reference
        m_pRefCoordSys = pi_rObj.m_pRefCoordSys;

        // Advise reference it is being used
        m_pRefCoordSys->AddInverseRef(pTempModel, this);

        // Release model from auto ptr and set
        m_pTransfoModel = pTempModel.release();
        }
    else
        {
        m_pTransfoModel = 0;
        m_pRefCoordSys  = 0;
        }

    // Copy has ref attribute
    m_HasReference = pi_rObj.m_HasReference;

    // Indicate that there is no last model
    m_pLastSystem   = 0;
    m_pLastModel    = 0;
    m_LastDirection = DIRECT;
    }

/** -----------------------------------------------------------------------------
    The destruction will advise other coordinate system it is related to of its
    destruction, thus permitting them to disconnect. The destruction will also
    provoke the destruction of all transformation models that belong to the
    coordinate system.
    -----------------------------------------------------------------------------
*/
HGF2DCoordSysImpl::~HGF2DCoordSysImpl()
    {
    HPRECONDITION(m_ListIsRefTo.empty());

    // Should be called before from HPMShareableObject<HGF2DCoordSys>
//    RemoveReferences();

    HPOSTCONDITION(m_ListHasRefTo.empty());
    HPOSTCONDITION(m_ListIsAltRefTo.empty());
    HPOSTCONDITION(m_ListIsRefTo.empty());
    }

void HGF2DCoordSysImpl::RemoveReferences()
    {
    // Disconnect from last used system
    // THIS OPERATION MUST BE THE FIRST OPERATION
    HFCMonitor CacheMonitor(m_CacheKey);
    m_pLastSystem = 0;
    m_pLastModel = 0;
    CacheMonitor.ReleaseKey();

    // Check if there is a reference
    if (m_HasReference)
        {
        // Advise reference that it is being destroyed
        CacheMonitor.Assign(m_pRefCoordSys->m_CacheKey);
        m_pRefCoordSys->m_pLastSystem = 0;
        m_pRefCoordSys->m_pLastModel = 0;
        CacheMonitor.ReleaseKey();
        }

        {
        HFCMonitor Monitor(m_ListsKey);

        // Verify that we are not the m_pRefCoordSys of another coord sys.
        // Technically impossible.
        HASSERT(m_ListIsRefTo.size() == 0);

        // Destroy the transformation models
        ListCoordSysToModel::iterator Itr;
        while (m_ListHasRefTo.size() > 0)
            {
            // take the first element in the map
            Itr = m_ListHasRefTo.begin();

            // Save pointer to coord sys
            HFCPtr<HGF2DCoordSysImpl> pCoordSys = Itr->first;

            // Destroy the transformation model
            delete Itr->second;

            // Remove entry
            m_ListHasRefTo.erase(Itr);

            // Advise other coord sys that reference is removed
            Monitor.ReleaseKey();
            pCoordSys->RemoveAltInverseRef(this);
            pCoordSys = 0;
            Monitor.Assign(m_ListsKey);
            }

        // Advise system it is an alternate system to ... to disconnect
        while (m_ListIsAltRefTo.size() > 0)
            {
            // take the first element in the map
            Itr = m_ListIsAltRefTo.begin();

            // Save pointer to coord sys
            HFCPtr<HGF2DCoordSysImpl> pCoordSys = Itr->first;

            // Remove entry
            m_ListIsAltRefTo.erase(Itr);

            // Advise other coord sys that reference is removed
            Monitor.ReleaseKey();
            pCoordSys->DisconnectFromAltRef(this);
            pCoordSys = 0;
            Monitor.Assign(m_ListsKey);
            }
        }

    // Destroy transformation model to reference
    if (m_HasReference)
        {
        m_pRefCoordSys->RemoveInverseRef(this);

        delete m_pTransfoModel;
        m_pTransfoModel = 0;
        }
    }


//-----------------------------------------------------------------------------
// operator=
// PRIVATE
// Assignment operator.  It duplicates another coord system object.
//-----------------------------------------------------------------------------
HGF2DCoordSysImpl& HGF2DCoordSysImpl::operator=(const HGF2DCoordSysImpl& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        HFCMonitor CacheMonitor(m_CacheKey);
        // DESTRUCTION SEQUENCE
        // Disconnect from last used system
        // THIS OPERATION MUST BE THE FIRST OPERATION
        m_pLastSystem = 0;
        m_pLastModel = 0;
        CacheMonitor.ReleaseKey();

            {
            HFCMonitor Monitor(m_ListsKey);

            // Verify that we are not the m_pRefCoordSys of another coord sys.
            // Technically impossible.
            HASSERT(m_ListIsRefTo.size() == 0);

            // Destroy the transformation models
            ListCoordSysToModel::iterator Itr;
            while (m_ListHasRefTo.size() > 0)
                {
                // take the first element in the map
                Itr = m_ListHasRefTo.begin();

                // Save pointer to coord sys
                HFCPtr<HGF2DCoordSysImpl> pCoordSys = Itr->first;

                // Destroy the transformation model
                delete Itr->second;

                // Remove entry
                m_ListHasRefTo.erase(Itr);

                // Advise other coord sys that reference is removed
                Monitor.ReleaseKey();
                pCoordSys->RemoveAltInverseRef(this);
                pCoordSys = 0;
                Monitor.Assign(m_ListsKey);
                }

            // Advise system it is an alternate system to ... to disconnect
            while (m_ListIsAltRefTo.size() > 0)
                {
                // take the first element in the map
                Itr = m_ListIsAltRefTo.begin();

                // Save pointer to coord sys
                HFCPtr<HGF2DCoordSysImpl> pCoordSys = Itr->first;

                // remove the entry
                m_ListIsAltRefTo.erase(Itr);

                // Advise other coord sys that reference is removed
                Monitor.ReleaseKey();
                pCoordSys->DisconnectFromAltRef(this);
                pCoordSys = 0;
                Monitor.Assign(m_ListsKey);
                }
            }

        // Check if there is a reference
        if (m_HasReference)
            {
            // Advise reference that it is being destroyed
            CacheMonitor.Assign(m_CacheKey);
            m_pRefCoordSys->m_pLastSystem = 0;
            CacheMonitor.ReleaseKey();
            m_pRefCoordSys->RemoveInverseRef(this);
            }

        // Destroy transformation model to reference
        if (m_HasReference)
            {
            delete m_pTransfoModel;
            m_pTransfoModel = 0;
            }

        // COPY SEQUENCE

        // Check if reference parameters must be copied
        if (m_HasReference)
            {
            // Duplicate model to reference
            HAutoPtr<HGF2DTransfoModel> pTempModel(pi_rObj.m_pTransfoModel->Clone());

            // Duplicate smart pointer to reference
            m_pRefCoordSys = pi_rObj.m_pRefCoordSys;

            // Advise reference it is being used
            m_pRefCoordSys->AddInverseRef(pTempModel, this);

            // Release transfo model
            m_pTransfoModel = pTempModel.release();
            }
        else
            {
            m_pTransfoModel = 0;
            m_pRefCoordSys  = 0;
            }

        // Copy has ref attribute
        m_HasReference = pi_rObj.m_HasReference;

        // Indicate that there is no last model
        CacheMonitor.Assign(m_CacheKey);
        m_pLastSystem   = 0;
        m_pLastModel    = 0;
        CacheMonitor.ReleaseKey();
        }

    // Return reference to self
    return (*this);
    }





/** -----------------------------------------------------------------------------
    This method permits to convert a coordinate pair expressed in the specified
    coordinate system to the one represented by self.

    @param pi_rpCoordSys IN Reference to smart pointer to a coordinate system,
                            describing the coordinate system used to express
                            the input coordinates.

    @param pi_XIn IN A double containing the X dimension coordinate value in
                     the present coordinate system.

    @param pi_YIn IN A double containing the Y dimension coordinate value in
                     the present coordinate system.

    @param po_pNewX OUT Pointer to an double that receives the result X
                       dimension coordinate value expressed in the current
                       coordinate system. This value is always returned expressed
                       in the X units of the current coordinate system.

    @param po_pNewY OUT Pointer to an double that receives the result Y
                       dimension coordinate value expressed in the current
                       coordinate system. This value is always returned expressed
                       in the Y units of the current coordinate system.

    @code
        HFCPtr<HGF2DCoordSys>   pImageASystem(new HGF2DCoordSys());

        // A kind of HGF2DTransfoModel
        HGFUTMTransfoModel      RelationOfImageAToUTM (OtherParameters);

        HFCPtr<HGF2DCoordSys>   pUTM_A(new HGF2DCoordSys (RelationOfImageAToUTM,
                                                                      pImageASystem));
        HGF2DLocation           PointA (3.0, 3.0, pImageASystem);

        double     NewRawX;
        double     NewRawY;
        pImageASystem->ConvertFrom (pUTM_A, 2.0, 3.4, &NewRawX, &NewRawY);

    @end

    @see ConvertTo()
    -----------------------------------------------------------------------------
*/
void HGF2DCoordSysImpl::ConvertFrom (const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys,
                                     double               pi_XIn,
                                     double               pi_YIn,
                                     double*              po_pNewX,
                                     double*              po_pNewY) const
    {
    HPRECONDITION(po_pNewX != 0);
    HPRECONDITION(po_pNewY != 0);
    HPRECONDITION(pi_rpCoordSys != 0);

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Conversion asked from itself ... copy values
        *po_pNewX = pi_XIn;
        *po_pNewY = pi_YIn;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);
        // The coordinate system is not itself ... convert
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            // This is the last used system ... convert
            // Convert inverse
            if (m_LastDirection == DIRECT)
                m_pLastModel->ConvertInverse(pi_XIn, pi_YIn, po_pNewX, po_pNewY);
            else
                m_pLastModel->ConvertDirect(pi_XIn, pi_YIn, po_pNewX, po_pNewY);

            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            // Convert inverse
            if (Direct == DIRECT)
                pTheModel->ConvertInverse(pi_XIn, pi_YIn, po_pNewX, po_pNewY);
            else
                pTheModel->ConvertDirect(pi_XIn, pi_YIn, po_pNewX, po_pNewY);
            }
        }
    }

/** -----------------------------------------------------------------------------
    This method permit to convert a coordinate pair expressed in the present
    coordinate system to the one given as first parameter.

    @param pi_rpCoordSys IN Reference to smart pointer to a coordinate system,
                            describing the coordinate system used to express
                            the output coordinates.

    @param pi_XIn IN A double containing the X dimension coordinate value in
                     the present coordinate system.

    @param pi_YIn IN A double containing the Y dimension coordinate value in
                     the present coordinate system.

    @param po_pNewX OUT Pointer to an double that receives the result X
                       dimension coordinate value expressed in the current
                       coordinate system. This value is always returned expressed
                       in the X units of the current coordinate system.

    @param po_pNewY OUT Pointer to an double that receives the result Y
                       dimension coordinate value expressed in the current
                       coordinate system. This value is always returned expressed
                       in the Y units of the current coordinate system.

    @code
        HFCPtr<HGF2DCoordSys>   pImageASystem(new HGF2DCoordSys());

        // A kind of HGF2DTransfoModel
        HGFUTMTransfoModel      RelationOfImageAToUTM (OtherParameters);

        HFCPtr<HGF2DCoordSys>   pUTM_A(new HGF2DCoordSys (RelationOfImageAToUTM,
                                       pImageASystem));
        double  NewRawX;
        double  NewRawY;
        pImageASystem->ConvertTo (pUTM_A, 2.0, 3.4, &NewRawX, &NewRawY);

    @end

    @see ConvertFrom()
    -----------------------------------------------------------------------------
*/
void HGF2DCoordSysImpl::ConvertTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys,
                                  double               pi_XIn,
                                  double               pi_YIn,
                                  double*              po_pNewX,
                                  double*              po_pNewY) const
    {
    HPRECONDITION(po_pNewX != 0);
    HPRECONDITION(po_pNewY != 0);

    HPRECONDITION(pi_rpCoordSys != 0);

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Conversion asked to itself ... copy values
        *po_pNewX = pi_XIn;
        *po_pNewY = pi_YIn;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);
        // The coordinate system is not itself ... convert
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            // This is the last used system ... convert
            // Convert inverse
            if (m_LastDirection == INVERSE)
                m_pLastModel->ConvertInverse(pi_XIn, pi_YIn, po_pNewX, po_pNewY);
            else
                m_pLastModel->ConvertDirect(pi_XIn, pi_YIn, po_pNewX, po_pNewY);

            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            // Convert direct
            if (Direct == DIRECT)
                pTheModel->ConvertDirect(pi_XIn, pi_YIn, po_pNewX, po_pNewY);
            else
                pTheModel->ConvertInverse(pi_XIn, pi_YIn, po_pNewX, po_pNewY);
            }
        }
    }


/** -----------------------------------------------------------------------------
    This method permits to convert a coordinate pair expressed in the specified
    coordinate system to the one represented by self.

    @param pi_rpCoordSys IN Reference to smart pointer to a coordinate system,
                            describing the coordinate system used to express
                            the input coordinates.

    @param pio_pX IN OUT Pointer to an double that contains on input the
                     value to convert and receives the result X dimension
                     coordinate value expressed in the current coordinate
                     system. This value is always returned expressed in the
                     X units of the current coordinate system.

    @param pio_pY IN OUT Pointer to an double that contains on input the
                     value to convert and receives the result Y dimension
                     coordinate value expressed in the current coordinate
                     system. This value is always returned expressed in
                     the Y units of the current coordinate system.

    @code
        HFCPtr<HGF2DCoordSys>   pImageASystem(new HGF2DCoordSys ());

        // A kind of HGF2DTransfoModel
        HGFUTMTransfoModel      RelationOfImageAToUTM (OtherParameters);

        HFCPtr<HGF2DCoordSys>   pUTM_A(new HGF2DCoordSys (RelationOfImageAToUTM,
                                                          pImageASystem));
        NewRawX = 34.56;
        NewRawY = 512.43;
        pImageASystem->ConvertFrom (pUTM_A, &NewRawX, &NewRawY);

    @end

    @see ConvertTo()
    -----------------------------------------------------------------------------
*/
void HGF2DCoordSysImpl::ConvertFrom(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys,
                                    double*              pio_pX,
                                    double*              pio_pY) const
    {
    HPRECONDITION(pio_pX != 0);
    HPRECONDITION(pio_pY != 0);

    HPRECONDITION(pi_rpCoordSys != 0);

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() != this)
        {
        HFCMonitor CacheMonitor(m_CacheKey);
        // The coordinate system is not itself ... convert
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            // This is the last used system ... convert
            // Convert inverse
            if (m_LastDirection == DIRECT)
                m_pLastModel->ConvertInverse(pio_pX, pio_pY);
            else
                m_pLastModel->ConvertDirect(pio_pX, pio_pY);

            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            // Convert inverse
            if (Direct == DIRECT)
                pTheModel->ConvertInverse(pio_pX, pio_pY);
            else
                pTheModel->ConvertDirect(pio_pX, pio_pY);
            }
        }
    }

/** -----------------------------------------------------------------------------
    This method permit to convert a coordinate pair expressed in the present
    coordinate system to the one given as first parameter.

    @param pi_rpCoordSys IN Reference to smart pointer to a coordinate system,
                            describing the coordinate system used to express
                            the output coordinates.

    @param pio_pX IN OUT Pointer to an double that contains on input the
                     value to convert and receives the result X dimension
                     coordinate value expressed in the current coordinate
                     system. This value is always returned expressed in the
                     X units of the current coordinate system.

    @param pio_pY IN OUT Pointer to an double that contains on input the
                     value to convert and receives the result Y dimension
                     coordinate value expressed in the current coordinate
                     system. This value is always returned expressed in
                     the Y units of the current coordinate system.


    @code
        HFCPtr<HGF2DCoordSys>    pImageASystem(new HGF2DCoordSys ());

        // A kind of HGF2DTransfoModel
        HGFUTMTransfoModel        RelationOfImageAToUTM (OtherParameters);

        HFCPtr<HGF2DCoordSys>    pUTM_A(new HGF2DCoordSys (RelationOfImageAToUTM,
                                                      pImageASystem));
        NewRawX = 34.56;
        NewRawY = 512.43;
        pImageASystem->ConvertTo (pUTM_A, &NewRawX, &NewRawY);

    @end

    @see ConvertFrom()
    -----------------------------------------------------------------------------
*/
void HGF2DCoordSysImpl::ConvertTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys,
                                  double*              pio_pX,
                                  double*              pio_pY) const
    {
    HPRECONDITION(pio_pX != 0);
    HPRECONDITION(pio_pY != 0);

    HPRECONDITION(pi_rpCoordSys != 0);

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() != this)
        {
        HFCMonitor CacheMonitor(m_CacheKey);
        // The coordinate system is not itself ... convert
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            // This is the last used system ... convert
            // Convert inverse
            if (m_LastDirection == INVERSE)
                m_pLastModel->ConvertInverse(pio_pX, pio_pY);
            else
                m_pLastModel->ConvertDirect(pio_pX, pio_pY);

            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            // Convert direct
            if (Direct == DIRECT)
                pTheModel->ConvertDirect(pio_pX, pio_pY);
            else
                pTheModel->ConvertInverse(pio_pX, pio_pY);
            }
        }
    }





/** -----------------------------------------------------------------------------
    This method extracts a transformation model from current coordinate system
    to the specified one. There is a corruption risk involved in using this
    method. If, for some reason a coordinate system is destroyed, or a reference
    to a coordinate system is changed through the method SetReference(), the
    model may become invalid. The user is advised never to make any change to
    the tree-like structure of the coordinate system set it is working with,
    while keeping a pointer to a coordinate system.


    @param pi_rpCoordSys IN Reference to a smart pointer to a coordinate system
                            to which a transformation model must be produced.
                            The transformation model may not exist really, since
                            the method always returns a transformation model
                            direct from present to given.

    @return Smart pointer to a transformation model expressing the transformation
            in the direct direction from the present coordinate system to
            specified one.  NULL is returned if no relation is found between the
            two coordinates systems.

    @code
        HFCPtr<HGF2DCoordSys> pImageASys(new HGF2DCoordSys ());

        HFCPtr<HGF2DTranfoModel> pModel (new HGF2DTransfoModel());
        HFCPtr<HGF2DCoordSys> pOtherSys (new HGF2DCoordSys (pModel,
                                                            pImageASys));

        HFCPtr<HGF2DTransfoModel>
                   pFastModel = pImageASys.GetTransfoModelTo (pOtherSys);
    @end

    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HGF2DCoordSysImpl::GetTransfoModelTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Create an identity model
        pResultModel = new HGF2DIdentity();
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);
        // The coordinate system is not itself ...
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            // This is the last used system ...
            // Create a copy
            pResultModel = m_pLastModel->Clone();

            // Check if it is the appropriate direction
            if (m_LastDirection == INVERSE)
                pResultModel->Reverse();

            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            if(Status == HGFCS_SUCCESS)
                {
                // Create a copy
                pResultModel = pTheModel->Clone();

                // Reverse copy if needed
                if (Direct == INVERSE)
                    pResultModel->Reverse();
                }
            else
                {
                // Make sure we return 0
                pResultModel = 0;
                }
            }
        }

    return pResultModel;
    }

/** -----------------------------------------------------------------------------
    This method sets the reference coordinate system for a coordinate system which
    has none, or changes the current reference coordinate system for one that has
    already one. The coordinate system set as a reference must not use the present
    coordinate system either as a direct or indirect reference. The coordinate
    system provided expresses the relationship with newly linked coordinate system.

    Please note that if this method is called on a coordinate system that already
    had a reference, all optimization links will be destroyed. This is true even
    if the same reference is given.

    @param pi_rModel IN A constant reference to the transformation model which
                          expresses the relationship to the given reference system.

    @param pi_rpCoordSys IN Reference to a smart pointer to a coordinate system
                         that becomes the reference system of the present
                         coordinate system.

    @code
        HFCPtr<HGF2DCoordSys>   pImageASys(new HGF2DCoordSys());

        HGF2DIdentity           Model ());
        HFCPtr<HGF2DCoordSys>   pOtherSys (new HGF2DCoordSys (&Model,
                                                               pImageASys));
        HFCPtr<HGF2DCoordSys>   pNewRefSys(new HGF2DCoordSys());

        HGF2DSimilitude         RefModel ());

        pImageASys->SetReference(pNewRefSys);
    @end

    @see GetReference()
    -----------------------------------------------------------------------------
*/
void HGF2DCoordSysImpl::SetReference(const HGF2DTransfoModel& pi_rModel,
                                     const HFCPtr<HGF2DCoordSysImpl>&    pi_rpCoordSys)
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    // The given coordinate system should not be self
    HPRECONDITION(pi_rpCoordSys.GetPtr() != this);

    // Invalidate last used link
    HFCMonitor CacheMonitor(m_CacheKey);
    m_pLastSystem= 0;
    m_pLastModel = 0;
    CacheMonitor.ReleaseKey();

    // Destroy the link to its reference system if it has one
    if (m_HasReference)
        {
        // Advise reference that it is being destroyed
        m_pRefCoordSys->RemoveInverseRef(this);

        CacheMonitor.Assign(m_pRefCoordSys->m_CacheKey);
        m_pRefCoordSys->m_pLastSystem = 0;
        CacheMonitor.ReleaseKey();

        // Destroy previous transfo model
        delete m_pTransfoModel;
        m_pTransfoModel = 0;

        m_HasReference = false;
        }

    // Remove all alternate references
    RemoveAllAlternateRefAndRecurse();

    // The new coordinate system to be used as a reference must not be known at this point
    HDEBUGCODE(HGF2DCoordSysImpl::Status MyState);
    HDEBUGCODE(TransfoDirection Direct);
    HDEBUGCODE(ProtectedGetTransfoModelTo (pi_rpCoordSys, &MyState, &Direct, *this));
    HPOSTCONDITION(MyState != HGFCS_SUCCESS);

    // Copy model
    HAutoPtr<HGF2DTransfoModel> pTempModel(pi_rModel.Clone());

    // Advise reference coordinate system it is ref
    pi_rpCoordSys->AddInverseRef(pTempModel, this);

    // Copy smart pointer to reference system
    m_pRefCoordSys = pi_rpCoordSys;

    // Indicate system has a reference and release model
    m_pTransfoModel = pTempModel.release();
    m_HasReference = true;
    }




/** -----------------------------------------------------------------------------
    Indicates if the relation between self and given coordinate system (the
    transfo model) preserves linearity

    @param pi_rpCoordSys IN Pointer to coordinate system to verify geometric
                            preservation property with.

    @return true if relation between models preserves linearity.

    @see HasShapePreservingRelationTo()
    @see HasDirectionPreservingRelationTo()
    @see HasParallelismPreservingRelationTo()
    -----------------------------------------------------------------------------
*/
bool HGF2DCoordSysImpl::HasLinearityPreservingRelationTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    bool   ReturnValue;

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Relation between itself preserves everything
        ReturnValue = true;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);

        // The coordinate system is not itself ...
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            ReturnValue = m_pLastModel->PreservesLinearity();
            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            ReturnValue = pTheModel->PreservesLinearity();
            }
        }

    return ReturnValue;
    }


/** -----------------------------------------------------------------------------
    Indicates if the relation between self and given coordinate system (the
    transfo model) preserves shape

    @param pi_rpCoordSys IN Pointer to coordinate system to verify geometric
                            preservation property with.

    @return true if relation between models preserves shape.

    @see HasLinearityPreservingRelationTo()
    @see HasDirectionPreservingRelationTo()
    @see HasParallelismPreservingRelationTo()
    -----------------------------------------------------------------------------
*/
bool HGF2DCoordSysImpl::HasShapePreservingRelationTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    bool   ReturnValue;

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Relation between itself preserves everything
        ReturnValue = true;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);

        // The coordinate system is not itself ...
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            ReturnValue = m_pLastModel->PreservesShape();
            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            ReturnValue = pTheModel->PreservesShape();
            }
        }

    return ReturnValue;
    }

/** -----------------------------------------------------------------------------
    Indicates if the relation between self and given coordinate system (the
    transfo model) preserves direction

    @param pi_rpCoordSys IN Pointer to coordinate system to verify geometric
                            preservation property with.

    @return true if relation between models preserves direction.

    @see HasLinearityPreservingRelationTo()
    @see HasShapePreservingRelationTo()
    @see HasParallelismPreservingRelationTo()
    -----------------------------------------------------------------------------
*/
bool HGF2DCoordSysImpl::HasDirectionPreservingRelationTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    bool   ReturnValue;

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Relation between itself preserves everything
        ReturnValue = true;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);

        // The coordinate system is not itself ...
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(pi_rpCoordSys.GetPtr() != 0);
            HASSERT(m_pLastSystem != 0);

            ReturnValue = m_pLastModel->PreservesDirection();
            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            ReturnValue = pTheModel->PreservesDirection();
            }
        }

    return ReturnValue;
    }


/** -----------------------------------------------------------------------------
    Indicates if the relation between self and given coordinate system (the
    transfo model) preserves parallelism

    @param pi_rpCoordSys IN Pointer to coordinate system to verify geometric
                            preservation property with.

    @return true if relation between models preserves parallelism.

    @see HasLinearityPreservingRelationTo()
    @see HasShapePreservingRelationTo()
    @see HasDirectionPreservingRelationTo()
    -----------------------------------------------------------------------------
*/
bool HGF2DCoordSysImpl::HasParallelismPreservingRelationTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    bool   ReturnValue;

    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Relation between itself preserves everything
        ReturnValue = true;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);

        // The coordinate system is not itself ...
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            ReturnValue = m_pLastModel->PreservesParallelism();
            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            ReturnValue = pTheModel->PreservesParallelism();
            }
        }

    return ReturnValue;
    }

/** -----------------------------------------------------------------------------
    Indicates if the relation between self and given coordinate system (the
    transfo model) can be expressed using uniquely scalings and translation.

    @param pi_rpCoordSys IN Pointer to coordinate system to verify geometric
                            preservation property with.

    @return true if relation between models is a stretch

    -----------------------------------------------------------------------------
*/
bool HGF2DCoordSysImpl::HasStretchRelationTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    bool   ReturnValue;


    // Check that the coordinate system is not itself
    if (pi_rpCoordSys.GetPtr() == this)
        {
        // Relation between itself preserves everything
        ReturnValue = true;
        }
    else
        {
        HFCMonitor CacheMonitor(m_CacheKey);

        // The coordinate system is not itself ...
        // Check if this is not the last used system
        if (m_pLastSystem == pi_rpCoordSys.GetPtr())
            {
            HASSERT(m_pLastSystem != 0);

            ReturnValue = m_pLastModel->IsStretchable();
            CacheMonitor.ReleaseKey();
            }
        else
            {
            CacheMonitor.ReleaseKey();

            TransfoDirection        Direct;
            HGF2DCoordSysImpl::Status   Status;

            // Find transformation model
            HGF2DTransfoModel* pTheModel = (const_cast<HGF2DCoordSysImpl*>(this))->FindOrCreateTransfoModel(pi_rpCoordSys, &Status, &Direct);

            // Check if one was found
            HASSERT(Status == HGFCS_SUCCESS);

            ReturnValue = pTheModel->IsStretchable();
            }
        }

    return ReturnValue;
    }


#ifdef __HMR_DEBUG

size_t HGF2DCoordSysImpl::GetBranchCount() const
    {
    ListCoordSysToModel::const_iterator Itr(m_ListIsRefTo.begin());
    size_t BranchCount = m_ListIsRefTo.size();
    while (Itr != m_ListIsRefTo.end())
        {
        BranchCount += Itr->first->GetBranchCount();
        Itr++;
        }

    return BranchCount;
    }

size_t HGF2DCoordSysImpl::GetCoordSysCount() const
    {
    return 0;
    }

#endif


//-----------------------------------------------------------------------------
// FindOrCreateTransfoModel
// This private method searches for the coordinate systems,
// for a transformation model from present system to specified one. If no model
// can be found, one will be constructed. The model found can be either direct or
// inverse. The proper direction to transform from present to specified is returned
// as a enum value in the given variable.
// Note that this method will create new transformation models if no direct path
// can be found from present system to specified one.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DCoordSysImpl::FindOrCreateTransfoModel(const HFCPtr<HGF2DCoordSysImpl>&    pi_rpCoordSys,
                                                               Status*           po_pStatus,
                                                               TransfoDirection* po_pDirect)
    {
    HPRECONDITION(po_pDirect != 0);
    HPRECONDITION(po_pStatus != 0);

    HGF2DTransfoModel* pResultModel;

    // Check in known systems
    pResultModel = FindNearTransfoModel (pi_rpCoordSys, po_pStatus, po_pDirect);

    // Check if one was found
    if (*po_pStatus != HGFCS_SUCCESS)
        {
        // No transfo model was found in known system ... try on the far side
        pResultModel = SearchAndCreateBridgeTransfoModel (pi_rpCoordSys, po_pStatus, po_pDirect);
        }

    return pResultModel;
    }



//-----------------------------------------------------------------------------
// FindNearTransfoModel
// This private method searches for the directly known coordinate systems,
// for a transformation model from present system to specified one. If no model
// can be found, HGF2DCoordSys::NO_RELATION is returned in po_pStatus.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DCoordSysImpl::FindNearTransfoModel(const HFCPtr<HGF2DCoordSysImpl>&   pi_rpCoordSys,
                                                           Status* po_pStatus,
                                                           TransfoDirection*      po_pDirect)
    {
    HPRECONDITION(po_pStatus != 0);
    HPRECONDITION(po_pDirect != 0);

    HGF2DTransfoModel* pResultModel = 0;
    *po_pStatus = HGFCS_NO_RELATION_FOUND;

    HFCMonitor CacheMonitor(m_CacheKey);

    // Check if this system is the last asked for system
    if (m_pLastSystem == pi_rpCoordSys)
        {
        HASSERT(m_pLastSystem != 0);

        // This is the last system that was asked for ... return last transfo model
        pResultModel = m_pLastModel;
        *po_pDirect = m_LastDirection;
        CacheMonitor.ReleaseKey();
        *po_pStatus = HGFCS_SUCCESS;
        }
    else
        {
        CacheMonitor.ReleaseKey();

        // This system is not the last asked for ... search in directly known systems
        // First check the reference system
        if (m_HasReference && m_pRefCoordSys == pi_rpCoordSys)
            {
            CacheMonitor.Assign(m_CacheKey);

            // It is the reference system ... set this as last system
            m_pLastSystem = m_pRefCoordSys;

            // Set its model as last
            m_pLastModel = m_pTransfoModel;

            // Save direction
            m_LastDirection = DIRECT;

            // Set return values
            pResultModel = m_pLastModel;
            *po_pDirect = m_LastDirection;
            *po_pStatus = HGFCS_SUCCESS;

            CacheMonitor.ReleaseKey();
            }
        else
            {
            HFCMonitor Monitor(m_ListsKey);

            // It is not the reference ... try for system it is a reference to
            ListCoordSysToModel::iterator Itr = m_ListIsRefTo.find(pi_rpCoordSys);

            // Check if found
            if (Itr != m_ListIsRefTo.end())
                {
                CacheMonitor.Assign(m_CacheKey);
                // The present system is a reference to specified one
                // It is the reference system ... set this as last system
                m_pLastSystem = Itr->first;

                // Set its model as last
                m_pLastModel = Itr->second;

                // Save direction
                m_LastDirection = INVERSE;

                // Set return values
                pResultModel = m_pLastModel;
                *po_pDirect = m_LastDirection;
                *po_pStatus = HGFCS_SUCCESS;

                CacheMonitor.ReleaseKey();
                }
            else
                {
                // The present system is not a reference to specified
                // Try composed list direct
                ListCoordSysToModel::iterator Itr;
                Itr = m_ListHasRefTo.find(pi_rpCoordSys);

                // Check if found
                if (Itr != m_ListHasRefTo.end())
                    {
                    CacheMonitor.Assign(m_CacheKey);

                    // The present system has a reference to specified one
                    m_pLastSystem = Itr->first;

                    // Set its model as last
                    m_pLastModel = Itr->second;

                    // Save direction
                    m_LastDirection = DIRECT;

                    // Set return values
                    pResultModel = m_pLastModel;
                    *po_pDirect = m_LastDirection;
                    *po_pStatus = HGFCS_SUCCESS;

                    CacheMonitor.ReleaseKey();
                    }
                else
                    {
                    // Maybe it is an alternate reference to this one ...
                    Itr = m_ListIsAltRefTo.find(pi_rpCoordSys);

                    // Check if found
                    if (Itr != m_ListIsAltRefTo.end())
                        {
                        CacheMonitor.Assign(m_CacheKey);

                        // The present system is an alternate reference to specified one
                        m_pLastSystem = Itr->first;

                        // Set its model as last
                        m_pLastModel = Itr->second;

                        // Save direction
                        m_LastDirection = INVERSE;

                        // Set return values
                        pResultModel = m_pLastModel;
                        *po_pDirect = m_LastDirection;
                        *po_pStatus = HGFCS_SUCCESS;

                        CacheMonitor.ReleaseKey();
                        }
                    }
                }
            }
        }

    return pResultModel;
    }




//-----------------------------------------------------------------------------
// SearchAndCreateBridgeTransfoModel
// PROTECTED
// This method should only be called when search for a known system has failed.
// This private method searches for a non-directly known coordinate systems,
// for a transformation model from present system to specified one. If no model
// can be found, HGF2DCoordSys::NO_RELATION is returned.
// This method will ask other known coordinate system for a relation to the specified
// coordinate system. The search starts by the reference system. It this fails to
// procuce a path, then search is started on systems it is a reference to. If
// this also fails, then search is started on the list of system it has a
// reference to.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DCoordSysImpl::SearchAndCreateBridgeTransfoModel(const HFCPtr<HGF2DCoordSysImpl>&         pi_rpCoordSys,
                                                                        Status* po_pStatus,
                                                                        TransfoDirection*      po_pDirect)
    {
    HPRECONDITION(po_pStatus != 0);
    HPRECONDITION(po_pDirect != 0);

    HAutoPtr<HGF2DTransfoModel> pResultModel;

    *po_pStatus = HGFCS_NO_RELATION_FOUND;

    // First check the reference system
    if (m_HasReference)
        {
        HFCPtr<HGF2DTransfoModel> pTempModel = m_pRefCoordSys->ProtectedGetTransfoModelTo(pi_rpCoordSys,
                                               po_pStatus,
                                               po_pDirect,
                                               *this);

        // Check if it was found
        if (*po_pStatus == HGFCS_SUCCESS)
            {
            // A transformation was found ... add our part
            HFCPtr<HGF2DTransfoModel> pTempModel2;
            pTempModel2 = m_pTransfoModel->ComposeInverseWithDirectOf(*pTempModel);
            pResultModel = pTempModel2->Clone();

            // Save path in ref to list
            SaveNewPathTo (*pResultModel, pi_rpCoordSys);

            HFCMonitor CacheMonitor(m_CacheKey);
            m_pLastSystem = pi_rpCoordSys;
            m_pLastModel = pResultModel;
            m_LastDirection = DIRECT;
            CacheMonitor.ReleaseKey();

            *po_pStatus = HGFCS_SUCCESS;
            *po_pDirect = DIRECT;
            }
        }

    // Check if previous search was successful
    if (*po_pStatus != HGFCS_SUCCESS)
        {
        // No path was found try systems it is a reference to
        bool Found = false;
        HFCPtr<HGF2DTransfoModel> pTempModel;

        HFCMonitor Monitor(m_ListsKey);
        ListCoordSysToModel::iterator FoundItr;
        ListCoordSysToModel::iterator Itr;

        for (Itr = m_ListIsRefTo.begin(); (!Found) && (Itr != m_ListIsRefTo.end()); ++Itr)
            {
            pTempModel = ((*Itr).first)->ProtectedGetTransfoModelTo(pi_rpCoordSys,
                                                                    po_pStatus,
                                                                    po_pDirect,
                                                                    *this);

            if ((Found = (*po_pStatus == HGFCS_SUCCESS)))
                FoundItr = Itr;
            }

        // Check if found
        if (Found)
            {
            // Create copy of model to found system
            HAutoPtr<HGF2DTransfoModel> pNewModel;
            pNewModel = ((*FoundItr).second)->Clone();
            Monitor.ReleaseKey();

            // Reverse the model
            pNewModel->Reverse();

            // A transformation was found ... add our part
            HFCPtr<HGF2DTransfoModel> pTempModel2;
            pTempModel2 = pNewModel->ComposeInverseWithDirectOf(*pTempModel);
            pResultModel = pTempModel2->Clone();

            // Delete the temporary model allocated
            delete pNewModel.release();

            // Save path in ref to list
            SaveNewPathTo(*pResultModel, pi_rpCoordSys);

            HFCMonitor CacheMonitor(m_CacheKey);
            m_pLastSystem = pi_rpCoordSys;
            m_pLastModel = pResultModel;
            m_LastDirection = DIRECT;
            CacheMonitor.ReleaseKey();

            *po_pDirect = DIRECT;
            }
        }

    return(pResultModel.release());
    }


//-----------------------------------------------------------------------------
// ProtectedGetTransfoModelTo
// PROTECTED
// This method searches for a transformation model through the known relations.
// An initiator coordinate system is given , to prevent the search from
// backtracking in the direction of the caller. A new model is always returned
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DCoordSysImpl::ProtectedGetTransfoModelTo(
    const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys,
    Status*   po_pStatus,
    TransfoDirection*        po_pDirect,
    const HGF2DCoordSysImpl&     pi_rInitiatorSystem) const
    {
    HPRECONDITION(po_pStatus != 0);
    HPRECONDITION(po_pDirect != 0);

    HPRECONDITION(pi_rpCoordSys != &pi_rInitiatorSystem);

    HFCPtr<HGF2DTransfoModel> pResultModel;

    *po_pStatus = HGFCS_NO_RELATION_FOUND;

    // First check itself
    if (pi_rpCoordSys == this)
        {
        // It is itself ... return a neutral model
        pResultModel = new HGF2DIdentity ();
        *po_pStatus = HGFCS_SUCCESS;
        *po_pDirect = DIRECT;
        }
    else
        {
        HFCMonitor Monitor(m_ListsKey);

        // Try composed list direct
        ListCoordSysToModel::const_iterator Itr;
        Itr = m_ListHasRefTo.find(pi_rpCoordSys);

        // Check if found
        if (Itr != m_ListHasRefTo.end())
            {
            // Set return values
            pResultModel = Itr->second->Clone();
            *po_pDirect = DIRECT;
            *po_pStatus = HGFCS_SUCCESS;
            }
        else
            {
            // Maybe it is an alternate reference to this one ...
            Itr = m_ListIsAltRefTo.find(pi_rpCoordSys);

            // Check if found
            if (Itr != m_ListIsAltRefTo.end())
                {
                // Set return values
                pResultModel = Itr->second->Clone();
                pResultModel->Reverse();
                *po_pDirect = DIRECT;
                *po_pStatus = HGFCS_SUCCESS;
                }
            }
        }

    if (*po_pStatus != HGFCS_SUCCESS && m_HasReference)
        {
        // Check if it is initiator
        if (&pi_rInitiatorSystem != m_pRefCoordSys.GetPtr())
            {
            // It is not the initiator system

            // Check if the system we seek is the reference
            if (pi_rpCoordSys == m_pRefCoordSys)
                {
                // The system we want is the reference ... clone the model
                pResultModel = m_pTransfoModel->Clone();
                *po_pStatus = HGFCS_SUCCESS;
                *po_pDirect = DIRECT;
                }
            else
                {
                HFCPtr<HGF2DTransfoModel> pTempModel;
                pTempModel = m_pRefCoordSys->ProtectedGetTransfoModelTo(pi_rpCoordSys,
                                                                        po_pStatus,
                                                                        po_pDirect,
                                                                        *this);

                // Check if it was found
                if (*po_pStatus == HGFCS_SUCCESS)
                    {
                    // A transformation was found ... add our part
                    pResultModel = m_pTransfoModel->ComposeInverseWithDirectOf (*pTempModel);

                    *po_pDirect = DIRECT;

                    // Save path in ref to list
                    const_cast<HGF2DCoordSysImpl*>(this)->SaveNewPathTo(*(pResultModel->Clone()),
                                                                        pi_rpCoordSys);
                    }
                }
            }
        }

    // Check if previous search was successful
    if (*po_pStatus != HGFCS_SUCCESS)
        {
        HFCMonitor Monitor(m_ListsKey);

        // Try locating the system in the known one that use self as reference
        ListCoordSysToModel::const_iterator Itr = m_ListIsRefTo.find(pi_rpCoordSys);

        // Check if found
        if (Itr != m_ListIsRefTo.end())
            {
            // It was found ... duplicate model
            pResultModel = Itr->second->Clone();

            // reverse copy
            pResultModel->Reverse();

            *po_pStatus = HGFCS_SUCCESS;
            *po_pDirect = DIRECT;
            }
        else
            {
            // No path was found through systems it is a reference to
            bool Found = false;
            HFCPtr<HGF2DTransfoModel> pTempModel;

            ListCoordSysToModel::const_iterator FoundIndex;
            ListCoordSysToModel::const_iterator Index;

            for (Index = m_ListIsRefTo.begin(); (!Found) && (Index != m_ListIsRefTo.end()); ++Index)
                {
                // Check that it is not the not the initiator system
                if (Index->first != &pi_rInitiatorSystem)
                    {
                    // This is not the initiator system

                    // This is not the model desired
                    pTempModel = Index->first->ProtectedGetTransfoModelTo(pi_rpCoordSys,
                                                                          po_pStatus,
                                                                          po_pDirect,
                                                                          *this);

                    if ((Found = (*po_pStatus == HGFCS_SUCCESS)))
                        FoundIndex = Index;
                    }
                }


            // Check if found
            if (Found)
                {
                // Create copy of model to found system
                HAutoPtr<HGF2DTransfoModel> pNewModel;
                pNewModel = FoundIndex->second->Clone();

                // Reverse the model
                pNewModel->Reverse();

                // A transformation was found ... add our part
                pResultModel = pNewModel->ComposeInverseWithDirectOf(*pTempModel);

                // Delete the temporary model allocated
                delete pNewModel.release();

                // Save path in ref to list
                *po_pDirect = DIRECT;

                // Save path in ref to list
                const_cast<HGF2DCoordSysImpl*>(this)->SaveNewPathTo(*(pResultModel->Clone()), pi_rpCoordSys);
                }
            }
        }

    return pResultModel;
    }





//-----------------------------------------------------------------------------
// SaveNewPathTo
// This method adds a new path to the present system. It also warns the
// other system, that a reference is added to it.
//-----------------------------------------------------------------------------
void HGF2DCoordSysImpl::SaveNewPathTo(HGF2DTransfoModel& pi_rModel,
                                      const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys)
    {
    HFCMonitor Monitor(m_ListsKey);

    // Add system and Model in the list
    m_ListHasRefTo.insert (ListCoordSysToModel::value_type(pi_rpCoordSys,
                                                           &pi_rModel));
    Monitor.ReleaseKey();

    // Advise other system it is added as a reference
    pi_rpCoordSys->AddAltInverseRef (&pi_rModel, this);
    }


//-----------------------------------------------------------------------------
// AddInverseRef
// PRIVATE
// This private method is used to add inverse reference to a coord system.
// This inverse reference indicate that the present coord system is used as
// a reference to another coordinate system.
//-----------------------------------------------------------------------------
void HGF2DCoordSysImpl::AddInverseRef(HGF2DTransfoModel* pi_pModel,
                                      const HFCPtr<HGF2DCoordSysImpl>&  pi_rpCoordSys)
    {
    HPRECONDITION(pi_pModel != 0);
    HPRECONDITION(pi_rpCoordSys != 0);

    // Add coordinate system and reference to model
    HFCMonitor Monitor(m_ListsKey);
    m_ListIsRefTo.insert(ListCoordSysToModel::value_type(pi_rpCoordSys, pi_pModel));
    }

//-----------------------------------------------------------------------------
// RemoveInverseRef
// PRIVATE
// This private method is used to remove inverse reference to a coord system.
// This inverse reference indicate that the present coord system is used as
// a reference to another coordinate system. Usually the inversaly refered system
// calls this method to this ref to indicate destruction
//-----------------------------------------------------------------------------
void  HGF2DCoordSysImpl::RemoveInverseRef(const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys)
    {
    HFCMonitor Monitor(m_ListsKey);
    HPRECONDITION(pi_rpCoordSys != 0);

    ListCoordSysToModel::iterator Itr = m_ListIsRefTo.find(pi_rpCoordSys);

    // If it was found
    if (Itr != m_ListIsRefTo.end())
        {
        // Hhum!, We must to reset the LastSystem to remove the cyclic reference
        // between CoordSys.
        // Ex. HGF2DCoordSys C1, C2
        //     HRARaster R1(C1), R2 (C2)
        //
        //     R1.ChangeCoordSys (C2)   --> C1.LastSystem = C2 --> C1.RefCount=2
        //     R2.ChangeCoordSys (C1)   --> C2.LastSystem = C1 --> C2.RefCount=2
        //
        // Now, if you delete C1 and C2, the objects is not deleted, because the refCounts are
        // == 1
        HFCMonitor CacheMonitor(m_CacheKey);
        m_pLastSystem= 0;
        m_pLastModel = 0;
        CacheMonitor.ReleaseKey();

        // Remove coord sys reference
        m_ListIsRefTo.erase (Itr);
        Monitor.ReleaseKey();
        }
    }

//-----------------------------------------------------------------------------
// RemoveAllAlternateRefAndRecurse
//-----------------------------------------------------------------------------
void HGF2DCoordSysImpl::RemoveAllAlternateRefAndRecurse ()
    {
    list<HFCPtr<HGF2DCoordSysImpl> > CopyListIsRefTo;

    // Invalidate last used link
    HFCMonitor CacheMonitor(m_CacheKey);
    m_pLastSystem= 0;
    m_pLastModel = 0;
    CacheMonitor.ReleaseKey();

        {
        ListCoordSysToModel::iterator Itr;
        HFCMonitor Monitor(m_ListsKey);

        // Remove all alternate links

        // Remove all direct alternate links and
        // Destroy the transformation models
        while (m_ListHasRefTo.size() > 0)
            {
            // take the first element in the map
            Itr = m_ListHasRefTo.begin();

            // Save pointer to coord sys
            HFCPtr<HGF2DCoordSysImpl> pCoordSys = Itr->first;

            // Destroy the transformation model
            delete Itr->second;

            // Remove entry
            m_ListHasRefTo.erase(Itr);

            // Advise other coord sys that reference is removed
            Monitor.ReleaseKey();
            pCoordSys->RemoveAltInverseRef(this);
            pCoordSys = 0;
            Monitor.Assign(m_ListsKey);
            }

        // Remove links it is an alternate reference to
        // Advise system it is an alternate system to ... to disconnect
        while (m_ListIsAltRefTo.size() > 0)
            {
            // take the first element in the map
            Itr = m_ListIsAltRefTo.begin();

            // Save pointer to coord sys
            HFCPtr<HGF2DCoordSysImpl> pCoordSys = Itr->first;

            // Remove entry
            m_ListIsAltRefTo.erase(Itr);

            // Advise other coord sys that reference is removed
            Monitor.ReleaseKey();
            pCoordSys->DisconnectFromAltRef(this);
            pCoordSys = 0;
            Monitor.Assign(m_ListsKey);
            }

        // make a copy of the coordsys in m_ListIsRefTo.
        for (Itr = m_ListIsRefTo.begin(); Itr != m_ListIsRefTo.end(); Itr++)
            CopyListIsRefTo.push_back(HFCPtr<HGF2DCoordSysImpl>(Itr->first));
        }

    // At this point there is no more alternate links to or from the present one

    // Now we ask coordinate system that use the present as reference to do likewise
    list<HFCPtr<HGF2DCoordSysImpl> >::const_iterator Itr;
    for (Itr = CopyListIsRefTo.begin(); Itr != CopyListIsRefTo.end() ; Itr++)
        {
        // Recurse
        (*Itr)->RemoveAllAlternateRefAndRecurse();
        }
    }


//-----------------------------------------------------------------------------
// DisconnectFromAltRef
//-----------------------------------------------------------------------------
void  HGF2DCoordSysImpl::DisconnectFromAltRef (const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys)
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    HFCMonitor Monitor(m_ListsKey);
    ListCoordSysToModel::iterator Itr = m_ListHasRefTo.find(pi_rpCoordSys);

    // Check if found
    if (Itr != m_ListHasRefTo.end())
        {
        // Destroy the transformation model
        delete Itr->second;

        // Remove coord sys and model reference
        m_ListHasRefTo.erase (Itr);
        Monitor.ReleaseKey();

        // Hhum!, We must to reset the LastSystem to remove the cyclic reference
        // between CoordSys.
        // Ex. HGF2DCoordSys C1, C2
        //     HRARaster R1(C1), R2 (C2)
        //
        //     R1.ChangeCoordSys (C2)   --> C1.LastSystem = C2 --> C1.RefCount=2
        //     R2.ChangeCoordSys (C1)   --> C2.LastSystem = C1 --> C2.RefCount=2
        //
        // Now, if you delete C1 and C2, the objects is not deleted, because the refCounts are
        // == 1
        HFCMonitor CacheMonitor(m_CacheKey);
        m_pLastSystem= 0;
        m_pLastModel = 0;
        CacheMonitor.ReleaseKey();
        }
    }




//-----------------------------------------------------------------------------
// AddAltInverseRef
// PRIVATE
// This private method is used to add inverse alternate reference to a coord system.
// This inverse alternate reference indicate that the present coord system is used as
// a reference to another coordinate system.
//-----------------------------------------------------------------------------
void  HGF2DCoordSysImpl::AddAltInverseRef(HGF2DTransfoModel* pi_pModel,
                                          const HFCPtr<HGF2DCoordSysImpl>&  pi_rpCoordSys)
    {
    HPRECONDITION(pi_pModel != 0);
    HPRECONDITION(pi_rpCoordSys != 0);

    // Add coordinate system and reference to model
    HFCMonitor Monitor(m_ListsKey);
    m_ListIsAltRefTo.insert(ListCoordSysToModel::value_type(pi_rpCoordSys, pi_pModel));
    }


//-----------------------------------------------------------------------------
// RemoveAltInverseRef
// PRIVATE
// This private method is used to remove inverse reference to a coord system.
// This inverse reference indicate that the present coord system is used as
// a reference to another coordinate system. Usually the inversaly refed system
// calls this method to this ref to indicate destruction
//-----------------------------------------------------------------------------
void  HGF2DCoordSysImpl::RemoveAltInverseRef (const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys)
    {
    HPRECONDITION(pi_rpCoordSys != 0);

    HFCMonitor Monitor(m_ListsKey);
    ListCoordSysToModel::iterator Itr = m_ListIsAltRefTo.find(pi_rpCoordSys);

    // Check if found
    if (Itr != m_ListIsAltRefTo.end())
        {
        // Hhum!, We must to reset the LastSystem to remove the cyclic reference
        // between CoordSys.
        // Ex. HGF2DCoordSys C1, C2
        //     HRARaster R1(C1), R2 (C2)
        //
        //     R1.ChangeCoordSys (C2)   --> C1.LastSystem = C2 --> C1.RefCount=2
        //     R2.ChangeCoordSys (C1)   --> C2.LastSystem = C1 --> C2.RefCount=2
        //
        // Now, if you delete C1 and C2, the objects is not deleted, because the refCounts are
        // == 1
        HFCMonitor CacheMonitor(m_CacheKey);
        m_pLastSystem= 0;
        m_pLastModel = 0;
        CacheMonitor.ReleaseKey();

        // Remove coord sys and model reference
        m_ListIsAltRefTo.erase (Itr);
        Monitor.ReleaseKey();
        }
    }


