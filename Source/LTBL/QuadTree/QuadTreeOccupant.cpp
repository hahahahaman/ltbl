#include <LTBL/QuadTree/QuadTreeOccupant.h>

#include <LTBL/QuadTree/QuadTreeNode.h>
#include <LTBL/QuadTree/QuadTree.h>



#include <cassert>

namespace qdt
{
QuadTreeOccupant::QuadTreeOccupant()
    : m_aabb(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1.0f, 1.0f)),
      m_pQuadTreeNode(NULL), m_pQuadTree(NULL)
{
}

void QuadTreeOccupant::treeUpdate()
{
    if(m_pQuadTree == nullptr)
        return;

    if(m_pQuadTreeNode == nullptr)
    {
        // If it fits in the root now, add it
        QuadTreeNode* pRootNode = m_pQuadTree->m_pRootNode.get();

        if(pRootNode->m_region.contains(m_aabb))
        {
            // Remove from outside root and add to tree
            m_pQuadTree->m_outsideRoot.erase(this);

            pRootNode->add(this);
        }
    }
    else
        m_pQuadTreeNode->update(this);
}

void QuadTreeOccupant::removeFromTree()
{
    if(m_pQuadTreeNode == NULL)
    {
        // Not in a node, should be outside root then
        assert(m_pQuadTree != NULL);

        m_pQuadTree->m_outsideRoot.erase(this);

        m_pQuadTree->onRemoval();
    }
    else
        m_pQuadTreeNode->removeFromTreeNode(this);
}

const AABB& QuadTreeOccupant::getAABB()
{
    return m_aabb;
}
}
