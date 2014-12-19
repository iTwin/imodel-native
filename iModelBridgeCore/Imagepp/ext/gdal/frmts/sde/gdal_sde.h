#ifndef GDAL_SDE_INCLUDED
#define GDAL_SDE_INCLUDED

#include "gdal_pam.h"


CPL_CVSID("$Id: gdal_sde.h 10726 2007-01-30 04:43:45Z hobu $");

CPL_C_START
void    GDALRegister_SDE(void);


CPL_C_END

#include <sdetype.h>
#include <sdeerno.h>
#include <sderaster.h>

#include "cpl_string.h"
#include "ogr_spatialref.h"

#include "sdeerror.h"
#include "sderasterband.h"
#include "sdedataset.h"

#endif
