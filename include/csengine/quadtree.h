/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef QUADTREE_H
#define QUADTREE_H

#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/box.h"

// A quadtree node can be in three states: empty, full, or partial.
// If empty or full the state of the children does not matter.
#define CS_QUAD_EMPTY 0
#define CS_QUAD_PARTIAL 1
#define CS_QUAD_UNKNOWN 2
#define CS_QUAD_FULL 3

class csQuadtree;
class Dumper;


#if 0
//// old Quadtree code
/**
 * A quadtree node.
 */
class csQuadtreeNode
{
  friend class csQuadtree;
  friend class Dumper;

private:
  /**
   * Children.
   * 0 = topleft, 1=topright, 2=bottomright, 3=bottomleft.
   */
  csQuadtreeNode* children;

  /// Center point for this node.
  csVector2 center;
  /// Contents state of this node.
  int state;

private:
  /// Make an empty quadtree node.
  csQuadtreeNode ();

  /**
   * Destroy this quadtree node.
   */
  ~csQuadtreeNode ();

  /// Set center.
  void SetCenter (const csVector2& cen) { center = cen; }

public:
  /// Get center.
  const csVector2& GetCenter () { return center; }

  /// Get contents state.
  int GetState () { return state; }

  /// Set state.
  void SetState (int st) { state = st; }
};

/**
 * The quadtree.
 */
class csQuadtree
{
  friend class Dumper;

private:
  /// The root of the tree.
  csQuadtreeNode* root;
  /// The bounding box of the entire tree.
  csBox2 bbox;

private:
  /// Build the tree with a given depth.
  void Build (csQuadtreeNode* node, const csBox2& box, int depth);

  /// Insert a polygon in the node.
  bool InsertPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox2& cur_bbox, const csBox2& pol_bbox);

  /// Test a polygon in the node.
  bool TestPolygon (csQuadtreeNode* node,
	csVector2* verts, int num_verts,
	const csBox2& cur_bbox, const csBox2& pol_bbox);

  /// Test a point in the node.
  int TestPoint (csQuadtreeNode* node, const csVector2& point);

public:
  /**
   * Create an empty tree with the given box.
   */
  csQuadtree (const csBox2& box, int depth);

  /**
   * Destroy the whole quadtree.
   */
  virtual ~csQuadtree ();

  /**
   * Make tree empty.
   */
  void MakeEmpty () { root->SetState (CS_QUAD_EMPTY); }

  /**
   * Is the tree full?
   */
  bool IsFull () { return root->GetState () == CS_QUAD_FULL; }

  /**
   * Insert a polygon into the quad-tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible.
   */
  bool InsertPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   * Return true if polygon is visible.
   */
  bool TestPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test if a given point is visible in the quad-tree.
   * Returns CS_QUAD_FULL if not visible, CS_QUAD_EMPTY
   * if visible and CS_QUAD_PARTIAL if undecided.
   * This function returns CS_QUAD_UNKNOWN if the point is not
   * in the quadtree.
   */
  int TestPoint (const csVector2& point);
};

#endif /// old quadtree code

/**
 *  4 child node states are stored in one byte.
 *  2 bits per node, in sequence. topleft, topright, botright, botleft
 *  nodenrs are: 0 = topleft, 1=topright, 2=bottomright, 3=bottomleft.
 */
#define CS_QUAD_ALL_EMPTY 0x0 /* because CS_QUAD_EMPTY == 0 */
#define CS_QUAD_ALL_FULL 0xFF /* because CS_QUAD_FULL == 3 */

/**
 *  QuadTree
 */
class csQuadTree {
  friend class Dumper;
private:
  /// bounding box of the quadtree
  csBox2 bbox;
  /// depth of the tree, 1 == 1 node only, a root.
  int max_depth;
  /// the state of the root node.
  int root_state;

  /** state of all children, and their children.
   *  they are ordered like this:
   *  root has children in byte 0.
   *  nodes in byte 0 have children in byte 0+node_nr+1(1,2,3,4).
   *  nodes in byte 1 have children in byte 4+node_nr+1.
   *  nodes in byte n have children in byte 4*n+node_nr+1
   *  So for byte n, take 4*n + node_nr+1 as the new byte
   */
  unsigned char* states;
  /// convenience variable: how many bytes alloced in states
  int state_size;


  /** this function is used for traversing the quadtree.
   *  it will get the nodes bounding box, state, byte offset and node_nr,
   *  all these are this node's values.
   *  and custom clientdata.
   *  It can return an integer.
   *  node_depth==1 means you are the root, and so on,
   *  if the node_depth == max_depth you are a leaf.
   */
  typedef int (csQuadTree::quad_traverse_func)(const csBox2& node_bbox, 
    int node_depth, int node_state, int offset, int node_nr, void* data);

  /** private functions to help dealing with quadtree
   *  call all four children. Of the node with it's state at offset and node_nr
   *  box is the bounding box of the node, depth is its depth.
   *  each will be passed the data.
   *  returns return values of children in retval.
   *  note that theoretically the state of the caller could be changed.
   *  also: the offset -1 denotes the root node is calling.
   *  func is of type quad_traverse_func.
   */
  void CallChildren(int (csQuadTree::*func)(const csBox2& node_bbox,
    int node_depth, int node_state, int offset, int node_nr, void* data), 
    const csBox2& box, int depth, int offset, int node_nr, 
    void *data, int retval[4]);

  /** Convenience version, the retval argument is omitted, return values
   * are discarded.
   */
  void CallChildren(int (csQuadTree::*func)(const csBox2& node_bbox,
    int node_depth, int node_state, int offset, int node_nr, void* data), 
    const csBox2& box, int depth, int offset, int node_nr, 
    void *data);
  /** Get the state of a node, in byte offset , nodenr given.
   *  offset -1 means root node.
   */
  int GetNodeState(int offset, int nodenr);

  /** Set the state of a node, in byte offset , nodenr given.
   *  offset -1 means root node.
   */
  void SetNodeState(int offset, int nodenr, int newstate);

  /** for a node, check if data (a csVector2 * )
   * is in this node, possibly recursing.
   * returning the QUAD_FULL/PARTIAL/EMPTY if it hits.
   * returning QUAD_UNKOWN if the point is not in the node.
   */
  quad_traverse_func test_point_func;
  /** gather result from retval for testpoint
   */
  int GetTestPointResult(int retval[4]); 

  /** struct with info for polygon being inserted
   */
  struct insert_poly_info {
    csVector2* verts;
    int num_verts;
    const csBox2& pol_bbox;
  };
  
  /** for a node, insert polygon into it (possibly in its children too)
   * expects data to be a struct insert_poly_info*
   * returns true if the polygon covered previously empty space.
   */
  quad_traverse_func insert_polygon_func;

  /** for a node, mark it by casting (void*)data to an int.
   *  that is the new state.
   */
  quad_traverse_func mark_node_func;

public:
  /** create a quadtree of depth, using about 2**twice depth bytes. depth >= 1
   *  depth=1 is only the root.
   */
  csQuadTree(const csBox2& the_box, int the_depth);

  /// destroy quadtree
  ~csQuadTree();

  /**
   * Is the tree full?
   */
  bool IsFull () { return root_state == CS_QUAD_FULL; }

  /** 
   * Make the tree empty again
   */
  void MakeEmpty();

  /**
   * Insert a polygon into the quad-tree.
   * Return true if the tree was modified (i.e. if parts of the
   * polygon were visible.
   */
  bool InsertPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test for polygon visibility with the quad-tree.
   * Return true if polygon is visible.
   */
  bool TestPolygon (csVector2* verts, int num_verts,
  	const csBox2& pol_bbox);

  /**
   * Test if a given point is visible in the quad-tree.
   * Returns CS_QUAD_FULL if not visible, CS_QUAD_EMPTY
   * if visible and CS_QUAD_PARTIAL if undecided.
   * This function returns CS_QUAD_UNKNOWN if the point is not
   * in the quadtree.
   */
  int TestPoint (const csVector2& point);
};


#endif /*QUADTREE_H*/

