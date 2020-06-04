//
// Created by Cyprien Plateau--Holleville on 20/06/2019.
//

#ifndef MIRAGE_MATH_HPP
#define MIRAGE_MATH_HPP

#include <cassert>
#include <cmath>

namespace mrg
{
    constexpr double Pi  = 3.14159265358979323846;
    constexpr double Tau = 6.28318530717958647692;

    inline double Atan(const double val)
    {
        return std::atan(val);
    }

    inline double Exp(const double val)
    {
        return std::exp(val);
    }

    inline double Sqrt(const double val)
    {
        return std::sqrt(val);
    }

    template<typename T>
    T Pow(const T val, const unsigned int power)
    {
        static_assert(std::is_arithmetic<T>::value, "This function only works with numbers.");
        return std::pow(val, power);
    }

    inline double Log(const double val)
    {
        return std::log(val);
    }

    inline double Sin(const double val)
    {
        return std::sin(val);
    }

    inline double Cos(const double val)
    {
        return std::cos(val);
    }

    template<typename T>
    T Ceil(const T n)
    {
        static_assert(std::is_arithmetic<T>::value, "This function only works with numbers.");
        return std::ceil(n);
    }

    template<typename T>
    T Floor(const T n)
    {
        static_assert(std::is_arithmetic<T>::value, "This function only works with numbers.");
        return static_cast<T>(std::floor(n));
    }

    template<typename T>
    T Trunc(const T n)
    {
        static_assert(std::is_arithmetic<T>::value, "This function only works with numbers.");
        return static_cast<T>(std::trunc(n));
    }
}

#endif //MIRAGE_MATH_HPP
