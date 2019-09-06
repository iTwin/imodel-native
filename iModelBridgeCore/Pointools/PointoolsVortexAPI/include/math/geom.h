/*  geometryc.h  26 September 2000  */

#pragma once

#define ERROR        0
#define I_MAX        2147483647
#define PI           3.14159265358979323846264338327950288419716939937510
#define SUCCESS      1

#define DEG_TO_RAD   ( PI / 180.0 )
#define MAX(a,b) ( (a)>(b) ? (a) : (b) ) 
#define MIN(a,b) ( (a)>(b) ? (b) : (a) )
#define RAD_TO_DEG   ( 180.0 / PI )

BEGIN_EXTERN_C

	extern float angle_deg_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3 );
	extern float angle_rad_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3 );
	extern float angle_rad_3d ( float x1, float y1, float z1, float x2, float y2, float z2,
			float x3, float y3, float z3 );
	extern float angle_rad_nd ( int n, float vec1[], float vec2[] );
	extern float anglei_deg_2d ( float x1, float y1, float x2, float y2, float x3, 
			float y3 );
	extern float anglei_rad_2d ( float x1, float y1, float x2, float y2, float x3, 
			float y3 );
	extern int   box_contains_point_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float x4, float y4, 
			float z4, float x, float y, float z );
	extern float box_point_dist_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float x4, float y4, 
			float z4, float x, float y, float z );
	extern void  circle_dia2imp_2d ( float x1, float y1, float x2, float y2, float *r, 
			float *xc, float *yc );
	extern int   circle_exp_contains_point_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3, float x, float y );
	extern int   circle_imp_contains_point_2d ( float radius, float xc, float yc,
			float x, float y );
	extern void  circle_imp_line_par_int_2d ( float radius, float xc, float yc,
			float x0, float y0, float f, float g, int *nint, float x[], float y[] );
	extern void  circle_points_2d ( int n, float x[], float y[] );
	extern float cone_area_3d ( float h, float r );
	extern float cone_volume_3d ( float h, float r );
	extern float cot_rad ( float angle );
	extern float cot_deg ( float angle );
	extern float cross_2d ( float x1, float y1, float x2, float y2 );
	extern void  cross_3d ( float x1, float y1, float z1, float x2, float y2, float z2, 
			float *x3, float *y3, float *z3 );
	extern float cross0_2d ( float x0, float y0, float x1, float y1, float x2, float y2 );
	extern void  cross0_3d ( float x0, float y0, float z0, float x1, float y1, float z1,
			float x2, float y2, float z2, float *x3, float *y3, float *z3 );
	extern void  direction_pert_3d ( float sigma, int *iseed, float vbase[3], 
			float vran[3] );
	extern void  direction_random_3d ( int *iseed, float vran[3] );
	extern void  direction_random_nd ( int n, int *iseed, float w[] );
	extern float dot_2d ( float x1, float y1, float x2, float y2 );
	extern float dot_3d ( float x1, float y1, float z1, float x2, float y2, float z2 );
	extern float dot0_2d ( float x0, float y0, float x1, float y1, float x2, 
			float y2 );
	extern float dot0_3d ( float x0, float y0, float z0, float x1, float y1, float z1, 
			float x2, float y2, float z2 );
	extern float dot_nd ( int n, float vec1[], float vec2[] );
	extern float enorm_2d ( float x1, float y1 );
	extern float enorm_3d ( float x1, float y1, float z1 );
	extern float enorm_nd ( int n, float x[] );
	extern float enorm0_2d ( float x0, float y0, float x1, float y1 );
	extern float enorm0_3d ( float x0, float y0, float z0, float x1, float y1, float z1 );
	extern float enorm0_nd ( int n, float x[], float y[] );
	extern float enormsq0_2d ( float x0, float y0, float x1, float y1 );
	extern float enormsq0_3d ( float x0, float y0, float z0, float x1, float y1, float z1 );
	extern float enormsq0_nd ( int n, float x0[], float x1[] );
	extern int   get_seed ( void );
	extern int   halfspace_imp_triangle_int_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float a, float b, 
			float c, float d, float x[], float y[], float z[] );
	int   halfspace_norm_triangle_int_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float xp, float yp, 
			float zp, float xn, float yn, float zn, float x[], float y[], float z[] );
	int   halfspace_triangle_int_3d ( float x1, float y1, float z1, 
			float x2, float y2, float z2, float x3, float y3, float z3, float dist1, 
			float dist2, float dist3, float x[], float y[], float z[] );
	int   i_random ( int ilo, int ihi, int *iseed );
	void  line_exp2imp_2d ( float x1, float y1, float x2, float y2, float *a, 
			float *b, float *c );
	void  line_exp2par_3d ( float x1, float y1, float z1, float x2, float y2,  
			float z2, float *f, float *g, float *h, float *x0, float *y0, 
			float *z0 );
	float line_exp_point_dist_2d ( float x1, float y1, float x2, float y2, 
			float x, float y );
	float line_exp_point_dist_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x, float y, float z );
	float line_exp_point_dist_signed_2d ( float x1, float y1, float x2, 
			float y2, float x, float y );
	float line_seg_point_dist_3d ( float x1, float y1, float z1, float x2,  
			float y2, float z2, float x, float y, float z );
	void  line_seg_point_near_3d ( float x1, float y1, float z1, 
			float x2, float y2, float z2, float x, float y, float z,
			float *xn, float *yn, float *zn, float *dist, float *t );
	float lines_imp_angle_2d ( float a1, float b1, float c1, 
			float a2, float b2, float c2 );
	void lines_imp_int_2d ( float a1, float b1, float c1, float a2, float b2, 
			float c2, int *ival, float *x, float *y );
	float lines_seg_dist_3d ( float x1, float y1, float z1, float x2, float y2,  
			float z2, float x3, float y3, float z3, float x4, float y4, float z4 );
	int   minquad ( float x1, float y1, float x2, float y2, float x3, float y3, 
			float *xmin, float *ymin );
	float normal_01_sample ( int *iseed );
	int   para_contains_point_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float x, float y, 
			float z );
	float para_point_dist_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float x, float y, 
			float z );
	int   parabola_ex ( float x1, float y1, float x2, float y2, float x3, float y3, 
			float *x, float *y );
	int   parabola_ex2 ( float x1, float y1, float x2, float y2, float x3, float y3, 
			float *x, float *y, float *a, float *b, float *c );
	void  plane_exp2imp_3d ( float x1, float y1, float z1, float x2, float y2, 
			float z2, float x3, float y3, float z3, float *a, float *b, float *c, 
			float *d );
	void  plane_exp2norm_3d ( float x1, float y1, float z1, float x2, float y2, 
			float z2, float x3, float y3, float z3,
			float *xp, float *yp, float *zp, float *xn, float *yn, float *zn );
	void  plane_exp_normal_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float *xn, 
			float *yn, float *zn );
	void  plane_imp2norm_3d ( float a, float b, float c, float d, 
			float *xp, float *yp, float *zp, float *xn, float *yn, float *zn );
	void  plane_imp_line_seg_near_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float a, float b, float c, float d, float *dist, 
			float *xp, float *yp, float *zp, float *xls, float *yls, float *zls );
	float plane_imp_point_dist_3d ( float a, float b, float c, float d, 
			float x, float y, float z );
	float plane_imp_point_dist_signed_3d ( float a, float b, float c, float d, 
			float x, float y, float z );
	void  plane_imp_triangle_int_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float a, float b, 
			float c, float d, int *num_int, float x[], float y[], float z[] );
	void  plane_imp_triangle_int_add_3d ( float x1, float y1, float z1, 
			float x2, float y2, float z2, float dist1, float dist2, 
			int *num_int, float x[], float y[], float z[] );
	void  plane_norm_basis_3d ( float xp, float yp, float zp, float xn, 
			float yn, float zn, float *xq, float *yq, float *zq, float *xr, 
			float *yr, float *zr );
	void  plane_norm2imp_3d ( float xp, float yp, float zp, float xn, 
			float yn, float zn, float *a, float *b, float *c, float *d );
	float points_colin_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3 );
	float points_colin_3d ( float x1, float y1, float z1, float x2, float y2, 
			float z2, float x3, float y3, float z3 );
	float polygon_1_2d ( int n, float x[], float y[] );
	float polygon_area_2d ( int n, float x[], float y[] );
	float polygon_area_2_2d ( int n, float x[], float y[] );
	float polygon_area_2_3d ( int n, float x[], float y[], float z[] );
	float polygon_area_3d ( int n, float x[], float y[], float z[], 
			float normal[] );
	void  polygon_centroid_2d ( int n, float x[], float y[], 
			float *cx, float *cy );
	void  polygon_centroid_2_2d ( int n, float x[], float y[], float *cx, 
			float *cy );
	void  polygon_centroid_3d ( int n, float x[], float y[], float z[], 
			float *cx, float *cy, float *cz );
	float polygon_x_2d ( int n, float x[], float y[] );
	float polygon_y_2d ( int n, float x[], float y[] );
	float polygon_xx_2d ( int n, float x[], float y[] );
	float polygon_xy_2d ( int n, float x[], float y[] );
	float polygon_yy_2d ( int n, float x[], float y[] );
	float rmat2_det ( float a[2][2] );
	float rmat2_inverse ( float a[2][2], float b[2][2] );
	float rmat3_det ( float a[3][3] );
	float rmat3_inverse ( float a[3][3], float b[3][3] );
	float rmat4_det ( float a[4][4] );
	float rmat5_det ( float a[5][5] );
	float sphere_imp_volume_3d ( float r );
	float tan_deg ( float angle );
	void  tetra_centroid_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float x4, 
			float y4, float z4, float *xc, float *yc, float *zc );
	float tetra_volume_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float x4, 
			float y4, float z4 );
	void  tmat_init ( float a[4][4] );
	void  tmat_mxm ( float a[4][4], float b[4][4], float c[4][4] );
	void  tmat_mxp ( float a[4][4], float x[4], float y[4] );
	void  tmat_mxp2 ( float a[4][4], float x[][3], float y[][3], int n );
	void  tmat_mxv ( float a[4][4], float x[4], float y[4] );
	void  tmat_rot_axis ( float a[4][4], float b[4][4], float angle, 
			char axis );
	void  tmat_rot_vector ( float a[4][4], float b[4][4], float angle, 
			float v1, float v2, float v3 );
	void  tmat_scale ( float a[4][4], float b[4][4], float sx, float sy, 
			float sz );
	void  tmat_shear ( float a[4][4], float b[4][4], char *axis, float s );
	void  tmat_trans ( float a[4][4], float b[4][4], float x, float y, 
			float z );
	float torus_volume_3d ( float r1, float r2 );
	float triangle_area_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3 );
	float triangle_area_signed_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3 );
	float triangle_area_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3 );
	void  triangle_centroid_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3, float *x, float *y );
	void  triangle_centroid_3d ( float x1, float y1, float z1, float x2, 
			float y2, float z2, float x3, float y3, float z3, float *x, 
			float *y, float *z );
	void  triangle_incircle_2d ( float x1, float y1, float x2, float y2, 
			float x3, float y3, float *xc, float *yc, float *r );
	float uniform_01_sample ( int *iseed );
	void  vector_rotate_2d ( float x1, float y1, float angle, float *x2, 
			float *y2 );
	void  vector_rotate_3d ( float x1, float y1, float z1, float *x2, float *y2, 
			float *z2, float xa, float ya, float za, float angle );
	void  vector_unit_2d ( float *x1, float *y1 );
	void  vector_unit_3d ( float *x1, float *y1, float *z1 );
	void  vector_unit_nd ( int n, float x[] );
END_EXTERN_C

