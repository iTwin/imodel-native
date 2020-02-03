!!ARBvp1.0
#**************************************************
# ARB Vertex Program
# Copyright (c) 2008 Pointools Ltd
#--------------------------------------------------
# May 2008 Pointools v1.7
#**************************************************
ATTRIB _p = vertex.position;
ATTRIB _t0 = vertex.texcoord[0];
ATTRIB _t1 = vertex.texcoord[1];
ATTRIB _c = vertex.color;
ATTRIB _sel = vertex.attrib[1];
ATTRIB _n = vertex.normal;

#params
PARAM Mvp[4] = { state.matrix.mvp };
PARAM Mt[4] = { state.matrix.texture[0] };
PARAM Mt1[4] = { state.matrix.texture[1] };
PARAM Me[4] = { state.matrix.program[1].inverse };
PARAM Mr[4] = { state.matrix.program[2] };

#program constants
PARAM k_sel = { 0.4, 0, 0, 0 };
PARAM k_hide = { 0.40, 0, 0, 0 };
PARAM sel_col = { 1.0, 0.5, 0.2, 1.0 };
PARAM one= { 1.0, 1.0, 1.0, 1.0 };
PARAM tone= { 0.8, 1.0, 1.0, 1.0 };
PARAM twofivesix = { 128.0, 0.5, 0, 0 };

PARAM qS = program.env[0];
PARAM qO = program.env[1];
PARAM plane = program.env[2];

#light params
PARAM ld = state.light[0].position;
PARAM hd = state.light[0].half;

#material params
PARAM sp = state.material.shininess;

PARAM dif = state.lightprod[0].diffuse;
PARAM spc = state.lightprod[0].specular;
PARAM amb = state.lightprod[0].ambient;

#Temporaries
TEMP tmp0, tmp1, tmp2, c0, c1, hide, en, dt, lc,editColResult;

#Output
OUTPUT p_ = result.position;
OUTPUT t0_ = result.texcoord[0];
OUTPUT t1_ = result.texcoord[1];
OUTPUT c_ = result.color;
OUTPUT c2_ = result.color.secondary;
OUTPUT f_ = result.fogcoord;

# Transform texture coordinates.
DP4 t0_.x, Mt[0], _t0;

#compare to sel value
#SLT tmp1, _sel.x, k_sel.x;
MUL tmp1.x, _sel.x, twofivesix.x;
FRC tmp2.x, tmp1.x;
SGE tmp1.x, tmp2.x, k_sel.x;

#invert comparision of hide point 
SGE hide.x, k_hide.x, _sel.x;

#tone down the inclusion
MUL tmp0, tmp1, tone;

#and blend with colour
MUL tmp2, sel_col, tmp0.x;
SUB tmp1, one, tmp0.x;
MUL tmp0, tmp1, _c;

ADD editColResult, tmp0, tmp2; 

#compute clip coords
DP4 tmp1.x, Mvp[0], _p;
DP4 tmp1.y, Mvp[1], _p;
DP4 tmp1.z, Mvp[2], _p;
DP4 tmp1.w, Mvp[3], _p;

#write the fog coordinate
DP4 f_.x, Mvp[2], _p;

MUL tmp2, tmp1, hide.x;

#clip point if x,y or z out of range
MOV p_, tmp2;

#lighting ---------------------------------
# Transform the normal into eye space.
DP3 en.x, Me[0], _n;
DP3 en.y, Me[1], _n;
DP3 en.z, Me[2], _n;

# compute lighting
DP3 dt.x, en, ld;
DP3 dt.y, en, hd;
MOV dt.w, sp.x;
LIT lc, dt;

# Plane shader -----------------------------
	#uncompress vertex z
MAD dt, _p, qS, qO;
	#tranform to project space
DP4 tmp0.x, Mr[0], dt;
DP4 tmp0.y, Mr[1], dt;
DP4 tmp0.z, Mr[2], dt;
DP4 tmp0.w, Mr[3], dt;
	#compute texture value
DP4 t1_.x, tmp0, plane;

#color -------------------------------------
# Accumulate color contributions.
MUL en, amb, editColResult;
MUL dt, dif, editColResult;
MAD tmp0, lc.y, dt, en;
MAD c_.xyz, lc.z, spc, tmp0;
MOV c_.w, dif.w;

#secondary spec color
MUL c2_.xyz, lc.z, spc;
MOV c2_.w, dif.w;
END