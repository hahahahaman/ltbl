#ifndef QDT_QUADTREEOCCUPANT_H
#define QDT_QUADTREEOCCUPANT_H

#include <LTBL/QuadTree/QuadTreeNode.h>
#include <LTBL/Constructs/AABB.h>

namespace qdt
{
	class QuadTreeOccupant
	{
	private:
		class QuadTreeNode* m_pQuadTreeNode;
		class QuadTree* m_pQuadTree;

	protected:
		AABB m_aabb;

	public:
		QuadTreeOccupant();

		void treeUpdate();
		void removeFromTree();

        const AABB& getAABB();

		friend class QuadTreeNode;
		friend class QuadTree;
	};
}

#endif
