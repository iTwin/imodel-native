/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMPond.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include < vcclr.h >
#include ".\dtm.h"
#include ".\dtmexception.h"

#if defined(Public)

#undef Public
#endif

#using <mscorlib.dll>

using namespace System;
using namespace System::Runtime::InteropServices;
namespace SRI = System::Runtime::InteropServices;


BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

///<summary>Pond Design Criteria - designs a pond given parameters</summary>
///<author>james.goode</author>
///<date>6/2011</date>
  
public ref class DTMPondDesignCriteria
    {

    public:

        ///<summary> Enum for pond target - voume or elevation</summary>
        enum class DTMPondTargetEnum
            {
            Volume = 1,
            Elevation = 2
            };

        ///<summary> Enum for pond reference points - perimeter or invert</summary>
        enum class DTMPondDesignMethodEnum
            {
            BottomUp = 1,
            TopDown = 2,        
            };

        ///<summary> Enum for the pond result</summary>
        enum class DTMPondResult
            {
            TargetObtained = 0,
            TargetVolumeLessThanPondBoundary = 1,
            TargetVolumeExceedsPondMaxVolume = 2,
            NoDesignCriteria = 3,
            NoReferencePoints = 4,
            UnknownError = 5
            };        

    private:

        array<BGEO::DPoint3d>^ m_points;
        double m_sideSlope;
        double m_freeBoard;
        DTMPondTargetEnum m_pondTarget;
        DTMPondDesignMethodEnum  m_designMethod;
        double m_targetElevation;
        double m_targetVolume;

        double m_pondElevation;
        double m_pondVolume;
        DTMPondResult m_lastResult;

    public:

        DTMPondDesignCriteria()
            {
            // Set default params
            m_sideSlope = 1.0;
            m_freeBoard = 0.0;
            m_pondTarget = DTMPondTargetEnum::Volume;
            m_targetVolume = 100000.0;
            m_targetElevation = 0.0;

            m_pondElevation = 0.0;
            m_pondVolume = 0.0;
            m_lastResult = DTMPondResult::TargetObtained;

            };   

        DTMPondDesignCriteria(DTMPondDesignMethodEnum perimeterOrInvert, array<BGEO::DPoint3d>^ perimerOrInvertPoints,  double sideSlope, double freeBoard, DTMPondTargetEnum pondTarget, double target)
            {
            // Set design params
            m_designMethod = perimeterOrInvert;
            m_points = perimerOrInvertPoints;
            m_sideSlope = sideSlope;
            m_freeBoard = freeBoard;
            m_pondTarget = pondTarget;

            if (m_pondTarget == DTMPondTargetEnum::Elevation)
                m_targetElevation = target;
            else
                m_targetVolume = target;   

            m_pondElevation = 0.0;
            m_pondVolume = 0.0;
            m_lastResult = DTMPondResult::TargetObtained;

            }

        property double SideSlope
            {
            double get()
                {
                return m_sideSlope;
                }
            void set(double value)
                {
                m_sideSlope = value;
                }                
            }

        property double FreeBoard
            {
            double get()
                {
                return m_freeBoard;
                }
            void set(double value)
                {
                m_freeBoard = value;
                }
            }

        property DTMPondTargetEnum PondTarget
            {
            DTMPondTargetEnum get()
                {
                return m_pondTarget;
                }
            void set(DTMPondTargetEnum value)
                {
                m_pondTarget = value;
                }
            }

        property DTMPondDesignMethodEnum PondDesignMethod
            {
            DTMPondDesignMethodEnum get()
                {
                return m_designMethod;
                }
            void set(DTMPondDesignMethodEnum value)
                {
                m_designMethod = value;
                }
            }

        property double TargetVolume
            {
            double get()
                {
                return m_targetVolume;
                }
            void set(double value)
                {
                m_targetVolume = value;
                }
            }

        property double TargetElevation
            {
            double get()
                {
                return m_targetElevation;
                }
            void set(double value)
                {
                m_targetElevation = value;
                }
            }

        void SetReferencePoints(array<BGEO::DPoint3d>^ points)
            {
            m_points = points;
            }

        property array<BGEO::DPoint3d>^ ReferencePoints
            {
            array<BGEO::DPoint3d>^ get()
                {
                return m_points;
                }
            }

        property double AchievedPondElevation
            {
            double get()
                {
                return m_pondElevation;
                }
            }

        property double AchievedPondVolume
            {
            double get()
                {
                return m_pondVolume;
                }
            }

        property DTMPondResult LastResult
            {
            DTMPondResult get()
                {
                return m_lastResult;
                }
            }

        DTMPondResult CreatePond([Out] DTM^% pondDTM)
            {                               

            // No reference points - throw out the call
            if (m_points->Length==0)
                return DTMPondResult::NoReferencePoints;
          
            // Make the call...                
            long pondFlag;
            double pondElevation;
            double pondVolume;

            long numPoints = (long) m_points->Length;
            pin_ptr<BGEO::DPoint3d const> tPoint = &m_points[0];
            
            BcDTMPtr dtm = BcDTM::DesignPondToTargetVolumeOrElevation(&pondFlag, &pondElevation, &pondVolume,
                (::DPoint3d*)&tPoint[0], numPoints, (long) m_designMethod, (long) m_pondTarget,
                m_targetVolume, m_targetElevation, m_sideSlope, m_freeBoard);

            m_lastResult = (DTMPondResult) pondFlag;
            m_pondVolume = pondVolume;
            m_pondElevation = pondElevation;

            if (dtm!=NULL)
                {
                pondDTM = gcnew DTM (dtm.get());
                return (DTMPondResult) pondFlag;
                }
                             
            return DTMPondResult::UnknownError;                            

            }

    };


END_BENTLEY_TERRAINMODELNET_NAMESPACE
