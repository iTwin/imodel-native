//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSEditor.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSEditor
//-----------------------------------------------------------------------------
// General class for Stretchers.
//-----------------------------------------------------------------------------

#include "HGSEditorImplementation.h"
#include "HCDPacket.h"

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
inline HGSEditor::HGSEditor()
    : HGSGraphicTool()
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HGSEditor::~HGSEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetType
//-----------------------------------------------------------------------------
inline HCLASS_ID HGSEditor::GetType() const
    {
    return HGSEditorImplementation::CLASS_ID;
    }

//-----------------------------------------------------------------------------
// public
// GetFirstRun
//-----------------------------------------------------------------------------
inline void* HGSEditor::GetFirstRun(HUINTX* po_pStartPosX,
                                    HUINTX* po_pStartPosY,
                                    size_t* po_pPixelCount,
                                    void*   pi_pTransaction) const
    {
    HPRECONDITION(GetImplementation() != 0);

    return ((HGSEditorImplementation*)GetImplementation())->GetFirstRun(po_pStartPosX,
                                                                        po_pStartPosY,
                                                                        po_pPixelCount,
                                                                        pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// GetNextRun
//-----------------------------------------------------------------------------
inline void* HGSEditor::GetNextRun(HUINTX*  po_pStartPosX,
                                   HUINTX*  po_pStartPosY,
                                   size_t*  po_pPixelCount,
                                   void*    pi_pTransaction) const

    {
    HPRECONDITION(GetImplementation() != 0);

    return ((HGSEditorImplementation*)GetImplementation())->GetNextRun(po_pStartPosX,
                                                                       po_pStartPosY,
                                                                       po_pPixelCount,
                                                                       pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// GetRun
//-----------------------------------------------------------------------------
inline void* HGSEditor::GetRun(HUINTX   pi_PosX,
                               HUINTX   pi_PosY,
                               size_t   pi_PixelCount,
                               void*    pi_pTransaction) const
    {
    HPRECONDITION(GetImplementation() != 0);

    return ((HGSEditorImplementation*)GetImplementation())->GetRun(pi_PosX,
                                                                   pi_PosY,
                                                                   pi_PixelCount,
                                                                   pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// SetRun
//-----------------------------------------------------------------------------
inline void HGSEditor::SetRun(HUINTX      pi_PosX,
                              HUINTX      pi_PosY,
                              size_t      pi_PixelCount,
                              const void* pi_pRun,
                              void*       pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->SetRun(pi_PosX,
                                                            pi_PosY,
                                                            pi_PixelCount,
                                                            pi_pRun,
                                                            pi_pTransaction);
    }


//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
inline void HGSEditor::GetPixels(const HUINTX*  pi_pPositionsX,
                                 const HUINTX*  pi_pPositionsY,
                                 size_t         pi_PixelCount,
                                 void*          po_pBuffer) const
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->GetPixels(pi_pPositionsX,
                                                               pi_pPositionsY,
                                                               pi_PixelCount,
                                                               po_pBuffer);
    }

//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
inline void HGSEditor::GetPixels(HUINTX     pi_StartPosX,
                                 HUINTX     pi_StartPosY,
                                 HSINTX     pi_DeltaX,
                                 HSINTX     pi_DeltaY,
                                 size_t     pi_PixelCount,
                                 void*      po_pBuffer) const
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->GetPixels(pi_StartPosX,
                                                               pi_StartPosY,
                                                               pi_DeltaX,
                                                               pi_DeltaY,
                                                               pi_PixelCount,
                                                               po_pBuffer);
    }

//-----------------------------------------------------------------------------
// public
// GetFirstPixel
//-----------------------------------------------------------------------------
inline void* HGSEditor::GetFirstPixel(HUINTX*   po_pPosX,
                                      HUINTX*   po_pPosY) const
    {
    HPRECONDITION(GetImplementation() != 0);

    return ((HGSEditorImplementation*)GetImplementation())->GetFirstPixel(po_pPosX,
                                                                          po_pPosY);
    }

//-----------------------------------------------------------------------------
// public
// GetNextPixel
//-----------------------------------------------------------------------------
inline void* HGSEditor::GetNextPixel(HUINTX*    po_pPosX,
                                     HUINTX*    po_pPosY) const
    {
    HPRECONDITION(GetImplementation() != 0);

    return ((HGSEditorImplementation*)GetImplementation())->GetNextPixel(po_pPosX,
                                                                         po_pPosY);
    }

//-----------------------------------------------------------------------------
// public
// GetPixel
//-----------------------------------------------------------------------------
inline void* HGSEditor::GetPixel(HUINTX     pi_PosX,
                                 HUINTX     pi_PosY) const
    {
    HPRECONDITION(GetImplementation() != 0);

    return ((HGSEditorImplementation*)GetImplementation())->GetPixel(pi_PosX,
                                                                     pi_PosY);
    }

//-----------------------------------------------------------------------------
// public
// SetPixel
//-----------------------------------------------------------------------------
inline void HGSEditor::SetPixel(HUINTX          pi_PosX,
                                HUINTX          pi_PosY,
                                const void*     pi_pValue)
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->SetPixel(pi_PosX,
                                                              pi_PosY,
                                                              pi_pValue);
    }

//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
inline void HGSEditor::Clear(const void* pi_pValue,
                             void*       pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->Clear(pi_pValue, pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// ClearRun
//-----------------------------------------------------------------------------
inline void HGSEditor::ClearRun(HUINTX      pi_StartPosX,
                                HUINTX      pi_StartPosY,
                                size_t      pi_PixelCount,
                                const void* pi_pValue,
                                void*       pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->ClearRun(pi_StartPosX,
                                                              pi_StartPosY,
                                                              pi_PixelCount,
                                                              pi_pValue,
                                                              pi_pTransaction);
    }


//-----------------------------------------------------------------------------
// public
// MergeRun
//-----------------------------------------------------------------------------
inline void HGSEditor::MergeRuns(HUINTX         pi_StartPosX,
                                 HUINTX         pi_StartPosY,
                                 size_t         pi_Width,
                                 size_t         pi_Height,
                                 const void*    pi_pRun,
                                 void*          pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);

    ((HGSEditorImplementation*)GetImplementation())->MergeRuns(pi_StartPosX,
                                                               pi_StartPosY,
                                                               pi_Width,
                                                               pi_Height,
                                                               pi_pRun,
                                                               pi_pTransaction);

    }




