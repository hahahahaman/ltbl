#ifndef QDT_QUADTREENODE_H
#define QDT_QUADTREENODE_H

#include <LTBL/Constructs.h>
#include <LTBL/QuadTree/QuadTreeOccupant.h>

#include <vector>
#include <unordered_set>
#include <memory>

namespace qdt
{
	class QuadTreeNode
	{
	private:
		class QuadTreeNode* m_pParent; // idk some  weird C idea of declaring the struct/class before the type
		class QuadTree* m_pQuadTree; // kind of like typedef, but i don't think it's actually necessary

		// Cannot use a unique_ptr, since the vector requires copy ctor/assignment ops
		std::vector<std::vector<QuadTreeNode>>* m_children;
		bool m_hasChildren;

		std::unordered_set<class QuadTreeOccupant*> m_pOccupants;

		AABB m_region;

		int m_level;

		int m_numOccupantsBelow;

		inline QuadTreeNode* getChild(const sf::Vector2i &position);

		void getPossibleOccupantPosition(QuadTreeOccupant* pOc, sf::Vector2i &point);

		void addToThisLevel(QuadTreeOccupant* pOc);

		// Returns true if occupant was added to children
		bool addToChildren(QuadTreeOccupant* pOc);

		void getOccupants(std::unordered_set<QuadTreeOccupant*> &occupants);

		void partitionTreeNode();
		void destroyChildren();
		void mergeTreeNode();

		void update(QuadTreeOccupant* pOc);
		void removeFromTreeNode(QuadTreeOccupant* pOc);

	public:
		static int minNumOccupants;
		static int maxNumOccupants;
		static int maxNumLevels;

		static float m_oversizeMultiplier;

		QuadTreeNode();
		QuadTreeNode(const AABB &region, int level, QuadTreeNode* pParent = NULL, QuadTree* pQuadTree = NULL);
		~QuadTreeNode();

		// For use after using default constructor
		void create(const AABB &region, int level, QuadTreeNode* pParent = NULL, QuadTree* pQuadTree = NULL);

		QuadTree* getTree();

		void add(QuadTreeOccupant* pOc);

		const AABB &getRegion();

		void getAllOccupantsBelow(std::vector<QuadTreeOccupant*> &occupants);

		friend class QuadTreeOccupant;
		friend class QuadTree;
	};
}

#endif
