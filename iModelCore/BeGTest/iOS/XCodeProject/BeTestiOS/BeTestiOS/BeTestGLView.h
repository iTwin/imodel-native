/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#import <UIKit/UIKit.h>

@interface BeTestGLView : UIView {
    CAEAGLLayer*    m_eaglLayer;
    EAGLContext*    m_context;
    GLuint          m_frameBuffer;
    GLuint          m_renderBuffer;
}

@property (readonly) EAGLContext*   GLContext;
@property (readonly) GLuint         FrameBufferId;
@property (readonly) GLuint         RenderBufferId;

@end
