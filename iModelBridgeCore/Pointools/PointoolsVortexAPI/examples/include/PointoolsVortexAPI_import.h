// ***************************************************************
//  Pointools Vortex API   version:  1.4   ·  date July 2009
//  -------------------------------------------------------------
//  Header file for Pointools Vortex API
//  For API Import only
//  -------------------------------------------------------------
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
// ***************************************************************
// 
// ***************************************************************
#ifndef POINTOOLS_API_IMPORT_HEADER
#define POINTOOLS_API_IMPORT_HEADER 1

typedef unsigned int	PTenum;
typedef bool			PTbool;
typedef int				PTint;
typedef int				PTres;
typedef unsigned int	PTuint;
typedef float			PTfloat;
typedef double			PTdouble;
typedef short			PTshort;
typedef unsigned short	PTushort;
typedef unsigned __int64	PTuint64;
typedef __int64				PTint64;
typedef void			PTvoid;
typedef PTuint			PThandle;
typedef unsigned char	PTubyte;

/* All strings are WCHAR, ie 2bytes per char */ 
#define PTstr wchar_t*

/* Client Server */
#define PT_CLIENT_SERVER_CACHE_NAME_MODE_GUID			1
#define PT_CLIENT_SERVER_CACHE_NAME_MODE_SERVER_PATH	2

/* Shader Enables */ 
#define PT_RGB_SHADER					0x01
#define PT_INTENSITY_SHADER				0x02
#define PT_BLENDING_SHADER				0x03
#define PT_PLANE_SHADER					0x04
#define PT_LIGHTING						0x05
#define PT_CLIPPING						0x06
#define PT_CHANNEL_RENDER				0x07

/* Display Enables */ 
#define PT_ADAPTIVE_POINT_SIZE			0X100
#define PT_FRONT_BIAS					0X101
#define PT_DELAYED_CHANNEL_LOAD			0X102

/* Shader Settings */ 
#define PT_PLANE_SHADER_DISTANCE		0x11	
#define PT_PLANE_SHADER_VECTOR			0x12
#define PT_PLANE_SHADER_OFFSET			0x13

#define PT_INTENSITY_SHADER_CONTRAST	0x14
#define PT_INTENSITY_SHADER_BRIGHTNESS	0x15

#define PT_RGB_SHADER_CONTRAST			0x16
#define PT_RGB_SHADER_BRIGHTNESS		0x17

#define PT_LIGHT_VECTOR					0x18
#define PT_LIGHT_ANGLE					0x19
#define PT_LIGHT_COLOUR					0x1a
#define PT_LIGHT_AMBIENT_COLOUR			0x1b
#define PT_LIGHT_DIFFUSE_COLOUR			0x1c
#define PT_LIGHT_SPECULAR_COLOUR		0x1d
#define PT_LIGHT_STRENGTH				0x1f
#define PT_LIGHT_AMBIENT_STRENGTH		0x20
#define PT_LIGHT_DIFFUSE_STRENGTH		0x21
#define PT_LIGHT_SPECULAR_STRENGTH		0x22

#define PT_INTENSITY_SHADER_RAMP		0x23
#define PT_PLANE_SHADER_RAMP			0x24

#define PT_MATERIAL_AMBIENT				0X25
#define PT_MATERIAL_DIFFUSE				0x26
#define PT_MATERIAL_SPECULAR			0x27
#define PT_MATERIAL_GLOSSINESS			0x28

#define PT_PLANE_SHADER_EDGE			0x29

/* texture edge */ 
#define PT_EDGE_REPEAT					0x00
#define PT_EDGE_CLAMP					0x01
#define PT_EDGE_BLACK					0x02
#define PT_EDGE_MIRROR					0x03

/* units */ 
#define PT_METERS						0x100
#define PT_DECIMETERS					0x101
#define PT_CENTIMETERS					0x102
#define PT_MILLIMETERS					0x103
#define PT_FEET							0x104
#define PT_FEET_US						0x106
#define PT_INCHES						0x105

/* draw modes */ 
#define PT_DRAW_MODE_STATIC				0x01
#define PT_DRAW_MODE_INTERACTIVE		0x02
#define PT_DRAW_MODE_DEFAULT			0x00
#define PT_DRAW_MODE_COMPATIBILITY		0x04

/* selection modes */ 
#define PT_SELECT						0x01		
#define PT_DESELECT						0x02

#define PT_MAX_VIEWPORTS		256

/* point attributes */ 
#define PT_HAS_INTENSITY		0x01
#define PT_HAS_RGB				0x02
#define PT_HAS_NORMAL			0x04
#define PT_HAS_FILTER			0x08
#define PT_HAS_CLASSIFICATION	0x10


/* ramps */ 
#define PT_INTENSITY_RAMP_TYPE	0x01
#define PT_PLANE_RAMP_TYPE		0x02

/* coordinate base */ 
#define PT_AUTO_BASE_DISABLED	0x0
#define PT_AUTO_BASE_CENTER		0x01
#define PT_AUTO_BASE_REDUCE		0x02
#define PT_AUTO_BASE_FIRST_ONLY 0x04

/* editing */ 
#define PT_EDIT_MODE_SELECT		0X01
#define PT_EDIT_MODE_UNSELECT	0X02
#define PT_EDIT_MODE_UNHIDE		0X03
#define PT_EDIT_MAX_LAYERS		6

/* editing mode */ 
#define PT_EDIT_WORK_ON_ALL			0x01
#define PT_EDIT_WORK_ON_VIEW		0x02
#define PT_EDIT_WORK_ON_PROPORTION	0x03

/* per point filter */ 
#define PT_EDIT_PNT_SELECTED		0x80
#define PT_EDIT_PNT_LYR1			0x01
#define PT_EDIT_PNT_LYR2			0x02
#define PT_EDIT_PNT_LYR3			0x03
#define PT_EDIT_PNT_LYR4			0x04
#define PT_EDIT_PNT_LYR5			0x05
#define PT_EDIT_PNT_LYR6			0x06
#define PT_EDIT_PNT_LYR7			0x07

/* query */ 
#define PT_QUERY_DENSITY_FULL			0x01
#define	PT_QUERY_DENSITY_VIEW			0X02
#define PT_QUERY_DENSITY_LIMIT			0X03
#define	PT_QUERY_DENSITY_VIEW_COMPLETE	0X04
#define PT_QUERY_DENSITY_SPATIAL		0x07

#define PT_QUERY_RGB_MODE_ACTUAL			0x04
#define PT_QUERY_RGB_MODE_SHADER			0x05
#define PT_QUERY_RGB_MODE_SHADER_NO_SELECT  0x06

/* tuning */ 
#define PT_LOADING_BIAS_SCREEN	0x01
#define PT_LOADING_BIAS_NEAR	0x02
#define PT_LOADING_BIAS_FAR		0x03
#define PT_LOADING_BIAS_POINT	0x04

/* eye perspective type */ 
#define PT_PROJ_PERSPECTIVE_GL		0x01
#define PT_PROJ_PERSPECTIVE_DX		0x02
#define PT_PROJ_PERSPECTIVE_BLINN	0x03

/* channel constants */ 
/* draw as */ 
#define PT_CHANNEL_AS_OFFSET	0x01
#define PT_CHANNEL_AS_RAMP		0x02

/* options */ 
#define PT_CHANNEL_OUT_OF_CORE	0X01

/* meta data */ 
#define PT_MAX_META_STR_LEN			1024

/* generic */ 
#define PT_TRUE	true
#define PT_FALSE false
#define PT_NULL 0

/* Viewport context types */ 
#define PT_GL_VIEWPORT				0X01
#define PT_DX_VIEWPORT				0X02
#define PT_SW_VIEWPORT				0X03

/* clipping options */
#define PT_CLIP_OUTSIDE				0x01
#define PT_CLIP_INSIDE				0x02

#ifndef PT_VORTEX_STATIC_LIB

/* initialization */ 
typedef PTbool	(__stdcall *PTINITIALIZE)(const PTubyte*);
typedef PTbool	(__stdcall *PTISINITIALIZED)(void);
typedef PTvoid	(__stdcall *PTSETWORKINGFOLDER)(const PTstr);
typedef const PTstr	(__stdcall *PTGETWORKINGFOLDER)(void);
typedef const	PTstr	(__stdcall *PTGETVERSIONSTRING)( void );
typedef PTvoid	(__stdcall *PTGETVERSIONNUM)( PTubyte *version );
typedef PTvoid (__stdcall *PTRELEASE)( void );


/* Client Server */
typedef PTbool 		(__stdcall *PTCREATEFAKEPOD)(PTstr originalServerFile, PTvoid *bentleyData, PTuint bentleyDataSize, PTstr fakeFile);
typedef PTuint 		(__stdcall *PTSETRELEASECLIENTSERVERBUFFERCALLBACK)(PTbool (*function)(PTvoid *buffer));
typedef PTuint 		(__stdcall *PTSETSERVERCALLBACK)(PTuint (*function)(PTvoid *dataSend, PTuint dataSendSize, PTvoid *extDataSend, PTuint extDataSendSize, PTvoid **dataReceive, PTuint *dataReceiveSize));
typedef PTuint 		(__stdcall *PTPROCESSSERVERREQUEST)(PTvoid *dataReceive, PTuint dataReceiveSize, PTvoid **dataSend, PTuint *dataSendSize);
typedef PTuint 		(__stdcall *PTPROCESSSERVERREQUESTCLIENTID)(PTvoid *dataReceive, PTuint dataReceiveSize, PTvoid **dataSend, PTuint *dataSendSize, PTuint64 *clientIDFirst64, PTuint64 *clientIDSecond64);
typedef PTuint 		(__stdcall *PTPROCESSSERVERREQUESTCLIENTID2)(PTvoid *dataReceive, PTuint dataReceiveSize, PTuint64 *clientIDFirst64, PTuint64 *clientIDSecond64, PTuint (*function)(PTvoid *dataSend, PTuint dataSendSize, PTuint64 clientIDFirst64, PTuint64 clientIDSecond64));
typedef PTbool 		(__stdcall *PTSETCLIENTSERVERLOGFILE)(PTstr logFile);
typedef PTbool		(__stdcall *PTSETCLIENTSERVERSENDRETRIES)(PTuint numRetries, PTuint milliseconds, PTuint millisecondsIncrement);
typedef void		(__stdcall *PTGETCLIENTSERVERSENDRETRIES)(PTuint *numRetries, PTuint *delayMilliseconds, PTuint *incrementMilliseconds);
typedef PTbool		(__stdcall *PTSETCLIENTSTREAMING)(PTuint min, PTuint max, PTuint refresh, PTdouble scalar);
typedef PTbool		(__stdcall *PTSERVERCLIENTLOST)(PTuint64 clientIDFirst64, PTuint64 clientIDSecond64);
typedef void		(__stdcall *PTGETSESSIONID)(PTuint64 &idFirst64, PTuint64 &idSecond64);



/* Client Server Caching */
typedef PTbool 		(__stdcall *PTSETCLIENTCACHEFOLDER)(PTstr path);
typedef const PTstr	(__stdcall *PTGETCLIENTCACHEFOLDER)(void);
typedef PTbool 		(__stdcall *PTENABLECLIENTSERVERCACHING)(PTbool enable);
typedef PTbool 		(__stdcall *PTGETCLIENTSERVERCACHINGENABLED)(void);
typedef PTbool 		(__stdcall *PTSETCLIENTSERVERCACHEDATASIZE)(PTuint size);
typedef PTuint 		(__stdcall *PTGETCLIENTSERVERCACHEDATASIZE)(void);
typedef PTbool 		(__stdcall *PTSETCLIENTCACHECOMPLETIONTHRESHOLD)(PTuint size);
typedef PTuint 		(__stdcall *PTGETCLIENTCACHECOMPLETIONTHRESHOLD)(void);
typedef PTbool		(__stdcall *PTSETCLIENTSERVERCACHENAMEMODE)(PTenum mode);
typedef PTenum		(__stdcall *PTGETCLIENTSERVERCACHENAMEMODE)(void);


/* opening data */ 
typedef PThandle (__stdcall *PTOPENPOD)(const PTstr);

typedef PThandle (__stdcall *PTOPENPODSTRUCTUREDSTORAGESTREAM)(const PTstr filepath, PTvoid *stream);
typedef PThandle (__stdcall *PTISOPEN)(const PTstr);
typedef PThandle (__stdcall *PTBROWSEANDOPENPOD)(void);


/* basic handle management */ 
typedef PThandle (__stdcall  *PTGETCLOUDHANDLEBYINDEX)(PThandle, PTuint);
typedef PTuint   (__stdcall  *PTGETNUMCLOUDSINSCENE)(PThandle);

/* management */ 
typedef PTint	(__stdcall *PTNUMSCENES)(void);
typedef PTint	(__stdcall *PTGETSCENEHANDLES)(PThandle *);
typedef PTres	(__stdcall *PTSCENEINFO)(PThandle, PTstr, PTint &, PTuint &, PTuint &, PTbool &, PTbool &);
typedef PTres	(__stdcall *PTCLOUDINFO)(PThandle, PTstr, PTuint &, PTuint &, PTbool &);
typedef const PTstr		(__stdcall *PTSCENEFILE)( PThandle );

typedef PTres	(__stdcall *PTLAYERBOUNDS)( PTuint layer, PTfloat *lower3, PTfloat *upper3, bool approx_fast );
typedef PTres	(__stdcall *PTLAYERBOUNDSD)( PTuint layer, PTdouble *lower3, PTdouble *upper3, bool approx_fast );

typedef PTres	(__stdcall *PTSCENEBOUNDS)(PThandle, PTfloat*, PTfloat*);
typedef PTres	(__stdcall *PTSCENEBOUNDSD)(PThandle, PTdouble*, PTdouble*);
typedef PTres	(__stdcall *PTCLOUDBOUNDS)(PThandle, PTfloat*, PTfloat*);
typedef PTres	(__stdcall *PTCLOUDBOUNDSD)(PThandle, PTdouble*, PTdouble*);

typedef PTres	(__stdcall *PTSHOWSCENE)(PThandle, PTbool);
typedef PTres	(__stdcall *PTSHOWCLOUD)(PThandle, PTbool);

typedef PTbool	(__stdcall *PTISSCENEVISIBLE)(PThandle);
typedef PTbool	(__stdcall *PTISCLOUDVISIBLE)(PThandle);

typedef PTres	(__stdcall *PTUNLOADSCENE)( PThandle scene );
typedef PTres	(__stdcall *PTRELOADSCENE)( PThandle scene );

typedef PTvoid	(__stdcall *PTREMOVEALL)(void);
typedef PTres	(__stdcall *PTREMOVESCENE)(PThandle);

typedef PTvoid	(__stdcall *PTNORMALISECOORDINATESYSTEM)(void);
typedef PTvoid	(__stdcall *PTPROJECTTOWORLDCOORDS)( PTdouble* );
typedef PTvoid	(__stdcall *PTWORLDTOPROJECTCOORDS)( PTdouble* );

/* Meta data */ 
typedef PThandle	(__stdcall *PTREADPODMETA)( const PTstr filepath );
typedef PThandle	(__stdcall *PTGETMETADATAHANDLE)( PThandle sceneHandle );
typedef PTbool		(__stdcall *PTGETMETADATA)( PThandle metadataHandle, PTstr name, PTint &num_clouds, 
											   PTuint64 &num_points, PTuint &scene_spec, PTdouble *lower3, PTdouble *upper3 );
typedef PTbool		(__stdcall *PTGETMETATAG)( PThandle metadataHandle, const PTstr tagName, const PTstr value );
typedef PTres		(__stdcall *PTSETMETATAG)( PThandle metadataHandle, const PTstr tagName, const PTstr value );
typedef PTres		(__stdcall *PTWRITEMETATAGS)( PThandle metadataHandle );

typedef PTvoid		(__stdcall *PTFREEMETADATA)( PThandle metadataHandle );

/* user metatags */ 
typedef PTint		(__stdcall *PTNUMUSERMETASECTIONS)( PThandle metadataHandle );
typedef const PTstr	(__stdcall *PTUSERMETASECTIONNAME)( PThandle metadataHandle, PTint section_index );

typedef PTint		(__stdcall *PTNUMUSERMETATAGSINSECTION)( PThandle metadataHandle, PTint section_index );
typedef PTbool		(__stdcall *PTGETUSERMETATAGBYINDEX)( PThandle metadataHandle, PTint section_index, PTint tag_index, PTstr name, PTstr value );
typedef PTbool		(__stdcall *PTGETUSERMETATAGBYNAME)( PThandle metadataHandle, const PTstr sectionDotName, PTstr value );

/* scene duplication */ 
typedef PThandle (__stdcall *PTCREATESCENEINSTANCE)( PThandle scene );

/* transformation */ 
typedef PTres (__stdcall *PTSETCLOUDTRANSFORM)( PThandle cloud, const PTdouble *transform4x4, bool row_order );
typedef PTres (__stdcall *PTSETSCENETRANSFORM)( PThandle scene, const PTdouble *transform4x4, bool row_order );
typedef PTres (__stdcall *PTGETCLOUDTRANSFORM)( PThandle cloud, PTdouble *transform4x3, bool row_order );
typedef PTres (__stdcall *PTGETSCENETRANSFORM)( PThandle scene, PTdouble *transform4x3, bool row_order );

/* proxy data */ 
typedef PTuint	(__stdcall *PTGETCLOUDPROXYPOINTS)(PThandle, PTint, PTfloat *, PTubyte *);
typedef PTuint	(__stdcall *PTGETSCENEPROXYPOINTS)(PThandle, PTint, PTfloat *, PTubyte *);

/* error management */ 
typedef PTstr	(__stdcall *PTGETLASTERRORSTRING)(void);
typedef PTres	(__stdcall *PTGETLASTERRORCODE)(void);

/* draw */ 
typedef PTvoid  (__stdcall *PTOVERRIDEDRAWMODE)(PTenum mode);
typedef PTvoid	(__stdcall *PTDRAWGL)(PTbool dynamic);
typedef PTvoid	(__stdcall *PTDRAWSCENEGL)(PThandle scene, PTbool dynamic);
typedef PTvoid	(__stdcall *PTDRAWINTERACTIVEGL)(void);
typedef PTuint	(__stdcall *PTKBLOADED)(PTbool resetCounters);
typedef PTuint	(__stdcall *PTWEIGHTEDPTSLOADED)(PTbool resetCounters);

typedef PTint64	(__stdcall *PTPTSLOADEDINVIEWPORTSINCEDRAW)( PThandle hScene );
typedef	PTint64	(__stdcall *PTPTSTOLOADINVIEWPORT)( PThandle hScene, PTbool recompute );
typedef PTvoid  (__stdcall *PTENDDRAWFRAMEMETRICS)( void );
typedef PTvoid  (__stdcall *PTSTARTDRAWFRAMEMETRICS)( void );

/* units */ 
typedef PTvoid	(__stdcall *PTSETHOSTUNITS)(PTenum units);
typedef PTenum	(__stdcall *PTGETHOSTUNITS)(void);

/* Coordinate Truncation */ 
typedef PTvoid (__stdcall *PTSETAUTOBASEMETHOD)( PTenum type );
typedef PTenum (__stdcall *PTGETAUTOBASEMETHOD)( void );
typedef PTvoid (__stdcall *PTGETCOORDINATEBASE)( PTdouble *coordinateBase );
typedef PTvoid (__stdcall *PTSETCOORDINATEBASE)( PTdouble *coordinateBase );

/* viewports */ 
typedef PTint	(__stdcall *PTADDVIEWPORT)(PTint index, const PTstr name, PTenum contextType);
typedef PTvoid	(__stdcall *PTREMOVEVIEWPORT)(PTint index);
typedef PTvoid	(__stdcall *PTSETVIEWPORT)(PTint index);
typedef PTint	(__stdcall *PTSETVIEWPORTBYNAME)(const PTstr name);
typedef PTint	(__stdcall *PTVIEWPORTINDEXFROMNAME)( const PTstr name );

typedef PTvoid	(__stdcall *PTCAPTUREVIEWPORTINFO)();
typedef PTvoid	(__stdcall *PTSTOREVIEW)();
typedef PTint	(__stdcall *PTCURRENTVIEWPORT)();

typedef PTvoid* (__stdcall *PTCREATEBITMAPVIEWPORT)(int w, int h, const PTstr name);
typedef PTvoid	(__stdcall *PTDESTROYBITMAPVIEWPORT)(const PTstr name);


typedef PTvoid	(__stdcall *PTENABLEVIEWPORT)(PTint index);
typedef PTvoid	(__stdcall *PTDISABLEVIEWPORT)(PTint index);
typedef PTbool	(__stdcall *PTISVIEWPORTENABLED)(PTint index);
typedef PTbool	(__stdcall *PTISCURRENTVIEWPORTENABLED)(void);

/* view */ 
typedef PTbool	(__stdcall *PTREADVIEWFROMGL)( void );

typedef PTvoid	(__stdcall *PTSETVIEWPROJECTIONORTHO)( PTdouble l, PTdouble r, PTdouble b, PTdouble t, 
													  PTdouble n, PTdouble f );
typedef PTvoid	(__stdcall *PTSETVIEWPROJECTIONFRUSTUM)( PTdouble l, PTdouble r, PTdouble b, PTdouble t, 
														PTdouble n, PTdouble f );
typedef PTvoid	(__stdcall *PTSETVIEWPROJECTIONMATRIX)( const PTdouble *matrix, bool row_major );
typedef PTvoid	(__stdcall *PTSETVIEWPROJECTIONPERSPECTIVE)( PTenum perspective_type, PTdouble fov_deg, 
															PTdouble aspect, PTdouble n, PTdouble f);

typedef PTvoid	(__stdcall *PTSETVIEWEYELOOKAT)( const PTdouble *eye, const PTdouble *target, const PTdouble *up );
typedef PTvoid	(__stdcall *PTSETVIEWEYEMATRIX)( const PTdouble *matrix, bool row_major );
typedef PTvoid	(__stdcall *PTSETVIEWPORTSIZE)( PTint left, PTint bottom, PTuint width, PTuint height ); 

typedef PTvoid (__stdcall *PTGETVIEWEYEMATRIX)( PTdouble *matrix );
typedef PTvoid (__stdcall *PTGETVIEWPROJECTIONMATRIX)( PTdouble *matrix );

/* bounds of data */ 
typedef PTbool	(__stdcall *PTGETLOWERBOUND)(PTdouble *lower);
typedef PTbool	(__stdcall *PTGETUPPERBOUND)(PTdouble *upper);

/* shader options */ 
typedef PTres	(__stdcall *PTENABLE)(PTenum option);
typedef PTres	(__stdcall *PTDISABLE)(PTenum option);
typedef PTbool	(__stdcall *PTISENABLED)(PTenum option);

typedef PTres	(__stdcall *PTPOINTSIZE)(PTfloat size);

typedef PTres	(__stdcall *PTSHADEROPTIONF)(PTenum shader_option, PTfloat value);
typedef PTres	(__stdcall *PTSHADEROPTIONFV)(PTenum shader_option, PTfloat *value);
typedef PTres	(__stdcall *PTSHADEROPTIONI)(PTenum shader_option, PTint value);

typedef PTres	(__stdcall *PTGETSHADEROPTIONF)(PTenum shader_option, PTfloat *value);
typedef PTres	(__stdcall *PTGETSHADEROPTIONFV)(PTenum shader_option, PTfloat *values);
typedef PTres	(__stdcall *PTGETSHADEROPTIONI)(PTenum shader_option, PTint *value);
typedef PTvoid	(__stdcall *PTRESETSHADEROPTIONS)(void);

typedef PTvoid	(__stdcall *PTCOPYSHADERSETTINGS)(PTuint dest_viewport);
typedef PTvoid	(__stdcall *PTCOPYSHADERSETTINGSTOALL)();

typedef PTint	(__stdcall *PTNUMRAMPS)(void);
typedef const PTstr (__stdcall *PTRAMPINFO)( PTint ramp, PTenum *type );
typedef PTres	(__stdcall *PTADDCUSTOMRAMP)( const PTstr name, PTint numKeys, const PTfloat *positions, 
											 const PTubyte* colour3vals, PTbool interpolateInHSL );

/* lighting options */ 
typedef PTres	(__stdcall *PTLIGHTOPTIONF)(PTenum light_option, PTfloat value);
typedef PTres	(__stdcall *PTLIGHTOPTIONFV)(PTenum light_option, PTfloat *value);
typedef PTres	(__stdcall *PTLIGHTOPTIONI)(PTenum light_option, PTint value);

typedef PTres	(__stdcall *PTGETLIGHTOPTIONF)(PTenum light_option, PTfloat *value);
typedef PTres	(__stdcall *PTGETLIGHTOPTIONI)(PTenum light_option, PTint *value);
typedef PTvoid	(__stdcall *PTRESETLIGHTOPTIONS)(void);

typedef PTvoid	(__stdcall *PTCOPYLIGHTSETTINGS)(PTuint dest_viewport);
typedef PTvoid	(__stdcall *PTCOPYLIGHTSETTINGSTOALL)(void);

/* persistence */ 
typedef PTuint	(__stdcall *PTGETPERVIEWPORTDATASIZE)(void);
typedef PTuint	(__stdcall *PTGETPERVIEWPORTDATA)(PTubyte *data);
typedef PTres	(__stdcall *PTSETPERVIEWPORTDATA)(const PTubyte *data);
typedef PTvoid	(__stdcall *PTSETVIEWPORTPOINTSBUDGET)( PTint budget );
typedef PTint	(__stdcall *PTGETVIEWPORTPOINTSBUDGET)( void );

/* editing */ 
typedef PTres	(__stdcall * PTSETSELECTPOINTSMODE)( PTenum select_mode );
typedef PTenum	(__stdcall * PTGETSELECTPOINTSMODE)( void );
typedef PTvoid	(__stdcall * PTSELECTPOINTSBYRECT)( PTint x_edge, PTint y_edge, PTint width, PTint height ); 
typedef PTres	(__stdcall * PTSELECTPOINTSBYFENCE)( PTint num_vertices, const PTint *vertices );
typedef PTres	(__stdcall * PTSELECTPOINTSBYCUBE)( const PTdouble *centre, PTdouble radius );
typedef PTres	(__stdcall * PTSELECTPOINTSBYBOX)( const PTdouble *lower, const PTdouble *upper );
typedef PTres	(__stdcall * PTSELECTPOINTSBYORIENTEDBOX)( const PTdouble *lower, const PTdouble *upper, const PTdouble *pos, PTdouble *uAxis, PTdouble *vAxis);
typedef PTres	(__stdcall * PTSELECTPOINTSBYSPHERE)( const PTdouble *centre, PTdouble radius );
typedef PTres	(__stdcall * PTSELECTPOINTSBYPLANE)( const PTdouble *origin, const PTdouble *normal, PTdouble thickness );

typedef PTvoid	(__stdcall * PTHIDESELECTED)(void);
typedef PTvoid	(__stdcall * PTISOLATESELECTED)(void);
typedef PTvoid	(__stdcall * PTUNHIDEALL)(void);
typedef PTvoid	(__stdcall * PTUNSELECTALL)(void);
typedef PTvoid  (__stdcall * PTRESETSELECTION)(void);
typedef PTvoid	(__stdcall * PTSELECTALL)(void);

typedef PTvoid  (__stdcall * PTINVERTSELECTION)(void);
typedef PTvoid  (__stdcall * PTINVERTVISIBILITY)(void);

typedef PTvoid	(__stdcall * PTSETSELECTIONSCOPE)( PThandle sceneOrCloudHandle );
typedef PTvoid  (__stdcall * PTSETSELECTIONDRAWCOLOR)( const PTubyte *col3 );
typedef PTvoid  (__stdcall * PTGETSELECTIONDRAWCOLOR)( PTubyte *col3 );

typedef PTvoid	(__stdcall * PTREFRESHEDIT)(void);
typedef PTvoid	(__stdcall * PTCLEAREDIT)(void);
typedef PTvoid	(__stdcall * PTSTOREEDIT)( const PTstr name );
typedef PTbool	(__stdcall * PTRESTOREEDIT)( const PTstr name );
typedef PTbool	(__stdcall * PTRESTOREEDITBYINDEX)( PTint index );
typedef PTint	(__stdcall * PTNUMEDITS)( void );
typedef const	PTstr (__stdcall * PTEDITNAME)( PTint index );
typedef PTbool	(__stdcall * PTDELETEEDIT)( const PTstr name );
typedef PTbool	(__stdcall * PTDELETEEDITBYINDEX)( PTint index );
typedef PTvoid	(__stdcall * PTDELETEALLEDITS)( void );

typedef PTuint64 (__stdcall *_PTCOUNTVISIBLEPOINTS)( void );

typedef PTvoid	(__stdcall * PTCREATEEDITFROMDATA)( const PTubyte *data );
typedef PTint	(__stdcall * PTGETEDITDATASIZE)( PTint index );
typedef PTint	(__stdcall * PTGETEDITDATA)( PTint index, PTubyte *data );
	
typedef PTres	(__stdcall * PTRESETSCENEEDITING)( PThandle scene );

typedef PTvoid	(__stdcall *PTSETEDITWORKINGMODE)( PTenum mode );
typedef PTenum	(__stdcall *PTGETEDITWORKINGMODE)( void );

typedef PTvoid *(__stdcall *_PTGETEDITDATATREE)( PTint index );
typedef PTvoid  (__stdcall *_PTCREATEEDITFROMDATATREE)( PTvoid * );

typedef PTvoid  (__stdcall *PTSELECTPOINTSINLAYER)( PTuint layer );
typedef PTvoid  (__stdcall *PTDESELECTPOINTSINLAYER)( PTuint layer );
typedef PTres	(__stdcall *PTSELECTCLOUD)( PThandle cloud );
typedef PTres	(__stdcall *PTDESELECTCLOUD)( PThandle cloud );
typedef PTres	(__stdcall *PTSELECTSCENE)( PThandle cloud );
typedef PTres	(__stdcall *PTDESELECTSCENE)( PThandle cloud );


/* options*/ 
typedef PTvoid	(__stdcall *PTDYNAMICFRAMERATE)(PTfloat fps);
typedef PTfloat	(__stdcall *PTGETDYNAMICFRAMERATE)(void);

typedef PTvoid	(__stdcall *PTSTATICOPTIMIZER)(PTfloat opt);
typedef PTfloat	(__stdcall *PTGETSTATICOPTIMIZER)(void);

typedef PTvoid	(__stdcall *PTGLOBALDENSITY)(PTfloat opt);
typedef PTfloat	(__stdcall *PTGETGLOBALDENSITY)(void);

/* Query */ 
typedef PTres	(__stdcall *PTSETINTERSECTIONRADIUS)( PTfloat radius );
typedef PTfloat (__stdcall *PTGETINTERSECTIONRADIUS)(void);

typedef PTint	(__stdcall *PTFINDNEARESTSCREENPOINT)( PThandle scene, PTint screenx, PTint screeny, PTdouble *pnt );
typedef PTfloat (__stdcall *PTFINDNEARESTPOINT)( PThandle scene, const PTdouble *pnt, PTdouble *nearest );
typedef PTint	(__stdcall *PTFINDNEARESTSCREENPOINTWDEPTH)( PThandle scene, PTint screenx, PTint screeny, 
															PTfloat *depthArray4x4, PTdouble *pnt );

typedef PTbool	(__stdcall * PTINTERSECTRAYPNTINDEX)( PThandle scene, const PTdouble *origin, const PTdouble *direction,
													 PThandle *cloud, PThandle *pntPartA, PThandle *pntPartB );

typedef PTbool	(__stdcall * PTINTERSECTRAY)( PThandle scene, const PTdouble *origin, const PTdouble *direction,
											 PTdouble *intersection, PTenum densityType, PTfloat densityValue );

typedef PTbool	(__stdcall * PTINTERSECTRAYINTERPOLATED)( PThandle scene, const PTdouble *origin, 
														 const PTdouble *direction, PThandle *tmpPointHandle );

typedef PTbool	(__stdcall * PTPOINTDATA)( PThandle cloud, PThandle pointIndex, 
										  PTdouble *position, PTshort *intensity, PTubyte *rgb, PTfloat *normal );

typedef PTuint	(__stdcall * PTPOINTATTRIBUTES)( PThandle cloud, PThandle pntPartA, PThandle pntPartB );

typedef PTbool	(__stdcall * PTGETPOINTATTRIBUTE)( PThandle cloud, PThandle pntPartA, PThandle pntPartB, 
												  PTuint attribute, void* data );

typedef PTvoid	(__stdcall *PTFLIPMOUSEYCOORDS)(void);
typedef PTvoid	(__stdcall *PTDONTFLIPMOUSEYCOORDS)(void);

/* point layers */ 
typedef PTbool (__stdcall *PTSETCURRENTLAYER)( PTuint layer );
typedef PTuint (__stdcall *PTGETCURRENTLAYER)( void );
typedef PTbool (__stdcall *PTLOCKLAYER)( PTuint layer, PTbool lock );
typedef PTbool (__stdcall *PTISLAYERLOCKED)( PTuint layer );
typedef PTbool (__stdcall *PTSHOWLAYER)( PTuint layer, PTbool show );
typedef PTbool (__stdcall *PTISLAYERSHOWN)( PTuint layer );
typedef PTbool (__stdcall *PTDOESLAYERHAVEPOINTS)( PTuint layer );
typedef PTvoid (__stdcall *PTCLEARPOINTSFROMLAYER)( PTuint layer );
typedef PTvoid (__stdcall *PTRESETLAYERS)( void );

typedef PTbool		(__stdcall *PTSETLAYERCOLOR)( PTuint layer, PTfloat *rgb, PTfloat blend );
typedef PTfloat *	(__stdcall *PTGETLAYERCOLOR)( PTuint layer );
typedef PTfloat		(__stdcall *PTGETLAYERCOLORBLEND)( PTuint layer );
typedef PTvoid		(__stdcall *PTRESETLAYERCOLORS)( void );

typedef PTbool (__stdcall *PTCOPYSELTOCURRENTLAYER)( PTbool deselect );
typedef PTbool (__stdcall *PTMOVESELTOCURRENTLAYER)( PTbool deselect );

/* points extraction */ 
typedef PThandle (__stdcall * PTCREATESELPOINTSQUERY)(void);
typedef PThandle (__stdcall * PTCREATEVISPOINTSQUERY)(void);
typedef PThandle (__stdcall * PTCREATEBOUNDINGBOXQUERY)( PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, 
														PTdouble maxy, PTdouble maxz );
typedef PThandle (__stdcall * PTCREATEORIENTEDBOUNDINGBOXQUERY)( PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, 
																PTdouble maxy, PTdouble maxz, PTdouble posx, PTdouble posy, PTdouble posz, 
																PTdouble ux, PTdouble uy, PTdouble uz, PTdouble vx, PTdouble vy, PTdouble vz);
typedef PThandle (__stdcall * PTCREATEBOUNDINGSPHEREQUERY)( PTdouble *cen, PTdouble radius );
typedef PThandle (__stdcall * PTCREATEKRIGSURFACEQUERY) (PTuint numPoints, PTdouble *pnts );
typedef PThandle (__stdcall * PTCREATEFRUSTUMPOINTSCACHE)( PTuint maxPoints );
typedef PThandle (__stdcall * PTCREATEFRUSTUMPOINTSQUERY)( void );
typedef PThandle (__stdcall * PTCREATEKNNQUERY)(PTfloat *vertices, PTint numQueryVertices, PTint k, PTfloat queryLOD);
typedef PTbool (__stdcall * PTRESETQUERY)( PThandle query );
typedef PTres (__stdcall *PTSETQUERYRGBMODE)( PThandle query, PTenum mode );
typedef PTbool (__stdcall *PTDELETEQUERY)( PThandle query );
typedef PTres  (__stdcall *PTSETQUERYSCOPE)( PThandle query, PThandle sceneOrCloudHandle );

typedef PTres  (__stdcall *PTSETQUERYDENSITY)( PThandle query, PTenum densityType, PTfloat densityValue );
typedef PTuint (__stdcall * PTGETQUERYPOINTSD)( PThandle query, PTuint buffersize, PTdouble *geomBuffer, 
											   PTubyte *rgbBuffer, PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer );

typedef PTuint (__stdcall * PTGETQUERYPOINTSF)( PThandle query, PTuint buffersize, PTfloat *geomBuffer, PTubyte *rgbBuffer, 
											   PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer  );

typedef PTuint (__stdcall * PTGETDETAILEDQUERYPOINTSF)( PThandle query, PTuint bufferSize, PTfloat *geomBuffer, PTubyte *rgbBuffer, 
													   PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer,  
													   PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels );

typedef PTuint (__stdcall *PTGETDETAILEDQUERYPOINTSD)( PThandle query, PTuint bufferSize, PTdouble *geomBuffer, PTubyte *rgbBuffer, 
													  PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer,  
													  PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels );

typedef PTuint (__stdcall *PTGETQUERYPOINTSMULTIF)( PThandle query, PTuint numResultSets, PTuint buffersize, PTuint *resultSetSize, PTfloat **geomBufferArray, 
												   PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray );

typedef PTuint (__stdcall *PTGETQUERYPOINTSMULTID)( PThandle query, PTuint numResultSets, PTuint buffersize, PTuint *resultSetSize, PTdouble **geomBufferArray, 
												   PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray );

/* tuning parameters */ 
typedef PTvoid	(__stdcall * PTSETCACHESIZEMB)( PTuint mb );
typedef PTuint	(__stdcall * PTGETCACHESIZEMB)( void );
typedef PTvoid	(__stdcall * PTAUTOCACHESIZE)( void );
typedef PTres	(__stdcall * PTSETLOADINGPRIORITYBIAS)( PTenum bias );
typedef PTenum	(__stdcall * PTGETLOADINGPRIORITYBIAS)( void );
typedef PTres	(__stdcall * PTSETTUNINGPARAMETERFV)( PTenum param, const PTfloat *values );
typedef PTres	(__stdcall * PTGETTUNINGPARAMETERFV)( PTenum param, PTfloat *values );

/* user channels */ 
typedef PThandle (__stdcall * PTCREATEPOINTCHANNEL)( PTstr name, PTenum typesize, PTuint multiple, 
													void* default_value, PTuint flags );
typedef PThandle (__stdcall * PTCOPYPOINTCHANNEL)(PThandle channel, PTstr destName, PTuint destFlags);
typedef PTres	 (__stdcall * PTDELETEPOINTCHANNEL)( PThandle channel );
typedef PTres	 (__stdcall * PTSUBMITPOINTCHANNELUPDATE)( PThandle query, PThandle channel );
typedef PTres	 (__stdcall * PTDRAWPOINTCHANNELAS)( PThandle channel, PTenum method, PTfloat param1, PTfloat param2 );
typedef PTres	 (__stdcall * PTWRITECHANNELSFILE)( const PTstr filename, PTint numChannels, const PThandle *channels );
typedef PTuint64 (__stdcall * PTWRITECHANNELSFILETOBUFFER)(PTint numChannels, const PThandle *channels, PTubyte *&buffer, PTuint64 &bufferSize);
typedef PTvoid	 (__stdcall * PTRELEASECHANNELSFILEBUFFER)(PTuint64 bufferHandle);
typedef PTres	 (__stdcall * PTREADCHANNELSFILE)( const PTstr filename, PTint &numChannels, const PThandle **channels );
typedef PTres	 (__stdcall * PTREADCHANNELSFILEFROMBUFFER)(void *buffer, PTuint64 bufferSize, PTint &numChannels, const PThandle **channels );

typedef PTres	 (__stdcall * PTSETCHANNELOOCFOLDER)( const PTstr foldername );
typedef PTvoid	 (__stdcall * PTDELETEALLCHANNELS)( void );
typedef PTres	 (__stdcall * PTGETCHANNELINFO)(PThandle handle, PTstr name, PTenum& typesize, PTuint& multiple, void *defaultValue, PTuint& flags);
typedef PThandle (__stdcall * PTGETCHANNELBYNAME)( const PTstr channelname );

typedef PThandle (__stdcall * PTCREATEPOINTCHANNELFROMLAYERS)( PTstr name, PThandle sceneHandle );	
typedef PTbool	 (__stdcall * PTLAYERSFROMPOINTCHANNEL)( PThandle userChannel, PThandle sceneHandle );

/* Clipping planes */
typedef PTvoid	(__stdcall * PTENABLECLIPPING)( void );
typedef PTvoid	(__stdcall * PTDISABLECLIPPING)( void );
typedef PTres	(__stdcall * PTSETCLIPSTYLE)( PTuint style );
typedef PTuint	(__stdcall * PTGETNUMCLIPPINGPLANES)( void );
typedef PTbool	(__stdcall * PTISCLIPPINGPLANEENABLED)( PTuint plane );
typedef PTres	(__stdcall * PTENABLECLIPPINGPLANE)( PTuint plane );
typedef PTres	(__stdcall * PTDISABLECLIPPINGPLANE)( PTuint plane );
typedef PTres	(__stdcall * PTSETCLIPPINGPLANEPARAMETERS)( PTuint plane, PTdouble a, PTdouble b, PTdouble c, PTdouble d );

typedef PTres	(__stdcall * _PTDIAGNOSTIC)( PTvoid *data );


/* Interface Function Pointers Section ----------------------------------------------------------------------------------------------------*/ 
/* initialisation */ 
extern PTINITIALIZE ptInitialize;
extern PTISINITIALIZED ptIsInitialized;
extern PTSETWORKINGFOLDER ptSetWorkingFolder;
extern PTGETWORKINGFOLDER ptGetWorkingFolder;
extern PTGETVERSIONSTRING ptGetVersionString;
extern PTGETVERSIONNUM ptGetVersionNum;
extern PTRELEASE ptRelease;

/* Client Server */

extern PTCREATEFAKEPOD							ptCreateFakePOD;				
extern PTSETSERVERCALLBACK						ptSetServerCallBack;
extern PTSETRELEASECLIENTSERVERBUFFERCALLBACK	ptSetReleaseClientServerBufferCallBack;
extern PTPROCESSSERVERREQUEST					ptProcessServerRequest;		
extern PTPROCESSSERVERREQUESTCLIENTID			ptProcessServerRequestClientID;
extern PTPROCESSSERVERREQUESTCLIENTID2			ptProcessServerRequestClientID2;

extern PTSETCLIENTSERVERLOGFILE					ptSetClientServerLogFile;	
extern PTSETCLIENTSERVERSENDRETRIES				ptSetClientServerSendRetries;
extern PTGETCLIENTSERVERSENDRETRIES				ptGetClientServerSendRetries;
extern PTSETCLIENTSTREAMING						ptSetClientStreaming;
extern PTSERVERCLIENTLOST						ptServerClientLost;
extern PTGETSESSIONID							ptGetSessionID;


/* Client Server Caching */ 
extern PTSETCLIENTCACHEFOLDER					ptSetClientCacheFolder;	
extern PTGETCLIENTCACHEFOLDER					ptGetClientCacheFolder;			
extern PTENABLECLIENTSERVERCACHING				ptEnableClientServerCaching;			
extern PTGETCLIENTSERVERCACHINGENABLED			ptGetClientServerCachingEnabled;		
extern PTSETCLIENTSERVERCACHEDATASIZE			ptSetClientServerCacheDataSize;		
extern PTGETCLIENTSERVERCACHEDATASIZE			ptGetClientServerCacheDataSize;		
extern PTSETCLIENTCACHECOMPLETIONTHRESHOLD		ptSetClientCacheCompletionThreshold;	
extern PTGETCLIENTCACHECOMPLETIONTHRESHOLD		ptGetClientCacheCompletionThreshold;
extern PTSETCLIENTSERVERCACHENAMEMODE			ptSetClientServerCacheNameMode;
extern PTGETCLIENTSERVERCACHENAMEMODE			ptGetClientServerCacheNameMode;


/* file */ 
extern PTOPENPOD ptOpenPOD;
extern PTOPENPODSTRUCTUREDSTORAGESTREAM ptOpenPODStructuredStorageStream;
extern PTISOPEN ptIsOpen;
extern PTBROWSEANDOPENPOD ptBrowseAndOpenPOD;

extern PTUNLOADSCENE ptUnloadScene;
extern PTRELOADSCENE ptReloadScene;

extern PTREMOVESCENE ptRemoveScene;
extern PTREMOVEALL ptRemoveAll;

/* scene info */ 
extern PTGETCLOUDHANDLEBYINDEX ptGetCloudHandleByIndex;
extern PTGETNUMCLOUDSINSCENE ptGetNumCloudsInScene;

extern PTNUMSCENES ptNumScenes;
extern PTGETSCENEHANDLES ptGetSceneHandles;

extern PTSCENEINFO ptSceneInfo;
extern PTCLOUDINFO ptCloudInfo;
extern PTSCENEFILE ptSceneFile;

/* meta data */ 
extern  PTREADPODMETA ptReadPODMeta;
extern  PTGETMETADATAHANDLE ptGetMetaDataHandle;
extern  PTGETMETADATA ptGetMetaData;

extern  PTGETMETATAG ptGetMetaTag;
extern	PTSETMETATAG ptSetMetaTag;
extern	PTWRITEMETATAGS ptWriteMetaTags;

extern  PTFREEMETADATA ptFreeMetaData;

extern  PTNUMUSERMETASECTIONS ptNumUserMetaSections;
extern  PTUSERMETASECTIONNAME ptUserMetaSectionName;

extern  PTNUMUSERMETATAGSINSECTION ptNumUserMetaTagsInSection;
extern  PTGETUSERMETATAGBYINDEX ptGetUserMetaTagByIndex;
extern  PTGETUSERMETATAGBYNAME ptGetUserMetaTagByName;

/* scene instancing */ 
extern PTCREATESCENEINSTANCE ptCreateSceneInstance;

/* transformation */ 
extern PTSETCLOUDTRANSFORM ptSetCloudTransform;
extern PTSETSCENETRANSFORM ptSetSceneTransform;
extern PTGETCLOUDTRANSFORM ptGetCloudTransform;
extern PTGETSCENETRANSFORM ptGetSceneTransform;

/* bounds info */ 
extern PTSCENEBOUNDS ptSceneBounds;
extern PTSCENEBOUNDSD ptSceneBoundsd;
extern PTCLOUDBOUNDS ptCloudBounds;
extern PTCLOUDBOUNDSD ptCloudBoundsd;
extern PTLAYERBOUNDS ptLayerBounds;
extern PTLAYERBOUNDSD ptLayerBoundsd;


extern PTGETLOWERBOUND ptGetLowerBound;
extern PTGETUPPERBOUND ptGetUpperBound;

/* coordinate system */ 
extern PTNORMALISECOORDINATESYSTEM ptNormaliseCoordinateSystem;
extern PTPROJECTTOWORLDCOORDS ptProjectToWorldCoords;
extern PTWORLDTOPROJECTCOORDS ptWorldToProjectCoords;
extern PTSETAUTOBASEMETHOD ptSetAutoBaseMethod;
extern PTGETAUTOBASEMETHOD ptGetAutoBaseMethod;
extern PTSETCOORDINATEBASE ptSetCoordinateBase;
extern PTGETCOORDINATEBASE ptGetCoordinateBase;

/* error handling */ 
extern PTGETLASTERRORSTRING	 ptGetLastErrorString;
extern PTGETLASTERRORCODE ptGetLastErrorCode;

/* display */ 
extern PTSHOWSCENE ptShowScene;
extern PTSHOWCLOUD ptShowCloud;

extern PTISSCENEVISIBLE ptIsSceneVisible;
extern PTISCLOUDVISIBLE ptIsCloudVisible;

extern PTOVERRIDEDRAWMODE ptOverrideDrawMode;
extern PTDRAWGL ptDrawGL;
extern PTDRAWSCENEGL ptDrawSceneGL;
extern PTDRAWINTERACTIVEGL ptDrawInteractiveGL;
extern PTKBLOADED ptKbLoaded;
extern PTWEIGHTEDPTSLOADED ptWeightedPtsLoaded;
extern PTPTSLOADEDINVIEWPORTSINCEDRAW ptPtsLoadedInViewportSinceDraw;
extern PTPTSTOLOADINVIEWPORT ptPtsToLoadInViewport;
extern PTENDDRAWFRAMEMETRICS ptEndDrawFrameMetrics;
extern PTSTARTDRAWFRAMEMETRICS ptStartDrawFrameMetrics;

extern PTSETQUERYSCOPE ptSetQueryScope;

extern PTRESETQUERY ptResetQuery;

/* units */ 
extern PTSETHOSTUNITS ptSetHostUnits;
extern PTGETHOSTUNITS ptGetHostUnits;

/* enable / disable */ 
extern PTENABLE ptEnable;
extern PTDISABLE ptDisable;
extern PTISENABLED ptIsEnabled;

/* lighting */ 
extern PTLIGHTOPTIONF ptLightOptionf;
extern PTLIGHTOPTIONFV ptLightOptionfv;
extern PTLIGHTOPTIONI ptLightOptioni;
extern PTGETLIGHTOPTIONF ptGetLightOptionf;
extern PTGETLIGHTOPTIONI ptGetLightOptioni;

extern PTRESETLIGHTOPTIONS ptResetLightOptions;
extern PTCOPYLIGHTSETTINGS ptCopyLightSettings;
extern PTCOPYLIGHTSETTINGSTOALL ptCopyLightSettingsToAll;

/* shader options */ 
extern PTPOINTSIZE ptPointSize;

extern PTSHADEROPTIONF ptShaderOptionf;
extern PTSHADEROPTIONFV ptShaderOptionfv;
extern PTSHADEROPTIONI ptShaderOptioni;

extern PTGETSHADEROPTIONF ptGetShaderOptionf;
extern PTGETSHADEROPTIONF ptGetShaderOptionfv;
extern PTGETSHADEROPTIONI ptGetShaderOptioni;
extern PTRESETSHADEROPTIONS ptResetShaderOptions;

extern PTCOPYSHADERSETTINGS ptCopyShaderSettings;
extern PTCOPYSHADERSETTINGSTOALL ptCopyShaderSettingsToAll;

/* shading ramps */ 
extern PTNUMRAMPS ptNumRamps;
extern PTRAMPINFO ptRampInfo;
extern PTADDCUSTOMRAMP ptAddCustomRamp;

/* viewports */ 
extern PTADDVIEWPORT ptAddViewport;
extern PTREMOVEVIEWPORT ptRemoveViewport;
extern PTSETVIEWPORT ptSetViewport;
extern PTSETVIEWPORTBYNAME ptSetViewportByName;
extern PTVIEWPORTINDEXFROMNAME ptViewportIndexFromName;

extern PTCAPTUREVIEWPORTINFO ptCaptureViewportInfo;
extern PTSTOREVIEW ptStoreView;
extern PTCURRENTVIEWPORT ptCurrentViewport;

extern PTCREATEBITMAPVIEWPORT ptCreateBitmapViewport;
extern PTDESTROYBITMAPVIEWPORT ptDestroyBitmapViewport;

extern PTENABLEVIEWPORT ptEnableViewport;
extern PTDISABLEVIEWPORT ptDisableViewport;
extern PTISVIEWPORTENABLED ptIsViewportEnabled;
extern PTISCURRENTVIEWPORTENABLED ptIsCurrentViewportEnabled;

extern	PTGETPERVIEWPORTDATASIZE ptGetPerViewportDataSize;
extern	PTGETPERVIEWPORTDATA ptGetPerViewportData;
extern	PTSETPERVIEWPORTDATA ptSetPerViewportData;

extern PTSETVIEWPORTPOINTSBUDGET ptSetViewportPointsBudget;
extern PTGETVIEWPORTPOINTSBUDGET ptGetViewportPointsBudget;

/* view */ 
extern PTREADVIEWFROMGL ptReadViewFromGL;

extern PTSETVIEWPROJECTIONORTHO ptSetViewProjectionOrtho;
extern PTSETVIEWPROJECTIONFRUSTUM ptSetViewProjectionFrustum;
extern PTSETVIEWPROJECTIONMATRIX ptSetViewProjectionMatrix;
extern PTSETVIEWPROJECTIONPERSPECTIVE ptSetViewProjectionPerspective;

extern PTSETVIEWEYELOOKAT ptSetViewEyeLookAt;
extern PTSETVIEWEYEMATRIX ptSetViewEyeMatrix;

extern PTSETVIEWPORTSIZE ptSetViewportSize;

extern PTGETVIEWEYEMATRIX ptGetViewEyeMatrix;
extern PTGETVIEWPROJECTIONMATRIX ptGetViewProjectionMatrix;

/* editing */ 
extern PTSETSELECTPOINTSMODE ptSetSelectPointsMode;
extern PTGETSELECTPOINTSMODE ptGetSelectPointsMode;
extern PTSELECTPOINTSBYRECT ptSelectPointsByRect; 
extern PTSELECTPOINTSBYFENCE ptSelectPointsByFence;
extern PTSELECTPOINTSBYBOX ptSelectPointsByBox;
extern PTSELECTPOINTSBYORIENTEDBOX ptSelectPointsByOrientedBox;
extern PTSELECTPOINTSBYCUBE ptSelectPointsByCube;
extern PTSELECTPOINTSBYPLANE ptSelectPointsByPlane;
extern PTSELECTPOINTSBYSPHERE ptSelectPointsBySphere;

extern PTHIDESELECTED ptHideSelected;
extern PTISOLATESELECTED ptIsolateSelected;
extern PTUNHIDEALL ptUnhideAll;
extern PTUNSELECTALL ptUnselectAll;
extern PTRESETSELECTION ptResetSelection;
extern PTSELECTALL ptSelectAll;

extern _PTCOUNTVISIBLEPOINTS _ptCountVisiblePoints;

extern PTSETSELECTIONSCOPE ptSetSelectionScope;
extern PTSETSELECTIONDRAWCOLOR ptSetSelectionDrawColor;
extern PTGETSELECTIONDRAWCOLOR ptGetSelectionDrawColor;

extern PTINVERTSELECTION ptInvertSelection;
extern PTINVERTVISIBILITY ptInvertVisibility;

extern PTRESETSCENEEDITING ptResetSceneEditing;

extern PTREFRESHEDIT ptRefreshEdit;
extern PTCLEAREDIT ptClearEdit;
extern PTSTOREEDIT ptStoreEdit;
extern PTRESTOREEDIT ptRestoreEdit;
extern PTRESTOREEDITBYINDEX ptRestoreEditByIndex;
extern PTNUMEDITS ptNumEdits;
extern PTEDITNAME ptEditName;

extern PTDELETEEDIT ptDeleteEdit;
extern PTDELETEEDITBYINDEX ptDeleteEditByIndex;
extern PTDELETEALLEDITS ptDeleteAllEdits;

extern PTGETEDITDATA ptGetEditData;
extern PTGETEDITDATASIZE ptGetEditDataSize;
extern PTCREATEEDITFROMDATA ptCreateEditFromData;

extern PTSETEDITWORKINGMODE ptSetEditWorkingMode;
extern PTGETEDITWORKINGMODE ptGetEditWorkingMode;

extern PTSELECTPOINTSINLAYER ptSelectPointsInLayer;  
extern PTDESELECTPOINTSINLAYER ptDeselectPointsInLayer;
extern PTSELECTCLOUD ptSelectCloud;
extern PTDESELECTCLOUD ptDeselectCloud;
extern PTSELECTSCENE ptSelectScene;
extern PTDESELECTSCENE ptDeselectScene;


/* visualisation settings */ 
extern PTDYNAMICFRAMERATE ptDynamicFrameRate;
extern PTGETDYNAMICFRAMERATE ptGetDynamicFrameRate;

extern PTSTATICOPTIMIZER ptStaticOptimizer;
extern PTGETSTATICOPTIMIZER ptGetStaticOptimizer;

extern PTGETGLOBALDENSITY ptGetGlobalDensity;
extern PTGLOBALDENSITY	ptGlobalDensity;

/* screen based point */ 
extern PTFINDNEARESTSCREENPOINT ptFindNearestScreenPoint;
extern PTFINDNEARESTSCREENPOINTWDEPTH  ptFindNearestScreenPointWDepth;
extern PTFINDNEARESTPOINT ptFindNearestPoint;
extern PTFLIPMOUSEYCOORDS ptFlipMouseYCoords;
extern PTDONTFLIPMOUSEYCOORDS ptDontFlipMouseYCoords;

/* ray intersection */ 
extern PTGETINTERSECTIONRADIUS ptGetIntersectionRadius;
extern PTSETINTERSECTIONRADIUS ptSetIntersectionRadius;
extern PTINTERSECTRAY ptIntersectRay;
extern PTINTERSECTRAYPNTINDEX ptIntersectRayPntIndex;
extern PTINTERSECTRAYINTERPOLATED ptIntersectRayInterpolated;

/* point data access */ 
extern PTGETSCENEPROXYPOINTS ptGetSceneProxyPoints;
extern PTGETCLOUDPROXYPOINTS ptGetCloudProxyPoints;

extern PTPOINTDATA ptPointData;
extern PTPOINTATTRIBUTES ptPointAttributes;
extern PTGETPOINTATTRIBUTE ptGetPointAttribute; 	

extern PTCREATESELPOINTSQUERY ptCreateSelPointsQuery;
extern PTCREATEVISPOINTSQUERY ptCreateVisPointsQuery;
extern PTCREATEBOUNDINGBOXQUERY ptCreateBoundingBoxQuery;
extern PTCREATEORIENTEDBOUNDINGBOXQUERY ptCreateOrientedBoundingBoxQuery;
extern PTCREATEBOUNDINGSPHEREQUERY ptCreateBoundingSphereQuery;
extern PTCREATEKRIGSURFACEQUERY ptCreateKrigSurfaceQuery;
extern PTCREATEFRUSTUMPOINTSCACHE ptCreateFrustumPointsCache;
extern PTCREATEFRUSTUMPOINTSQUERY ptCreateFrustumPointsQuery;
extern PTCREATEKNNQUERY ptCreateKNNQuery;
extern PTDELETEQUERY ptDeleteQuery;

extern PTSETQUERYRGBMODE ptSetQueryRGBMode;
extern PTSETQUERYDENSITY ptSetQueryDensity;
extern PTGETQUERYPOINTSD ptGetQueryPointsd;
extern PTGETQUERYPOINTSF ptGetQueryPointsf;
extern PTGETDETAILEDQUERYPOINTSF ptGetDetailedQueryPointsf;
extern PTGETDETAILEDQUERYPOINTSD ptGetDetailedQueryPointsd;

extern PTGETQUERYPOINTSMULTIF ptGetQueryPointsMultif;
extern PTGETQUERYPOINTSMULTID ptGetQueryPointsMultid;


/* point layers */ 
extern PTSETCURRENTLAYER ptSetCurrentLayer;
extern PTGETCURRENTLAYER ptGetCurrentLayer;
extern PTLOCKLAYER ptLockLayer;
extern PTISLAYERLOCKED ptIsLayerLocked;
extern PTSHOWLAYER ptShowLayer;
extern PTISLAYERSHOWN ptIsLayerShown;
extern PTDOESLAYERHAVEPOINTS ptDoesLayerHavePoints;
extern PTCLEARPOINTSFROMLAYER ptClearPointsFromLayer;
extern PTRESETLAYERS ptResetLayers;

extern PTSETLAYERCOLOR ptSetLayerColor;
extern PTGETLAYERCOLOR ptGetLayerColor;
extern PTGETLAYERCOLORBLEND ptGetLayerColorBlend;
extern PTRESETLAYERCOLORS ptResetLayerColors;

extern PTCOPYSELTOCURRENTLAYER ptCopySelToCurrentLayer;
extern PTMOVESELTOCURRENTLAYER ptMoveSelToCurrentLayer;

/* tuning */ 
extern PTSETCACHESIZEMB ptSetCacheSizeMb;
extern PTGETCACHESIZEMB ptGetCacheSizeMb;
extern PTAUTOCACHESIZE ptAutoCacheSize;
extern PTSETLOADINGPRIORITYBIAS ptSetLoadingPriorityBias;
extern PTGETLOADINGPRIORITYBIAS ptGetLoadingPriorityBias;
extern PTSETTUNINGPARAMETERFV ptSetTuningParameterfv;
extern PTGETTUNINGPARAMETERFV ptGetTuningParameterfv;

/* user point channel */ 
extern PTCREATEPOINTCHANNEL ptCreatePointChannel;
extern PTCOPYPOINTCHANNEL ptCopyPointChannel;
extern PTDELETEPOINTCHANNEL ptDeletePointChannel;
extern PTSUBMITPOINTCHANNELUPDATE ptSubmitPointChannelUpdate;
extern PTDRAWPOINTCHANNELAS  ptDrawPointChannelAs;
extern PTWRITECHANNELSFILE	ptWriteChannelsFile;
extern PTWRITECHANNELSFILETOBUFFER ptWriteChannelsFileToBuffer;
extern PTRELEASECHANNELSFILEBUFFER ptReleaseChannelsFileBuffer;
extern PTREADCHANNELSFILE	ptReadChannelsFile;
extern PTREADCHANNELSFILEFROMBUFFER ptReadChannelsFileFromBuffer;
extern PTSETCHANNELOOCFOLDER ptSetChannelOOCFolder;
extern PTDELETEALLCHANNELS ptDeleteAllChannels;
extern PTGETCHANNELINFO ptGetChannelInfo;
extern PTGETCHANNELBYNAME ptGetChannelByName;
extern PTCREATEPOINTCHANNELFROMLAYERS ptCreatePointChannelFromLayers;	
extern PTLAYERSFROMPOINTCHANNEL ptLayersFromPointChannel;


/* Clipping planes */
extern PTENABLECLIPPING ptEnableClipping;
extern PTDISABLECLIPPING ptDisableClipping;
extern PTSETCLIPSTYLE ptSetClipStyle;
extern PTGETNUMCLIPPINGPLANES ptGetNumClippingPlanes;
extern PTISCLIPPINGPLANEENABLED ptIsClippingPlaneEnabled;
extern PTENABLECLIPPINGPLANE ptEnableClippingPlane;
extern PTDISABLECLIPPINGPLANE ptDisableClippingPlane;
extern PTSETCLIPPINGPLANEPARAMETERS ptSetClippingPlaneParameters;


#endif // End Static Lib

#endif