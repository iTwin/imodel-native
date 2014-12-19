//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurface.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSSurface
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSSurface.h>

#include <Imagepp/all/h/HGSSurfaceImplementationFactory.h>
#include <Imagepp/all/h/HGSGraphicToolImplementationFactory.h>
#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSVoidSurface.h>
#include <Imagepp/all/h/HGSBlitter.h>
#include <Imagepp/all/h/HGSVoidSurface.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGSSurface::HGSSurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor,
                       const HGSToolbox*                    pi_pToolbox,
                       const HGSSurfaceCapabilities*        pi_pCapabilities)
    {
    HPRECONDITION(pi_rpDescriptor != 0);

    // test if we need to create immediately the implementation
    if (pi_pToolbox != 0 || pi_pCapabilities != 0)
        {
        // yes because we have requirements
        m_pImplementation =
            HGSSurfaceImplementationFactory::GetInstance()->Create(pi_rpDescriptor,
                                                                   pi_pToolbox,
                                                                   pi_pCapabilities);
        HPOSTCONDITION(m_pImplementation != 0);
        }
    else
        {
        // create a void surface, the real surface must be create later
        m_pImplementation = new HGSVoidSurface(pi_rpDescriptor);
        }
    }


//-----------------------------------------------------------------------------
// public
// Constructor
// DO NOT USE THIS CONSTRUCTOR, UNLESS IT'S ABSOLUTELY NECESSARY
//-----------------------------------------------------------------------------
HGSSurface::HGSSurface(HGSSurfaceImplementation* pi_pImplementation)
    {
    // YOU GIVE YOUR IMPLEMTATION WHEN CALLING THIS CONSTRUCTOR
    // THE IMPLEMENTATION WILL BE DESTOYED BY THE SURFACE ITSELF LATER

    HPRECONDITION(pi_pImplementation != 0);

    m_pImplementation   = pi_pImplementation;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSSurface::~HGSSurface()
    {
    }

//-----------------------------------------------------------------------------
// public
// RequestCompatibility
//-----------------------------------------------------------------------------
void HGSSurface::RequestCompatibility(const HGSToolbox*             pi_pToolbox,
                                      const HGSSurfaceCapabilities* pi_pCapabilities)
    {
    // test if there is already an implementation
    if (m_pImplementation->IsCompatibleWith(HGSVoidSurface::CLASS_ID))
        {
        // keep a copy of the current implementation
        HAutoPtr<HGSSurfaceImplementation> pOldImplementation(m_pImplementation.release());

        // if no, simply create  a new surface implementation
        m_pImplementation =
            HGSSurfaceImplementationFactory::
            GetInstance()->Create(pOldImplementation->GetSurfaceDescriptor(),
                                  pi_pToolbox,
                                  pi_pCapabilities);

        HPOSTCONDITION(m_pImplementation != 0);

        // the HGSVoidSurface is use when we have not allocate the implementation
        // we can transfert options instead of copied it
        const HGSSurfaceOptions& rOptions = pOldImplementation->GetOptions();
        HGSSurfaceOptions::const_iterator Itr(rOptions.begin());
        while (Itr != rOptions.end())
            {
            m_pImplementation->AddOption((*Itr).second);
            Itr++;
            }
        }
    else
        {
        // if yes, test if the current implementation is compatible
        if ((pi_pCapabilities != 0 &&
             !m_pImplementation->GetCreator()->GetCapabilities()->Supports(*pi_pCapabilities)) ||
            (pi_pToolbox != 0 &&
             !HGSGraphicToolImplementationFactory::GetInstance()->IsSurfaceCompatibleWithToolbox(m_pImplementation->GetClassID(), *pi_pToolbox)))

            {
            // if not, test if we can change the implementation

            // test if we have a memory surface descriptor
            if (m_pImplementation->GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID))
                {
                // keep a copy of the current implementation
                HAutoPtr<HGSSurfaceImplementation> pOldImplementation(m_pImplementation.release());

                // create  a new surface implementation
                m_pImplementation =
                    HGSSurfaceImplementationFactory::GetInstance()->Create(pOldImplementation->GetSurfaceDescriptor(),
                                                                           pi_pToolbox,
                                                                           pi_pCapabilities);

                HPOSTCONDITION(m_pImplementation != 0);

                // now, copy all options into the new impementation
                const HGSSurfaceOptions& rOptions = pOldImplementation->GetOptions();
                HGSSurfaceOptions::const_iterator Itr(rOptions.begin());
                while (Itr != rOptions.end())
                    {
                    m_pImplementation->AddOption((*Itr).second->Clone());
                    Itr++;
                    }
                }
            else
                {
                HASSERT(0);
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// protected
// GetImplementation
//-----------------------------------------------------------------------------
HGSSurfaceImplementation* HGSSurface::GetImplementation() const
    {
    // test if we have already allocated the implementation
    if (m_pImplementation->IsCompatibleWith(HGSVoidSurface::CLASS_ID))
        {
        HAutoPtr<HGSSurfaceImplementation> pOldImplementation(m_pImplementation.release());

        // no, create it
        // yes because we have requirements
        const_cast<HGSSurface*>(this)->m_pImplementation =
            HGSSurfaceImplementationFactory::GetInstance()->Create(pOldImplementation->GetSurfaceDescriptor(),
                                                                   0,
                                                                   0);

        HPOSTCONDITION(m_pImplementation != 0);

        const HGSSurfaceOptions& rOptions = pOldImplementation->GetOptions();
        HGSSurfaceOptions::const_iterator Itr(rOptions.begin());
        while (Itr != rOptions.end())
            {
            m_pImplementation->AddOption((*Itr).second);
            Itr++;
            }
        }

    return m_pImplementation;
    }

//-----------------------------------------------------------------------------
// public
// CreateCompatibleSurface
//-----------------------------------------------------------------------------
HGSSurface* HGSSurface::CreateCompatibleSurface(const HFCPtr<HGSSurfaceDescriptor>& pi_rpSurfaceDescriptor) const
    {
    HPRECONDITION(pi_rpSurfaceDescriptor != 0);

    // test if we have already allocated the implementation
    if (m_pImplementation->IsCompatibleWith(HGSVoidSurface::CLASS_ID))
        {
        HAutoPtr<HGSSurfaceImplementation> pOldImplementation(m_pImplementation.release());

        const_cast<HGSSurface*>(this)->m_pImplementation =
            HGSSurfaceImplementationFactory::GetInstance()->Create(pOldImplementation->GetSurfaceDescriptor(),
                                                                   0,
                                                                   0);
        HPOSTCONDITION(m_pImplementation != 0);

        const HGSSurfaceOptions& rOptions = pOldImplementation->GetOptions();
        HGSSurfaceOptions::const_iterator Itr(rOptions.begin());
        while (Itr != rOptions.end())
            {
            m_pImplementation->AddOption((*Itr).second);
            Itr++;
            }
        }

    return new HGSSurface(m_pImplementation->GetCreator()->Create(pi_rpSurfaceDescriptor));
    }

//-----------------------------------------------------------------------------
// public
// Update
//-----------------------------------------------------------------------------
void HGSSurface::Update()
    {
    // check if we deal already with the true surface implementation
    if (m_pOriginalImplementation != 0)
        {
        // if not, copy the temporay surface in the true surface

        HFCPtr<HGSSurface> pTemporarySurface(CreateCompatibleSurface(m_pImplementation->GetSurfaceDescriptor()));
        m_pImplementation = m_pOriginalImplementation.release();

        HFCPtr<HGSBlitter> pBlitter(new HGSBlitter());
        pBlitter->SetFor(this);
        pBlitter->BlitFrom(pTemporarySurface);
        }
    }
