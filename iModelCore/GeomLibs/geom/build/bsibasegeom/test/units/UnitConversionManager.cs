/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// UnitConversionManager.cs
// Copyright 2001-2004 Haestad Methods, Inc. All Rights Reserved.

using System;
using System.Collections;
using System.Diagnostics;

namespace Haestad.Support.Units
{
    /// <summary>
    /// Singleton instance manages all singletons in unit subsystem:
    /// units, dimensions, unit systems.
    /// </summary>
    public class UnitConversionManager
    {
                #region Static Public Methods

        static public UnitConversionManager Current
        {
            get
            {
                if (CurrentManager == null)
                    CurrentManager = new UnitConversionManager();
                return CurrentManager;
            }
        }

                static public double EmitterExponent
        {
                        get
                        {
                                return m_EmitterExponent;
                        }
                        set
                        {
                                m_EmitterExponent = value;
                        }
                }

                #endregion

                #region Public Enumerations

                /// <summary>
                /// DimensionIndex and UnitIndex were originally (framwork v2) NOT intended to be persisted.
                /// They were meant to be a dynamic, in-memory index. However Domain v3 persisted them so now
                /// both styles of enums -- GEMS or .NET -- can be persisted, and are guaranteed not to change.
                /// (Lesson learned: these should have been declared internal originally.)
                /// </summary>
                public enum DimensionIndex
        {
            None,   //Unitless: Favor singletons for sentinel values instead of null, 34 dimensions.
            Angle,
            Area,
                        Concentration,
            Currency,
                        CurrencyPerEnergy,
                        CurrencyPerLength,
                        Diffusivity,
            ElectricalFrequency,
                        EmitterCoefficient,
                        Energy,
            Flow,
                        FlowDensityPerArea,
                        FlowDensityPerCapita,
                        InfiltrationRate,
            Length,
            Mass,
                        MassRate,
                        NthOrderBulkReactionRate,
            Percent,
                        Population,
                        PopulationDensityPerArea,
            Power,
            Pressure,
                        RainfallIntensity,
                        ReactionRate,
            RotationalFrequency,
            Scale,
            Slope,
                        SpecificWeight,
                        SurfaceReactionRate,
            Temperature,
            Time,
                        Unitless,
            Velocity,
            Volume,
                        WeirCoefficient,
                        ZeroOrderSurfaceReactionRate,
                        DiameterLength,
                        MassPerArea,
                        Inertia,
                        CurrencyPerPower,
                        CostPerUnitVolume,
                        EnergyPerUnitVolume,
                        Torque,
                        SpringConstant,
                        Force,
                        Density,
                        DischargePerPressureDrop,

        }

        public enum UnitIndex
        {
            None,
            AcreFeet,
            AcreFeetPerDay,
            AcreFeetPerHour,
            AcreFeetPerMinute,
            AcreInches,
            AcreInchPerHour,
            AcreInchPerMinute,
            Acres,
            AngleDegrees,
            AngleMinutes,
            AngleQuadrants,
            AngleRadians,
            AngleRevolutions,
            AngleSeconds,
            Atmospheres,
            Bars,
                        // TODO 9 : Provided by .Net Framework?
                        // BaseCurrency,
                        Capita,
            Celsius,
            CentimeterPerMeter,
            Centimeters,
                        CentimetersPerDay,
                        CentimetersPerHour,
                        CentimetersPerMinute,
                        Centistokes,
            CFM,
                        CFMPerPSI,
            CFS,
                        CFSPerPSI,
                        CFSPerAcres,
                        CFSPerSquareFeet,
                        CFSPerSquareMiles,
            CubicCentimeters,
            CubicFeet,
            CubicFeetPerDay,
                        CubicFeetPerDayPerPSI,
            CubicFeetPerMinute,
                        CubicFeetPerMinutePerPSI,
            CubicFeetPerSecond,
                        CubicFeetPerSecondPerPSI,
            CubicInches,
            CubicMeters,
            CubicMetersPerDay,
                        CubicMetersPerDayPerMetersOfH2O,
            CubicMetersPerHour,
                        CubicMetersPerHourPerMetersOfH2O,
            CubicMetersPerMinute,
                        CubicMetersPerMinutePerMetersOfH2O,
            CubicMetersPerSecond,
                        CubicMetersPerSecondPerMetersOfH2O,
                        CubicMetersPerHectaresPerDay,
                        CubicMetersPerSquareKilometerPerDay,
                        CubicMetersPerSquareMeterPerDay,
            CubicYards,
                        Customer,
            Days,
            Decimeters,
            Dollars,
                        DollarsPerFoot,
                        DollarsPerMeter,
                        DollarsPerKiloWattHour,
                        Employee,
            Fahrenheit,
            Feet,
            FeetOfH2O,
                        FeetPerDay,
                        FeetPerInch,
            FootHorizontalPerFootVertical,
            FootPer1000Feet,
            FootPerFoot,
            FootPerMile,
                        FootPoundals,
            FootVerticalPerFootHorizontal,
            Gallons,
            GallonsPerDay,
                        GallonsPerDayPerPSI,
            GallonsPerMinute,
                        GallonsPerMinutePerPSI,
            GallonsPerSecond,
                        GallonsPerSecondPerPSI,
                        // TODO 9 : Provided by .Net Framework?
                        //GigaCurrency,
                        GPDPerAcres,
                        GPDPerCapita,
                        GPDPerSquareFeet,
                        GPDPerSquareMile,
                        GPM,
                        GPMPerAcres,
                        GPMPerPSI,
                        GPMPerSquareFeet,
                        GPMPerSquareMile,
            Gram,
                        GramsPerDay,
                        GramsPerHour,
                        GramsPerMinute,
                        GramsPerSecond,
                        Guest,
            Hectares,
            Hertz,
            HorizontalPerVertical,
            Horsepower,
            Hours,
                        HundredCapita,
            ImperialGallonsPerDay,
            ImperialGallonsPerMinute,
            ImperialGallonsPerSecond,
            ImpGallons,
                        ImperialGallonsPerDayPerPSI,
                        ImperialGallonsPerMinutePerPSI,
                        ImperialGallonsPerSecondPerPSI,
            Inches,
                        InchesPerDay,
                        InchesPerHour,
                        InchesPerMinute,
            InchPerFoot,
                        InfiltrationRateCentimetersPerDay,
                        InfiltrationRateCentimetersPerHour,
                        InfiltrationRateCentimetersPerMinute,
                        InfiltrationRateInchesPerDay,
                        InfiltrationRateInchesPerHour,
                        InfiltrationRateInchesPerMinute,
                        InfiltrationRateMillimetersPerDay,
                        InfiltrationRateMillimetersPerHour,
                        InfiltrationRateMillimetersPerMinute,
                        Joules,
                        // TODO 9 : Provided by .Net Framework?
                        //KiloCurrency,
            Kilograms,
                        KilogramsPerDay,
                        KilogramsPerHour,
                        KilogramsPerMinute,
                        KilogramsPerSecond,
            KilogramsPerSquareCentimeter,
                        KiloJoules,
            Kilometers,
                        KiloNewtonsPerCubicMeter,
            KiloPascals,
                        KiloWattHours,
            Kilowatts,
            Liters,
                        LitersPerCapitaPerDay,
            LitersPerDay,
                        LitersPerDayPerMetersOfH2O,
            LitersPerMinute,
                        LitersPerMinutePerMetersOfH2O,
            LitersPerSecond,
                        LitersPerSecondPerMetersOfH2O,
                        LitersPerHectaresPerDay,
                        LitersPerSquareKilometerPerDay,
                        LitersPerSquareMeterPerDay,
                        // TODO 9 : Provided by .Net Framework?
                        //MegaCurrency,
            MegaLitersPerDay,
                        MegaLitersPerDayPerMetersOfH2O ,
            MeterHorizontalPerMeterVertical,
            MeterPerKilometer,
            MeterPerMeter,
            Meters,
            MetersOfH2O,
            MetersPerCm,
                        MetersPerDay,
                        MetersPerSecond,
            MeterVerticalPerMeterHorizontal,
            Mfeet,
            MGD,
                        MGDPerPSI,
            MGDImperial,
                        MGDImperialPerPSI,
                        MicrogramsPerDay,
                        MicrogramsPerHour,
                        MicrogramsPerLiter,
                        MicrogramsPerLiterNPerDay,
                        MicrogramsPerLiterNPerSecond,
                        MicrogramsPerMinute,
                        MicrogramsPerSecond,
                        MicrogramsPerSquareFeetPerDay,
                        MicrogramsPerSquareMeterPerDay,
                        MicrogramsPerSquareMeterPerSecond,
            Miles,
            Millifeet,
            Milligram,
                        MilliGramsPerDay,
                        MilliGramsPerHour,
                        MilligramsPerLiter,
                        MilligramsPerLiterNPerDay,
                        MilligramsPerLiterNPerSecond,
                        MilliGramsPerMinute,
                        MilliGramsPerSecond,
                        MilliGramsPerSquareFeetPerDay,
                        MilliGramsPerSquareMeterPerDay,
                        MilliGramsPerSquareMeterPerSecond,
            MillimeterHorizontalPerMeterVertical,
            MillimeterPerMeter,
            Millimeters,
            MillimetersOfH2O,
                        MilliMetersPerDay,
                        MilliMetersPerHour,
                        MillimetersPerMinute,
            MillimeterVerticalPerMeterHorizontal,
            MillionGallons,
            MillionLiters,
            Minutes,
                        NewtonsPerCubicMeter,
            NewtonsPerSquareMeter,
            OneOverSlope,
                        PartsPerBillion,
                        PartsPerBillionNPerDay,
                        PartsPerBillionNPerSecond,
                        PartsPerMillion,
                        PartsPerMillionNPerDay,
                        PartsPerMillionNPerSecond,
                        Passenger,
                        PerDay,
            PercentPercent,
            PercentSlope,
                        //PerMinute is at bottom
                        PerSecond,
                        Person,
                        PersonsPerAcre,
                        PersonsPerSquareFeet,
                        PersonsPerSquareKilometer,
                        PersonsPerHectares,
                        PersonsPerSquareMeter,
                        PersonsPerSquareMile,
            Pounds,
                        PoundsForcePerCubicFoot,
                        PoundsPerCubicFoot,
                        PoundsPerCubicFootNPerDay,
                        PoundsPerCubicFootNPerSecond,
                        PoundsPerDay,
                        PoundsPerHour,
                        PoundsPerMillionGallons,
                        PoundsPerMillionGallonsNPerDay,
                        PoundsPerMillionGallonsNPerSecond,
                        PoundsPerMinute,
                        PoundsPerSecond,
            PoundsPerSquareFoot,
            PoundsPerSquareInch,
            PSI,
                        Resident,
            RPM,
            Seconds,
            SquareCentimeters,
            SquareFeet,
                        SquareFeetPerSecond,
            SquareInches,
            SquareKilometers,
            SquareMeters,
                        SquareMetersPerSecond,
            SquareMiles,
            SquareMillimeters,
            SquareYards,
                        Stokes,
                        Student,
                        ThousandCapita,
            ThousandGallons,
            ThousandLiters,
            ThousandSquareFeet,
                        UnitlessPercent,
                        UnitlessUnit,
            VelocityCentimetersPerHour,
            VelocityCentimetersPerMinute,
            VelocityCentimetersPerSecond,
            VelocityFeetPerHour,
            VelocityFeetPerMinute,
            VelocityFeetPerSecond,
            VelocityInchesPerHour,
            VelocityInchesPerMinute,
            VelocityInchesPerSecond,
            VelocityKilometersPerHour,
            VelocityKnot,
            VelocityKnotInternational,
            VelocityMetersPerHour,
            VelocityMetersPerMinute,
            VelocityMetersPerSecond,
            VelocityMilePerHour,
            VerticalPerHorizontal,
            Watts,
                        WeirCoefficientSi,
                        WeirCoefficientUs,
                        Yards,
            Years,
                        // As of Domain v3 and CSD, UnitIndex's cannot be changed. New units must be added sequentially, at the end.
                        MillionLitersPerDay,
                        PerMinute,
                        InchMiles,
                        InchFeet,
                        FootMiles,
                        FootFeet,
                        MillimeterMeters,
                        MillimeterKilometers,
                        MeterMeters,
                        MeterKilometers,
                        InchMeters,
                        MillimeterMiles,
                        PoundsPerAcre,
                        KilogramsPerHectare,
                        DollarsPerKiloWatt,
                        PoundSquareFeet,
                        NewtonSquareMeters,
                        DollarsPerHorsepower,
                        DollarsPerCubicCentimeters,
                        DollarsPerLiters,
                        DollarsPerCubicMeters,
                        DollarsPerCubicInches,
                        DollarsPerGallons,
                        DollarsPerImpGallons,
                        DollarsPerCubicFeet,
                        DollarsPerCubicYards,
                        DollarsPerAcreInches,
                        DollarsPerAcreFeet,
                        DollarsPerMillionGallons,
                        DollarsPerThousandGallons,
                        DollarsPerThousandLiters,
                        DollarsPerMillionLiters,
                        KilogramsPerSquareMeter,
                        KiloWattHourPerMillionGallons,
                        KiloWattHourPerMillionLiters,
                        KiloWattHourPerCubicMeters,
                        KiloWattHourPerCubicFeet,
                        PerHour,
                        NewtonMeters,
                        PoundFeet,
                        PoundPerInch,
                        NewtonPerMillimeter,
                        PoundForce,
                        KiloPoundForce,
                        Newton,
                        KiloNewton,
                        SlugPerCubicFoot,
                        PoundPerCubicFoot,
                        KilogramPerCubicMeter,
                        CfsPerSquareRootFooH20,
                        CmsPerSquareRootMeterH20,
                        LPerSecPerSquareRootKpa,
                        GpmPerSquareRootPsi,
                        Milliseconds,
        }

        public enum UnitSystemIndex
        {
            None,           // Replaces VSW "unitless" unit system.
            Si,
            UsCustomary,
                        Both
        }

                #endregion

                #region Constructors

        private UnitConversionManager()
        {
            //Sequence here is critical.
            InitializeDimensions();
            InitializeUnitSystems();
            InitializeUnits();

                        Debug.Assert(Enum.GetValues(typeof(DimensionIndex)).Length == m_rgdimension.Length);
                        Debug.Assert(Enum.GetValues(typeof(UnitIndex)).Length == m_rgunit.Length);
                        Debug.Assert(Enum.GetValues(typeof(UnitSystemIndex)).Length == m_rgunitsystem.Length);
        }

                #endregion

                #region Public Methods

                /// <summary>
                /// Return a list of all known dimensions. Can be useful for enumerating
                /// all known units. For testing and low-performance parsing. Be cautious
                /// of algorithms that require this method in high performance situations.
                /// </summary>
                public IList AvailableDimensions()
                {
                        ArrayList aal = new ArrayList();
                        foreach (DimensionIndex adi in Enum.GetValues(typeof(DimensionIndex)))
                                aal.Add(DimensionAt(adi));
                        return aal;
                }

                public Dimension DimensionAt(DimensionIndex adimensionindex)
        {
            return m_rgdimension[(int)adimensionindex];
        }

                public UnitIndex UnitIndexFor(Unit unit)
                {
                        for (int i=0; i<m_rgunit.Length; ++i)
                        {
                                if (m_rgunit[i] == unit)
                                        return (UnitIndex)i;
                        }
                        return UnitIndex.None;
                }

                public Unit UnitAt(UnitIndex aunitindex)
        {
            return m_rgunit[(int)aunitindex];
        }

                public UnitSystem UnitSystemAt(UnitSystemIndex aunitsystemindex)
        {
            return m_rgunitsystem[(int)aunitsystemindex];
        }

                #endregion

                #region Protected Methods

                protected void InitializeDimensions()
        {
            //Enums here taken from Dimension.FromEnum() 11/6/2003.
            m_rgdimension = new Dimension[] {
                new Dimension("none",                                                   0),
                new Dimension("angle",                                                  16),
                new Dimension("area",                                                   2),
                                new Dimension("concentration",                                  10),
                new Dimension("currency",                                               26),
                                new Dimension("currencyperenergy",                              23),
                                new Dimension("currencyperlength",                              34),
                new Dimension("diffusivity",                                    12),
                                new Dimension("electricalFrequency",                    102),
                                new Dimension("emittercoefficient",                             33),
                                new Dimension("energy",                                                 28),
                new Dimension("flow",                                                   4),
                                new Dimension("flowdensityperarea",                             25),
                                new Dimension("flowpercapita",                                  31),            //TODO 7: should be flowdensitypercapita?
                                new Dimension("infiltrationrate",                               17),
                new Dimension("length",                                                 1),
                new Dimension("mass",                                                   3),
                                new Dimension("massrate",                                               29),
                                new Dimension("nthorderbulkreactionrate",               20),
                new Dimension("percent",                                                5),
                                new Dimension("population",                                             27),
                                new Dimension("populationdensityperarea",               32),
                                new Dimension("power",                                                  14),
                new Dimension("pressure",                                               9),
                                new Dimension("rainfallintensity",                              15),
                                new Dimension("reactionrate",                                   7),
                new Dimension("rotationalfrequency",                    103),
                new Dimension("scaleDimension",                                 19),
                new Dimension("slope",                                                  24),
                                new Dimension("specificweight",                                 30),
                                new Dimension("surfacereactionrate",                    8),
                new Dimension("temperature",                                    13),
                new Dimension("time",                                                   6),
                                new Dimension("unitless",                                               35),
                new Dimension("velocity",                                               18),
                new Dimension("volume",                                                 11),
                                new Dimension("weircoefficient",                                104),
                                new Dimension("zeroordersurfacereactionrate",   21),
                                new Dimension("diameterLength",                                 105),
                                new Dimension("massPerArea",                                    106),
                                new Dimension("currencyPerPower",                               22),
                                new Dimension("inertia",                                                108),
                                new Dimension("costperunitvolume",                              109),
                                new Dimension("energyperunitvolume",                    110),
                                new Dimension("torque",                                                 111),
                                new Dimension("springconstant",                                 112),
                                new Dimension("force",                                                  113),
                                new Dimension("density",                                                114),
                                new Dimension("dischargeperpressuredrop",               115),
                        };
        }

        protected void InitializeUnits()
        {
            //TODO 0: assign unit systems, proof/test all units!!!
            //Updated by Whar to be as per GEMS hmiUnitLib.idl 8/7/2002. 13 new dimensions, 115 new units, 1 new unit system.
                        m_rgunit = new Unit[] {
                                new     Unit("none",                                                                    DimensionAt(DimensionIndex.None),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            0),
                                new     Unit("acreFeet",                                                                DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             8.1071319e-4,                                           10),
                                new     Unit("acreFeetPerDay",                                                  DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             70.04561962,                                            25),
                                new     Unit("acreFeetPerHour",                                                 DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             2.918567484,                                            24),
                                new     Unit("acreFeetPerMinute",                                               DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             4.86427914e-2,                                          13),
                                new     Unit("acreInches",                                                              DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             9.728558e-3,                                            9),
                                new     Unit("acreInchPerHour",                                                 DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             35.022809808,                                           23),
                                new     Unit("acreInchPerMinute",                                               DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             0.5837134968,                                           22),
                                new     Unit("acres",                                                                   DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             2.47105381e-4,                                          10),
                                new     Unit("angleDegrees",                                                    DimensionAt(DimensionIndex.Angle),                                                              UnitSystemAt(UnitSystemIndex.None),                             57.295779,                                                      2),
                                new     Unit("angleMinutes",                                                    DimensionAt(DimensionIndex.Angle),                                                              UnitSystemAt(UnitSystemIndex.None),                             3437.7468,                                                      3),
                                new     Unit("angleQuadrants",                                                  DimensionAt(DimensionIndex.Angle),                                                              UnitSystemAt(UnitSystemIndex.None),                             0.63661977,                                                     5),
                                new     Unit("angleRadians",                                                    DimensionAt(DimensionIndex.Angle),                                                              UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("angleRevolutions",                                                DimensionAt(DimensionIndex.Angle),                                                              UnitSystemAt(UnitSystemIndex.None),                             0.15915494,                                                     6),
                                new     Unit("angleSeconds",                                                    DimensionAt(DimensionIndex.Angle),                                                              UnitSystemAt(UnitSystemIndex.None),                             206264.81,                                                      4),
                                new     Unit("atmospheres",                                                             DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             9.86923267e-3,                                  11),
                                new     Unit("bars",                                                                    DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.01,                                                           6),
                                // TODO 9 : Provided by .Net Framework?
                                //new   Unit("baseCurrency",                                                    DimensionAt(DimensionIndex.Currency),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("capita",                                                                  DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("celsius",                                                                 DimensionAt(DimensionIndex.Temperature),                                                UnitSystemAt(UnitSystemIndex.Si),                               new     BaseUnitConverter(),                    1),
                                new     Unit("centimeterPerMeter",                                              DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             100.0,                                                          3),
                                new     Unit("centimeters",                                                             DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.None),                             100.0,                                                          3),
                                new     Unit("centimetersPerDay",                                               DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             144,                                                            7),
                                new     Unit("centimetersPerHour",                                              DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             6,                                                                      9),
                                new     Unit("centimetersPerMinute",                                    DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             0.1,                                                            2),
                                new     Unit("centistokes",                                                             DimensionAt(DimensionIndex.Diffusivity),                                                UnitSystemAt(UnitSystemIndex.None),                             1.0e6,                                                          4),
                                new     Unit("cfm",                                                                             DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             2118.88002,                                                     18),
                                new Unit("cfmPerPSI",                                                           DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(2118.88002),          17),
                                new     Unit("cfs",                                                                             DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.UsCustomary),              35.314667,                                                      16),
                                new Unit("cfsPerPSI",                                                           DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(35.314667),                                           15),
                                new     Unit("cfsPerAcres",                                                             DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1.6540967e-3,                                           2),
                                new     Unit("cfsPerSquareFeet",                                                DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             3.797265e-8,                                            1),
                                new     Unit("cfsPerSquareMiles",                                               DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1.05861767,                                                     3),
                                new     Unit("cubicCentimeters",                                                DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             1.0e6,                                                          1),
                                new     Unit("cubicFeet",                                                               DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             35.314667,                                                      7),
                                new     Unit("cubicFeetPerDay",                                                 DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             3051187.229,                                            12),
                                new Unit("cubicFeetPerDayPerPSI",                                       DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(3051187.229),                                                 12),
                                new     Unit("cubicFeetPerMinute",                                              DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             2118.88002,                                                     17),
                                new Unit("cubicFeetPerMinutePerPSI",                            DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(2118.88002),                                          16),
                                new     Unit("cubicFeetPerSecond",                                              DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             35.314667,                                                      11),
                                new Unit("cubicFeetPerSecondPerPSI",                            DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(35.314667),                                           11),
                                new     Unit("cubicInches",                                                             DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             61023.74,                                                       4),
                                new     Unit("cubicMeters",                                                             DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            3),
                                new     Unit("cubicMetersPerDay",                                               DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             86400.0,                                                        6),
                                new Unit("cubicMetersPerDayPerMetersOfH2O",                     DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(86400.0),                                                     6),
                                new     Unit("cubicMetersPerHour",                                              DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             3600.0,                                                         5),
                                new Unit("cubicMetersPerHourPerMetersOfH2O",            DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(3600.0),                                                              5),
                                new     Unit("cubicMetersPerMinute",                                    DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             60.0,                                                           15),
                                new Unit("cubicMetersPerMinutePerMetersOfH2O",          DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(60.0),                                                                14),
                                new     Unit("cubicMetersPerSecond",                                    DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            4),
                                new Unit("cubicMetersPerSecondPerMetersOfH2O",          DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(1.0),                                                         4),
                                new     Unit("cubicMetersPerHectaresPerDay",                    DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             10,                                                                     15),
                                new     Unit("cubicMetersPerSquareKilometerPerDay",             DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1.0e3,                                                          14),
                                new     Unit("cubicMetersPerSquareMeterPerDay",                 DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1.0e-3,                                                         13),
                                new     Unit("cubicYards",                                                              DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             1.3079506,                                                      8),
                                new     Unit("customer",                                                                DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            4),
                                new     Unit("days",                                                                    DimensionAt(DimensionIndex.Time),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0/24.0,                                                       4),
                                new     Unit("decimeters",                                                              DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.None),                             10.0,                                                           11),
                                new     CurrencyBasedUnit("dollars",                                    DimensionAt(DimensionIndex.Currency),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new CurrencyBasedUnit("dollarsPerFoot",                         DimensionAt(DimensionIndex.CurrencyPerLength),                                  UnitSystemAt(UnitSystemIndex.UsCustomary),              0.3048,                                                         0), // No equivalent GEMS enum value
                                new CurrencyBasedUnit("dollarsPerMeter",                        DimensionAt(DimensionIndex.CurrencyPerLength),                                  UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            1), // No equivalent GEMS enum value
                            new CurrencyBasedUnit("dollarsPerKiloWattHour",             DimensionAt(DimensionIndex.CurrencyPerEnergy),                                  UnitSystemAt(UnitSystemIndex.Both),                             1.0,                                                            1), // No equivalent GEMS enum value
                                new     Unit("employee",                                                                DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            5),
                                new     Unit("fahrenheit",                                                              DimensionAt(DimensionIndex.Temperature),                                                UnitSystemAt(UnitSystemIndex.UsCustomary),              new     DegreesFahrenheitConverter(),   2),
                                new     Unit("feet",                                                                    DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              3.280839895,                                            5),
                                new     Unit("feetOfH2O",                                                               DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.33455255,                                                     8),
                                new     Unit("feetPerDay",                                                              DimensionAt(DimensionIndex.SurfaceReactionRate),                                UnitSystemAt(UnitSystemIndex.UsCustomary),              283464.56736,                                           3),
                                new     Unit("feetPerInch",                                                             DimensionAt(DimensionIndex.Scale),                                                              UnitSystemAt(UnitSystemIndex.None),                             8.333333333,                                            2),
                                new     Unit("footHorizontalPerFootVertical",                   DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new SlopeConverter(),                   7),
                                new     Unit("footPer1000Feet",                                                 DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         4),
                                new     Unit("footPerFoot",                                                             DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new BaseUnitConverter(),                        5),
                                new     Unit("footPerMile",                                                             DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             5280.0,                                                         8),
                                new     Unit("footPoundals",                                                    DimensionAt(DimensionIndex.Energy),                                                             UnitSystemAt(UnitSystemIndex.None),                             23.73,                                                          4),
                                new     Unit("footVerticalPerFootHorizontal",                   DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new BaseUnitConverter(),                6),
                                new     Unit("gallons",                                                                 DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             264.17205,                                                      5),
                                new     Unit("gallonsPerDay",                                                   DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             22824465.12,                                            8),
                                new Unit("gallonsPerDayPerPSI",                                         DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(22824465.12),                                         8),
                                new     Unit("gallonsPerMinute",                                                DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             15850.323,                                                      19),
                                new Unit("gallonsPerMinutePerPSI",                                      DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(15850.323),                                           18),
                                new     Unit("gallonsPerSecond",                                                DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             264.17205,                                                      7),
                                new Unit("gallonsPerSecondPerPSI",                                      DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(264.17205),                                           7),
                                // TODO 9 : Provided by .Net Framework?
                                //new   Unit("gigaCurrency",                                                    DimensionAt(DimensionIndex.Currency),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.000000001,                                            4),
                                new     Unit("gpdPerAcres",                                                             DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.UsCustomary),              1069.070643,                                            8),
                                new     Unit("gpdPerCapita",                                                    DimensionAt(DimensionIndex.FlowDensityPerCapita),                               UnitSystemAt(UnitSystemIndex.UsCustomary),              0.264172,                                                       1),
                                new     Unit("gpdPerSquareFeet",                                                DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             2.45423867e-2,                                          7),
                                new     Unit("gpdPerSquareMiles",                                               DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             684202.474691,                                          9),
                                new     Unit("gpm",                                                                             DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.UsCustomary),              15850.323,                                                      20),
                                new     Unit("gpmPerAcres",                                                             DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             0.742410169,                                            5),
                                new Unit("gpmPerPSI",                                                           DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.UsCustomary),              new EmitterCoefficientConverterUS(15850.323),                                           19),
                                new     Unit("gpmPerSquareFeet",                                                DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1.7043324e-5,                                           4),
                                new     Unit("gpmPerSquareMiles",                                               DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             475.1406074,                                            6),
                                new     Unit("gram",                                                                    DimensionAt(DimensionIndex.Mass),                                                               UnitSystemAt(UnitSystemIndex.None),                             0.001,                                                          2),
                                new     Unit("gramsPerDay",                                                             DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             86.4,                                                           18),
                                new     Unit("gramsPerHour",                                                    DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             3.6,                                                            13),
                                new     Unit("gramsPerMinute",                                                  DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             6.0e-2,                                                         8),
                                new     Unit("gramsPerSecond",                                                  DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.0e-3,                                                         3),
                                new     Unit("guest",                                                                   DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            8),
                                new     Unit("hectares",                                                                DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0e-4,                                                         4),
                                new     Unit("hertz",                                                                   DimensionAt(DimensionIndex.ElectricalFrequency),                                UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("horizontalPerVertical",                                   DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new SlopeConverter(),                   9),
                                new     Unit("horsepower",                                                              DimensionAt(DimensionIndex.Power),                                                              UnitSystemAt(UnitSystemIndex.UsCustomary),              1.34124,                                                        3),
                                new     Unit("hours",                                                                   DimensionAt(DimensionIndex.Time),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            3),
                                new     Unit("hundredCapita",                                                   DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             0.01,                                                           3),
                                new     Unit("imperialGallonsPerDay",                                   DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             19005356.16,                                            10),
                                new     Unit("imperialGallonsPerMinute",                                DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             13198.164,                                                      21),
                                new     Unit("imperialGallonsPerSecond",                                DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             219.9694,                                                       9),
                                new     Unit("impGallons",                                                              DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             219.9694,                                                       6),
                                new Unit("imperialGallonsPerDayPerPSI",                         DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(19005356.16),                                         10),
                                new Unit("imperialGallonsPerMinutePerPSI",                      DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(13198.164),                                           20),
                                new Unit("imperialGallonsPerSecondPerPSI",                      DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(219.9694),                                            9),
                                new     Unit("inches",                                                                  DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              39.37007874,                                            8),
                                new     Unit("inchesPerDay",                                                    DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             56.69291376,                                            3),
                                new     Unit("inchesPerHour",                                                   DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.UsCustomary),              2.36220474,                                                     4),
                                new     Unit("inchesPerMinute",                                                 DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             0.039370079,                                            5),
                                new     Unit("inchPerFoot",                                                             DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             12.0,                                                           11),
                                new     Unit("infiltrationRateCentimetersPerDay",               DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             144,                                                            7),
                                new     Unit("infiltrationRateCentimetersPerHour",              DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             6,                                                                      9),
                                new     Unit("infiltrationRateCentimetersPerMinute",    DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             0.1,                                                            2),
                                new     Unit("infiltrationRateInchesPerDay",                    DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             56.69291376,                                            3),
                                new     Unit("infiltrationRateInchesPerHour",                   DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             2.36220474,                                                     4),
                                new     Unit("infiltrationRateInchesPerMinute",                 DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             0.039370079,                                            5),
                                new     Unit("infiltrationRateMillimetersPerDay",               DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             1440,                                                           6),
                                new     Unit("infiltrationRateMillimetersPerHour",              DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             60,                                                                     8),
                                new     Unit("infiltrationRateMillimetersPerMinute",    DimensionAt(DimensionIndex.InfiltrationRate),                                   UnitSystemAt(UnitSystemIndex.None),                             1,                                                                      1),
                                new     Unit("joules",                                                                  DimensionAt(DimensionIndex.Energy),                                                             UnitSystemAt(UnitSystemIndex.None),                             1,                                                                      1),
                                // TODO 9 : Provided by .Net Framework?
                                //new   Unit("kiloCurrency",                                                    DimensionAt(DimensionIndex.Currency),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.001,                                                          2),
                                new     Unit("kilograms",                                                               DimensionAt(DimensionIndex.Mass),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("kilogramsPerDay",                                                 DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             8.64e-2,                                                        19),
                                new     Unit("kilogramsPerHour",                                                DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             3.6e-3,                                                         14),
                                new     Unit("kilogramsPerMinute",                                              DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             6.0e-5,                                                         9),
                                new     Unit("kilogramsPerSecond",                                              DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.0e-6,                                                         4),
                                new     Unit("kilogramsPerSquareCentimeter",                    DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.019716e-2,                                            5),
                                new     Unit("kiloJoules",                                                              DimensionAt(DimensionIndex.Energy),                                                             UnitSystemAt(UnitSystemIndex.None),                             1.0e-3,                                                         2),
                                new     Unit("kilometers",                                                              DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.None),                             0.001,                                                          2),
                                new     Unit("kiloNewtonsPerCubicMeter",                                DimensionAt(DimensionIndex.SpecificWeight),                                             UnitSystemAt(UnitSystemIndex.None),                             0.001,                                                          2),
                                new     Unit("kiloPascals",                                                             DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            3),
                                new     Unit("kiloWattHours",                                                   DimensionAt(DimensionIndex.Energy),                                                             UnitSystemAt(UnitSystemIndex.Both),                             1.0/3600000,                                            3),
                                new     Unit("kilowatts",                                                               DimensionAt(DimensionIndex.Power),                                                              UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            2),
                                new     Unit("liters",                                                                  DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         2),
                                new     Unit("litersPerCapitaPerDay",                                   DimensionAt(DimensionIndex.FlowDensityPerCapita),                               UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            2),
                                new     Unit("litersPerDay",                                                    DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             86400000.0,                                                     1),
                                new Unit("litersPerDayPerMetersOfH2O",                          DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(86400000.0),                                                  1),
                                new     Unit("litersPerMinute",                                                 DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             60000,                                                          14),
                                new Unit("litersPerMinutePerMetersOfH2O",                       DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(60000),                                                               13),
                                new     Unit("litersPerSecond",                                                 DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.Si),                               1000.0,                                                         3),
                                new Unit("litersPerSecondPerMetersOfH2O",                       DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.Si),                               1000.0,                                                         3),
                                new     Unit("litersPerHectaresPerDay",                                 DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1e4,                                                            12),
                                new     Unit("litersPerSquareKilometerPerDay",                  DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.None),                             1e6,                                                            11),
                                new     Unit("litersPerSquareMeterPerDay",                              DimensionAt(DimensionIndex.FlowDensityPerArea),                                 UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            10),
                                // TODO 9 : Provided by .Net Framework?
                                //new   Unit("megaCurrency",                                                    DimensionAt(DimensionIndex.Currency),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.000001,                                                       3),
                                new     Unit("megaLitersPerDay",                                                DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             86.4,                                                           2),
                                new Unit("megaLitersPerDayPerMetersOfH2O",                      DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterSI(86.4),                                                                2),
                                new     Unit("meterHorizontalPerMeterVertical",                 DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new SlopeConverter(),                   15),
                                new     Unit("meterPerKilometer",                                               DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         12),
                                new     Unit("meterPerMeter",                                                   DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new BaseUnitConverter(),                        13),
                                new     Unit("meters",                                                                  DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            1),
                                new     Unit("metersOfH2O",                                                             DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.101972,                                                       4),
                                new     Unit("metersPerCm",                                                             DimensionAt(DimensionIndex.Scale),                                                              UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("metersPerDay",                                                    DimensionAt(DimensionIndex.SurfaceReactionRate),                                UnitSystemAt(UnitSystemIndex.Si),                               86400,                                                          2),
                                new     Unit("metersPerSecond",                                                 DimensionAt(DimensionIndex.SurfaceReactionRate),                                UnitSystemAt(UnitSystemIndex.None),                             1,                                                                      1),
                                new     Unit("meterVerticalPerMeterHorizontal",                 DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new BaseUnitConverter(),                14),
                                new     Unit("mfeet",                                                                   DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              3280.839895,                                            6),
                                new     Unit("mgd",                                                                             DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             22.82446512,                                            26),
                                new Unit("mgdPerPSI",                                                           DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(22.82446512),                                         21),
                                new     Unit("mgdImperial",                                                             DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             19.00535616,                                            27),
                                new Unit("mgdImperialPerPSI",                                           DimensionAt(DimensionIndex.EmitterCoefficient),                                 UnitSystemAt(UnitSystemIndex.None),                             new EmitterCoefficientConverterUS(19.00535616),                                         22),
                                new     Unit("microgramsPerDay",                                                DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             8.64e7,                                                         16),
                                new     Unit("microgramsPerHour",                                               DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             3600000.0,                                                      11),
                                new     Unit("microgramsPerLiter",                                              DimensionAt(DimensionIndex.Concentration),                                              UnitSystemAt(UnitSystemIndex.None),                             1000,                                                           1),
                                new     Unit("microgramsPerLiterNPerDay",                               DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             86400000.0,                                                     2),
                                new     Unit("microgramsPerLiterNPerSecond",                    DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             1000,                                                           1),
                                new     Unit("microgramsPerMinute",                                             DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             60000.0,                                                        6),
                                new     Unit("microgramsPerSecond",                                             DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         1),
                                new     Unit("microgramsPerSquareFeetPerDay",                   DimensionAt(DimensionIndex.ZeroOrderSurfaceReactionRate),               UnitSystemAt(UnitSystemIndex.None),                             8026822.967,                                            3),
                                new     Unit("microgramsPerSquareMeterPerDay",                  DimensionAt(DimensionIndex.ZeroOrderSurfaceReactionRate),               UnitSystemAt(UnitSystemIndex.None),                             86400000.0,                                                     2),
                                new     Unit("microgramsPerSquareMeterPerSecond",               DimensionAt(DimensionIndex.ZeroOrderSurfaceReactionRate),               UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         1),
                                new     Unit("miles",                                                                   DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              6.213711922e-4,                                         10),
                                new     Unit("millifeet",                                                               DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              3280.839895,                                            7),
                                new     Unit("milligram",                                                               DimensionAt(DimensionIndex.Mass),                                                               UnitSystemAt(UnitSystemIndex.None),                             0.000001,                                                       3),
                                new     Unit("milliGramsPerDay",                                                DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             86400.0,                                                        17),
                                new     Unit("milliGramsPerHour",                                               DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             3600.0,                                                         12),
                                new     Unit("milliGramsPerLiter",                                              DimensionAt(DimensionIndex.Concentration),                                              UnitSystemAt(UnitSystemIndex.Both),                             1,                                                                      3),
                                new     Unit("milliGramsPerLiterNPerDay",                               DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.Si),                               86400.0,                                                        6),
                                new     Unit("milliGramsPerLiterNPerSecond",                    DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            5),
                                new     Unit("milliGramsPerMinute",                                             DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             60.0,                                                           7),
                                new     Unit("milliGramsPerSecond",                                             DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.Both),                             1.0,                                                            2),
                                new     Unit("milliGramsPerSquareFeetPerDay",                   DimensionAt(DimensionIndex.ZeroOrderSurfaceReactionRate),               UnitSystemAt(UnitSystemIndex.UsCustomary),              8026.822967,                                            6),
                                new     Unit("milliGramsPerSquareMeterPerDay",                  DimensionAt(DimensionIndex.ZeroOrderSurfaceReactionRate),               UnitSystemAt(UnitSystemIndex.Si),                               86400.0,                                                        5),
                                new     Unit("milliGramsPerSquareMeterPerSecond",               DimensionAt(DimensionIndex.ZeroOrderSurfaceReactionRate),               UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            4),
                                new     Unit("millimeterHorizontalPerMeterVertical",    DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             100000.0,                                   18),
                                new     Unit("millimeterPerMeter",                                              DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         16),
                                new     Unit("millimeters",                                                             DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         4),
                                new     Unit("millimetersOfH2O",                                                DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             101.972,                                                        2),
                                new     Unit("milliMetersPerDay",                                               DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             1440,                                                           6),
                                new     Unit("milliMetersPerHour",                                              DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.Si),                               60,                                                                     8),
                                new     Unit("milliMetersPerMinute",                                    DimensionAt(DimensionIndex.RainfallIntensity),                                  UnitSystemAt(UnitSystemIndex.None),                             1,                                                                      1),
                                new     Unit("millimeterVerticalPerMeterHorizontal",    DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                         17),
                                new     Unit("millionGallons",                                                  DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             2.6417205e-4,                                           11),
                                new     Unit("millionLiters",                                                   DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             1.0e-3,                                                         14),
                                new     Unit("minutes",                                                                 DimensionAt(DimensionIndex.Time),                                                               UnitSystemAt(UnitSystemIndex.None),                             60.0,                                                           2),
                                new     Unit("newtonsPerCubicMeter",                                    DimensionAt(DimensionIndex.SpecificWeight),                                             UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            1),
                                new     Unit("newtonsPerSquareMeter",                                   DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         1),
                                new     Unit("oneOverSlope",                                                    DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            2),
                                new     Unit("partsPerBillion",                                                 DimensionAt(DimensionIndex.Concentration),                                              UnitSystemAt(UnitSystemIndex.None),                             1000,                                                           2),
                                new     Unit("partsPerBillionNPerDay",                                  DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             86400000.0,                                                     4),
                                new     Unit("partsPerBillionNPerSecond",                               DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             1000,                                                           3),
                                new     Unit("partsPerMillion",                                                 DimensionAt(DimensionIndex.Concentration),                                              UnitSystemAt(UnitSystemIndex.None),                             1,                                                                      4),
                                new     Unit("partsPerMillionNPerDay",                                  DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             86400.0,                                                        8),
                                new     Unit("partsPerMillionNPerSecond",                               DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            7),
                                new     Unit("passenger",                                                               DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            6),
                                new     Unit("perDay",                                                                  DimensionAt(DimensionIndex.ReactionRate),                                               UnitSystemAt(UnitSystemIndex.UsCustomary),              86400,                                                          2),
                                new     Unit("percentPercent",                                                  DimensionAt(DimensionIndex.Percent),                                                    UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("percentSlope",                                                    DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             100.0,                                                          1),
                                new     Unit("perSecond",                                                               DimensionAt(DimensionIndex.ReactionRate),                                               UnitSystemAt(UnitSystemIndex.Si),                               1,                                                                      1),
                                new     Unit("person",                                                                  DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            7),
                                new     Unit("personsPerAcre",                                                  DimensionAt(DimensionIndex.PopulationDensityPerArea),                   UnitSystemAt(UnitSystemIndex.UsCustomary),              4046.8564464278,                                        2),
                                new     Unit("personsPerSquareFeet",                                    DimensionAt(DimensionIndex.PopulationDensityPerArea),                   UnitSystemAt(UnitSystemIndex.None),                             0.09290304,                                                     1),
                                new     Unit("personsPerSquareKilometer",                               DimensionAt(DimensionIndex.PopulationDensityPerArea),                   UnitSystemAt(UnitSystemIndex.Si),                               1e6,                                                            5),
                                new     Unit("personsPerHectares",                                              DimensionAt(DimensionIndex.PopulationDensityPerArea),                   UnitSystemAt(UnitSystemIndex.None),                             1e4,                                                            6),
                                new     Unit("personsPerSquareMeter",                                   DimensionAt(DimensionIndex.PopulationDensityPerArea),                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            4),
                                new     Unit("personsPerSquareMile",                                    DimensionAt(DimensionIndex.PopulationDensityPerArea),                   UnitSystemAt(UnitSystemIndex.None),                             2589988.1005586708,                                     3),
                                new     Unit("pounds",                                                                  DimensionAt(DimensionIndex.Mass),                                                               UnitSystemAt(UnitSystemIndex.None),                             2.2046226,                                                      4),
                                new     Unit("poundsForcePerCubicFoot",                                 DimensionAt(DimensionIndex.SpecificWeight),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              6.365882e-3,                                            3),
                                new     Unit("poundsPerCubicFoot",                                              DimensionAt(DimensionIndex.Concentration),                                              UnitSystemAt(UnitSystemIndex.None),                             6.242621e-5,                                            5),
                                new     Unit("poundsPerCubicFootNPerDay",                               DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             9.11336779,                                                     10),
                                new     Unit("poundsPerCubicFootNPerSecond",                    DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             6.242621e-5,                                            9),
                                new     Unit("poundsPerDay",                                                    DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             0.1904793926,                                           20),
                                new     Unit("poundsPerHour",                                                   DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             7.93664136e-3,                                          15),
                                new     Unit("poundsPerMillionGallons",                                 DimensionAt(DimensionIndex.Concentration),                                              UnitSystemAt(UnitSystemIndex.None),                             8.3452,                                                         6),
                                new     Unit("poundsPerMillionGallonsNPerDay",                  DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             721025.28,                                                      12),
                                new     Unit("poundsPerMillionGallonsNPerSecond",               DimensionAt(DimensionIndex.NthOrderBulkReactionRate),                   UnitSystemAt(UnitSystemIndex.None),                             8.3452,                                                         11),
                                new     Unit("poundsPerMinute",                                                 DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.32277356e-4,                                          10),
                                new     Unit("poundsPerSecond",                                                 DimensionAt(DimensionIndex.MassRate),                                                   UnitSystemAt(UnitSystemIndex.None),                             2.2046226e-6,                                           5),
                                new     Unit("poundsPerSquareFoot",                                             DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             20.8854,                                                        7),
                                new     Unit("poundsPerSquareInch",                                             DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.45037738e-1,                                          9),
                                new     Unit("psi",                                                                             DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.45037738e-1,                          10),
                                new     Unit("resident",                                                                DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            9),
                                new     Unit("rpm",                                                                             DimensionAt(DimensionIndex.RotationalFrequency),                                UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            1),
                                new     Unit("seconds",                                                                 DimensionAt(DimensionIndex.Time),                                                               UnitSystemAt(UnitSystemIndex.None),                             3600.0,                                                         1),
                                new     Unit("squareCentimeters",                                               DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             10000.0,                                                        2),
                                new     Unit("squareFeet",                                                              DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.UsCustomary),              10.7639104,                                             7),
                                new     Unit("squareFeetPerSecond",                                             DimensionAt(DimensionIndex.Diffusivity),                                                UnitSystemAt(UnitSystemIndex.UsCustomary),              10.76391,                                                       2),
                                new     Unit("squareInches",                                                    DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             1550.0031,                                                      6),
                                new     Unit("squareKilometers",                                                DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0e-6,                                                         5),
                                new     Unit("squareMeters",                                                    DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            3),
                                new     Unit("squareMetersPerSecond",                                   DimensionAt(DimensionIndex.Diffusivity),                                                UnitSystemAt(UnitSystemIndex.Si),                               1,                                                                      1),
                                new     Unit("squareMiles",                                                             DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             3.86102159e-7,                                          11),
                                new     Unit("squareMillimeters",                                               DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             1000000.0,                                                      1),
                                new     Unit("squareYards",                                                             DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.19599005,                                                     9),
                                new     Unit("stokes",                                                                  DimensionAt(DimensionIndex.Diffusivity),                                                UnitSystemAt(UnitSystemIndex.None),                             1.0e4,                                                          3),
                                new     Unit("student",                                                                 DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            10),
                                new     Unit("thousandCapita",                                                  DimensionAt(DimensionIndex.Population),                                                 UnitSystemAt(UnitSystemIndex.None),                             0.001,                                                          2),
                                new     Unit("thousandGallons",                                                 DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             2.6417205e-1,                                           12),
                                new     Unit("thousandLiters",                                                  DimensionAt(DimensionIndex.Volume),                                                             UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            13),
                                new     Unit("thousandSquareFeet",                                              DimensionAt(DimensionIndex.Area),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.076391e-2,                                            8),
                                new     Unit("unitlessPercent",                                                 DimensionAt(DimensionIndex.Percent),                                                    UnitSystemAt(UnitSystemIndex.None),                             0.01,                                                           2),
                            // KCUL - December 30, 2002
                            new Unit("unitlessUnit",                                                    DimensionAt(DimensionIndex.Unitless),                                                   UnitSystemAt(UnitSystemIndex.None),                             1,                                                                      0),
                                new     Unit("velocityCentimetersPerHour",                              DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             3.6e5,                                                          1),
                                new     Unit("velocityCentimetersPerMinute",                    DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             6.0e3,                                                          2),
                                new     Unit("velocityCentimetersPerSecond",                    DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             100.0,                                                          3),
                                new     Unit("velocityFeetPerHour",                                             DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.18110234e4,                                           9),
                                new     Unit("velocityFeetPerMinute",                                   DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             196.85039,                                                      11),
                                new     Unit("velocityFeetPerSecond",                                   DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             3.2808399,                                                      13),
                                new     Unit("velocityInchesPerHour",                                   DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.417323e5,                                                     8),
                                new     Unit("velocityInchesPerMinute",                                 DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             2.36220474e3,                                           10),
                                new     Unit("velocityInchesPerSecond",                                 DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             39.370079,                                                      12),
                                new     Unit("velocityKilometersPerHour",                               DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             3.6,                                                            6),
                                new     Unit("velocityKnot",                                                    DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.942606763,                                            16),
                                new     Unit("velocityKnotInternational",                               DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.94384448,                                                     15),
                                new     Unit("velocityMetersPerHour",                                   DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             3600,                                                           4),
                                new     Unit("velocityMetersPerMinute",                                 DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             60.0,                                                           5),
                                new     Unit("velocityMetersPerSecond",                                 DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             1.0,                                                            7),
                                new     Unit("velocityMilePerHour",                                             DimensionAt(DimensionIndex.Velocity),                                                   UnitSystemAt(UnitSystemIndex.None),                             2.2369363,                                                      14),
                                new     Unit("verticalPerHorizontal",                                   DimensionAt(DimensionIndex.Slope),                                                              UnitSystemAt(UnitSystemIndex.None),                             new BaseUnitConverter(),                        10),
                                new     Unit("watts",                                                                   DimensionAt(DimensionIndex.Power),                                                              UnitSystemAt(UnitSystemIndex.None),                             1000.0,                                                         1),
                                new     Unit("weircoefficientSi",                                               DimensionAt(DimensionIndex.WeirCoefficient),                                    UnitSystemAt(UnitSystemIndex.Si),                               0.5520735,                                                      1),
                                new     Unit("weircoefficientUs",                                               DimensionAt(DimensionIndex.WeirCoefficient),                                    UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            2),
                                new     Unit("yards",                                                                   DimensionAt(DimensionIndex.Length),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0936133,                                                      9),
                                new     Unit("years",                                                                   DimensionAt(DimensionIndex.Time),                                                               UnitSystemAt(UnitSystemIndex.None),                             1.0/8760.0,                                                     5),
                                // As of Domain v3 and CSD, UnitIndex's cannot be changed. New units must be added sequentially, at the end.
                                new     Unit("millionLitersPerDay",                                             DimensionAt(DimensionIndex.Flow),                                                               UnitSystemAt(UnitSystemIndex.None),                             86.40,                                                          28),
                                new Unit("perMinute",                                                           DimensionAt(DimensionIndex.ReactionRate),                                               UnitSystemAt(UnitSystemIndex.None),                             60,                                                                     3),

                                new Unit("inchMiles",                                                           DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            1),
                new Unit("inchFeet",                                                            DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              5280.0,                                                         2),
                                new Unit("footMiles",                                                           DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              (1.0 / 12.0),                                           3),
                                new Unit("footFeet",                                                            DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              440.0,                                                          4),
                                new Unit("millimeterMeters",                                            DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.Si),                               40877.3376,                                                     5),
                                new Unit("millimeterKilometers",                                        DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.Si),                               40.8773376,                                                     6),
                                new Unit("meterMeters",                                                         DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.Si),                               40.8773376,                                                     7),
                                new Unit("meterKilometers",                                                     DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.Si),                               0.040877338,                                            8),
                                new Unit("inchMeters",                                                          DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              1609.34400,                                                     9),
                                new Unit("millimeterMiles",                                                     DimensionAt(DimensionIndex.DiameterLength),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              25.4000000,                                                     10),

                                new Unit("poundsPerAcre",                                                       DimensionAt(DimensionIndex.MassPerArea),                                                UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            1),
                                new Unit("kilogramsPerHectare",                                         DimensionAt(DimensionIndex.MassPerArea),                                                UnitSystemAt(UnitSystemIndex.Si),                               1.12085116518377,                                       2),
                                new CurrencyBasedUnit("dollarsPerKiloWatt",                 DimensionAt(DimensionIndex.CurrencyPerPower),                                       UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            1),
                                new Unit("poundSquareFeet",                                                     DimensionAt(DimensionIndex.Inertia),                                                    UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            1),
                                new Unit("newtonSquareMeters",                                          DimensionAt(DimensionIndex.Inertia),                                                    UnitSystemAt(UnitSystemIndex.Si),                               0.41325322,                                                     2),
                                new CurrencyBasedUnit("dollarsPerHorsepower",           DimensionAt(DimensionIndex.CurrencyPerPower),                                   UnitSystemAt(UnitSystemIndex.UsCustomary),              0.745699872,                                            2),

                                new CurrencyBasedUnit("dollarsPerCubicCentimeters",     DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/1000000.0000000,                                      1),
                                new CurrencyBasedUnit("dollarsPerLiters",                       DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/1000.0000000,                                         2),
                                new CurrencyBasedUnit("dollarsPerCubicMeters",          DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/1.0000000,                                            3),
                                new CurrencyBasedUnit("dollarsPerCubicInches",          DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/61023.7400000,                                        4),
                                new CurrencyBasedUnit("dollarsPerGallons",                      DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/264.1720500,                                          5),
                                new CurrencyBasedUnit("dollarsPerImpGallons",           DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/219.9694000,                                          6),
                                new CurrencyBasedUnit("dollarsPerCubicFeet",            DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/35.3146670,                                           7),
                                new CurrencyBasedUnit("dollarsPerCubicYards",           DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/1.3079506,                                            8),
                                new CurrencyBasedUnit("dollarsPerAcreInches",           DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/0.0097286,                                            9),
                                new CurrencyBasedUnit("dollarsPerAcreFeet",                     DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/0.0008107,                                            10),
                                new CurrencyBasedUnit("dollarsPerMillionGallons",       DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/0.0002642,                                            11),
                                new CurrencyBasedUnit("dollarsPerThousandGallons",      DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/0.2641721,                                            12),
                                new CurrencyBasedUnit("dollarsPerThousandLiters",       DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/1.0000000,                                            13),
                                new CurrencyBasedUnit("dollarsPerMillionLiters",        DimensionAt(DimensionIndex.CostPerUnitVolume),                                  UnitSystemAt(UnitSystemIndex.Si),                               1/0.0010000,                                            14),
                                new Unit("kilogramsPerSquareMeter",                                     DimensionAt(DimensionIndex.Pressure),                                                   UnitSystemAt(UnitSystemIndex.None),                             101.971621,                                                     12),
                                new Unit("kiloWattHourPerMillionGallons",                       DimensionAt(DimensionIndex.EnergyPerUnitVolume),                                UnitSystemAt(UnitSystemIndex.UsCustomary),              1/0.0002642,                                            1),
                                new Unit("kiloWattHourPerMillionLiters",                        DimensionAt(DimensionIndex.EnergyPerUnitVolume),                                UnitSystemAt(UnitSystemIndex.Si),                               1/0.0010000,                                            2),
                                new Unit("kiloWattHourPerCubicMeters",                          DimensionAt(DimensionIndex.EnergyPerUnitVolume),                                UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            3),
                                new Unit("kiloWattHourPerCubicFeet",                            DimensionAt(DimensionIndex.EnergyPerUnitVolume),                                UnitSystemAt(UnitSystemIndex.UsCustomary),              1/35.3146670,                                           4),
                                new Unit("perHour",                                                                     DimensionAt(DimensionIndex.ReactionRate),                                               UnitSystemAt(UnitSystemIndex.None),                             3600,                                                           4),

                                new Unit("newtonMeters",                                                        DimensionAt(DimensionIndex.Torque),                                                             UnitSystemAt(UnitSystemIndex.Si),                               1.0,                                                            2),
                                new Unit("poundFeet",                                                           DimensionAt(DimensionIndex.Torque),                                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              0.737562,                                                       1),
                                new Unit("poundPerInch",                                                        DimensionAt(DimensionIndex.SpringConstant),                                             UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            1),
                                new Unit("newtonPerMillimeter",                                         DimensionAt(DimensionIndex.SpringConstant),                                             UnitSystemAt(UnitSystemIndex.Si),                               0.175126797,                                            2),
                                new Unit("poundForce",                                                          DimensionAt(DimensionIndex.Force),                                                              UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            1),
                                new Unit("kiloPoundForce",                                                      DimensionAt(DimensionIndex.Force),                                                              UnitSystemAt(UnitSystemIndex.UsCustomary),              0.001,                                                          2),
                                new Unit("newton",                                                                      DimensionAt(DimensionIndex.Force),                                                              UnitSystemAt(UnitSystemIndex.Si),                               4.4488222,                                                      3),
                                new Unit("kiloNewton",                                                          DimensionAt(DimensionIndex.Force),                                                              UnitSystemAt(UnitSystemIndex.Si),                               0.0044488222,                                           4),
                                new Unit("slugPerCubicFoot",                                            DimensionAt(DimensionIndex.Density),                                                    UnitSystemAt(UnitSystemIndex.UsCustomary),              1.0,                                                            1),
                                new Unit("poundPerCubicFoot",                                           DimensionAt(DimensionIndex.Density),                                                    UnitSystemAt(UnitSystemIndex.UsCustomary),              32.174054,                                                      2),
                                new Unit("kilogramPerCubicMeter",                                       DimensionAt(DimensionIndex.Density),                                                    UnitSystemAt(UnitSystemIndex.Si),                               515.3788,                                                       3),
                                new Unit("cfsPerSquareRootFooH20",                                      DimensionAt(DimensionIndex.DischargePerPressureDrop),                   UnitSystemAt(UnitSystemIndex.UsCustomary),              19.5,                                                           1),
                                new Unit("cmsPerSquareRootMeterH20",                            DimensionAt(DimensionIndex.DischargePerPressureDrop),                   UnitSystemAt(UnitSystemIndex.Si),                               1,                                                                      2),
                                new Unit("lPerSecPerSquareRootKpa",                                     DimensionAt(DimensionIndex.DischargePerPressureDrop),                   UnitSystemAt(UnitSystemIndex.Si),                               319.3305497,                                            3),
                                new Unit("gpmPerSquareRootPsi",                                         DimensionAt(DimensionIndex.DischargePerPressureDrop),                   UnitSystemAt(UnitSystemIndex.UsCustomary),              13290.37762,                                            4),
                                new     Unit("milliseconds",                                                    DimensionAt(DimensionIndex.Time),                                                               UnitSystemAt(UnitSystemIndex.None),                             3600000.0,                                                      6),
                        };
                }

        protected void InitializeUnitSystems()
        {
            m_rgunitsystem = new UnitSystem[] {
                new UnitSystem("none"),
                new UnitSystem("si"),
                new UnitSystem("usCustomary"),
                                new UnitSystem("both")
            };
        }

                #endregion

                #region Private Fields

                static private UnitConversionManager CurrentManager;
        private Dimension[] m_rgdimension;
        private Unit[] m_rgunit;
        private UnitSystem[] m_rgunitsystem;
                static private double m_EmitterExponent;

                #endregion

                #region BaseUnitConverter

                /// <summary>
        /// Nop converter for the base unit in an equation dimension.
        /// </summary>
        private class BaseUnitConverter : IUnitConverter
        {
            public double FromBaseUnit(double adouble)
            {
                return adouble;
            }
            public double ToBaseUnit(double adouble)
            {
                return adouble;
            }
        }

                #endregion

                #region DegreesFahrenheitConverter

                private class DegreesFahrenheitConverter : IUnitConverter
        {
            public double FromBaseUnit(double adoubleCelsius)
            {
                return ((9.0/5.0) * adoubleCelsius) + 32.0;
            }
            public double ToBaseUnit(double adoubleFahrenheit)
            {
                return (5.0/9.0) * (adoubleFahrenheit - 32.0);
            }
        }

                #endregion

                #region SlopeConverter

                private class SlopeConverter : IUnitConverter
                {
                        public double FromBaseUnit(double adouble)
                        {
                                if(adouble == 0.0)
                                        return 0.0;
                                else return 1/adouble;
                        }
                        public double ToBaseUnit(double adouble)
                        {
                                if(adouble == 0.0)
                                        return 0.0;
                                else return 1/adouble;
                        }
                }

                #endregion

                #region EmitterCoefficient Converters

                private class EmitterCoefficientConverterUS : IUnitConverter
                {
                        public EmitterCoefficientConverterUS(double aTargetUnitFlowFactor)
                        {
                                m_TargetUnitFlowFactor = aTargetUnitFlowFactor;
                        }
                        public double FromBaseUnit(double aDouble)
                        {
                                if(aDouble == 0.0)
                                        return 0.0;
                                else return aDouble * m_TargetUnitFlowFactor / Math.Pow(TARGET_UNIT_PRESSURE_FACTOR, UnitConversionManager.EmitterExponent);
                        }
                        public double ToBaseUnit(double aDouble)
                        {
                                if(aDouble == 0.0)
                                        return 0.0;
                                else return aDouble * Math.Pow(TARGET_UNIT_PRESSURE_FACTOR, UnitConversionManager.EmitterExponent)/ (m_TargetUnitFlowFactor);
                        }

                        protected virtual double TARGET_UNIT_PRESSURE_FACTOR
                        {
                                get { return 1.422334; }
                        }

                        private double m_TargetUnitFlowFactor;
                }

                private class EmitterCoefficientConverterSI : EmitterCoefficientConverterUS
                {
                        public EmitterCoefficientConverterSI(double aTargetUnitFlowFactor)
                                : base(aTargetUnitFlowFactor)
                        {
                        }
                        protected override double TARGET_UNIT_PRESSURE_FACTOR
                        {
                                get { return 1.0; }
                        }
                }
                #endregion EmitterCoefficient Converter



    }
}
