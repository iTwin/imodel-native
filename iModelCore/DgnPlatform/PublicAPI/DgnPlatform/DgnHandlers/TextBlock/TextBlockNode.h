/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextBlockNode.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "TextAPICommon.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! In the TextBlock DOM, this is an abstract base class for components of the DOM, and is not to be used directly.
//! @note "Nominal" range refers to the glyph cell box, or the union of such boxes (depending on the type of node).
//! @note "Exact" range refers to the glyph black box, or the union of such boxes (depending on the type of node).
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct TextBlockNode
    {
    protected:  mutable DPoint3d    m_origin;
    protected:  mutable DRange3d    m_nominalRange;
    protected:  mutable DRange3d    m_exactRange;

    public:                                                     TextBlockNode                   ();
    public:                                                     TextBlockNode                   (TextBlockNodeCR);
    public:                         virtual                     ~TextBlockNode                  ();

    protected:                      virtual TextBlockNodeLevel  _GetUnitLevel                   () const = 0;
    protected:                      virtual DPoint3d            _GetOrigin                      () const;
    protected:                      virtual Transform           _GetTransform                   () const;

    protected:                      virtual DRange3d            _GetNominalRange                () const;
    protected:                      virtual DRange3d            _GetTransformedNominalRange     () const;
    protected:                      virtual double              _GetNominalWidth                () const;
    protected:                      virtual double              _GetNominalHeight               () const;

    protected:                      virtual DRange3d            _GetExactRange                  () const;
    protected:                      virtual DRange3d            _GetTransformedExactRange       () const;
    protected:                      virtual double              _GetExactWidth                  () const;
    protected:                      virtual double              _GetExactHeight                 () const;

    protected:                      virtual void                _Drop                           (TextBlockNodeArrayR);

    protected:                              bool                Equals                          (TextBlockNodeCR, TextBlockCompareOptionsCR) const;

    public:                                 TextBlockNodeLevel  GetUnitLevel                    () const;
    public:                                 DPoint3d            GetOrigin                       () const;
    public:     DGNPLATFORM_EXPORT          void                SetOrigin                       (DPoint3dCR);
    public:                                 Transform           GetTransform                    () const;

    public:     DGNPLATFORM_EXPORT          DRange3d            GetNominalRange                 () const;
    public:     DGNPLATFORM_EXPORT          DRange3d            ComputeTransformedNominalRange  () const;
    public:     DGNPLATFORM_EXPORT          double              GetNominalWidth                 () const;
    public:     DGNPLATFORM_EXPORT          double              GetNominalHeight                () const;

    public:     DGNPLATFORM_EXPORT          DRange3d            GetExactRange                   () const;
    public:     DGNPLATFORM_EXPORT          DRange3d            ComputeTransformedExactRange    () const;
    public:     DGNPLATFORM_EXPORT          double              GetExactWidth                   () const;
    public:     DGNPLATFORM_EXPORT          double              GetExactHeight                  () const;

    public:                                 DRange3d            ComputeJustificationRange       () const;
    public:     DGNPLATFORM_EXPORT          void                ExtendNominalRange              (DRange3dCR);
    public:     DGNPLATFORM_EXPORT          void                ExtendExactRange                (DRange3dCR);
    public:     DGNPLATFORM_EXPORT          void                Drop                            (TextBlockNodeArrayR);

    }; // TextBlockNode

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
