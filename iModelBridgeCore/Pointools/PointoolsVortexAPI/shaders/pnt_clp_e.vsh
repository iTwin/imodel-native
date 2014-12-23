!!ARBvp1.0
#**************************************************
# ARB Vertex Program
# Copyright (c) 2004 Pointools Ltd
#--------------------------------------------------
# September 2008 Pointools v1.7
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
PARAM Mt[4] = { state.matrix.texture[0] };
PARAM qS = program.env[0];
PARAM qO = program.env[1];

#program constants
PARAM ubd = { 1, 1, 1, 1 };
PARAM lbd = { -1, -1, -1, -1 };

#temps
TEMP en, nt, tmp0, tmp1, lc, c0, c1;

#output
OUTPUT p_ = result.position;
OUTPUT c_ = result.color.primary;
OUTPUT t_ = result.texcoord[0];

#uncompress vertex
MAD tmp0, _p, qS, qO;

#to normalized box space
DP4 tmp1.x, Mb[0], tmp0;
DP4 tmp1.y, Mb[1], tmp0;
DP4 tmp1.z, Mb[2], tmp0;
DP4 tmp1.w, Mb[3], tmp0;

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
DP3 nt.x, Me[0], _n;
DP3 nt.y, Me[1], _n;
DP3 nt.z, Me[2], _n;

#take absolute values
ABS en, nt;

# Edge colour
ADD tmp0, en.x, en.y;
POW c0, tmp0.x, e.x; 

#Accumulate color contributions.
MUL c_, _c, c0;

END