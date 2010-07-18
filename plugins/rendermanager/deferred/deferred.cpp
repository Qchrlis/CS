/*
    Copyright (C) 2010 by Joe Forte

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

#include "csplugincommon/rendermanager/renderview.h"
#include "csplugincommon/rendermanager/dependenttarget.h"
#include "csplugincommon/rendermanager/hdrhelper.h"
#include "csplugincommon/rendermanager/lightsetup.h"
#include "csplugincommon/rendermanager/operations.h"
#include "csplugincommon/rendermanager/portalsetup.h"
#include "csplugincommon/rendermanager/posteffects.h"
#include "csplugincommon/rendermanager/render.h"
#include "csplugincommon/rendermanager/renderlayers.h"
#include "csplugincommon/rendermanager/rendertree.h"
#include "csplugincommon/rendermanager/shadersetup.h"
#include "csplugincommon/rendermanager/standardsorter.h"
#include "csplugincommon/rendermanager/svsetup.h"
#include "csplugincommon/rendermanager/viscull.h"

#include "iengine.h"
#include "ivideo.h"
#include "ivaria/reporter.h"
#include "csutil/cfgacc.h"

#include "deferred.h"

using namespace CS::RenderManager;

CS_PLUGIN_NAMESPACE_BEGIN(RMDeferred)
{

SCF_IMPLEMENT_FACTORY(RMDeferred);

/**
 * Allocates a RenderView using placement new with the given memory pool.
 */
CS::RenderManager::RenderView *CreateNewRenderView(CS::RenderManager::RenderView::Pool &pool, iView *view)
{
  #include "csutil/custom_new_disable.h"

  RenderView *rview = new(pool) CS::RenderManager::RenderView (view);

  #include "csutil/custom_new_enable.h"

  return rview;
}

/**
 * Draws the given texture over the contents of the entire screen.
 */
void DrawFullscreenTexture(iTextureHandle *tex, iGraphics3D *graphics3D)
{
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

  int w, h;
  tex->GetRendererDimensions (w, h);

  graphics3D->SetZMode (CS_ZBUF_NONE);
  graphics3D->BeginDraw (CSDRAW_2DGRAPHICS | CSDRAW_CLEARSCREEN);
  graphics3D->DrawPixmap (tex, 
                          0, 
                          0,
                          graphics2D->GetWidth (),
                          graphics2D->GetHeight (),
                          0,
                          0,
                          w,
                          h,
                          0);
}

/**
 * Iterate over all lights within a context, call functor for each one.
 * Does not use any blocking at all.
 */
template<typename ContextType, typename Fn>
void ForEachLight(ContextType& context, Fn& fn)
{
  iLightList *list = context.sector->GetLights ();

  const int count = list->GetCount ();
  for (int i = 0; i < count; i++)
  {
    fn (list->Get (i));
  }
}

/**
 * Iterate over all mesh nodes within context, call functor for each opaque one.
 * Does not use any blocking at all.
 */
template<typename ContextType, typename Fn>
void ForEachOpaqueMeshNode(ContextType &context, Fn &fn)
{
  typedef CS::RenderManager::Implementation::NoOperationBlock<
    typename ContextType::TreeType::MeshNode*
  > noBlockType;

  typename ContextType::TreeType::MeshNodeTreeIteratorType it = 
    context.meshNodes.GetIterator ();

  noBlockType noBlock;
  // Helper object for calling function
  CS::RenderManager::Implementation::OperationCaller<
    Fn, 
    noBlockType,
    typename OperationTraits<Fn>::Ordering
  > caller (fn, noBlock);

  while (it.HasNext ())
  {
    typename ContextType::TreeType::MeshNode *node = it.Next ();
    CS_ASSERT_MSG("Null node encountered, should not be possible", node);

    if (!node->isTransparent)
      caller (node);
  }
}

/**
 * Iterate over all mesh nodes within context, call functor for each transparent one.
 * Does not use any blocking at all.
 */
template<typename ContextType, typename Fn>
void ForEachTransparentMeshNode(ContextType &context, Fn &fn)
{
  typedef CS::RenderManager::Implementation::NoOperationBlock<
    typename ContextType::TreeType::MeshNode*
  > noBlockType;

  typename ContextType::TreeType::MeshNodeTreeIteratorType it = 
    context.meshNodes.GetIterator ();

  noBlockType noBlock;
  // Helper object for calling function
  CS::RenderManager::Implementation::OperationCaller<
    Fn, 
    noBlockType,
    typename OperationTraits<Fn>::Ordering
  > caller (fn, noBlock);

  while (it.HasNext ())
  {
    typename ContextType::TreeType::MeshNode *node = it.Next ();
    CS_ASSERT_MSG("Null node encountered, should not be possible", node);

    if (node->isTransparent)
      caller (node);
  }
}

/**
 * Renderer for multiple contexts where all objects are drawn
 * to a single render target.
 *
 * Usage: 
 *  1. Attach desired render targets.
 *  2. Using a reverse iterator, iterate over all contexts.
 *  3. Call FinishDraw()
 *
 * Example:
 * \code
 * // ... contexts setup etc. ...
 *
 * {
 *   DeferredTreeRenderer<RenderTree, UpdateFunctor> 
 *     render (renderView->GetGraphics3D (), shaderManager);
 *
 *   ForEachContextReverse (renderTree, render);
 *   g3d->FinishDraw();
 * }
 *
 * // ... apply post processing ...
 * \endcode
 */
template<typename RenderTree>
class DeferredTreeRenderer
{
public:

  DeferredTreeRenderer(iGraphics3D* g3d, iShaderManager *shaderMgr)
    : 
  meshRender(g3d, shaderMgr),
  graphics3D(g3d),
  shaderMgr(shaderMgr)
  {}

  ~DeferredTreeRenderer() {}

  /**
   * Render all contexts.
   */
  void operator()(typename RenderTree::ContextNode *context)
  {
    RenderView *rview = context->renderView;
    
    iEngine *engine = rview->GetEngine ();
    iCamera *cam = rview->GetCamera ();
    iClipper2D *clipper = rview->GetClipper ();
    
    // Setup the camera etc.. @@should be delayed as well
    graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
    graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

    int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | context->drawFlags;
    graphics3D->BeginDraw (drawFlags);
    graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

    size_t layerCount = context->svArrays.GetNumLayers ();
    for (size_t layer = 0; layer < layerCount; ++layer)
    {
      meshRender.SetLayer (layer);
      ForEachOpaqueMeshNode (*context, meshRender);
    }
  }
 
private:

  SimpleContextRender<RenderTree> meshRender;

  iGraphics3D *graphics3D;
  iShaderManager *shaderMgr;
};

/**
 * Renderer for multiple contexts where all transparent objects are drawn.
 */
template<typename RenderTree>
class TransparentMeshTreeRenderer
{
public:

  TransparentMeshTreeRenderer(iGraphics3D* g3d, iShaderManager *shaderMgr)
    : 
  meshRender(g3d, shaderMgr),
  graphics3D(g3d),
  shaderMgr(shaderMgr)
  {}

  ~TransparentMeshTreeRenderer() {}

  /**
   * Render all contexts.
   */
  void operator()(typename RenderTree::ContextNode *context)
  {
    RenderView *rview = context->renderView;
    
    iEngine *engine = rview->GetEngine ();
    iCamera *cam = rview->GetCamera ();
    iClipper2D *clipper = rview->GetClipper ();
    
    // Setup the camera etc.. @@should be delayed as well
    graphics3D->SetProjectionMatrix (context->perspectiveFixup * cam->GetProjectionMatrix ());
    graphics3D->SetClipper (clipper, CS_CLIPPER_TOPLEVEL);

    graphics3D->SetWorldToCamera (context->cameraTransform.GetInverse ());

    size_t layerCount = context->svArrays.GetNumLayers ();
    for (size_t layer = 0; layer < layerCount; ++layer)
    {
      meshRender.SetLayer (layer);
      ForEachTransparentMeshNode (*context, meshRender);
    }
  }
 
private:

  SimpleContextRender<RenderTree> meshRender;

  iGraphics3D *graphics3D;
  iShaderManager *shaderMgr;
};

//----------------------------------------------------------------------
template<typename RenderTreeType, typename LayerConfigType>
class StandardContextSetup
{
public:
  typedef StandardContextSetup<RenderTreeType, LayerConfigType> ThisType;
  typedef StandardPortalSetup<RenderTreeType, ThisType> PortalSetupType;

  StandardContextSetup(RMDeferred *rmanager, const LayerConfigType &layerConfig)
    : 
  rmanager(rmanager), 
  layerConfig(layerConfig),
  recurseCount(0), 
  maxPortalRecurse(rmanager->maxPortalRecurse)
  {}

  StandardContextSetup (const StandardContextSetup &other, const LayerConfigType &layerConfig)
    :
  rmanager(other.rmanager), 
  layerConfig(layerConfig),
  recurseCount(other.recurseCount),
  maxPortalRecurse(other.maxPortalRecurse)
  {}

  void operator()(typename RenderTreeType::ContextNode &context, 
                  typename PortalSetupType::ContextSetupData &portalSetupData)
  {
    CS::RenderManager::RenderView* rview = context.renderView;
    iSector* sector = rview->GetThisSector ();

    if (recurseCount > maxPortalRecurse) return;
    
    iShaderManager* shaderManager = rmanager->shaderManager;

    // @@@ This is somewhat "boilerplate" sector/rview setup.
    sector->PrepareDraw (rview);
    // Make sure the clip-planes are ok
    CS::RenderViewClipper::SetupClipPlanes (rview->GetRenderContext ());

    // Do the culling
    iVisibilityCuller *culler = sector->GetVisibilityCuller ();
    Viscull<RenderTreeType> (context, rview, culler);

    // Set up all portals
    {
      recurseCount++;
      PortalSetupType portalSetup (rmanager->portalPersistent, *this);      
      portalSetup (context, portalSetupData);
      recurseCount--;
    }
    
    // Sort the mesh lists  
    {
      StandardMeshSorter<RenderTreeType> mySorter (rview->GetEngine ());
      mySorter.SetupCameraLocation (rview->GetCamera ()->GetTransform ().GetOrigin ());
      ForEachMeshNode (context, mySorter);
    }

    // After sorting, assign in-context per-mesh indices
    {
      SingleMeshContextNumbering<RenderTreeType> numbering;
      ForEachMeshNode (context, numbering);
    }

    // Setup the SV arrays
    // Push the default stuff
    SetupStandardSVs (context, layerConfig, shaderManager, sector);

    // Setup the material&mesh SVs
    {
      StandardSVSetup<RenderTreeType, MultipleRenderLayer> svSetup (context.svArrays, layerConfig);
      ForEachMeshNode (context, svSetup);
    }

    // Setup shaders and tickets
    SetupStandardShader (context, shaderManager, layerConfig);
    SetupStandardTicket (context, shaderManager, layerConfig);
  }

private:

  RMDeferred *rmanager;

  const LayerConfigType &layerConfig;

  int recurseCount;
  int maxPortalRecurse;
};

//----------------------------------------------------------------------
RMDeferred::RMDeferred(iBase *parent) : scfImplementationType(this, parent)
{
  SetTreePersistent (treePersistent);
}

//----------------------------------------------------------------------
bool RMDeferred::Initialize(iObjectRegistry *registry)
{
  const char *messageID = "crystalspace.rendermanager.deferred";

  objRegistry = registry;

  csRef<iGraphics3D> graphics3D = csQueryRegistry<iGraphics3D> (objRegistry);
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();
   
  shaderManager = csQueryRegistry<iShaderManager> (objRegistry);
  stringSet = csQueryRegistryTagInterface<iStringSet> (objRegistry, "crystalspace.shared.stringset");

  treePersistent.Initialize (shaderManager);
  portalPersistent.Initialize (shaderManager, graphics3D, treePersistent.debugPersist);
  lightRenderPersistent.Initialize (registry);

  // Initialize the extra data in the persistent tree data.
  RenderTreeType::TreeTraitsType::Initialize (treePersistent, registry);
  
  // Read Config settings.
  csConfigAccess cfg (objRegistry);
  maxPortalRecurse = cfg->GetInt("RenderManager.Deferred.MaxPortalRecurse", 30);

  bool layersValid = false;
  const char* layersFile = cfg->GetStr ("RenderManager.Deferred.Layers", 0);
  if (layersFile)
  {
    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID, 
      "Reading render layers from '%s'", layersFile);

    layersValid = CS::RenderManager::AddLayersFromFile (objRegistry, layersFile, renderLayer);
    
    if (!layersValid) 
      renderLayer.Clear();
  }
  
  csRef<iLoader> loader = csQueryRegistry<iLoader> (objRegistry);
  if (!layersValid)
  {
    if (!loader->LoadShader ("/shader/deferred/fill_gbuffer.xml"))
    {
      csReport (objRegistry, CS_REPORTER_SEVERITY_WARNING,
	messageID, "Could not load fill_gbuffer shader");
    }

    csReport (objRegistry, CS_REPORTER_SEVERITY_NOTIFY, messageID,
	"Using default render layers");

    iShaderVarStringSet *svStringSet = shaderManager->GetSVNameStringset ();
    iShader *shader = shaderManager->GetShader ("fill_gbuffer");

    SingleRenderLayer baseLayer (shader);
    baseLayer.AddShaderType (stringSet->Request("gbuffer fill"));

    renderLayer.AddLayers (baseLayer);
  }

  // Creates the accumulation buffer.
  int flags = CS_TEXTURE_2D | CS_TEXTURE_NOMIPMAPS | CS_TEXTURE_CLAMP | CS_TEXTURE_NPOTS;
  
  accumBuffer = graphics3D->GetTextureManager ()->CreateTexture (graphics2D->GetWidth (),
    graphics2D->GetHeight (),
    csimg2D,
    "rgba16_f",
    flags,
    nullptr);

  if (!accumBuffer)
  {
    csReport(objRegistry, CS_REPORTER_SEVERITY_ERROR, messageID, "Could not create accumulation buffer!");
    return false;
  }

  GBuffer::Description desc;
  desc.colorBufferCount = 3;
  desc.hasDepthBuffer = true;
  desc.width = graphics2D->GetWidth ();
  desc.height = graphics2D->GetHeight ();
  desc.colorBufferFormat = nullptr;

  if (!gbuffer.Initialize (desc, 
                           graphics3D, 
                           shaderManager->GetSVNameStringset (), 
                           objRegistry))
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::RenderView(iView *view)
{
  iGraphics3D *graphics3D = view->GetContext ();
  iGraphics2D *graphics2D = graphics3D->GetDriver2D ();

  view->UpdateClipper ();

  int frameWidth = graphics3D->GetWidth ();
  int frameHeight = graphics3D->GetHeight ();
  view->GetCamera ()->SetViewportSize (frameWidth, frameHeight);

  // Setup renderview
  csRef<CS::RenderManager::RenderView> rview;
  rview.AttachNew (CreateNewRenderView (treePersistent.renderViewPool, view));

  // Computes the left, right, top, and bottom of the view frustum.
  iPerspectiveCamera *camera = view->GetPerspectiveCamera ();
  float invFov = camera->GetInvFOV ();
  float l = -invFov * camera->GetShiftX ();
  float r =  invFov * (frameWidth - camera->GetShiftX ());
  float t = -invFov * camera->GetShiftY ();
  float b =  invFov * (frameHeight - camera->GetShiftY ());
  rview->SetFrustum (l, r, t, b);

  portalPersistent.UpdateNewFrame ();

  iEngine *engine = view->GetEngine ();
  engine->UpdateNewFrame ();  
  engine->FireStartFrame (rview);

  iSector *startSector = rview->GetThisSector ();
  if (!startSector)
    return false;

  RenderTreeType renderTree (treePersistent);
  RenderTreeType::ContextNode *startContext = renderTree.CreateContext (rview);

  // Add gbuffer textures to be visualized.
  {
    size_t count = gbuffer.GetColorBufferCount ();
    if (count > 0)
    {
      int w, h;
      gbuffer.GetColorBuffer (0)->GetRendererDimensions (w, h);
      float aspect = (float)w / h;
      
      for (size_t i = 0; i < count; i++)
      {
        renderTree.AddDebugTexture (gbuffer.GetColorBuffer (i), aspect);
      }

      if (gbuffer.GetDepthBuffer ())
      {
        renderTree.AddDebugTexture (gbuffer.GetDepthBuffer (), aspect);
      }
    }
  }

  // Setup the main context
  {
    ContextSetupType contextSetup (this, renderLayer);
    ContextSetupType::PortalSetupType::ContextSetupData portalData (startContext);

    contextSetup (*startContext, portalData);
  }

  // Render all contexts, back to front
  gbuffer.Attach ();
  {
    graphics3D->SetZMode (CS_ZBUF_MESH);

    DeferredTreeRenderer<RenderTreeType> render (graphics3D, shaderManager);
    ForEachContextReverse (renderTree, render);

    graphics3D->FinishDraw ();
  }
  gbuffer.Detach ();

  // Fills the accumulation buffer.
  AttachAccumBuffer (graphics3D, false);
  {
    int drawFlags = engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS | startContext->drawFlags;
    drawFlags &= ~CSDRAW_CLEARSCREEN;

    graphics2D->Clear (graphics2D->FindRGB (0, 0, 0));

    graphics3D->BeginDraw (drawFlags);
    graphics3D->SetWorldToCamera (startContext->cameraTransform.GetInverse ());
    
    // Iterate through lights adding results into accumulation buffer.
    DeferredLightRenderer render (graphics3D,
                                  shaderManager,
                                  stringSet,
                                  rview,
                                  gbuffer,
                                  lightRenderPersistent);

    render.OutputAmbientLight ();
    ForEachLight (*startContext, render);
  }
  DetachAccumBuffer (graphics3D);

  // Draws the transparent objects.
   AttachAccumBuffer (graphics3D, true);
  {
    graphics3D->SetZMode (CS_ZBUF_MESH);

    TransparentMeshTreeRenderer<RenderTreeType> render (graphics3D, shaderManager);
    render (startContext);

    graphics3D->FinishDraw ();
  }
  DetachAccumBuffer (graphics3D);

  // Output the final result to the backbuffer.
  {
    DrawFullscreenTexture (accumBuffer, graphics3D);

    graphics3D->FinishDraw ();
  }

  renderTree.RenderDebugTextures (graphics3D);

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::PrecacheView(iView *view)
{
  return RenderView (view);
}

//----------------------------------------------------------------------
bool RMDeferred::AttachAccumBuffer(iGraphics3D *graphics3D, bool useGbufferDepth)
{
  if (!graphics3D->SetRenderTarget (accumBuffer, false, 0, rtaColor0))
    return false;

  if (useGbufferDepth && gbuffer.HasDepthBuffer ())
  {
    if (!graphics3D->SetRenderTarget (gbuffer.GetDepthBuffer (), false, 0, rtaDepth))
      return false; 
  }
  
  if (!graphics3D->ValidateRenderTargets ())
    return false;

  return true;
}

//----------------------------------------------------------------------
bool RMDeferred::DetachAccumBuffer(iGraphics3D *graphics3D)
{
  graphics3D->UnsetRenderTargets ();
  return true;
}

}
CS_PLUGIN_NAMESPACE_END(RMDeferred)