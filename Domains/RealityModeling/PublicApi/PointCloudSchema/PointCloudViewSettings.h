/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudViewSettings.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------+
|   NOTE: This file was moved from $(SrcRoot)MstnPlatform\PPModules\PointCloud\Component\PointCloudHandler\PointCloudViewSettings.h
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/PointCloudColorDef.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

#define CLASSIFCATION_COUNT    (32)
#define CLASSIFCATION_BIT_MASK (0x1F)
#define CLASSIFCATION_MAXDISPLAYCOUNT 13

//&&ep get rid of LasClassificationInfo (use PointCloudClassificationSettings instead)

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct LasClassificationInfo
    {
    public:
        /*---------------------------------------------------------------------------------**//**
        * This ctor leaves the object unitialized.
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        LasClassificationInfo()
            {
            }

        /*---------------------------------------------------------------------------------**//**
        * visibleState contains the visibility state of the 32 classification categories.
        * classificationColor is a pointer to an array of 32 ColorDef
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        LasClassificationInfo(uint32_t visibleState, BePointCloud::PointCloudColorDefP classificationColor)
            {
            m_visibleState = visibleState;
            m_blendColor = false;
            m_useBasecolor= false;
            memcpy(m_classificationColor, classificationColor, CLASSIFCATION_COUNT * sizeof(BePointCloud::PointCloudColorDef));
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        ~LasClassificationInfo()
            {
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GetClassificationStates() const 
            { 
            return m_visibleState; 
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        unsigned char* GetClassificationStatesAdv() const 
            { 
            return (unsigned char*)&m_visibleStateAdv[0]; 
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationStates(uint32_t states) 
            { 
            m_visibleState = states; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie  08/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool GetClassificationState(unsigned char idx) const    
            { 
            int arrayPosition = idx / 8;

            int positionBitField = idx % 8;

            return TO_BOOL(m_visibleStateAdv[arrayPosition] & 0x00000001 << positionBitField); 

            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie  08/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationState(unsigned char idx, bool state)    
            { 
            int arrayPosition = idx / 8;

            int positionBitField = idx % 8;



            if (state)

                m_visibleStateAdv[arrayPosition] |= (0x00000001 << positionBitField); 

            else

                m_visibleStateAdv[arrayPosition] &= ~(0x00000001 << positionBitField);

            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        BePointCloud::PointCloudColorDefCP GetClassificationColors() const 
            {
            return m_classificationColor; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationColors(BePointCloud::PointCloudColorDefCP classificationColors) 
            {
            memcpy(m_classificationColor, classificationColors, CLASSIFCATION_COUNT * sizeof(BePointCloud::PointCloudColorDef));
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationVisibleStatesAdv(unsigned char const* advState) 
            { 
            memcpy(m_visibleStateAdv, advState, 32 * sizeof(unsigned char));
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        BePointCloud::PointCloudColorDefCR GetClassificationColor(unsigned char idx) const 
            {
            return m_classificationColor[idx]; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationColor(unsigned char idx, BePointCloud::PointCloudColorDef& color)
            {
            m_classificationColor[idx] = color; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool const GetBlendColor() const 
            {
            return m_blendColor;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetBlendColor(bool use)
            {
            m_blendColor = use;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool const GetUseBaseColor() const 
            {
            return m_useBasecolor;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUseBaseColor(bool use)
            {
            m_useBasecolor = use;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool GetClassActiveState(unsigned char idx) const    
            { 
            int arrayPosition = idx / 8;
            int positionBitField = idx % 8;
            return TO_BOOL(m_activeStateAdv[arrayPosition] & 0x00000001 << positionBitField); 
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassActiveState(unsigned char idx, bool state)    
            { 
            int arrayPosition = idx / 8;
            int positionBitField = idx % 8;

            if (state)
                m_activeStateAdv[arrayPosition] |= (0x00000001 << positionBitField); 
            else
                m_activeStateAdv[arrayPosition] &= ~(0x00000001 << positionBitField);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUnclassState(bool use)
            {
            m_unclassVisible = use;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool GetUnclassState() const 
            {
            return m_unclassVisible;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        BePointCloud::PointCloudColorDef GetUnclassColor() const 
            {
            return m_unclassColor; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUnclassColor(BePointCloud::PointCloudColorDef& color)
            {
            m_unclassColor = color; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void InitDefault()
            {
            m_blendColor = false;
            m_useBasecolor = false;
            m_visibleState = 0xFFFFFFFF;  // sets all to visible

            m_classificationColor[0].SetRed(255);   m_classificationColor[0].SetGreen(255); m_classificationColor[0].SetBlue(0);    //    eCreated = 0,
            m_classificationColor[1].SetRed(255);   m_classificationColor[1].SetGreen(255); m_classificationColor[1].SetBlue(255);  //    eUnclassified,
            m_classificationColor[2].SetRed(255);   m_classificationColor[2].SetGreen(0);   m_classificationColor[2].SetBlue(0);    //    eGround,
            m_classificationColor[3].SetRed(0);     m_classificationColor[3].SetGreen(200); m_classificationColor[3].SetBlue(50);   //    eLowVegetation,
            m_classificationColor[4].SetRed(0);     m_classificationColor[4].SetGreen(200); m_classificationColor[4].SetBlue(100);  //    eMediumVegetation,
            m_classificationColor[5].SetRed(0);     m_classificationColor[5].SetGreen(200); m_classificationColor[5].SetBlue(200);  //    eHighVegetation,
            m_classificationColor[6].SetRed(150);   m_classificationColor[6].SetGreen(150); m_classificationColor[6].SetBlue(150);  //    eBuilding,
            m_classificationColor[7].SetRed(0);     m_classificationColor[7].SetGreen(50);  m_classificationColor[7].SetBlue(100);  //    eLowPoint,
            m_classificationColor[8].SetRed(255);   m_classificationColor[8].SetGreen(255); m_classificationColor[8].SetBlue(255);  //    eModelKeyPoint,
            m_classificationColor[9].SetRed(0);     m_classificationColor[9].SetGreen(0);   m_classificationColor[9].SetBlue(255);  //    eWater = 9,
            m_classificationColor[10].SetRed(0);    m_classificationColor[10].SetGreen(0);  m_classificationColor[10].SetBlue(0);   // = 10 // reserved for ASPRS Definition
            m_classificationColor[11].SetRed(0);    m_classificationColor[11].SetGreen(0);  m_classificationColor[11].SetBlue(0);   // = 11 // reserved for ASPRS Definition
            m_classificationColor[12].SetRed(0);    m_classificationColor[12].SetGreen(0);  m_classificationColor[12].SetBlue(0);   //    eOverlapPoints = 12

            // init classification color vector
            memset (&m_classificationColor[13], 255, (CLASSIFCATION_COUNT - 13) * sizeof(BePointCloud::PointCloudColorDef)); // all white
            memset (&m_visibleStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible
            memset (&m_activeStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible

            m_unclassVisible = true;

            m_unclassColor.SetRed(255); m_unclassColor.SetGreen(0); m_unclassColor.SetBlue(255);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void Reset()
            {
            m_visibleState = 0xFFFFFFFF;  // sets all to visible
            memset (m_classificationColor, 255, CLASSIFCATION_COUNT * sizeof(BePointCloud::PointCloudColorDef)); // all white

            m_blendColor = false;

            m_useBasecolor = false;

            memset (&m_visibleStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible

            memset (&m_activeStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible

            m_unclassVisible = false;

            }

        private:
            uint32_t            m_visibleState;

            BePointCloud::PointCloudColorDef  m_classificationColor[CLASSIFCATION_COUNT];

            bool                m_blendColor;

            bool                m_useBasecolor;

            unsigned char       m_visibleStateAdv[32];

            unsigned char       m_activeStateAdv[32];

            BePointCloud::PointCloudColorDef  m_unclassColor;

            bool                m_unclassVisible;
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
