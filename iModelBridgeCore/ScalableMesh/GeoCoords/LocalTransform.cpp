/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/LocalTransform.cpp $
|    $RCSfile: LocalTransform.cpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/11/23 21:47:17 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/GeoCoords/LocalTransform.h>
#include <ScalableMesh/Foundations/Exception.h>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

enum State
    {
    STATE_NOTHING,
    STATE_SIMPLEX_TO_LOCAL,
    STATE_SIMPLEX_TO_GLOBAL,
    STATE_DUPLEX,
    };


static State                                GetCommonState         (State                               lhs,
                                                                    State                               rhs)
    {
    return static_cast<State>(lhs & rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalTransform::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    TransfoModel                            m_localToGlobal;
    TransfoModel                            m_globalToLocal;
    State                                   m_state;

    explicit                                Impl                   (const TransfoModel&                 localToGlobal,
                                                                    const TransfoModel&                 globalToLocal,
                                                                    State                               state)
        :   m_localToGlobal(localToGlobal),
            m_globalToLocal(globalToLocal),
            m_state(state)
        {
        }

    static void                             UpdateForEdit          (ImplPtr&                            instancePtr)
        {
        // Copy on write when config is shared
        if (instancePtr->IsShared())
            instancePtr = new Impl(*instancePtr);
        }


    bool                                    IsEquivalent           (const Impl&                         rhs) const;

    bool                                    HasLocalToGlobal         ()
        {
        return STATE_SIMPLEX_TO_GLOBAL == m_state;
        }

    bool                                    HasGlobalToLocal         ()
        {
        return STATE_SIMPLEX_TO_LOCAL == m_state;
        }

    bool                                    HasDuplex              ()
        {
        return STATE_DUPLEX == m_state;
        }

    };





/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline LocalTransform::LocalTransform (Impl* implP)
    :   m_implP(implP)
    {
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LocalTransform& LocalTransform::GetIdentity ()
    {
    static const LocalTransform IDENTITY(new Impl(TransfoModel::GetIdentity(), TransfoModel::GetIdentity(), STATE_DUPLEX));
    return IDENTITY;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateFrom    (const TransfoModel& localToGlobal,
                                                const TransfoModel& globalToLocal)
    {
    return LocalTransform(new Impl(localToGlobal, globalToLocal, STATE_DUPLEX));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateFromToGlobal (const TransfoModel& localToGlobal)
    {
    return LocalTransform(new Impl(localToGlobal, TransfoModel::GetIdentity(), STATE_SIMPLEX_TO_GLOBAL));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateFromToLocal (const TransfoModel& globalToLocal)
    {
    return LocalTransform(new Impl(TransfoModel::GetIdentity(), globalToLocal, STATE_SIMPLEX_TO_LOCAL));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateDuplexFromToGlobal (const TransfoModel& localToGlobal)
    {
    SMStatus inverseOfStatus;
    TransfoModel globalToLocal(InverseOf(localToGlobal, inverseOfStatus));

    if (SMStatus::S_SUCCESS != inverseOfStatus)
        throw CustomException(L"Error computing inverse transformation model!");

    return LocalTransform(new Impl(localToGlobal, globalToLocal, STATE_DUPLEX));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateDuplexFromToLocal (const TransfoModel& globalToLocal)
    {
    SMStatus inverseOfStatus;
    TransfoModel localToGlobal(InverseOf(globalToLocal, inverseOfStatus));

    if (SMStatus::S_SUCCESS != inverseOfStatus)
        throw CustomException(L"Error computing inverse transformation model!");

    return LocalTransform(new Impl(localToGlobal, globalToLocal, STATE_DUPLEX));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateDuplexFromToGlobal (const TransfoModel& localToGlobal,
                                                         SMStatus&             status)
    {
    SMStatus inverseOfStatus;
    TransfoModel globalToLocal(InverseOf(localToGlobal, inverseOfStatus));

    status = (SMStatus::S_SUCCESS == inverseOfStatus) ? S_SUCCESS : S_ERROR;
    return LocalTransform(new Impl(localToGlobal, globalToLocal, STATE_DUPLEX));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform LocalTransform::CreateDuplexFromToLocal (const TransfoModel& globalToLocal,
                                                        SMStatus&             status)
    {
    SMStatus inverseOfStatus;
    TransfoModel localToGlobal(InverseOf(globalToLocal, inverseOfStatus));

    status = (SMStatus::S_SUCCESS == inverseOfStatus) ? S_SUCCESS : S_ERROR;
    return LocalTransform(new Impl(localToGlobal, globalToLocal, STATE_DUPLEX));
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform::~LocalTransform ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform::LocalTransform (const LocalTransform& rhs)
    :   m_implP(rhs.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform& LocalTransform::operator= (const LocalTransform& rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalTransform::IsIdentity () const
    {
    return m_implP->m_localToGlobal.IsIdentity() && m_implP->m_globalToLocal.IsIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalTransform::IsDuplex () const
    {
    return STATE_DUPLEX == m_implP->m_state;
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalTransform::Impl::IsEquivalent (const Impl& rhs) const
    {
    const State lhsState = m_state;
    const State rhsState = rhs.m_state;

    const State commonState = GetCommonState(lhsState, rhsState);
    switch (commonState)
        {
    case STATE_DUPLEX:
        return m_localToGlobal.IsEquivalent(rhs.m_localToGlobal) && 
               m_globalToLocal.IsEquivalent(rhs.m_globalToLocal);
    case STATE_SIMPLEX_TO_LOCAL:
        return m_globalToLocal.IsEquivalent(rhs.m_globalToLocal);
    case STATE_SIMPLEX_TO_GLOBAL:
        return m_localToGlobal.IsEquivalent(rhs.m_localToGlobal);
        }
    
    // No common state. We will need to compute inverse of one side to be able to effect the comparison.
    assert(STATE_NOTHING == commonState);


    const TransfoModel* localToGlobalSideP = 0;
    const TransfoModel* globalToLocalSideP = 0;

    if (STATE_SIMPLEX_TO_GLOBAL == lhsState)
        {
        assert(STATE_SIMPLEX_TO_LOCAL == rhsState);
        localToGlobalSideP = &m_localToGlobal;
        globalToLocalSideP = &rhs.m_globalToLocal;
        }
    else 
        {
        assert(STATE_SIMPLEX_TO_GLOBAL == rhsState);
        assert(STATE_SIMPLEX_TO_LOCAL == lhsState);

        localToGlobalSideP = &rhs.m_globalToLocal;
        globalToLocalSideP = &m_localToGlobal;
        }

    SMStatus inverseStatus;
    const TransfoModel globalToLocalSideInverse(InverseOf(*globalToLocalSideP, inverseStatus));
    
    return SMStatus::S_SUCCESS == inverseStatus &&
           localToGlobalSideP->IsEquivalent(globalToLocalSideInverse);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalTransform::IsEquivalent (const LocalTransform& rhs) const
    {
    return m_implP->IsEquivalent(*rhs.m_implP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalTransform::HasToGlobal () const
    {
    return m_implP->HasLocalToGlobal();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TransfoModel& LocalTransform::GetToGlobal () const
    {
    if (m_implP->HasLocalToGlobal())
        return m_implP->m_localToGlobal;

    SMStatus status = const_cast<LocalTransform&>(*this).ComputeDuplex();
    assert(S_SUCCESS == status);

    return m_implP->m_localToGlobal;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalTransform::HasToLocal () const
    {
    return m_implP->HasGlobalToLocal();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TransfoModel& LocalTransform::GetToLocal () const
    {
    if (m_implP->HasGlobalToLocal())
        return m_implP->m_globalToLocal;

    SMStatus status = const_cast<LocalTransform&>(*this).ComputeDuplex();
    assert(S_SUCCESS == status);
    
    return m_implP->m_globalToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus LocalTransform::ComputeDuplex()
    {
    if (STATE_DUPLEX == m_implP->m_state)
        return S_SUCCESS;


    Impl::UpdateForEdit(m_implP);

    SMStatus status;

    if (STATE_SIMPLEX_TO_GLOBAL == m_implP->m_state)
        m_implP->m_globalToLocal = InverseOf(m_implP->m_localToGlobal, status);
    else
        m_implP->m_localToGlobal = InverseOf(m_implP->m_globalToLocal, status);

    return (SMStatus::S_SUCCESS == status) ? S_SUCCESS : S_ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus LocalTransform::Append(const LocalTransform& rhs)
    {
    SMStatus status;
    LocalTransform result(Combine(*this, rhs, status));
    
    if (S_SUCCESS != status)
        return S_ERROR;

    using namespace std;
    swap(*this, result);
    return S_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
 void swap (LocalTransform& lhs,
            LocalTransform& rhs)
    {
    using namespace std;
    swap(lhs.m_implP, rhs.m_implP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* TDORAY: Use the TransfoModel::Combine version that returns a status
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform Combine     (const LocalTransform&       lhs,
                            const LocalTransform&       rhs,
                            SMStatus&     status)
    {
    status = SMStatus::S_SUCCESS;

    const State lhsState = lhs.m_implP->m_state;
    const State rhsState = rhs.m_implP->m_state;

    const State commonState = GetCommonState(lhsState, rhsState);
    switch (commonState)
        {
    case STATE_DUPLEX:
        return LocalTransform::CreateFrom(Combine(rhs.GetToGlobal(), lhs.GetToGlobal()), 
                                          Combine(lhs.GetToLocal(), rhs.GetToLocal()));
    case STATE_SIMPLEX_TO_LOCAL:
        return LocalTransform::CreateFromToLocal(Combine(lhs.GetToLocal(), rhs.GetToLocal()));
    case STATE_SIMPLEX_TO_GLOBAL:
        return LocalTransform::CreateFromToGlobal(Combine(rhs.GetToGlobal(), lhs.GetToGlobal()));
        }

    // No common state found. Will need to compute inverse of at least one side so that it becomes possible
    // to compute the required multiplication. We will favor local to global computation as it is what
    // is stored to WKT and is most commonly used (for the moment).
    assert(STATE_NOTHING == commonState);

    if (STATE_SIMPLEX_TO_GLOBAL == lhsState)
        {
        assert(STATE_SIMPLEX_TO_LOCAL == rhsState);
        
        SMStatus inverseStatus;
        TransfoModel rhsLocalToGlobal = InverseOf(rhs.GetToLocal(), inverseStatus);
        
        if (SMStatus::S_SUCCESS != inverseStatus)
            {
            status = SMStatus::S_ERROR;
            return LocalTransform::GetIdentity();
            }

        return LocalTransform::CreateFromToGlobal(Combine(rhsLocalToGlobal, lhs.GetToGlobal()));
        }
    else 
        {
        assert(STATE_SIMPLEX_TO_GLOBAL == rhsState);
        assert(STATE_SIMPLEX_TO_LOCAL == lhsState);

        SMStatus inverseStatus;
        TransfoModel lhsLocalToGlobal = InverseOf(lhs.GetToLocal(), inverseStatus);
        
        if (SMStatus::S_SUCCESS != inverseStatus)
            {
            status = SMStatus::S_ERROR;
            return LocalTransform::GetIdentity();
            }

        return LocalTransform::CreateFromToGlobal(Combine(rhs.GetToGlobal(), lhsLocalToGlobal));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransform Combine (const LocalTransform&   lhs,
                        const LocalTransform&   rhs)
    {
    SMStatus status;
    LocalTransform result(Combine(lhs, rhs, status));
    if (SMStatus::S_SUCCESS != status)
        throw CustomException(L"Error combining local transforms!");
    
    return result;
    }

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
