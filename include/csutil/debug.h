/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_UTIL_DEBUG__
#define __CS_UTIL_DEBUG__

struct iObjectRegistry;

#define DG_ADD_O(object_reg,obj,desc) \
  csDebuggingGraph::AddObject(object_reg,(void*)(obj),__FILE__,__LINE__,desc)
#define DG_REM_O(object_reg,obj) \
  csDebuggingGraph::RemoveObject(object_reg,(void*)(obj),__FILE__,__LINE__)
#define DG_ADDCHILD_O(object_reg,parent,child) \
  csDebuggingGraph::AddChild(object_reg,(void*)(parent),(void*)(child))
#define DG_ADDPARENT_O(object_reg,child,parent) \
  csDebuggingGraph::AddParent(object_reg,(void*)(child),(void*)(parent))
#define DG_REMCHILD_O(object_reg,parent,child) \
  csDebuggingGraph::RemoveChild(object_reg,(void*)(parent),(void*)(child))
#define DG_REMPARENT_O(object_reg,child,parent) \
  csDebuggingGraph::RemoveParent(object_reg,(void*)(child),(void*)(parent))
#define DG_LINK_O(object_reg,parent,child) \
  csDebuggingGraph::AddChild(object_reg,(void*)(parent),(void*)(child)); \
  csDebuggingGraph::AddParent(object_reg,(void*)(child),(void*)(parent))
#define DG_UNLINK_O(object_reg,parent,child) \
  csDebuggingGraph::RemoveChild(object_reg,(void*)(parent),(void*)(child)); \
  csDebuggingGraph::RemoveParent(object_reg,(void*)(child),(void*)(parent))

// The following versions of the defines are only available in
// debug mode. In release mode they do nothing. They expect SetupGraph()
// to be called with a valid object registry.
#ifdef CS_DEBUG
#define DG_ADD(obj,desc) \
  csDebuggingGraph::AddObject(NULL,(void*)(obj),__FILE__,__LINE__,desc)
#define DG_REM(obj) \
  csDebuggingGraph::RemoveObject(NULL,(void*)(obj),__FILE__,__LINE__)
#define DG_ADDCHILD(parent,child) \
  csDebuggingGraph::AddChild(NULL,(void*)(parent),(void*)(child))
#define DG_ADDPARENT(child,parent) \
  csDebuggingGraph::AddParent(NULL,(void*)(child),(void*)(parent))
#define DG_REMCHILD(parent,child) \
  csDebuggingGraph::RemoveChild(NULL,(void*)(parent),(void*)(child))
#define DG_REMPARENT(child,parent) \
  csDebuggingGraph::RemoveParent(NULL,(void*)(child),(void*)(parent))
#define DG_LINK(parent,child) \
  csDebuggingGraph::AddChild(NULL,(void*)(parent),(void*)(child)); \
  csDebuggingGraph::AddParent(NULL,(void*)(child),(void*)(parent))
#define DG_UNLINK(parent,child) \
  csDebuggingGraph::RemoveChild(NULL,(void*)(parent),(void*)(child)); \
  csDebuggingGraph::RemoveParent(NULL,(void*)(child),(void*)(parent))
#else
#define DG_ADD(obj,desc)
#define DG_REM(obj)
#define DG_ADDCHILD(parent,child)
#define DG_ADDPARENT(child,parent)
#define DG_REMCHILD(parent,child)
#define DG_REMPARENT(child,parent)
#define DG_LINK(parent,child)
#define DG_UNLINK(parent,child)
#endif

/**
 * This is a static class that helps with debugging.
 * It will register an object in the object registry that keeps
 * track of allocations in a graph. Later on you can add/remove
 * allocations from that graph.
 */
class csDebuggingGraph
{
public:
  /**
   * Set the mode to use for debugging the graph. One mode
   * is not exact which means that this class will keep the graph
   * correctly in sync independent of the functions that are called.
   * i.e. in this mode when RemoveObject() is called the object will
   * automatically be unlinked from its parents and children so the
   * graph is always ok.<p>
   * If 'exact' is true however (default), the graph will only do the
   * operations requested. i.e. removing an object will not automatically
   * remove the links. RemoveParent() has to be called explicitely. In this
   * mode it is possible that the tree will not be correct and the
   * dump will show this.<p>
   * Special note! In debug mode (CS_DEBUG) this function will put the
   * pointer to the object registry in iSCF::object_reg. That way we
   * can use this debugging functionality in places where the object
   * registry is not available.
   */
  static void SetupGraph (iObjectRegistry* object_reg, bool exact);

  /**
   * Add a new object to the debug graph and link to its parent.
   */
  static void AddObject (iObjectRegistry* object_reg,
  	void* object, char* file, int linenr,
  	char* description, ...);

  /**
   * Remove an object from the debug tree. This will automatically
   * unlink the object from all its parents and children unless
   * we are in 'exact' mode.
   */
  static void RemoveObject (iObjectRegistry* object_reg,
  	void* object, char* file, int linenr);

  /**
   * Add a child to an object.
   */
  static void AddChild (iObjectRegistry* object_reg,
  	void* parent, void* child);

  /**
   * Add a parent to an object.
   */
  static void AddParent (iObjectRegistry* object_reg,
  	void* child, void* parent);

  /**
   * Unlink a child from its parent.
   */
  static void RemoveChild (iObjectRegistry* object_reg,
  	void* parent, void* child);

  /**
   * Unlink a parent from its child.
   */
  static void RemoveParent (iObjectRegistry* object_reg,
  	void* child, void* parent);

  /**
   * Completely clear everything in the debug graph.
   */
  static void Clear (iObjectRegistry* object_reg);

  /**
   * Dump all the resulting graphs.
   */
  static void Dump (iObjectRegistry* object_reg);

  /**
   * Dump the graph containing the given object. You should
   * usually leave reset_mark alone. That's for internal use.
   * If find_root is true it will try to find the root
   * for the graph containing the object. The root is defined as
   * the graph element having the fewest parents.
   */
  static void Dump (iObjectRegistry* object_reg, void* object,
  	bool find_root = false, bool reset_mark = true);
};

#endif //__CS_UTIL_DEBUG__

