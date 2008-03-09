/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_IENGINE_CAMERA_H__
#define __CS_IENGINE_CAMERA_H__

/** \file
 * Camera object
 */

/**
 * \addtogroup engine3d_views
 * @{ */

#include "csgeom/matrix4.h"
#include "csutil/scf.h"
#include "csgeom/matrix4.h"

#define CS_VEC_FORWARD   csVector3(0,0,1)
#define CS_VEC_BACKWARD  csVector3(0,0,-1)
#define CS_VEC_RIGHT     csVector3(1,0,0)
#define CS_VEC_LEFT      csVector3(-1,0,0)
#define CS_VEC_UP        csVector3(0,1,0)
#define CS_VEC_DOWN      csVector3(0,-1,0)

#define CS_VEC_ROT_RIGHT  csVector3(0,1,0)
#define CS_VEC_ROT_LEFT   csVector3(0,-1,0)
#define CS_VEC_TILT_RIGHT (-csVector3(0,0,1))
#define CS_VEC_TILT_LEFT  (-csVector3(0,0,-1))
#define CS_VEC_TILT_UP    (-csVector3(1,0,0))
#define CS_VEC_TILT_DOWN  (-csVector3(-1,0,0))

class csOrthoTransform;
class csPlane3;
class csVector2;
class csVector3;

struct iSector;
struct iCamera;
struct iSceneNode;

/**
 * Implement this interface if you are interested in learning when
 * the camera changes sector.
 *
 * This callback is used by:
 * - iCamera
 */
struct iCameraSectorListener : public virtual iBase
{
  SCF_INTERFACE (iCameraSectorListener, 0, 0, 1);

  /// Fired when the sector changes.
  virtual void NewSector (iCamera* camera, iSector* sector) = 0;
};

/**
 * Camera class. This class represents camera objects which can be used to
 * render a world in the engine. A camera has the following properties:
 * - Home sector: The sector in which rendering starts.
 * - Transformation: This is an orthonormal transformation which is applied
 *   to all rendered objects to move them from world space to camera space.
 *   It is the mathematical representation of position and direction of the
 *   camera. The position should be inside the home sector.
 * - Field of View: Controls the size on screen of the rendered objects and
 *   can be used for zooming effects. The FOV can be given either in pixels
 *   or as an angle in degrees.
 * - Shift amount: The projection center in screen coordinates.
 * - Mirrored Flag: Should be set to true if the transformation is mirrored.
 * - Far Plane: A distant plane that is orthogonal to the view direction. It
 *   is used to clip away all objects that are farther away than a certain
 *   distance, usually to improve rendering speed.
 * - Camera number: An identifier for a camera transformation, used
 *   internally in the engine to detect outdated vertex buffers.
 * - Only Portals Flag: If this is true then no collisions are detected for
 *   camera movement except for portals.
 *
 * Main creators of instances implementing this interface:
 * - iEngine::CreateCamera()
 * - csView
 * 
 * Main ways to get pointers to this interface:
 * - iView::GetCamera()
 * 
 * Main users of this interface:
 * - csView
 * - iView
 */
struct iCamera : public virtual iBase
{
  SCF_INTERFACE(iCamera, 2,1,2);
  /**
   * Create a clone of this camera. Note that the array of listeners
   * is not cloned.
   */
  virtual csPtr<iCamera> Clone () const = 0;

  /**
   * Get the scene node that this object represents.
   * \todo iCamera doesn't yet support iMovable so scene nodes are not
   * properly working yet.
   */
  virtual iSceneNode* QuerySceneNode () = 0;

  /// Return the FOV (field of view) in pixels
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual int GetFOV () const = 0;
  /// Return the inverse flield of view (1/FOV) in pixels
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetInvFOV () const = 0;
  /// Return the FOV (field of view) in degrees.
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetFOVAngle () const = 0;

  /**
   * Set the FOV in pixels. 'fov' is the desired FOV in pixels. 'width' is
   * the display width, also in pixels.
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual void SetFOV (int fov, int width) = 0;
  /**
   * Set the FOV in degrees. 'fov' is the desired FOV in degrees. 'width' is
   * the display width in pixels.
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual void SetFOVAngle (float fov, int width) = 0;

  /**
   * Set the X shift amount. The parameter specified the desired X coordinate
   * on screen of the projection center of the camera.
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetShiftX () const = 0;
  /**
   * Set the Y shift amount. The parameter specified the desired Y coordinate
   * on screen of the projection center of the camera.
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual float GetShiftY () const = 0;
  /**
   * Set the shift amount. The parameter specified the desired projection
   * center of the camera on screen.
   */
  CS_DEPRECATED_METHOD_MSG("Use iPerspectiveCamera instead")
  virtual void SetPerspectiveCenter (float x, float y) = 0;

  /**
   * Get the transform corresponding to this camera. In this transform,
   * 'other' is world space and 'this' is camera space.
   * WARNING! It is illegal to directly assign to the given transform
   * in order to modify it. To change the entire transform you have
   * to use SetTransform(). Note that it is legal to modify the
   * returned transform otherwise. Just do not assign to it.
   */
  virtual csOrthoTransform& GetTransform () = 0;

  /// 'const' version of GetTransform ()
  virtual const csOrthoTransform& GetTransform () const = 0;

  /**
   * Set the transform corresponding to this camera. In this transform,
   * 'other' is world space and 'this' is camera space.
   */
  virtual void SetTransform (const csOrthoTransform& tr) = 0;

  /**
   * Moves the camera a relative amount in world coordinates.
   * If 'cd' is true then collision detection with objects and things
   * inside the sector is active. Otherwise you can walk through objects
   * (but portals will still be correctly checked).
   */
  virtual void MoveWorld (const csVector3& v, bool cd = true) = 0;
  /**
   * Moves the camera a relative amount in camera coordinates.
   */
  virtual void Move (const csVector3& v, bool cd = true) = 0;
  /**
   * Moves the camera a relative amount in world coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveWorldUnrestricted (const csVector3& v) = 0;
  /**
   * Moves the camera a relative amount in camera coordinates,
   * ignoring portals and walls. This is used by the wireframe
   * class. In general this is useful by any camera model that
   * doesn't want to restrict its movement by portals and
   * sector boundaries.
   */
  virtual void MoveUnrestricted (const csVector3& v) = 0;

  /// Get the current sector.
  virtual iSector* GetSector () const = 0;
  /// Move to another sector.
  virtual void SetSector (iSector*) = 0;

  /**
   * Eliminate roundoff error by snapping the camera orientation to a
   * grid of density n
   */
  virtual void Correct (int n) = 0;

  /// Return true if space is mirrored.
  virtual bool IsMirrored () const = 0;
  /// Set mirrored state.
  virtual void SetMirrored (bool m) = 0;

  /**
   * Get the 3D far plane that should be used to clip all geometry.
   * If this function returns 0 no far clipping is required.
   * Otherwise it must be used to clip the object before
   * drawing.
   */
  virtual csPlane3* GetFarPlane () const = 0;

  /**
   * Set the 3D far plane used to clip all geometry.
   * If the pointer is 0 then far plane clipping will be disabled.
   * Otherwise it will be enabled and the plane will be copied (so you
   * can free or reuse the pointer you give here). Note that the far-plane
   * will cull away geometry which is on the negative side of the plane
   * (with csPlane3::Classify() function).
   */
  virtual void SetFarPlane (csPlane3* fp) = 0;

  /**
   * Get the camera number. This number is changed for every new camera
   * instance and it is also updated whenever the camera transformation
   * changes. This number can be used to cache camera vertex arrays, for
   * example.
   */
  virtual long GetCameraNumber () const = 0;

  /// Calculate perspective corrected point for this camera.
  virtual csVector2 Perspective (const csVector3& v) const = 0;
  /// Calculate inverse perspective corrected point for this camera.
  virtual csVector3 InvPerspective (const csVector2& p, float z) const = 0;

  /**
   * If the hit-only-portals flag is true then only portals will be
   * checked with the 'MoveWorld()' function. This is a lot faster
   * but it does mean that you will have to do collision detection with
   * non-portal polygons using another technique. The default for this
   * flag is true.
   */
  virtual void OnlyPortals (bool hop) = 0;

  /// Get the hit-only-portals flag.
  virtual bool GetOnlyPortals () = 0;

  /// Add a listener to this camera.
  virtual void AddCameraSectorListener (iCameraSectorListener* listener) = 0;
  /// Remove a listener from this camera.
  virtual void RemoveCameraSectorListener (iCameraSectorListener* listener) = 0;
  
  /// Get the projection matrix for this camera
  virtual const CS::Math::Matrix4& GetProjectionMatrix (int viewWidth,
    int viewHeight) = 0;
  
  /**
   * Return the planes limiting the visible volume (as specified by the
   * projection). The returned planes are in camera space.
   */
  virtual const csPlane3* GetVisibleVolume (size_t& num) = 0;
};

struct iPerspectiveCamera : public virtual iBase
{
  SCF_INTERFACE(iPerspectiveCamera, 1, 0, 0);
  
  /// Get the iCamera interface for this camera.
  virtual iCamera* GetCamera() = 0;
  
  virtual int GetFOV () const = 0;
  /// Return the inverse flield of view (1/FOV) in pixels
  virtual float GetInvFOV () const = 0;
  /// Return the FOV (field of view) in degrees.
  virtual float GetFOVAngle () const = 0;
  virtual void SetFOV (int fov, int width) = 0;
  
  /**
   * Set the FOV in degrees. 'fov' is the desired FOV in degrees. 'width' is
   * the display width in pixels.
   */
  virtual void SetFOVAngle (float fov, int width) = 0;

  /**
   * Set the X shift amount. The parameter specified the desired X coordinate
   * on screen of the projection center of the camera.
   */
  virtual float GetShiftX () const = 0;
  /**
   * Set the Y shift amount. The parameter specified the desired Y coordinate
   * on screen of the projection center of the camera.
   */
  virtual float GetShiftY () const = 0;
  /**
   * Set the shift amount. The parameter specified the desired projection
   * center of the camera on screen.
   */
  virtual void SetPerspectiveCenter (float x, float y) = 0;
};

struct iCustomMatrixCamera : public virtual iBase
{
  SCF_INTERFACE(iCustomMatrixCamera, 1, 0, 0);
  
  /// Get the iCamera interface for this camera.
  virtual iCamera* GetCamera() = 0;
  
  virtual void SetProjectionMatrix (const CS::Math::Matrix4& mat) = 0;
};

/** @} */

#endif // __CS_IENGINE_CAMERA_H__

