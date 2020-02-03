/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/OffscreenRendering/D3DProcessing.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

class D3DProcessing
{
public:

    D3DProcessing(ID3D10Device* pDevice);

    ~D3DProcessing();

    ID3D10Device* GetDevice() {return m_pDevice;}  

    HRESULT LoadEffectFile(wstring const& filename);

    HRESULT CreateTechnique();


    HRESULT CreateRenderTargetAndStagingResource();

    HRESULT CreateSourceTexture();
        
    HRESULT CreateTextureReceiver();
        
    HRESULT UpdateSourceTexture(UCHAR const* pixels);
       
    void  PostInitSetup();

    void Render(UINT backgroundColorARGB, D3DXMATRIX& matrix);

    HRESULT RenderToBuffer(UCHAR* pOutBuffer, UCHAR const* pSrcBuffer, UINT backgroundColorARGB, D3DXMATRIX& matrix);

    HRESULT ConvertToLUV(double* pTileOut, HFCPtr<HRABitmap> pTileIn);
       
private:
    ID3D10Device* m_pDevice;

    // fx file and technique
    ID3D10Effect*           m_pEffect;
    ID3D10EffectTechnique*  m_pRenderTileTechnique;
    ID3D10EffectTechnique*  m_pRenderTileTechniqueRGBtoLUV;
    ID3D10InputLayout*      m_pRenderTileLayout;
    ID3D10EffectShaderResourceVariable* m_pShaderSrcTexture;
    ID3D10EffectMatrixVariable* m_pPixelTransformation;

    // Render target associated objects.
    ID3D10Texture2D*        m_pRenderTarget;
    ID3D10RenderTargetView* m_pRenderTargetView;

    // A compatible resource with m_pRenderTargetView so we can read back from m_pRenderTarget
    ID3D10Texture2D*        m_pStagingResource;     

    // The source texture (tile pixels)
    ID3D10Texture2D*            m_pSrcTexture;
    ID3D10ShaderResourceView*   m_pSrcTextureShaderResView; // so we can map it in our shader.

    // The triangles(tile geometry) that will receive the src texture.
    ID3D10Buffer* m_pTexReceiver;

    // For LUV Processing
    ID3D10Texture2D*        m_pRenderTargetFloat;
    ID3D10RenderTargetView* m_pRenderTargetViewFloat;
    ID3D10Texture2D*        m_pStagingResourceFloat;     

};
