#pragma once

//=======================================================================================
// @bsiclass                                            Daryl.Holmwood     05/2009
//+===============+===============+===============+===============+===============+======
class CRefUnitsConverter
    {
    private:
        Transform m_transform;
        Transform m_transformInv;

        Transform m_refTransform;
        Transform m_refTransformInv;
        double rdScale;

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        DgnModelRefP getRootModel(DgnModelRefP model)
            {
            if(mdlModelRef_getActive())
                return mdlModelRef_getActive();

            return model;
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        double getUORsToMetersMatrix(Transform& trsf, DgnModelRefP modelRef, bool useUORS)
            {
            mdlTMatrix_getIdentity(&trsf);

            if (useUORS) return 1;

            double uorPerMeter = 1.0 / mdlModelRef_getUorPerMeter (modelRef);
            DPoint3d ptGO;
            StatusInt siResult = mdlModelRef_getGlobalOrigin (modelRef, &ptGO);
            BeAssert (siResult == SUCCESS);  

            // Scale the matrix to UORs and translate it to the GO.
            mdlTMatrix_scale (&trsf, &trsf, uorPerMeter, uorPerMeter, uorPerMeter);
            mdlTMatrix_translate(&trsf, &trsf, -ptGO.x, -ptGO.y, -ptGO.z);

            return uorPerMeter;
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void GetTransformData
        (
        DgnModelRefP    hSrcModel,
        double          &rdScale,
        DgnModelRefP    viewModel,
        bool            useUORS,
        bool            checkRefForActivated = false
        )
            {
            DgnModelRefP rootModel = NULL;
            DgnAttachmentP pRef = NULL;
            StatusInt siResult;

            rdScale = fc_1;

            if(hSrcModel && (!checkRefForActivated || !mdlModelRef_isActiveModel (hSrcModel)) && NULL != (pRef = mdlRefFile_getInfo (hSrcModel)))
                {
                DgnAttachmentP pRef2 = NULL;

                if(viewModel)
                    pRef2 = mdlRefFile_getInfo(viewModel);

                mdlRefFile_getTransformFromParent (&m_refTransform, pRef, pRef2);
                //                    mdlRefFile_getTransformToMaster (&m_transform, pRef);
                siResult = mdlRefFile_getDoubleParameters (&rdScale, REFERENCE_SCALE, hSrcModel);
                BeAssert (siResult == SUCCESS);
                rootModel = getRootModel (hSrcModel);
                }
            else
                {
                mdlTMatrix_getIdentity (&m_refTransform);
                rootModel = hSrcModel;
                }

            Transform oActiveRootTransform;
            rdScale *= getUORsToMetersMatrix(oActiveRootTransform, rootModel, useUORS);

            Transform oActiveRefTransform;
            rdScale *= getUORsToMetersMatrix(oActiveRefTransform, hSrcModel, useUORS);



            mdlTMatrix_getInverse(&m_transform, &m_refTransform);

            mdlTMatrix_multiply (&m_transform, &oActiveRootTransform, &m_transform);
            mdlTMatrix_multiply (&m_refTransform, &oActiveRefTransform, &m_refTransform);

            mdlTMatrix_getInverse(&m_transformInv, &m_transform);
            mdlTMatrix_getInverse(&m_refTransformInv, &m_refTransform);
            }

    public:
        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        CRefUnitsConverter(DgnModelRefP modelRef, bool useUORS = false)
            {
            GetTransformData(modelRef, rdScale, NULL, useUORS);
            }

        CRefUnitsConverter(DgnModelRefP modelRef, DgnModelRefP viewModelRef, bool useUORS = false)
            {
            GetTransformData(modelRef, rdScale, viewModelRef, useUORS);
            }

        CRefUnitsConverter ()
            {}

        void Init (DgnModelRefP modelRef, bool useUORS = false, bool checkRefForActivated = false)
            {
            GetTransformData (modelRef, rdScale, NULL, useUORS, checkRefForActivated);
            }

        void Init (DgnModelRefP modelRef, DgnModelRefP viewModelRef, bool useUORS = false)
            {
            GetTransformData (modelRef, rdScale, viewModelRef, useUORS);
            }

        TransformCP GetTransform() const
            {
            return &m_transform;
            }

        TransformCP GetTransformInv() const
            {
            return &m_transformInv;
            }


        TransformCP GetRefTransform() const
            {
            return &m_refTransform;
            }

        TransformCP GetRefTransformInv() const
            {
            return &m_refTransformInv;
            }



        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRefUorsToRootMeters(DPoint3d* pts, int nbPoints)
            {
            mdlTMatrix_transformPointArray(pts, &m_transform, nbPoints);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRootMetersToRefUors(DPoint3d* pts, int nbPoints)
            {
            mdlTMatrix_transformPointArray(pts, &m_transformInv, nbPoints);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRootUorsToRefMeters(DPoint3d* pts, int nbPoints)
            {
            mdlTMatrix_transformPointArray(pts, &m_refTransform, nbPoints);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRefMetersToRootUors(DPoint3d* pts, int nbPoints)
            {
            mdlTMatrix_transformPointArray(pts, &m_refTransformInv, nbPoints);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRefUorsToRootMeters(DPoint3d& pt)
            {
            mdlTMatrix_transformPoint(&pt, &m_transform);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRootMetersToRefUors(DPoint3d& pt)
            {
            mdlTMatrix_transformPoint(&pt, &m_transformInv);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRootUorsToRefMeters(DPoint3d& pt)
            {
            mdlTMatrix_transformPoint(&pt, &m_refTransform);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        void FullRefMetersToRootUors(DPoint3d& pt)
            {
            mdlTMatrix_transformPoint(&pt, &m_refTransformInv);
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        double ScaleUorsToMeters(double value)
            {
            return value * rdScale;
            }
        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood     05/2009
        //+===============+===============+===============+===============+===============+======
        double ScaleMetersToUors(double value)
            {
            return value / rdScale;
            }
    };


