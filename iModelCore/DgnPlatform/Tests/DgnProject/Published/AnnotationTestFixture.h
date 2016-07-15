//-------------------------------------------------------------------------------------- 
//     $Source: Tests/DgnProject/Published/AnnotationTestFixture.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include "DgnHandlersTests.h"
#include <DgnPlatform/Annotations/Annotations.h>

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTestFixture : public GenericDgnModelTestFixture
{
public:
    static AnnotationTextStylePtr   createAnnotationTextStyle(DgnDbR project , const char* styleName);
    static AnnotationTextRunPtr     createAnnotationTextRun(DgnDbR project , AnnotationTextStylePtr runStyle);
    static AnnotationParagraphPtr   createAnnotationParagraph(DgnDbR project , AnnotationTextStylePtr parStyle, AnnotationTextRunPtr textRun);
    static AnnotationTextBlockPtr   createAnnotationTextBlock(DgnDbR project , AnnotationTextStylePtr docStyle, AnnotationParagraphPtr par1);
    static AnnotationLeaderStylePtr createAnnotationLeaderStyle(DgnDbR project, const char* styleName);
    static AnnotationFrameStylePtr  createAnnotationFrameStyle(DgnDbR project, const char* styleName);
    static TextAnnotationSeedPtr    createAnnotationSeed(DgnDbR project, const char* seedName);
    
}; // AnnotationTestFixture
