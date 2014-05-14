

#include <LTBL/Light/EmissiveLight.h>

#include <cmath>

//TODO figure out how to rotate without changing measurements, hmm don't think that's possible

using namespace ltbl;

EmissiveLight::EmissiveLight()
	: m_angleDegs(0.0f), m_color(1.0f, 1.0f, 1.0f), m_intensity(1.0f)
{
}

void EmissiveLight::setTexture(sf::Texture* texture)
{

	m_texture = texture;

	// Update aabb and the render dims
	sf::Vector2u textureSize(m_texture->getSize());
	m_halfRenderDims.x = static_cast<float>(textureSize.x) / 2.0f;
	m_halfRenderDims.y = static_cast<float>(textureSize.y) / 2.0f;

	m_aabb.setHalfDims(m_halfRenderDims);

	m_aabb.calculateHalfDims();
	m_aabb.calculateCenter();

	treeUpdate();
}

void EmissiveLight::render()
{
	glPushMatrix();
	const sf::Vector2f &center = m_aabb.getCenter();

	sf::Texture::bind(m_texture);

	glTranslatef(center.x, center.y, 0.0f);
	glRotatef(m_angleDegs, 0.0f, 0.0f, 1.0f);

	// Clamp the intensity
	float renderIntensity = m_intensity;

	if(renderIntensity > 1.0f)
		renderIntensity = 1.0f;

    //rgb
	glColor4f(m_color.x, m_color.y, m_color.z, renderIntensity);

	// Have to render upside-down because SFML loads the Textures upside-down

	glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex2f(-m_halfRenderDims.x, -m_halfRenderDims.y);
		glTexCoord2i(1, 0);
		glVertex2f(m_halfRenderDims.x, -m_halfRenderDims.y);
		glTexCoord2i(1, 1);
		glVertex2f(m_halfRenderDims.x, m_halfRenderDims.y);
		glTexCoord2i(0, 1);
		glVertex2f(-m_halfRenderDims.x, m_halfRenderDims.y);
	glEnd();

	// Reset color
	glColor3f(1.0f,1.0f, 1.0f);

	glPopMatrix();
}

void EmissiveLight::setCenter(const sf::Vector2f &newCenter)
{
	m_aabb.setCenter(newCenter);
	treeUpdate();
}

void EmissiveLight::incCenter(const sf::Vector2f &increment)
{
	m_aabb.incCenter(increment);
	treeUpdate();
}

sf::Vector2f EmissiveLight::getCenter()
{
	return m_aabb.getCenter();
}

void EmissiveLight::setDims(const sf::Vector2f &newDims)
{
	m_halfRenderDims = newDims / 2.0f;

	// set AABB
	m_aabb.setHalfDims(m_halfRenderDims);

	// Re-rotate AABB if it is rotated
	if(m_angleDegs != 0.0f)
		setRotation(m_angleDegs);

	treeUpdate();
}

sf::Vector2f EmissiveLight::getDims()
{
	return m_aabb.getDims();
}

void EmissiveLight::setRotation(float angleDegs)
{
    if(angleDegs < 0.f)
        angleDegs = 360.f + angleDegs;

	// Update the render angle
	m_angleDegs = angleDegs;

	// get original AABB
	m_aabb.setHalfDims(m_halfRenderDims);

	//m_aabb.calculateHalfDims();
	//m_aabb.calculateCenter();

	// If angle is the normal angle, exit
	if(m_angleDegs == 0.0f)
		return;

	// Rotate the aabb
	m_aabb.setRotatedAABB(angleDegs);

	treeUpdate();
}

void EmissiveLight::incRotation(float increment)
{
	// increment render angle
	m_angleDegs += increment;

	// Rotate the aabb without setting to original aabb, so rotation is relative
	m_aabb.setRotatedAABB(increment);
}

float EmissiveLight::getRotation()
{
	return m_angleDegs;
}

void EmissiveLight::setIntensity(float intensity)
{
    m_intensity = intensity;
}

void EmissiveLight::incIntensity(float increment)
{
    m_intensity += increment;
    treeUpdate();
}

float EmissiveLight::getIntensity() const
{
    return m_intensity;
}

void EmissiveLight::setColor(sf::Vector3f color)
{
    m_color = color;
}

sf::Vector3f EmissiveLight::getColor() const
{
    return m_color;
}
