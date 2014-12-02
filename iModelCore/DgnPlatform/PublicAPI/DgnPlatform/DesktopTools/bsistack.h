/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/bsistack.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __bsiStackH__
#define __bsiStackH__

/*__BENTLEY_INTERNAL_ONLY__*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    03/97
+---------------+---------------+---------------+---------------+---------------+------*/
void
bsiStack_walk
(
void *pvException
);

#ifdef  __pagstrucH__
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    PhilipMcGraw    09/99
+---------------+---------------+---------------+---------------+---------------+------*/
void
bsiStack_walkRaw
(
unsigned long pc,
unsigned long sp,
unsigned long bp,
CallStackFrame *callStackFrameP,
int maxFrames
);
#endif
#endif
