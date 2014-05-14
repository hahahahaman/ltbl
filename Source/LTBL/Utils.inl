//Utils.inl

/*
Declaration of template functions in Utils.h
*/

namespace ltbl
{
template<class T>
T wrap(T val, T size)
{
    if(val < 0)
        return val + size;

    if(val >= size)
        return val - size;

    return val;
};

template <typename T>
T magnitude2d(const sf::Vector2<T>& vector)
{
    return sqrt(vector.x * vector.x + vector.y * vector.y);
};

template <typename T>
T magnitudeSquared2d(const sf::Vector2<T>& vector)
{
    return vector.x * vector.x + vector.y * vector.y;
};

template <typename T>
sf::Vector2<T> normalize2d(const sf::Vector2<T>& vector)
{
    T m = sqrt(vector.x * vector.x + vector.y * vector.y);
	return sf::Vector2<T>(vector.x / m, vector.y / m);
};

template <typename T>
float dot2d(const sf::Vector2<T> vector1, const sf::Vector2<T>& vector2)
{
    return vector1.x * vector2.x + vector1.y * vector2.y;
};

template <typename T>
float cross2d(const sf::Vector2<T> vector1, const sf::Vector2<T> &vector2)
{
    return vector1.x * vector2.y - vector1.y * vector2.x;
};

template <typename T>
sf::Vector2<T> operator*(T scale, const sf::Vector2<T> &vector)
{
    return vector * scale;
};

template <typename T>
std::ostream &operator<<(std::ostream &output, const sf::Vector2<T> &vector)
{
	std::cout << '(' << vector.x << ", " << vector.y << ')';
	return output;
};

};
