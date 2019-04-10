/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DGNModelUtilities.hpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder::DGNFileHolder ()
    :   m_dgnFileP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder::DGNFileHolder    (DgnFileP    dgnFileP)
    :   m_dgnFileP(dgnFileP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a file holder for a dgnFile object whose ref count already have
*               been incremented (which is the case for a working/manually opened dgn
*               file). It is required that returned holder become the first
*               owner ever of the dgn file instance or said otherwise that dgn file
*               passed as parameter had no prior owner.
*
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder DGNFileHolder::CreateFromWorking (DgnFileP dgnFileP)
    {        
    assert(0 == dgnFileP || 1 == dgnFileP->GetFileRefCount());

    return DGNFileHolder(dgnFileP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a file holder for a dgnFile object obtains from a ref counted ptr.
*
* @bsimethod                                                 Mathieu.St-Pierre    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder DGNFileHolder::CreateFromWorking (DgnFilePtr dgnFilePtr)
    {            
    if (dgnFilePtr.get() != 0)
        {
        dgnFilePtr->AddRef();
        }

    return DGNFileHolder(dgnFilePtr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a file holder for an existing dgnFile (which is the case for
*               an active dgn file).
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder DGNFileHolder::CreateFromActive (DgnFileP dgnFileP)
    {
    if (0 != dgnFileP)
        {
        assert(1 <= dgnFileP->GetFileRefCount());
        dgnFileP->AddRef();
        }

    return DGNFileHolder(dgnFileP);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder::DGNFileHolder (const DGNFileHolder& rhs)
    :   m_dgnFileP(rhs.m_dgnFileP)
    {
    if (0 != m_dgnFileP)
        m_dgnFileP->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder& DGNFileHolder::operator= (const DGNFileHolder& rhs)
    {
    DGNFileHolder newHolder(rhs);
    std::swap(newHolder.m_dgnFileP, m_dgnFileP);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNFileHolder::~DGNFileHolder ()
    {
    if (0 != m_dgnFileP)
        m_dgnFileP->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DgnFileP DGNFileHolder::GetP () const
    {
    return m_dgnFileP;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void DGNFileHolder::Reset ()
    {
    DGNFileHolder newHolder;
    std::swap(newHolder.m_dgnFileP, m_dgnFileP);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNModelRefHolder::Impl : public HFCShareableObject<DGNModelRefHolder::Impl>
    {
    DgnModelRefP                    m_modelRefP;

    virtual                         ~Impl                                  () = 0
        {
        assert(0 == m_modelRefP);
        }

protected:
    explicit                        Impl                                   ()
        :   m_modelRefP(0)
        {
        }

    explicit                        Impl                                   (DgnModelRefP            modelRefP)
        :   m_modelRefP(modelRefP)
        {
        }

    void                            FreeReferenceModel                         ()
        {
        if (0 != m_modelRefP)
            m_modelRefP = 0;
        }

    void                            FreeWorkingModel                           ()
        {
        if (0 != m_modelRefP)
            {            
            DgnModelP  dgnCache = m_modelRefP->GetDgnModelP();

            assert(dgnCache != 0);
            
            dgnCache->Release();
            
            m_modelRefP = 0;
            }
        }

    void                            FreeActiveModel                            ()
        {
        if (0 != m_modelRefP)
            m_modelRefP = 0;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNModelRefHolder::NullImpl : public Impl
    {
    explicit                        NullImpl                               ()
        :   Impl()
        {
        }

    virtual                         ~NullImpl                              ()
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNModelRefHolder::WorkingImpl : public Impl
    {
    DGNFileHolder                   m_dgnFile;
    
    explicit                        WorkingImpl                            (const DGNFileHolder&    dgnFile,
                                                                            DgnModelRefP            modelRefP)
        :   Impl(modelRefP),
            m_dgnFile(dgnFile)
        {
        assert(dgnFile.GetP() == modelRefP->GetDgnFileP());        
        assert(!modelRefP->IsDgnAttachment());
        }

    explicit                        WorkingImpl                            (DgnModelRefP            modelRefP)
        :   Impl(modelRefP),
            m_dgnFile(DGNFileHolder::CreateFromWorking(modelRefP->GetDgnFileP()))
        {
        assert(!modelRefP->IsDgnAttachment());
        }

    virtual                         ~WorkingImpl                           ()
        {
        FreeWorkingModel();
        }

    };



/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNModelRefHolder::ActiveImpl : public Impl
    {
    DGNFileHolder                   m_dgnFile;

    explicit                        ActiveImpl                             (const DGNFileHolder&    dgnFile,
                                                                            DgnModelRefP            modelRefP)
        :   Impl(modelRefP),
            m_dgnFile(dgnFile)
        {
        assert(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef() == modelRefP);
        assert(dgnFile.GetP() == modelRefP->GetDgnFileP());        
        assert(!modelRefP->IsDgnAttachment());
        }

    explicit                        ActiveImpl                             (DgnModelRefP            modelRefP)
        :   Impl(modelRefP),
            m_dgnFile(DGNFileHolder::CreateFromActive(modelRefP->GetDgnFileP()))
        {
        assert(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef() == modelRefP);
        assert(!modelRefP->IsDgnAttachment());
        }

    virtual                         ~ActiveImpl                            ()
        {
        FreeActiveModel();
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNModelRefHolder::ReferenceImpl : public Impl
    {
    DGNModelRefHolder                  m_rootModelRef;

    explicit                        ReferenceImpl                          (const DGNModelRefHolder&   rootModelRef,
                                                                            DgnModelRefP            referenceModelRefP)
        :   Impl(referenceModelRefP),
            m_rootModelRef(rootModelRef)
        {
        assert(referenceModelRefP->IsDgnAttachment());
        }

    virtual                         ~ReferenceImpl                         ()
        {
        FreeReferenceModel();
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder::DGNModelRefHolder (Impl* implP)
    :   m_implP(implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a holder for a working model.
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder DGNModelRefHolder::CreateFromWorking  (const DGNFileHolder&    dgnFile,
                                                                DgnModelRefP            modelRefP)
    {
    assert(0 != dgnFile.GetP());
    assert(0 != modelRefP);
    return DGNModelRefHolder(new WorkingImpl(dgnFile, modelRefP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a holder for a working model.
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder DGNModelRefHolder::CreateFromWorking (DgnModelRefP modelRefP)
    {
    assert(0 != modelRefP);
    return DGNModelRefHolder(new WorkingImpl(modelRefP));
    }

/*---------------------------------------------------------------------------------**//**
* @description   Create a holder for an active or existing model.
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder DGNModelRefHolder::CreateFromActive   (const DGNFileHolder&    dgnFile,
                                                                DgnModelRefP            modelRefP)
    {
    assert(0 != dgnFile.GetP());
    assert(0 != modelRefP);
    return DGNModelRefHolder(new ActiveImpl(dgnFile, modelRefP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a holder for an active or existing model.
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder DGNModelRefHolder::CreateFromActive  (DgnModelRefP modelRefP)
    {
    assert(0 != modelRefP);
    return DGNModelRefHolder(new ActiveImpl(modelRefP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  Create a holder for a reference model specifying the root model of
*               this reference.
* @bsimethod                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder DGNModelRefHolder::CreateFromReference    (const DGNModelRefHolder&    rootModel,
                                                                    DgnModelRefP                referenceModelRefP)
    {
    assert(0 != referenceModelRefP);
    return DGNModelRefHolder(new ReferenceImpl(rootModel, referenceModelRefP));
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder::DGNModelRefHolder ()
    :   m_implP(new NullImpl)
    {
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder::DGNModelRefHolder (const DGNModelRefHolder&   rhs)
    :   m_implP(rhs.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DGNModelRefHolder& DGNModelRefHolder::operator= (const DGNModelRefHolder& rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline DgnModelRefP DGNModelRefHolder::GetP () const
    {
    return m_implP->m_modelRefP;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                 Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline void DGNModelRefHolder::Reset ()
    {
    m_implP = new NullImpl;
    }
