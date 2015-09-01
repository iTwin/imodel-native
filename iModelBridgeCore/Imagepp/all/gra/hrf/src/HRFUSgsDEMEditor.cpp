//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsDEMEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFDtedEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFUSgsDEMEditor.h>


//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>

/*

#define BAND_1 0
#define BAND_2 1
#define BAND_3 2
#define BAND_4 3
    */

//-----------------------------------------------------------------------------
//                              Notes
//-----------------------------------------------------------------------------
//
//  Scaling data with a linear factor IS defenitly NOT be the best ways to
//  enhance data visualization.  Because they often provide  a meam AND a
//  standard diviation for a given dataset, we suppose the data should normally
//  follow a kind of a bell curve (wich can be define as a gaussian function...).
//
//  In that case, the smartest ways of scaling these data to spread their values
//  evenly across all the band width should be using a gaussian function.
//
//  We have provided a gaussian lookup table, and to see wich value the pixel
//  will be scaled, is to find it's proper z quote value.
//
//        X      : Original (data) pixel value.
//        X_m    : Given all pixel (data) mean.
//        st_dev : Given standard diviation.
//
//        Z      : Quote to find in the lookup table. The index will represent
//                 the new scaled pixel value.
//
//        Z = (X - X_m) / st_dev
//
//  Find where Z is in the look up, the index will be the scaled value.

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFUSgsDEMEditor::HRFUSgsDEMEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFGdalSupportedFileEditor(pi_rpRasterFile,
                                 pi_Page,
                                 pi_Resolution,
                                 pi_AccessMode)
    {

    //Return the raw data
    m_UseLinearBandScaling = false;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFUSgsDEMEditor::~HRFUSgsDEMEditor()
    {
    }



