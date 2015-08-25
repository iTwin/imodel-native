/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Foundations/Algorithm.hpp $
|    $RCSfile: Algorithm.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/12/20 16:23:43 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct StaticLinearIntegerGenerator<T, 1>
    {
private:
    T                               m_current;

public:
    explicit                        StaticLinearIntegerGenerator                   (T           start) 
        :   m_current(start - 1) {}
    T                               operator()                                     ()
        { return ++m_current; }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct StaticLinearIntegerGenerator<T, -1>
    {
private:
    T                               m_current;

public:
    explicit                        StaticLinearIntegerGenerator                   (T           start) 
        :   m_current(start + 1) {}
    T                               operator()                                     ()
        { return --m_current; }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
struct RandomIntegerGeneratorStrategy
    {
public:
    static T                        Generate                                   (const T&            min,
                                                                                const T&            range)
        {
        static const T MAX_BIN_SIZE = (T)RAND_MAX + 1;

        if (range <= ((T)MAX_BIN_SIZE))
            return min + (T)rand() % range;

        T lastBinSize = range % MAX_BIN_SIZE;
        if (0 == lastBinSize)
            lastBinSize = MAX_BIN_SIZE;

        const T binQty = (range / MAX_BIN_SIZE) + ((0 == lastBinSize) ? 0 : 1); 

        const T binIndex = Generate(0, binQty);
        const T binValue = binIndex * MAX_BIN_SIZE;

        const T newMin = min + binValue;
        const T newRange = (binIndex + 1 == binQty) ? lastBinSize : MAX_BIN_SIZE;

        return Generate(newMin, newRange);
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
RandomIntegerGenerator<T>::RandomIntegerGenerator  (const T&    min,
                                                    const T&    max) 
    : m_min(min), m_range(max - min + 1)
    { assert(max > min); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
T RandomIntegerGenerator<T>::operator() () const                     
    { return RandomIntegerGeneratorStrategy<T>::Generate(m_min, m_range); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, T T_MIN, T T_MAX, bool RangeSupported>
struct StaticRandomIntegerGeneratorStrategy
    {
private:
    static const T                  RANGE = T_MAX - T_MIN + 1;

public:
    static T                        Generate                                   ()
        {
        static const T MAX_BIN_SIZE = (T)RAND_MAX + 1;
        static const T LAST_BIN_SIZE = (0 == (RANGE % MAX_BIN_SIZE)) ? MAX_BIN_SIZE : RANGE % MAX_BIN_SIZE;
        static const T BIN_QTY = (RANGE / MAX_BIN_SIZE) + (0 == LAST_BIN_SIZE ? 0 : 1); 

        const T BinIndex = RandomIntegerGeneratorStrategy<T>::Generate(0, BIN_QTY);

        const T NewMin = T_MIN + BinIndex * MAX_BIN_SIZE;
        const T NewRange = (BinIndex + 1 == BIN_QTY) ? LAST_BIN_SIZE : MAX_BIN_SIZE;

        return RandomIntegerGeneratorStrategy<T>::Generate(NewMin, NewRange);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, T T_MIN, T T_MAX>
struct StaticRandomIntegerGeneratorStrategy<T, T_MIN, T_MAX, true>
    {
private:
    friend struct                   StaticRandomIntegerGenerator<T, T_MIN, T_MAX>;

    static const T                  RANGE = T_MAX - T_MIN + 1;

    static T                        Generate                                   ()
        {
        T_MIN + rand() % RANGE;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T, T T_MIN, T T_MAX>
T StaticRandomIntegerGenerator<T, T_MIN, T_MAX>::operator() () const 
    {
    static const bool RANGE_SUPPORTED = RANGE <= ((T)RAND_MAX + 1); 
    return StaticRandomIntegerGeneratorStrategy<T, T_MIN, T_MAX, RANGE_SUPPORTED>::Generate();
    }
