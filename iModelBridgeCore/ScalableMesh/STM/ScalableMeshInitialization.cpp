//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshInitialization.cpp $
//:>    $RCSfile: ScalableMeshInitialization.cpp,v $
//:>   $Revision: 1.5 $
//:>       $Date: 2011/01/10 17:36:51 $
//:>     $Author: Raymond.Gauthier $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>

struct Initializer
    {
    explicit                Initializer                    () 
        { 
        try
            {
            OnInit(); 
            }
        catch (...)
            {
            assert(!"Exception thrown in static initializer initialization");
            }
        }
                            ~Initializer                   () 
        { 
        try
            {
            OnFinalize(); 
            }
        catch (...)
            {
            assert(!"Exception thrown in static initializer finalization");
            }
        }
private:

    /*---------------------------------------------------------------------------------**//**
    * @description  Called only once on DLL load
    * @bsimethod                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void                    OnInit                         () const
        {
        // It is assumed that stm dll will be delivered only with MicroStation and thus that rastercore will have initialized ipp.
        BeAssert(Bentley::ImagePP::ImageppLib::IsInitialized());
        //InitializeGeoCoord();
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @description  Called only once on DLL unload
    * @bsimethod                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void                    OnFinalize                     () const
        {

        }


    void                    InitializeGeoCoord             () const
        {
        //Set DLL Path
        //HRFRasterFileFactory::GetInstance()->SetGeocoordPath("basegeocoord.dll");
        //MS : The correct location of the GeoCoord data should be passed by the application 
        //(i.e. : ScalableMesh cannot know where the GeoCoord data have been deployed).
        //HCPGCoordUtility::GetInstance()->InitializeGeoCoord(AString("D:\\BSI\\08.09.00.xx\\out\\Release\\Mstn\\Bentley\\MicroStation\\GeoCoordinateData\\"));
        }

    };
static const Initializer    INITIALIZER_INSTANCE;
