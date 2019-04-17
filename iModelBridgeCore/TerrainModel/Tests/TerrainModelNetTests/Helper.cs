using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.IO;
using BGEO = Bentley.GeometryNET;
using NUnit.Framework;

namespace Bentley.TerrainModelNET.NUnit
{
    class Helper
    {
        public static string GetTestDataLocation ()
            {
            string sourceDirectory;
            var domain = AppDomain.CurrentDomain;

            // If we are being shadowed look at the currentdomain.basedirectory
            if (domain.ShadowCopyFiles)
                sourceDirectory = domain.BaseDirectory;
            else
                sourceDirectory = System.IO.Path.GetDirectoryName (typeof (Helper).Assembly.Location);

            return System.IO.Path.Combine (sourceDirectory, "Data", "TerrainModelNet");
            }

        public static BGEO.DPoint3d[] Copy(System.Collections.Generic.IEnumerable<BGEO.DPoint3d> sourcePt)
        {
            System.Collections.Generic.IEnumerator<BGEO.DPoint3d> enumerator = sourcePt.GetEnumerator();

            int nPoint = 0;
            enumerator.Reset();
            while (enumerator.MoveNext()) nPoint++;

            BGEO.DPoint3d[] ret = new BGEO.DPoint3d[nPoint];

            int iPoint = 0;

            foreach (BGEO.DPoint3d pt in sourcePt)
            {
                ret[iPoint++] = pt;
            }
            return ret;
        }

        /// <summary>
        /// Class To Store DTM Validation Results
        /// </summary>
        public class ValidationResult
        {
            internal bool triangulationValid = false;
            internal long numPoints = 0;
            internal long numLines = 0;
            internal long numTriangles = 0;
            internal long numFeatures = 0;
            internal double area = 0.0;
            internal double volume = 0.0;

            /// <summary>
            /// ValidationResult Constructor
            /// </summary>

            internal ValidationResult(bool valid, long numPoints, long numLines, long numTriangles, long numFeatures, double area, double volume)
            {
                this.triangulationValid = valid;
                this.numPoints = numPoints;
                this.numLines = numLines;
                this.numTriangles = numTriangles;
                this.numFeatures = numFeatures;
                this.area = area;
                this.volume = volume;
            }

            /// <summary>
            /// Validation Result Properties
            /// </summary>

            public bool TriangulationValid
            {
                get { return triangulationValid; }
            }

            public long NumPoints
            {
                get { return numPoints; }
            }

            public long NumLines
            {
                get { return numLines; }
            }

            public long NumTriangles
            {
                get { return numTriangles; }
            }

            public long NumFeatures
            {
                get { return numFeatures; }
            }

            public double Area
            {
                get { return area; }
            }

            public double Volume
            {
                get { return volume; }
            }
        }

        /// <summary>
        /// Validate An In Memory DTM
        /// </summary>
        public static ValidationResult ValidateDTM(DTM dtm)
        {
            ValidationResult validationResult = new ValidationResult(false, 0, 0, 0, 0, 0.0, 0.0);
            if (dtm.CheckTriangulation() == true)
            {
                validationResult.triangulationValid = true;
                validationResult.numPoints = dtm.VerticesCount;
                validationResult.numLines = dtm.CalculateFeatureStatistics ().TrianglesLinesCount;
                validationResult.numTriangles = dtm.TrianglesCount;
                validationResult.numFeatures = dtm.CalculateFeatureStatistics ().FeaturesCount;
                VolumeCriteria volumeCriteria = new VolumeCriteria();
                VolumeResult volumeResult = dtm.CalculatePrismoidalVolumeToElevation(0.0, volumeCriteria);
                validationResult.area = volumeResult.Area;
                validationResult.volume = volumeResult.BalanceVolume;
            }
            return (validationResult);
        }

        /// <summary>
        /// Import And Validate A DTM
        /// </summary>
        public static ValidationResult ImportAndValidateDTM(string folderName, string dtmName)
        {
            ValidationResult validationResult = new ValidationResult(false, 0, 0, 0, 0, 0.0, 0.0);
            DTM dtm = DTM.CreateFromFile(folderName + "\\" + dtmName);
            Assert.IsNotNull(dtm, "Failed To Import DTM");
            if (dtm.CheckTriangulation() == true)
            {
                validationResult.triangulationValid = true;
                validationResult.numPoints = dtm.VerticesCount;
                validationResult.numLines = dtm.CalculateFeatureStatistics ().TrianglesLinesCount;
                validationResult.numTriangles = dtm.TrianglesCount;
                validationResult.numFeatures = dtm.CalculateFeatureStatistics ().FeaturesCount;
                VolumeCriteria volumeCriteria = new VolumeCriteria();
                VolumeResult volumeResult = dtm.CalculatePrismoidalVolumeToElevation(0.0, volumeCriteria);
                validationResult.area = volumeResult.Area;
                validationResult.volume = volumeResult.BalanceVolume;
            }
            dtm.Dispose();
            return (validationResult);
        }

        /// <summary>
        /// Create A Set Of Random Points
        /// </summary>
        public static BGEO.DPoint3d[] CreateRandomPoints(int seed,long numRandomPoints,double xMin,double xMax,double yMin,double yMax,double zMin,double zMax )
        {
            Random randomNumber = new Random(seed) ;

            double deltaX = xMax - xMin;
            double deltaY = yMax - yMin;
            double deltaZ = zMax - zMin;

            BGEO.DPoint3d[] randomPoints = new BGEO.DPoint3d[numRandomPoints];
            for (int n = 0; n < numRandomPoints; ++n)
            {
                randomPoints[n].X = xMin + deltaX * randomNumber.NextDouble();
                randomPoints[n].Y = yMin + deltaY * randomNumber.NextDouble();
                randomPoints[n].Z = zMin + deltaZ * randomNumber.NextDouble();
            }
            return( randomPoints ) ;
        }

        /// <summary>
        /// Stroke A Linear Feature
        /// </summary>
        public static BGEO.DPoint3d[] StrokeLinearFeature(double strokeInterval,BGEO.DPoint3d[] linearFeature )
        {
            // If Input linear Feature Has A Zero Size Return NULL

            if (linearFeature.Length == 0 ) return (null);

            // Calculate Length Of Linear Feature

            double length = 0.0;
            for (int n = 1 ; n < linearFeature.Length ; ++n)
            {
                double dx = linearFeature[n].X - linearFeature[n - 1].X;
                double dy = linearFeature[n].Y - linearFeature[n - 1].Y;
                length = length + Math.Sqrt(dx * dx + dy * dy);
            }

            // Return Input Linear Feature

            if (strokeInterval <= 0.0 || length == 0.0 || strokeInterval >= length)
            {
                BGEO.DPoint3d[] strokedLinearFeature = new BGEO.DPoint3d[linearFeature.Length];
                for (int n = 1; n < linearFeature.Length; ++n)
                {
                    strokedLinearFeature[n] = linearFeature[n];
                }
                return (strokedLinearFeature);
            }

            //  Stroke Linear Feature

            else
            {
                ArrayList strokedPoints = new ArrayList();
                for (int n = 1; n < linearFeature.Length ; ++n)
                {
                    // Add Start Segment Point To Array list

                    BGEO.DPoint3d point = new BGEO.DPoint3d();
                    point = linearFeature[n-1] ;
                    strokedPoints.Add(point);

                    // Stroke Between Start Segment Point And End Segment Point
 
                    double dx = linearFeature[n].X - linearFeature[n - 1].X;
                    double dy = linearFeature[n].Y - linearFeature[n - 1].Y;
                    double dz = linearFeature[n].Z - linearFeature[n - 1].Z;
                    length = Math.Sqrt(dx * dx + dy * dy);
                    if (length > strokeInterval)
                    {
                        double strokeLength = 0.0;
                        int numIntersections = (int)(length / strokeInterval);
                        for (int i = 0; i < numIntersections; ++i)
                        {
                            BGEO.DPoint3d strokePoint = new BGEO.DPoint3d();
                            strokeLength = strokeLength + strokeInterval;
                            strokePoint.X = linearFeature[n - 1].X + dx * strokeLength / length;
                            strokePoint.Y = linearFeature[n - 1].Y + dy * strokeLength / length;
                            strokePoint.Z = linearFeature[n - 1].Z + dz * strokeLength / length;
                            strokedPoints.Add(strokePoint);
                        }
                    }

                    // Add End Segment Point To Array list

                    point = new BGEO.DPoint3d();
                    point = linearFeature[n];
                    strokedPoints.Add(point);
                }

                // Copy Array List Points To BGEO Array

                BGEO.DPoint3d[] strokedLinearFeature = new BGEO.DPoint3d[strokedPoints.Count];

                int m = 0;
                foreach(BGEO.DPoint3d point in strokedPoints)
                {
                    strokedLinearFeature[m] = point;
                    ++m;
                }

                // Return Stroked points

                return (strokedLinearFeature);
            }
        }
    }
}

    



