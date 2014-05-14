#ifndef LTBL_AABB_H
#define LTBL_AABB_H

#include <LTBL/Utils.h>
#include <cmath>

/**
Axis Aligned Bounding Boxes (AABB)

necessary for the quad trees apparently...
maybe also for collision
*/

class AABB
{
private:
	sf::Vector2f m_center;
	sf::Vector2f m_halfDims; // distance from center to bounderies

    float m_angleDegs;
	sf::Vector2f m_lowerBound;
	sf::Vector2f m_upperBound;

public:
	void calculateHalfDims();
	void calculateCenter();
	void calculateBounds();

	// Constructor
	AABB();
	AABB(const sf::Vector2f &lowerBound, const sf::Vector2f &upperBound);

    void setRotatedAABB(float angleDegs);
	AABB getRotatedAABB(float angleDegs) const;

    void setDims(const sf::Vector2f &newDims);
	sf::Vector2f getDims() const;

	void setHalfDims(const sf::Vector2f &newDims);
	const sf::Vector2f &getHalfDims() const;

	void setBounds(const sf::Vector2f &newLowerBound, const sf::Vector2f &newUpperBound);

	void setLowerBound(const sf::Vector2f &newLowerBound);
	const sf::Vector2f &getLowerBound() const;

	void setUpperBound(const sf::Vector2f &newUpperBound);
	const sf::Vector2f &getUpperBound() const;

	void setCenter(const sf::Vector2f &newCenter);
	void incCenter(const sf::Vector2f &increment);
	const sf::Vector2f &getCenter() const;

	// Utility
	bool intersects(const AABB &other) const;
	bool contains(const AABB &other) const;

	// Render the AABB for debugging purposes
	void debugRender() const;

	//friend class AABB;
};

#endif
