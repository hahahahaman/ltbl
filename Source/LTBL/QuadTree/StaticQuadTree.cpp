#include <LTBL/QuadTree/StaticQuadTree.h>

#include <cassert>

namespace qdt
{
	StaticQuadTree::StaticQuadTree()
		: m_created(false)
	{
	}

	StaticQuadTree::StaticQuadTree(const AABB &rootRegion)
		: m_created(false)
	{
		m_pRootNode.reset(new QuadTreeNode(rootRegion, 0, nullptr, this));

		m_created = true;
	}

	void StaticQuadTree::create(const AABB &rootRegion)
	{
		m_pRootNode.reset(new QuadTreeNode(rootRegion, 0, nullptr, this));

		m_created = true;
	}

	void StaticQuadTree::add(QuadTreeOccupant* pOc)
	{
		assert(m_created);

		setQuadTree(pOc);

		// If the occupant fits in the root node
		if(m_pRootNode->getRegion().contains(pOc->getAABB()))
			m_pRootNode->add(pOc);
		else
			m_outsideRoot.insert(pOc);
	}

	void StaticQuadTree::clearTree()
	{
		m_pRootNode.reset();

		m_outsideRoot.clear();

		m_created = false;
	}

	bool StaticQuadTree::created()
	{
		return m_created;
	}
}
