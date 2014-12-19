//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSGraphicToolImplementationFactory.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGSGraphicToolImplementationFactory
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGSGraphicToolImplementationFactory.h>

#include <Imagepp/all/h/HGSGraphicToolAttributes.h>
#include <Imagepp/all/h/HGSSurfaceImplementation.h>
#include <Imagepp/all/h/HGSGraphicToolImplementationCreator.h>
#include <Imagepp/all/h/HGSToolBox.h>

HFC_IMPLEMENT_SINGLETON(HGSGraphicToolImplementationFactory)

//-----------------------------------------------------------------------------
// public
// Default Constructor.
//-----------------------------------------------------------------------------
HGSGraphicToolImplementationFactory::HGSGraphicToolImplementationFactory ()
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HGSGraphicToolImplementationFactory::~HGSGraphicToolImplementationFactory()
    {
    }

//-----------------------------------------------------------------------------
// public
// Register
//-----------------------------------------------------------------------------
HGSGraphicToolImplementation* HGSGraphicToolImplementationFactory::Create(
    HCLASS_ID                       pi_GraphicToolID,
    const HGSGraphicToolAttributes* pi_pAttributes,
    HGSSurfaceImplementation*       pi_pSurfaceImplementation) const
    {
    HPRECONDITION(pi_pAttributes != 0);
    HPRECONDITION(pi_pSurfaceImplementation != 0);

    HAutoPtr<HGSGraphicToolCapabilities> pRequiredCapabilities(pi_pAttributes->GetRequiredCapabilities());
    const HGSGraphicToolImplementationCreator* pCreator =
        FindCreator(pi_GraphicToolID,
                    pi_pSurfaceImplementation->GetClassID(),
                    *pRequiredCapabilities);

    HGSGraphicToolImplementation* pGraphicToolImplementation = 0;
    if (pCreator != 0)
        pGraphicToolImplementation = pCreator->Create(pi_pAttributes, pi_pSurfaceImplementation);

    return pGraphicToolImplementation;
    }

//-----------------------------------------------------------------------------
// public
// FindCreator
//-----------------------------------------------------------------------------
const HGSGraphicToolImplementationCreator* HGSGraphicToolImplementationFactory
::FindCreator(HCLASS_ID                         pi_GraphicToolID,
              HCLASS_ID                         pi_SurfaceImplementationID,
              const HGSGraphicToolCapabilities& pi_rCapabilities) const
    {
    const HGSGraphicToolImplementationCreator* pCreator = 0;

    // parse the list of tool creators
    Creators::const_iterator Itr;
    for (Itr = m_Creators.begin(); Itr != m_Creators.end(); Itr++)
        {
        // test if we have the same tool, if it is for the same surface and if it has the same attribute
        if ((*Itr)->GetGraphicToolID() == pi_GraphicToolID &&
            (*Itr)->GetSurfaceImplementationID() == pi_SurfaceImplementationID &&
            (*Itr)->GetCapabilities()->Supports(pi_rCapabilities))
            pCreator = (*Itr);
        }

    return pCreator;
    }

//-----------------------------------------------------------------------------
// public
// FindCreator
//-----------------------------------------------------------------------------
bool HGSGraphicToolImplementationFactory::IsSurfaceCompatibleWithToolbox(
    HCLASS_ID           pi_SurfaceImplementationID,
    const HGSToolbox&   pi_rToolbox) const
    {
    bool AppropriateSurface = true;

    // for each element of the toolbox, test if we have the same tool
    // in the implementation
    uint32_t Count = pi_rToolbox.CountTools();
    for (uint32_t ToolIndex = 0; ToolIndex < Count && AppropriateSurface; ToolIndex++)
        {
        HGSGraphicTool* pTool = pi_rToolbox.GetTool(ToolIndex);

        HAutoPtr<HGSGraphicToolCapabilities> pToolRequiredCapabilities(pTool->GetAttributes().GetRequiredCapabilities());

        if (FindCreator(pTool->GetType(),
                        pi_SurfaceImplementationID,
                        *pToolRequiredCapabilities) == 0)
            AppropriateSurface = false;
        }

    return AppropriateSurface;
    }
