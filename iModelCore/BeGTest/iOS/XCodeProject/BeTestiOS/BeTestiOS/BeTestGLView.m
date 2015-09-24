#import "BeTestGLView.h"

@implementation BeTestGLView

@synthesize GLContext       = m_context;
@synthesize FrameBufferId   = m_frameBuffer;
@synthesize RenderBufferId  = m_renderBuffer;

- (id) initWithFrame: (CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setupLayer];
        [self setupContext];
        [self setupRenderBuffer];
        [self setupFrameBuffer];
    }
    return self;
}

- (void) dealloc
{
    m_context = nil;
}

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (void) setupLayer
{
    m_eaglLayer = (CAEAGLLayer*) self.layer;
    m_eaglLayer.opaque = TRUE;
    m_eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE],
                                      kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
}

- (void) setupContext
{
    m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (!m_context)
    {
        NSLog (@"Failed to initialize context");
        exit (1);
    }
    if (![EAGLContext setCurrentContext:m_context])
    {
        NSLog (@"Failed to set current context");
        exit (1);
    }
}

- (void) setupRenderBuffer
{
    glGenRenderbuffers(1, &m_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
    [m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:m_eaglLayer];
}

- (void) setupFrameBuffer
{
    glGenFramebuffers(1, &m_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (GL_FRAMEBUFFER_COMPLETE != status)
    {
        NSLog(@"Failed to setup frame buffer");
        exit (1);
    }
}

@end
