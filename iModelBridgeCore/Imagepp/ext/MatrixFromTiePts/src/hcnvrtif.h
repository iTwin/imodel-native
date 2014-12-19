#ifndef __HCNVRTIF_H__
#define __HCNVRTIF_H__

#include "oldhtypes.h"
#include "matrix.h"

HSTATUS sHConvertTIF_MakeModelerFromScaleAndTiePts (pMAT_   po_pMatrix,
                                                    uint16_t       pi_NbVal_GeoTiePoint,
                                                    double        *pi_pVal_GeoTiePoint,
                                                    uint16_t       pi_NbVal_GeoPixelScale,
                                                    double        *pi_pVal_GeoPixelScale);

#endif