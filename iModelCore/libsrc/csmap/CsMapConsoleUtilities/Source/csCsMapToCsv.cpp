//===========================================================================
// $Header$
//
//    (C) Copyright 2007 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined 
// in section 499C of the penal code of the State of California.  
// Use of this information by anyone other than authorized employees
// of Autodesk, Inc. is granted only under a written non-disclosure 
// agreement, expressly prescribing the scope and manner of such use.       
//
// CREATED BY:
//      Norm Olsen
//
// DESCRIPTION:
//

#include "csConsoleUtilities.hpp"

struct csWktPrjNameMap_ csWktPrjNameMap [] =
{
/*	The following is a guide to keep literal constants within the size of the 
	table elements.
                                     1         2         3         4         5         6
                            123456789012345678901234567890123456789012345678901234567890123  */
	{ cs_PRJCOD_LM1SP,9801,"Lambert_Conformal_Conic_1SP",			// WKT OCR
	                       "CT_LambertConfConic_1SP",				// WKT GeoTiff
	                       "Lambert_Conformal_Conic",				// WKT ESRI (std frm)
	                       "Lambert Conformal Conic",				// WKT Oracle
	                       "Lambert_Conic_Conformal_1SP",			// WKT Geo Tools
	                       "Lambert Conic Conformal (1SP)",			// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
//ADSK-HW October 2nd 2006 - adding a new entry for 9801 from MapGuide
	{ cs_PRJCOD_LM1SP,9801,"",
	                       "",
	                       "Lambert Conformal Conic",
	                       "",				                        // WKT Oracle
	                       "",			                            // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_LM2SP,9802,"Lambert_Conformal_Conic_2SP",
	                       "CT_LambertConfConic_2SP",
	                       "Lambert_Conformal_Conic",				// WKT ESRI (std frm)
	                       "Lambert Conformal Conic",				// WKT Oracle
	                       "Lambert_Conic_Conformal_2SP",			// Geo Tools
	                       "Lambert Conic Conformal (2SP)",
	                       "",
	                       ""},
	{ cs_PRJCOD_LM2SP,9802,"",
	                       "",
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Lambert_Conformal_Conic_2SP",			// Geo Tools
	                       "Lambert Conic Conformal (2SP)",
	                       "",
	                       ""},
	{ cs_PRJCOD_LMBLG,9803,"Lambert_Conformal_Conic_2SP_Belgium",
	                       "",
	                       "",										// WKT ESRI (std frm)
	                       "Lambert Conformal Conic (Belgium 1972)",// WKT Oracle
	                       "Lambert_Conic_Conformal_2SP_Belgium",	// Geo Tools
	                       "Lambert Conic Conformal (2SP Belgium)",
	                       "",
	                       ""},
	{ cs_PRJCOD_MRCAT,9804,"Mercator_1SP",
	                       "CT_Mercator",
	                       "Mercator",								// WKT ESRI (std frm)
	                       "Mercator",								// WKT Oracle
	                       "Mercator_1SP",							// WKT Geo Tools
	                       "Mercator (1SP)",
	                       "",
	                       ""},
	{ cs_PRJCOD_MRCAT,9805,"Mercator_2SP",
	                       "CT_Mercator",
	                       "Mercator",								// WKT ESRI (std frm)
	                       "Mercator",								// WKT Oracle
	                       "Mercator_2SP",							// WKT Geo Tools
	                       "Mercator (2SP)",
	                       "",
	                       ""},
	{ cs_PRJCOD_CSINI,9806,"Cassini-Soldner",
	                       "CT_CassiniSoldner",
	                       "Cassini",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Cassini-Soldner",						// WKT Geo Tools
	                       "Cassini-Soldner",
	                        "",
	                        ""},
	{ cs_PRJCOD_TRMER,9807,"Transverse_Mercator",
	                       "CT_TransverseMercator",
	                       "Transverse_Mercator",					// WKT ESRI (std frm)
	                       "Transverse Mercator",					// WKT Oracle
	                       "Transverse_Mercator",					// WKT Geo Tools
	                       "Transverse Mercator",
	                        "",
	                        ""},
	{ cs_PRJCOD_TRMER,9807,"",
	                       "",
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Transverse Mercator Finnish KKJ",		// WKT Geo Tools
	                       "",
	                        "",
	                        ""},
	{cs_PRJCOD_GAUSSK,9807,"Gauss_Kruger",
	                       "GaussKruger",
	                       "Gauss_Kruger",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Transverse Mercator",					// WKT Heo Tools
	                       "Transverse Mercator",
	                        "",
	                        ""},
	{ cs_PRJCOD_SOTRM,9808,"Transverse_Mercator_South_Oriented",
	                       "CT_TransverseMercator_SouthOriented",
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Transverse_Mercator_South_Orientated",	// WKT Geo Tools
	                       "Transverse Mercator (South Orientated)",
	                        "",
	                        ""},
	{ cs_PRJCOD_OSTRO,9809,"Oblique_Stereographic",
	                       "CT_ObliqueStereographic",
	                       "Double_Stereographic",					// WKT ESRI (std frm)
	                       "Stereographic",							// WKT Oracle
	                       "Oblique_Stereographic",					// WKT Geo Tools
	                       "Oblique Stereographic",
	                        "",
	                        ""},
	{ cs_PRJCOD_PSTRO,9810,"Polar_Stereographic",
	                       "CT_PolarStereographic",
	                       "Stereographic",							// WKT ESRI (std frm)
	                       "Stereographic",							// WKT Oracle
	                       "Polar_Stereographic",					// WKT Geo Tools
	                       "Polar Stereographic",
	                        "",
	                        ""},
	{ cs_PRJCOD_NZLND,9811,"New_Zealand_Map_Grid",
	                       "CT_NewZealandMapGrid",
	                       "New_Zealand_Map_Grid",					// WKT ESRI (std frm)
	                       "New Zealand Map Grid",					// WKT Oracle
	                       "New_Zealand_Map_Grid",					// Wkt Geo Tools
	                       "New Zealand Map Grid",
	                        "",
	                        ""},
//ADSK-HW October 2nd 2006 - type fixed in the second field as in MapGuide - was hotine_oblique_mercator instead of Hotine_Oblique_Mercator
	{cs_PRJCOD_HOM1XY,9812,"Hotine_Oblique_Mercator",
	                       "CT_ObliqueMercator_Hotine",
	                       "Hotine_Oblique_Mercator_Azimuth_Natural_Origin",	// WKT ESRI (std frm)
	                       "Hotine Oblique Mercator",				// WKT Oracle
	                       "Hotine_Oblique_Mercator",				// WKT Geo Tools				
	                       "Hotine Oblique Mercator",
	                        "",
	                        ""},
	{cs_PRJCOD_LABORD,9813,"Laborde_Oblique_Mercator",
	                       "CT_ObliqueMercator_Laborde",
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools				
	                       "Laborde Oblique Mercator",
	                       "",
	                       ""},
	{ cs_PRJCOD_SWISS,9814,"Swiss_Oblique_Cylindrical",
	                       "CT_SwissObliqueCylindrical",
	                       "",										// WKT ESRI (std frm)
	                       "Swiss Oblique Mercator",				// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "Swiss Oblique Cylindrical",
	                       "",
	                       ""},
	{ cs_PRJCOD_RSKEW,9815,"Oblique_Mercator",
	                       "CT_ObliqueMercator",
	                       "Hotine_Oblique_Mercator_Azimuth_Center",// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Oblique_Mercator",						// WKT Geo Tools
	                       "Oblique Mercator",
	                       "",
	                       ""},
	{cs_PRJCOD_TUNGRD,9816,"Tunisia_Mining_Grid",
	                       "",
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Tunisia_Mining_Grid",					// WKT Geo Tools
	                       "Tunisia Mining Grid",
	                       "",
	                       ""},
//ADSK-HW October 2nd 2006 - 9817 added from MapGuide
	{cs_PRJCOD_LMNC,  9817,"Lambert Conic Near-Conformal",
	                       "",
	                       "",
	                       "",										// WKT Oracle
	                       ""					                    // WKT Geo Tools
	                       "Lambert Conic Near-Conformal",
	                       "",
	                       ""},
	{ cs_PRJCOD_PLYCN,9818,"Polyconic",
	                       "CT_Polyconic",
	                       "Polyconic",								// WKT ESRI (std frm)
	                       "Polyconic",								// WKT Oracle
	                       "American_Polyconic",					// WKT Geo Tools
	                       "American Polyconic",
	                       "",
	                       ""},
	{cs_PRJCOD_KROVAK,9819,"Krovak",
	                       "",
	                       "Krovak",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
						   "Krovak_Oblique_Conic_Conformal",		// WKT Geo Tools
	                       "Krovak Oblique Conic Conformal",
	                       "",
	                       ""},
	{ cs_PRJCOD_AZMEA,9820,"Lambert_Azimuthal_Equal_Area",
	                       "CT_LambertAzimuthalEqualArea",
	                       "Lambert_Azimuthal_Equal_Area",				// WKT ESRI (std frm)
	                       "Lambert Azimuthal Equal Area",				// WKT Oracle
						   "Lambert_Azimuthal_Equal_Area_Spherical",	// WKT Geo Tools
	                       "Lambert Azimuthal Equal Area",
	                       "",
	                       ""},
//ADSK-HW October 2nd 2006 - 9821 added from MapGuide
	{cs_PRJCOD_AZMEAS,9821,"Lambert Azimuthal Equal Area (Spherical)",
	                       "",
	                       "",
	                       "",				                        // WKT Oracle
						   "",	                                    // WKT Geo Tools
	                       "Lambert Azimuthal Equal Area (Spherical)",
	                       "",
	                       ""},
	{ cs_PRJCOD_ALBER,9822,"Albers_Conic_Equal_Area",
	                       "CT_AlbersEqualArea",
	                       "Albers",								// WKT ESRI (std frm)
	                       "Albers Conical Equal Area",				// WKT Oracle
	                       "Albers_Conic_Equal_Area",				// WKT Geo Tools
//ADSK-HW October 2nd 2006 - empty field updated from MapGuide
	                       "Albers Equal Area",
	                       "",
	                       ""},
	{ cs_PRJCOD_EDCYL,9823,"Equirectangular",
	                       "CT_Equirectangular",
	                       "Equidistant_Cylindrical",				// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Equidistant_Cylindrical",				// WKT Geo Tools
	                       "Equidistant Cylindrical",
	                       "",
	                       ""},
	{   cs_PRJCOD_UTM,9824,"Transverse_Mercator",
	                       "CT_TransverseMercator",
	                       "Transverse_Mercator",					// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Transverse_Mercator",					// WKT Geo Tools
	                       "Transverse Mercator",
	                        "",
	                        ""},
//ADSK-HW October 2nd 2006 - 9825, 9826 added from MapGuide
	{cs_PRJCOD_PSEUDO,9825,"Pseudo Plate Carree",
	                       "",
	                       "",
	                       "",										// WKT Oracle
	                       "",					                    // WKT Geo Tools
	                       "Pseudo Plate Carree",
	                       "",
	                       ""},
	{ cs_PRJCOD_LMWO, 9826,"Lambert Conic Conformal (West Orientated)",
	                       "",
	                       "",
	                       "",										// WKT Oracle
	                       "",					                    // WKT Geo Tools
	                       "Lambert Conic Conformal (West Orientated)",
	                       "",
	                       ""},
//ADSK-HW October 2nd 2006 - 2nd field updated from MapGuide - was empty instead of "Bonne Pseudoconical Projection"
	{ cs_PRJCOD_BONNE,9827,"Bonne Pseudoconical Projection",
	                       "",
	                       "Bonne",									// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "Bonne",
	                       "",
	                       ""},
	{ cs_PRJCOD_EDCNC,   0,"Equidistant_Conic",
	                       "",
	                       "Equidistant_Conic",						// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_AZMED,   0,"Lambert_Azimuthal_Equidistant",
	                       "CT_LambertAzimuthalEquidistant",
	                       "Azimuthal_Equidistant",					// WKT ESRI (std frm)
	                       "Azimuthal Equidistant",					// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "Lambert Azimuthal Equidistant",
	                       "",
	                       ""},
	{ cs_PRJCOD_MILLR,   0,"Miller_Cylindrical",
	                       "CT_MillerCylindrical",
	                       "Miller_Cylindrical",					// WKT ESRI (std frm)
	                       "Miller Cylindrical",					// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_SINUS,   0,"Sinusoidal",
	                       "CT_Sinusoidal",
	                       "Sinusoidal",							// WKT ESRI (std frm)
	                       "Sinusoidal",							// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_VDGRN,   0,"VanDerGrinten",
	                       "CT_VanDerGrinten",
	                       "Van_der_Grinten_I",						// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_ROBIN,   0,"Robinson",
	                       "CT_Robinson",
	                       "Robinson",								// WKT ESRI (std frm)
	                       "Robinson",								// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_EKRT4,   0,"Eckert_IV",
	                       "",
	                       "Eckert_IV",								// WKT ESRI (std frm)
	                       "Eckert IV",								// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_EKRT6,   0,"Eckert_VI",
	                       "",
	                       "Eckert_VI",								// WKT ESRI (std frm)
	                       "Eckert VI",								// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_MOLWD,   0,"Mollweide",
	                       "Mollweide",
	                       "Mollweide",								// WKT ESRI (std frm)
	                       "Mollweide",								// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_PSTRO,   0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Stereographic_North_Pole",				// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_PSTRO,   0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Stereographic_South_Pole",				// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_HOM2XY,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Hotine_Oblique_Mercator_Two_Point_Natural_Origin",	// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
//ADSK-HW October 2nd 2006 - added from MapGuide
    { cs_PRJCOD_HOM2XY,  0,"Rectified Skew Orthomorphic, Two Point Form",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
	{  cs_PRJCOD_RSKEW,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Rectified_Skew_Orthomorphic_Natural_Origin", // WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_ORTHO,   0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Orthographic",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
//ADSK-HW October 2nd 2006 - added from MapGuide
    { cs_PRJCOD_ORTHO,   0,"Orthographic Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_GNOMC,   0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Gnomonic",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
//ADSK-HW October 2nd 2006 - added from MapGuide
    { cs_PRJCOD_GNOMC,   0,"Gnomonic Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_EDCYL,   0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Cylindrical_Equal_Area",				// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
//ADSK-HW November 2nd 2007 - OGC added from MapGuide
	{ cs_PRJCOD_WINKL,   0,"Winkel-Tripel, variable standard latitude",					// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Winkel_Tripel",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Eckert_I",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Eckert_II",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Eckert_III",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Eckert_V",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Gall_Stereographic",					// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Behrmann",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Quartic_Authalic",						// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Loximuthal",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Two_Point_Equidistant",					// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Winkel_I",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Winkel_II",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Aitoff",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Hammer_Aitoff",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Flat_Polar_Quartic",					// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Craster_Parabolic",						// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Times",									// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Vertical_Near_Side_Perspective",		// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Plate_Carree",							// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Fuller",								// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Cube",									// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Transverse_Mercator_Complex",			// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "Flat_Polar_Quartic",					// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Bonne_South_Orientated",				// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Lambert_Conic_Conformal_West_Orientated",	// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Transverse_Mercator_Zoned_Grid_System",	// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
	{ cs_PRJCOD_NOTYET,  0,"",										// WKT OCR
	                       "",										// WKT GeoTiff
	                       "",										// WKT ESRI (std frm)
	                       "",										// WKT Oracle
	                       "Lambert_Conic_Near-Conformal",			// WKT Geo Tools
	                       "",										// EPSG Name
	                       "",										// Application Alternate
	                       ""},										// Local alternate
//ADSK-HW October 2nd 2006 - added from MapGuide
	{ cs_PRJCOD_UNITY,   0,"Null Projection",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_WCCSL,   0,"Lambert Conformal Conic, Wisconsin County Variation",
	                       "",
	                       "",
	                       "",                                  // WKT Oracle
	                       "",	                                // Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_MNDOTL,  0,"Lambert Conformal Conic, Minnesota DOT Variation",
	                       "",
	                       "",
	                       "",                                  // WKT Oracle
	                       "",	                                // Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_WCCST,   0,"Transverse Mercator, Wisconsin County Variation",
	                       "",
	                       "",
	                       "",			                        // WKT Oracle
	                       "",			                        // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_MNDOTT,  0,"Transverse Mercator, Minnesota DOT Variation",
	                       "",
	                       "",
	                       "",			                        // WKT Oracle
	                       "",			                        // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_TRMRS,   0,"Transverse Mercator per J. P. Snyder",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_HOM1UV,  0,"Unrectified Hotine Oblique Mercator Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_HOM2UV,  0,"Unrectified Hotine Oblique Mercator Projection, Two Point Form",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_MODPC,   0,"Lallemand IMW Modified Polyconic Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_SSTRO,   0,"Oblique Sterographic Projection, per Snyder (USA)",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_PSTROSL, 0,"Polar Sterographic Projection with Standard Circle",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_HMLSN,   0,"Goode Homolosine Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_NACYL,   0,"Normal Aspect, Equal Area Cylindrical Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_TACYL,   0,"Transverse Aspect, Equal Area Cylindrical Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_BPCNC,   0,"Bipolar Oblique Conformal Conic Projection",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_TRMERAF, 0,"Transverse Mercator (Gauss/Kruger) with Affine Post Process",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_OBQCYL,  0,"Oblique Cylindrical Projection (Generalized)",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_AZEDE,   0,"Lambert Azimuthal Equidistant, Elevated Ellipsoid",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_LMTAN,   0,"Lambert Tangential Conformal Conic Projection",
	                       "",
	                       "",
	                       "Lambert Conformal Conic",			// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_LMTAN,   0,"Lambert Tangential",
	                       "",
	                       "",
	                       "Lambert Conformal Conic",			// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_OSTN02,  0,"Ordnance Survey National Grid Transformation of 2002",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_OSTN97,  0,"Ordnance Survey National Grid Transformation of 1997",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_MRCATK,  0,"Mercator Cylindrical Projection with Scale Reduction",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_RSKEWO,  0,"Rectified Skew Orthomorphic, Skew Azimuth at Rectified Origin",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_RSKEWC,  0,"Rectified Skew Orthomorphic, Origin & Azimuth at Center",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_KRVK95,  0,"Krovak Oblique Conformal Conic/95 Adjustment",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_MSTRO,   0,"Modified Sterographic Projection",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
	{ cs_PRJCOD_SYS34,   0,"Danish System 34, UTM + polynomials (pre-1999 vinatge)",
	                       "",
	                       "",
	                       "",									// WKT Oracle
	                       "",					                // WKT Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_SYS34_99,0,"Danish System 34, UTM + polynomials (1999 vintage)",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
    { cs_PRJCOD_TRMRKRG, 0,"Transverse Mercator using Kruger Formulation",
	                       "",
	                       "",
	                       "",			                            // WKT Oracle
	                       "",			                            // Geo Tools
	                       "",
	                       "",
	                       ""},
//ADSK-HW October 2nd 2006 - end of new projections added from MapGuide
	{   cs_PRJCOD_END,   0,"",
	                       "",
	                       "",
	                       "",										// WKT Oracle
	                       "",										// WKT Geo Tools
	                       "",
	                       "",
	                       ""}
};

/******************************************************************************
	The following table maps a WKT parameter name with an enumeration value.
	Where appropriate, equivalent cs_PRMCOD_????? values are given as well as
	EPSG numeric code values.  Note, that:

	1> cs_PRMCOD_????? is actually an index into the cs_Prjprm table defined
	   above. 
	2> This table includes datum parameter codes as these values appear as
	   elements of the Parameter type in WKT.
	3> The above enumeration classifies several parameters as being Cartesian
	   transformation parameters as that uis what they will be in the soon to
	   be release RefCon Engine product.
	4> Several names are given, and the actual names used depend upon the
	   specific flavor of WKT being processed.  CS-MAP prefers the OGC flavor
	   of WKT believing it to be the most standardized of the various flavors,
	   although the ESRI flavor may be the most ubiquitous.

	Some of the names here are guesses as I can't find official documentation
	for all of these.  Help me out if and when you can (888-ASK-NORM).
*/
struct csWktPrmNameMap_ csWktPrmNameMap [] =
{
	{ csWktPrmXTranslation,   cs_WKTCOD_DELTAX,8605,"X_Axis_Translation",	// WKT OCR
													"X_Axis_Translation",	// WKT GeoTiff
													"X_Axis_Translation",	// WKT ESRI (std frm?)
													"X Axis Translation",	// WKT Oracle
													"",						// WKT Geo Tools
													"X-axis translation",	// EPGS Name
													"",						// Application ALternate Name
													"" },					// Local Alternate Name
	{ csWktPrmYTranslation,   cs_WKTCOD_DELTAY,8606,"Y_Axis_Translation",
													"Y_Axis_Translation",
													"Y_Axis_Translation",
													"Y Axis Translation",	// WKT Oracle
													"",						// WKT Geo Tools
													"Y-axis translation",
													"",
													""},
	{ csWktPrmZTranslation,   cs_WKTCOD_DELTAZ,8607,"Z_Axis_Translation",
													"Z_Axis_Translation",
													"Z_Axis_Translation",
													"Z Axis Translation",	// WKT Oracle
													"",						// WKT Geo Tools
													"Z-axis translation",
													"",
													""},
	{ csWktPrmXRotation,      cs_WKTCOD_ROTATX,8608,"X_Axis_Rotation",
													"X_Axis_Rotation",
													"X_Axis_Rotation",
													"X Axis Rotation",		// WKT Oracle
													"",						// WKT Geo Tools
													"X-axis rotation",
													"",
													""},
	{ csWktPrmYRotation,      cs_WKTCOD_ROTATY,8609,"Y_Axis_Rotation",
													"Y_Axis_Rotation",
													"Y_Axis_Rotation",
													"Y Axis Rotation",		// WKT Oracle
													"",						// WKT Geo Tools
													"Y-axis rotation",
													"",
													""},
	{ csWktPrmZRotation,      cs_WKTCOD_ROTATZ,8610,"Z_Axis_Rotation",
													"Z_Axis_Rotation",
													"Z_Axis_Rotation",
													"Z Axis Rotation",		// WKT Oracle
													"",						// WKT Geo Tools
													"Z-axis rotation",
													"",
													""},
	{ csWktPrmDatumScale,     cs_WKTCOD_BWSCAL,8611,"Datum_Scale",
													"Datum_Sale",
													"Scale_Difference",
													"Scale Difference",		// WKT Oracle
													"",						// WKT Geo Tools
													"Scale difference",
													"",
													""},
	{csWktPrmLatitudeOfOrg,   cs_WKTCOD_ORGLAT,8801,"latitude_of_origin",
													"NatOriginLat",
													"Latitude_Of_Origin",
													"Latitude_Of_Origin",	// WKT Oracle
													"latitude_of_origin",	// WKT Geo Tools
													"Latitude of natural origin",
													"",
													""},
	{csWktPrmLongitudeOfOrg,  cs_PRMCOD_CNTMER,8802,"central_meridian",
													"NatOriginLong",
													"Central_Meridian",
													"Central_Meridian",		// WKT Oracle
													"central_meridian",		// WKT Geo Tools
													"Longitude of natural origin",
													"",
													""},
	/* ESRI uses Central_Meridian for all projections, except the New Zealnd Map Grid.  For
	   that one projection it uses Longitude_of_Origin".  Strange, and painful, but true. */
	{csWktPrmLongitudeOfOrg,  cs_PRMCOD_CNTMER,8802,"",
													"",
													"Longitude_of_Origin",	// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmLongitudeOfOrg,  cs_PRMCOD_CNTMER,8802,"",
													"",
													"Longitude_Of_Origin",		// ESRI uses this for 1 projection: New Zealand Map Grid
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmScaleAtOrigin,   cs_WKTCOD_SCLRED,8805,"scale_factor",
													"ScaleAtNatOrigin",
													"Scale_Factor",
													"Scale_Factor",			// WKT Oracle
													"scale_factor",			// WKT Geo Tools
													"Scale factor at natural origin",
													"",
													""},
	{csWktPrmFalseEasting,     cs_WKTCOD_FEAST,8806,"false_easting",
													"FalseEasting",
													"False_Easting",
													"False_Easting",		// WKT Oracle
													"false_easting",		// WKT Geo Tools
													"False easting",
													"",
													""},
	{csWktPrmFalseNorthing,   cs_WKTCOD_FNORTH,8807,"false_northing",
													"FalseNorthing",
													"False_Northing",
													"False_Northing",		// WKT Oracle
													"false_northing",		// WKT Geo Tools
													"False northing",
													"",
													""},
	{csWktPrmLatitudeOfCtr,   cs_PRMCOD_GCPLAT,8811,"latitude_of_center",
													"CenterLat",
													"Latitude_Of_Center",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Latitude of projection centre",
													"",
													""},
	{csWktPrmLongitudeOfCtr,  cs_PRMCOD_GCPLNG,8812,"longitude_of_center",
													"CenterLong",
													"Longitude_Of_Center",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Longitude of projection centre",
													"",
													""},
	{      csWktPrmAzimuth,    cs_PRMCOD_GCAZM,8813,"azimuth",
													"AzimuthAngle",
													"Azimuth",
													"Azimuth",				// WKT Oracle
													"",						// WKT Geo Tools
													"Azimuth of initial line",
													"",
													""},
	{csWktPrmRectifiedGrdAng, cs_PRMCOD_SKWAZM,8814,"rectified_grid_angle",
													"RectifiedGridAngle",
													"Rectified_Grid_Angle",		/*??*/
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Angle from Rectified to Skew Grid",
													"",
													""},
	{ csWktPrmScaleAtCenter,  cs_WKTCOD_SCLRED,8815,"scale_factor",
													"ScaleAtCenter",
													"Scale_Factor",
													"",						// WKT Oracle
													"scale_factor",			// WKT Geo Tools
													"Scale factor on initial line",
													"",
													""},
	{csWktPrmEastingOfCtr,     cs_WKTCOD_FEAST,8816,"false_easting",
													"FalseEasting",
													"False_Easting",
													"",						// WKT Oracle
													"false_easting",		// WKT Geo Tools
													"Easting at projection center",
													"",
													""},
	{csWktPrmNorthingOfCtr,   cs_WKTCOD_FNORTH,8817,"false_northing",
													"FalseNorthing",
													"False_Northing",
													"",						// WKT Oracle
													"false_northing",		// WKT Geo Tools
													"Northing at projection center",
													"",
													""},
	{csWktPrmPsdoParallelLat,cs_PRMCOD_OSTDPLL,8818,"pseudo_standard_parallel_1",
													"",
													"Pseudo_Standard_Parallel_1",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Latitude of pseudo standard parallel",
													"",
													""},
	{csWktPrmPsdoParallelScl, cs_PRMCOD_NOTYET,8819,"scale_factor",
													"ScaleAtCenter",
													"Scale_Factor",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Scale factor on pseudo standard parallel",
													"",
													""},
	{csWktPrmLatOfFalseOrg,   cs_WKTCOD_ORGLAT,8821,"latitude_of_origin",
													"FalseOriginLat",
													"Latitude_Of_Origin",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Latitude of false origin",
													"",
													""},
	{csWktPrmLngOfFalseOrg,   cs_WKTCOD_ORGLNG,8822,"central_meridian",
													"FalseOriginLong",
													"Central_Meridian",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Longitude of false origin",
													"",
													""},
	{csWktPrmStdParallel1,   cs_PRMCOD_NSTDPLL,8823,"standard_parallel_1",
													"StdParallel1",
													"Standard_Parallel_1",
													"Standard_Parallel_1",	// WKT Oracle
													"standard_parallel1",	// WKT Geo Tools
													"Longitude of 1st standard parallel",
													"",
													""},
	{csWktPrmStdParallel2,  cs_PRMCOD_SSTDPLL,8824,"standard_parallel_2",
													"StdParallel2",
													"Standard_Parallel_2",
													"Standard_Parallel_2",	// WKT Oracle
													"standard_parallel2",	// WKT Geo Tools
													"Longitude of 2nd standard parallel",
													"",
													""},
	{csWktPrmEastingFalseOrg, cs_WKTCOD_FEAST,8826,"false_easting",
													"FalseOriginEasting",
													"False_Easting",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Easting of false origin",
													"",
													""},
	{csWktPrmNorthingFalseOrg,cs_WKTCOD_FNORTH,8827,"false_northing",
													"FalseOriginNorthing",
													"False_Northing",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Northing of false origin",
													"",
													""},
	{  csWktPrmLatOfStdParall,cs_PRMCOD_STDCIR,8832,"",
													"",
													"",
													"",						// WKT Oracle
													"latitude_true_scale",	// WKT Geo Tools
													"Latitude of Standard Parallel",
													"",
													""},
//ADSK-HW October 2nd 2006 - adding 2 new entries from MapGuide
    {csWktPrmZone,cs_PRMCOD_UTMZN,0,                "Zone",
													"Zone",
													"Zone",
													"Zone",						// WKT Oracle
													"Zone",						// WKT Geo Tools
													"Zone",
													"",
													""},
    {csWktPrmRegion,cs_PRMCOD_DENRGN,0,             "Region",
													"Region",
													"Region",
													"Region",						// WKT Oracle
													"Region",						// WKT Geo Tools
													"Region",
													"",
													""},
	/* We use the cs_PRMCOD_NOTYET to indicate a parameter which is not
	   used by any CS-MAP projection, but will likely have at least an
	   equivalent parameter in the future.  Notice, that WKT does not support
	   any of the EPSG projections which require these parameters either. */
	{csWktPrmSemiMajor,          cs_PRMCOD_NOTYET,0,"",
													"",
													"",						// WKT ESRI
													"",						// WKT Oracle
													"semi_major",			// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmSemiMinor,          cs_PRMCOD_NOTYET,0,"",
													"",
													"",						// WKT ESRI
													"",						// WKT Oracle
													"semi_minor",			// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmXScale,             cs_PRMCOD_NOTYET,0,"",
													"",
													"X_Scale",				// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmYScale,             cs_PRMCOD_NOTYET,0,"",
													"",
													"Y_Scale",				// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmXYRotation,         cs_PRMCOD_NOTYET,0,"",
													"",
													"XY_Plane_Rotation",	// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmPseudoStdParall1,   cs_PRMCOD_NOTYET,0,"",
													"",
													"Pseudo_Standard_Parallel_1",	// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmPseudoStdParall2,   cs_PRMCOD_NOTYET,0,"",
													"",
													"Pseudo_Standard_Parallel_2",	// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmOption,             cs_PRMCOD_NOTYET,0,"",
													"",
													"Option",				// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmHeight,             cs_PRMCOD_NOTYET,0,"",
													"",
													"Height",				// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmCentralParallel,    cs_PRMCOD_NOTYET,0,"",
													"",
													"Central_Parallel",		// WKT ESRI
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmSphericalOrgLat, cs_PRMCOD_NOTYET,8828,"",
													"",
													"",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Spherical latitude of origin",
													"",
													""},
	{csWktPrmSphericalOrgLng, cs_PRMCOD_NOTYET,8829,"",
													"",
													"",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Spherical longitude of origin",
													"",
													""},
	{csWktPrmSystemWestLimit, cs_PRMCOD_NOTYET,8830,"",
													"",
													"",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Initial longitude",
													"",
													""},
	{csWktPrmSystemZoneWidth, cs_PRMCOD_NOTYET,8831,"",
													"NatOriginLong",
													"",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"Zone width",
													"",
													""},
	/* The following are parameters which appear to be supported by WKT but not
	   by EPSG.  Therefore, the names are (in many cases) guesses and there are
	   no EPSG names or codes. */
	{csWktPrmLatFirstPoint,   cs_PRMCOD_P1LAT,8802,"latitude_of_1st_point",
													"",
													"Latitude_Of_1st_Point",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmLngFirstPoint,    cs_PRMCOD_P1LNG,   0,"longitude_of_1st_point",
													"",
													"Longitude_Of_1st_Point",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmLatSecondPoint,   cs_PRMCOD_P2LAT,   0,"latitude_of_2nd_point",
													"",
													"Latitude_Of_2nd_Point",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	{csWktPrmLngSecondPoint,  cs_PRMCOD_P2LNG,8802,"longitude_of_2nd_point",
													"",
													"Longitude_Of_2nd_Point",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	/* The Y Axis Azimuth parameter is a parameter MSI has added to its 
	   implementation of the Lambert Equidistant Azimuthal and the Lambert
	   Equal Area Azimuthal.  Neither EPSG or WKT support it. */
	{csWktPrmYaxisAzimuth,   cs_PRMCOD_YAXISAZ,   0,"",
													"",
													"",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""},
	/* The following entry terminates the table. */
	{    csWktPrmUnknown,    cs_PRMCOD_NOTUSED,   0,"",
													"",
													"",
													"",						// WKT Oracle
													"",						// WKT Geo Tools
													"",
													"",
													""}
};

/******************************************************************************
	The MSI <-->  WKT unit mapping table.  There are a ton more units, but I
	only have reliable information about the WKT names assigned to a very few.
	So, for now, we do these very few and add others as information becomes
	available.  We assume the names will vary with the various flavors. */
struct csWktUnitNameMap_ csWktUnitNameMap [] =
{
	{      "Meter", 9001, "metre",                  "Meter",
													"Meter",
													"Meter",
													"Meter",		// WKT Oracle
													"metre",		// WKT Geo Tools
													"",
													""},
//ADSK-HW October 2nd 2006 - new entry from MapGuide
	{      "Meter", 9001, "metre",                  "metre",
													"metre",
													"metre",
													"metre",		// WKT Oracle
													"metre",		// WKT Geo Tools
													"",
													""},
	{  "Kilometer", 9036, "kilometre",              "Kilometer",
													"Kilometer",
													"Kilometer",
													"Kilometer",	// WKT Oracle
													"kilometre",	// WKT Geo Tools
													"",
													""},
	{      "IFoot", 9002, "foot",                   "Foot_Intnl",
													"Foot_Intnl",
													"Foot",
													"Foot (International)",	// WKT Oracle
													"3.048 dm",				// WKT Geo Tools
													"",
													""},
	{       "Foot", 9003, "US survey foot",         "Foot_US",
													"Foot_US",
													"Foot_US",
													"U.S. Foot",		// WKT Oracle
													"3.048 dm",			// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{       "Foot", 9003, "US survey foot",         "US survey foot",
													"Foot_US",
													"Foot_US",
													"U.S. Foot",		// WKT Oracle
													"3.048 dm",			// WKT Geo Tools
													"",
													""},
	{       "Mile", 9035, "US survey mile",         "Mile_US",
													"Mile_US",
													"Mile_US",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-HW October 2nd 2006 - fixing 3rd parameter that is empty to "IMile" from MapGuide
	{      "IMile", 9093,   "Statute Mile",         "IMile",
													"",
													"",
													"Mile",				// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-HW October 2nd 2006 - fixing 2nd and 3rd parameter that are empty to "Yard" and "IYard" from MapGuide
	{      "IYard",    0,               "Yard",     "IYard",
													"",
													"",
													"Yard",				// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "IntnlChain",9097,           "chain",         "",
													"",
													"chain",
													"Chain",			// WKT Oracle
													"",
													""},
	{  "IntnlLink",9098,            "link",         "",
													"",
													"link",
													"Link",				// WKT Oracle
													"",
													""},
	{ "ClarkeFoot", 9005, "Clarke's foot",          "foot_clarke",
													"Foot_Clarke",
													"Foot_Clarke",
													"Clarke's Foot",	// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{ "ClarkeFoot", 9005, "Clarke's foot",          "Clarke's foot",
													"Foot_Clarke",
													"Foot_Clarke",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "ClarkeYard", 9037, "Clarke's yard",          "Yard_Clarke",
													"Yard_Clarke",
													"Yard_Clarke",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "SearsYard", 9040, "Sears yard",              "Yard_Sears",
													"Yard_Sears",
													"Yard_Sears",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{"SearsYard", 9040, "British yard (Sears 1922)","British yard (Sears 1922)",
													"Yard_Sears",
													"Yard_Sears",
													"Yard (Sears)",		// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "IndianFt75", 9083, "Indian foot (1975)",     "",
													"",
													"",
													"Indian Foot",		// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "IndianYard", 9084, "Indian yard",            "Yard_Indian",
													"Yard_Indian",
													"Yard_Indian",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{ "IndianYard", 9084, "Indian yard",            "Indian yard",
													"Yard_Indian",
													"Yard_Indian",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "ClarkeLink", 9084, "Clarke's link",          "Link_Clarke",
													"Link_Clarke",
													"Link_Clarke",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{"GoldCoastFoot", 9094, "Gold Coast foot",      "gold_coast_foot",
													"GoldCoastFoot",
													"Foot_Gold_Coast",
													"",					// WKT Oracle
													"3.048 dm",			// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{"GoldCoastFoot", 9094, "Gold Coast foot",      "Gold Coast foot",
													"GoldCoastFoot",
													"Foot_Gold_Coast",
													"",					// WKT Oracle
													"3.048 dm",			// WKT Geo Tools
													"",
													""},
	{  "BenoitChain", 9062, "British chain (Benoit 1895 B)",
													"benoit_chain",
													"BenoitChain",
													"Chain_Benoit_1895_B",
													"Chain (Benoit)",	// WKT Oracle
													"2.012 dam",		// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{  "BenoitChain", 9062, "British chain (Benoit 1895 B)",
													"British chain (Benoit 1895 B)",
													"BenoitChain",
													"Chain_Benoit_1895_B",
													"",					// WKT Oracle
													"2.012 dam",		// WKT Geo Tools
													"",
													""},
	{   "BenoitLink", 9063, "British link (Benoit 1895 B)",
													"benoit_link",
													"BenoitLink",
													"Link_Benoit_1895_B",
													"Link (Benoit)",	// WKT Oracle
													"2.012 dam",		// WKT Geo Tools
													"",
													""},
	{  "SearsChain", 9042, "British chain (Sears 1922)",
//ADSK-HW October 2nd 2006 - fixing parameter that is empty to "Chain_Sears" from MapGuide
													"Chain_Sears",
													"",
													"Chain_Sears",
													"Chain (Sears)",	// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{  "SearsChain", 9042, "British chain (Sears 1922)",
													"British chain (Sears 1922)",
													"",
													"Chain_Sears",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{  "SearsLink", 9043, "British link (Sears 1922)",
													"British link (Sears 1922)",
													"",
													"Link_Sears",
													"Link (Sears)",		// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{  "SearsFoot", 9041, "British foot (Sears 1922)",
//ADSK-HW October 2nd 2006 - fixing parameter that is empty to "SearsFoot" from MapGuide
													"SearsFoot",
													"",
													"Foot_Sears",
													"",					// WKT Oracle
													"3.048 dm",			// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{  "SearsFoot", 9041, "British foot (Sears 1922)",
													"British foot (Sears 1922)",
													"",
													"Foot_Sears",
													"",					// WKT Oracle
													"3.048 dm",			// WKT Geo Tools
													"",
													""},
	{  "IndianYd37", 9085, "Indian yard (1937)",
//ADSK-HW October 2nd 2006 - fixing parameter that is empty to "IndianYd37" from MapGuide
													"IndianYd37",
													"",
													"Yard_Indian_1937",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{  "IndianYd62", 9086, "Indian yard (1962)",	"IndianYd62",
													"",
													"Yard_Indian_1962",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{  "IndianYd75", 9087, "Indian yard (1975)",	"IndianYd75",
													"",
													"Yard_Indian_1975",
													"Yard (Indian)",	// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{  "GermanMeter", 9031, "German legal metre",   "german_meter",
													"GermanMeter",
													"German_Meter",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
//ADSK-BD April 30, 2007 - Alternate spelling of unit
	{  "GermanMeter", 9031, "German legal metre",   "German legal metre",
													"German legal metre",
													"German legal metre",
													"German legal metre",					// WKT Oracle
													"German legal metre",					// WKT Geo Tools
													"",
													""},
//ADSK-NTO 11/16/2007 -- Added to support the recognition of LOCAL_CS systems.
	{   "Centimeter",     0,                  "",   "",
													"",
													"",
													"Centimeter",			// WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
	{   "Millimeter",     0,                  "",   "",
													"",
													"",
													"Millimeter",			// WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
	{          "Rod",     0,                  "",   "",
													"",
													"",
													"Rod",					// WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
	{      "ModAmFt",     0,                  "",   "",
													"",
													"",
													"Modified American Foot",	// WKT Oracle
													"",							// WKT Geo Tools
													"",
													""},
	{        "NautM",  9030,     "nautical mile",   "",
													"",
													"",
													"Nautical Mile",	    // WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
	{         "Inch",    0,                   "",   "",
													"",
													"",
													"Inch",					// WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
	{         "Mile",    0,                   "",   "",
													"",
													"",
													"Mile",					// WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
// Begin Angular units of measure.
	{     "Radian", 9101, "radian",                 "Radian",
													"Radian",
													"Radian",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{     "Degree", 9102, "degree",                 "Degree",
													"Degree",
													"Degree",
													"Decimal Degree",	// WKT Oracle
													"degree of angle",	// WKT Geo Tools
													"",
													""},
//ADSK-HW October 2nd 2006 - adding 2 new entries from MapGuide because degree is often found with different spellings
	{     "Degree", 9102, "degree",                 "Degrees",
													"Degrees",
													"Degrees",
													"Degrees",	        // WKT Oracle
													"Degrees",	        // WKT Geo Tools
													"",
													""},
	{     "Degree", 9102, "degree",                 "degree of angle",
													"degree of angle",
													"degree of angle",
													"degree of angle",	        // WKT Oracle
													"degree of angle",	        // WKT Geo Tools
													"",
													""},
	{       "Grad", 9105, "grad",                   "Grad",
													"Grad",
													"Grad",
													"",					// WKT Oracle
													"0.016 rad",		// WKT Geo Tools
													"",
													""},
	{      "Grade", 9105, "grad",                   "Grad",
													"Grad",
													"Grad",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
											 		""},
	{       "Grad", 9106, "gon",                    "Gon",
													"Gon",
													"Gon",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	// The following entries are simply to keep the flavor detector working, we don't
	// really support them.
	{ "50kilometers",    0,    "",                  "",
													"",
													"50_Kilometers",	// WKT ESRI
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{ "150Kilometers",    0,    "",                 "",
													"",
													"150_Kilometers",	// WKT ESRI
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{        "Fathom",    0,    "",                 "",
													"",
													"",					// WKT ESRI
													"Fathom",			// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""},
	{      "NautM-UK",    0,    "",                 "",
													"",
													"",						// WKT ESRI
													"Nautical Mile (UK)",	// WKT Oracle
													"",						// WKT Geo Tools
													"",
													""},
	// The following entry terminates the table.
	{           "",    0, "",	                    "",
													"",
													"",
													"",					// WKT Oracle
													"",					// WKT Geo Tools
													"",
													""}
};

extern "C" void UtilityFromC1 ()
{
	std::wofstream oStrm1 (L"ProjectionKeyNameMap.csv",std::ios_base::out | std::ios_base::trunc);
	if (oStrm1.is_open ())
	{
		csWriteProjectionCsv (oStrm1);
		oStrm1.close ();
	}
	std::wofstream oStrm2 (L"ParameterKeyNameMap.csv",std::ios_base::out | std::ios_base::trunc);
	if (oStrm2.is_open ())
	{
		csWriteParameterCsv (oStrm2);
		oStrm2.close ();
	}
	std::wofstream oStrm3 (L"LinearUnitKeyNameMap.csv",std::ios_base::out | std::ios_base::trunc);
	if (oStrm3.is_open ())
	{
		csWriteLinearUnitCsv (oStrm3);
		oStrm3.close ();
	}
	std::wofstream oStrm4 (L"AngularUnitKeyNameMap.csv",std::ios_base::out | std::ios_base::trunc);
	if (oStrm4.is_open ())
	{
		csWriteAngularUnitCsv (oStrm4);
		oStrm4.close ();
	}
}
void csWriteProjectionCsv (std::wostream& oStrm)
{
	struct cs_Prjtab_ *pp;
	wchar_t msiPrjName [128];

	TcsKeyNameMapFile::WriteCsvFileHeader (oStrm);
	struct csWktPrjNameMap_* tblPtr;
	
	for (tblPtr = csWktPrjNameMap;tblPtr->CsMapCode != cs_PRJCOD_END;++tblPtr)
	{
		msiPrjName [0] = L'\0';
		for (pp = cs_Prjtab;*pp->key_nm != '\0';pp++)
		{
			if (pp->code == tblPtr->CsMapCode) break;
		}
		if (*pp->key_nm != '\0')
		{
			mbstowcs (msiPrjName,pp->key_nm,sizeof (msiPrjName));
		}

		oStrm << L"0" << L','						// InternalId
		      << tblPtr->EpsgNbr << L','
		      << L','								// ESRI Number
		      << L','								// Oracle Number
		      << L','								// MapInfo Number
		      << L','								// OEM Number
		      << L','								// Oracle9 Number
		      << tblPtr->CsMapCode << L','			// CS-Map Number
		      << msiPrjName << L','					// CS-Map Name
		      << L','								// Cartomatix Name
		      << L','								// Autodesk Name
		      << tblPtr->EpsgName << L','			// EPSG Name
		      << tblPtr->WktEsriName << L','		// ESRI Name
		      << tblPtr->WktOracleName << L','		// Oracle Name
		      << L','								// MapInfo Name
		      << L','								// OEM Name
		      << tblPtr->WktGeoToolsName << L','	// GeoTools Name
		      << tblPtr->WktOgcName << L','			// OCR Name
		      << L','								// OGC Name
		      << tblPtr->WktGeoTiffName << L','		// GeoTiff Name
		      << L','								// Oracle9 Name
		      << L','								// Conversion Name
		      << L','								// Conversion Comment
		      << L','								// Maintenance Comment
		      << L','								// Quality
		      << L','								// UserFlag
		   // <<  									// RfcnQualifier
		      << std::endl;
	}
}

void csWriteParameterCsv (std::wostream& oStrm)
{
	unsigned short msiPrmCode;
	wchar_t msiPrmName [128];

	TcsKeyNameMapFile::WriteCsvFileHeader (oStrm);
	struct csWktPrmNameMap_* tblPtr;
	
	for (tblPtr = csWktPrmNameMap;tblPtr->PrmCode != csWktPrmUnknown;++tblPtr)
	{
		msiPrmName [0] = L'\0';
		msiPrmCode = tblPtr->MsiParmCode;
		mbstowcs (msiPrmName,csPrjprm [msiPrmCode].label,sizeof (msiPrmName));
		
		oStrm << L"0" << L','						// InternalId
		      << tblPtr->EpsgNbr << L','
		      << L','								// ESRI Number
		      << L','								// Oracle Number
		      << L','								// MapInfo Number
		      << L','								// OEM Number
		      << L','								// Oracle9 Number
		      << tblPtr->MsiParmCode << L','		// CS-Map Number
		      << msiPrmName << L','					// CS-Map Name
		      << L','								// Cartomatix Name
		      << L','								// Autodesk Name
		      << tblPtr->EpsgName << L','			// EPSG Name
		      << tblPtr->WktEsriName << L','		// ESRI Name
		      << tblPtr->WktOracleName << L','		// Oracle Name
		      << L','								// MapInfo Name
		      << L','								// OEM Name
		      << tblPtr->WktGeoToolsName << L','	// GeoTools Name
		      << tblPtr->WktOgcName << L','			// OCR Name
		      << L','								// OGC Name
		      << tblPtr->WktGeoTiffName << L','		// GeoTiff Name
		      << L','								// Oracle9 Name
		      << L','								// Conversion Name
		      << L','								// Conversion Comment
		      << L','								// Maintenance Comment
		      << L','								// Quality
		      << L','								// UserFlag
		   // <<  									// RfcnQualifier
		      << std::endl;
	}
}
void csWriteLinearUnitCsv (std::wostream& oStrm)
{
	TcsKeyNameMapFile::WriteCsvFileHeader (oStrm);
	struct csWktUnitNameMap_* tblPtr;

	for (tblPtr = csWktUnitNameMap;tblPtr->MsiName [0] != '\0';++tblPtr)
	{
		if (tblPtr->EpsgNbr >= 9100)
		{
			// Ignore angular units.
			continue;
		}
		oStrm << L"0" << L','						// InternalId
		      << tblPtr->EpsgNbr << L','
		      << L','								// ESRI Number
		      << L','								// Oracle Number
		      << L','								// MapInfo Number
		      << L','								// OEM Number
		      << L','								// Oracle9 Number
		      << L','								// CS-Map Number
		      << tblPtr->MsiName << L','			// CS-Map Name
		      << L','								// Cartomatix Name
		      << tblPtr->MsiName << L','			// Autodesk Name
		      << tblPtr->EpsgName << L','			// EPSG Name
		      << tblPtr->WktEsriName << L','		// ESRI Name
		      << tblPtr->WktOracleName << L','		// Oracle Name
		      << L','								// MapInfo Name
		      << L','								// OEM Name
		      << tblPtr->WktGeoToolsName << L','	// GeoTools Name
		      << tblPtr->WktOgcName << L','			// OCR Name
		      << L','								// OGC Name
		      << tblPtr->WktGeoTiffName << L','		// GeoTiff Name
		      << L','								// Oracle9 Name
		      << L','								// Conversion Name
		      << L','								// Conversion Comment
		      << L','								// Maintenance Comment
		      << L','								// Quality
		      << L','								// UserFlag
		   // <<  									// RfcnQualifier
		      << std::endl;
	}
}
void csWriteAngularUnitCsv (std::wostream& oStrm)
{
	TcsKeyNameMapFile::WriteCsvFileHeader (oStrm);
	struct csWktUnitNameMap_* tblPtr;

	for (tblPtr = csWktUnitNameMap;tblPtr->MsiName [0] != '\0';++tblPtr)
	{
		if (tblPtr->EpsgNbr < 9100)
		{
			// Ignore linear units.
			continue;
		}
		oStrm << L"0" << L','						// InternalId
		      << tblPtr->EpsgNbr << L','
		      << L','								// ESRI Number
		      << L','								// Oracle Number
		      << L','								// MapInfo Number
		      << L','								// OEM Number
		      << L','								// Oracle9 Number
		      << L','								// CS-Map Number
		      << tblPtr->MsiName << L','			// CS-Map Name
		      << L','								// Cartomatix Name
		      << tblPtr->MsiName << L','			// Autodesk Name
		      << tblPtr->EpsgName << L','			// EPSG Name
		      << tblPtr->WktEsriName << L','		// ESRI Name
		      << tblPtr->WktOracleName << L','		// Oracle Name
		      << L','								// MapInfo Name
		      << L','								// OEM Name
		      << tblPtr->WktGeoToolsName << L','	// GeoTools Name
		      << tblPtr->WktOgcName << L','			// OCR Name
		      << L','								// OGC Name
		      << tblPtr->WktGeoTiffName << L','		// GeoTiff Name
		      << L','								// Oracle9 Name
		      << L','								// Conversion Name
		      << L','								// Conversion Comment
		      << L','								// Maintenance Comment
		      << L','								// Quality
		      << L','								// UserFlag
		   // <<  									// RfcnQualifier
		      << std::endl;
	}
}
