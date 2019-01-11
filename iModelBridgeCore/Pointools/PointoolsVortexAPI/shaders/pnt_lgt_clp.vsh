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
#params
PARAM Mvp[4] = { state.matrix.mvp 	};
PARAM Me[4] = { state.matrix.program[1].inverse };
PARAM Mb[4] = { state.matrix.program[2] };
PARAM Mr[4] = { state.matrix.program[3] };
PARAM Mt[4] = { state.matrix.texture[0]	};
PARAM qS = program.env[0];
PARAM qO = program.env[1];
PARAM plane = program.env[2];

#program constants
PARAM ubd = { 1, 1, 1, 1 };
PARAM lbd = { -1, -1, -1, -1 };

#light params
PARAM ld = state.light[0].position;
PARAM hd = state.light[0].half;
PARAM sp = state.material.shininess;

#material / light products
PARAM amb = state.lightprod[0].ambient;
PARAM dif = state.lightprod[0].diffuse;
PARAM spc = state.lightprod[0].specular;

#temps
TEMP en, tmp0, tmp1, lc, c0, c1;
#output
OUTPUT p_ = result.position;
OUTPUT c_ = result.color.primary;
OUTPUT t_ = result.texcoord[0];
OUTPUT t1_ = result.texcoord[1];
OUTPUT c2_ = result.color.secondary;

#uncompress vertex
MAD tmp0, _p, qS, qO;

#to normalized box space
DP4 tmp1.x, Mb[0], tmp0;
DP4 tmp1.y, Mb[1], tmp0;
DP4 tmp1.z, Mb[2], tmp0;
DP4 tmp1.w, Mb[3], tmp0;

# Calculate geometry texture coords
	#tranform to project space
DP4 c0.x, Mr[0], tmp0;
DP4 c0.y, Mr[1], tmp0;
DP4 c0.z, Mr[2], tmp0;
DP4 c0.w, Mr[3], tmp0;

	#compute texture value
DP4 t1_.x, c0, plane;

#compare to bounds
SGE c0, tmp1, lbd;
SLT c1, tmp1, ubd;

#compute clip coords
DP4 tmp1.x, Mvp[0], _p;
DP4 tmp1.y, Mvp[1], _p;
DP4 tmp1.z, Mvp[2], _p;
DP4 tmp1.w, Mvp[3], _p;


#clip point if x,y or z out of range
MUL tmp0, tmp1, c0.x;
MUL tmp1, tmp0, c0.y;
MUL tmp0, tmp1, c0.z;
MUL tmp1, tmp0, c1.x;
MUL tmp0, tmp1, c1.y;
MUL p_, tmp0, c1.z;

# Transform texture coordinates.
DP4 t_.x, Mt[0], _t;

# Transform the normal into eye space.
DP3 en.x, Me[0], _n;
DP3 en.y, Me[1], _n;
DP3 en.z, Me[2], _n;

# lighting
DP3 tmp1.x, en, ld;
DP3 tmp1.y, en, hd;
MOV tmp1.w, sp.x;
LIT lc, tmp1;

# Accumulate color contributions.
# Accumulate color contributions.
MUL tmp0, amb, _c;
MUL tmp1, dif, _c;
MAD en, lc.y, tmp1, tmp0;
MAD c_.xyz, lc.z, spc, en;
MOV c_.w, dif.w;

MUL c2_.xyz, lc.z, spc;
MOV c2_.w, dif.w;
END