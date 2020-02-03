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

#params
PARAM Mvp[4] = { state.matrix.mvp };
PARAM Mt[4] = { state.matrix.texture[0] };
PARAM Mt1[4] = { state.matrix.texture[1] };

#program constants
PARAM k_sel = { 0.4, 0, 0, 0 };
PARAM k_hide = { 0.40, 0, 0, 0 };
PARAM sel_col = { 1.0, 0.5, 0.2, 1.0 };
PARAM one= { 1.0, 1.0, 1.0, 1.0 };
PARAM tone= { 0.8, 1.0, 1.0, 1.0 };
PARAM twofivesix = { 128.0, 0.5, 0, 0 };

#Temporaries
TEMP tmp0, tmp1, tmp2, c0, c1, hide;

#Output
OUTPUT p_ = result.position;
OUTPUT t0_ = result.texcoord[0];
OUTPUT t1_ = result.texcoord[1];
OUTPUT c_ = result.color;

# Transform texture coordinates.
DP4 t0_.x, Mt[0], _t0;

#compare to sel value
#SLT tmp1, _sel.x, k_sel.x;
MUL tmp1.x, _sel.x, twofivesix.x;
FRC tmp2.x, tmp1.x;
SGE tmp1.x, tmp2.x, k_sel.x;

#invert comparision of hide point 
SGE hide.x, k_hide.x, _sel.x;
#SUB hide.x, one.x, tmp0.x;

#tone down the inclusion
MUL tmp0, tmp1, tone;

#and blend with colour
MUL tmp2, sel_col, tmp0.x;
SUB tmp1, one, tmp0.x;
MUL tmp0, tmp1, _c;

ADD c_, tmp0, tmp2; 

#compute clip coords
DP4 tmp1.x, Mvp[0], _p;
DP4 tmp1.y, Mvp[1], _p;
DP4 tmp1.z, Mvp[2], _p;
DP4 tmp1.w, Mvp[3], _p;

MUL tmp2, tmp1, hide.x;

#clip point if x,y or z out of range
MOV p_, tmp2;

#color
#MOV c_, _c;
END