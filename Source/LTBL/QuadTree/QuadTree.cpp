#include <LTBL/QuadTree/QuadTree.h>

#include <LTBL/QuadTree/QuadTreeNode.h>
#include <LTBL/QuadTree/QuadTree.h>

#include <SFML/OpenGL.hpp>



namespace qdt
{
	void QuadTree::onRemoval()
	{
	}

	void QuadTree::setQuadTree(QuadTreeOccupant* pOc)
	{
		pOc->m_pQuadTree = this;
	}

	void QuadTree::query_Region(const AABB &region, std::vector<QuadTreeOccupant*> &result)
	{
		// Query outside root elements
		for(std::unordered_set<QuadTreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
		{
			QuadTreeOccupant* pOc = *it;

			if(region.intersects(pOc->m_aabb))
			{
				// intersects, add to list
				result.push_back(pOc);
			}
		}

		std::list<QuadTreeNode*> open;

		open.push_back(m_pRootNode.get());

		while(!open.empty())
		{
			// Depth-first (results in less memory usage), remove objects from open list
			QuadTreeNode* pCurrent = open.back();
			open.pop_back();

			// Add occupants if they are in the region
			for(std::unordered_set<QuadTreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				QuadTreeOccupant* pOc = *it;

				if(region.intersects(pOc->m_aabb))
				{
					// intersects, add to list
					result.push_back(pOc);
				}
			}

			// Add children to open list if they intersect the region
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
					{
					    // m_children vector<vector <QuadTreeNode>>
						if(region.intersects((*pCurrent->m_children)[x][y].m_region))
							open.push_back(&(*pCurrent->m_children)[x][y]);
					}
			}
		}
	}

	void QuadTree::query_Region(const AABB &region, const sf::Vector2f &maxDims, std::vector<QuadTreeOccupant*> &result)
	{
		// Query outside root elements
		for(std::unordered_set<QuadTreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
		{
			QuadTreeOccupant* pOc = *it;
			AABB aabb = pOc->m_aabb;

			if(pOc->m_aabb.getDims().x > maxDims.x)
                aabb.setDims(sf::Vector2f(maxDims.x, aabb.getDims().y));

            if(pOc->m_aabb.getDims().y > maxDims.y)
                aabb.setDims(sf::Vector2f(aabb.getDims().x, maxDims.y));

			if(region.intersects(aabb))
			{
				// intersects, add to list
				result.push_back(pOc);
			}
		}

		std::list<QuadTreeNode*> open;

		open.push_back(m_pRootNode.get());

		while(!open.empty())
		{
			// Depth-first (results in less memory usage), remove objects from open list
			QuadTreeNode* pCurrent = open.back();
			open.pop_back();

			// Add occupants if they are in the region
			for(std::unordered_set<QuadTreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				QuadTreeOccupant* pOc = *it;

				AABB aabb = pOc->m_aabb;

                if(pOc->m_aabb.getDims().x > maxDims.x)
                    aabb.setDims(sf::Vector2f(maxDims.x, aabb.getDims().y));

                if(pOc->m_aabb.getDims().y > maxDims.y)
                    aabb.setDims(sf::Vector2f(aabb.getDims().x, maxDims.y));

				if(region.intersects(pOc->m_aabb))
				{
					// intersects, add to list
					result.push_back(pOc);
				}
			}

			// Add children to open list if they intersect the region
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
					{
					    // m_children vector<vector <QuadTreeNode>>
						if(region.intersects((*pCurrent->m_children)[x][y].m_region))
							open.push_back(&(*pCurrent->m_children)[x][y]);
					}
			}
		}
	}



	void QuadTree::debugRender()
	{
		// Render outside root AABB's
		glColor3f(0.5f, 0.2f, 0.1f);

		for(std::unordered_set<QuadTreeOccupant*>::iterator it = m_outsideRoot.begin(); it != m_outsideRoot.end(); it++)
			(*it)->m_aabb.debugRender();

		// Now draw the tree
		std::list<QuadTreeNode*> open;

		open.push_back(m_pRootNode.get());

		while(!open.empty())
		{
			// Depth-first (results in less memory usage), remove objects from open list
			QuadTreeNode* pCurrent = open.back();
			open.pop_back();

			// Render node region AABB
			glColor3f(0.4f, 0.9f, 0.7f);

			pCurrent->m_region.debugRender();

			glColor3f(0.5f, 0.2f, 0.2f);

			// Render occupants
			for(std::unordered_set<QuadTreeOccupant*>::iterator it = pCurrent->m_pOccupants.begin(); it != pCurrent->m_pOccupants.end(); it++)
			{
				QuadTreeOccupant* pOc = *it;

				pOc->m_aabb.debugRender();
			}

			// Add children to open list if they are visible
			if(pCurrent->m_hasChildren)
			{
				for(int x = 0; x < 2; x++)
					for(int y = 0; y < 2; y++)
						open.push_back(&(*pCurrent->m_children)[x][y]);
			}
		}
	}
}
