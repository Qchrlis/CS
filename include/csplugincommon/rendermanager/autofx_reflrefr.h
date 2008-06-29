/*
    Copyright (C) 2008 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__

#include "csplugincommon/rendermanager/posteffects.h"

namespace CS
{
  namespace RenderManager
  {

    /**
     * Class for automatic plane reflection/refraction textures.
     *
     * Usage: Functor for TraverseUsedSVSets
     */
    template<typename RenderTree, typename ContextSetup>
    class AutoFX_ReflectRefract
    {
    public:
      struct PersistentData
      {
        CS::ShaderVarStringID svTexPlaneRefl;
        CS::ShaderVarStringID svTexPlaneRefr;
        CS::ShaderVarStringID svTexPlaneReflDepth;
        CS::ShaderVarStringID svTexPlaneRefrDepth;
        
        CS::ShaderVarStringID svPlaneRefl;
        CS::ShaderVarStringID svClipPlaneReflRefr;
        
        CS::ShaderVarStringID svReflXform;
        csRef<csShaderVariable> reflXformSV;
        bool screenFlipped;
        float mappingStretch;
        
        uint currentFrame;
        uint updatesThisFrame;
      
        TextureCacheT<CS::Utility::ResourceCache::ReuseIfOnlyOneRef> texCache;
        TextureCacheT<CS::Utility::ResourceCache::ReuseIfOnlyOneRef> texCacheDepth;
        struct ReflectRefractSVs
        {
          csTicks lastUpdate;
          uint lastUpdateFrame;
          csTransform lastCamera;
        
          csRef<csShaderVariable> reflectSV;
          csRef<csShaderVariable> refractSV;
          csRef<csShaderVariable> reflectDepthSV;
          csRef<csShaderVariable> refractDepthSV;
          
          csRef<iShaderVariableContext> clipPlaneReflContext;
          csRef<csShaderVariable> clipPlaneReflSV;
          csRef<iShaderVariableContext> clipPlaneRefrContext;
          csRef<csShaderVariable> clipPlaneRefrSV;
          
          ReflectRefractSVs() : lastUpdate (0), lastUpdateFrame (~0) {}
        };
        typedef csHash<ReflectRefractSVs, csPtrKey<iMeshWrapper> > ReflRefrCache;
        ReflRefrCache reflRefrCache;
          
        int resolutionReduceRefl;
        int resolutionReduceRefr;
        uint texUpdateInterval;
        uint maxUpdatesPerFrame;
        float cameraChangeThresh;
        
        uint dbgReflRefrTex;
        
        PersistentData() :
          currentFrame (0),
	  texCache (csimg2D, "rgb8",  // @@@ FIXME: Use same format as main view ...
	    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	     | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP,
	    "target", 0,
	    CS::Utility::ResourceCache::ReuseIfOnlyOneRef ()),
	  texCacheDepth (csimg2D, "d32",
	    CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP
	     | CS_TEXTURE_NPOTS | CS_TEXTURE_CLAMP | CS_TEXTURE_SCALE_UP,
	    "target", 0,
	    CS::Utility::ResourceCache::ReuseIfOnlyOneRef ())
	{
	}
        
	void Initialize (iObjectRegistry* objReg,
			 RenderTreeBase::DebugPersistent& dbgPersist,
			 PostEffectManager* postEffects)
	{
	  dbgReflRefrTex = dbgPersist.RegisterDebugFlag ("textures.reflrefr");
	
	  csRef<iShaderManager> shaderManager =
	    csQueryRegistry<iShaderManager> (objReg);
	  
	  iShaderVarStringSet* strings = shaderManager->GetSVNameStringset();
	  svTexPlaneRefl = strings->Request ("tex plane reflect");
	  svTexPlaneRefr = strings->Request ("tex plane refract");
	  svTexPlaneReflDepth = strings->Request ("tex plane reflect depth");
	  svTexPlaneRefrDepth = strings->Request ("tex plane refract depth");
	  
	  svPlaneRefl = strings->Request ("plane reflection");
	  svClipPlaneReflRefr = strings->Request ("clip plane reflection");
	  
	  csConfigAccess config (objReg);
	  resolutionReduceRefl = config->GetInt (
	    "RenderManager.Reflections.Downsample", 1);
	  resolutionReduceRefr = config->GetInt (
	    "RenderManager.Refractions.Downsample", resolutionReduceRefl);
	  texUpdateInterval = config->GetInt (
	    "RenderManager.Reflections.UpdateInterval", 0);
	  maxUpdatesPerFrame = config->GetInt (
	    "RenderManager.Reflections.MaxUpdatesPerFrame", 0);
	  mappingStretch = config->GetFloat (
	    "RenderManager.Reflections.MappingStretch", 1.0f);
	  cameraChangeThresh = config->GetFloat (
	    "RenderManager.Reflections.CameraChangeThreshold", 0.01f);
	  
	  svReflXform = strings->Request ("reflection coord xform");
	  reflXformSV.AttachNew (new csShaderVariable (svReflXform));
	  screenFlipped = postEffects ? postEffects->ScreenSpaceYFlipped() : false;
	    
	  csRef<iGraphics3D> g3d = csQueryRegistry<iGraphics3D> (objReg);
	  texCache.SetG3D (g3d);
	  texCacheDepth.SetG3D (g3d);
	}
      
	void UpdateNewFrame ()
	{
	  csTicks currentTicks = csGetTicks ();
	  typename ReflRefrCache::GlobalIterator reflRefrIt (
	    reflRefrCache.GetIterator ());
	  while (reflRefrIt.HasNext())
	  {
	    ReflectRefractSVs& meshReflectRefract = reflRefrIt.NextNoAdvance();
	    // Don't remove if update interval hasn't passed yet
	    if ((texUpdateInterval > 0)
	      && ((currentTicks - meshReflectRefract.lastUpdate) <=
	        texUpdateInterval))
	    {
	      reflRefrIt.Next();
	      continue;
	    }
	    // Don't remove if not in line for round-robin update
	    if ((maxUpdatesPerFrame > 0)
	      && (currentFrame - meshReflectRefract.lastUpdateFrame < 
	        ((reflRefrCache.GetSize()+maxUpdatesPerFrame-1) / maxUpdatesPerFrame)))
	    {
	      reflRefrIt.Next();
	      continue;
	    }
	    reflRefrCache.DeleteElement (reflRefrIt);
	  }
	  
	  currentFrame++;
	  updatesThisFrame = 0;
	
	  texCache.AdvanceFrame (currentTicks);
	  texCacheDepth.AdvanceFrame (currentTicks);
	}
      };
      
      AutoFX_ReflectRefract (PersistentData& persist,
        ContextSetup& contextFunction) : persist (persist),
        contextFunction (contextFunction)
      {}
    
      void operator() (typename RenderTree::MeshNode* node,
                       size_t layer,
                       typename RenderTree::MeshNode::SingleMesh& mesh,
                       const csBitArray& names)
      {
        // Check if reflection or refraction is needed
        if (!names.IsBitSetTolerant (persist.svTexPlaneRefl)
            && !names.IsBitSetTolerant (persist.svTexPlaneRefr))
          return;
        
        /* @@@ NOTE:The same mesh, if rendered in multiple views, will get
           the same reflection/refraction texture. This is wrong.
           However, ignore for now, until someone complains */
        typename PersistentData::ReflectRefractSVs& meshReflectRefract =
          persist.reflRefrCache.GetOrCreate (mesh.meshWrapper);
        
	typename RenderTree::ContextNode& context = node->owner;
	RenderTree& renderTree = context.owner;
        RenderView* rview = context.renderView;
        
	csShaderVariableStack localStack;
	context.svArrays.SetupSVStack (localStack, layer, mesh.contextLocalId);
	
	csTicks currentTicks = csGetTicks ();
	bool forceUpdate = (persist.currentFrame != meshReflectRefract.lastUpdateFrame);
	forceUpdate &= (persist.texUpdateInterval == 0)
	  || ((currentTicks - meshReflectRefract.lastUpdate) >
	    persist.texUpdateInterval);
	  
	forceUpdate &= (persist.maxUpdatesPerFrame == 0)
	  || (persist.currentFrame - meshReflectRefract.lastUpdateFrame >= 
	    ((persist.reflRefrCache.GetSize()+persist.maxUpdatesPerFrame-1)
	      / persist.maxUpdatesPerFrame));
	
	forceUpdate &= (persist.maxUpdatesPerFrame == 0)
	  || (persist.updatesThisFrame < persist.maxUpdatesPerFrame);
	  
	
        iCamera* cam = rview->GetCamera();
	/* Compute a difference for the camera between the last render;
	   if bigger than a certain threshold, rerender (whether it's the
	   reflections's turn or not) */
	if (!forceUpdate && (persist.cameraChangeThresh > 0))
	{
	  const csTransform& camTrans = cam->GetTransform();
	  float cameraDiff;
	  
	  cameraDiff = fabsf (camTrans.GetO2T().m11
	    - meshReflectRefract.lastCamera.GetO2T().m11);
	  cameraDiff += fabsf (camTrans.GetO2T().m12
	    - meshReflectRefract.lastCamera.GetO2T().m12);
	  cameraDiff += fabsf (camTrans.GetO2T().m13
	    - meshReflectRefract.lastCamera.GetO2T().m13);
	    
	  cameraDiff += fabsf (camTrans.GetO2T().m21
	    - meshReflectRefract.lastCamera.GetO2T().m21);
	  cameraDiff += fabsf (camTrans.GetO2T().m22
	    - meshReflectRefract.lastCamera.GetO2T().m22);
	  cameraDiff += fabsf (camTrans.GetO2T().m23
	    - meshReflectRefract.lastCamera.GetO2T().m23);
	    
	  cameraDiff += fabsf (camTrans.GetO2T().m31
	    - meshReflectRefract.lastCamera.GetO2T().m31);
	  cameraDiff += fabsf (camTrans.GetO2T().m32
	    - meshReflectRefract.lastCamera.GetO2T().m32);
	  cameraDiff += fabsf (camTrans.GetO2T().m33
	    - meshReflectRefract.lastCamera.GetO2T().m33);
	
	  cameraDiff += fabsf (camTrans.GetO2TTranslation().x
	    - meshReflectRefract.lastCamera.GetO2TTranslation().x);
	  cameraDiff += fabsf (camTrans.GetO2TTranslation().y
	    - meshReflectRefract.lastCamera.GetO2TTranslation().y);
	  cameraDiff += fabsf (camTrans.GetO2TTranslation().z
	    - meshReflectRefract.lastCamera.GetO2TTranslation().z);
	    
	  forceUpdate = cameraDiff >= persist.cameraChangeThresh;
	}

        bool usesReflTex = names.IsBitSetTolerant (persist.svTexPlaneRefl);
        bool usesReflDepthTex = names.IsBitSetTolerant (persist.svTexPlaneReflDepth);
        bool needReflTex = (usesReflTex
            && (!meshReflectRefract.reflectSV.IsValid() || forceUpdate))
          || (usesReflDepthTex
            && (!meshReflectRefract.reflectDepthSV.IsValid() || forceUpdate));
          
        bool usesRefrTex = names.IsBitSetTolerant (persist.svTexPlaneRefr);
        bool usesRefrDepthTex = names.IsBitSetTolerant (persist.svTexPlaneRefrDepth);
        bool needRefrTex = (usesRefrTex 
            && (!meshReflectRefract.refractSV.IsValid() || forceUpdate))
          || (usesRefrDepthTex 
            && (!meshReflectRefract.refractDepthSV.IsValid() || forceUpdate));
        
        int renderW = 512, renderH = 512;
        context.GetTargetDimensions (renderW, renderH);
        int txt_w_refl = renderW >> persist.resolutionReduceRefl;
        int txt_h_refl = renderH >> persist.resolutionReduceRefl;
        int txt_w_refr = renderW >> persist.resolutionReduceRefr;
        int txt_h_refr = renderH >> persist.resolutionReduceRefr;
        
        // @@@ Kludge: don't render nested reflections
        bool doRender = localStack[persist.svClipPlaneReflRefr] == 0;
        
        csPlane3 reflRefrPlane, reflRefrPlane_cam;
        csBox2 clipBoxRefl, clipBoxRefr;
        if ((needReflTex || needRefrTex) && doRender)
        {
          // Compute reflect/refract plane
          if ((localStack.GetSize() > persist.svPlaneRefl)
              && (localStack[persist.svPlaneRefl] != 0))
          {
            // Grab reflection plane from a SV
            csShaderVariable* planeSV = localStack[persist.svPlaneRefl];
            csVector4 v;
            planeSV->GetValue (v);
            reflRefrPlane.Set (v.x, v.y, v.z, v.w);
          }
          else
          {
	    /* Guess reflection plane from mesh bbox:
	      Take smallest dimension of object space bounding box, make that
	      the durection of reflect plane */
	    const csBox3& objBB =
	      mesh.meshWrapper->GetMeshObject()->GetObjectModel()->GetObjectBoundingBox();
	    const csVector3& bbSize = objBB.GetSize();
	    
	    int axis;
	    if ((bbSize[0] < bbSize[1]) && (bbSize[0] < bbSize[2]))
	    {
	      axis = 0;
	    }
	    else if (bbSize[1] < bbSize[2])
	    {
	      axis = 1;
	    }
	    else
	    {
	      axis = 2;
	    }
	    
	    csVector3 planeNorm (0);
	    planeNorm[axis] = -1;
            reflRefrPlane.Set (planeNorm, 0);
            reflRefrPlane.SetOrigin (objBB.GetCenter());
	  }
	  
          reflRefrPlane =
            mesh.meshWrapper->GetMovable()->GetFullTransform().This2Other (reflRefrPlane);
          
	  reflRefrPlane_cam = 
	    cam->GetTransform().Other2This (reflRefrPlane);
	  doRender = reflRefrPlane.Classify (cam->GetTransform().GetOrigin()) <= 0;
          
	  meshReflectRefract.clipPlaneReflSV.AttachNew (new csShaderVariable (
	    persist.svClipPlaneReflRefr));
	  meshReflectRefract.clipPlaneReflSV->SetValue (csVector4 (
	    -reflRefrPlane.A(),
	    -reflRefrPlane.B(),
	    -reflRefrPlane.C(),
	    -reflRefrPlane.D()));
	  meshReflectRefract.clipPlaneReflContext.AttachNew (
	    new csShaderVariableContext);
	  meshReflectRefract.clipPlaneReflContext->AddVariable (
	    meshReflectRefract.clipPlaneReflSV);
            
	  meshReflectRefract.clipPlaneRefrSV.AttachNew (new csShaderVariable (
	    persist.svClipPlaneReflRefr));
	  meshReflectRefract.clipPlaneRefrSV->SetValue (csVector4 (
	    reflRefrPlane.A(),
	    reflRefrPlane.B(),
	    reflRefrPlane.C(),
	    reflRefrPlane.D()));
	  meshReflectRefract.clipPlaneRefrContext.AttachNew (
	    new csShaderVariableContext);
	  meshReflectRefract.clipPlaneRefrContext->AddVariable (
	    meshReflectRefract.clipPlaneRefrSV);
	    
	  // Compute screen space bounding box
	  {
	    const csBox3& objBB_world =  mesh.meshWrapper->GetWorldBoundingBox();
	    float min_z, max_z;
	    objBB_world.ProjectBox (cam->GetTransform(),
	      cam->GetProjectionMatrix(), clipBoxRefl, min_z, max_z,
	      txt_w_refl, txt_h_refl);
	    objBB_world.ProjectBox (cam->GetTransform(),
	      cam->GetProjectionMatrix(), clipBoxRefr, min_z, max_z,
	      txt_w_refr, txt_h_refr);
	  }
	  
	  /* To reduce bleeding from filtering, fuzz the reflection mapping to 
	      be actually stretched out a pixel at the corners */
	  if ((persist.resolutionReduceRefr > 0)
	      || (persist.resolutionReduceRefl > 0))
	  {
	    /* Since reflection + refraction share the same TC transform, 
	      compute the 'fuzz' from the smaller one. This can lead to 
	      distortions when the resolutions differ a lot ... but hopefully
	      doesn't happen often */
	    csBox2 dstBox;
	    int txt_w, txt_h;
	    if (persist.resolutionReduceRefr > persist.resolutionReduceRefl)
	    {
	      dstBox = clipBoxRefr;
	      txt_w = txt_w_refr;
	      txt_h = txt_h_refr;
	    }
	    else
	    {
	      dstBox = clipBoxRefl;
	      txt_w = txt_w_refl;
	      txt_h = txt_h_refl;
	    }
	    
	    float oldMinY = dstBox.MinY();
	    dstBox.SetMin (1, txt_h-dstBox.MaxY());
	    dstBox.SetMax (1, txt_h-oldMinY);
	    dstBox *= csBox2 (0, 0, txt_w, txt_h);
	    csBox2 useBox (dstBox.MinX() + persist.mappingStretch,
	      dstBox.MinY() + persist.mappingStretch,
	      dstBox.MaxX() - persist.mappingStretch,
	      dstBox.MaxY() - persist.mappingStretch);
	    float dw = dstBox.MaxX() - dstBox.MinX();
	    float dh = dstBox.MaxY() - dstBox.MinY();
	    float sw = useBox.MaxX() - useBox.MinX();
	    float sh = useBox.MaxY() - useBox.MinY();
	    float scaleX = sw/dw;
	    float scaleY = sh/dh;
	    float cX1 = useBox.GetCenter().x/(float)txt_w;
	    float cY1 = useBox.GetCenter().y/(float)txt_h;
	    float cX2 = dstBox.GetCenter().x/(float)txt_w;
	    float cY2 = dstBox.GetCenter().y/(float)txt_h;
	    float dX = (0.5f-cX1)*scaleX + cX2;
	    float dY = (0.5f-cY1)*scaleY + cY2;
	    
	    persist.reflXformSV->SetValue (csVector4 (0.5f * scaleX,
	      (persist.screenFlipped ? 0.5f : -0.5f) * scaleY,
	      dX, dY));
	  }
	  else
	  {
	    persist.reflXformSV->SetValue (csVector4 (0.5f,
	      (persist.screenFlipped ? 0.5f : -0.5f),
	      0.5f, 0.5f));
	  }
        }
        
        typename RenderTree::ContextNode* reflCtx = 0;
        typename RenderTree::ContextNode* refrCtx = 0;
        
        if (usesReflTex || usesReflDepthTex)
        {
	  csRef<csShaderVariable> svReflection;
	  csRef<csShaderVariable> svReflectionDepth;
	  
	  if (needReflTex && doRender)
	  {
	    // Compute reflection view
	    iCamera* cam = rview->GetCamera();
	    // Create a new view
	    csRef<CS::RenderManager::RenderView> reflView;
	    csRef<iCamera> newCam (cam->Clone());
	    iCamera* inewcam = newCam;
    #include "csutil/custom_new_disable.h"
	    reflView.AttachNew (
	      new (renderTree.GetPersistentData().renderViewPool) RenderView (
	        *rview));
    #include "csutil/custom_new_enable.h"
            reflView->SetCamera (inewcam);
            reflView->GetMeshFilter().AddFilterMesh (mesh.meshWrapper);
            reflView->SetClipPlane (reflRefrPlane_cam.Inverse ());
	    
	    // Change the camera transform to be a reflection across reflRefrPlane
	    csReversibleTransform reflection (csTransform::GetReflect (reflRefrPlane));
	    csTransform reflection_cam (csTransform::GetReflect (reflRefrPlane_cam));
	    const csTransform& world2cam (inewcam->GetTransform());
	    csTransform reflected;
	    reflected.SetO2T (reflection_cam.GetO2T() * world2cam.GetO2T());
	    reflected.SetOrigin (reflection.Other2This (world2cam.GetOrigin()));
	    inewcam->SetTransform (reflected);
	    inewcam->SetMirrored (true);
	    
	    csRef<iTextureHandle> tex;
	    csRef<iTextureHandle> texDepth;
	    if (usesReflTex)
	    {
	      tex = 
	        persist.texCache.QueryUnusedTexture (txt_w_refl, txt_h_refl, 0);
	    }
	    if (usesReflDepthTex)
	    {
	      texDepth = 
	        persist.texCacheDepth.QueryUnusedTexture (txt_w_refl, txt_h_refl, 0);
	    }
	    
	    // Set up context for reflection, clipped to plane
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBoxRefl));
	    reflView->SetClipper (newView);
  
	    reflCtx = renderTree.CreateContext (reflView);
	    reflCtx->renderTargets[rtaColor0].texHandle = tex;
	    reflCtx->renderTargets[rtaDepth].texHandle = texDepth;
	    reflCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
	    reflCtx->shadervars = meshReflectRefract.clipPlaneReflContext;
	      
	    svReflection.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefl));
	    svReflection->SetValue (tex);
	    meshReflectRefract.reflectSV = svReflection;
	    
	    svReflectionDepth.AttachNew (new csShaderVariable (
	      persist.svTexPlaneReflDepth));
	    svReflectionDepth->SetValue (texDepth);
	    meshReflectRefract.reflectDepthSV = svReflectionDepth;
	    
	    if (renderTree.IsDebugFlagEnabled (persist.dbgReflRefrTex))
	    {
	      float dbgAspect = (float)txt_w_refl/(float)txt_h_refl;
	      renderTree.AddDebugTexture (tex, dbgAspect);
	      renderTree.AddDebugTexture (texDepth, dbgAspect);
	    }
	  }
	  else
	  {
	    svReflection = meshReflectRefract.reflectSV;
	    svReflectionDepth = meshReflectRefract.reflectDepthSV;
	    
	    if (renderTree.IsDebugFlagEnabled (persist.dbgReflRefrTex)
	        && doRender)
	    {
	      float dbgAspect = (float)txt_w_refl/(float)txt_h_refl;
	      iTextureHandle* texh = 0;
	      if (svReflection.IsValid()) svReflection->GetValue (texh);
	      renderTree.AddDebugTexture (texh, dbgAspect);
	      texh = 0;
	      if (svReflectionDepth.IsValid()) svReflectionDepth->GetValue (texh);
	      renderTree.AddDebugTexture (texh, dbgAspect);
	    }
	  }
	  
	  // Attach reflection texture to mesh
	  localStack[persist.svTexPlaneRefl] = svReflection;
	  localStack[persist.svTexPlaneReflDepth] = svReflectionDepth;
	  localStack[persist.svReflXform] = persist.reflXformSV;
	}
        
        if (usesRefrTex || usesRefrDepthTex)
        {
	  csRef<csShaderVariable> svRefraction;
	  csRef<csShaderVariable> svRefractionDepth;
	  
	  if (needRefrTex && doRender)
	  {
	    // Set up context for refraction, clipped to plane
	    
	    // Create a new view
	    csRef<CS::RenderManager::RenderView> refrView;
    #include "csutil/custom_new_disable.h"
	    refrView.AttachNew (
	      new (renderTree.GetPersistentData().renderViewPool) RenderView (
		*rview));
    #include "csutil/custom_new_enable.h"
	    
	    csRef<iTextureHandle> tex;
	    csRef<iTextureHandle> texDepth;
	    if (usesRefrTex)
	    {
	      tex = 
	        persist.texCache.QueryUnusedTexture (txt_w_refr, txt_h_refr, 0);
	    }
	    if (usesRefrDepthTex)
	    {
	      texDepth = 
	        persist.texCacheDepth.QueryUnusedTexture (txt_w_refr, txt_h_refr, 0);
	    }
	    
	    // Set up context for reflection, clipped to plane
	    csRef<iClipper2D> newView;
	    newView.AttachNew (new csBoxClipper (clipBoxRefr));
	    refrView->SetClipper (newView);
            refrView->GetMeshFilter().AddFilterMesh (mesh.meshWrapper);
            refrView->SetClipPlane (reflRefrPlane_cam);
  
	    refrCtx = renderTree.CreateContext (refrView);
	    refrCtx->renderTargets[rtaColor0].texHandle = tex;
	    refrCtx->renderTargets[rtaDepth].texHandle = texDepth;
	    refrCtx->drawFlags = CSDRAW_CLEARSCREEN | CSDRAW_CLEARZBUFFER;
	    refrCtx->shadervars = meshReflectRefract.clipPlaneRefrContext;
	      
	    // Attach reflection texture to mesh
	    svRefraction.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefr));
	    svRefraction->SetValue (tex);
	    meshReflectRefract.refractSV = svRefraction;
	    
	    svRefractionDepth.AttachNew (new csShaderVariable (
	      persist.svTexPlaneRefrDepth));
	    svRefractionDepth->SetValue (texDepth);
	    meshReflectRefract.refractDepthSV = svRefractionDepth;
	    
	    if (renderTree.IsDebugFlagEnabled (persist.dbgReflRefrTex))
	    {
	      float dbgAspect = (float)txt_w_refr/(float)txt_h_refr;
	      renderTree.AddDebugTexture (tex, dbgAspect);
	      renderTree.AddDebugTexture (texDepth, dbgAspect);
	    }
	  }
  	  else
	  {
	    svRefraction = meshReflectRefract.refractSV;
	    svRefractionDepth = meshReflectRefract.refractDepthSV;
	    
	    if (renderTree.IsDebugFlagEnabled (persist.dbgReflRefrTex)
	        && doRender)
	    {
	      float dbgAspect = (float)txt_w_refl/(float)txt_h_refl;
	      iTextureHandle* texh = 0;
	      if (svRefraction.IsValid()) svRefraction->GetValue (texh);
	      renderTree.AddDebugTexture (texh, dbgAspect);
	      texh = 0;
	      if (svRefractionDepth.IsValid()) svRefractionDepth->GetValue (texh);
	      renderTree.AddDebugTexture (texh, dbgAspect);
	    }
	  }
	  
          // Attach refraction texture to mesh
	  localStack[persist.svTexPlaneRefr] = svRefraction;
	  localStack[persist.svTexPlaneRefrDepth] = svRefractionDepth;
	  localStack[persist.svReflXform] = persist.reflXformSV;
	}
	if ((needReflTex || needRefrTex) && doRender)
	{
	  meshReflectRefract.lastUpdate = currentTicks;
	  meshReflectRefract.lastUpdateFrame = persist.currentFrame;
	  meshReflectRefract.lastCamera = cam->GetTransform();
	  persist.updatesThisFrame++;
	}
	
        // Setup the new contexts
	if (reflCtx) contextFunction (*reflCtx);
	if (refrCtx) contextFunction (*refrCtx);
      }
    protected:
      PersistentData& persist;
      ContextSetup& contextFunction;
    };

  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_AUTOFX_REFLREFR_H__
