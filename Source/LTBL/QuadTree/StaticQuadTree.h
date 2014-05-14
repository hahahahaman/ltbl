#ifndef QDT_STATICQUADTREE_H
#define QDT_STATICQUADTREE_H

#include <LTBL/QuadTree/QuadTree.h>

namespace qdt
{
	class StaticQuadTree :
		public QuadTree
	{
	private:
		bool m_created;

	public:
		StaticQuadTree();
		StaticQuadTree(const AABB &rootRegion);

		void create(const AABB &rootRegion);

		// Inherited from QuadTree
		void add(QuadTreeOccupant* pOc);

		void clearTree();

		bool created();
	};
}

#endif
