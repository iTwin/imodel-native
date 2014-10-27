/*
 * Copyright (c) 2008, Autodesk, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Autodesk, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Autodesk, Inc. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Autodesk, Inc. OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "cs_map.h"

char cs_NullXformName [64] = "NullTransformation";

struct cs_GxIndex_* cs_GxIndex = NULL;
unsigned cs_GxIndexCount = 0;

struct cs_XfrmTab_ cs_XfrmTab [] =
{
	//  Key Name    , initi  , check  , methodCode    , method flags    , epsg , description
    {         "NULL",CSnullxS,CSnullxN,CSnullxQ,cs_DTCMTH_NULLX,cs_XFRMFLGS_NULLX,   0UL,"Null Transformation, produces no change in coordinates"},
	{        "WGS72",CSwgs72S,CSwgs72N,CSwgs72Q,cs_DTCMTH_WGS72,cs_XFRMFLGS_WGS72,   0UL,"WGS72 Transformation ala DMA"},
	{   "MOLODENSKY",CSmolodS,CSmolodN,CSmolodQ,cs_DTCMTH_MOLOD,cs_XFRMFLGS_MOLOD,9604UL,"Molodensky Transformation"},
//	{ "ABMOLODENSKY",CSamoloS,CSamoloN,CSamoloQ,cs_DTCMTH_AMOLO,cs_XFRMFLGS_AMOLO,9605UL,"Abridged Molodensky Transformation"},
	{   "GEOCENTRIC",CSgeoctS,CSgeoctN,CSgeoctQ,cs_DTCMTH_GEOCT,cs_XFRMFLGS_GEOCT,9603UL,"Geocentric Translation Transformation"},
	{   "4PARAMETER",CSparm4S,CSparm4N,CSparm4Q,cs_DTCMTH_4PARM,cs_XFRMFLGS_4PARM,   0UL,"Four Parameter Transformation"},
	{   "6PARAMETER",CSparm6S,CSparm6N,CSparm6Q,cs_DTCMTH_6PARM,cs_XFRMFLGS_6PARM,   0UL,"Six Parameter Transformation"},
	{    "BURSAWOLF",CSbursaS,CSbursaN,CSbursaQ,cs_DTCMTH_BURSA,cs_XFRMFLGS_BURSA,9606UL,"Bursa Wolf Transformation (aka Position Vector)"},
	{   "BURSAFRAME",CSframeS,CSframeN,CSframeQ,cs_DTCMTH_FRAME,cs_XFRMFLGS_FRAME,9607UL,"Coordinate Frame Rotation Transformation"},
	{   "7PARAMETER",CSparm7S,CSparm7N,CSparm7Q,cs_DTCMTH_7PARM,cs_XFRMFLGS_7PARM,   0UL,"Seven Parameter Transformation"},
	{  "MOLOBADEKAS",CSbdkasS,CSbdkasN,CSbdkasQ,cs_DTCMTH_BDKAS,cs_XFRMFLGS_BDKAS,9636UL,"Molodensky Badekas Transformation"},
	{       "MULREG",CSmulrgS,CSmulrgN,CSmulrgQ,cs_DTCMTH_MULRG,cs_XFRMFLGS_MULRG,9802UL,"Multiple Regression Transformation ala DMA"},
//	{       "POLYNM",CSplynmS,CSplynmN,CSplynmQ,cs_DTCMTH_PLYNM,cs_XFRMFLGS_PLYNM,   0UL,"General Polynomial Transformation"},
	{  "GRID_INTERP",CSgridiS,CSgridiN,CSgridiQ,cs_DTCMTH_GFILE,cs_XFRMFLGS_GFILE,9654UL,"Grid File Interpolation"},
	{   "3PARAMETER",CSparm3S,CSparm3N,CSparm3Q,cs_DTCMTH_3PARM,cs_XFRMFLGS_3PARM,   0UL,"Three Parameter Transformation -- DEPRECATED"},
	{             "",NULL,    NULL,    NULL,    cs_DTCMTH_NONE, 0,                   0UL,"End of table marker"}
};

struct cs_GridFormatTab_ cs_GridFormatTab [] =
{
	//      Key Name,   initi,   check,     methodCode,     method flags,  epsg, description
	{         "NTv1",CScntv1S,CScntv1Q,cs_DTCFRMT_CNTv1,cs_FRMTFLGS_CNTv1,9614UL,"Canadian National Transformation, Version 1"},
	{         "NTv2",CScntv2S,CScntv2Q,cs_DTCFRMT_CNTv2,cs_FRMTFLGS_CNTv2,9615UL,"Canadian National Transformation, Version 2"},
	{       "NADCON",CSnadcnS,CSnadcnQ,cs_DTCFRMT_NADCN,cs_FRMTFLGS_NADCN,9613UL,"US NADCON (i.e. las/los pair)"},
	{        "FRGEO",CSfrnchS,CSfrnchQ,cs_DTCFRMT_FRNCH,cs_FRMTFLGS_FRNCH,9655UL,"French Geocentric Interpolation"},
	{        "JPPAR",CSjapanS,CSjapanQ,cs_DTCFRMT_JAPAN,cs_FRMTFLGS_JAPAN,   0UL,"Japanese Grid Mesh Interpolation (i.e. .par)"},
	{        "ATS77",CSats77S,CSats77Q,cs_DTCFRMT_ATS77,cs_FRMTFLGS_ATS77,9634UL,"Maritime Provinces Polynomial Interpolation"},
	{       "OSTN97",CSost97S,CSost97Q,cs_DTCFRMT_OST97,cs_FRMTFLGS_OST97,   0UL,"Ordnance Survey National Transformation ala 1997"},
	{       "OSTN02",CSost02S,CSost02Q,cs_DTCFRMT_OST02,cs_FRMTFLGS_OST02,9633UL,"Ordnance Survey National Transformation ala 2002"},
	{             "",NULL,    NULL,    cs_DTCFRMT_NONE, 0,                   0UL,"End of table marker"}
};

struct cs_PivotDatumTbl_ cs_PivotDatumTbl [] =
{
	{"WGS84",        0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   },
	{"",             0   }
};
