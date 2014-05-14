

#include <LTBL/Light/Light.h>

#include <cassert>

namespace ltbl
{

Light::Light()
    : m_intensity(1.0f),
      m_radius(100.0f),
      m_center(0.0f, 0.0f),
      m_color(1.0f, 1.0f, 1.0f),
      m_size(40.0f),
      m_updateRequired(true), m_alwaysUpdate(true), m_pStaticTexture(NULL), // For static light
      m_pWin(NULL), m_pLightSystem(NULL), m_shaderAttenuation(true),
      m_bleed(1.0f), m_linearizeFactor(0.2f)
{
}

Light::~Light()
{
    // Destroy the static Texture if one exists
    if(m_alwaysUpdate)
        delete m_pStaticTexture;
}

void Light::switchStaticTexture()
{
    sf::Vector2f dims(m_aabb.getDims());

    m_pStaticTexture->setActive();

    glViewport(0, 0, static_cast<unsigned int>(dims.x), static_cast<unsigned int>(dims.y));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Flip the projection, since SFML draws the bound textures (on the FBO) upside down
    glOrtho(0, static_cast<unsigned int>(dims.x), static_cast<unsigned int>(dims.y), 0, -100.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

/*
AABB* Light::getAABB()
{
    return &m_aabb;
}
*/

void Light::calculateAABB()
{
    m_aabb.setCenter(m_center);
    m_aabb.setDims(sf::Vector2f(m_radius, m_radius));
}

bool Light::alwaysUpdate()
{
    return m_alwaysUpdate;
}

void Light::setAlwaysUpdate(bool always)
{
    // Must add to the light system before calling this
    assert(m_pWin != NULL);

    if(!always && m_alwaysUpdate) // If previously set to false, create a render Texture for the static texture
    {
        sf::Vector2f dims(m_aabb.getDims());

        // Check if large enough textures are supported
        unsigned int maxDim = sf::Texture::getMaximumSize();

        if(maxDim <= dims.x || maxDim <= dims.y)
        {
            std::cout << "Attempted to create a too large static light. Switching to dynamic." << std::endl;
            return;
        }

        // Create the render Texture based on aabb dims
        m_pStaticTexture = new sf::RenderTexture();

        unsigned int uiDimsX = static_cast<unsigned int>(dims.x);
        unsigned int uiDimsY = static_cast<unsigned int>(dims.y);

        m_pStaticTexture->create(uiDimsX, uiDimsY, false);

        m_pStaticTexture->setSmooth(true);

        m_pStaticTexture->setActive();
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        m_pWin->setActive();

        m_updateRequired = true;
    }
    else if(always && !m_alwaysUpdate) // If previously set to true, destroy the render Texture
        delete m_pStaticTexture;

    m_alwaysUpdate = always;
}

void Light::setRadius(float radius)
{
    assert(m_alwaysUpdate);

    m_radius = radius;

    calculateAABB();
    treeUpdate();
}

void Light::incRadius(float increment)
{
    assert(m_alwaysUpdate);

    m_radius += increment;

    calculateAABB();
    treeUpdate();
}

float Light::getRadius() const
{
    return m_radius;
}

void Light::setCenter(const sf::Vector2f& center)
{
    m_center = center;

    m_aabb.setCenter(m_center);

    treeUpdate();

    m_updateRequired = true;
}

void Light::incCenter(const sf::Vector2f& increment)
{
    m_center += increment;

    m_aabb.incCenter(increment);

    treeUpdate();

    m_updateRequired = true;
}

sf::Vector2f Light::getCenter() const
{
    return m_center;
}

void Light::setIntensity(float intensity)
{
    m_intensity = intensity;
}

void Light::incIntensity(float increment)
{
    m_intensity += increment;

    m_updateRequired = true;
}

float Light::getIntensity() const
{
    return m_intensity;
}

void Light::setSize(float sizea)
{
    m_size = sizea;
}

void Light::incSize(float increment)
{
    m_size += increment;

    m_updateRequired = true;
}

float Light::getSize() const
{
    return m_size;
}

void Light::setBleed(float bleed)
{
    m_bleed = bleed;
}

void Light::incBleed(float increment)
{
    m_bleed = increment;

    m_updateRequired = true;
}

float Light::getBleed() const
{
    return m_bleed;
}

void Light::setLinearizeFactor(float linearizeFactor)
{
    m_linearizeFactor = linearizeFactor;
}

void Light::incLinearizeFactor(float increment)
{
    m_linearizeFactor += increment;

    m_updateRequired = true;
}

float Light::getLinearizeFactor() const
{
    return m_linearizeFactor;
}

void Light::setColor(const sf::Vector3f& color)
{
    m_color = color;
}

sf::Vector3f Light::getColor() const
{
    return m_color;
}

}
