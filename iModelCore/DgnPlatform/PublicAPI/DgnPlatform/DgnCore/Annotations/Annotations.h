//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/Annotations.h $ 
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

//! @defgroup Annotations Annotations
//! The "annotations" group contains APIs related to representing graphical markup in a project, typically in a drawing model, of physical geometry. For example, a TextAnnotation is a piece of text with an optional frame and leaders/arrows.

#include "AnnotationFrame.h"
#include "AnnotationFrameDraw.h"
#include "AnnotationFrameLayout.h"
#include "AnnotationFrameStyle.h"
#include "AnnotationLeader.h"
#include "AnnotationLeaderDraw.h"
#include "AnnotationLeaderLayout.h"
#include "AnnotationLeaderStyle.h"
#include "AnnotationTextBlock.h"
#include "AnnotationTextBlockDraw.h"
#include "AnnotationTextBlockLayout.h"
#include "AnnotationTextStyle.h"
#include "TextAnnotation.h"
#include "TextAnnotationDraw.h"
#include "TextAnnotationSeed.h"

//__PUBLISH_SECTION_END__

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Utility class to call a parameterless void callback when destructed; particularly useful with lambdas.
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct CallbackOnDestruct : public NonCopyableClass
{
    typedef std::function<void ()> T_Callback;

private:
    T_Callback m_callback;

    void Call() { if(m_callback) m_callback(); }

public:
    explicit CallbackOnDestruct(T_Callback const& callback) : m_callback(callback) { }
    ~CallbackOnDestruct() { Call(); }
    void CallThenCancel() { Call(); Cancel(); }
    void Cancel() { m_callback = NULL; }
};

END_BENTLEY_DGN_NAMESPACE
