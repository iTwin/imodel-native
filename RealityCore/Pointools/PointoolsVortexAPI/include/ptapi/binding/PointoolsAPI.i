%module PointoolsAPI
%{
#include "PointoolsAPI.h"
%}
int		ptInitialize();

int		ptOpenPOD(const char *filepath);
int		ptBrowseAndOpenPOD();
void	ptClearAll();

/* draw */ 
void	ptDrawGL();
void	ptCaptureView();

/* bounds of data */ 
float	ptGetLXBound();
float	ptGetLYBound();
float	ptGetLZBound();

float	ptGetUXBound();
float	ptGetUYBound();
float	ptGetUZBound();

/* shader configuration */ 
void	ptEnableLighting();
void	ptDisableLighting();
int		ptIsLightingEnabled();
\
void	ptEnableIntensity();
void	ptDisableIntensity();
int		ptIsIntensityEnabled();

void	ptEnableRGB();
void	ptDisableRGB();
int		ptIsRGBEnabled();

void	ptEnablePlaneShader();
void	ptDisablePlaneShader();
int		ptIsPlaneShaderEnabled();

void	ptSetPlaneShaderOffset(float offset);
void	ptSetPlaneShaderDistance(float distance);
void	ptSetPlaneShaderVector(float x, float y, float z);

float	ptGetPlaneShaderOffset();
float	ptGetPlaneShaderDistance();
float	ptGetPlaneShaderVectorX();
float	ptGetPlaneShaderVectorY();
float	ptGetPlaneShaderVectorZ();

void	ptSetDynamicFrameRate(float fps);
float	ptGetDynamicFrameRate();

void	ptSetStaticOptimizer(float opt);
float	ptGetStaticOptimizer();

void	ptSetPointSize(float psize);
float	ptGetPointSize();

void	ptEnableBlending();
void	ptDisableBlending();
int		ptIsBlendingEnabled();

/* visibility */ 
void	ptEnableClipBox();
void	ptDisableClipBox();
int		ptIsClipBoxEnabled();

void	ptSetClipBox(float lx, float ly, float lz, float ux, float uy, float uz);
void	ptSetClipBoxLX(float v);
void	ptSetClipBoxLY(float v);
void	ptSetClipBoxLZ(float v);

void	ptSetClipBoxUX(float v);
void	ptSetClipBoxUY(float v);
void	ptSetClipBoxUZ(float v);

void	ptSetClipSectionThickness(float th);
void	ptSetClipSectionInX(float pos);
void	ptSetClipSectionInY(float pos);
void	ptSetClipSectionInZ(float pos);

void	ptNudgeClipSectionPos();
void	ptNudgeClipSectionNeg();

void	ptSetClipToBounds();

float	ptGetClipBoxLX();
float	ptGetClipBoxLY();
float	ptGetClipBoxLZ();
float	ptGetClipBoxUX();
float	ptGetClipBoxUY();
float	ptGetClipBoxUZ();

void	ptSetVisible(int cloud);
void	ptSetHidden(int cloud);
int		ptGetVisible(int cloud);

void	ptDisableDraw();
void	ptEnableDraw();
int		ptIsDrawEnabled();

/* interface */
/* interface */ 
void	ptShowInterface();

void	ptFlipMouseYCoords();
void	ptDontFlipMouseYCoords();

/* event notification */
int		ptMouseMove(int x, int y);

int		ptLButtonUp(int x, int y);
int		ptLButtonDown(int x, int y);

int		ptRButtonUp(int x, int y);
int		ptRButtonDown(int x, int y);

int		ptMButtonUp(int x, int y);
int		ptMButtonDown(int x, int y);
int		ptKeyDown(char key);

void	ptDrawEvent();

/* opengl interaction */ 
int		ptFindNearestPoint(int screenx, int screeny);
float	ptNearestPointX();
float	ptNearestPointY();
float	ptNearestPointZ();	

/* clipbox editing */ 
void	ptStartClipBoxEdit();
void	ptEndClipBoxEdit();





