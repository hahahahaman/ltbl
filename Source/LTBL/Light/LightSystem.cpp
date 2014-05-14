

#include <LTBL/QuadTree/QuadTreeOccupant.h>
#include <LTBL/Light/LightSystem.h>
#include <LTBL/Light/ShadowFin.h>
#include <LTBL/Utils.h>

#include <cassert>

namespace ltbl
{
LightSystem::LightSystem()
    : m_ambientColor(55, 55, 55), m_checkForHullIntersect(true),
      m_prebuildTimer(0), m_useBloom(true), m_maxFins(1), m_obliqueShadows(true)
{
}

//WTF? apparently in debug mode lightsystem uses the empty^ constructor in combination with the one below but in release it uses the one below only
//THIS DIDNT HAPPEN IN WINDOWS
LightSystem::LightSystem(const AABB &region, sf::RenderWindow* pRenderWindow, const std::string &finImagePath, const std::string &lightAttenuationShaderPath)
    : m_ambientColor(55, 55, 55), m_checkForHullIntersect(true),
      m_prebuildTimer(0), m_pWin(pRenderWindow), m_useBloom(true), m_maxFins(1), m_obliqueShadows(true)
{
    // Load the soft shadows texture
    if(!m_softShadowTexture.loadFromFile(finImagePath))
        abort(); // Could not find the texture, abort

    if(!m_lightAttenuationShader.loadFromFile(lightAttenuationShaderPath, sf::Shader::Fragment))
        abort();

    setup(region);
}

LightSystem::~LightSystem()
{
    // Destroy resources
    clearLights();
    clearConvexHulls();
    clearEmissiveLights();
}

void LightSystem::create(const AABB &region, sf::RenderWindow* pRenderWindow, const std::string &finImagePath, const std::string &lightAttenuationShaderPath)
{
    m_pWin = pRenderWindow;

    // Load the soft shadows texture
    if(!m_softShadowTexture.loadFromFile(finImagePath))
        abort(); // Could not find the texture, abort

    if(!m_lightAttenuationShader.loadFromFile(lightAttenuationShaderPath, sf::Shader::Fragment))
        abort();

    setup(region);
}

void LightSystem::setView(const sf::View &view)
{
    sf::Vector2f viewSize(view.getSize());
    m_viewAABB.setDims(sf::Vector2f(viewSize.x, viewSize.y));
    sf::Vector2f viewCenter(view.getCenter());

    // Flipped
    m_viewAABB.setCenter(sf::Vector2f(viewCenter.x, viewSize.y - viewCenter.y));
}

void LightSystem::cameraSetup()
{
    glLoadIdentity();
    glTranslatef(-m_viewAABB.getLowerBound().x, -m_viewAABB.getLowerBound().y, 0.0f);
    //glTranslatef(0.0f, m_viewAABB.getUpperBound().y, 0.0f);
    //printf("%f, %f\n", -m_viewAABB.getLowerBound().x, -m_viewAABB.getLowerBound().y);
    //glScalef(1.0f, -1.0f, 1.0f);
}

void LightSystem::maskShadow(Light* light, ConvexHull* convexHull, bool minPoly, float depth)
{
    // ----------------------------- Determine the Shadow Boundaries -----------------------------

    sf::Vector2f lCenter(light->getCenter());
    float lRadius = light->getRadius();

    sf::Vector2f hCenter(convexHull->getWorldCenter());

    const int numVertices = convexHull->getPointCount();

    std::vector<bool> backFacing(numVertices);

    for(int i = 0; i < numVertices; i++)
    {
        sf::Vector2f firstVertex(convexHull->getWorldVertex(i));
        int secondIndex = (i + 1) % numVertices;
        sf::Vector2f secondVertex(convexHull->getWorldVertex(secondIndex));
        sf::Vector2f middle((firstVertex + secondVertex) / 2.0f);

        // Use normal to take light width into account, this eliminates popping
        sf::Vector2f lightNormal(-(lCenter.y - middle.y), lCenter.x - middle.x);

        sf::Vector2f centerToBoundry(middle - hCenter);

        if(dot2d(centerToBoundry, lightNormal) < 0)
            lightNormal *= -1.f;

        lightNormal = normalize2d<float>(lightNormal) * light->m_size;

        sf::Vector2f L((lCenter - lightNormal) - middle);

        if (dot2d(convexHull->m_normals[i], L) > 0)
            backFacing[i] = false;
        else
            backFacing[i] = true;
    }

    int firstBoundryIndex = 0;
    int secondBoundryIndex = 0;

    for(int currentEdge = 0; currentEdge < numVertices; currentEdge++)
    {
        int nextEdge = (currentEdge + 1) % numVertices;

        if (backFacing[currentEdge] && !backFacing[nextEdge])
            firstBoundryIndex = nextEdge;

        if (!backFacing[currentEdge] && backFacing[nextEdge])
            secondBoundryIndex = nextEdge;
    }

    // -------------------------------- Shadow Fins --------------------------------

    sf::Vector2f firstBoundryPoint(convexHull->getWorldVertex(firstBoundryIndex));

    sf::Vector2f lightNormal(-(lCenter.y - firstBoundryPoint.y), lCenter.x - firstBoundryPoint.x);

    sf::Vector2f centerToBoundry(firstBoundryPoint - hCenter);

    if(dot2d(centerToBoundry, lightNormal) < 0)
        lightNormal *= -1.f;

    lightNormal = normalize2d(lightNormal) * light->getSize();

    ShadowFin firstFin;

    firstFin.m_rootPos = firstBoundryPoint;
    firstFin.m_umbra = firstBoundryPoint - (lCenter + lightNormal);
    firstFin.m_umbra = normalize2d(firstFin.m_umbra) * lRadius;

    firstFin.m_penumbra = firstBoundryPoint - (lCenter - lightNormal);
    firstFin.m_penumbra = normalize2d(firstFin.m_penumbra) * lRadius;

    ShadowFin secondFin;

    sf::Vector2f secondBoundryPoint = convexHull->getWorldVertex(secondBoundryIndex);

    lightNormal.x = -(lCenter.y - secondBoundryPoint.y);
    lightNormal.y = lCenter.x - secondBoundryPoint.x;

    centerToBoundry = secondBoundryPoint - hCenter;

    if(dot2d(centerToBoundry, lightNormal) < 0)
        lightNormal *= -1.f;

    lightNormal = normalize2d(lightNormal) * light->getSize();

    secondFin.m_rootPos = secondBoundryPoint;
    secondFin.m_umbra = secondBoundryPoint - (lCenter + lightNormal);
    secondFin.m_umbra = normalize2d(secondFin.m_umbra) * lRadius;

    secondFin.m_penumbra = secondBoundryPoint - (lCenter - lightNormal);
    secondFin.m_penumbra = normalize2d(secondFin.m_penumbra) * lRadius;

    std::vector<ShadowFin> finsToRender_firstBoundary;
    std::vector<ShadowFin> finsToRender_secondBoundary;

    // Store generated fins to render later
    // First two, will always be there
    finsToRender_firstBoundary.push_back(firstFin);
    finsToRender_secondBoundary.push_back(secondFin);

    // Need to get address of fin in array instead of firstFin/secondFin since they are copies, and we want it to be modified directly
    // Can avoid by not creating firstFin and secondFin and instead using finsToRender[0] and finsToRender[1]
    // Also, move the boundary points for rendering depending on the number of hulls added
    sf::Vector2f mainUmbraRoot1;
    sf::Vector2f mainUmbraVec1;

    firstBoundryIndex = wrap(firstBoundryIndex + addExtraFins(*convexHull, finsToRender_firstBoundary, *light, firstBoundryIndex, false, mainUmbraRoot1, mainUmbraVec1), numVertices);

    sf::Vector2f mainUmbraRoot2;
    sf::Vector2f mainUmbraVec2;

    secondBoundryIndex = wrap(secondBoundryIndex - addExtraFins(*convexHull, finsToRender_secondBoundary, *light, secondBoundryIndex, true, mainUmbraRoot2, mainUmbraVec2), numVertices);

    // ----------------------------- Drawing the umbra -----------------------------

    glDisable(GL_TEXTURE_2D);

    //glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
    glBlendFunc(GL_DST_ALPHA, GL_ZERO);

    glColor4f(0.0f, 0.0f, 0.0f, 1.0f - convexHull->m_transparency);

    if(!convexHull->m_renderLightOverHull)
    {
        sf::Vector2f throughCenter(normalize2d(hCenter - lCenter) * lRadius);

        // 3 rays all the time, less polygons
        glBegin(GL_TRIANGLE_STRIP);

        glVertex3f(mainUmbraRoot1.x, mainUmbraRoot1.y, depth);
        glVertex3f(mainUmbraRoot1.x + mainUmbraVec1.x, mainUmbraRoot1.y + mainUmbraVec1.y, depth);
        glVertex3f(hCenter.x, hCenter.y, depth);
        glVertex3f(hCenter.x + throughCenter.x, hCenter.y + throughCenter.y, depth);
        glVertex3f(mainUmbraRoot2.x, mainUmbraRoot2.y, depth);
        glVertex3f(mainUmbraRoot2.x + mainUmbraVec2.x, mainUmbraRoot2.y + mainUmbraVec2.y, depth);

        glEnd();
    }
    else
    {
        glBegin(GL_TRIANGLE_STRIP);

        // Umbra and penumbra sides done separately, since they do not follow light rays
        glVertex3f(mainUmbraRoot1.x, mainUmbraRoot1.y, depth);
        glVertex3f(mainUmbraRoot1.x + mainUmbraVec1.x, mainUmbraRoot1.y + mainUmbraVec1.y, depth);

        int endV;

        if(firstBoundryIndex < secondBoundryIndex)
            endV = secondBoundryIndex - 1;
        else
            endV = secondBoundryIndex + numVertices - 1;

        // Mask off around the hull, requires more polygons
        for(int v = firstBoundryIndex + 1; v <= endV; v++)
        {
            // get actual vertex
            int vi = v % numVertices;

            sf::Vector2f startVert(convexHull->getWorldVertex(vi));
            sf::Vector2f endVert(normalize2d(startVert - light->m_center) * light->m_radius + startVert);

            // 2 points for ray in strip
            glVertex3f(startVert.x, startVert.y, depth);
            glVertex3f(endVert.x, endVert.y, depth);
        }

        glVertex3f(mainUmbraRoot2.x, mainUmbraRoot2.y, depth);
        glVertex3f(mainUmbraRoot2.x + mainUmbraVec2.x, mainUmbraRoot2.y + mainUmbraVec2.y, depth);

        glEnd();
    }

    // Render shadow fins
    glEnable(GL_TEXTURE_2D);

    sf::Texture::bind(&m_softShadowTexture);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for(unsigned int f = 0, numFins = finsToRender_firstBoundary.size(); f < numFins; f++)
        finsToRender_firstBoundary[f].render(convexHull->m_transparency);

    for(unsigned int f = 0, numFins = finsToRender_secondBoundary.size(); f < numFins; f++)
        finsToRender_secondBoundary[f].render(convexHull->m_transparency);
}

int LightSystem::addExtraFins(const ConvexHull &hull, std::vector<ShadowFin> &fins, const Light &light, int boundryIndex, bool wrapCW, sf::Vector2f &mainUmbraRoot, sf::Vector2f &mainUmbraVec)
{
    ShadowFin* pFin = &fins.back();

    sf::Vector2f hCenter(hull.getWorldCenter());

    int secondEdgeIndex;
    int numVertices = static_cast<signed>(hull.getPointCount());

    mainUmbraRoot = fins.back().m_rootPos;
    mainUmbraVec = fins.back().m_umbra;

    unsigned int i;

    for(i = 0; i < m_maxFins; i++)
    {
        if(wrapCW)
            secondEdgeIndex = wrap(boundryIndex - 1, numVertices);
        else
            secondEdgeIndex = wrap(boundryIndex + 1, numVertices);

        sf::Vector2f edgeVec(normalize2d(hull.getPoint(secondEdgeIndex) - hull.getPoint(boundryIndex)));

        sf::Vector2f penNorm(normalize2d(pFin->m_penumbra));

        float angle1 = acosf(dot2d(penNorm, edgeVec));
        float angle2 = acosf(dot2d(penNorm, normalize2d(pFin->m_umbra)));

        if(angle1 >= angle2)
            break; // No intersection, break

        // Change existing fin to attatch to side of hull
        pFin->m_umbra = edgeVec * light.m_radius;

        // calculate a lower fin instensity based on ratio of angles (0 if angles are same, so disappears then)
        pFin->m_umbraBrightness = 1.0f - angle1 / angle2;

        // Add the extra fin
        sf::Vector2f secondBoundryPoint(hull.getWorldVertex(secondEdgeIndex));

        sf::Vector2f lightNormal(-(light.m_center.y - secondBoundryPoint.y), light.m_center.x - secondBoundryPoint.x);

        sf::Vector2f centerToBoundry(secondBoundryPoint - hCenter);

        if(dot2d(centerToBoundry, lightNormal) < 0)
            lightNormal *= -1.f;

        lightNormal = normalize2d(lightNormal) * light.getSize();

        ShadowFin newFin;

        mainUmbraRoot = newFin.m_rootPos = secondBoundryPoint;
        newFin.m_umbra = secondBoundryPoint - (light.m_center + lightNormal);
        mainUmbraVec = newFin.m_umbra = normalize2d(newFin.m_umbra) * light.m_radius;
        newFin.m_penumbra = pFin->m_umbra;

        newFin.m_penumbraBrightness = pFin->m_umbraBrightness;

        fins.push_back(newFin);

        pFin = &fins.back();

        boundryIndex = secondEdgeIndex;
    }

    return i;
}

void LightSystem::setup(const AABB &region)
{
    // Create the quad trees
    m_lightTree.create(region);
    m_hullTree.create(region);
    m_emissiveTree.create(region);

    // Base RT size off of window resolution
    sf::Vector2u viewSizeui(m_pWin->getSize());

    m_compositionTexture.create(viewSizeui.x, viewSizeui.y, false);
    m_compositionTexture.setSmooth(true);

    m_compositionTexture.setActive();
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    m_bloomTexture.create(viewSizeui.x, viewSizeui.y, false);
    m_bloomTexture.setSmooth(true);

    m_bloomTexture.setActive();
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    m_lightTempTexture.create(viewSizeui.x, viewSizeui.y, false);
    m_lightTempTexture.setSmooth(true);

    m_lightTempTexture.setActive();
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    m_pWin->setActive();
}

void LightSystem::addLight(Light* newLight)
{
    newLight->m_pWin = m_pWin;
    newLight->m_pLightSystem = this;
    m_lights.insert(newLight);
    m_lightTree.add(newLight);
}

void LightSystem::addConvexHull(ConvexHull* newConvexHull)
{
    m_convexHulls.insert(newConvexHull);
    m_hullTree.add(newConvexHull);
}

void LightSystem::addEmissiveLight(EmissiveLight* newEmissiveLight)
{
    m_emissiveLights.insert(newEmissiveLight);
    m_emissiveTree.add(newEmissiveLight);
}

void LightSystem::removeLight(Light* pLight)
{
    std::unordered_set<Light*>::iterator it = m_lights.find(pLight);

/*
    assert(it != m_lights.end());
    (*it)->removeFromTree();

    m_lights.erase(it);

    delete pLight;
    */

    if(it != m_lights.end())
    {
        (*it)->removeFromTree();

        m_lights.erase(it);

        delete pLight;
    }

}

void LightSystem::removeConvexHull(ConvexHull* pHull)
{
    std::unordered_set<ConvexHull*>::iterator it = m_convexHulls.find(pHull);

    if(it != m_convexHulls.end())
    {
        (*it)->removeFromTree();

        m_convexHulls.erase(it);

        delete pHull;
    }
}

void LightSystem::removeEmissiveLight(EmissiveLight* pEmissiveLight)
{
    std::unordered_set<EmissiveLight*>::iterator it = m_emissiveLights.find(pEmissiveLight);

    if(it != m_emissiveLights.end())
    {
        (*it)->removeFromTree();

        m_emissiveLights.erase(it);

        delete pEmissiveLight;
    }
}

void LightSystem::clearLights()
{
    // Delete contents
    for(std::unordered_set<Light*>::iterator it = m_lights.begin(); it != m_lights.end(); it++)
        delete *it;

    m_lights.clear();

    if(m_lightTree.created())
    {
        m_lightTree.clearTree();
        m_lightTree.create(AABB(sf::Vector2f(-50.0f, -50.0f), sf::Vector2f(-50.0f, -50.0f)));
    }
}

void LightSystem::clearConvexHulls()
{
    // Delete contents
    for(std::unordered_set<ConvexHull*>::iterator it = m_convexHulls.begin(); it != m_convexHulls.end(); it++)
        delete *it;

    m_convexHulls.clear();

    if(!m_hullTree.created())
    {
        m_hullTree.clearTree();
        m_hullTree.create(AABB(sf::Vector2f(-50.0f, -50.0f), sf::Vector2f(-50.0f, -50.0f)));
    }
}

void LightSystem::clearEmissiveLights()
{
    // Delete contents
    for(std::unordered_set<EmissiveLight*>::iterator it = m_emissiveLights.begin(); it != m_emissiveLights.end(); it++)
        delete *it;

    m_emissiveLights.clear();

    if(m_emissiveTree.created())
    {
        m_emissiveTree.clearTree();
        m_emissiveTree.create(AABB(sf::Vector2f(-50.0f, -50.0f), sf::Vector2f(-50.0f, -50.0f)));
    }
}

void LightSystem::switchLightTemp()
{
    if(m_currentRenderTexture != cur_lightTemp)
    {
        m_lightTempTexture.setActive();

        //if(currentRenderTexture == cur_lightStatic)
        switchWindowProjection();

        m_currentRenderTexture = cur_lightTemp;
    }
}

void LightSystem::switchComposition()
{
    if(m_currentRenderTexture != cur_main)
    {
        m_compositionTexture.setActive();

        if(m_currentRenderTexture == cur_lightStatic)
            switchWindowProjection();

        m_currentRenderTexture = cur_main;
    }
}

void LightSystem::switchBloom()
{
    if(m_currentRenderTexture != cur_bloom)
    {
        m_bloomTexture.setActive();

        if(m_currentRenderTexture == cur_lightStatic)
            switchWindowProjection();

        m_currentRenderTexture = cur_bloom;
    }
}

void LightSystem::switchWindow()
{
    m_pWin->resetGLStates();
}

void LightSystem::switchWindowProjection()
{
    sf::Vector2f viewSize(m_viewAABB.getDims());
    sf::Vector2u viewSizeui(static_cast<unsigned int>(viewSize.x), static_cast<unsigned int>(viewSize.y));

    glViewport(0, 0, viewSizeui.x, viewSizeui.y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Upside-down, because SFML is pro like that
    glOrtho(0, viewSize.x, 0, viewSize.y, -100.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void LightSystem::clearLightTexture(sf::RenderTexture &renTex)
{
    glLoadIdentity();

    renTex.clear(sf::Color::Transparent);

    // Clear with quad, since glClear is not working for some reason... if results in very ugly artifacts. MUST clear with full color, with alpha!
    glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

    sf::Vector2u size(renTex.getSize());
    float width = static_cast<float>(size.x);
    float height = static_cast<float>(size.y);

    glBlendFunc(GL_ONE, GL_ZERO);

    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(width, 0.0f);
    glVertex2f(width, height);
    glVertex2f(0.0f, height);
    glEnd();
}

void LightSystem::renderLights()
{
    // So will switch to main render textures from SFML projection
    m_currentRenderTexture = cur_lightStatic;

    sf::Vector2f viewCenter(m_viewAABB.getCenter());
    sf::Vector2f viewSize(m_viewAABB.getDims());

    glDisable(GL_TEXTURE_2D);

    if(m_useBloom)
    {
        // Clear the bloom texture
        switchBloom();
        glLoadIdentity();

        m_bloomTexture.clear(sf::Color::Transparent);

        glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

        glBlendFunc(GL_ONE, GL_ZERO);

        // Clear with quad, since glClear is not working for some reason... if results in very ugly artifacts
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(viewSize.x, 0.0f);
        glVertex2f(viewSize.x, viewSize.y);
        glVertex2f(0.0f, viewSize.y);
        glEnd();

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    switchComposition();
    glLoadIdentity();

    m_compositionTexture.clear(m_ambientColor);

    glColor4b(m_ambientColor.r, m_ambientColor.g, m_ambientColor.b, m_ambientColor.a);

    glBlendFunc(GL_ONE, GL_ZERO);

    // Clear with quad, since glClear is not working for some reason... if results in very ugly artifacts
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(viewSize.x, 0.0f);
    glVertex2f(viewSize.x, viewSize.y);
    glVertex2f(0.0f, viewSize.y);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // get visible lights
    std::vector<qdt::QuadTreeOccupant*> visibleLights;
    m_lightTree.query_Region(m_viewAABB, visibleLights);

    // Add lights from pre build list if there are any
    if(!m_lightsToPreBuild.empty())
    {
        if(m_prebuildTimer < 2)
        {
            m_prebuildTimer++;

            const unsigned int numLightsToPreBuild = m_lightsToPreBuild.size();

            for(unsigned int i = 0; i < numLightsToPreBuild; i++)
            {
                m_lightsToPreBuild[i]->m_updateRequired = true;
                visibleLights.push_back(m_lightsToPreBuild[i]);
            }
        }
        else
            m_lightsToPreBuild.clear();
    }

    const unsigned int numVisibleLights = visibleLights.size();

    for(unsigned int l = 0; l < numVisibleLights; l++)
    {
        Light* pLight = static_cast<Light*>(visibleLights[l]);

        // Skip invisible lights
        if(pLight->m_intensity == 0.0f)
            continue;

        bool updateRequired = false;

        if(pLight->alwaysUpdate())
            updateRequired = true;
        else if(pLight->m_updateRequired)
            updateRequired = true;

        // get hulls that the light affects
        std::vector<qdt::QuadTreeOccupant*> regionHulls;
        m_hullTree.query_Region(pLight->getAABB(), regionHulls);

        const unsigned int numHulls = regionHulls.size();

        if(!updateRequired)
        {
            // See if any of the hulls need updating
            for(unsigned int h = 0; h < numHulls; h++)
            {
                ConvexHull* pHull = static_cast<ConvexHull*>(regionHulls[h]);

                if(pHull->m_updateRequired)
                {
                    pHull->m_updateRequired = false;
                    updateRequired = true;
                    break;
                }
            }
        }

        if(updateRequired)
        {
            sf::Vector2f staticTextureOffset;

            // Activate the intermediate render Texture
            if(pLight->alwaysUpdate())
            {
                switchLightTemp();

                clearLightTexture(m_lightTempTexture);

                cameraSetup();

                // Reset color
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                pLight->switchStaticTexture();
                m_currentRenderTexture = cur_lightStatic;

                clearLightTexture(*pLight->m_pStaticTexture);

                // Reset color
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                staticTextureOffset = pLight->m_aabb.getDims() / 2.0f;

                glTranslatef(-pLight->m_aabb.getLowerBound().x, -pLight->m_aabb.getLowerBound().y, 0.0f);
            }

            if(pLight->m_shaderAttenuation)
            {
                sf::Shader::bind(&m_lightAttenuationShader);

                if(pLight->alwaysUpdate())
                    m_lightAttenuationShader.setParameter("lightPos", pLight->getCenter().x - m_viewAABB.getLowerBound().x, pLight->getCenter().y - m_viewAABB.getLowerBound().y);
                else
                    m_lightAttenuationShader.setParameter("lightPos", pLight->getCenter().x - pLight->m_aabb.getLowerBound().x, pLight->getCenter().y - pLight->m_aabb.getLowerBound().y);

                //rgb
                m_lightAttenuationShader.setParameter("lightColor", pLight->getColor().x, pLight->getColor().y, pLight->getColor().z);

                m_lightAttenuationShader.setParameter("radius", pLight->getRadius());
                m_lightAttenuationShader.setParameter("bleed", pLight->getBleed());
                m_lightAttenuationShader.setParameter("linearizeFactor", pLight->getLinearizeFactor());

                // Render the current light
                pLight->renderLightSolidPortion();

                sf::Shader::bind(NULL); //glUseProgram(0);  ?
                // this was unbind before but that has been removed
            }
            else
                // Render the current light
                pLight->renderLightSolidPortion();

            // Mask off lights
            if(m_checkForHullIntersect)
                for(unsigned int h = 0; h < numHulls; h++)
                {
                    ConvexHull* pHull = static_cast<ConvexHull*>(regionHulls[h]);

                    sf::Vector2f hullToLight(pLight->m_center - pHull->getWorldCenter());
                    hullToLight = normalize2d(hullToLight) * pLight->getSize();

                    if(!pHull->pointInsideHull(pLight->getCenter() - hullToLight))
                        maskShadow(pLight, pHull, !pHull->m_renderLightOverHull, 2.0f);
                }
            else
                for(unsigned int h = 0; h < numHulls; h++)
                {
                    ConvexHull* pHull = static_cast<ConvexHull*>(regionHulls[h]);

                    maskShadow(pLight, pHull, !pHull->m_renderLightOverHull, 2.0f);
                }

            // Render the hulls only for the hulls that had
            // their shadows rendered earlier (not out of bounds)
            for(unsigned int h = 0; h < numHulls; h++)
            {
                 //static_cast<ConvexHull*>(regionHulls[h])->renderHull(2.0f)
                 if(!static_cast<ConvexHull*>(regionHulls[h])->m_renderLightOverHull)
                 {
                     m_pWin->draw(*static_cast<ConvexHull*>(regionHulls[h]));
                 }
            }

            // Soft light angle fins (additional masking)
            pLight->renderLightSoftPortion();

            // Color reset
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            // Now render that intermediate render Texture to the main render Texture
            if(pLight->alwaysUpdate())
            {
                m_lightTempTexture.display();

                switchComposition();
                glLoadIdentity();

                sf::Texture::bind(&m_lightTempTexture.getTexture());

                glBlendFunc(GL_ONE, GL_ONE);

                // Texture is upside-down for some reason, so draw flipped
                glBegin(GL_QUADS);
                glTexCoord2i(0, 1);
                glVertex2f(0.0f, 0.0f);
                glTexCoord2i(1, 1);
                glVertex2f(viewSize.x, 0.0f);
                glTexCoord2i(1, 0);
                glVertex2f(viewSize.x, viewSize.y);
                glTexCoord2i(0, 0);
                glVertex2f(0.0f, viewSize.y);
                glEnd();

                // Bloom render
                if(m_useBloom && pLight->getIntensity() > 1.0f)
                {
                    switchBloom();
                    glLoadIdentity();

                    sf::Texture::bind(&m_lightTempTexture.getTexture());

                    glBlendFunc(GL_ONE, GL_ONE);

                    // Bloom amount
                    glColor4f(1.0f, 1.0f, 1.0f, pLight->m_intensity - 1.0f);

                    // Texture is upside-down for some reason, so draw flipped
                    glBegin(GL_QUADS);
                    glTexCoord2i(0, 1);
                    glVertex2f(0.0f, 0.0f);
                    glTexCoord2i(1, 1);
                    glVertex2f(viewSize.x, 0.0f);
                    glTexCoord2i(1, 0);
                    glVertex2f(viewSize.x, viewSize.y);
                    glTexCoord2i(0, 0);
                    glVertex2f(0.0f, viewSize.y);
                    glEnd();

                    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                }
            }
            else
            {
                pLight->m_pStaticTexture->display();

                switchComposition();
                cameraSetup();

                sf::Texture::bind(&pLight->m_pStaticTexture->getTexture());

                glTranslatef(pLight->m_center.x - staticTextureOffset.x, pLight->m_center.y - staticTextureOffset.y, 0.0f);

                glBlendFunc(GL_ONE, GL_ONE);

                sf::Vector2u lightStaticTextureSize(pLight->m_pStaticTexture->getSize());
                const float lightStaticTextureWidth = static_cast<float>(lightStaticTextureSize.x);
                const float lightStaticTextureHeight = static_cast<float>(lightStaticTextureSize.y);

                glBegin(GL_QUADS);
                glTexCoord2i(0, 0);
                glVertex2f(0.0f, 0.0f);
                glTexCoord2i(1, 0);
                glVertex2f(lightStaticTextureWidth, 0.0f);
                glTexCoord2i(1, 1);
                glVertex2f(lightStaticTextureWidth, lightStaticTextureHeight);
                glTexCoord2i(0, 1);
                glVertex2f(0.0f, lightStaticTextureHeight);
                glEnd();

                // Bloom render
                if(m_useBloom && pLight->getIntensity() > 1.0f)
                {
                    switchBloom();
                    cameraSetup();

                    glTranslatef(pLight->m_center.x - staticTextureOffset.x, pLight->m_center.y - staticTextureOffset.y, 0.0f);

                    sf::Texture::bind(&pLight->m_pStaticTexture->getTexture());

                    glBlendFunc(GL_ONE, GL_ONE);

                    // Bloom amount
                    glColor4f(1.0f, 1.0f, 1.0f, pLight->m_intensity - 1.0f);

                    glBegin(GL_QUADS);
                    glTexCoord2i(0, 0);
                    glVertex2f(0.0f, 0.0f);
                    glTexCoord2i(1, 0);
                    glVertex2f(lightStaticTextureWidth, 0.0f);
                    glTexCoord2i(1, 1);
                    glVertex2f(lightStaticTextureWidth, lightStaticTextureHeight);
                    glTexCoord2i(0, 1);
                    glVertex2f(0.0f, lightStaticTextureHeight);
                    glEnd();

                    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                }
            }

            pLight->m_updateRequired = false;
        }
        else
        {
            switchComposition();

            // Render existing texture
            sf::Texture::bind(&pLight->m_pStaticTexture->getTexture());

            sf::Vector2f staticTextureOffset(pLight->getCenter() - pLight->m_aabb.getLowerBound());

            cameraSetup();
            glTranslatef(pLight->getCenter().x - staticTextureOffset.x, pLight->getCenter().y - staticTextureOffset.y, 0.0f);

            glBlendFunc(GL_ONE, GL_ONE);

            sf::Vector2u lightStaticTextureSize(pLight->m_pStaticTexture->getSize());
            const float lightStaticTextureWidth = static_cast<float>(lightStaticTextureSize.x);
            const float lightStaticTextureHeight = static_cast<float>(lightStaticTextureSize.y);

            glBegin(GL_QUADS);
            glTexCoord2i(0, 0);
            glVertex2f(0.0f, 0.0f);
            glTexCoord2i(1, 0);
            glVertex2f(lightStaticTextureWidth, 0.0f);
            glTexCoord2i(1, 1);
            glVertex2f(lightStaticTextureWidth, lightStaticTextureHeight);
            glTexCoord2i(0, 1);
            glVertex2f(0.0f, lightStaticTextureHeight);
            glEnd();

            // Bloom render
            if(m_useBloom && pLight->getIntensity() > 1.0f)
            {
                switchBloom();
                cameraSetup();

                glTranslatef(pLight->getCenter().x - staticTextureOffset.x, pLight->getCenter().y - staticTextureOffset.y, 0.0f);

                sf::Texture::bind(&pLight->m_pStaticTexture->getTexture());

                glBlendFunc(GL_ONE, GL_ONE);

                // Bloom amount
                glColor4f(1.0f, 1.0f, 1.0f, pLight->getIntensity() - 1.0f);

                glBegin(GL_QUADS);
                glTexCoord2i(0, 0);
                glVertex2f(0.0f, 0.0f);
                glTexCoord2i(1, 0);
                glVertex2f(lightStaticTextureWidth, 0.0f);
                glTexCoord2i(1, 1);
                glVertex2f(lightStaticTextureWidth, lightStaticTextureHeight);
                glTexCoord2i(0, 1);
                glVertex2f(0.0f, lightStaticTextureHeight);
                glEnd();

                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        regionHulls.clear();
    }

    // Emissive lights
    std::vector<qdt::QuadTreeOccupant*> visibleEmissiveLights;
    m_emissiveTree.query_Region(m_viewAABB, visibleEmissiveLights);

    const unsigned int numEmissiveLights = visibleEmissiveLights.size();

    for(unsigned int i = 0; i < numEmissiveLights; i++)
    {
        EmissiveLight* pEmissive = static_cast<EmissiveLight*>(visibleEmissiveLights[i]);

        if(m_useBloom && pEmissive->getIntensity() > 1.0f)
        {
            switchBloom();
            cameraSetup();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            pEmissive->render();
        }
        else
        {
            switchComposition();
            cameraSetup();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            pEmissive->render();
        }
    }

    m_bloomTexture.display();

    m_compositionTexture.display();

    switchWindow();
}

void LightSystem::buildLight(Light* pLight)
{
    m_lightsToPreBuild.push_back(pLight);
}

void LightSystem::renderLightTexture()
{
    //TODO: fix this sometime later, seems like a hack
    //y axis stuff is flipped so rotation is as well
    glTranslatef(0.0f, m_pWin->getView().getSize().y, 0.0f);
    glScalef(1.0f, -1.0f, 1.0f);

    sf::Vector2f viewSize(m_viewAABB.getDims());

    // Translate by negative camera coordinates. glLoadIdentity will not work, probably
    // because SFML stores view transformations in the projection matrix
    glTranslatef(m_viewAABB.getLowerBound().x, -m_viewAABB.getLowerBound().y, 0.0f);

    sf::Texture::bind(&m_compositionTexture.getTexture());

    // if no glBlendFunc there is reverse shadows
    // set up color function to multiply the existing color with the render texture color
    // Seperate allows you to set color and alpha functions seperately
    //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //when combined with oblique shadows it makes light look like it is behind background, good fortransparent shadows
    //glBlendFunc(GL_ALPHA, GL_CONSTANT_COLOR); //background is blended
    //glBlendFunc(GL_DST_ALPHA, GL_ONE); //umm some strange effect in combination with gl_one, gl_one, good for oblique shadows

    //condition ? if_true : if_false;
    (m_obliqueShadows) ? glBlendFunc(GL_DST_ALPHA, GL_ONE) : glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

    glBegin(GL_QUADS);
    glTexCoord2i(0, 0);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2i(1, 0);
    glVertex2f(viewSize.x, 0.0f);
    glTexCoord2i(1, 1);
    glVertex2f(viewSize.x, viewSize.y);
    glTexCoord2i(0, 1);
    glVertex2f(0.0f, viewSize.y);
    glEnd();

    if(m_useBloom)
    {
        sf::Texture::bind(&m_bloomTexture.getTexture());

        //glBlendFunc(GL_ONE, GL_ONE); //for transparent shadows
        //glBlendFunc(GL_ONE, GL_SRC_COLOR); //dull oblique shadows
        //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //preferred oblique shadows
        (m_obliqueShadows) ? glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR) : glBlendFunc(GL_ONE, GL_ONE);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 0);
        glVertex2f(0.0f, 0.0f);
        glTexCoord2i(1, 0);
        glVertex2f(viewSize.x, 0.0f);
        glTexCoord2i(1, 1);
        glVertex2f(viewSize.x, viewSize.y);
        glTexCoord2i(0, 1);
        glVertex2f(0.0f, viewSize.y);
        glEnd();
    }

    // Reset blend function
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWin->resetGLStates();
}

qdt::QuadTree* LightSystem::getLightTree()
{
    return &m_lightTree;
}

qdt::QuadTree* LightSystem::getEmissiveTree()
{
    return &m_emissiveTree;
}

qdt::QuadTree* LightSystem::getHullTree()
{
    return &m_hullTree;
}

void LightSystem::debugRender()
{
    // set to a more useful-for-OpenGL projection
    //sf::Vector2f viewSize(m_viewAABB.getDims());

    glLoadIdentity();

    cameraSetup();

    //idk why but debug aabb render like sfml and everything else doesn't, moved this to renderLightTexture
    //glTranslatef(0.0f, m_viewAABB.getUpperBound().y, 0.0f);
    //glScalef(1.0f, -1.0f, 1.0f);

    // Render all trees
    m_lightTree.debugRender();
    m_emissiveTree.debugRender();
    m_hullTree.debugRender();

    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    // Reset to SFML's projection
    m_pWin->resetGLStates();
}
}
