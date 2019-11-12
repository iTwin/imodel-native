/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include < vcclr.h >
#include "./dtm.h"
#include "./dtmexception.h"
#include "./Bentley.Civil.DTM.h"
#include "TerrainModel/Drainage/drainage.h"
#if defined(Public)

#undef Public
#endif

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

///<summary>Pond Design</summary>
///<author>Vince.Unrau</author>
///<date>9/2017</date>
  
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
            TopDown = 2        
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
        bool m_isBerm;
        double m_bermSlope;
        double m_bermWidth;
        bool m_isCrown;
        double m_crownWidth;
        bool m_isBermFillOnly;
        double m_pondElevation;
        double m_pondVolume;
        double m_cornerStrokeTolerance;
        double m_bermToTargetDtmSlope;
        DTMPondResult m_lastResult;
        DTM^ m_targetDTM;

    public:

        DTMPondDesignCriteria()
            {
            // Set default params
            m_designMethod = DTMPondDesignMethodEnum::BottomUp;
            m_sideSlope = 1.0;
            m_freeBoard = 0.0;
            m_pondTarget = DTMPondTargetEnum::Volume;
            m_targetVolume = 100000.0;
            m_targetElevation = 0.0;
            m_isBerm = false;
            m_bermSlope = 1.0;
            m_bermWidth = 5.0;
            m_isCrown = false;
            m_crownWidth = 5.0;
            m_pondElevation = 0.0;
            m_pondVolume = 0.0;
            m_isBermFillOnly = true;
            m_cornerStrokeTolerance = 10.0;
            m_bermToTargetDtmSlope = 1.0;
            m_lastResult = DTMPondResult::TargetObtained;
            };   

        DTMPondDesignCriteria(DTMPondDesignMethodEnum perimeterOrInvert, array<BGEO::DPoint3d>^ perimerOrInvertPoints, double sideSlope, double freeBoard, DTMPondTargetEnum pondTarget, double target,
            bool IsBerm, double bermSlope, double bermWidth, double bermToTargetDtmSlope, bool isBermFillOnly, bool IsCrown, double crownWidth, double cornerStrokeTolerance, DTM^ targetDTM)
            {
            // Set design params
            m_designMethod = perimeterOrInvert;
            SetReferencePoints(perimerOrInvertPoints);
            m_sideSlope = sideSlope;
            m_freeBoard = freeBoard;
            m_pondTarget = pondTarget;
            if (pondTarget == DTMPondTargetEnum::Volume)
                m_targetVolume = target;
            else
                m_targetElevation = target;

            m_isBerm = IsBerm;
            m_bermSlope = bermSlope;
            m_bermWidth = bermWidth;
            m_isCrown = IsCrown;
            m_crownWidth = CrownWidth;
            m_isBermFillOnly = isBermFillOnly;
            m_cornerStrokeTolerance = cornerStrokeTolerance;
            m_bermToTargetDtmSlope = bermToTargetDtmSlope;
            m_pondElevation = 0.0;
            m_pondVolume = 0.0;
            m_lastResult = DTMPondResult::TargetObtained;
            m_targetDTM = targetDTM;
            };

        DTMPondDesignCriteria(DTMPondDesignMethodEnum perimeterOrInvert, array<BGEO::DPoint3d>^ perimerOrInvertPoints, double sideSlope, double freeBoard, DTMPondTargetEnum pondTarget, double target)
            {
            m_designMethod = perimeterOrInvert;
            SetReferencePoints(perimerOrInvertPoints);
            m_sideSlope = sideSlope;
            m_freeBoard = freeBoard;
            m_pondTarget = pondTarget;
            if (pondTarget == DTMPondTargetEnum::Volume)
                m_targetVolume = target;
            else
                m_targetElevation = 0.0;

            m_isBerm = false;
            m_bermSlope = 1.0;
            m_bermWidth = 5.0;
            m_isCrown = false;
            m_crownWidth = 5.0;
            m_pondElevation = 0.0;
            m_pondVolume = 0.0;
            m_isBermFillOnly = true;
            m_cornerStrokeTolerance = 10.0;
            m_bermToTargetDtmSlope = 1.0;
            m_lastResult = DTMPondResult::TargetObtained;
            }

        property array<BGEO::DPoint3d>^ PerimPoints
            {
            array<BGEO::DPoint3d>^ get()
                {
                return m_points;
                }
            }

        property DTM^ TargetDTM
            {
            Bentley::TerrainModelNET::DTM^ get()
                {
                return m_targetDTM;
                }
            void set(DTM^ value)
                {
                m_targetDTM = value;
                }
            }

        property double BermToTargetDtmSlope
            {
            double get()
                {
                return m_bermToTargetDtmSlope;
                }
            void set(double value)
                {
                m_bermToTargetDtmSlope = value;
                }
            }

        property double CornerStrokeTolerance
            {
            double get()
                {
                return m_cornerStrokeTolerance;
                }
            void set(double value)
                {
                m_cornerStrokeTolerance = value;
                }
            }

        property bool IsBermFillOnly
            {
            bool get()
                {
                return (bool) m_isBermFillOnly;
                }
            void set(bool value)
                {
                m_isBermFillOnly = value;
                }
            }

        property bool IsCrown
            {
            bool get()
                {
                return m_isCrown;
                }
            void set(bool value)
                {
                m_isCrown = value;
                }
            }

        property bool IsBerm
            {
            bool get()
                {
                return m_isBerm;
                }
            void set(bool value)
                {
                m_isBerm = value;
                }
            }

        property double CrownWidth
            {
            double get()
                {
                return m_crownWidth;
                }
            void set(double value)
                {
                m_crownWidth = value;
                }
            }

        property double BermWidth
            {
            double get()
                {
                return m_bermWidth;
                }
            void set(double value)
                {
                m_bermWidth = value;
                }
            }

        property double BermSlope
            {
            double get()
                {
                return m_bermSlope;
                }
            void set(double value)
                {
                m_bermSlope = value;
                }
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
            void set(double value)
                {
                m_pondElevation = value;
                }
            }

        property double AchievedPondVolume
            {
            double get()
                {
                return m_pondVolume;
                }
            void set(double value)
                {
                m_pondVolume = value;
                }
            }

        property DTMPondResult LastResult
            {
            DTMPondResult get()
                {
                return m_lastResult;
                }
            void set(DTMPondResult value)
                {
                m_lastResult = value;
                }
            }

        DTMPondResult DTMPondDesignCriteria::CreatePond([Out] DTM^% pondDTM);

    };


END_BENTLEY_TERRAINMODELNET_NAMESPACE
