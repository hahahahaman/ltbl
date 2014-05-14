

#ifndef LTBL_LIGHT_POINT_H
#define LTBL_LIGHT_POINT_H

#include <LTBL/Light/Light.h>

#include <memory>


/**
*Gives off light creating shadows
*/

namespace ltbl
{
class Light_Point : public Light
{
    private:
        //internal stuff is in radians, but outside it's all degree
        float m_directionAngle;
        float m_spreadAngle;
        float m_softSpreadAngle;

        float m_lightSubdivisionSize;

        //fix the diff after calculateAABB
        sf::Vector2f aabbCenterDiff;

    public:
        Light_Point();
        ~Light_Point();

        // Inherited
        virtual void renderLightSolidPortion() override;
        virtual void renderLightSoftPortion() override;
        virtual void calculateAABB() override;

        virtual void setCenter(const sf::Vector2f& center) override;

        //setters and getters and incrementers
        void setDirectionAngle(float directionAngle);
        void incDirectionAngle(float increment);
        float getDirectionAngle() const;

        void setSpreadAngle(float spreadAngle);
        void incSpreadAngle(float increment);
        float getSpreadAngle() const;

        void setSoftSpreadAngle(float softSpreadAngle);
        void incSoftSpreadAngle(float increment);
        float getSoftSpreadAngle() const;

        void setLightSubdivisionSize(float lightSubdivisionSize);
        void incLightSubdivisionSize(float increment);
        float getLightSubdivisionSize() const;
};
}

#endif
