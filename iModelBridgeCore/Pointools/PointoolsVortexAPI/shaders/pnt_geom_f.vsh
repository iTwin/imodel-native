!!ARBvp1.0
#**************************************************
# ARB Vertex Program
# Copyright (c) 2004 Pointools Ltd
#--------------------------------------------------
# October 2004 Pointools v1.5
#**************************************************
ATTRIB _p = vertex.position;
ATTRIB _t = vertex.texcoord[0];
ATTRIB _c = vertex.color;
#params
PARAM Mvp[4] = { state.matrix.mvp };
PARAM Mr[4] = { state.matrix.program[2] };
PARAM Mt[4] = { state.matrix.texture[0]	};
PARAM qS = program.env[0];
PARAM qO = program.env[1];
PARAM plane = program.env[2];
#program constants
PARAM ubd = { 1, 1, 1, 1 };
PARAM lbd = { -1, -1, -1, -1 };
#Temporaries
TEMP tmp0, tmp1, c0, c1;
#Output
OUTPUT p_ = result.position;
OUTPUT t0_ = result.texcoord[0];
OUTPUT t1_ = result.texcoord[1];
OUTPUT c_ = result.color;
OUTPUT f_ = result.fogcoord;

#compute clip coords
DP4 p_.x, Mvp[0], _p;
DP4 p_.y, Mvp[1], _p;
DP4 p_.z, Mvp[2], _p;
DP4 p_.w, Mvp[3], _p;

#write the fog coordinate
DP4 f_.x, Mvp[2], _p;

# Transform intensity texture coords
DP4 t0_.x, Mt[0], _t;
# Calculate geometry texture coords
	#uncompress vertex
MAD tmp0, _p, qS, qO;
	#tranform to plane space
DP4 tmp1.x, Mr[0], tmp0;
DP4 tmp1.y, Mr[1], tmp0;
DP4 tmp1.z, Mr[2], tmp0;
DP4 tmp1.w, Mr[3], tmp0;
	#compute texture value
DP4 t1_.x, tmp1, plane;
#color
MOV c_, _c;
END