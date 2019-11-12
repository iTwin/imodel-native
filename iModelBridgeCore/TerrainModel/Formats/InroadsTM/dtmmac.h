//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------*/
/* dtmmac.h                                            aec    07-Sep-1990     */
/*----------------------------------------------------------------------------*/
/* Macros used with the dtm stuff.                                            */
/*----------------------------------------------------------------------------*/

#pragma once

#define DTMPOINTTODPOINT(zs,p,q) { (q) = (p)[0].cor; }

#define DTMDPOINTTOPOINT(zs,p,q) { (q)[0].cor = (p); }

#define DTMTINTODPOINT(zs,p,q) { (q)[0] = (p)->p1->cor;  (q)[1] = (p)->p2->cor;  (q)[2] = (p)->p3->cor; }
