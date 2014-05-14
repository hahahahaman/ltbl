#include <LTBL/Utils.h>

#include <cstdlib>

namespace ltbl
{

float rad2Degs(float angleRads)
{
    return angleRads * (180.f / pif);
};

float deg2Rads(float angleDegs)
{
    return angleDegs * (pif / 180.f);
};

float getFloatVal(std::string strConvert)
{
    return static_cast<float>(atof(strConvert.c_str()));
};

};
