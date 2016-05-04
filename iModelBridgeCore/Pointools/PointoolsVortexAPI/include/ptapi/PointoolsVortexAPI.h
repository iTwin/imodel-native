// ***************************************************************
//  Pointools Vortex API   version:  1.5.1.1   ·  date July 2010
//  -------------------------------------------------------------
//  Header file for Pointools Vortex API
//  For Build Use Only. Clients should use VortexAPI.h
//  -------------------------------------------------------------
//  Copyright (C) Pointools Ltd 2007-09 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************
#pragma once

#include <Vortex/VortexAPI.h>

/*context */ 
#define PT_GLOBAL_CONTEXT				0x01
#define PT_SCENE_CONTEXT				0x02
#define PT_CLOUD_CONTEXT				0x03
#define	PT_VIEWPORT_CONTEXT				0x04

/* imaging */ 
#define PT_IMAGE_TYPE_COLOUR		0x01
#define PT_IMAGE_TYPE_NORMAL		0x02
#define PT_IMAGE_TYPE_DEPTH			0x03
#define PT_IMAGE_TYPE_BUMP			0X04


/* fitting */ 
#define PT_FIT_MODE_USE_SELECTED	0x01
#define PT_FIT_MODE_USE_INPUT		0x02

#define PTAPI	STDCALL_ATTRIBUTE


/* view parameters - these operate in current viewport */ 
PTbool	PTAPI ptReadViewFromDX( void );

/* draw */ 
PTvoid	PTAPI ptDrawDX( void );
PTvoid	PTAPI ptDrawCB( void );
PTvoid	PTAPI ptDrawSceneDX( PThandle scene, PTbool dynamic );
PTvoid	PTAPI ptDrawSceneCB( PThandle scene, PTbool dynamic );

/* query : Warning Partial implementation only may assert */ 
PThandle PTAPI ptCreatePlaneQuery( PTdouble planeX, PTdouble planeY, PTdouble planeZ, PTdouble planeK, PTdouble thickness );
PThandle PTAPI ptCreatePolygonQuery( PTint numVertices, PTdouble *vertices, PTdouble thickness );

/* image generation : Warning not implemented, will assert */ 
PThandle	PTAPI ptCreateImageBuffer( PTint width, PTint height, PTubyte *buffer );
PTbool		PTAPI ptBindImageBuffer( PThandle );
PTbool		PTAPI ptGeneratePolygonImage( PTdouble *fencePoints, PTuint numPoints, PTdouble thickness );
PTbool		PTAPI ptGenerateViewportImage( void );
PTuint		PTAPI ptGetImageBufferHBITMAP( PThandle image );
PTvoid		PTAPI ptDeleteImage( PThandle image );


/* unit tests */ 
PTbool	PTAPI _ptUnitTests( PTenum test );
PTres	PTAPI _ptDiagnostic( PTvoid *data );
