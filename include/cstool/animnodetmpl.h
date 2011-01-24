/*
  Copyright (C) 2011 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __CS_IMESH_ANIMNODE_TEMPLATE_H__
#define __CS_IMESH_ANIMNODE_TEMPLATE_H__

/**\file
 * Base implementation of CS::Animation::iSkeletonAnimNode objects.
 */

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/refarr.h"
#include "csutil/weakref.h"
#include "imesh/animnode/skeleton2anim.h"

namespace CS {
namespace Animation {

/// This macro defines the implementation of an animation node plugin manager
#define CS_DECLARE_ANIMNODE_MANAGER(nodename, nodeinterface, nodetype)			\
  class nodename##Manager : public scfImplementation2<nodename##Manager, \
    CS::Animation::iSkeleton##nodeinterface##Manager,			\
    iComponent>								\
      {									\
      public:								\
	CS_LEAKGUARD_DECLARE(nodename##Manager);			\
									\
	nodename##Manager (iBase* parent);				\
									\
	virtual nodetype* CreateAnimNodeFactory (const char* name);	\
	virtual nodetype* FindAnimNodeFactory (const char* name);	\
	virtual void RemoveAnimNodeFactory (const char* name);		\
	virtual void ClearAnimNodeFactories ();				\
									\
	virtual bool Initialize (iObjectRegistry*);			\
									\
	void Report (int severity, const char* msg, ...) const;		\
									\
      private:								\
	iObjectRegistry* object_reg;					\
	csHash<csRef<nodetype>, csString> nodeFactories;		\
									\
	friend class nodename##Factory;					\
	friend class nodename;						\
      };


/// This macro implements the implementation of an animation node plugin manager
#define CS_IMPLEMENT_ANIMNODE_MANAGER(nodename, nodetype, id)		\
  SCF_IMPLEMENT_FACTORY(nodename##Manager);				\
  CS_LEAKGUARD_IMPLEMENT(nodename##Manager);				\
									\
  nodename##Manager::nodename##Manager (iBase* parent)			\
    : scfImplementationType (this, parent)				\
    {}									\
									\
  nodetype* nodename##Manager::CreateAnimNodeFactory (const char* name)	\
    {									\
      csRef<nodetype> newFact;						\
      newFact.AttachNew (new nodename##Factory (this, name));		\
									\
      return nodeFactories.PutUnique (name, newFact);			\
    }									\
									\
  nodetype* nodename##Manager::FindAnimNodeFactory (const char* name)	\
    {									\
      return nodeFactories.Get (name, 0);				\
    }									\
									\
  void nodename##Manager::RemoveAnimNodeFactory (const char* name)	\
    {									\
      nodeFactories.DeleteAll (name);					\
    }									\
									\
  void nodename##Manager::ClearAnimNodeFactories ()			\
  {									\
    nodeFactories.DeleteAll ();						\
  }									\
									\
  bool nodename##Manager::Initialize (iObjectRegistry* object_reg)	\
  {									\
    this->object_reg = object_reg;					\
    return true;							\
  }									\
									\
  void nodename##Manager::Report (int severity, const char* msg, ...) const \
  {									\
    va_list arg;							\
    va_start (arg, msg);						\
    csRef<iReporter> rep (csQueryRegistry<iReporter> (object_reg));	\
    if (rep)								\
      rep->ReportV (severity,						\
		    "crystalspace.mesh.animesh.animnode.##id",		\
		    msg, arg);						\
    else								\
      {									\
	csPrintfV (msg, arg);						\
	csPrintf ("\n");						\
      }									\
    va_end (arg);							\
  }

/// This macro implements the CreateInstance and FindNode methods of an animation node factory
/// with a single child
#define CS_IMPLEMENT_ANIMNODE_FACTORY_SINGLE(nodename)			\
  csPtr<CS::Animation::iSkeletonAnimNode> nodename##Factory::CreateInstance \
    (CS::Animation::iSkeletonAnimPacket* packet, CS::Animation::iSkeleton* skeleton) \
    {									\
      csRef<nodename> newP;						\
      newP.AttachNew (new nodename (this, skeleton));			\
									\
      if (childNodeFactory)						\
	{								\
	  csRef<CS::Animation::iSkeletonAnimNode> node =		\
	    childNodeFactory->CreateInstance (packet, skeleton);	\
	  newP->childNode = node;					\
	}								\
									\
      return csPtr<CS::Animation::iSkeletonAnimNode> (newP);		\
    }									\
									\
  CS::Animation::iSkeletonAnimNodeFactory* nodename##Factory::FindNode (const char* name) \
    {									\
      if (this->name == name)						\
	return this;							\
									\
      if (childNodeFactory)						\
	return childNodeFactory->FindNode (name);			\
									\
      return nullptr;							\
    }

/// This macro implements the GetFactory and FindNode methods of an animation node with a single child
#define CS_IMPLEMENT_ANIMNODE_SINGLE(nodename)				\
  CS::Animation::iSkeletonAnimNodeFactory* nodename::GetFactory () const \
    {									\
      return factory;							\
    }									\
									\
  CS::Animation::iSkeletonAnimNode* nodename::FindNode (const char* name) \
    {									\
      if (factory->name == name)					\
	return this;							\
									\
      if (childNode)							\
	return childNode->FindNode (name);				\
									\
      return nullptr;							\
    }

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory
 */
class CS_CRYSTALSPACE_EXPORT csSkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  csSkeletonAnimNodeFactory (const char* name);

  /**
   * Destructor
   */
  virtual ~csSkeletonAnimNodeFactory () {}

  /**
   * Get the name of this factory
   */
  virtual const char* GetNodeName () const;

 protected:
  csString name;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory with a single child
 */
class CS_CRYSTALSPACE_EXPORT csSkeletonAnimNodeFactorySingle
  : public csSkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  csSkeletonAnimNodeFactorySingle (const char* name);

  /**
   * Destructor
   */
  virtual ~csSkeletonAnimNodeFactorySingle () {}

  /**
   * Set the child animation node of this node. It is valid to provide a null pointer.
   */
  virtual void SetChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Get the child animation node of this node, or nullptr if there are none.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode () const;

 protected:
  csRef<CS::Animation::iSkeletonAnimNodeFactory> childNodeFactory;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNode with a single child
 */
class CS_CRYSTALSPACE_EXPORT csSkeletonAnimNodeSingle
{
 public:
  /**
   * Constructor
   */
  csSkeletonAnimNodeSingle (CS::Animation::iSkeleton* skeleton);

  /**
   * Destructor
   */
  virtual ~csSkeletonAnimNodeSingle () {}

  /**
   * Get the child node of this node, or nullptr if there are none.
   */
  virtual iSkeletonAnimNode* GetChildNode () const;

  /**
   * Start playing the node, it will therefore start modifying the state of the skeleton.
   */
  virtual void Play ();

  /**
   * Stop playing the node, it will no longer modify the state of the skeleton.
   */
  virtual void Stop ();

  /**
   * Set the current playback position, in seconds. If time is set beyond the end of the
   * animation then it will be capped.
   */
  virtual void SetPlaybackPosition (float time);

  /**
   * Get the current playback position, in seconds (ie a time value between 0 and GetDuration()).
   */
  virtual float GetPlaybackPosition () const;

  /**
   * Get the time length of this node, in seconds
   */
  virtual float GetDuration () const;

  /**
   * Set the playback speed.
   */
  virtual void SetPlaybackSpeed (float speed);

  /**
   * Get the playback speed. The default value is 1.0.
   */
  virtual float GetPlaybackSpeed () const;

  /**
   * Blend the state of this node into the global skeleton state.
   *
   * \param state The global blend state to blend into
   * \param baseWeight Global weight for the blending of this node
   */
  virtual void BlendState (csSkeletalState* state, float baseWeight = 1.0f);

  /**
   * Update the state of the animation generated by this node
   * \param dt The time since the last update, in seconds
   */
  virtual void TickAnimation (float dt);

  /**
   * Return whether or not this node is currently playing and needs any blending.
   */
  virtual bool IsActive () const;

  /**
   * Add a new animation callback to this node
   * \param callback The callback object
   */
  virtual void AddAnimationCallback (iSkeletonAnimCallback* callback);

  /**
   * Remove the given animation callback from this node
   * \param callback The callback object
   */
  virtual void RemoveAnimationCallback (iSkeletonAnimCallback* callback);

 protected:
  csWeakRef<CS::Animation::iSkeleton> skeleton;
  csRef<CS::Animation::iSkeletonAnimNode> childNode;
  bool isPlaying;
  float playbackSpeed;
};

/**
 * Base implementation of a CS::Animation::iSkeletonAnimNodeFactory with more than one child
 */
class CS_CRYSTALSPACE_EXPORT csSkeletonAnimNodeFactoryMulti
  : public csSkeletonAnimNodeFactory
{
 public:
  /**
   * Constructor
   */
  csSkeletonAnimNodeFactoryMulti (const char* name);

  /**
   * Destructor
   */
  virtual ~csSkeletonAnimNodeFactoryMulti () {}

  /**
   * Add a child animation node to this node. It is NOT valid to provide a null pointer.
   */
  virtual void AddChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Remove a child animation node from this node.
   */
  virtual void RemoveChildNode (iSkeletonAnimNodeFactory* factory);

  /**
   * Remove all child animation nodes from this node.
   */
  virtual void ClearChildNodes ();

  /**
   * Get the child animation node of this node with the given index.
   */
  virtual iSkeletonAnimNodeFactory* GetChildNode (size_t index) const;

 protected:
  csRefArray<CS::Animation::iSkeletonAnimNodeFactory> childNodeFactories;
};

} // namespace Animation
} // namespace CS

#endif // __CS_IMESH_ANIMNODE_TEMPLATE_H__
