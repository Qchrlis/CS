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
#ifndef __CS_ASSIMPLOADER_H__
#define __CS_ASSIMPLOADER_H__

#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "imap/reader.h"
#include "imap/modelload.h"

#include "assimp/assimp.hpp"      // C++ importer interface
#include "assimp/aiScene.h"       // Output data structure
#include "assimp/aiPostProcess.h" // Post processing flags

struct iEngine;
struct iImageIO;
struct iMaterialWrapper;
struct iObjectRegistry;
struct iPluginManager;
struct iReporter;
struct iTextureWrapper;

namespace CS {
namespace Mesh {
  struct iAnimatedMeshFactory;
}
}

CS_PLUGIN_NAMESPACE_BEGIN(AssimpLoader)
{

/**
 * Open Asset Import Library loader for Crystal Space
 */

struct AnimeshData
{
  csRef<CS::Mesh::iAnimatedMeshFactory> factory;
  csDirtyAccessArray<float> vertices;
  csDirtyAccessArray<float> texels;
  csDirtyAccessArray<float> normals;
  csDirtyAccessArray<float> tangents;
  csDirtyAccessArray<float> binormals;
  csDirtyAccessArray<float> colors;
  bool hasNormals;
  bool hasTexels;
  bool hasTangents;
  bool hasColors;

  AnimeshData ()
  : hasNormals (false), hasTexels (false), hasTangents (false), hasColors (false)
  {}
};

class AssimpLoader : 
  public scfImplementation3<AssimpLoader,
			    iBinaryLoaderPlugin,
			    iModelLoader,
			    iComponent>
{
public:

  AssimpLoader (iBase*);
  virtual ~AssimpLoader ();

  //-- iComponent
  virtual bool Initialize (iObjectRegistry *object_reg);

  //-- iBinaryLoaderPlugin
  virtual bool IsThreadSafe () { return true; }
  virtual csPtr<iBase> Parse (iDataBuffer* buf, iStreamSource*,
    iLoaderContext* ldr_context, iBase* context, iStringArray*);

  //-- iModelLoader
  virtual iMeshFactoryWrapper* Load (const char* factname, const char* filename);
  virtual iMeshFactoryWrapper* Load (const char* factname, iDataBuffer* buffer);
  virtual bool IsRecognized (const char* filename);
  virtual bool IsRecognized (iDataBuffer* buffer);

 private:
  void ImportScene ();

  iTextureWrapper* FindTexture (const char* filename);
  iTextureWrapper* LoadTexture (iDataBuffer* buffer, const char* filename);

  void ImportTexture (aiTexture* texture, size_t index);
  void ImportMaterial (aiMaterial* material, size_t index);

  void ImportGenmesh (aiNode* node);
  void ImportGenmeshSubMesh (iGeneralFactoryState* gmstate, aiNode* node);

  void ImportAnimesh (aiNode* node);
  void PreProcessAnimeshSubMesh (AnimeshData* animeshData, aiNode* node);
  void ImportAnimeshSubMesh (AnimeshData* animeshData, aiNode* node);
  void ImportAnimation (aiAnimation* animation);

private:
  iObjectRegistry* object_reg;

  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iTextureManager> textureManager;
  csRef<iLoaderContext> loaderContext;
  csRef<iImageIO> imageLoader;

  csRefArray<iTextureWrapper> textures;
  csRefArray<iMaterialWrapper> materials;

  const aiScene* scene;
  unsigned int importFlags;

  csRef<iMeshFactoryWrapper> firstMesh;
};

}
CS_PLUGIN_NAMESPACE_END(AssimpLoader)

#endif // __CS_ASSIMPLOADER_H__