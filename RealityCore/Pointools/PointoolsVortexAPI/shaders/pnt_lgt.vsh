!!ARBvp1.0
#**************************************************
# ARB Vertex Program
# Copyright (c) 2004 Pointools Ltd
#--------------------------------------------------
# October 2004 Pointools v1.5
#**************************************************
ATTRIB _p = vertex.position;
ATTRIB _n = vertex.normal;
ATTRIB _c = vertex.color;
ATTRIB _t = vertex.texcoord[0];

#matrices
PARAM Mvp[4] = { state.matrix.mvp };
PARAM Mt[4] = { state.matrix.texture[0] };
PARAM Me[4] = { state.matrix.program[1].inverse };
PARAM Mr[4] = { state.matrix.program[2] };

#light params
PARAM ld = state.light[0].position;
PARAM hd = state.light[0].half;

#material params
PARAM sp = state.material.shininess;

PARAM dif = state.lightprod[0].diffuse;
PARAM spc = state.lightprod[0].specular;
PARAM amb = state.lightprod[0].ambient;

#Geometry params
PARAM qS = program.env[0];
PARAM qO = program.env[1];
PARAM plane = program.env[2];

#temps
TEMP en, tmp, dt, lc;

#outputs
OUTPUT p_ = result.position;
OUTPUT c_ = result.color.primary;
OUTPUT t_ = result.texcoord[0];
OUTPUT t1_ = result.texcoord[1];
OUTPUT c2_ = result.color.secondary;

# Transform the vertex to clip coordinates.
DP4 p_.x, Mvp[0], _p;
DP4 p_.y, Mvp[1], _p;
DP4 p_.z, Mvp[2], _p;
DP4 p_.w, Mvp[3], _p;

# Transform texture coordinates.
DP4 t_.x, Mt[0], _t;

# Transform the normal into eye space.
DP3 en.x, Me[0], _n;
DP3 en.y, Me[1], _n;
DP3 en.z, Me[2], _n;

# compute lighting
DP3 dt.x, en, ld;
DP3 dt.y, en, hd;
MOV dt.w, sp.x;
LIT lc, dt;

# Calculate geometry texture coords
	#uncompress vertex z
MAD dt, _p, qS, qO;
	#tranform to project space
DP4 tmp.x, Mr[0], dt;
DP4 tmp.y, Mr[1], dt;
DP4 tmp.z, Mr[2], dt;
DP4 tmp.w, Mr[3], dt;
	#compute texture value
DP4 t1_.x, tmp, plane;

# Accumulate color contributions.
MUL en, amb, _c;
MUL dt, dif, _c;
MAD tmp, lc.y, dt, en;
MAD c_.xyz, lc.z, spc, tmp;
MOV c_.w, dif.w;

#secondary spec color
MUL c2_.xyz, lc.z, spc;
MOV c2_.w, dif.w;
END