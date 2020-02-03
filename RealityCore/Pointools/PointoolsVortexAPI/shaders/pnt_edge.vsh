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

PARAM k = { 0, 1, 1, 1 };

PARAM e = program.env[2];

#temps
TEMP en, nt, tmp, c;

#outputs
OUTPUT p_ = result.position;
OUTPUT c_ = result.color.primary;
OUTPUT t_ = result.texcoord[0];
OUTPUT t1_ = result.texcoord[1];

# Transform the vertex to clip coordinates.
DP4 p_.x, Mvp[0], _p;
DP4 p_.y, Mvp[1], _p;
DP4 p_.z, Mvp[2], _p;
DP4 p_.w, Mvp[3], _p;

# Transform texture coordinates.
DP4 t_.x, Mt[0], _t;

# Transform the normal into eye space.
DP3 nt.x, Me[0], _n;
DP3 nt.y, Me[1], _n;
DP3 nt.z, Me[2], _n;

#take absolute values
ABS en, nt;

# Edge colour
ADD tmp, en.x, en.y;
POW c, tmp.x, e.x; 

#Accumulate color contributions.
MUL c_, _c, c;

END