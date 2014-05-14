#ifndef QDT_QUADTREE_H
#define QDT_QUADTREE_H

#include <LTBL/QuadTree/QuadTreeNode.h>
#include <LTBL/QuadTree/QuadTreeOccupant.h>

#include <unordered_set>
#include <memory>
#include <list>

namespace qdt
{
	// Base class for dynamic and static QuadTree types
	class QuadTree
	{
	protected:
		std::unordered_set<QuadTreeOccupant*> m_outsideRoot;

		std::unique_ptr<QuadTreeNode> m_pRootNode;

		// Called whenever something is removed, an action can be defined by derived classes
		// Defaults to doing nothing
		virtual void onRemoval();

		void setQuadTree(QuadTreeOccupant* pOc);

	public:
		virtual void add(QuadTreeOccupant* pOc) = 0;

		void query_Region(const AABB &region, std::vector<QuadTreeOccupant*> &result);

		//gives a max dimension for the object
		void query_Region(const AABB &region, const sf::Vector2f &maxDims, std::vector<QuadTreeOccupant*> & result);

		void debugRender();

		friend class QuadTreeNode;
		friend class QuadTreeOccupant;
	};
}

#endif
