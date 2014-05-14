#ifndef LTBL_LIGHTSYSTEM_H
#define LTBL_LIGHTSYSTEM_H

#include <SFML/OpenGL.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <LTBL/QuadTree/StaticQuadTree.h>
#include <LTBL/Light/Light.h>
#include <LTBL/Light/Light_Point.h>
#include <LTBL/Light/EmissiveLight.h>
#include <LTBL/Light/ConvexHull.h>
#include <LTBL/Light/ShadowFin.h>
#include <LTBL/Constructs.h>
#include <LTBL/Utils.h>

#include <unordered_set>
#include <vector>

/**
*System that controlls lights, emissive, lights and convex hulls, it has quad trees for all
*
*Used to access and render all of the objects above^
*/

namespace ltbl
{
	class LightSystem
	{
	private:
		sf::RenderWindow* m_pWin;

		std::unordered_set<Light*> m_lights;

		std::unordered_set<EmissiveLight*> m_emissiveLights;

		std::unordered_set<ConvexHull*> m_convexHulls;

		std::vector<Light*> m_lightsToPreBuild;

		qdt::StaticQuadTree m_lightTree;
		qdt::StaticQuadTree m_hullTree;
		qdt::StaticQuadTree m_emissiveTree;

		sf::RenderTexture m_compositionTexture;
		sf::RenderTexture m_lightTempTexture;
		sf::RenderTexture m_bloomTexture;

		sf::Shader m_lightAttenuationShader;

		sf::Texture m_softShadowTexture;

		int m_prebuildTimer;

		void maskShadow(Light* light, ConvexHull* convexHull, bool minPoly, float depth);

		// Returns number of fins added
		int addExtraFins(const ConvexHull &hull, std::vector<ShadowFin> &fins, const Light &light, int boundryIndex, bool wrapCW, sf::Vector2f &mainUmbraRoot, sf::Vector2f &mainUmbraVec);

		void cameraSetup();
		void setup(const AABB &region);

		// Switching between render textures
		void switchLightTemp();
		void switchComposition();
		void switchBloom();
		void switchWindow();

		void switchWindowProjection();

		enum CurrentRenderTexture
		{
			cur_lightTemp, cur_shadow, cur_main, cur_bloom, cur_window, cur_lightStatic
		} m_currentRenderTexture;

		void clearLightTexture(sf::RenderTexture &renTex);

	public:
		AABB m_viewAABB;

		sf::Color m_ambientColor;

		bool m_checkForHullIntersect;
		bool m_useBloom;
        bool m_obliqueShadows;

		unsigned int m_maxFins;

		LightSystem();
		LightSystem(const AABB &region, sf::RenderWindow* pRenderWindow, const std::string &finImagePath, const std::string &lightAttenuationShaderPath);
		~LightSystem();

		void create(const AABB &region, sf::RenderWindow* pRenderWindow, const std::string &finImagePath, const std::string &lightAttenuationShaderPath);

		void setView(const sf::View &view);

		// All objects are controlled through pointer
		void addLight(Light* newLight);
		void addConvexHull(ConvexHull* newConvexHull);
		void addEmissiveLight(EmissiveLight* newEmissiveLight);

		void removeLight(Light* pLight);
		void removeConvexHull(ConvexHull* pHull);
		void removeEmissiveLight(EmissiveLight* pEmissiveLight);

		// Pre-builds the light
		void buildLight(Light* pLight);

		void clearLights();
		void clearConvexHulls();
		void clearEmissiveLights();

		// Renders lights to the light texture (big black sheet covering the area of the light system, shadow effect)
		void renderLights();

		void renderLightTexture();

		qdt::QuadTree* getLightTree();
		qdt::QuadTree* getEmissiveTree();
		qdt::QuadTree* getHullTree();

		void debugRender();
	};
}

#endif
