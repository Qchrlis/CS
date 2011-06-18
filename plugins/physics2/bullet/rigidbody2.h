#ifndef __CS_BULLET_RIGIDBODY_H__
#define __CS_BULLET_RIGIDBODY_H__

#include "bullet2.h"
#include "common2.h"
#include "collisionobject2.h"

CS_PLUGIN_NAMESPACE_BEGIN (Bullet2)
{
class csBulletPhysicalSystem;
class csBulletDefaultKinematicCallback;

using CS::Physics::iRigidBody;
using CS::Physics::iSoftBody;

class csBulletRigidBody : public scfImplementationExt1<
  csBulletRigidBody, csBulletCollisionObject, 
  CS::Physics::iRigidBody>
{
  friend class csBulletSoftBody;
  friend class csBulletJoint;
private:
  btRigidBody* btBody;
  //CS::Physics::PhysicalBodyType bodyType;
  CS::Physics::RigidBodyState physicalState;
  float density;
  float linearDampening;
  float angularDampening;
  float friction;
  float softness;
  float elasticity;

  csRef<CS::Physics::iKinematicCallback> kinematicCb;

public:
  csBulletRigidBody (csBulletSystem* phySys);
  virtual ~csBulletRigidBody ();

  //iCollisionObject
  virtual iCollisionObject* QueryCollisionObject () {return dynamic_cast<csBulletCollisionObject*> (this);}

  virtual void SetObjectType (CS::Collision::CollisionObjectType type) {}
  virtual CS::Collision::CollisionObjectType GetObjectType () {return CS::Collision::COLLISION_OBJECT_PHYSICAL;}

  virtual void SetAttachedMovable (iMovable* movable) {csBulletCollisionObject::SetAttachedMovable (movable);}
  virtual iMovable* GetAttachedMovable () {return csBulletCollisionObject::GetAttachedMovable ();}

  virtual void SetTransform (const csOrthoTransform& trans) {csBulletCollisionObject::SetTransform (trans);}
  virtual csOrthoTransform GetTransform () {return csBulletCollisionObject::GetTransform ();}

  virtual void RebuildObject () {csBulletCollisionObject::RebuildObject ();}

  virtual void AddCollider (CS::Collision::iCollider* collider, const csOrthoTransform& relaTrans);
  virtual void RemoveCollider (CS::Collision::iCollider* collider) {csBulletCollisionObject::RemoveCollider (collider);}
  virtual void RemoveCollider (size_t index) {csBulletCollisionObject::RemoveCollider (index);}

  virtual CS::Collision::iCollider* GetCollider (size_t index) {return csBulletCollisionObject::GetCollider (index);}
  virtual size_t GetColliderCount () {return colliders.GetSize ();}

  virtual void SetCollisionGroup (const char* name) {csBulletCollisionObject::SetCollisionGroup (name);}
  virtual const char* GetCollisionGroup () const {return csBulletCollisionObject::GetCollisionGroup ();}

  virtual void SetCollisionCallback (CS::Collision::iCollisionCallback* cb) {collCb = cb;}
  virtual CS::Collision::iCollisionCallback* GetCollisionCallback () {return collCb;}

  virtual bool Collide (iCollisionObject* otherObject) {return csBulletCollisionObject::Collide (otherObject);}
  virtual CS::Collision::HitBeamResult HitBeam (const csVector3& start, const csVector3& end)
  { return csBulletCollisionObject::HitBeam (start, end);}

  btRigidBody* GetBulletRigidPointer () {return btBody;}
  virtual void RemoveBulletObject ();
  virtual void AddBulletObject ();

  //iPhysicalBody

  virtual CS::Physics::PhysicalBodyType GetBodyType () const {return CS::Physics::BODY_RIGID;}
  virtual iRigidBody* QueryRigidBody () {return dynamic_cast<iRigidBody*>(this);}
  virtual iSoftBody* QuerySoftBody () {return NULL;}

  virtual bool Disable ();
  virtual bool Enable ();
  virtual bool IsEnabled ();

  virtual float GetMass ();

  virtual float GetDensity () const {return density;}
  virtual void SetDensity (float density);

  virtual float GetVolume ();

  virtual void AddForce (const csVector3& force);

  virtual void SetLinearVelocity (const csVector3& vel);
  virtual csVector3 GetLinearVelocity (size_t index = 0) const;

  virtual void SetFriction (float friction);
  virtual float GetFriction () {return friction;}

  //iRigidBody
  virtual CS::Physics::RigidBodyState GetState () {return physicalState;}
  virtual bool SetState (CS::Physics::RigidBodyState state);

  virtual void SetElasticity (float elasticity);
  virtual float GetElasticity () {return elasticity;}

  virtual void SetAngularVelocity (const csVector3& vel);
  virtual csVector3 GetAngularVelocity () const;

  virtual void AddTorque (const csVector3& torque);

  virtual void AddRelForce (const csVector3& force);
  virtual void AddRelTorque (const csVector3& torque);

  virtual void AddForceAtPos (const csVector3& force,
      const csVector3& pos);
  virtual void AddForceAtRelPos (const csVector3& force,
      const csVector3& pos);

  virtual void AddRelForceAtPos (const csVector3& force,
      const csVector3& pos);
  virtual void AddRelForceAtRelPos (const csVector3& force,
      const csVector3& pos);

  virtual csVector3 GetForce () const;
  virtual csVector3 GetTorque () const;

  virtual void SetKinematicCallback (CS::Physics::iKinematicCallback* cb) {kinematicCb = cb;}
  virtual CS::Physics::iKinematicCallback* GetKinematicCallback () {return kinematicCb;}

  virtual void SetLinearDampener (float d);
  virtual float GetLinearDampener () {return linearDampening;}

  virtual void SetRollingDampener (float d);
  virtual float GetRollingDampener () {return angularDampening;}
};

class csBulletDefaultKinematicCallback : public scfImplementation1<
  csBulletDefaultKinematicCallback, CS::Physics::iKinematicCallback>
{
public:
  csBulletDefaultKinematicCallback ();
  virtual ~csBulletDefaultKinematicCallback();
  virtual void GetBodyTransform (CS::Physics::iRigidBody* body, csOrthoTransform& transform) const;
};
}
CS_PLUGIN_NAMESPACE_END (Bullet2)

#endif