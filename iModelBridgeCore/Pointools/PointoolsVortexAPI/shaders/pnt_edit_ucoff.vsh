!!ARBvp1.0
#**************************************************
# ARB Vertex Program
# Copyright (c) 2004 Pointools Ltd
#--------------------------------------------------
# October 2007 Pointools v1.7
#**************************************************
ATTRIB _p = vertex.position;
ATTRIB _t = vertex.texcoord[0];
ATTRIB _c = vertex.color;
ATTRIB _off = vertex.texcoord[1];

#params
PARAM Mvp[4] = { state.matrix.mvp };
PARAM Mt[4] = { state.matrix.texture[0]	};
PARAM qS = program.env[0];
PARAM qO = program.env[1];
PARAM iqS = program.env[3];
PARAM clearW = { 1,1,1,0 };
PARAM reduce = { 1, 1, 0.2, 1 };
PARAM blend = program.local[0];

#Temporaries
TEMP tmp0, tmp1, s, c0, c1, dist;

#Output
OUTPUT p_ = result.position;
OUTPUT t0_ = result.texcoord[0];
OUTPUT c_ = result.color;

# Transform intensity texture coords
DP4 t0_.x, Mt[0], _t;

# Calculate vertex with offset
#uncompress vertex
MAD tmp1, _p,qS, qO;
MOV tmp0, _off;
MUL tmp0, tmp0, blend;
ADD tmp1, tmp0, tmp1;

#tranform 
DP4 tmp0.x, Mvp[0], tmp1;
DP4 tmp0.y, Mvp[1], tmp1;
DP4 tmp0.z, Mvp[2], tmp1;
DP4 tmp0.w, Mvp[3], tmp1;

MOV p_, tmp0;

DP3 dist.x, _off, _off;

#MOV tmp1, _c;
#MUL tmp1.x, reduce.z, dist.x;
#MOV c_, tmp1;

#color
MOV c_, _c;
END