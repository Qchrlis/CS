/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter
              (C) 2003 by Anders Stenberg

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

#include "cssysdef.h"

#include "iutil/document.h"
#include "iutil/strset.h"
#include "ivideo/rndbuf.h"
#include "ivideo/render3d.h"
#include "ivideo/rendermesh.h"
#include "iengine/rview.h"
#include "iengine/sector.h"
#include "iengine/mesh.h"
#include "iengine/material.h"
#include "ivideo/material.h"

#include "fullquad.h"

class csFullscreenQuad : public iRenderBufferSource
{
private:
  csRef<iRender3D> r3d;
  csRef<iRenderBuffer> vertices;
  csRef<iRenderBuffer> indices;
  csRef<iRenderBuffer> texcoords;

  csStringID vertices_name, indices_name, texcoords_name;

public:

  SCF_DECLARE_IBASE;

  csFullscreenQuad (iRender3D* r3d, iStringSet* strings)
  {
    SCF_CONSTRUCT_IBASE (0)

      csFullscreenQuad::r3d = r3d;

    vertices = r3d->CreateRenderBuffer (
      sizeof (csVector3)*4, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 3, false);
    texcoords = r3d->CreateRenderBuffer (
      sizeof (csVector2)*4, CS_BUF_STATIC,
      CS_BUFCOMP_FLOAT, 2, false);
    indices = r3d->CreateRenderBuffer (
      sizeof (unsigned int)*4, CS_BUF_STATIC,
      CS_BUFCOMP_UNSIGNED_INT, 1, true);

    csVector3* vbuf = (csVector3*)vertices->Lock(CS_BUF_LOCK_NORMAL);
    vbuf[0] = csVector3 ( -1,  1, 0);
    vbuf[1] = csVector3 (  1,  1, 0);
    vbuf[2] = csVector3 (  1, -1, 0);
    vbuf[3] = csVector3 ( -1, -1, 0);
    vertices->Release();

    unsigned int* ibuf = (unsigned int*)indices->Lock(CS_BUF_LOCK_NORMAL);
    ibuf[0] = 0;  ibuf[1] = 1;  ibuf[2] = 2;    ibuf[3] = 3;
    indices->Release();

    csVector2* tcbuf = (csVector2*)texcoords->Lock(CS_BUF_LOCK_NORMAL);
    tcbuf[0] = csVector2 (0, 1);
    tcbuf[1] = csVector2 (1, 1);
    tcbuf[2] = csVector2 (1, 0);
    tcbuf[3] = csVector2 (0, 0);
    texcoords->Release();

    vertices_name = strings->Request ("vertices");
    indices_name = strings->Request ("indices");
    texcoords_name = strings->Request ("texture coordinates");
  }

  iRenderBuffer* GetRenderBuffer(csStringID name)
  {
    if (name == vertices_name)
      return vertices;
    if (name == indices_name)
      return indices;
    if (name == texcoords_name)
      return texcoords;
    return 0;
  }
};

SCF_IMPLEMENT_IBASE (csFullscreenQuad)
SCF_IMPLEMENTS_INTERFACE (iRenderBufferSource)
SCF_IMPLEMENT_IBASE_END

//---------------------------------------------------------------------------


SCF_IMPLEMENT_FACTORY(csFullScreenQuadRSType);
SCF_IMPLEMENT_FACTORY(csFullScreenQuadRSLoader);

//---------------------------------------------------------------------------

csFullScreenQuadRSType::csFullScreenQuadRSType (iBase* p) : csBaseRenderStepType (p)
{
}

csPtr<iRenderStepFactory> csFullScreenQuadRSType::NewFactory()
{
  return csPtr<iRenderStepFactory> 
    (new csFullScreenQuadRenderStepFactory (object_reg));
}

//---------------------------------------------------------------------------

csFullScreenQuadRSLoader::csFullScreenQuadRSLoader (iBase* p) : csBaseRenderStepLoader (p)
{
  init_token_table (tokens);
}

csPtr<iBase> csFullScreenQuadRSLoader::Parse (iDocumentNode* node, 
				       iLoaderContext* ldr_context,      
				       iBase* context)
{
  csRef<iRenderStep> step = 
    new csFullScreenQuadRenderStep (object_reg);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    csStringID id = tokens.Request (child->GetValue ());
    switch (id)
    {
      case XMLTOKEN_MATERIAL:
	{
	  ((csFullScreenQuadRenderStep*)(void*)step)->
            SetMaterial (child->GetContentsValue ());
	}
	break;
      case XMLTOKEN_SHADERTYPE:
        {
          csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (
            object_reg, "crystalspace.renderer.stringset", iStringSet);
          ((csFullScreenQuadRenderStep*)(void*)step)->
            SetShaderType (strings->Request (child->GetContentsValue ()));
        }
        break;
      default:
	if (synldr) synldr->ReportBadToken (child);
	return 0;
    }
  }

  return csPtr<iBase> (step);
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFullScreenQuadRenderStepFactory);
  SCF_IMPLEMENTS_INTERFACE(iRenderStepFactory);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csFullScreenQuadRenderStepFactory::csFullScreenQuadRenderStepFactory (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  csFullScreenQuadRenderStepFactory::object_reg = object_reg;
}

csFullScreenQuadRenderStepFactory::~csFullScreenQuadRenderStepFactory ()
{
}

csPtr<iRenderStep> csFullScreenQuadRenderStepFactory::Create ()
{
  return csPtr<iRenderStep> 
    (new csFullScreenQuadRenderStep (object_reg));
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csFullScreenQuadRenderStep);
  SCF_IMPLEMENTS_INTERFACE(iRenderStep);
SCF_IMPLEMENT_EMBEDDED_IBASE_END;

csFullScreenQuadRenderStep::csFullScreenQuadRenderStep (
  iObjectRegistry* object_reg)
{
  SCF_CONSTRUCT_IBASE(0);

  csRef<iStringSet> strings = CS_QUERY_REGISTRY_TAG_INTERFACE (object_reg, 
    "crystalspace.renderer.stringset", iStringSet);

  csRef<iRender3D> r3d = 
    CS_QUERY_REGISTRY (object_reg, iRender3D);

  fullquad = new csFullscreenQuad (r3d, strings);

  engine = CS_QUERY_REGISTRY (object_reg, iEngine);
  material = "";
}

csFullScreenQuadRenderStep::~csFullScreenQuadRenderStep ()
{
  delete fullquad;
}

void csFullScreenQuadRenderStep::Perform (iRenderView* rview, iSector* sector)
{
  csRef<iRender3D> r3d = rview->GetGraphics3D();

  //r3d->BeginDraw (CSDRAW_3DGRAPHICS);
  iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (material);
  if (mat != 0)
  {
    mat->Visit(); // @@@ here?
    iShaderWrapper* wrapper = 
      mat->GetMaterialHandle()->GetShader(shadertype);
    iShader* shader = wrapper->GetShader ();
    wrapper->SelectMaterial (mat->GetMaterial ());
    iShaderTechnique *tech = shader->GetBestTechnique ();

    if (tech != 0)
    {
      for (int p=0; p<tech->GetPassCount (); p++)
      {
        iShaderPass *pass = tech->GetPass (p);

        csRenderMesh mesh;
        mesh.clip_plane = CS_CLIP_NOT;
        mesh.clip_portal = CS_CLIP_NOT;
        mesh.clip_z_plane = CS_CLIP_NOT;
        mesh.do_mirror = false;
        mesh.indexstart = 0;
        mesh.indexend = 4;
        mesh.buffersource = fullquad;
        csReversibleTransform trans (csMatrix3(), csVector3 (0, 0, -2));
        mesh.transform = &trans;
        mesh.meshtype = CS_MESHTYPE_QUADS;
        mesh.z_buf_mode = CS_ZBUF_NONE;
        mesh.material = mat;

        uint mixmode = pass->GetMixmodeOverride ();
        if (mixmode != 0)
          mesh.mixmode = mixmode;
        else
          mesh.mixmode = CS_FX_COPY;

        pass->Activate (0);
        pass->SetupState (&mesh);
        r3d->DrawMesh (&mesh);
        pass->ResetState ();
        pass->Deactivate ();
      }
    }
  }
  /*csRenderMesh mesh;
  mesh.clip_plane = CS_CLIP_NOT;
  mesh.clip_portal = CS_CLIP_NOT;
  mesh.clip_z_plane = CS_CLIP_NOT;
  mesh.do_mirror = false;
  mesh.indexstart = 0;
  mesh.indexend = 4;
  mesh.mixmode = CS_FX_COPY;
  mesh.streamsource = csFullscreenQuad::instance;
  csReversibleTransform trans (csMatrix3(), csVector3 ());
  mesh.transform = &trans;
  mesh.meshtype = CS_MESHTYPE_QUADS;
  mesh.z_buf_mode = CS_ZBUF_NONE;
  csRef<iShaderManager> shadman = CS_QUERY_REGISTRY (object_reg, iShaderManager);

  //r3d->SetRenderTarget (engine->FindTexture ("TARGET2")->GetTextureHandle ());  
  /*mesh.mathandle = engine->GetMaterialList ()->FindByName ("post a")->GetMaterialHandle ();
  csRef<iShader> shader = shadman->GetShader ("posteffect a");
  r3d->BeginDraw (CSDRAW_3DGRAPHICS | CSDRAW_CLEARSCREEN );
  shader->GetBestTechnique ()->GetPass (0)->Activate (&mesh);
  shader->GetBestTechnique ()->GetPass (0)->SetupState (&mesh);
  r3d->DrawMesh (&mesh);
  shader->GetBestTechnique ()->GetPass (0)->ResetState ();
  shader->GetBestTechnique ()->GetPass (0)->Deactivate ();
  r3d->FinishDraw ();*/
};
