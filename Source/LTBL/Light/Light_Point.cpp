#include <LTBL/Light/Light_Point.h>
#include <LTBL/Light/ShadowFin.h>

#include <LTBL/Utils.h>

#include <cassert>

namespace ltbl
{

Light_Point::Light_Point()
    : m_directionAngle(0.0f), m_spreadAngle(pifTimes2),
      m_softSpreadAngle(0.0f), m_lightSubdivisionSize(pif / 24.0f),
      aabbCenterDiff(sf::Vector2f(0.0f, 0.0f))
{
}

Light_Point::~Light_Point()
{
}

void Light_Point::renderLightSolidPortion()
{
    float renderIntensity = getIntensity();

    // Clamp the render intensity
    if(renderIntensity > 1.0f)
        renderIntensity = 1.0f;

    assert(renderIntensity >= 0.0f);

    float r = getColor().x * renderIntensity;
    float g = getColor().y * renderIntensity;
    float b = getColor().z * renderIntensity;

    glColor4f(r, g, b, renderIntensity);
    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(getCenter().x, getCenter().y);

    // set the edge color for rest of shape
    int numSubdivisions = static_cast<int>(m_spreadAngle / m_lightSubdivisionSize);
    float startAngle = m_directionAngle - m_spreadAngle / 2.0f;

    for(int currentSubDivision = 0; currentSubDivision <= numSubdivisions; currentSubDivision++)
    {
        float angle = startAngle + currentSubDivision * m_lightSubdivisionSize;
        glVertex2f(getRadius() * cosf(angle) + getCenter().x, getRadius() * sinf(angle) + getCenter().y);
    }

    glEnd();

    //reset color
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Light_Point::renderLightSoftPortion()
{
    // If light goes all the way around do not render fins
    if(m_spreadAngle >= pifTimes2 || m_softSpreadAngle == 0.0f)
        return;

    // Create to shadow fins to mask off a portion of the light
    ShadowFin fin1;

    float umbraAngle1 = m_directionAngle - m_spreadAngle / 2.0f;
    float penumbraAngle1 = umbraAngle1 + m_softSpreadAngle;
    fin1.m_penumbra = sf::Vector2f(getRadius() * cosf(penumbraAngle1), getRadius() * sinf(penumbraAngle1));
    fin1.m_umbra = sf::Vector2f(getRadius() * cosf(umbraAngle1), getRadius() * sinf(umbraAngle1));
    fin1.m_rootPos = getCenter();

    fin1.render(1.0f);

    ShadowFin fin2;

    float umbraAngle2 = m_directionAngle + m_spreadAngle / 2.0f;
    float penumbraAngle2 = umbraAngle2 - m_softSpreadAngle;
    fin2.m_penumbra = sf::Vector2f(getRadius() * cosf(penumbraAngle2),getRadius() * sinf(penumbraAngle2));
    fin2.m_umbra = sf::Vector2f(getRadius() * cosf(umbraAngle2), getRadius()* sinf(umbraAngle2));
    fin2.m_rootPos = getCenter();

    fin2.render(1.0f);
}

void Light_Point::calculateAABB()
{
    //fuck it time to look up the answer on a math forum
    //http://mathforum.org/kb/thread.jspa?forumID=128&threadID=970755&messageID=3226275
    //^ this guy
    //nvm fuck that guy

    /*http://stackoverflow.com/questions/1336663/2d-bounding-box-of-a-sector
    let's do this:
    Generate the following points:
        The circle's center - got it already
        The positions of the two radii on the circle
        The points on the circle for every angle between the two that divides by 90o (maximum of 4 points)
    Calculate the min and max x and y from the above points. This is your bounding box
    */

    if(m_directionAngle < 0.f)
            m_directionAngle = pifTimes2 + m_directionAngle;

    if(m_spreadAngle < 0.f)
        m_spreadAngle = 0.f;

    if(m_spreadAngle >= pifTimes2 ||  m_spreadAngle == 0.0f) // 1 real value, 1 flag
    {
        sf::Vector2f diff(getRadius(), getRadius());
        m_aabb.setLowerBound(getCenter() - diff);
        m_aabb.setUpperBound(getCenter() + diff);

        aabbCenterDiff = sf::Vector2f(0.0f, 0.0f);
    }
    else
    {
        auto eqDirectionAngle = remainder(m_directionAngle, pifTimes2);

        //pointers to the needed points
        std::vector<std::unique_ptr<sf::Vector2f>> points;

        //find equivalent angles
        auto halfSpread = m_spreadAngle / 2.0f;
        auto angle1 = eqDirectionAngle - halfSpread;
        auto angle2 = eqDirectionAngle  + halfSpread;

        //position of two radii
        sf::Vector2f frustVec1(getCenter().x + getRadius()*cosf(angle1), getCenter().y + getRadius()*sinf(angle1));
        sf::Vector2f frustVec2(getCenter().x + getRadius()*cosf(angle2), getCenter().y + getRadius()*sinf(angle2));

        points.push_back(std::unique_ptr<sf::Vector2f>(new auto(frustVec1)));
        points.push_back(std::unique_ptr<sf::Vector2f>(new auto(frustVec2)));

        //checks to see if axis angles are overlapped by the sector of the circle
        //since the spread angle is split in half, there are negative angles thus must search negative angle axises
        for(int i = -2; i < 3; i++)
        {
            auto axisAngle = float(i) * pif/2.f; //pif is radians

            if(axisAngle >= angle1 && axisAngle <= angle2 )
            {
                points.push_back(std::unique_ptr<sf::Vector2f>(new sf::Vector2f(getCenter().x + getRadius()*cosf(axisAngle), getCenter().y + getRadius()*sinf(axisAngle))));
            }

            sf::Vector2f LowerBound(getCenter());
            sf::Vector2f UpperBound(getCenter());

            for(const auto& i : points)
            {
                // Expand lower bound
                if(i->x < LowerBound.x)
                    LowerBound.x = i->x;

                if(i->y < LowerBound.y)
                    LowerBound.y = i->y;

                // Expand upper bound
                if(i->x > UpperBound.x)
                    UpperBound.x = i->x;

                if(i->y > UpperBound.y)
                    UpperBound.y = i->y;
            }

            m_aabb.setBounds(LowerBound, UpperBound);
        }

        aabbCenterDiff = m_aabb.getCenter() - getCenter();
    }
}

void Light_Point::setCenter(const sf::Vector2f& center)
{
    m_center = center;

    m_aabb.setCenter(m_center + aabbCenterDiff);

    treeUpdate();

    m_updateRequired = true;
}

void Light_Point::setDirectionAngle(float directionAngle)
{
    assert(alwaysUpdate());

    m_directionAngle = deg2Rads(directionAngle);

    calculateAABB();
    treeUpdate();
}

void Light_Point::incDirectionAngle(float increment)
{
    assert(alwaysUpdate());

    m_directionAngle += deg2Rads(increment);

    calculateAABB();
    treeUpdate();
}

float Light_Point::getDirectionAngle() const
{
    return rad2Degs(m_directionAngle);
}

void Light_Point::setSpreadAngle(float spreadAngle)
{
    assert(alwaysUpdate());

    m_spreadAngle = deg2Rads(spreadAngle);

    calculateAABB();
    treeUpdate();
}

void Light_Point::incSpreadAngle(float increment)
{
    assert(alwaysUpdate());

    m_spreadAngle += deg2Rads(increment);

    calculateAABB();
    treeUpdate();
}

float Light_Point::getSpreadAngle() const
{
    return rad2Degs(m_spreadAngle);
}

void Light_Point::setSoftSpreadAngle(float softSpreadAngle)
{
    m_softSpreadAngle = deg2Rads(softSpreadAngle);
}

void Light_Point::incSoftSpreadAngle(float increment)
{
    assert(alwaysUpdate());

    m_softSpreadAngle += deg2Rads(increment);

    calculateAABB();
}

float Light_Point::getSoftSpreadAngle() const
{
    return rad2Degs(m_softSpreadAngle);
}

void Light_Point::setLightSubdivisionSize(float lightSubdivisionSize)
{
    m_lightSubdivisionSize = deg2Rads(lightSubdivisionSize);
}

void Light_Point::incLightSubdivisionSize(float increment)
{
    assert(alwaysUpdate());

    m_lightSubdivisionSize += deg2Rads(increment);

    calculateAABB();
    treeUpdate();
}

float Light_Point::getLightSubdivisionSize() const
{
    return rad2Degs(m_lightSubdivisionSize);
}

}



