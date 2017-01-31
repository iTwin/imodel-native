\B=#include <msbsplineMaster.h>\n\n

! strip off all #includes....
\L\n\W\#include<u>=
\Ifc_10\I=10.0
\Ifc_1\I=1.0
\Ifc_m1\I=\S-1.0\S
\Ifc_pi\I=\SmsGeomConst_pi\S
\Ifc_2pi\I=\SmsGeomConst_2pi\S

MdlPublic=Public
BsiPublic=Public


\ImdlRMatrix_getIdentity\I=bsiRotMatrix_initIdentity
\ImdlTMatrix_isIdentity\I=bsiTransform_isIdentity
\nPublic\I=\nPublic
?=?