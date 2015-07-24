/*--------------------------------------------------------------------------------------+
|
|     $Source: BaseGeoCoord/transkit/baseGeoCoordMsg.r $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <RmgrTools\Tools\rtypes.r.h>
#include <GeoCoord\basegeodefs.r.h>

/*----------------------------------------------------------------------+
|                                                                       |
|   Messages                                                            |
|                                                                       |
+----------------------------------------------------------------------*/
MessageList MSGLISTID_GeoCoordErrors =
{
    {
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidCoordSys),                      "Invalid Geographic Coordinate System" },
    { (GeoCoordErrorBase - GEOCOORDERR_BadArg),                               "Invalid argument" },
    { (GeoCoordErrorBase - GEOCOORDERR_LibraryReadonly),                      "Coordinate System Library is ReadOnly" },
    { (GeoCoordErrorBase - GEOCOORDERR_IOError),                              "I/O Error reading or writing Coordinate System Library" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordSysNotFound),                     "Coordinate System not found" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordSysNoUniqueName),                 "Coordinate System name is not unique" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordSysIllegalName),                  "Coordinate System name has characters that are not allowed" },
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidCoordinateCode),                "Coordinate System projection not found" },
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidUnitCode),                      "Coordinate System units not found" },
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidDatumCode),                     "Coordinate System datum not found" },
    { (GeoCoordErrorBase - GEOCOORDERR_CantSetEllipsoid),                     "Coordinate System Ellipsoid can't be changed, datum specified" },
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidEllipsoidCode),                 "Coordinate System Ellipsoid not found" },
    { (GeoCoordErrorBase - GEOCOORDERR_ProjectionDoesntUseParameter),         "Coordinate System does not require use this parameter" },
    { (GeoCoordErrorBase - GEOCOORDERR_NoModelContainsGCS),                   "No Model in the file contains a Geographic Coordinate System" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordinateRange),                      "The coordinates are outside the valid range of the GCS" },
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidAffineParameters),              "The affine parameters are invalid. They represent an invalid transformation" },

    { (GeoCoordErrorBase - GEOCOORDERR_GeocentricNotSupported),               "Geocentric GeoTiff Model not supported" },
    { (GeoCoordErrorBase - GEOCOORDERR_IncompleteGeoTiffSpec),                "The GeoTiff specification was not complete" },
    { (GeoCoordErrorBase - GEOCOORDERR_UnexpectedGeoTiffModelType),           "Unexpected GeoTiff ModelType value or missing ModelType key" },
    { (GeoCoordErrorBase - GEOCOORDERR_UnexpectedGeoTiffPrimeMeridian),       "Unexpected GeoTiff Prime Meridian key value" },
    { (GeoCoordErrorBase - GEOCOORDERR_UnrecognizedLinearUnit),               "Unrecognized GeoTiff Linear Unit" },
    { (GeoCoordErrorBase - GEOCOORDERR_UnrecognizedAngularUnit),              "Unrecognized GeoTiff Angular Unit" },
    { (GeoCoordErrorBase - GEOCOORDERR_BadEllipsoidDefinition),               "Invalid GeoTiff Ellipsoid Definition" },
    { (GeoCoordErrorBase - GEOCOORDERR_ProjectionGeoKeyNotSupported),         "The GeoTiff 'Projection' key is not supported" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordTransNotSupported),               "The Projection specified in the GeoTiff key is not supported" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordParamNotNeededForTrans),          "A GeoTiff key specified a projection parameter that is not needed for the specified projection" },
    { (GeoCoordErrorBase - GEOCOORDERR_ProjectionParamNotSupported),          "The value specified in the GeoTiff Center key is not supported" },
    { (GeoCoordErrorBase - GEOCOORDERR_CoordSysSpecificationIncomplete),      "The GeoTiff keys did not specify all the parameters needed" },

    { (GeoCoordErrorBase - GEOCOORDERR_CantSaveGCS),                          "The Geographic CS uses a Projection that cannot be saved to GeoTiff" },
    { (GeoCoordErrorBase - GEOCOORDERR_InvalidGeographicEPSGCode),            "The Geographic CS EPSG code is not valid in GeoTiff" },
    }
};

MessageList MSGLISTID_GeoCoordNames =
{
    {
    { COORDSYS_ALBER,           "Albers' Equal Area Conic"                          },
    { COORDSYS_AZMEA,           "Azimuthal Equal Area"                              },
    { COORDSYS_AZMED,           "Azimuthal Equidistant"                             },
    { COORDSYS_BONNE,           "Bonne"                                             },
    { COORDSYS_BPCNC,           "Bipolar Conic"                                     },
    { COORDSYS_CSINI,           "Cassini"                                           },
    { COORDSYS_EDCNC,           "Equidistant Conic"                                 },
    { COORDSYS_EDCYL,           "Equidistant Cylindrical"                           },
    { COORDSYS_EKRT4,           "Eckert 4"                                          },
    { COORDSYS_EKRT6,           "Eckert 6"                                          },
    { COORDSYS_GNOMC,           "Gnomonic"                                          },
    { COORDSYS_HMLSN,           "Homolsine"                                         },
    { COORDSYS_LMBRT,           "Lambert Conformal Conic"                           },
    { COORDSYS_LMTAN,           "Lambert Tangential"                                },
    { COORDSYS_MILLR,           "Miller"                                            },
    { COORDSYS_MODPC,           "Modified Polyconic"                                },
    { COORDSYS_MOLWD,           "Mollweide"                                         },
    { COORDSYS_MRCAT,           "Mercator"                                          },
    { COORDSYS_MSTRO,           "Modified Stereographic"                            },
    { COORDSYS_NACYL,           "Normal Equal Area Cylindrical"                     },
    { COORDSYS_NZLND,           "New Zealand"                                       },
    { COORDSYS_OBLQ1,           "Oblique 1 Point"                                   },
    { COORDSYS_OBLQ2,           "Oblique 2 Points"                                  },
    { COORDSYS_ORTHO,           "Orthographic"                                      },
    { COORDSYS_PLYCN,           "Polyconic"                                         },
    { COORDSYS_ROBIN,           "Robinson"                                          },
    { COORDSYS_SINUS,           "Sinusoidal"                                        },
    { COORDSYS_STERO,           "Stereographic"                                     },
    { COORDSYS_TACYL,           "Transverse Equal Area Cylindrical"                 },
    { COORDSYS_TRMER,           "Transverse Mercator"                               },
    { COORDSYS_UNITY,           "Geographic (Latitude/Longitude)"                   },
    { COORDSYS_VDGRN,           "Van Der Grinten"                                   },

    { COORDSYS_UTMZN,           "Universal Transverse Mercator"                     },
    { COORDSYS_LM1SP,           "Lambert Conformal Conic 1 Standard Latitude"       },
    { COORDSYS_OSTRO,           "Oblique Stereographic"                             },
    { COORDSYS_PSTRO,           "Polar Stereographic"                               },
    { COORDSYS_RSKWC,           "Rectified Skew Orthomorphic (Center)"              },
    { COORDSYS_RSKEW,           "Rectified Skew Orthomorphic (Intersect)"           },
    { COORDSYS_SWISS,           "Swiss Oblique Cylindrical"                         },
    { COORDSYS_LMBLG,           "Lambert Conformal Conic (Belgian Variation)"       },
    { COORDSYS_SOTRM,           "South Orientated Transverse Mercator (RSA)"        },
    { COORDSYS_HOM1U,           "Oblique 1 Point (Unrectified)"                     },
    { COORDSYS_HOM2U,           "Oblique 2 Points (Unrectified)"                    },

    { COORDSYS_GAUSK,           "Gauss-Kruger"                                      },
    { COORDSYS_KRVKP,           "Krovak (precise origin)"                           },
    { COORDSYS_KRVKR,           "Krovak (rounded origin)"                           },
    { COORDSYS_MRCSR,           "Mercator with scale reduction"                     },
    { COORDSYS_OCCNC,           "OCCNC"                                             },
    { COORDSYS_KRVKG,           "Krovak (generalized)"                              },
    { COORDSYS_TRMAF,           "Transverse Mercator with Affine Processor"         },
    { COORDSYS_PSTSL,           "Polar Stereographic Standard Latitude"             },
    { COORDSYS_NERTH,           "Non-earth"                                         },

    { COORDSYS_HUEOV,           "Hungarian EOV"                                     },
    { COORDSYS_SYS34,           "Danish System 1934/1945 (Non-KMS)"                 },
    { COORDSYS_OST97,           "Ordinance Survey Transform 1997 (Great Britain)"   },
    { COORDSYS_OST02,           "Ordinance Survey Transform 2002 (Great Britain)"   },
    { COORDSYS_S3499,           "Danish System 1934/1945 (KMS 1999 Polynomials)"    },
    { COORDSYS_AZEDE,           "Azimuthal Equidistant (Elevated Ellipsoid)"        }, 
    { COORDSYS_LMMIN,           "Lambert Minnesota County"                          },
    { COORDSYS_LMWIS,           "Lambert Wisconsin County"                          },
    { COORDSYS_TMMIN,           "Transverse Mercator Minnesota DOT"                 },
    { COORDSYS_TMWIS,           "Transverse Mercator Wisconsin County"              },
    { COORDSYS_RSKWO,           "Rectified Skew Orthomorphic (Azimuth at Origin)"   },
    { COORDSYS_WINKT,           "Winkel-Tripel"                                     },
    { COORDSYS_TMKRG,           "Transverse Mercator Kruger Formulation"            },
    { COORDSYS_NESRT,           "Non-earth - Scale, Rotation then Translation"      },
    { COORDSYS_LMBRTAF,         "Lambert Conformal Conic with Affine Processor"     },

    { COORDSYS_UTMZNBF,         "Universal Transverse Mercator using BF Calculation" },
    { COORDSYS_TRMERBF,         "Transverse Mercator using BF Calculation"          },

    { COORDSYS_S3401,           "Danish System 1934/1945 (KMS 2001 Polynomials)"    },
    { COORDSYS_EDCYLE,          "Equidistant Cylindrical Projection (Ellipsoidal or Spherical)" },
    { COORDSYS_PCARREE,         "Plate Carree / Simple Cylindrical"                 },
    { COORDSYS_MRCATPV,         "Popular Visualization Pseudo Mercator"             },
    { COORDSYS_MNDOTOBL,        "Oblique Mercator Minnesota DOT"                    },                   
    }
};

MessageList    MSGLISTID_DgnGeoCoordStrings =
{
    {
    { DGNGEOCOORD_Msg_ElementTypeName,                        "Geographic Coordinate System"                          },
    { DGNGEOCOORD_Msg_ReprojectingCoordinateData,             "Reprojecting Coordinate Data"                          },
    { DGNGEOCOORD_Msg_ReprojectedPointsWithErrors,            "%ls:%ls reprojected, errors detected (%5.3f seconds)"    },
    { DGNGEOCOORD_Msg_ReprojectedPointsWithWarnings,          "%ls:%ls reprojected, warnings detected (%5.3f seconds)"  },
    { DGNGEOCOORD_Msg_ReprojectedPoints,                      "%ls:%ls reprojected (%5.3f seconds)"                     },
    { DGNGEOCOORD_Msg_PointsReprojectedDetail,                "%d points reprojected (%d points added), %d transforms calculated" },
    { DGNGEOCOORD_Msg_DomainErrors,                           "Error: %d points outside mathematical range of GCS"    },
    { DGNGEOCOORD_Msg_UsefulRangeErrors,                      "Warning: %d points outside useful range of GCS"        },
    { DGNGEOCOORD_Msg_OtherErrors,                            "Warning: %d other errors"                              },
    { DGNGEOCOORD_Msg_DatumError,                             "Can not convert from datum %ls to datum %ls, unable to reproject %ls:%ls" },
    { DGNGEOCOORD_Msg_GeoCoordACSType,                        "Geographic"                                            },
    { DGNGEOCOORD_Msg_NoGeoCoordinateSystem,                  "No Geographic Coordinate System is associated with this data"         },
    { DGNGEOCOORD_Msg_PointFromStringRequiresBoth,            "Both Latitude and Longitude are required"              },
    { DGNGEOCOORD_Msg_UnparseableInputAngle,                  "Cannot parse input angle : "                           },
    { DGNGEOCOORD_Msg_UnparseableInputElevation,              "Cannot parse input elevation : "                       },
    { DGNGEOCOORD_Msg_SubstituteLinearTransform,              "%ls:%ls transformed. A Linear Transform was used that gives the same results as reprojection. " },
    { DGNGEOCOORD_Msg_SubstituteLinearTransformDetails,       "The parent and reference have the same base Geographic Coordinate System. Therefore, it is possible to calculate a Linear Transform that achieves the same result as reprojection, but with much better performance. " },
    { DGNGEOCOORD_Msg_MiltaryGridOldCoordinatesName,          "Old Military Grid Coordinates"                          },
    { DGNGEOCOORD_Msg_MiltaryGridCoordinatesName,             "Military Grid Coordinates"                              },
    { DGNGEOCOORD_Msg_MiltaryGridCoordinatesWGS84Name,        "Military Grid Coordinates WGS84"                        },
    { DGNGEOCOORD_Msg_USNationalGridName,                     "US National Grid"                                       },
    { DGNGEOCOORD_Msg_MiltaryGridOldCoordinatesDescription,   "Old-style Military Grid Coordinates, using Bessell or Clarke Ellipsoid" },
    { DGNGEOCOORD_Msg_MiltaryGridCoordinatesDescription,      "Military Grid Coordinates, using this Model's Datum"    },
    { DGNGEOCOORD_Msg_MiltaryGridCoordinatesWGS84Description, "Military Grid Coordinates, WGS84 Datum"                 },
    { DGNGEOCOORD_Msg_USNationalGridDescription,              "US National Grid Coordinates (WGS84 Datum)"             },
    { DGNGEOCOORD_Msg_MilitaryGridACSType,                    "Military Grid"                                          }, 

    { DGNGEOCOORD_Msg_MilitaryGridNotRelative,                "Military Grid coordinates cannot be relative"           },
    { DGNGEOCOORD_Msg_MilitaryGridNotDelta,                   "Cannot report distances in Military Grid coordinates"   },
    { DGNGEOCOORD_Msg_CantConvertToMilitaryGrid,              "Could not convert point to Military Grid coordinates"   },
    { DGNGEOCOORD_Msg_CantConvertFromMilitaryGrid,            "Input string is not valid a Military Grid coordinate."  },
    { DGNGEOCOORD_Msg_DatumConvertNotSetErrors,               "Warning or Error: Datum converter could not be set and resulted in %d points being possibly incorrectly reprojected; this may be the result of grid shift files not being properly installed."  },
    { DGNGEOCOORD_Msg_VerticalConvertErrors,                  "Warning or Error: Vertical datum conversion errors occured for %d points for which elevation may have been incorrectly adjusted; this may be the result of Geoid or VERTCON files not being properly installed or configured."  },
    


    }
};
