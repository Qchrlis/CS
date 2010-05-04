/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
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

#include "cssysdef.h"
#include "csgeom/matrix3.h"
#include "csgeom/transfrm.h"
#include "csgeom/quaternion.h"
#include "csgeom/vector3.h"
#include "iengine/movable.h"
#include "iutil/strset.h"

// Bullet includes.
#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"

#include "softbodies.h"

CS_PLUGIN_NAMESPACE_BEGIN(Bullet)
{

csBulletSoftBody::csBulletSoftBody (csBulletDynamicsSystem* dynSys,
				    btSoftBody* body)
  : scfImplementationType (this), dynSys (dynSys), body (body)
{
  body->setUserPointer (this);
}

csBulletSoftBody::~csBulletSoftBody ()
{
  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (dynSys->bulletWorld);
  softWorld->removeSoftBody (body);
  delete body;
}

void csBulletSoftBody::DebugDraw (iView* rview)
{
  if (!dynSys->debugDraw)
  {
    dynSys->debugDraw = new csBulletDebugDraw (dynSys->inverseInternalScale);
    dynSys->bulletWorld->setDebugDrawer (dynSys->debugDraw);
  }

  btSoftBodyHelpers::Draw (body, dynSys->debugDraw);
  dynSys->debugDraw->DebugDraw (rview);
}

void csBulletSoftBody::SetMass (float mass)
{
  CS_ASSERT(mass > 0);

  btSoftRigidDynamicsWorld* softWorld =
    static_cast<btSoftRigidDynamicsWorld*> (dynSys->bulletWorld);
  softWorld->removeSoftBody (body);

  body->setTotalMass (mass);

  softWorld->addSoftBody (body);
}

float csBulletSoftBody::GetMass ()
{
  return body->getTotalMass ();
}

size_t csBulletSoftBody::GetVertexCount ()
{
  return body->m_nodes.size ();
}

csVector3 csBulletSoftBody::GetVertexPosition (size_t index)
{
  CS_ASSERT(index < (size_t) body->m_nodes.size ());
  return BulletToCS (body->m_nodes[index].m_x, dynSys->inverseInternalScale);
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex)
{
  CS_ASSERT(vertexIndex < (size_t) body->m_nodes.size ());
  body->setMass (vertexIndex, 0.0f);
}

void csBulletSoftBody::AnchorVertex (size_t vertexIndex, iRigidBody* body)
{
  csBulletRigidBody* rigidBody = dynamic_cast<csBulletRigidBody*> (body);
  CS_ASSERT(rigidBody
	    && vertexIndex < (size_t) this->body->m_nodes.size ()
	    && rigidBody->body);
  this->body->appendAnchor (vertexIndex, rigidBody->body);
}

void csBulletSoftBody::SetRigidity (float rigidity)
{
  CS_ASSERT(rigidity >= 0.0f && rigidity <= 1.0f);
  body->m_materials[0]->m_kLST = rigidity;
}

float csBulletSoftBody::GetRigidity ()
{
  return body->m_materials[0]->m_kLST;
}

void csBulletSoftBody::SetLinearVelocity (csVector3 velocity)
{
  // Strangely, Bullet defines adding velocities of soft bodies with a scale of 100...
  body->setVelocity (CSToBullet (velocity * 100.0f, dynSys->inverseInternalScale));
}

void csBulletSoftBody::SetLinearVelocity (csVector3 velocity, size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) body->m_nodes.size ());
  // Strangely, Bullet defines adding velocities of soft bodies with a scale of 100...
  body->addVelocity (CSToBullet (velocity * 100.0f, dynSys->inverseInternalScale)
		     - body->m_nodes[vertexIndex].m_v, vertexIndex);
}

csVector3 csBulletSoftBody::GetLinearVelocity (size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) body->m_nodes.size ());
  return BulletToCS (body->m_nodes[vertexIndex].m_v, dynSys->inverseInternalScale);
}

void csBulletSoftBody::AddForce (csVector3 force)
{
  // Strangely, Bullet defines adding forces to soft bodies with a scale of 100...
  body->addForce (CSToBullet (force * 100.0f, dynSys->inverseInternalScale));
}

void csBulletSoftBody::AddForce (csVector3 force, size_t vertexIndex)
{
  CS_ASSERT (vertexIndex < (size_t) body->m_nodes.size ());
  // Strangely, Bullet defines adding forces to soft bodies indices with a scale of 10000...
  body->addForce (CSToBullet (force * 10000.0f, dynSys->inverseInternalScale), vertexIndex);
}

}
CS_PLUGIN_NAMESPACE_END(Bullet)