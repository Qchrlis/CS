/*
    Copyright (C) 2000 by W.C.A. Wijngaards
  
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

#ifndef __CS_MATERIAL_H__
#define __CS_MATERIAL_H__

#include "csutil/cscolor.h"
#include "csobject/csobject.h"
#include "csobject/nobjvec.h"
#include "imater.h"

class csTextureHandle;
struct iTextureManager;


/**
 * A material class.
 */

class csMaterial : public iMaterial
{
private:
  /// flat shading color
  csColor flat_color;
  /// the texture of the material (can be NULL)
  csTextureHandle *texture;

  /// The diffuse reflection value of the material
  float diffuse;
  /// The ambient lighting of the material
  float ambient;
  /// The reflectiveness of the material
  float reflection;


public:
  /**
   * create an empty material
   */
  csMaterial ();
  /**
   * create a material with only the texture given.
   */
  csMaterial (csTextureHandle *txt);

  /**
   * destroy material
   */
  virtual ~csMaterial ();


  /// Get the flat shading color
  inline const csColor& GetFlatColor () const {return flat_color;}
  /// Set the flat shading color
  inline void SetFlatColor (const csColor& col) {flat_color = col;}

  /// Get the texture (if none NULL is returned)
  inline csTextureHandle *GetTextureHandle () const {return texture;}
  /// Set the texture (pass NULL to set no texture)
  inline void SetTextureHandle (csTextureHandle *tex) {texture = tex;}

  /// Get diffuse reflection constant for the material
  inline float GetDiffuse () const { return diffuse; }
  /// Set diffuse reflection constant for the material
  inline void SetDiffuse (float val) { diffuse = val; }

  /// Get ambient lighting for the material
  inline float GetAmbient () const { return ambient; }
  /// Set ambient lighting for the material
  inline void SetAmbient (float val) { ambient = val; }

  /// Get reflection of the material
  inline float GetReflection () const { return reflection; }
  /// Set reflection of the material
  inline void SetReflection (float val) { reflection = val; }

  //--------------------- iMaterial implementation ---------------------
  DECLARE_IBASE;
  ///
  virtual iTextureHandle* GetTexture ();
};

/**
 * csMaterialHandle represents a texture and its link
 * to the iMaterialHandle as returned by iTextureManager.
 */
class csMaterialHandle : public csObject
{
private:
  /// The corresponding iMaterial.
  iMaterial* material;
  /// The handle as returned by iTextureManager.
  iMaterialHandle* handle;

public:
  /// Construct a material handle given a material.
  csMaterialHandle (iMaterial* Image);

  /**
   * Construct a csMaterialHandle from a pre-registered AND prepared material 
   * handle. The engine takes over responsibility for destroying the material
   * handle. To prevent this IncRef () the material handle.
   */
  csMaterialHandle (iMaterialHandle *ith);

  /// Copy constructor
  csMaterialHandle (csMaterialHandle &th);
  /// Release material handle
  virtual ~csMaterialHandle ();

  /// Get the material handle.
  iMaterialHandle* GetMaterialHandle () { return handle; }

  /// Change the base material
  void SetMaterial (iMaterial* material);
  /// Get the material.
  iMaterial* GetMaterial () { return material; }

  /// Register the material with the texture manager
  void Register (iTextureManager *txtmng);

  CSOBJTYPE;
};

/**
 * This class is used to hold a list of materials.
 */
class csMaterialList : public csNamedObjVector
{
public:
  /// Initialize the array
  csMaterialList () : csNamedObjVector (16, 16)
  { }
  /// Destroy every material in the list
  virtual ~csMaterialList ();

  /// Create a new material.
  csMaterialHandle* NewMaterial (iMaterial* material);

  /**
   * Create a engine wrapper for a pre-prepared iTextureHandle
   * The handle will be IncRefed.
   */
  csMaterialHandle* NewMaterial (iMaterialHandle *ith);

  /// Return material by index
  csMaterialHandle *Get (int idx)
  { return (csMaterialHandle *)csNamedObjVector::Get (idx); }

  /// Find a material by name
  csMaterialHandle *FindByName (const char* iName)
  { return (csMaterialHandle *)csNamedObjVector::FindByName (iName); }
};

#endif // __CS_MATERIAL_H__
