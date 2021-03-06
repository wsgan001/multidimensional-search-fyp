#ifndef MDSEARCH_INDEXPSEUDOPYRAMIDTREE_H
#define MDSEARCH_INDEXPSEUDOPYRAMIDTREE_H

#include <vector>
#include <boost/unordered_map.hpp>
#include "IndexStructure.h"
#include "PyramidTree.h" // for 'ihash' structure

namespace mdsearch
{

	class IndexPseudoPyramidTree : public IndexStructure
	{

	public:
		enum CleanupProcedure
		{
			CLEANUP_PROC_DEFRAGMENT = 0,
			CLEANUP_PROC_REBUILD
		};

		IndexPseudoPyramidTree(unsigned int numDimensions, const Region& boundary,
			int maxEmptyElements = DEFAULT_MAX_EMPTY_ELEMENTS,
			CleanupProcedure cleanupProcedure = CLEANUP_PROC_DEFRAGMENT);

		/* Clear all points currently stored in the structure. */
		virtual void clear();

		/* Insert point into the Pyramid Tree.
		 * Returns true if the point was inserted successfully and
		 * false if the point is already stored in the structure. */
		virtual bool insert(const Point& point);
		/* Remove point from the tree.
		 * Returns true if the point was removed successfully and
		 * false if the point was not being stored. */
		virtual bool remove(const Point& point);
		/* Update the value of an existing point.
		 * Returns true if the point was updated and false if the
		 * old point is not stored in the structure. */
		virtual bool update(const Point& oldPoint, const Point& newPoint);
		/* Return true if the given point is being stored in the structure. */
		virtual bool pointExists(const Point& point);

		/* Return all points currently stored in the structure's array.
		 * NOTE: May contain points recently deleted. Use emptyIndices()
		 * to find out which indices of the point list contain deleted pints. */
		const PointList& allPoints() const;
		/* Return indices of all ele,ents of point array which are deleted. */
		const IndexList& emptyIndices() const;
		/* Return spatial region this tree covers. */
		const Region& getBoundary() const;

	private:
		static const unsigned int MAX_BUCKET_NUMBER = 300000;
		/* Maximum amount of empty elements in the point array before
		 * it is defragmented (to ensure contiguous memory). */
		static const unsigned int DEFAULT_MAX_EMPTY_ELEMENTS = -1;

		void insertToStructure(const Point& point, bool searchKeyExists);
		int getPointIndex(const Point& point);
		/* Defragments internal point arrays by deleting all unused points. */
		void defragment();
		/* Given the index of the last removed point in the 'points' array,
		 * update the remaining index entries in the hash table. */
		void updatePointIndices(unsigned int removedIndex);
		/* Rebuilts the entire Pyramid tree structure, ensuring unused
		 * points (which have been marked for deletion) are not present
		 * in the  new structure. */
		void rebuild();

		// Contains all the full d-dimensional points stored in the Pyramid tree
		PointList points;
		// Contains the SUM of all elements for each stored point
		// That is, pointSums[i] stores the sum of all elements of points[i]
		RealList pointSums;
		// Unordered_map for storing the key values of 1-dimensional interpolated point.
		// Key = hashed Point, value = list of indices of potential points
		// Each key can contain MULTIPLE indices
		typedef boost::unordered_map<int, IndexList> OneDMap;
		OneDMap hashMap;   
		// Stores the indices which are empty in the linear points list
		// As a point is removed from the structure, the index is removed but
		// the point is not (as removal from the array is an O(n) operation).
		// This keeps track of which elements of the point array ARE NOT
		// being used
		IndexList emptyElementIndices;
		// Maximum amount of empty elements allowed before the list is defragmented
		// A value of -1 means the point list will NEVER be defragmented
		int maxEmptyElements;
		CleanupProcedure cleanupProcedure;

		// Entire region of space the Pyramid tree covers
		// (points outside this region are ignored)
		Region boundary;
		Point minPoint;
		Point maxPoint;
		// Interval between buckets
		unsigned int bucketInterval;

		std::vector<int> scaleFactors;
		std::vector<int> cumulativeSFProducts;		

	};

}

#endif