//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFGraphicObject.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFGraphicObject
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DExtent.h>
#include <Imagepp/all/h/HMDMetaDataContainerList.h>
#include <Imagepp/all/h/HPMAttributeSet.h>
#include <Imagepp/all/h/HGFMessages.h>

// The class declaration must be the last include file.
#include <Imagepp/all/h/HGFGraphicObject.h>

HPM_REGISTER_ABSTRACT_CLASS(HGFGraphicObject, HPMPersistentObject)


/** -----------------------------------------------------------------------------
    Default Constructor.
    -----------------------------------------------------------------------------
*/
HGFGraphicObject::HGFGraphicObject()
    {
    m_pSysCoord     = new HGF2DCoordSys ();
    }


/** -----------------------------------------------------------------------------
    Constructor.
    -----------------------------------------------------------------------------
*/
HGFGraphicObject::HGFGraphicObject(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)

    {
    m_pSysCoord     = pi_pCoordSys;
    }

/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HGFGraphicObject::HGFGraphicObject(const HGFGraphicObject& pi_rObj)
    {
    m_pSysCoord              = pi_rObj.m_pSysCoord;
#ifdef IPP_HPM_ATTRIBUTES_ON_HRA
    m_pAttributes            = pi_rObj.m_pAttributes;
#endif
    }

/** -----------------------------------------------------------------------------
    Destroyer
    -----------------------------------------------------------------------------
*/
HGFGraphicObject::~HGFGraphicObject()
    {
    // Nothing to do
    }

/** -----------------------------------------------------------------------------
    Assignment operator
    -----------------------------------------------------------------------------
*/
HGFGraphicObject& HGFGraphicObject::operator=(const HGFGraphicObject& pi_rObj)
    {
    m_pSysCoord              = pi_rObj.m_pSysCoord;
#ifdef IPP_HPM_ATTRIBUTES_ON_HRA
    m_pAttributes            = pi_rObj.m_pAttributes;
#endif
    return *this;
    }


/** -----------------------------------------------------------------------------
    Sets the coordinate system of the graphic object to the specified one.

    @param pi_rpCoordSys IN A pointer to new reference coordinate system
                            the object will from now on refer to.

    @see GetCoordSys()
    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
void HGFGraphicObject::SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    if (GetCoordSys() != pi_pCoordSys)
        {
        HFCPtr<HGF2DCoordSys> pOldCS(m_pSysCoord);

        m_pSysCoord = pi_pCoordSys;
        m_pSysCoord->SetModificationState();    // CoordSys Modified

        // Let the children do their job
        SetCoordSysImplementation(pOldCS);

        // Notify everyone of the change.
        Propagate(HGFGeometryChangedMsg());
        }
    }


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Artifact to be removed in the future. This method is unnecessary since
// SetCoordSys() can be overriden.
//-----------------------------------------------------------------------------
void HGFGraphicObject::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pOldCoordSys)
    {
    // Nothing to do here.
    }


/** -----------------------------------------------------------------------------
    Indicates if the graphic object is opaque or not. An object is opaque if it
    is closed, filled and has no transparency or translucency. By default, a
    graphic object is not opaque.

    @return true if object is opaque and false otherwise.
    -----------------------------------------------------------------------------
*/
bool HGFGraphicObject::IsOpaque() const
    {
    return false;
    }


//-----------------------------------------------------------------------------
// public
// PrintState
//-----------------------------------------------------------------------------
void HGFGraphicObject::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    po_rOutput

            << "HGFGraphicObject"
            << endl;

#endif
    }

#ifdef IPP_HPM_ATTRIBUTES_ON_HRA
/** -----------------------------------------------------------------------------
    Returns the graphic object attributes

    @return A constant reference to the attribute set.

    @see HPMAttributeSet
    -----------------------------------------------------------------------------
*/
HPMAttributeSet const& HGFGraphicObject::GetAttributes() const
    {
    if (m_pAttributes == 0)
        (const_cast<HGFGraphicObject*>(this))->m_pAttributes = new HPMAttributeSet();

    return *m_pAttributes;
    }


/** -----------------------------------------------------------------------------
    Sets the graphic object attributes

    @param pi_rAttributes IN Constant reference to attributes to assign to graphic object

    @see HPMAttributeSet
    -----------------------------------------------------------------------------
*/
void HGFGraphicObject::SetAttributes(const HPMAttributeSet& pi_rAttributes)
    {
    if (m_pAttributes == 0)
        m_pAttributes = new HPMAttributeSet();

    HPMAttributeSetIterator Itr(pi_rAttributes.begin());

    // Pass all received attributes
    while (Itr != pi_rAttributes.end())
        {
        // Add the attribute to our list
        m_pAttributes->Set((*Itr));

        Itr++;
        }

    //HChk MRx
    // Propagate();
    }


/** -----------------------------------------------------------------------------
    Sets the graphic object attributes

    @param pi_rpAttributes IN Pointer to attributes set the contains the new
                           attributes. Only the reference to attribute set is copied.

    @see HPMAttributeSet
    -----------------------------------------------------------------------------
*/
void HGFGraphicObject::SetAttributes(HFCPtr<HPMAttributeSet>& pi_rpAttributes)
    {
    HPRECONDITION(pi_rpAttributes != 0);

    m_pAttributes = pi_rpAttributes;

    //HChk STx
//    Propagate();
    }


/** -----------------------------------------------------------------------------
    This method locks the attributes and returns a reference to the internal graphic
    object attributes to be possibly modified. The attributes must be unlocked
    after modifications have been performed.

    @return A reference to the internal attribute set that can be modified.

    @see HPMAttributeSet
    @see UnlockAttributes()
    -----------------------------------------------------------------------------
*/
HPMAttributeSet& HGFGraphicObject::LockAttributes() const
    {
    if (m_pAttributes == 0)
        (const_cast<HGFGraphicObject*>(this))->m_pAttributes = new HPMAttributeSet();

    return const_cast<HPMAttributeSet&>(*m_pAttributes);
    }


/** -----------------------------------------------------------------------------
    This method unlocks the attributes that has been previously locked by a call
    to LockAttributes(). Any modification to attributes in no more possible.

    @see LockAttributes()
    -----------------------------------------------------------------------------
*/
void HGFGraphicObject::UnlockAttributes() const
    {
    //HChk MRx
//    Propagate();
    }
#endif

const HFCPtr<HGF2DCoordSys>& HGFGraphicObject::GetCoordSys () const
    {
    return (m_pSysCoord);
    }

void HGFGraphicObject::LinkTo(HMGMessageSender* pi_pSender, bool pi_ReceiveMessagesInCurrentThread) const 
    { 
    HPRECONDITION(pi_pSender != 0); 
    EnsureSinkIsConstructed(); 
    pi_pSender->Link(m_pSink, pi_ReceiveMessagesInCurrentThread ? HMGThread::GetCurrentThreadID() : 0); 
    }

