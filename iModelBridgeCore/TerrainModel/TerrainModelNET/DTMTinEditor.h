/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMTinEditor.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include ".\Bentley.Civil.DTM.h"
#include ".\DTMFeature.h"
#include ".\DTMHelpers.h"

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
/// <summary>
/// The DTM Editor class
/// </summary>                
/// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
//=======================================================================================
public ref class DTMTinEditor
    {
    public: 

        //=======================================================================================
        /// <summary>
        /// Releases the DTM editor.
        /// </summary>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        ~DTMTinEditor(void);

        //=======================================================================================
        /// <summary>
        /// Selects a feature for edit.
        /// <param name="point">Coordinates of the point to select</param>
        /// </summary>
        /// <returns>If a feature was found.</returns>
        /// <category>DTM Edit</category>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        bool Select (DTMDynamicFeatureType featureType, BGEO::DPoint3d point);

        //=======================================================================================
        /// <summary>
        /// Delete the selected Featrue
        /// </summary>     
        /// <returns>If the delete was successfulThe slope area inside the polygon</returns>
        /// <category>DTM Edit</category>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        bool Delete();

        //=======================================================================================
        /// <summary>
        /// Gets the Points of the selected feature
        /// </summary>
        /// <category>DTM Edit</category>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        property array<BGEO::DPoint3d>^ SelectedFeaturePoints
            {
            array<BGEO::DPoint3d>^ get()
                {
                return m_featurePoints;
                }
            }

    internal:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTMTinEditor class.
        /// </summary>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        DTMTinEditor (BcDTMP nativeDtm)
            {
            m_nativeDtm = nativeDtm;
            }

    private: 

        //=======================================================================================
        /// <summary>
        /// Gets the unmanaged dtm held by the managed DTM.
        /// </summary>     
        /// <category>DTM Edit</category>
        /// <author>Daryl.Holmwood</author>                              <date>04/2008</date>
        //=======================================================================================
        property BcDTMP BcDTM
            {
            BcDTMP get();
            }

        array<BGEO::DPoint3d>^ m_featurePoints;
        BcDTMP m_nativeDtm;
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
