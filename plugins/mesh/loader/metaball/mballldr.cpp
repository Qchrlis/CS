/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Copyright (C) 2001 by W.C.A. Wijngaards

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
#include "csgeom/math3d.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "mballldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/metaball.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (ISO_LEVEL)
  CS_TOKEN_DEF (CHARGE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (TRUE_MAP)
  CS_TOKEN_DEF (TEX_SCALE)
  CS_TOKEN_DEF (RATE)

  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (SHIFT)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csMetaBallFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csMetaBallFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csMetaBallLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csMetaBallSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csMetaBallFactoryLoader)
IMPLEMENT_FACTORY (csMetaBallFactorySaver)
IMPLEMENT_FACTORY (csMetaBallLoader)
IMPLEMENT_FACTORY (csMetaBallSaver)

EXPORT_CLASS_TABLE (mballldr)
  EXPORT_CLASS (csMetaBallFactoryLoader, "crystalspace.mesh.loader.factory.metaball",
    "Crystal Space MetaBall Factory Loader")
  EXPORT_CLASS (csMetaBallFactorySaver, "crystalspace.mesh.saver.factory.metaball",
    "Crystal Space MetaBall Factory Saver")
  EXPORT_CLASS (csMetaBallLoader, "crystalspace.mesh.loader.metaball",
    "Crystal Space MetaBall Mesh Loader")
  EXPORT_CLASS (csMetaBallSaver, "crystalspace.mesh.saver.metaball",
    "Crystal Space MetaBall Mesh Saver")
EXPORT_CLASS_TABLE_END

csMetaBallFactoryLoader::csMetaBallFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csMetaBallFactoryLoader::~csMetaBallFactoryLoader ()
{
}

bool csMetaBallFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csMetaBallFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys,
  	"crystalspace.mesh.object.metaball", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.metaball",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.metaball\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csMetaBallFactorySaver::csMetaBallFactorySaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csMetaBallFactorySaver::~csMetaBallFactorySaver ()
{
}

bool csMetaBallFactorySaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

void csMetaBallFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // no params
}

//---------------------------------------------------------------------------

csMetaBallLoader::csMetaBallLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csMetaBallLoader::~csMetaBallLoader ()
{
}

bool csMetaBallLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

static UInt ParseMixmode (char* buf)
{
  CS_TOKEN_TABLE_START (modes)
    CS_TOKEN_TABLE (COPY)
    CS_TOKEN_TABLE (MULTIPLY2)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (ADD)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (KEYCOLOR)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  UInt Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
      case CS_TOKEN_COPY: Mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: Mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: Mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: Mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	Mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
        ScanStr (params, "%f", &alpha);
	Mixmode |= CS_FX_SETALPHA(alpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing the modes!\n", csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}

iBase* csMetaBallLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (LIGHTING)
	CS_TOKEN_TABLE (ISO_LEVEL)
	CS_TOKEN_TABLE (CHARGE)
	CS_TOKEN_TABLE (NUMBER)
	CS_TOKEN_TABLE (RATE)
	CS_TOKEN_TABLE (TRUE_MAP)
	CS_TOKEN_TABLE (TEX_SCALE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshObject* mesh = NULL;
  iMetaBallState* ballstate = NULL;
  MetaParameters* mp = NULL;
  iMeshWrapper* imeshwrap = QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (ballstate) ballstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_ISO_LEVEL:
	{
	  if (!mp) { printf("Please set FACTORY before ISO_LEVEL\n"); return NULL; }
	  float f;
	  ScanStr (params, "%f", &f);
	  mp->iso_level = f;
	}
	break;
      case CS_TOKEN_CHARGE:
	{
	  if (!mp) { printf("Please set FACTORY before CHARGE\n"); return NULL; }
	  float f;
	  ScanStr (params, "%f", &f);
	  mp->charge = f;
	}
	break;
      case CS_TOKEN_NUMBER:
	{
	  if (!ballstate) { printf("Please set FACTORY before NUMBER\n"); return NULL; }
	  int r;
	  ScanStr (params, "%d", &r);
	  ballstate->SetNumberMetaBalls (r);
	}
	break;
      case CS_TOKEN_RATE:
	{
	  if (!mp) { printf("Please set FACTORY before RATE\n"); return NULL; }
	  float r;
	  ScanStr (params, "%f", &r);
	  mp->rate = r;
	}
	break;
      case CS_TOKEN_TRUE_MAP:
	{
	  if (!ballstate) { printf("Please set FACTORY before TRUE_MAP\n"); return NULL; }
	  bool m;
	  ScanStr (params, "%b", &m);
	  ballstate->SetQualityEnvironmentMapping (m);
	}
	break;
      case CS_TOKEN_TEX_SCALE:
	{
	  if (!ballstate) { printf("Please set FACTORY before TEX_SCALE\n"); return NULL; }
	  float s;
	  ScanStr (params, "%f", &s);
	  ballstate->SetEnvironmentMappingFactor(s);
	}
	break;
      case CS_TOKEN_FACTORY:
	{
      ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
      ballstate = QUERY_INTERFACE (mesh, iMetaBallState);
	  mp = ballstate->GetParameters();
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
	  if (!ballstate) { printf("Please set FACTORY before MATERIAL\n"); return NULL; }
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
            // @@@ Error handling!
            mesh->DecRef ();
            return NULL;
	  }
	  ballstate->SetMaterial(mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
		if (!ballstate) { printf("Please set FACTORY before MIXMODE\n"); return NULL; }
        ballstate->SetMixMode (ParseMixmode (params));
	break;
	  case CS_TOKEN_LIGHTING:
		if (!ballstate) { printf("Please set FACTORY before MIXMODE\n"); return NULL; }
		bool l;
		ScanStr(params, "%b", &l);
		ballstate->SetLighting(l);
	break;
    }
  }

  if (ballstate) ballstate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csMetaBallSaver::csMetaBallSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csMetaBallSaver::~csMetaBallSaver ()
{
}

bool csMetaBallSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(strnew("  MIXMODE ("));
  if(mixmode&CS_FX_COPY) str->Push(strnew(" COPY ()"));
  if(mixmode&CS_FX_ADD) str->Push(strnew(" ADD ()"));
  if(mixmode&CS_FX_MULTIPLY) str->Push(strnew(" MULTIPLY ()"));
  if(mixmode&CS_FX_MULTIPLY2) str->Push(strnew(" MULTIPLY2 ()"));
  if(mixmode&CS_FX_KEYCOLOR) str->Push(strnew(" KEYCOLOR ()"));
  if(mixmode&CS_FX_TRANSPARENT) str->Push(strnew(" TRANSPARENT ()"));
  if(mixmode&CS_FX_ALPHA) {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(strnew(buf));
  }
  str->Push(strnew(")"));
}

void csMetaBallSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iMeshObject *mesh = QUERY_INTERFACE(obj, iMeshObject);
  if(!mesh)
  {
    printf("Error: non-mesh given to %s.\n", 
      fact->QueryDescription () );
    fact->DecRef();
    return;
  }
  iMetaBallState *state = QUERY_INTERFACE(obj, iMetaBallState);
  if(!state)
  {
    printf("Error: invalid mesh given to %s.\n", 
      fact->QueryDescription () );
    fact->DecRef();
    mesh->DecRef();
    return;
  }

  char buf[MAXLINE];
  char name[MAXLINE];
  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(strnew(buf));
  if(state->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, state->GetMixMode());
  }

  // Mesh information
  MetaParameters *mp = state->GetParameters();
  sprintf(buf, "NUMBER (%d)\n", state->GetNumberMetaBalls());
  str->Push(strnew(buf));
  sprintf(buf, "ISO_LEVEL (%f)\n",mp->iso_level );
  str->Push(strnew(buf));
  sprintf(buf, "CHARGE (%f)\n", mp->charge);
  str->Push(strnew(buf));
  sprintf(buf, "MATERIAL (%s)\n", state->GetMaterial()->
    QueryObject ()->GetName());
  str->Push(strnew(buf));
  sprintf(buf, "LIGHTING(%s)\n",(state->IsLighting())? "true" : "false");
  str->Push (strnew (buf));
  sprintf(buf, "NUMBER (%d)\n", state->GetNumberMetaBalls());
  str->Push(strnew(buf));
  sprintf(buf, "RATE (%f)\n",mp->rate);
  str->Push(strnew(buf));
  sprintf(buf, "TRUE_MAP (%s)\n",(state->GetQualityEnvironmentMapping())?"true":"false");
  str->Push(strnew(buf));
  sprintf(buf, "TEX_SCALE (%f)\n",state->GetEnvironmentMappingFactor());
  str->Push(strnew(buf));

  fact->DecRef();
  mesh->DecRef();
  state->DecRef();

}

//---------------------------------------------------------------------------

