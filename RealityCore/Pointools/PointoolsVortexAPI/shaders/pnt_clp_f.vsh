!!ARBvp1.0
#**************************************************
# ARB Vertex Program
# Copyright (c) 2004 Pointools Ltd
#--------------------------------------------------
# October 2004 Pointools v1.5
#**************************************************
ATTRIB _p = vertex.position;
ATTRIB _t0 = vertex.texcoord[0];
ATTRIB _t1 = vertex.texcoord[1];
ATTRIB _c = vertex.color;
#params
PARAM Mvp[4] = { state.matrix.mvp };
PARAM Mb[4] = { state.matrix.program[2] };
PARAM Mr[4] = { state.matrix.program[3] };
PARAM Mt[4] = { state.matrix.texture[0]	};
PARAM Mt1[4] = { state.matrix.texture[1]	};
PARAM qS = program.env[0];
PARAM qO = program.env[1];

#vector component of data->project matrix for z values
PARAM vRz = program.env[3];

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

#uncompress vertex
MAD tmp0, _p, qS, qO;

#to normalized box space
DP4 tmp1.x, Mb[0], tmp0;
DP4 tmp1.y, Mb[1], tmp0;
DP4 tmp1.z, Mb[2], tmp0;
DP4 tmp1.w, Mb[3], tmp0;

# Calculate geometry texture coords
	#tranform to project space
#DP4 c0.x, Mr[0], tmp0;
#DP4 c0.y, Mr[1], tmp0;
#DP4 c0.z, Mr[2], tmp0;
#DP4 c0.w, Mr[3], tmp0;

	#compute texture value
#DP4 t1_.x, c0, plane;

#compare to bounds
SGE c0, tmp1, lbd;
SLT c1, tmp1, ubd;

#compute clip coords
DP4 tmp1.x, Mvp[0], _p;
DP4 tmp1.y, Mvp[1], _p;
DP4 tmp1.z, Mvp[2], _p;
DP4 tmp1.w, Mvp[3], _p;

#write the fog coordinate
MOV f_.x, tmp1.z;

#clip point if x,y or z out of range
MUL tmp0, tmp1, c0.x;
MUL tmp1, tmp0, c0.y;
MUL tmp0, tmp1, c0.z;
MUL tmp1, tmp0, c1.x;
MUL tmp0, tmp1, c1.y;

MUL p_, tmp0, c1.z;
# Transform texture coordinates.
DP4 t0_.x, Mt[0], _t0;

#edit mode sel display
DP4 t1_.x, Mt1[0], _t1;

#color
MOV c_, _c;
END