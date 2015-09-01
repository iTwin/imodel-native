//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSPssToken.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DLocalProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DWorld.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HVE2DShape.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class HPSPssToken : public HFCShareableObject<HPSPssToken>
    {
public:
    HPSPssToken() {}
    HPSPssToken(HPSPssToken const&) {}
    virtual ~HPSPssToken() {}
    virtual HPSPssToken& operator = (HPSPssToken const& rhs) {
        return *this;
        }

    friend std::ostream& operator << (std::ostream& os, const HPSPssToken& obj)
        {
        return obj.Write(os);
        }

    virtual std::ostream& Write(std::ostream& os) const = 0;

protected:

private:
    };


/*---------------------------------------------------------------------------------**//**
* UTILITY TOKEN
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSImageIdToken : public HPSPssToken
    {
public:
    PSSImageIdToken(HFCPtr<HPSPssToken> pToken, int32_t id) : HPSPssToken(), m_token(pToken), m_id(id) {}
    virtual ~PSSImageIdToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "r" << m_id << " = " << *m_token;
        return os;
        }

    virtual ostringstream& Write (ostringstream& os)
        {
        os << "r" << m_id;
        return os;
        }

protected:
private:
    HFCPtr<HPSPssToken> m_token;
    int32_t             m_id;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSSelectWorldToken : public HPSPssToken
    {
public:
    PSSSelectWorldToken(HGF2DWorldIdentificator id) : HPSPssToken(), m_id(id) {}
    virtual ~PSSSelectWorldToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "SELW(" << m_id << ")";
        return os;
        }

protected:
private:
    HGF2DWorldIdentificator m_id;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSMosaicToken : public HPSPssToken
    {
public:
    PSSMosaicToken() : HPSPssToken() {}
    virtual ~PSSMosaicToken() {}

    void AddToken(HFCPtr<PSSImageIdToken> pToken) {
        m_list.push_back(pToken);
        }


    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        ListOfTokens::const_iterator itr (m_list.begin());
        ostringstream stringStream;

        os << "m0 = MO(";
        while(itr != m_list.end())
            {
            (*itr)->Write(stringStream);
            if (*itr != m_list.back())
                stringStream << ", ";
            ++itr;
            }
        os << stringStream.str() << ")";
        return os;
        }

    virtual ostringstream& Write (ostringstream& os)
        {
        os << "m0";
        return os;
        }

protected:
private:
    typedef std::list<HFCPtr<PSSImageIdToken> > ListOfTokens;

    ListOfTokens m_list;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSPageToken : public HPSPssToken
    {
public:
    PSSPageToken(HFCPtr<PSSMosaicToken> pMosaicToken) : HPSPssToken(), m_token (pMosaicToken) {}
    virtual ~PSSPageToken() {}
    virtual std::ostream& Write(std::ostream& os) const
        {
        ostringstream stringStream;
        m_token->Write(stringStream);
        os << "p0 = " << stringStream.str() << endl;
        os << "PG(p0)";
        return os;
        }

protected:
private:
    HFCPtr<PSSMosaicToken> m_token;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     ChantalPoulin  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSImagePageToken : public HPSPssToken
    {
public:
    PSSImagePageToken(HFCPtr<PSSImageIdToken> pImageToken) : HPSPssToken(), m_token (pImageToken) {}

    virtual ~PSSImagePageToken() {}
    virtual std::ostream& Write(std::ostream& os) const
        {
        ostringstream stringStream;
        m_token->Write(stringStream);
        os << "PG(" << stringStream.str() << ")" << endl;
        return os;
        }

protected:
private:
    HFCPtr<PSSImageIdToken> m_token;
    };

/*---------------------------------------------------------------------------------**//**
* IMAGECONTEXT
* @bsimethod                                                    StephanePoulin  01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSIFCToken : public HPSPssToken
    {
public:
    PSSIFCToken(int32_t id) : m_id(id) {}
    virtual ~PSSIFCToken() {}

        // from PSSToken
    virtual std::ostream& Write(std::ostream &os) const
    {
            ostringstream tokenId;
            tokenId << "ImgCt" << m_id;
            os << tokenId.str() << " = IFC()";

            if (!m_layersOn.empty() || !m_layersOff.empty())
                os << endl;

            if (!m_layersOn.empty())
                {
                os << "SETLAYERON(" << tokenId.str() << ",";
                std::list<WString>::const_iterator itr (m_layersOn.begin());

                Utf8String utf8Str;
                BeStringUtilities::WCharToUtf8(utf8Str,(*itr).c_str());

                os << "\"" << utf8Str << "\"";
                ++itr;
                while (itr != m_layersOn.end())
                    {
                    BeStringUtilities::WCharToUtf8(utf8Str,(*itr).c_str());

                    os << ",\"" << utf8Str << "\"";
                    ++itr;
                    }
                os << ")";
                }

            if (!m_layersOff.empty())
                {
                os << "SETLAYEROFF(" << tokenId.str() << ",";
                std::list<WString>::const_iterator itr (m_layersOff.begin());

                Utf8String utf8Str;
                BeStringUtilities::WCharToUtf8(utf8Str,(*itr).c_str());

                os << "\"" << utf8Str << "\"";
                ++itr;
                while (itr != m_layersOff.end())
                    {
                    BeStringUtilities::WCharToUtf8(utf8Str,(*itr).c_str());

                    os << ",\"" << utf8Str << "\"";
                    ++itr;
                    }
                os << ")";
                }

            return os;
    }

    void SetLayerOn(WString const& layer)
    {
            m_layersOn.push_back(layer);
    }

    void SetLayerOff(WString const& layer)
    {
            m_layersOff.push_back(layer);
    }

    WString GetName() const
        {
        wostringstream tokenId;
        tokenId << L"ImgCt" << m_id;
        return tokenId.str().c_str();
        }

    private:
    int32_t            m_id;
    std::list<WString> m_layersOn;
    std::list<WString> m_layersOff;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSImageFileGeorefContextToken : public HPSPssToken
    {
    public:
        PSSImageFileGeorefContextToken(int32_t id,
                                       double defaultRatioToMeterForRaster, 
                                       double defaultRatioToMeterForSisterFile, 
                                       bool   useSisterFile, 
                                       bool   usePCSLinearUnit, 
                                       bool   useDefaultUnitForGeoModel,
                                       bool   interpretAsIntergraphUnit)
            {
            m_id                                = id;
            m_defaultRatioToMeterForRaster      = defaultRatioToMeterForRaster;
            m_defaultRatioToMeterForSisterFile  = defaultRatioToMeterForSisterFile;
            m_useSisterFile                     = useSisterFile;
            m_usePCSLinearUnit                  = usePCSLinearUnit;
            m_useDefaultUnitForGeoModel         = useDefaultUnitForGeoModel;
            m_interpretAsIntergraphUnit         = interpretAsIntergraphUnit;        
            }

        virtual ~PSSImageFileGeorefContextToken() {}

        virtual std::ostream& Write(std::ostream &os) const
            {
            WString name (GetName());

            Utf8String utf8Str;
            BeStringUtilities::WCharToUtf8(utf8Str, name.c_str());

            os << utf8Str;
            os << " = GEOR(";
            os << m_defaultRatioToMeterForRaster << ",";
            os << m_defaultRatioToMeterForSisterFile << ",";
            os << m_useSisterFile << ",";
            os << m_usePCSLinearUnit << ",";
            os << m_useDefaultUnitForGeoModel << ",";
            os << m_interpretAsIntergraphUnit;        
            os << ")";

            return os;
            }

        WString GetName() const
            {    
            wostringstream tokenId;
            tokenId << L"GeorCt" << m_id;
            return tokenId.str().c_str();            
            }

    private:
        int32_t m_id;
        double m_defaultRatioToMeterForRaster;
        double m_defaultRatioToMeterForSisterFile;
        bool   m_useSisterFile;
        bool   m_usePCSLinearUnit;
        bool   m_useDefaultUnitForGeoModel;
        bool   m_interpretAsIntergraphUnit;        
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSTokenList : public HPSPssToken
    {
public:
    PSSTokenList() {}
    virtual ~PSSTokenList() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        std::list<HFCPtr<HPSPssToken> >::const_iterator itr (m_list.begin());
        while (itr != m_list.end())
            {
            os << *(*itr);
            if (*itr != m_list.back())
                os << ",";
            ++itr;
            }
        return os;
        }

    void AddToken(HFCPtr<HPSPssToken> pToken) {
        m_list.push_back(pToken);
        }
    bool IsEmpty() const {
        return m_list.empty();
        }

protected:
private:
    std::list<HFCPtr<HPSPssToken> > m_list;
    };

/*---------------------------------------------------------------------------------**//**
* TOKEN
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSIMFToken : public HPSPssToken
    {
    public:
    PSSIMFToken(WString const& fileName, uint32_t pageNumber) : HPSPssToken(), m_fileName(fileName), m_pageNumber(pageNumber) {}
        virtual ~PSSIMFToken() {}

        // from PSSToken
    virtual std::ostream& Write(std::ostream &os) const
        {
        Utf8String utf8Str;
        BeStringUtilities::WCharToUtf8(utf8Str,m_fileName.c_str());

        os << "IMF(\"" << utf8Str << "\", " << m_pageNumber;

        if (m_context.GetPtr())
            {
            WString name (m_context->GetName());
            
            BeStringUtilities::WCharToUtf8(utf8Str,name.c_str());

            os << ", " << utf8Str;
            
            }

        if (m_georeferenceContext.GetPtr())
            {
            // Must have a PSSIFCToken.
            HASSERT(NULL != m_context.GetPtr());

            WString name (m_georeferenceContext->GetName());
            BeStringUtilities::WCharToUtf8(utf8Str, name.c_str());
            os << ", " << utf8Str;
            }

        os <<  ")";

        return os;
        }

        void SetContext(const HFCPtr<PSSIFCToken> pToken)
            {
            m_context = pToken;
            }

        void SetGeoreferenceContext(HFCPtr<PSSImageFileGeorefContextToken> pGeorefContext)
            {
            m_georeferenceContext = pGeorefContext;
            }
    protected:
    private:
    WString                                m_fileName;
    uint32_t                               m_pageNumber;
    HFCPtr<PSSIFCToken>                    m_context;
    HFCPtr<PSSImageFileGeorefContextToken> m_georeferenceContext;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSDescriptionToken : public HPSPssToken
    {
public:
    PSSDescriptionToken(WString const& description) : HPSPssToken(), m_description(description) {}
    virtual ~PSSDescriptionToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        Utf8String utf8Str;
        BeStringUtilities::WCharToUtf8(utf8Str,m_description.c_str());

        os << "; " << utf8Str;
        return os;
        }

protected:
private:
    WString m_description;
    };

/*---------------------------------------------------------------------------------**//**
* RENDERING TOKEN
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSGammaToken : public HPSPssToken
    {
public:
    PSSGammaToken(double value) : HPSPssToken(), m_gamma(value) {}
    virtual ~PSSGammaToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "GAMC(" << m_gamma << ")";
        return os;
        }

protected:
private:
    double m_gamma;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSBrightnessToken : public HPSPssToken
    {
public:
    PSSBrightnessToken(int32_t value) : HPSPssToken(), m_brightness(value) {}
    virtual ~PSSBrightnessToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "BR(" << m_brightness << ")";
        return os;
        }

protected:
private:
    int32_t m_brightness;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSContrastToken : public HPSPssToken
    {
public:
    PSSContrastToken(int32_t value) : HPSPssToken(), m_contrast(value) {}
    virtual ~PSSContrastToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "CT(" << m_contrast << ")";
        return os;
        }

protected:
private:
    int32_t m_contrast;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSTintToken : public HPSPssToken
    {
public:
    PSSTintToken(int32_t red, int32_t green, int32_t blue) : HPSPssToken(), m_red(red), m_green(green), m_blue(blue) {}
    virtual ~PSSTintToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "TINT(" << m_red << "," << m_green << "," << m_blue << ")";
        return os;
        }

protected:
private:
    int32_t m_red;
    int32_t m_green;
    int32_t m_blue;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSInvertToken : public HPSPssToken
    {
public:
    PSSInvertToken() : HPSPssToken() {}
    virtual ~PSSInvertToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "INV()";
        return os;
        }

protected:
private:
    };

/*---------------------------------------------------------------------------------**//**
* COLORIZED BINARY IMAGE
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSColorizedBinaryImage : public HPSPssToken
    {
public:
    PSSColorizedBinaryImage(HFCPtr<HPSPssToken> pToken, int32_t bgRed, int32_t bgGreen, int32_t bgBlue, int32_t fgRed, int32_t fgGreen, int32_t fgBlue)
        : HPSPssToken(), m_token(pToken), m_bgRed(bgRed), m_bgGreen(bgGreen), m_bgBlue(bgBlue), m_fgRed(fgRed), m_fgGreen(fgGreen), m_fgBlue(fgBlue) {}
    virtual ~PSSColorizedBinaryImage() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "COLBI(" << *m_token << "," << m_bgRed << "," << m_bgGreen << "," << m_bgBlue << "," << m_fgRed << "," << m_fgGreen << "," << m_fgBlue << ")";
        return os;
        }
protected:
private:
    HFCPtr<HPSPssToken> m_token;
    int32_t             m_bgRed;
    int32_t             m_bgGreen;
    int32_t             m_bgBlue;
    int32_t             m_fgRed;
    int32_t             m_fgGreen;
    int32_t             m_fgBlue;
    };

/*---------------------------------------------------------------------------------**//**
* Alpha Cube (ALC)
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSAlphaCubeToken : public HPSPssToken
    {
public:
    PSSAlphaCubeToken(HFCPtr<HPSPssToken> pRenderingToken, int32_t opacity) : HPSPssToken(), m_renderingToken(pRenderingToken), m_opacity(opacity) {}
    virtual ~PSSAlphaCubeToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "ALC(" << *m_renderingToken << "," << m_opacity << ")";
        return os;
        }

protected:
private:
    int32_t             m_opacity;
    HFCPtr<HPSPssToken> m_renderingToken;
    };

/*---------------------------------------------------------------------------------**//**
* Alpha Palette (ALP)
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSAlphaPaletteToken : public HPSPssToken
    {
public:
    PSSAlphaPaletteToken(int32_t opacity, uint32_t index) : HPSPssToken(), m_opacity(opacity), m_index(index) {}
    virtual ~PSSAlphaPaletteToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "ALP(" << m_opacity << ", " << m_index << ")";
        return os;
        }

protected:
private:
    int32_t m_opacity;
    uint32_t m_index;
    };

/*---------------------------------------------------------------------------------**//**
* RGB Cube (RGBCUBE)
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSRgbCubeToken : public HPSPssToken
    {
public:
    PSSRgbCubeToken(int32_t redMin, int32_t redMax, int32_t greenMin, int32_t greenMax, int32_t blueMin, int32_t blueMax)
        : HPSPssToken(), m_redMin(redMin), m_redMax(redMax), m_greenMin(greenMin), m_greenMax(greenMax), m_blueMin(blueMin), m_blueMax(blueMax) { }

    virtual ~PSSRgbCubeToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "RGBCUBE(" << m_redMin << ", " << m_redMax << ", " << m_greenMin << ", " << m_greenMax << ", " << m_blueMin << ", " << m_blueMax << ")";
        return os;
        }

protected:
private:
    int32_t m_redMin;
    int32_t m_redMax;
    int32_t m_greenMin;
    int32_t m_greenMax;
    int32_t m_blueMin;
    int32_t m_blueMax;
    };

/*---------------------------------------------------------------------------------**//**
* FILTERED IMAGE (FII)
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSFIIToken : public HPSPssToken
    {
public:
    PSSFIIToken(HFCPtr<HPSPssToken> pToken, HFCPtr<HPSPssToken> const pRenderingToken) : HPSPssToken(), m_token(pToken), m_renderingToken(pRenderingToken) {}
    virtual ~PSSFIIToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "FII(" << *m_token << "," << *m_renderingToken << ")";
        return os;
        }

protected:
private:
    HFCPtr<HPSPssToken> m_token;
    HFCPtr<HPSPssToken> m_renderingToken;
    };

/*---------------------------------------------------------------------------------**//**
* TRANSLUCENT IMAGE (TLI)
* @bsiclass                                                     StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSTLIToken : public HPSPssToken
    {
public:
    PSSTLIToken(HFCPtr<HPSPssToken> pToken, HFCPtr<HPSPssToken> const& pRenderingToken) : HPSPssToken(), m_token(pToken), m_renderingToken(pRenderingToken) {}
    virtual ~PSSTLIToken() {}

    // from PSSToken
    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "TLI(" << *m_token << "," << *m_renderingToken << ")";
        return os;
        }

protected:
private:
    HFCPtr<HPSPssToken> m_token;
    HFCPtr<HPSPssToken> m_renderingToken;
    };

/*---------------------------------------------------------------------------------**//**
* GEOREFERENCE TOKEN
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* AFFINE (AFF) & PROJECTIVE(PROJ)
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSMatrixToken : public HPSPssToken
    {
public:
    PSSMatrixToken(HFCMatrix<3, 3> const& matrix) : HPSPssToken(), m_matrix(matrix) {}
    virtual ~PSSMatrixToken() {}

    virtual std::ostream& Write(std::ostream& os) const
        {
        if(m_matrix[2][0] != 0.0 || m_matrix[2][1] != 0.0 || m_matrix[2][2] != 1.0)
            {
            os << "PROJ(";
            os.width(16);
            os.precision(16);
            fixed(os);
            os << m_matrix[0][0] << ", " << m_matrix[0][1] << ", " << m_matrix[0][2] << ", ";
            os << m_matrix[1][0] << ", " << m_matrix[1][1] << ", " << m_matrix[1][2] << ", ";
            os << m_matrix[2][0] << ", " << m_matrix[2][1] << ", " << m_matrix[2][2];
            os << ")";
            }
        else
            {
            os << "AFF(" ;
            os.width(16);
            os.precision(16);
            fixed(os);
            os << m_matrix[0][0] << ", " << m_matrix[0][1] << ", " << m_matrix[0][2] << ", ";
            os << m_matrix[1][0] << ", " << m_matrix[1][1] << ", " << m_matrix[1][2];
            os << ")";
            }

        return os;
        }

protected:
private:
    HFCMatrix<3, 3> m_matrix;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSLocalProjectiveGridToken : public HPSPssToken
    {
public:
    PSSLocalProjectiveGridToken(HFCPtr<HGF2DLocalProjectiveGrid> const& pModel) : HPSPssToken(), m_pModel(pModel) {}
    virtual ~PSSLocalProjectiveGridToken() {}

    virtual std::ostream& Write(std::ostream& os) const
        {
        // NEEDSWORK_V895_PSS
        HGF2DLiteExtent Extent;

        uint32_t rows;
        uint32_t columns;
        list<HFCPtr<HGF2DTransfoModel> > modelList;
        m_pModel->GetPSSParameters (Extent, columns, rows, modelList);

        os << "LOCALPROJECTIVEGRID(";

        os.width(16);
        os.precision(16);
        fixed(os);

        os << Extent.GetOrigin().GetX() << ", ";
        os << Extent.GetOrigin().GetY() << ", ";
        os << Extent.GetCorner().GetX() << ", ";
        os << Extent.GetCorner().GetY() << ", ";

        os << rows << ", ";
        os << columns << ", " << endl;

        list<HFCPtr<HGF2DTransfoModel> >::const_iterator itr(modelList.begin());
        const HFCPtr<HGF2DTransfoModel> pGlobalAffine = *itr;
        itr++;

        PSSMatrixToken globalAfineToken(pGlobalAffine->GetMatrix());
        os << globalAfineToken;

        while (itr != modelList.end())
            {
            os << "," << endl;
            PSSMatrixToken token((*itr)->GetMatrix());
            os << token;
            itr++;
            }

        // close the LOCALPROJECTIVEGRID statement
        os << ") ";
        return os;
        }

protected:
private:
    HFCPtr<HGF2DLocalProjectiveGrid> m_pModel;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  01/2007
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSTFIGeorefToken : public HPSPssToken
    {
public:
    PSSTFIGeorefToken(HFCPtr<PSSIMFToken> pImfToken, HFCPtr<HPSPssToken> pGeorefToken, HGF2DWorldIdentificator worldId) : HPSPssToken(), m_pImfToken(pImfToken), m_pGeorefToken(pGeorefToken), m_worldId(worldId) {}
    virtual ~PSSTFIGeorefToken() {}

    virtual std::ostream& Write(std::ostream& os) const
        {
        os << "TFI(" << *m_pImfToken << ", " << *m_pGeorefToken << " USING " << m_worldId << ")";
        return os;
        }

protected:
private:
    HFCPtr<PSSIMFToken>     m_pImfToken;
    HFCPtr<HPSPssToken>     m_pGeorefToken;
    HGF2DWorldIdentificator m_worldId;
    };

/*---------------------------------------------------------------------------------**//**
* CLIP TOKEN
+---------------+---------------+---------------+---------------+---------------+------*/
class PSSClipToken : public HPSPssToken
    {
public:
    PSSClipToken(HFCPtr<HPSPssToken> pToken, HFCPtr<HVEShape> pClipShape, HFCPtr<HGF2DWorld> pWorld)
        : HPSPssToken(), m_token(pToken), m_pClipShape(new HVEShape(*pClipShape)), m_pWorld(pWorld) { }
    virtual ~PSSClipToken() {}

    virtual std::ostream& Write(std::ostream& os) const
        {
        m_pClipShape->ChangeCoordSys(reinterpret_cast<const HFCPtr<HGF2DCoordSys>&>(m_pWorld));
        os << "SHAPEDIMAGE(" << *m_token << ", ";
        WriteToStream (os, *m_pClipShape->GetShapePtr());
        os << ")";
        return os;
        }

protected:
    virtual void WriteToStream(std::ostream& os, HVE2DShape const& shape) const
        {
        // A simple shape
        if (shape.IsComplex())
            {
            const HVE2DShape::ShapeList& rShapes = ((const HVE2DComplexShape&)shape).GetShapeList();
            if (rShapes.size() > 0)
                {
                os << "UNFS(";
                bool First = true;
                for (HVE2DShape::ShapeList::const_iterator Itr = rShapes.begin(); Itr != rShapes.end(); ++Itr)
                    {
                    if (!First)
                        os << ", ";
                    First = false;
                    WriteToStream(os, *(*Itr));
                    }
                os << ")";
                }
            }

        else if (shape.HasHoles())
            {
            // Get the base shape and the list of holes
            const HVE2DShape& rBaseShape = ((const HVE2DHoledShape&)shape).GetBaseShape();
            const HVE2DShape::HoleList& rShapes = ((const HVE2DHoledShape&)shape).GetHoleList();

            os << "HSH(";
            WriteToStream(os, rBaseShape);
            if (rShapes.size() > 0)
                {
                for (HVE2DShape::HoleList::const_iterator Itr = rShapes.begin(); Itr != rShapes.end(); ++Itr)
                    {
                    os << ", ";
                    WriteToStream(os, *(*Itr));
                    }
                }
            os << ")";
            }

        else
            {
            // drop the list of points in a collection
            HGF2DLocationCollection Points;
            shape.Drop(&Points, 1.0);

            // draw the polygon
            os << "POG(";
            bool First = true;
            for (HGF2DLocationCollection::const_iterator Itr = Points.begin(); Itr != Points.end(); ++Itr)
                {
                if (!First)
                    os << ", ";
                First = false;

                ostringstream Coordinate;
                Coordinate.width(16);
                Coordinate.precision(16);
                fixed(Coordinate);
                Coordinate << (*Itr).GetX() << ", " << (*Itr).GetY();
                os << Coordinate.str();
                }
            os  << " USING " << m_pWorld->GetIdentificator()<< ")";
            }
        }

private:
    HFCPtr<HPSPssToken>      m_token;
    mutable HFCPtr<HVEShape> m_pClipShape;
    HFCPtr<HGF2DWorld>       m_pWorld;
    };

END_IMAGEPP_NAMESPACE