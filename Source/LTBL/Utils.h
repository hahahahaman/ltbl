#ifndef LTBL_UTILS_H
#define LTBL_UTILS_H

#include <SFML/Graphics.hpp>
#include <cmath>
//#define M_PI 3.14159 //redfined cuz in windows it is not defined

#include <string>
#include <iostream>

//various utility functions common in ltbl

namespace ltbl
{

const float pif = static_cast<float>(M_PI);
const float pifTimes2 = pif * 2.f;

float rad2Degs(float angleRads);

float deg2Rads(float angleDegs);

template<class T>
T wrap(T val, T size);

float getFloatVal(std::string strConvert);

template <typename T>
T magnitude2d(const sf::Vector2<T>& vector);

template <typename T>
T magnitudeSquared2d(const sf::Vector2<T>& vector);

template <typename T>
sf::Vector2<T> normalize2d(const sf::Vector2<T>& vector);

template <typename T>
float dot2d(const sf::Vector2<T> vector1, const sf::Vector2<T>& vector2);

template <typename T>
float cross2d(const sf::Vector2<T> vector1, const sf::Vector2<T> &vector2);

template <typename T>
sf::Vector2<T> operator*(T scale, const sf::Vector2<T> &vector);

template <typename T>
std::ostream &operator<<(std::ostream &output, const sf::Vector2<T> &vector);

};

//for the templates declarations
#include <LTBL/Utils.inl>

#endif
