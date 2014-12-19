//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSSurfaceImplementationFactory.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGSSurfaceImplementationFactory
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSSurfaceImplementationFactory.h>

#include <Imagepp/all/h/HGSSurfaceCapabilities.h>
#include <Imagepp/all/h/HGSToolbox.h>
#include <Imagepp/all/h/HGSGraphicToolImplementationFactory.h>

HFC_IMPLEMENT_SINGLETON(HGSSurfaceImplementationFactory)

//-----------------------------------------------------------------------------
// public
// Default Constructor.
//-----------------------------------------------------------------------------
HGSSurfaceImplementationFactory::HGSSurfaceImplementationFactory ()
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSSurfaceImplementationFactory::~HGSSurfaceImplementationFactory()
    {
    }

//-----------------------------------------------------------------------------
// public
// Register
//-----------------------------------------------------------------------------
HGSSurfaceImplementation*
HGSSurfaceImplementationFactory::Create(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor,
                                        const HGSToolbox*                   pi_pToolbox,
                                        const HGSSurfaceCapabilities*       pi_pCapabilities) const
    {
    HPRECONDITION(pi_rpDescriptor != 0);

#ifdef HVERIFYCONTRACT
    // get the required capabilities for the surface
    HAutoPtr<HGSSurfaceCapabilities> pRequiredCapabilities(pi_rpDescriptor->GetRequiredSurfaceCapabilities());

    // parse the list of creators and find an implementation supporting
    // all the parameters of the image

    // parse the list of attributes in the capabilities
    const HGSSurfaceImplementationCreator* pCreator = 0;
    uint32_t Priority = ULONG_MAX;
    Creators::const_iterator Itr;
    for (Itr = m_Creators.begin(); Itr != m_Creators.end(); Itr++)
        {
        // if we already have an appropriate creator, test if the priority of the
        // current surface implementation is better
        if(!(pCreator != 0 && (*Itr)->GetPriority() < Priority))
            {
            const HGSSurfaceCapabilities* pCapabilities = (*Itr)->GetCapabilities();
            bool AppropriateSurface = true;

            // test if the required surface capabilities are supported
            if(pCapabilities->Supports(*pRequiredCapabilities))
                {
                // test if the extra capabilities are supported if necessary
                if(pi_pCapabilities != 0 && !pCapabilities->Supports(*pi_pCapabilities))
                    AppropriateSurface = false;

                if(AppropriateSurface && pi_pToolbox != 0)
                    {
                    AppropriateSurface =
                        HGSGraphicToolImplementationFactory::GetInstance()->IsSurfaceCompatibleWithToolbox(
                            (*Itr)->GetSurfaceImplementationID(),
                            *pi_pToolbox);
                    }

                if(AppropriateSurface)
                    pCreator = (*Itr);
                }
            }
        }
    HASSERT(pCreator != 0);
#endif

    HGSSurfaceImplementation* pSurfaceImplementation = 0;
    pSurfaceImplementation = (*m_Creators.begin())->Create(pi_rpDescriptor);

    return pSurfaceImplementation;
    }

//-----------------------------------------------------------------------------
// public
// Register
//-----------------------------------------------------------------------------
void HGSSurfaceImplementationFactory::Register(const HGSSurfaceImplementationCreator* pi_pCreator)
    {
    // register the creator
    m_Creators.push_back(pi_pCreator);
    }

