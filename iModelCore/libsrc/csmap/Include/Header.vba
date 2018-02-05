' Copyright (c) 2008, Autodesk, Inc.
' All rights reserved.
'
' Redistribution and use in source and binary forms, with or without
' modification, are permitted provided that the following conditions are met:
'    ' Redistributions of source code must retain the above copyright
'      notice, this list of conditions and the following disclaimer.
'    ' Redistributions in binary form must reproduce the above copyright
'      notice, this list of conditions and the following disclaimer in the
'      documentation and/or other materials provided with the distribution.
'    ' Neither the name of the Autodesk, Inc. nor the names of its
'      contributors may be used to endorse or promote products derived
'      from this software without specific prior written permission.
'
' THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
' EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
' WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
' DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
' ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
' DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
' SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
' CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
' OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
' OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

' Note that functions which return null terminated 'C' strings in the C
' and C++ environments have a 'Vb' alternative.  That is, a function with
' the same name and signature with 'Vb' tacked on to the end of the name.
' The 'Vb' alternative functions perform exactly as the normal CS-MAP
' equivalent, except: a) the result is not null terminated, and b) the
' returned result is left justified with space fill to fill out the
' entire size of the provided String variable.  Usage as demonstrated
' by the following works reliably, without memory leaks,  and is possible
' without creating dependencies on extraneous DLL's and or static libraries.
'
'    Dim crsError As String
'    Dim csmapBufr As String *128
'    .
'    .
'    CS_errmsgVb (csmapBufr,Len(csmapBufr));
'    crsError = Trim (csmapBufr);
'    .
'    .

' Name Mapper enumerations
Public Enum EcsMapObjType As Integer
	csMapNone                     '   0 
	csMapFlavorName,              '   1
	csMapParameterKeyName,        '   2
	csMapProjectionKeyName,       '   3
	csMapGeodeticOpMthKeyName,    '   4
	csMapVerticalOpMthKeyName,    '   5
	csMapLinearUnitKeyName,       '   6
	csMapAngularUnitKeyName,      '   7
	csMapPrimeMeridianKeyName,    '   8
	csMapEllipsoidKeyName,        '   9
	csMapGeodeticXfrmKeyName,     '  10
	csMapVerticalXfrmKeyName,     '  11
	csMapDatumKeyName,            '  12
	csMapVerticalDatumKeyName,    '  13
	csMapGeoidKeyName,            '  14
	csMapGeographicCSysKeyName,   '  15
	csMapProjectedCSysKeyName,    '  16
	csMapGeographic3DKeyName,     '  17
	csMapUnknown,                 '  18
	csMapUnitKeyName,             '  19 -> Search for LinearUnit unit first, then AngularUnit 
	csMapProjGeoCSys              '  10 -> Search for Projected first, then Geographic
End Enum

Public Enum EcsNameFlavor As Integer
	csMapFlvrNone      =  0,        ' 0x00000000
	csMapFlvrEpsg      =  1,        ' 0x00000001
	csMapFlvrEsri      =  2,        ' 0x00000002
	csMapFlvrOracle    =  3,        ' 0x00000004
	csMapFlvrAutodesk  =  4,        ' 0x00000008
	csMapFlvrBentley   =  5,        ' 0x00000010
	csMapFlvrSafe      =  6,        ' 0x00000020
	csMapFlvrMapInfo   =  7,        ' 0x00000040
	csMapFlvrCtmx      =  8,        ' 0x00000080
	csMapFlvrCsMap     =  9,        ' 0x00000100
	csMapFlvrOGC       = 10,        ' 0x00000200
	csMapFlvrOCR       = 11,        ' 0x00000400
	csMapFlvrGeoTiff   = 12,        ' 0x00000800
	csMapFlvrGeoTools  = 13,        ' 0x00001000
	csMapFlvrOracle9   = 14,        ' 0x00002000
	csMapFlvrIBM       = 15,        ' 0x00004000
	csMapFlvrOem       = 16,        ' 0x00008000
	csMapFlvrAnon01    = 17,        ' Reserved
	csMapFlvrAnon02    = 18,        ' Reserved
	csMapFlvrAnon03    = 19,        ' Reserved
	csMapFlvrAnon04    = 20,        ' Reserved
	csMapFlvrAnon05    = 21,        ' Reserved
	csMapFlvrAnon06    = 22,        ' Reserved
	csMapFlvrAnon07    = 23,        ' Reserved
	csMapFlvrAnon08    = 24,        ' Reserved
	csMapFlvrAnon09    = 25,        ' Reserved
	csMapFlvrAnon10    = 26,        ' Reserved
	csMapFlvrAnon11    = 27,        ' Reserved
	csMapFlvrAnon12    = 28,        ' Reserved
	csMapFlvrAnon13    = 29,        ' Reserved
	csMapFlvrUser      = 30,        ' Reserved
	csMapFlvrLegacy    = 31,        ' Reserved
	csMapFlvrUnknown = 32
End Enum
' Name Mapper status return values
Const csNmMaprSt_Ok As Integer      = 0   ' Normal completion
Const csNmMaprSt_NoName As Integer  = 1   ' Source entry was found, no name defined with target flavor
Const csNmMaprSt_NoNbr As Integer   = 2   ' Source flavored entry was found, no number defined with target flavor
Const csNmMaprSt_NoMatch As Integer = 4   ' Source entry could not defined in Name Mapper
Const csNmMaprSt_NoMap As Integer  = -1   ' Name mapper initialization failed, use CS_errmsg to get reason

' ******************************************************************************
' Available functions.  Note: all of these modules are compiled with the __stdcall
' qualifier making them suitable for use by Visual Basic & Visual Basic for Applications.
' ******************************************************************************
Private Declare Function CS_altdr Lib "CsMapDll" (ByVal new_dir As String) As Integer
Private Declare Function CS_atof Lib "CsMapDll" (ByRef result As Double, ByVal value As String) As Long
Private Declare Function CS_azsphr Lib "CsMapDll" (ByRef ll_1 As Double, ByRef ll_2 As Double) As Double
Private Declare Function CS_cnvrg Lib "CsMapDll" (ByVal crd_sys As String, ByRef xycoord As Double) As Double
Private Declare Function CS_cnvrt Lib "CsMapDll" (ByVal src_cs As String, ByVal dest_cs As String, ByRef xycoord As Double) As Integer
Private Declare Function CS_csEnumVb Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer) As Integer
Private Declare Function CS_csEnumByGroup Lib "CsMapDll" (ByVal index As Integer, ByVal grp_name As String, ByRef cs_descr As CsDescr) As Integer
Private Declare Function CS_csGrpEnum Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer, ByVal grp_dscr As String, ByVal dscr_sz As Integer) As Integer
Private Declare Sub CS_csfnm Lib "CsMapDll" (ByVal my_bufr As String)
Private Declare Function CS_csIsValid Lib "CsMapDll" (ByVal key_name As String) As Integer
Private Declare Function CS_csRangeEnumSetup Lib "CsMapDll" (ByVal longitude As DOuble,ByVal latitude As Double) As Integer
Private Declare Function CS_csRangeEnumVb Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer) As Integer
Private Declare Function CS_dtEnumVb Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer) As Integer
Private Declare Sub CS_dtfnm Lib "CsMapDll" (ByVal my_bufr As String)
Private Declare Function CS_dtIsValid Lib "CsMapDll" (ByVal key_name As String) As Integer
Private Declare Function CS_elEnumVb Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer) As Integer
Private Declare Sub CS_elfnm Lib "CsMapDll" (ByVal my_bufr As String)
Private Declare Function CS_elIsValid Lib "CsMapDll" (ByVal key_name As String) As Integer
Private Declare Sub CS_errmsgVb Lib "CsMapDll" (ByVal my_bufr As String, ByVal bufr_size As Integer)
Private Declare Sub CS_fast Lib "CsMapDll" (ByVal fast As Integer)
Private Declare Function CS_ftoaVb Lib "CsMapDll" (ByVal buffer As String, ByVal size As Integer, ByVal value As Double, ByVal format As Long) As Long
Private Declare Sub CS_geoidCls Lib "CsMapDll" ()
Private Declare Function CS_geoidHgt Lib "CsMapDll" (ByRef lngLat As Double, ByRef geoidHeight As Double) As Integer
Private Declare Function CS_getDataDirectoryVb Lib "CsMapDll" (ByVal data_dir As String, ByVal dir_sz As Integer) As Integer
Private Declare Function CS_isgeo Lib "CsMapDll" (ByVal crd_sys As String) As Integer
Private Declare Function CS_llazdd Lib "CsMapDll" (ByVal e_rad As Double, ByVal e_sq As Double, ByRef ll_from As Double, ByRef ll_to As Double, ByRef dd As Double) As Double
Private Declare Function CS_prchk Lib "CsMapDll" (ByVal protect As Integer) As Integer
Private Declare Function CS_prjEnum LibVb "CsMapDll" (ByVal index As Integer, ByRef flags As Long, ByVal key_name As String, ByVal name_sz As Integer, ByVal prj_dscr As String, ByVal dscr_sz As Integer) As Integer
Private Declare Sub CS_recvr Lib "CsMapDll" ()
Private Declare Function CS_scale Lib "CsMapDll" (ByVal crd_sys As String, ByRef xycoord As Double) As Double
Private Declare Function CS_scalh Lib "CsMapDll" (ByVal crd_sys As String, ByRef xycoord As Double) As Double
Private Declare Function CS_scalk Lib "CsMapDll" (ByVal crd_sys As String, ByRef xycoord As Double) As Double
Private Declare Function CS_unEnumVb Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer) As Integer
Private Declare Function CS_unEnumPluralVb Lib "CsMapDll" (ByVal index As Integer, ByVal key_name As String, ByVal name_sz As Integer) As Integer
Private Declare Function CS_unitlu Lib "CsMapDll" (ByVal u_type As Integer, ByVal unit_nm As String) As Double

Private Declare Function CS_mgrsSetUp Lib "CsMapDll" (ByVal ellipsoid As String, ByVal bessel As Integer) As Integer
Private Declare Function CS_mgrsFromLlVb Lib "CsMapDll" (ByVal mgrs As String, ByRef xycoord As Double, ByVal precision As Integer) As Integer
Private Declare Function CS_llFromMgrs Lib "CsMapDll" (ByRef latLng As Double, ByVal mgrs As String) As Integer

Private Declare Function CS_geoctrSetUp Lib "CsMapDll" (ByVal ellipsoid As String) As Integer
Private Declare Function CS_geoctrGetLlh Lib "CsMapDll" (ByRef llh As Double, ByRef xyz As Double) As Integer
Private Declare Function CS_geoctrGetXyz Lib "CsMapDll" (ByRef xyz As Double, ByRef llh As Double) As Integer

Private Declare Function CS_getDatumOfVb Lib "CsMapDll" (ByVal csKeyName As String, ByVal datumName As String, ByVal size As Integer) As Integer
Private Declare Function CS_getDescriptionOfVb Lib "CsMapDll" (ByVal csKeyName As String, ByVal description As String, ByVal size As Integer) As Integer
Private Declare Function CS_getEllipsoidOfVb Lib "CsMapDll" (ByVal csKeyName As String, ByVal ellipsoidName As String, ByVal size As Integer) As Integer
Private Declare Function CS_getReferenceOfVb Lib "CsMapDll" (ByVal csKeyName As String, ByVal reference As String, ByVal size As Integer) As Integer
Private Declare Function CS_getSourceOfVb Lib "CsMapDll" (ByVal csKeyName As String, ByVal source As String, ByVal size As Integer) As Integer
Private Declare Function CS_getUnitsOfVb Lib "CsMapDll" (ByVal csKeyName As String, ByVal unitName As String, ByVal size As Integer) As Integer
Private Declare Function CS_getElValues Lib "CsMapDll" (ByVal elKeyName As String, ByRef eRadius As Double, ByRef eSquared As Double) As Integer

Private Declare Function CS_mapNameToIdVb Lib "CsMapDll" (ByVal type As EcsMapObjType, ByVal type As EcsNameFlavor, ByVal srcFlavor As EcsNameFlavor, ByVal srcName As String) As Integer
Private Declare Function CS_mapIdToIdVb Lib "CsMapDll" (ByVal type As EcsMapObjType, ByVal type As EcsNameFlavor, ByVal srcFlavor As EcsNameFlavor, ByVal srcId As Integer) As Integer
Private Declare Function CS_mapIdToNameVb Lib "CsMapDll" (ByVal type As EcsMapObjType, ByVal trgName As String, ByVal trgSize As Integer, ByVal type As EcsNameFlavor, ByVal srcFlavor As EcsNameFlavor, ByVal srcId As Integer) As Integer
Private Declare Function CS_mapNameToNameVb Lib "CsMapDll" (ByVal type As EcsMapObjType, ByVal trgName As String, ByVal trgSize As Integer, ByVal type As EcsNameFlavor, ByVal srcFlavor As EcsNameFlavor, ByVal srcName As String) As Integer
'
' Constants for use with CS_atof and CS_ftoaVb
'
Const cs_ATOF_PRCMSK As Long = &H1F&              ' precision mask (
Const cs_ATOF_VALLNG As Long = &H20&              ' value is valid longitude
Const cs_ATOF_VALLAT As Long = &H40&              ' value is valid latitude
Const cs_ATOF_MINSEC As Long = &H80&              ' minutes and seconds
Const cs_ATOF_MINUTE As Long = &H100&             ' minutes
Const cs_ATOF_FRACTN As Long = &H200&             ' standard decimal mode
Const cs_ATOF_EXPNT As Long = &H400&              ' scientific notation
Const cs_ATOF_OVRFLW As Long = &H800&             ' value can;t fit in buffer provided in format requested
Const cs_ATOF_COMMA As Long = &H1000&             ' insert thousands separator
Const cs_ATOF_DIRCHR As Long = &H2000&            ' insert direction characters (as opposed to sign character)
Const cs_ATOF_XEAST As Long = &H4000&             ' use e/w for directional characters
Const cs_ATOF_MINSEC0 As Long = 32768             ' include leading zero on minutes and seconds; Visual Basic won't take this one any other way
Const cs_ATOF_DEG0 As Long = &H10000              ' include leading zero on degrees
Const cs_ATOF_0BLNK As Long = &H20000             ' blank/null string is zero, zero produces null string.
Const cs_ATOF_FORCE3 As Long = &H40000            ' force three digits for degrees
Const cs_ATOF_RATIO As Long = &H80000             ' scale factor as a ratio
Const cs_ATOF_SECS60 As Long = &H100000           ' seconds value is greater than 60
Const cs_ATOF_MINS60 As Long = &H200000           ' minutes value is greater than 60
Const cs_ATOF_MLTPNT As Long = &H400000           ' multiple decimal point characters
Const cs_ATOF_MLTSGN As Long = &H800000           ' multiple sign indications
Const cs_ATOF_ERRCMA As Long = &H1000000          ' thousand separator positioning error
Const cs_ATOF_FMTERR As Long = &H2000000          ' other formatting error
Const cs_ATOF_ERRFLG As Long = &H80000000         ' formatting produced an error

Const cs_ATOF_XXXDFLT As Long = cs_ATOF_COMMA + 3
Const cs_ATOF_YYYDFLT As Long = cs_ATOF_COMMA + 3
Const cs_ATOF_LNGDFLT As Long = cs_ATOF_MINSEC + cs_ATOF_MINSEC0 + cs_ATOF_DIRCHR + cs_ATOF_XEAST + cs_ATOF_FORCE3 + 3
Const cs_ATOF_LATDFLT As Long = cs_ATOF_MINSEC + cs_ATOF_MINSEC0 + cs_ATOF_DIRCHR + 3
Const cs_ATOF_SCLDFLT As Long = 7
Const cs_ATOF_CNVDFLT As Long = cs_ATOF_MINSEC + cs_ATOF_MINSEC0 + cs_ATOF_DIRCHR + cs_ATOF_XEAST + 1

Const cs_UTYP_LEN As Integer = 76	' Unit type for linear  units, == Chr ('L')
Const cs_UTYP_ANG As Integer = 82	' Unit type for angular units, == Chr ('R')
'
' Unique code numbers assigned to each projection
'
Const cs_PRJCOD_UNITY As Integer    = 1   ' Unity pseudo projection
Const cs_PRJCOD_LMBRT As Integer    = 2   ' Lambert Conformal Conic 
Const cs_PRJCOD_TRMER As Integer    = 3   ' Transverse Mercator
Const cs_PRJCOD_ALBER As Integer    = 4   ' Albers Equal Area Conic
Const cs_PRJCOD_OBLQM As Integer    = 5   ' Oblique Mercator, Hotine
Const cs_PRJCOD_MRCAT As Integer    = 6   ' Mercator
Const cs_PRJCOD_AZMED As Integer    = 7   ' Azimuthal Equidistant
Const cs_PRJCOD_LMTAN As Integer    = 8   ' Lambert Tangential Conic
Const cs_PRJCOD_PLYCN As Integer    = 9   ' American Polyconic
Const cs_PRJCOD_MODPC As Integer    = 10  ' Modified Polyconic
Const cs_PRJCOD_AZMEA As Integer    = 11  ' Azimuthal Equal Area
Const cs_PRJCOD_EDCNC As Integer    = 12  ' Equidistant Conic, aka Simple Conic
Const cs_PRJCOD_MILLR As Integer    = 13  ' Miller Cylindrical
Const cs_PRJCOD_STERO As Integer    = 14  ' Stereographic
Const cs_PRJCOD_MSTRO As Integer    = 15  ' Modified Stereographic
Const cs_PRJCOD_NZLND As Integer    = 16  ' New Zealand National Grid System
Const cs_PRJCOD_SINUS As Integer    = 17  ' Sinusoidal
Const cs_PRJCOD_ORTHO As Integer    = 18  ' Orthographic
Const cs_PRJCOD_GNOMC As Integer    = 19  ' Gnomonic
Const cs_PRJCOD_EDCYL As Integer    = 20  ' Equidistant Cylindrical
Const cs_PRJCOD_VDGRN As Integer    = 21  ' Van der Grinten
Const cs_PRJCOD_CSINI As Integer    = 22  ' Cassini
Const cs_PRJCOD_ROBIN As Integer    = 23  ' Robinson
Const cs_PRJCOD_BONNE As Integer    = 24  ' Bonne
Const cs_PRJCOD_EKRT4 As Integer    = 25  ' Eckert IV
Const cs_PRJCOD_EKRT6 As Integer    = 26  ' Eckert VI
Const cs_PRJCOD_MOLWD As Integer    = 27  ' Molleweide
Const cs_PRJCOD_HMLSN As Integer    = 28  ' Goode Homolosine
Const cs_PRJCOD_NACYL As Integer    = 29  ' Normal Aspect, Equal Area Cylindrical
Const cs_PRJCOD_TACYL As Integer    = 30  ' Transverse Aspect, Equal Area Cylindrical
Const cs_PRJCOD_BPCNC As Integer    = 31  ' Bi Polar Conformal Conic
Const cs_PRJCOD_SWISS As Integer    = 32  ' Swiss Oblique Mercator (Rosenmund)
Const cs_PRJCOD_PSTRO As Integer    = 33  ' Polar Stereographic
Const cs_PRJCOD_OSTRO As Integer    = 34  ' Oblique Stereographic
Const cs_PRJCOD_SSTRO As Integer    = 35  ' Snyder's Oblique Stereographic
Const cs_PRJCOD_LM1SP As Integer    = 36  ' Single standard parallel variation of the Lambert Conformal Conic
Const cs_PRJCOD_LM2SP As Integer    = 37  ' Double standard parallel variation of the Lambert Conformal Conic
Const cs_PRJCOD_LMBLG As Integer    = 38  ' Belgian variation of the Lambert Conformal Conic Projection
Const cs_PRJCOD_WCCSL As Integer    = 39  ' Wisconsin County Coordinate System variation of the Lambert Conformal Conic
Const cs_PRJCOD_WCCST As Integer    = 40  ' Wisconsin County Coordinate System variation of the Transverse Mercator projection
Const cs_PRJCOD_MNDOTL As Integer   = 41  ' Minnesota Department of Transportation variation of the Lambert Conformal Conic
Const cs_PRJCOD_MNDOTT As Integer   = 42  ' Minnesota Department of Transportation variation of the Transverse Mercator projection
Const cs_PRJCOD_SOTRM As Integer    = 43  ' South Oriented variation of the Transverse Mercator Projection
Const cs_PRJCOD_UTM As Integer      = 44  ' The UTM direct variation of the Transverse Mercator projection
Const cs_PRJCOD_TRMRS As Integer    = 45  ' Transverse Mercator per J. P. Snyder
Const cs_PRJCOD_GAUSSK As Integer   = 46  ' Gauss-Kruger: Transverse Mercator without scale reduction parameter
Const cs_PRJCOD_KROVAK As Integer   = 47  ' Czech Krovak, with precise origin
Const cs_PRJCOD_KROVK1 As Integer   = 48  ' Obsolete, do not reuse. As IntegerRetained for compatibility with previous releases only: Czech Krovak, with rounded origin
Const cs_PRJCOD_MRCATK As Integer   = 49  ' Standard Mercator with a scale reduction factor instead of a standard parallel
Const cs_PRJCOD_OCCNC As Integer    = 50  ' Oblique conformal conic
Const cs_PRJCOD_KRVK95 As Integer   = 51  ' Czech Krovak, with precise origin, includes S-JTSK/95 adjustment
Const cs_PRJCOD_KRVK951 As Integer  = 52  ' Obsolete, do not reuse. As IntegerRetained for compatibility with previous releases only. Czech Krovak, with rounded origin, includes S-JTSK/95 adjustment
Const cs_PRJCOD_PSTROSL As Integer  = 53  ' Polar stereographic with standard latitude
Const cs_PRJCOD_TRMERAF As Integer  = 54  ' Transverse Mercator with affine post-processor
Const cs_PRJCOD_NERTH As Integer    = 55  ' Non-georeferenced coordinate system. As IntegerNamed Non-Earth by Map Info
Const cs_PRJCOD_OBQCYL As Integer   = 56  ' Oblique Cylindrical, a generalized version of the Swiss projection, specifically for Hungary
Const cs_PRJCOD_SYS34 As Integer    = 57  ' Combination of Transverse Mercator and a polynomial expansion used in Denmark
Const cs_PRJCOD_OSTN97 As Integer   = 58  ' The Transverse Mercator with specific parameters, with the OSTN97 grid shift tacked on. As IntegerThis is a combination of a projection and a datum shift.
Const cs_PRJCOD_AZEDE As Integer    = 59  ' Azimuthal Equi-Distant, Elevated ellipsoid
Const cs_PRJCOD_OSTN02 As Integer   = 60  ' The Transverse Mercator with specific parameters, with the OSTN02 grid shift tacked on. This is a combination of a projection and a datum shift
Const cs_PRJCOD_SYS34_99 As Integer = 61  ' Combination of Transverse Mercator and polynomial expansion used in Denmark. As IntegerPolynomials are of the 1999 vintage
Const cs_PRJCOD_TRMRKRG As Integer  = 62  ' Variation of the Transverse Mercator which uses the Kruger Formulation
Const cs_PRJCOD_WINKL As Integer    = 63  ' Winkel-Tripel projection
Const cs_PRJCOD_NRTHSRT As Integer  = 64  ' Nerth with scale and rotation
Const cs_PRJCOD_LMBRTAF As Integer  = 65  ' Lambert Conformal Conic with affine post-processor
Const cs_PRJCOD_SYS34_01 As Integer = 66  ' Combination of Transverse Mercator and polynomial expansion used in Denmark. As IntegerPolynomials are of the 1999 vintage, except for Bornholm, which are post 1999
Const cs_PRJCOD_EDCYLE As Integer   = 67  ' Equidistant Cylindrical, ellipsoid form supported. This variation replaces the original variation which only supported the spherical form of this projection
Const cs_PRJCOD_PCARREE As Integer  = 68  ' Plate Carree, standard form. As IntegerThis is _NOT_ the same as EPSG 9825 - Pseudo Plate Carree
Const cs_PRJCOD_MRCATPV As Integer  = 69  ' Psuedo Mercator, Popular Visualization
Const cs_PRJCOD_LMMICH As Integer   = 70  ' Lambert Conformal Conic, Michigan Variation

Const cs_PRJCOD_HOM1UV As Integer = 1281  ' Unrectified, single point form of Oblique Mercator
Const cs_PRJCOD_HOM1XY As Integer = 1282  ' Rectified, single point form of Oblique Mercator
Const cs_PRJCOD_HOM2UV As Integer = 1283  ' Unrectified, double point form of Oblique Mercator
Const cs_PRJCOD_HOM2XY As Integer = 1284  ' Rectified, double point form of Oblique Mercator
'
' Constants which provide meaning for the bits in a projection flag word.
' A projection flag word is an inclusive OR of all applicable bits.
'
Const cs_PRJFLG_SPHERE As Long = &H1&    ' Sphere supported.
Const cs_PRJFLG_ELLIPS As Long = &H2&    ' Ellipsoid supported. 

Const cs_PRJFLG_SCALK As Long = &H4&     ' Analytical K scale available
Const cs_PRJFLG_SCALH As Long = &H8&     ' Analytical H scale available
Const cs_PRJFLG_CNVRG As Long = &H10&    ' Analytical convergence available

Const cs_PRJFLG_CNFRM As Long = &H20&	   ' Conformal
Const cs_PRJFLG_EAREA As Long = &H40&	   ' Equal area
Const cs_PRJFLG_EDIST As Long = &H80&	   ' Equal distant, either h or k is always 1.
Const cs_PRJFLG_AZMTH As Long = &H100&   ' Azimuthal
Const cs_PRJFLG_GEOGR As Long = &H200&   ' Geographic coordinates
'
' Modifiers to surface type:
'
Const cs_PRJFLG_OBLQ As Long = &H400&      ' Oblique
Const cs_PRJFLG_TRNSV As Long = &H800&     ' Transverse
Const cs_PRJFLG_PSEUDO As Long = &H1000&   ' Psuedo
Const cs_PRJFLG_INTR As Long = &H2000&     ' Interruptible
'
' Surface Type: Projection is generally considered to be:
'
Const cs_PRJFLG_CYLND As Long = &H4000&    ' Cylindrical
Const cs_PRJFLG_CONIC As Long = 32768      ' Conic
Const cs_PRJFLG_FLAT As Long = &H10000     ' Azimuthal
Const cs_PRJFLG_OTHER As Long = &H20000    ' Other
'
' Special parameter handling
'
Const cs_PRJFLG_SCLRED As Long = &H2000000  ' Scale Reduction is supported.
Const cs_PRJFLG_ORGLAT As Long = &H4000000  ' Projection does not use an origin latitude parameter.
Const cs_PRJFLG_ORGLNG As Long = &H8000000  ' Projection does not use an origin longitude parameter.
