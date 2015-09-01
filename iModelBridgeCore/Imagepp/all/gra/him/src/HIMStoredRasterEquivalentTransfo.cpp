//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMStoredRasterEquivalentTransfo.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMStoredRasterEquivalentTransfo.h>

#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRAImageView.h>


/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HIMStoredRasterEquivalentTransfo::HIMStoredRasterEquivalentTransfo(const HFCPtr<HRARaster>& pi_rpRaster)
    {
    // The Logical CS.
    m_pLogicalCoordSys = pi_rpRaster->GetCoordSys();

    HFCPtr<HRARaster>     pCurrentLayer(pi_rpRaster);
    HFCPtr<HGF2DCoordSys> pCurrentLogicalCS(m_pLogicalCoordSys);
    bool                 ReferencesEncountered = false;
    bool                 ReferencesEnded = false;

    while(CanDecapsulate(pCurrentLayer))
        {
        if (IsAReference(pCurrentLayer))
            {
            ReferencesEncountered = true;

            if (ReferencesEnded)
                break;  // Second set of references, invalid.
            }
        else
            {
            if (ReferencesEncountered && !ReferencesEnded)
                {
                m_pLogicalCoordSysUnderReferences = pCurrentLayer->GetCoordSys();

                pCurrentLogicalCS = pCurrentLayer->GetCoordSys();

                ReferencesEnded = true;
                }
            else
                {
                if (pCurrentLayer->GetCoordSys() != pCurrentLogicalCS)
                    break;  // Keep same CS within non-reference layers.
                }
            }

        pCurrentLayer = Decapsulate(pCurrentLayer);
        }

    if (IsAStoredRaster(pCurrentLayer))
        {
        m_EquivalentIsComputed = true;
        m_UsesLogicalTransformations = ReferencesEncountered;

        if (m_pLogicalCoordSysUnderReferences == 0)
            m_pLogicalCoordSysUnderReferences = pCurrentLayer->GetCoordSys();

        m_pPhysicalCoordSys = ((HFCPtr<HRAStoredRaster>&)pCurrentLayer)->GetPhysicalCoordSys();

        m_pEquivalentTransfoModel = m_pPhysicalCoordSys->GetTransfoModelTo(m_pLogicalCoordSysUnderReferences);
        }
    else
        {
        m_EquivalentIsComputed = false;

        // Arbitrary value.
        m_UsesLogicalTransformations = false;
        }
    }


/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HIMStoredRasterEquivalentTransfo::~HIMStoredRasterEquivalentTransfo()
    {
    }


/** -----------------------------------------------------------------------------
    Check if we can decapsulate the specified raster. Currently, HRAImageView
    and HRAReferenceToRaster are supported.
    -----------------------------------------------------------------------------
*/
bool HIMStoredRasterEquivalentTransfo::CanDecapsulate(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    return (pi_rpRaster->IsCompatibleWith(HRAImageView::CLASS_ID) ||
            pi_rpRaster->IsCompatibleWith(HRAReferenceToRaster::CLASS_ID));
    }


/** -----------------------------------------------------------------------------
    Decapsulate the specified Raster : obtain its source raster. CanDecapsulate
    must be true for this method to work.
    -----------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMStoredRasterEquivalentTransfo::Decapsulate(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    HASSERT(CanDecapsulate(pi_rpRaster));

    if (pi_rpRaster->IsCompatibleWith(HRAImageView::CLASS_ID))
        return ((HFCPtr<HRAImageView>&)pi_rpRaster)->GetSource();

    if (pi_rpRaster->IsCompatibleWith(HRAReferenceToRaster::CLASS_ID))
        return ((HFCPtr<HRAReferenceToRaster>&)pi_rpRaster)->GetSource();

    return 0;
    }


/** -----------------------------------------------------------------------------
    Check if the specified raster is an HRAReferenceToRaster.
    -----------------------------------------------------------------------------
*/
bool HIMStoredRasterEquivalentTransfo::IsAReference(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    return pi_rpRaster->IsCompatibleWith(HRAReferenceToRaster::CLASS_ID);
    }


/** -----------------------------------------------------------------------------
    Check if the specified raster is an HRAStoredRaster
    -----------------------------------------------------------------------------
*/
bool HIMStoredRasterEquivalentTransfo::IsAStoredRaster(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    return pi_rpRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID);
    }


/** -----------------------------------------------------------------------------
    Check if we can compute the equivalent model. Other methods can be used only
    if this method returns true.
    -----------------------------------------------------------------------------
*/
bool HIMStoredRasterEquivalentTransfo::EquivalentTransfoCanBeComputed() const
    {
    return m_EquivalentIsComputed;
    }


/** -----------------------------------------------------------------------------
    Check if HRAReferenceToRaster instances are present. If there are none, the
    logical and physical coordinate systems can be extracted and used as if the
    raster was a derivative of the HRAStoredRaster class.
    -----------------------------------------------------------------------------
*/
bool HIMStoredRasterEquivalentTransfo::UsesLogicalTransformations() const
    {
    HASSERT(EquivalentTransfoCanBeComputed());

    return m_UsesLogicalTransformations;
    }


/** -----------------------------------------------------------------------------
    Retrieve the equivalent transformation model, as if the input raster was
    a simple HRAStoredRaster derivative.
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DTransfoModel> HIMStoredRasterEquivalentTransfo::GetEquivalentTransfoModel() const
    {
    HASSERT(EquivalentTransfoCanBeComputed());

    return m_pEquivalentTransfoModel;
    }


/** -----------------------------------------------------------------------------
    Retrieve the logical coordinate system.
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DCoordSys> HIMStoredRasterEquivalentTransfo::GetLogicalCoordSys() const
    {
    HASSERT(EquivalentTransfoCanBeComputed());

    return m_pLogicalCoordSys;
    }


/** -----------------------------------------------------------------------------
    Retrieve the physical coordinate system.
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DCoordSys> HIMStoredRasterEquivalentTransfo::GetPhysicalCoordSys() const
    {
    HASSERT(EquivalentTransfoCanBeComputed());

    return m_pPhysicalCoordSys;
    }


/** -----------------------------------------------------------------------------
    Take a shape represented in the logical system and transform it to map
    in the physical system.
    -----------------------------------------------------------------------------
*/
void HIMStoredRasterEquivalentTransfo::TransformLogicalShapeIntoPhysical(HVEShape& pio_rShape) const
    {
    HASSERT(EquivalentTransfoCanBeComputed());

    if (m_UsesLogicalTransformations)
        {
        // Undo job of the references
        pio_rShape.ChangeCoordSys(m_pLogicalCoordSys);
        pio_rShape.SetCoordSys(m_pLogicalCoordSysUnderReferences);
        pio_rShape.ChangeCoordSys(m_pPhysicalCoordSys);
        }
    else
        {
        // Simple transform, no reference
        pio_rShape.ChangeCoordSys(m_pPhysicalCoordSys);
        }
    }


/** -----------------------------------------------------------------------------
    Take a shape represented in the physical system and transform it to map
    in the logical system.
    -----------------------------------------------------------------------------
*/
void HIMStoredRasterEquivalentTransfo::TransformPhysicalShapeIntoLogical(HVEShape& pio_rShape) const
    {
    HASSERT(EquivalentTransfoCanBeComputed());

    if (m_UsesLogicalTransformations)
        {
        // Do the same job as the references
        pio_rShape.ChangeCoordSys(m_pLogicalCoordSysUnderReferences);
        pio_rShape.SetCoordSys(m_pLogicalCoordSys);
        }
    else
        {
        // Simple transform, no reference
        pio_rShape.ChangeCoordSys(m_pLogicalCoordSys);
        }
    }

