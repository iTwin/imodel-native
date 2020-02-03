/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/DirectComputeProcessing.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
class DirectComputeProcessing
    {
    public:
        DirectComputeProcessing(void);
        ~DirectComputeProcessing(void);

    bool Init();

    HRESULT RenderToBuffer(UCHAR* pOutBuffer, UCHAR const* pSrcBuffer, UINT backgroundColorARGB, D3DXMATRIX& matrix);
    };

