/*
 * Copyright (C) 1998-2001 by Jorrit Tyberghein
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdarg.h>

#include <stdio.h>
#include <ctype.h>

#include "cssysdef.h"
#include "cssys/sysdriv.h"
#include "qint.h"
#include "csutil/scf.h"
#include "csgeom/math2d.h"
#include "csgeom/math3d.h"
#include "csgeom/polyclip.h"
#include "csgeom/plane3.h"
#include "csgeom/frustum.h"
#include "ogl_g3dcom.h"
#include "ogl_txtcache.h"
#include "ogl_txtmgr.h"
#include "iutil/cfgfile.h"
#include "isys/system.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "iengine/polygon.h"
#include "iengine/camera.h"
#include "iengine/lightmap.h"
#include "ivideo/graph2d.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "csgfx/rgbpixel.h"
#include "qsqrt.h"

#include <GL/gl.h>

#define SysPrintf System->Printf

// uncomment the 'USE_MULTITEXTURE 1' define to enable code for
// multitexture support - this is independent of the extension detection,
// but  it may rely on the extension module to supply proper function
// prototypes for the ARB_MULTITEXTURE functions
//#define USE_MULTITEXTURE 1

// Whether or not we should try  and use OpenGL extensions. This should be
// removed eventually, when all platforms have been updated.
//#define USE_EXTENSIONS 1

// ---------------------------------------------------------------------------

// if you figure out how to support OpenGL extensions on your
// machine add an appropriate file in the 'ext/'
// directory and mangle the file 'ext/ext_auto.inc'
// to access your extension code
#if USE_EXTENSIONS
#include "ext/ext_auto.inc"
#else
void csGraphics3DOGLCommon::DetectExtensions() {}
#endif

#define BYTE_TO_FLOAT(x) ((x) * (1.0 / 255.0))

//@@@ Another experimental optimization:
// This optimization alone doesn't appear to be enough.
// We need better state caching. I keep it here to remind
// us of this.
//static GLuint prev_handle = 0;
void csglBindTexture (GLenum target, GLuint handle)
{
  //if (prev_handle != handle)
  //{
    //prev_handle = handle;
    glBindTexture (target, handle);
  //}
}

//@@@ For opt DrawPolygon:

void csGraphics3DOGLCommon::start_draw_poly ()
{
  if (in_draw_poly) return;
  in_draw_poly = true;
}

void csGraphics3DOGLCommon::end_draw_poly ()
{
  if (!in_draw_poly) return;
  in_draw_poly = false;
}

/*===========================================================================
 Fog Variables Section
 ===========================================================================*/

// size of the fog texture
static const unsigned int FOGTABLE_SIZE = 64;

// each texel in the fog table holds the fog alpha value at a certain
// (distance*density).  The median distance parameter determines the
// (distance*density) value represented by the texel at the center of
// the fog table.  The fog calculation is:
// alpha = 1.0 - exp( -(density*distance) / FOGTABLE_MEDIANDISTANCE)
//
static const double FOGTABLE_MEDIANDISTANCE = 10.0;
static const double FOGTABLE_MAXDISTANCE = FOGTABLE_MEDIANDISTANCE * 2.0;
static const double FOGTABLE_DISTANCESCALE = 1.0 / FOGTABLE_MAXDISTANCE;

// fog (distance*density) is mapped to a texture coordinate and then
// clamped.  This determines the clamp value.  Some OpenGL drivers don't
// like clamping textures so we must do it ourself
static const double FOGTABLE_CLAMPVALUE = 0.85;

static const double FOG_MAXVALUE = FOGTABLE_MAXDISTANCE * FOGTABLE_CLAMPVALUE;
/*=========================================================================
 Static growing array declaration for DrawTriangleMesh
=========================================================================*/
// smgh moved it here, no longer segfaults on exit as a consequence..
// Also IncRefing and DecRefing in the ctor/dtor, as the auxiliary buffer
// dynamic textures will utilise multiple instances of csGraphics3DOGLCommon

/// Static vertex array.
static DECLARE_GROWING_ARRAY_REF (tr_verts, csVector3);
/// Static uv array.
static DECLARE_GROWING_ARRAY_REF (uv_verts, csVector2);
/// Static uv array for multi-texture.
static DECLARE_GROWING_ARRAY_REF (uv_mul_verts, csVector2);
/// Array with colors.
static DECLARE_GROWING_ARRAY_REF (color_verts, csColor);
/// Array with RGBA colors.
static DECLARE_GROWING_ARRAY_REF (rgba_verts, GLfloat);
/// Array with fog values.
static DECLARE_GROWING_ARRAY_REF (fog_intensities, float);
/// Array with fog colors
static DECLARE_GROWING_ARRAY_REF (fog_color_verts, csColor);

/// Array for clipping.
static DECLARE_GROWING_ARRAY_REF (clipped_triangles, csTriangle);
/// Array for clipping.
static DECLARE_GROWING_ARRAY_REF (clipped_translate, int);
/// Array for clipping.
static DECLARE_GROWING_ARRAY_REF (clipped_vertices, csVector3);
/// Array for clipping.
static DECLARE_GROWING_ARRAY_REF (clipped_texels, csVector2);
/// Array for clipping.
static DECLARE_GROWING_ARRAY_REF (clipped_colors, csColor);
/// Array for clipping.
static DECLARE_GROWING_ARRAY_REF (clipped_fog, G3DFogInfo);

/*=========================================================================
 Method implementations
=========================================================================*/

csGraphics3DOGLCommon::csGraphics3DOGLCommon ():
  G2D (NULL), System (NULL)
{
  ShortcutDrawPolygon = 0;
  ShortcutStartPolygonFX = 0;
  ShortcutDrawPolygonFX = 0;
  ShortcutFinishPolygonFX = 0;

  texture_cache = NULL;
  lightmap_cache = NULL;
  txtmgr = NULL;
  m_fogtexturehandle = 0;
  dbg_max_polygons_to_draw = 2000000000;	// After 2 billion
  // polygons we give up :-)

  /// caps will be read from config or reset to defaults during Initialize.
  Caps.CanClip = false;
  Caps.minTexHeight = 2;
  Caps.minTexWidth = 2;
  Caps.maxTexHeight = 1024;
  Caps.maxTexWidth = 1024;
  Caps.fog = G3DFOGMETHOD_VERTEX;
  Caps.NeedsPO2Maps = true;
  Caps.MaxAspectRatio = 32768;
  GLCaps.need_screen_clipping = false;
  GLCaps.use_stencil = false;
  clip_optional[0] = OPENGL_CLIP_AUTO;
  clip_optional[1] = OPENGL_CLIP_SOFTWARE;
  clip_optional[2] = OPENGL_CLIP_SOFTWARE;
  clip_required[0] = OPENGL_CLIP_AUTO;
  clip_required[1] = OPENGL_CLIP_SOFTWARE;
  clip_required[2] = OPENGL_CLIP_SOFTWARE;
  clip_outer[0] = OPENGL_CLIP_AUTO;
  clip_outer[1] = OPENGL_CLIP_SOFTWARE;
  clip_outer[2] = OPENGL_CLIP_SOFTWARE;

  // default extension state is for all extensions to be OFF
  ARB_multitexture = false;
  clipper = NULL;
  cliptype = CS_CLIPPER_NONE;
  toplevel_init = false;
  stencil_init = false;
  planes_init = false;
  frustum_valid = false;

  // see note above
  tr_verts.IncRef ();
  uv_verts.IncRef ();
  uv_mul_verts.IncRef ();
  color_verts.IncRef ();
  fog_intensities.IncRef ();
  fog_color_verts.IncRef ();
  clipped_triangles.IncRef ();
  clipped_translate.IncRef ();
  clipped_vertices.IncRef ();
  clipped_texels.IncRef ();
  clipped_colors.IncRef ();
  clipped_fog.IncRef ();

  in_draw_poly = false;

  // Are we going to use the inverted orthographic projection matrix?
  inverted = false;

}

csGraphics3DOGLCommon::~csGraphics3DOGLCommon ()
{
  Close ();
  if (G2D) G2D->DecRef ();

  // see note above
  tr_verts.DecRef ();
  uv_verts.DecRef ();
  uv_mul_verts.DecRef ();
  color_verts.DecRef ();
  fog_intensities.DecRef ();
  fog_color_verts.DecRef ();
  clipped_triangles.DecRef ();
  clipped_translate.DecRef ();
  clipped_vertices.DecRef ();
  clipped_texels.DecRef ();
  clipped_colors.DecRef ();
  clipped_fog.DecRef ();
}

bool csGraphics3DOGLCommon::NewInitialize (iSystem * iSys)
{
  System = iSys;

  config.AddConfig(System, "/config/opengl.cfg");

  const char *driver = iSys->GetOptionCL ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.OpenGL.Canvas", OPENGL_2D_DRIVER);

  G2D = LOAD_PLUGIN (System, driver, NULL, iGraphics2D);
  if (!G2D)
    return false;

  txtmgr = new csTextureManagerOpenGL (System, G2D, config, this);

  m_renderstate.dither = config->GetBool ("Video.OpenGL.EnableDither", false);
  z_buf_mode = CS_ZBUF_NONE;
  width = height = -1;
  Caps.CanClip = config->GetBool("Video.OpenGL.Caps.CanClip", false);
  Caps.minTexHeight = config->GetInt("Video.OpenGL.Caps.minTexHeight", 2);
  Caps.minTexWidth = config->GetInt("Video.OpenGL.Caps.minTexWidth", 2);
  Caps.maxTexHeight = config->GetInt("Video.OpenGL.Caps.maxTexHeight", 1024);
  Caps.maxTexWidth = config->GetInt("Video.OpenGL.Caps.maxTexWidth", 1024);
  Caps.fog = G3DFOGMETHOD_VERTEX;
  Caps.NeedsPO2Maps = config->GetBool("Video.OpenGL.Caps.NeedsPO2Maps", true);
  Caps.MaxAspectRatio = config->GetInt("Video.OpenGL.Caps.MaxAspectRatio", 
    32768);
  GLCaps.use_stencil = config->GetBool ("Video.OpenGL.Caps.Stencil", false);
  GLCaps.need_screen_clipping =
  	config->GetBool ("Video.OpenGL.Caps.NeedScreenClipping", false);
  GLCaps.nr_hardware_planes = config->GetInt ("Video.OpenGL.Caps.HWPlanes", 6);

  unsigned int i, j;
  const char* clip_opt = config->GetStr ("Video.OpenGL.ClipOptional", "auto");
  if (!strcmp (clip_opt, "auto"))
    clip_optional[0] = OPENGL_CLIP_AUTO;
  else
  {
    for (j = i = 0 ; i < strlen (clip_opt) ; i++)
    {
      char c = clip_opt[i];
      if ((c == 's' || c == 'S') && !GLCaps.use_stencil) continue;
      if ((c == 'p' || c == 'P') && GLCaps.nr_hardware_planes <= 0) continue;
      if (c == 'z' || c == 'Z') continue;
      clip_optional[j++] = c;
      if (j >= 3) break;
    }
    while (j < 3) clip_optional[j++] = '0';
    SysPrintf (MSG_INITIALIZATION, "  Optional Clipping: %c%c%c\n",
      clip_optional[0], clip_optional[1], clip_optional[2]);
  }

  const char* clip_req = config->GetStr ("Video.OpenGL.ClipRequired", "auto");
  if (!strcmp (clip_req, "auto"))
    clip_required[0] = OPENGL_CLIP_AUTO;
  else
  {
    for (j = i = 0 ; i < strlen (clip_req) ; i++)
    {
      char c = clip_req[i];
      if ((c == 's' || c == 'S') && !GLCaps.use_stencil) continue;
      if ((c == 'p' || c == 'P') && GLCaps.nr_hardware_planes <= 0) continue;
      if (c == 'z' || c == 'Z') continue;
      if (c == 'n' || c == 'N') continue;
      clip_required[j++] = c;
      if (j >= 3) break;
    }
    while (j < 3) clip_required[j++] = '0';
    SysPrintf (MSG_INITIALIZATION, "  Required Clipping: %c%c%c\n",
        clip_required[0], clip_required[1], clip_required[2]);
  }

  const char* clip_out = config->GetStr ("Video.OpenGL.ClipOuter", "zsp0");
  if (!strcmp (clip_out, "auto"))
    clip_outer[0] = OPENGL_CLIP_AUTO;
  else
  {
    for (j = i = 0 ; i < strlen (clip_out) ; i++)
    {
      char c = clip_out[i];
      if ((c == 's' || c == 'S') && !GLCaps.use_stencil) continue;
      if ((c == 'p' || c == 'P') && GLCaps.nr_hardware_planes <= 0) continue;
      if ((c == 'z' || c == 'Z' || c == 's' || c == 'S' || c == 'p' || c == 'P')
    	  && GLCaps.need_screen_clipping) continue;
      if (c == 'n' || c == 'N') continue;
      clip_outer[j++] = c;
      if (j >= 3) break;
    }
    while (j < 3) clip_outer[j++] = '0';
    SysPrintf (MSG_INITIALIZATION, "  Outer Clipping: %c%c%c\n",
        clip_outer[0], clip_outer[1], clip_outer[2]);
  }

  m_renderstate.alphablend = true;
  m_renderstate.mipmap = 0;
  m_renderstate.gouraud = true;
  m_renderstate.lighting = true;
  m_renderstate.textured = true;

  m_config_options.do_multitexture_level = 0;
  m_config_options.do_extra_bright = false;

  return true;
}

struct ModRes
{
  char mode;
  int pref_order;
  int cnt;
};

static int compare_mode (const void* el1, const void* el2)
{
  ModRes* m1 = (ModRes*)el1;
  ModRes* m2 = (ModRes*)el2;
  if (m1->cnt < m2->cnt) return 1;
  else if (m1->cnt > m2->cnt) return -1;
  else if (m1->pref_order < m2->pref_order) return 1;
  else if (m1->pref_order > m2->pref_order) return -1;
  else return 0;
}

void csGraphics3DOGLCommon::PerfTest ()
{
  bool compute_optional = clip_optional[0] == OPENGL_CLIP_AUTO;
  bool compute_outer = clip_outer[0] == OPENGL_CLIP_AUTO;
  bool compute_required = clip_required[0] == OPENGL_CLIP_AUTO;
  if (!compute_optional && !compute_outer && !compute_required)
    return;

  SetPerspectiveAspect (height);
  SetPerspectiveCenter (width/2, height/2);

  G3DTriangleMesh mesh;
  int res = 64;
  mesh.num_vertices = (res+1)*(res+1);
  mesh.num_triangles = res*res*2;
  mesh.num_vertices_pool = 1;
  mesh.clip_portal = CS_CLIP_NEEDED;
  mesh.clip_plane = CS_CLIP_NOT;
  mesh.use_vertex_color = true;
  mesh.do_fog = false;
  mesh.do_morph_texels = false;
  mesh.do_morph_colors = false;
  mesh.vertex_mode = G3DTriangleMesh::VM_WORLDSPACE;
  mesh.fxmode = CS_FX_GOURAUD;
  mesh.morph_factor = 0;
  mesh.mat_handle = NULL;
  mesh.vertex_fog = NULL;
  mesh.triangles = new csTriangle [mesh.num_triangles];
  mesh.vertices[0] = new csVector3 [mesh.num_vertices];
  mesh.texels[0] = new csVector2 [mesh.num_vertices];
  mesh.vertex_colors[0] = new csColor [mesh.num_vertices];

  float zx, zy, z;
  // First we calculate the z which will bring the top-left vertex
  // further than (-width2,-height2).
  zx = -5. / ((-width2-width2) * inv_aspect);
  zy = -5. / ((-height2-height2) * inv_aspect);
  if (zy < zx) z = zy;
  else z = zx;

  int x, y, i, t;
  float fx, fy;
  i = 0;
  for (y = 0 ; y <= res ; y++)
  {
    fy = float (y) / float (res) - .5;
    for (x = 0 ; x <= res ; x++)
    {
      fx = float (x) / float (res) - .5;
      mesh.vertices[0][i].Set (10.*fx, 10.*fy, z);
      mesh.texels[0][i].Set (0, 0);
      mesh.vertex_colors[0][i].Set (1, 0, 0);
      i++;
    }
  }
  i = 0;
  t = 0;
  for (y = 0 ; y < res ; y++)
  {
    for (x = 0 ; x < res ; x++)
    {
      mesh.triangles[t].c = i;
      mesh.triangles[t].b = i+1;
      mesh.triangles[t].a = i+res+1;
      t++;
      mesh.triangles[t].c = i+1;
      mesh.triangles[t].b = i+res+1+1;
      mesh.triangles[t].a = i+res+1;
      t++;
      i++;
    }
    i++;
  }

  BeginDraw (CSDRAW_3DGRAPHICS);

  int dw = width / 20;
  int dh = height / 20;
  csBoxClipper clipper (dw, dh, width-dw, height-dh);
  SetClipper ((iClipper2D*)&clipper, CS_CLIPPER_TOPLEVEL);
  ResetNearPlane ();

  ModRes test_modes[9];
  int test_mode_cnt = 0;
  test_modes[test_mode_cnt].mode = '0';
  test_modes[test_mode_cnt++].pref_order = 3;
  test_modes[test_mode_cnt].mode = 'z';
  test_modes[test_mode_cnt++].pref_order = 7;
  test_modes[test_mode_cnt].mode = 'Z';
  test_modes[test_mode_cnt++].pref_order = 6;
  if (GLCaps.use_stencil)
  {
    test_modes[test_mode_cnt].mode = 's';
    test_modes[test_mode_cnt++].pref_order = 2;
    test_modes[test_mode_cnt].mode = 'S';
    test_modes[test_mode_cnt++].pref_order = 1;
  }
  if (GLCaps.nr_hardware_planes > 0)
  {
    test_modes[test_mode_cnt].mode = 'p';
    test_modes[test_mode_cnt++].pref_order = 5;
    test_modes[test_mode_cnt].mode = 'P';
    test_modes[test_mode_cnt++].pref_order = 4;
  }

  if (compute_outer && !GLCaps.need_screen_clipping)
  {
    //========
    // First test clipping geometry against the outer portal (close
    // to screen boundaries).
    //========
    for (i = 0 ; i < test_mode_cnt ; i++)
    {
      clip_outer[0] = test_modes[i].mode;
      int cnt = 0;
      cs_time end = System->GetTime () + 1000;
      while (System->GetTime () < end)
      {
        glDepthMask (GL_TRUE);
        glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
        csReversibleTransform o2c;	// Identity transform.
        SetObjectToCamera (&o2c);
        DrawTriangleMesh (mesh);
        cnt++;
      }
      test_modes[i].cnt = cnt;
      printf ("    %d FPS for %c\n", cnt,
    	  test_modes[i].mode); fflush (stdout);
    }
    // Sort the results.
    qsort (test_modes, test_mode_cnt, sizeof (ModRes), compare_mode);
    clip_outer[0] = test_modes[0].mode;
    if (test_mode_cnt > 1) clip_outer[1] = test_modes[1].mode;
    else clip_outer[1] = '0';
    if (test_mode_cnt > 2) clip_outer[2] = test_modes[2].mode;
    else clip_outer[2] = '0';
  }
  else
  {
    if (compute_outer)
    {
      clip_outer[0] = '0';
      clip_outer[1] = '0';
      clip_outer[2] = '0';
    }
  }

  //========
  // Now test again for a very small clipper. This test is to see if
  // it is beneficial to even enable clipping on optional portals.
  //========
  csBoxClipper clipper2 (width/2-dw, height/2-dh, width/2+dw, height/2+dh);
  SetClipper ((iClipper2D*)&clipper2, CS_CLIPPER_OPTIONAL);
  // First relocate all vertices so that the mesh fits on screen
  // but not in the small clipper.
  // First we calculate the z which will bring the top-left vertex
  // beyond (dw,dh).
  zx = -5. / ((dw-width2) * inv_aspect);
  zy = -5. / ((dh-height2) * inv_aspect);
  if (zy > zx) z = zy;
  else z = zx;
  for (i = 0 ; i < mesh.num_vertices ; i++)
    mesh.vertices[0][i].z = z;
  for (i = 0 ; i < test_mode_cnt ; i++)
  {
    clip_optional[0] = test_modes[i].mode;
    int cnt = 0;
    cs_time end = System->GetTime () + 1000;
    while (System->GetTime () < end)
    {
      glDepthMask (GL_TRUE);
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
      csReversibleTransform o2c;	// Identity transform.
      SetObjectToCamera (&o2c);
      DrawTriangleMesh (mesh);
      cnt++;
    }
    test_modes[i].cnt = cnt;
    printf ("    %d FPS for %c (small clipper)\n", cnt,
    	test_modes[i].mode); fflush (stdout);
  }

  // Sort the results.
  qsort (test_modes, test_mode_cnt, sizeof (ModRes), compare_mode);
  int j;
  if (compute_required)
  {
    j = 0;
    for (i = 0 ; i < 3 ; i++)
    {
      while (j < test_mode_cnt &&
    	  (test_modes[j].mode == 'z' || test_modes[j].mode == 'Z')) j++;
      if (j >= test_mode_cnt) clip_required[i] = '0';
      else clip_required[i] = test_modes[j].mode;
      j++;
    }
  }
  if (compute_optional)
  {
    j = 0;
    for (i = 0 ; i < 3 ; i++)
    {
      if (j >= test_mode_cnt) clip_optional[i] = '0';
      else clip_optional[i] = test_modes[j].mode;
      j++;
    }
  }
  SysPrintf(MSG_INITIALIZATION, "    Video.OpenGL.ClipOuter = %c%c%c\n", clip_outer[0],
  	clip_outer[1], clip_outer[2]);
  SysPrintf(MSG_INITIALIZATION, "    Video.OpenGL.ClipRequired = %c%c%c\n", clip_required[0],
  	clip_required[1], clip_required[2]);
  SysPrintf(MSG_INITIALIZATION, "    Video.OpenGL.ClipOptional = %c%c%c\n", clip_optional[0],
  	clip_optional[1], clip_optional[2]);

  SetClipper (NULL, CS_CLIPPER_NONE);

  FinishDraw ();
  Print (NULL);
}

void csGraphics3DOGLCommon::SharedInitialize (csGraphics3DOGLCommon *d)
{
  txtmgr = d->txtmgr;
  z_buf_mode = CS_ZBUF_NONE;
  width = height = -1;

  m_renderstate.dither = d->m_renderstate.dither;

  m_renderstate.alphablend = true;
  m_renderstate.mipmap = 0;
  m_renderstate.gouraud = true;
  m_renderstate.lighting = true;
  m_renderstate.textured = true;

  m_config_options.do_multitexture_level = 0;
  m_config_options.do_extra_bright = false;
}

bool csGraphics3DOGLCommon::NewOpen (const char *Title)
{
  CommonOpen ();

  if (!G2D->Open (Title))
  {
    SysPrintf (MSG_FATAL_ERROR, "Error opening Graphics2D context.\n");
    // set "not opened" flag
    width = height = -1;
    return false;
  }
  // See if we find any OpenGL extensions, and set the corresponding
  // flags. Look at the bottom
  // of ogl_g3d.h for known extensions (currently only multitexture)
#if USE_EXTENSIONS
  DetectExtensions ();
#endif

  if (m_renderstate.dither)
    glEnable (GL_DITHER);
  else
    glDisable (GL_DITHER);

  if (config->GetBool ("Video.OpenGL.HintPerspectiveFast", false))
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
  else
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  m_config_options.do_extra_bright = config->GetBool
        ("Video.OpenGL.ExtraBright", false);
  // determine what blend mode to use when combining lightmaps with
  // their  underlying textures.  This mode is set in the Opengl
  // configuration  file
  struct
  {
    char *blendstylename;
    GLenum srcblend, dstblend;
  }
  blendstyles[] =
  {
    { "multiplydouble", GL_DST_COLOR, GL_SRC_COLOR } ,
    { "multiply", GL_DST_COLOR, GL_ZERO } ,
    { "add", GL_ONE, GL_ONE } ,
    { "coloradd", GL_ONE, GL_SRC_COLOR } ,
    { "ps2tristage", (GLenum)696969, (GLenum)696969 } ,
    { NULL, GL_DST_COLOR, GL_ZERO }
  };

  // try to match user's blend name with a name in the blendstyles table
  const char *lightmapstyle = config->GetStr
        ("Video.OpenGL.LightmapMode","multiplydouble");
  int blendstyleindex = 0;
  while (blendstyles[blendstyleindex].blendstylename != NULL)
  {
    if (strcmp (lightmapstyle, blendstyles[blendstyleindex].blendstylename) == 0)
    {
      m_config_options.m_lightmap_src_blend = blendstyles[blendstyleindex].srcblend;
      m_config_options.m_lightmap_dst_blend = blendstyles[blendstyleindex].dstblend;
      break;
    }
    blendstyleindex++;
  }

#if USE_EXTENSIONS
  // check with the GL driver and see if it supports the multitexure
  // extension
  if (ARB_multitexture)
  {
    // if you support multitexture, you should allow more than one
    // texture, right?  Let's see how many we can get...
    GLint maxtextures;
    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &maxtextures);

    if (maxtextures > 1)
    {
      m_config_options.do_multitexture_level = maxtextures;
      SysPrintf (MSG_INITIALIZATION, "Using multitexture extension with %d texture units\n", maxtextures);
    }
    else
    {
      SysPrintf (MSG_INITIALIZATION, "WARNING: driver supports multitexture extension but only allows one texture unit!\n");
    }
  }
#endif

  // tells OpenGL driver we align texture data on byte boundaries,
  // instead of perhaps word or dword boundaries
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  // generate the exponential 1D texture for use in vertex fogging
  // this texture holds a 'table' of alpha values forming an exponential
  // curve, used for generating exponential fog by mapping it onto
  // fogged polygons as we draw them.
  const unsigned int FOGTABLE_SIZE = 64;
  unsigned char *transientfogdata = new unsigned char[FOGTABLE_SIZE * 4];
  for (unsigned int fogindex = 0; fogindex < FOGTABLE_SIZE; fogindex++)
  {
    transientfogdata[fogindex * 4 + 0] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 1] = (unsigned char) 255;
    transientfogdata[fogindex * 4 + 2] = (unsigned char) 255;
    double fogalpha =
    (256 * (1.0 - exp (-float (fogindex) * FOGTABLE_MAXDISTANCE / FOGTABLE_SIZE)));
    transientfogdata[fogindex * 4 + 3] = (unsigned char) fogalpha;
  }
  // prevent weird effects when 0 distance fog wraps around to the
  // 'max fog' texel
  transientfogdata[(FOGTABLE_SIZE - 1) * 4 + 3] = 0;

  // dump the fog table into an OpenGL texture for later user.
  // The texture is FOGTABLE_SIZE texels wide and one texel high;
  // we could use a 1D texture but some OpenGL drivers don't
  // handle 1D textures very well
  glGenTextures (1, &m_fogtexturehandle);
  csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D (GL_TEXTURE_2D, 0, 4, FOGTABLE_SIZE, 1, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, transientfogdata);

  delete [] transientfogdata;

  glGetIntegerv (GL_MAX_TEXTURE_SIZE, &max_texture_size);
  // adjust max texture size if bigger than maxwidth/height from config
  if(Caps.maxTexWidth < max_texture_size)
    max_texture_size = Caps.maxTexWidth;
  if(Caps.maxTexHeight < max_texture_size)
    max_texture_size = Caps.maxTexHeight;

  int max_cache_size = 1024*1024*16; // 32mb combined cache
  texture_cache = new OpenGLTextureCache (max_cache_size, 24);
  lightmap_cache = new OpenGLLightmapCache (max_cache_size, 24);
  texture_cache->SetBilinearMapping (config->GetBool
        ("Video.OpenGL.EnableBilinearMap", true));

  GLenum errtest;
  errtest = glGetError ();
  if (errtest != GL_NO_ERROR)
  {
    //SysPrintf (MSG_DEBUG_0, "openGL error string: %s\n", gluErrorString (errtest));
    SysPrintf (MSG_DEBUG_0, "openGL error: %d\n", errtest);
  }

  // if blend style is 'auto' try to determine which mode to use by drawing on the
  // frame buffer.  We check the results to see if the OpenGL driver provides good
  // support for multipledouble (2*SRC*DST) blend mode; if not, fallback to the
  // normal multiply blend mode
  if (strcmp (lightmapstyle, "auto") == 0)
  {
    GLenum srcblend, dstblend;

    Guess_BlendMode (&srcblend, &dstblend);

    m_config_options.m_lightmap_src_blend = srcblend;
    m_config_options.m_lightmap_dst_blend = dstblend;
  }

  end_draw_poly ();

  glCullFace (GL_FRONT);
  glEnable (GL_CULL_FACE);

  // now that we know what pixelformat we use, clue the texture manager in
  txtmgr->SetPixelFormat (*G2D->GetPixelFormat ());

  PerfTest ();

  return true;
}

void csGraphics3DOGLCommon::CommonOpen ()
{
  DrawMode = 0;
  width = G2D->GetWidth ();
  height = G2D->GetHeight ();
  width2 = width / 2;
  height2 = height / 2;
  SetDimensions (width, height);
  // default lightmap blend style
  m_config_options.m_lightmap_src_blend = GL_DST_COLOR;
  m_config_options.m_lightmap_dst_blend = GL_ZERO;
}

void csGraphics3DOGLCommon::SharedOpen (csGraphics3DOGLCommon *d)
{
  CommonOpen ();
  ARB_multitexture = d->ARB_multitexture;
  m_config_options.do_multitexture_level =
    d->m_config_options.do_multitexture_level;
  m_config_options.do_extra_bright = d->m_config_options.do_extra_bright;
  m_config_options.m_lightmap_src_blend = d->m_config_options.m_lightmap_src_blend;
  m_config_options.m_lightmap_dst_blend = d->m_config_options.m_lightmap_dst_blend;
  m_fogtexturehandle = d->m_fogtexturehandle;
  texture_cache = d->texture_cache;
  lightmap_cache = d->lightmap_cache;
}

void csGraphics3DOGLCommon::Close ()
{
  if ((width == height) && height == -1)
    return;

  // we should remove all texture handles before we kill the graphics context
  delete txtmgr; txtmgr = NULL;
  delete texture_cache; texture_cache = NULL;
  delete lightmap_cache; lightmap_cache = NULL;
  if (clipper) { clipper->DecRef (); clipper = NULL; cliptype = CS_CLIPPER_NONE; }

  if (m_fogtexturehandle)
  {
    glDeleteTextures (1, &m_fogtexturehandle);
    m_fogtexturehandle = 0;
  }
  // kill the graphics context
  if (G2D)
    G2D->Close ();

  width = height = -1;
}

void csGraphics3DOGLCommon::SetDimensions (int width, int height)
{
  csGraphics3DOGLCommon::width = width;
  csGraphics3DOGLCommon::height = height;
  csGraphics3DOGLCommon::width2 = width / 2;
  csGraphics3DOGLCommon::height2 = height / 2;
  frustum_valid = false;
}

void csGraphics3DOGLCommon::SetupStencil ()
{
  if (stencil_init) return;
  stencil_init = true;
  if (clipper && GLCaps.use_stencil)
  {
    // First set up the stencil area.
    glEnable (GL_STENCIL_TEST);
    glClearStencil (0);
    glClear (GL_STENCIL_BUFFER_BIT);
    glStencilFunc (GL_ALWAYS, 1, 1);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    int nv = clipper->GetNumVertices ();
    csVector2* v = clipper->GetClipPoly ();
    glColor4f (0, 0, 0, 0);
    glShadeModel (GL_FLAT);
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_DEPTH_TEST);
    glEnable (GL_BLEND);
    glBlendFunc (GL_ZERO, GL_ONE);
    glBegin (GL_TRIANGLE_FAN);
    for (int i = 0 ; i < nv ; i++)
      glVertex2f (v[i].x, v[i].y);
    glEnd ();
    glDisable (GL_STENCIL_TEST);
  }
}

void csGraphics3DOGLCommon::SetupClipPlanes ()
{
  if (planes_init) return;
  planes_init = true;
  // This routine assumes the hardware planes can handle the
  // required number of planes from the clipper.
  if (clipper && GLCaps.nr_hardware_planes >= 0)
  {
    CalculateFrustum ();
    csPlane3 pl;
    GLdouble plane_eq[4];
    int i, i1;
    i1 = frustum.GetNumVertices ()-1;
    for (i = 0 ; i < frustum.GetNumVertices () ; i++)
    {
      pl.Set (csVector3 (0), frustum[i], frustum[i1]);
      plane_eq[0] = pl.A ();
      plane_eq[1] = pl.B ();
      plane_eq[2] = pl.C ();
      plane_eq[3] = pl.D ();
      glClipPlane (GL_CLIP_PLANE0+i, plane_eq);
      i1 = i;
    }
    plane_eq[0] = 0;
    plane_eq[1] = 0;
    plane_eq[2] = 1;
    plane_eq[3] = -.001;
    glClipPlane (GL_CLIP_PLANE0+i, plane_eq);
  }
}

void csGraphics3DOGLCommon::CalculateFrustum ()
{
  if (frustum_valid) return;
  frustum_valid = true;
  if (clipper)
  {
    frustum.MakeEmpty ();
    int nv = clipper->GetNumVertices ();
    csVector3 v3;
    v3.z = 1;
    csVector2* v = clipper->GetClipPoly ();
    int i, i1;
    i1 = nv-1;
    for (i = 0 ; i < nv ; i++)
    {
      v3.x = (v[i].x - width2) * inv_aspect;
      v3.y = (v[i].y - height2) * inv_aspect;
      frustum.AddVertex (v3);
      i1 = i;
    }
  }
}

void csGraphics3DOGLCommon::SetClipper (iClipper2D* clip, int cliptype)
{
  if (clip) clip->IncRef ();
  if (clipper) clipper->DecRef ();
  clipper = clip;
  if (!clipper) cliptype = CS_CLIPPER_NONE;
  csGraphics3DOGLCommon::cliptype = cliptype;
  frustum_valid = false;
  stencil_init = false;
  planes_init = false;

#if 0
// @@@ TODO: init z-buffer
  if (!toplevel_init && cliptype == CS_CLIPPER_TOPLEVEL)
  {
    // We have a toplevel clipper and we didn't initialize it yet.
    // In this case we will update the Z-buffer around the top-level
    // portal so that we don't need clipping for DrawTriangleMesh.
    // @@@ SideNote! This toplevel_init could cause problems for multiple
    // views. Have to think about this.
    toplevel_init = true;
    int nv = clipper->GetNumVertices ();
    csVector2* v = clipper->GetClipPoly ();
    int i, i1;
    i1 = nv-1;
    for (i = 0 ; i < nv ; i++)
    {
      i1 = i;
    }
  }
#endif
}

bool csGraphics3DOGLCommon::BeginDraw (int DrawFlags)
{
  // if 2D graphics is not locked, lock it
  if ((DrawFlags & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
   && (!(DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))))
  {
    if (!G2D->BeginDraw ())
      return false;

    dbg_current_polygon = 0;
  }

  if (DrawFlags & CSDRAW_CLEARZBUFFER)
  {
    glDepthMask (GL_TRUE);
    if (DrawFlags & CSDRAW_CLEARSCREEN)
      glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    else
      glClear (GL_DEPTH_BUFFER_BIT);
  }
  else if (DrawFlags & CSDRAW_CLEARSCREEN)
    G2D->Clear (0);

  DrawMode = DrawFlags;

  end_draw_poly ();

  toplevel_init = false;

  return true;
}

void csGraphics3DOGLCommon::FinishDraw ()
{
  // ASSERT( G2D );


  if (DrawMode & (CSDRAW_2DGRAPHICS | CSDRAW_3DGRAPHICS))
  {
    G2D->FinishDraw ();
  }
  DrawMode = 0;
}

void csGraphics3DOGLCommon::Print (csRect * area)
{
  G2D->Print (area);
  // glClear(GL_COLOR_BUFFER_BIT);
}

static float SetupBlend (UInt mode, float m_alpha, bool has_alpha)
{
  // Note: In all explanations of Mixing:
  // Color: resulting color
  // SRC:   Color of the texel (content of the texture to be drawn)
  // DEST:  Color of the pixel on screen
  // Alpha: Alpha value of the polygon
  bool enable_blending = true;
  switch (mode & CS_FX_MASK_MIXMODE)
  {
    case CS_FX_MULTIPLY:
      // Color = SRC * DEST +   0 * SRC = DEST * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ZERO, GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_MULTIPLY2:
      // Color = SRC * DEST + DEST * SRC = 2 * DEST * SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_ADD:
      // Color = 1 * DEST + 1 * SRC = DEST + SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_ONE, GL_ONE);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_ALPHA:
      // Color = Alpha * DEST + (1-Alpha) * SRC
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_TRANSPARENT:
      // Color = 1 * DEST + 0 * SRC
      m_alpha = 0.0f;
      glBlendFunc (GL_ZERO, GL_ONE);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
    case CS_FX_COPY:
    default:
      enable_blending = has_alpha;
      // Color = 0 * DEST + 1 * SRC = SRC
      m_alpha = 1.0f;
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      break;
  }

  if (enable_blending)
    glEnable (GL_BLEND);
  else
    glDisable (GL_BLEND);
  return m_alpha;
}


#define SMALL_D 0.01

/**
 * The engine often generates "empty" polygons, for example
 * (2, 2) - (317,2) - (317,2) - (2, 2)
 * To avoid too much computations, DrawPolygon detects such polygons by
 * counting the number of "real" vertices (i.e. the number of vertices,
 * distance between which is bigger that some amount). The "right" formula
 * for distance is sqrt(dX^2 + dY^2) but to avoid root and multiply
 * DrawPolygon checks abs(dX) + abs(dY). This is enough.
 */
#define VERTEX_NEAR_THRESHOLD   0.001

void csGraphics3DOGLCommon::DrawPolygonSingleTexture (G3DPolygonDP & poly)
{
  if (poly.num < 3)
    return;

  int i;
  float P1, P2, P3, P4;
  float Q1, Q2, Q3, Q4;

  // take shortcut drawing if possible
  if (ShortcutDrawPolygon && (this->*ShortcutDrawPolygon) (poly))
    return;

  // For debugging: is we reach the maximum number of polygons to draw
  // we simply stop.
  dbg_current_polygon++;
  if (dbg_current_polygon == dbg_max_polygons_to_draw - 1)
    return;
  if (dbg_current_polygon >= dbg_max_polygons_to_draw - 1)
    return;

  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1; i < poly.num; i++)
  {
    if ((ABS (poly.vertices[i].sx - poly.vertices[i - 1].sx)
	 + ABS (poly.vertices[i].sy - poly.vertices[i - 1].sy))
	 	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }
  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means
    // that  the plane of the polygon is almost perpendicular to the
    // eye of the viewer. In this case, nothing much can be seen of
    // the plane anyway so we just take one value for the entire
    // polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  iPolygonTexture *tex = poly.poly_texture;
  csMaterialHandle* mat_handle = (csMaterialHandle*)poly.mat_handle;
  iTextureHandle* txt_handle = mat_handle->GetTexture ();
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();

  // find lightmap information, if any
  iLightMap *thelightmap = tex->GetLightMap ();

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  CacheTexture (tex);

  // @@@ The texture transform matrix is currently written as
  // T = M*(C-V)
  // (with V being the transform vector, M the transform matrix, and C
  // the position in camera space coordinates. It would be better (more
  // suitable for the following calculations) if it would be written
  // as T = M*C - V.
  P1 = poly.plane.m_cam2tex->m11;
  P2 = poly.plane.m_cam2tex->m12;
  P3 = poly.plane.m_cam2tex->m13;
  P4 = -(P1 * poly.plane.v_cam2tex->x
	 + P2 * poly.plane.v_cam2tex->y
	 + P3 * poly.plane.v_cam2tex->z);
  Q1 = poly.plane.m_cam2tex->m21;
  Q2 = poly.plane.m_cam2tex->m22;
  Q3 = poly.plane.m_cam2tex->m23;
  Q4 = -(Q1 * poly.plane.v_cam2tex->x
	 + Q2 * poly.plane.v_cam2tex->y
	 + Q3 * poly.plane.v_cam2tex->z);

  // Precompute everything so that we can calculate (u,v) (texture space
  // coordinates) for every (sx,sy) (screen space coordinates). We make
  // use of the fact that 1/z, u/z and v/z are linear in screen space.
  float J1, J2, J3, K1, K2, K3;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane of the polygon is too small.
    J1 = J2 = J3 = 0;
    K1 = K2 = K3 = 0;
  }
  else
  {
    J1 = P1 * inv_aspect + P4 * M;
    J2 = P2 * inv_aspect + P4 * N;
    J3 = P3 + P4 * O;
    K1 = Q1 * inv_aspect + Q4 * M;
    K2 = Q2 * inv_aspect + Q4 * N;
    K3 = Q3 + Q4 * O;
  }

  bool tex_transp;
  int poly_alpha = poly.alpha;

  csGLCacheData *texturecache_data;
  texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
  tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
  GLuint texturehandle = texturecache_data->Handle;

  float flat_r = 1., flat_g = 1., flat_b = 1.;

  glShadeModel (GL_FLAT);
  if (m_renderstate.textured)
    glEnable (GL_TEXTURE_2D);
  else
  {
    glDisable (GL_TEXTURE_2D);
    UByte r, g, b;
    poly.mat_handle->GetTexture ()->GetMeanColor (r, g, b);
    flat_r = BYTE_TO_FLOAT (r);
    flat_g = BYTE_TO_FLOAT (g);
    flat_b = BYTE_TO_FLOAT (b);
  }

  SetGLZBufferFlags ();

  if (poly_alpha == 0 && poly.mixmode != CS_FX_COPY)
  {
    float alpha = 1.0f - BYTE_TO_FLOAT (poly_alpha);
    bool has_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
    alpha = SetupBlend (poly.mixmode, alpha, has_alpha);
    glColor4f (flat_r, flat_g, flat_b, alpha);
  }
  else if ((poly_alpha > 0) || tex_transp)
  {
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    if (poly_alpha > 0)
      glColor4f (flat_r, flat_g, flat_b, 1.0 - BYTE_TO_FLOAT (poly_alpha));
    else
      glColor4f (flat_r, flat_g, flat_b, 1.0);
  }
  else
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
  }

  csglBindTexture (GL_TEXTURE_2D, texturehandle);

  float sx, sy, sz, one_over_sz, u_over_sz, v_over_sz;

  // Check if there was a DrawPolygon the previous time.
  start_draw_poly ();

  // First copy all data in an array so that we can minimize
  // the amount of code that goes between glBegin/glEnd. This
  // is from an OpenGL high-performance FAQ.
  // @@@ HARDCODED LIMIT OF 64 VERTICES!
  static GLfloat glverts[4*64];
  static GLfloat gltxt[2*64];
  int vtidx = 0;
  int txtidx = 0;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    // we must communicate the perspective correction (1/z) for
    // textures by using homogenous coordinates in either texture
    // space or in object (vertex) space.  We do it in texture
    // space.
    // glTexCoord4f(u_over_sz,v_over_sz,one_over_sz,one_over_sz);
    // glVertex3f(poly.vertices[i].sx, poly.vertices[i].sy,
    // -one_over_sz);

    // modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates
    gltxt[txtidx++] = u_over_sz * sz;
    gltxt[txtidx++] = v_over_sz * sz;
    glverts[vtidx++] = poly.vertices[i].sx * sz;
    glverts[vtidx++] = poly.vertices[i].sy * sz;
    glverts[vtidx++] = -1.0;
    glverts[vtidx++] = sz;
  }

  GLfloat* p_gltxt, * p_glverts;
  p_gltxt = gltxt;
  p_glverts = glverts;

  glBegin (GL_TRIANGLE_FAN);
  for (i = 0 ; i < poly.num ; i++)
  {
    glTexCoord2fv (p_gltxt); p_gltxt += 2;
    glVertex4fv (p_glverts); p_glverts += 4;
  }
  glEnd ();

  // If we have need other texture passes (for whatever reason)
  // we set the z-buffer to second pass state.
  if ((thelightmap && m_renderstate.lighting) ||
      m_config_options.do_extra_bright ||
      poly.use_fog ||
      mat_handle->GetNumTextureLayers () > 0)
    SetGLZBufferFlagsPass2 (false);

  // Here we add all extra texture layers if there are some.
  static GLfloat layer_gltxt[2*64];
  int j;
  for (j = 0 ; j < mat_handle->GetNumTextureLayers () ; j++)
  {
    csTextureLayer* layer = mat_handle->GetTextureLayer (j);
    iTextureHandle* txt_handle = layer->txt_handle;
    csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();
    csGLCacheData *texturecache_data;
    texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
    //tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
    GLuint texturehandle = texturecache_data->Handle;
    csglBindTexture (GL_TEXTURE_2D, texturehandle);

    float alpha = 1.0f - BYTE_TO_FLOAT (layer->mode & CS_FX_MASK_ALPHA);
    alpha = SetupBlend (layer->mode, alpha, false);
    glColor4f (1., 1., 1., alpha);
    if (mat_handle->TextureLayerTranslated (j))
    {
      GLfloat* src = gltxt;
      p_gltxt = layer_gltxt;
      int uscale = layer->uscale;
      int vscale = layer->vscale;
      int ushift = layer->ushift;
      int vshift = layer->vshift;
      for (i = 0 ; i < poly.num ; i++)
      {
	*p_gltxt++ = (*src++) * uscale + ushift;
	*p_gltxt++ = (*src++) * vscale + vshift;
      }
      p_gltxt = layer_gltxt;
    }
    else
    {
      p_gltxt = gltxt;
    }
    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    for (i = 0 ; i < poly.num ; i++)
    {
      glTexCoord2fv (p_gltxt); p_gltxt += 2;
      glVertex4fv (p_glverts); p_glverts += 4;
    }
    glEnd ();
  }

  // next draw the lightmap over the texture.  The two are blended
  // together. If a lightmap exists, extract the proper
  // data (GL handle, plus texture coordinate bounds)
  if (thelightmap && m_renderstate.lighting)
  {
    csGLCacheData *lightmapcache_data = (csGLCacheData *)thelightmap->GetCacheData ();
    GLuint lightmaphandle = lightmapcache_data->Handle;

    csglBindTexture (GL_TEXTURE_2D, lightmaphandle);
    glEnable (GL_TEXTURE_2D);

    glEnable (GL_BLEND);
    // The following blend function is configurable.
    glBlendFunc (m_config_options.m_lightmap_src_blend,
		 m_config_options.m_lightmap_dst_blend);

    txtidx = 0;
    float lm_scale_u = lightmapcache_data->lm_scale_u;
    float lm_scale_v = lightmapcache_data->lm_scale_v;
    float lm_offset_u = lightmapcache_data->lm_offset_u;
    float lm_offset_v = lightmapcache_data->lm_offset_v;
    for (i = 0; i < poly.num; i++)
    {
      gltxt[txtidx] = (gltxt[txtidx] - lm_offset_u) * lm_scale_u;
      txtidx++;
      gltxt[txtidx] = (gltxt[txtidx] - lm_offset_v) * lm_scale_v;
      txtidx++;
    }

    p_gltxt = gltxt;
    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    for (i = 0 ; i < poly.num ; i++)
    {
      glTexCoord2fv (p_gltxt); p_gltxt += 2;
      glVertex4fv (p_glverts); p_glverts += 4;
    }
    glEnd ();
  }

  // An extra pass which improves the lighting on SRC*DST so that
  // it looks more like 2*SRC*DST.
  if (m_config_options.do_extra_bright)
  {
    glDisable (GL_TEXTURE_2D);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
    // glBlendFunc (GL_ZERO, GL_SRC_COLOR);

    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    glColor4f (2, 2, 2, 0);
    for (i = 0; i < poly.num; i++)
    {
      glVertex4fv (p_glverts);
      p_glverts += 4;
    }
    glEnd ();
  }
  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glShadeModel (GL_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    for (i = 0; i < poly.num; i++)
    {
      // Formula for fog is:
      // C = I * F + (1-I) * P
      // I = intensity = density * thickness
      // F = fog color
      // P = texture color
      // C = destination color

      // specify fog vertex color
      glColor3f (poly.fog_info[i].r, poly.fog_info[i].g, poly.fog_info[i].b);

      // specify fog vertex alpha value using the fog table and fog
      // distance
      if (poly.fog_info[i].intensity > FOG_MAXVALUE)
	glTexCoord2f (FOGTABLE_CLAMPVALUE,0.0);
      else
	glTexCoord2f (poly.fog_info[i].intensity / FOGTABLE_MAXDISTANCE, 0.0);

      // specify fog vertex location
      glVertex4fv (p_glverts); p_glverts += 4;
    }
    glEnd ();

  }
}

void csGraphics3DOGLCommon::DrawPolygonZFill (G3DPolygonDP & poly)
{
  if (poly.num < 3)
    return;

  int i;

  // count 'real' number of vertices
  int num_vertices = 1;
  for (i = 1; i < poly.num; i++)
  {
    if ((ABS (poly.vertices[i].sx - poly.vertices[i - 1].sx)
	 + ABS (poly.vertices[i].sy - poly.vertices[i - 1].sy))
	 	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }
  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return;

  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means
    // that  the plane of the polygon is almost perpendicular to the
    // eye of the viewer. In this case, nothing much can be seen of
    // the plane anyway so we just take one value for the entire
    // polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  glDisable (GL_TEXTURE_2D);
  glShadeModel (GL_FLAT);
  SetGLZBufferFlags ();
  glBlendFunc (GL_ZERO, GL_ONE);
  glEnable (GL_BLEND);

  // First copy all data in an array so that we can minimize
  // the amount of code that goes between glBegin/glEnd. This
  // is from an OpenGL high-performance FAQ.
  // @@@ HARDCODED LIMIT OF 64 VERTICES!
  static GLfloat glverts[4*64];
  int vtidx = 0;
  float sx, sy, sz, one_over_sz;
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    glverts[vtidx++] = poly.vertices[i].sx * sz;
    glverts[vtidx++] = poly.vertices[i].sy * sz;
    glverts[vtidx++] = -1.0;
    glverts[vtidx++] = sz;
  }

  GLfloat* p_glverts;
  p_glverts = glverts;

  glBegin (GL_TRIANGLE_FAN);
  for (i = 0 ; i < poly.num ; i++)
  {
    glVertex4fv (p_glverts); p_glverts += 4;
  }
  glEnd ();
}

void csGraphics3DOGLCommon::DrawPolygonDebug (G3DPolygonDP &/* poly */ )
{
}

// Calculate round (f) of a 16:16 fixed pointer number
// and return a long integer.
inline long
round16 (long f)
{
  return (f + 0x8000) >> 16;
}

//static iTextureHandle* prev_handle = NULL;
//static UInt prev_mode = ~0;

void csGraphics3DOGLCommon::StartPolygonFX (iMaterialHandle * handle, UInt mode)
{
  end_draw_poly ();

//@@@ Experimental!!!
// If the following code is enabled then speed goes up considerably in some
// cases. However this doesn't work very well. We need a global state mechanism
// because other functions may mess up the state as well.
//  if (handle == prev_handle && mode == prev_mode) return;
//  prev_handle = handle;
//  prev_mode = mode;


  // try shortcut method if available
  if (ShortcutStartPolygonFX && (this ->* ShortcutStartPolygonFX) (handle, mode))
    return;

  m_gouraud = m_renderstate.lighting && m_renderstate.gouraud && ((mode & CS_FX_GOURAUD) != 0);
  m_mixmode = mode;
  m_alpha = 1.0f - BYTE_TO_FLOAT (mode & CS_FX_MASK_ALPHA);

  GLuint texturehandle = 0;
  bool has_alpha = false;
  m_multimat = NULL;
  if (handle && m_renderstate.textured)
  {
    CacheTexture (handle);
    iTextureHandle* txt_handle = handle->GetTexture ();
    csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();
    csGLCacheData *cachedata = (csGLCacheData *)txt_mm->GetCacheData ();
    texturehandle = cachedata->Handle;
    if (txt_handle)
      has_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
    if (((csMaterialHandle*)handle)->GetNumTextureLayers () > 0)
      m_multimat = (csMaterialHandle*)handle;
  }
  if ((mode & CS_FX_MASK_MIXMODE) == CS_FX_ALPHA)
    m_gouraud = true;

  // set proper shading: flat is typically faster, but we need smoothing
  // enabled when doing gouraud shading -GJH
  if (m_gouraud)
    glShadeModel (GL_SMOOTH);
  else
    glShadeModel (GL_FLAT);

  m_alpha = SetupBlend (mode, m_alpha, has_alpha);

  m_textured = (texturehandle != 0);
  if (m_textured)
  {
    csglBindTexture (GL_TEXTURE_2D, texturehandle);
    glEnable (GL_TEXTURE_2D);
  }
  else
    glDisable (GL_TEXTURE_2D);

  SetGLZBufferFlags ();
}

void csGraphics3DOGLCommon::FinishPolygonFX ()
{
  // attempt shortcut method
  if (ShortcutFinishPolygonFX && (this ->* ShortcutFinishPolygonFX) ())
    return;
}

void csGraphics3DOGLCommon::DrawPolygonFX (G3DPolygonDPFX & poly)
{
  // take shortcut if possible
  if (ShortcutDrawPolygonFX && (this ->* ShortcutDrawPolygonFX) (poly))
    return;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (!m_textured)
  {
    flat_r = BYTE_TO_FLOAT (poly.flat_color_r);
    flat_g = BYTE_TO_FLOAT (poly.flat_color_g);
    flat_b = BYTE_TO_FLOAT (poly.flat_color_b);
  }
  int i;

  // First we calculate all information for the polygon
  // and put it in buffers. That makes it faster to process later.
  static GLfloat glverts[4*64];
  static GLfloat gltxt[2*64];
  static GLfloat glcol[4*64];
  int vtidx = 0;
  int txtidx = 0;
  int colidx = 0;
  float sx, sy;
  for (i = 0; i < poly.num; i++)
  {
    gltxt[txtidx++] = poly.vertices[i].u;
    gltxt[txtidx++] = poly.vertices[i].v;
    if (m_gouraud)
    {
      glcol[colidx++] = flat_r * poly.vertices[i].r;
      glcol[colidx++] = flat_g * poly.vertices[i].g;
      glcol[colidx++] = flat_b * poly.vertices[i].b;
      glcol[colidx++] = m_alpha;
    }
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    float sz = poly.vertices[i].z;
    if (ABS (sz) < SMALL_EPSILON) sz = 1.0/ SMALL_EPSILON;
    else sz = 1./sz;
    glverts[vtidx++] = poly.vertices[i].sx * sz;
    glverts[vtidx++] = poly.vertices[i].sy * sz;
    glverts[vtidx++] = -1;
    glverts[vtidx++] = sz;
  }

  // Now we actually draw the first pass. If we have a multi-pass
  // texture then we draw the first polygon without gouraud shading
  // (even if it should have some) and add the gouraud shading later.
  GLfloat* p_gltxt, * p_glverts, * p_glcol;
  p_gltxt = gltxt;
  p_glverts = glverts;
  p_glcol = glcol;
  bool do_gouraud = m_gouraud;
  if (m_multimat) do_gouraud = false;
  if (!do_gouraud)
    glColor4f (flat_r, flat_g, flat_b, m_alpha);
  glBegin (GL_TRIANGLE_FAN);
  for (i = 0 ; i < poly.num ; i++)
  {
    glTexCoord2fv (p_gltxt); p_gltxt += 2;
    if (do_gouraud)
    {
      glColor4fv (p_glcol); p_glcol += 4;
    }
    glVertex4fv (p_glverts); p_glverts += 4;
  }
  glEnd ();

  // If we have extra passes (for multi-texture or fog or whatever)
  // we first remember all attributes and set Z-buffer ok.
  bool multipass =
    	poly.use_fog ||
	m_multimat;
  if (multipass)
  {
    // let OpenGL save old values for us
    // -GL_COLOR_BUFFER_BIT saves blend types among other things,
    // -GL_DEPTH_BUFFER_BIT saves depth test function
    // -GL_TEXTURE_BIT saves current texture handle
    glPushAttrib ( GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT );
    SetGLZBufferFlagsPass2 (false);
  }

  if (m_multimat)
  {
    glShadeModel (GL_FLAT);
    static GLfloat layer_gltxt[2*64];
    int j;
    for (j = 0 ; j < m_multimat->GetNumTextureLayers () ; j++)
    {
      csTextureLayer* layer = m_multimat->GetTextureLayer (j);
      iTextureHandle* txt_handle = layer->txt_handle;
      csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();
      csGLCacheData *texturecache_data;
      texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
      //tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
      GLuint texturehandle = texturecache_data->Handle;
      csglBindTexture (GL_TEXTURE_2D, texturehandle);
      float alpha = 1.0f - BYTE_TO_FLOAT (layer->mode & CS_FX_MASK_ALPHA);
      alpha = SetupBlend (layer->mode, alpha, false);
      glColor4f (1., 1., 1., alpha);
      if (m_multimat->TextureLayerTranslated (j))
      {
        GLfloat* src = gltxt;
        p_gltxt = layer_gltxt;
        int uscale = layer->uscale;
        int vscale = layer->vscale;
        int ushift = layer->ushift;
        int vshift = layer->vshift;
        for (i = 0 ; i < poly.num ; i++)
        {
	  *p_gltxt++ = (*src++) * uscale + ushift;
	  *p_gltxt++ = (*src++) * vscale + vshift;
        }
        p_gltxt = layer_gltxt;
      }
      else
      {
        p_gltxt = gltxt;
      }
      p_glverts = glverts;
      glBegin (GL_TRIANGLE_FAN);
      for (i = 0 ; i < poly.num ; i++)
      {
        glTexCoord2fv (p_gltxt); p_gltxt += 2;
        glVertex4fv (p_glverts); p_glverts += 4;
      }
      glEnd ();
    }

    // If we have to do gouraud shading we do it here.
    if (m_gouraud)
    {
      glDisable (GL_TEXTURE_2D);
      glShadeModel (GL_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBegin (GL_TRIANGLE_FAN);
      p_glverts = glverts;
      p_glcol = glcol;
      for (i = 0 ; i < poly.num ; i++)
      {
        glColor4fv (p_glcol); p_glcol += 4;
        glVertex4fv (p_glverts); p_glverts += 4;
      }
      glEnd ();
    }
  }

  // If there is vertex fog then we apply that last.
  if (poly.use_fog)
  {
    // we need to texture and blend, without vertex color
    // interpolation
    glEnable (GL_TEXTURE_2D);
    csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    glDepthFunc (GL_EQUAL);
    glShadeModel (GL_SMOOTH);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    p_glverts = glverts;
    glBegin (GL_TRIANGLE_FAN);
    for (i = 0; i < poly.num; i++)
    {
      sx = poly.vertices[i].sx - width2;
      sy = poly.vertices[i].sy - height2;

      // Formula for fog is:
      // C = I * F + (1-I) * P
      // I = intensity = 1-exp(density * thickness)
      // F = fog color
      // P = texture color
      // C = destination color
      // the '1-exp' part is embodied in the fog texture; view the
      // fog texture as a function producing the alpha value, with
      // input (texture coordinate) being equal to I.

      // specify fog vertex color
      glColor3f (poly.fog_info[i].r, poly.fog_info[i].g, poly.fog_info[i].b);

      // specify fog vertex transparency
      float I = poly.fog_info[i].intensity;
      if (I > FOG_MAXVALUE)
	I = FOGTABLE_CLAMPVALUE;
      else
	I = I * FOGTABLE_DISTANCESCALE;
      glTexCoord2f (I, I);

      // specify fog vertex location
      glVertex4fv (p_glverts); p_glverts += 4;
    }
    glEnd ();
  }

  if (multipass)
  {
    // Restore state (blend mode, texture handle, etc.) for next
    // triangle
    glPopAttrib ();
    if (!m_textured)
      glDisable (GL_TEXTURE_2D);
    if (!m_gouraud)
      glShadeModel (GL_FLAT);
  }
}

// Find out the location of a vertex by recursing through the
// clipinfo tree.
static void ResolveVertex (
	csClipInfo* ci,
	int* clipped_translate,
	csVector3* overts, csVector2* otexels,
	csColor* ocolors, G3DFogInfo* ofog,
	csVector2& texel, csColor& color, G3DFogInfo& fog)
{
  switch (ci->type)
  {
    case CS_CLIPINFO_ORIGINAL:
//printf ("  ORIGINAL idx=%d\n", ci->original.idx); fflush (stdout);
      texel = otexels[ci->original.idx];
      if (ocolors) color = ocolors[ci->original.idx];
      if (ofog) fog = ofog[ci->original.idx];
      break;
    case CS_CLIPINFO_ONEDGE:
    {
      int i1 = ci->onedge.i1;
      int i2 = ci->onedge.i2;
      float r = ci->onedge.r;
//printf ("  ONEDGE i1=%d i2=%d r=%g\n", i1, i2, r); fflush (stdout);
      texel = otexels[i1] * (1-r) + otexels[i2] * r;
      if (ocolors)
      {
	color.red = ocolors[i1].red * (1-r) + ocolors[i2].red * r;
	color.green = ocolors[i1].green * (1-r) + ocolors[i2].green * r;
	color.blue = ocolors[i1].blue * (1-r) + ocolors[i2].blue * r;
      }
      if (ofog)
      {
	fog.intensity = ofog[i1].intensity*(1-r)+ofog[i2].intensity*r;
	fog.r = ofog[i1].r * (1-r) + ofog[i2].r * r;
	fog.g = ofog[i1].g * (1-r) + ofog[i2].g * r;
	fog.b = ofog[i1].b * (1-r) + ofog[i2].b * r;
      }
      break;
    }
    case CS_CLIPINFO_INSIDE:
    {
//printf ("  INSIDE r=%g\n", ci->inside.r); fflush (stdout);
      csVector2 texel1, texel2;
      csColor color1, color2;
      G3DFogInfo fog1, fog2;
      ResolveVertex (ci->inside.ci1, clipped_translate, overts, otexels,
		ocolors, ofog, texel1, color1, fog1);
      ResolveVertex (ci->inside.ci2, clipped_translate, overts, otexels,
		ocolors, ofog, texel2, color2, fog2);
      delete ci->inside.ci1;
      delete ci->inside.ci2;
      ci->type = CS_CLIPINFO_ORIGINAL;
      float r = ci->inside.r;
      texel = texel1 * (1-r) + texel2 * r;
      if (ocolors)
      {
	color.red = color1.red * (1-r) + color2.red * r;
	color.green = color1.green * (1-r) + color2.green * r;
	color.blue = color1.blue * (1-r) + color2.blue * r;
      }
      if (ofog)
      {
	fog.intensity = fog1.intensity*(1-r)+fog2.intensity*r;
	fog.r = fog1.r * (1-r) + fog2.r * r;
	fog.g = fog1.g * (1-r) + fog2.g * r;
	fog.b = fog1.b * (1-r) + fog2.b * r;
      }
      break;
    }
  }
}

void csGraphics3DOGLCommon::ClipTriangleMesh (
    int num_triangles,
    int num_vertices,
    csTriangle* triangles,
    csVector3* vertices,
    csVector2* texels,
    csColor* vertex_colors,
    G3DFogInfo* vertex_fog,
    int& num_clipped_triangles,
    int& num_clipped_vertices,
    bool exact_clipping)
{
  // Make sure the frustum is ok.
  CalculateFrustum ();

  // Now calculate the frustum as seen in object space for the given
  // mesh.
  csPlane3 frustum_planes[100];	// @@@ Arbitrary number.
  csPoly3D obj_frustum;
  int i, j, j1;
  for (i = 0 ; i < frustum.GetNumVertices () ; i++)
    obj_frustum.AddVertex (o2c.This2OtherRelative (frustum[i]));
  int num_frust = frustum.GetNumVertices ();
  j1 = num_frust-1;
  for (j = 0 ; j < num_frust ; j++)
  {
    frustum_planes[j].Set (csVector3 (0), obj_frustum[j], obj_frustum[j1]);
    j1 = j;
  }
  csVector3 frust_origin = o2c.This2Other (csVector3 (0));
  // In addition to the frustum planes itself we also calculate all
  // diagonal planes which go from one side of the frustum to the other.
  // These are going to be used to detect the special case of a triangle
  // that has none of its vertices in the frustum. But this triangle can
  // still be visible. To detect this we test if there is one of these
  // extra planes that cuts the triangle in two. Since the number of diagonal
  // planes is half the number of frustum planes we can save some
  // calculation here.
  csPlane3 diagonal_planes[50];	// @@@ Arbitrary number.
  int num_diagonal_planes = 0;
  if (num_frust > 3)
    // Use (num_frust+1)/2 to make sure that odd frustums get one extra plane.
    for (j = 0 ; j < (num_frust+1) / 2 ; j++)
    {
      j1 = j + (num_frust+1) / 2;
      j1 = j1 % num_frust;
      diagonal_planes[num_diagonal_planes++].Set
      	(csVector3 (0), obj_frustum[j], obj_frustum[j1]);
    }

  // Setup the viewing plane.
  //@@@frustum_planes[num_frust].Set (0, 0, -1, .01);

  // Make sure our worktables are big enough for the clipped mesh.
  int num_tri = num_triangles*2;
  if (num_tri < 50) num_tri += 50;	// Increase for low # of triangles.
  if (num_tri > clipped_triangles.Limit ())
  {
    // Use two times as many triangles. Hopefully this is enough.
    clipped_triangles.SetLimit (num_tri);
  }
  if (num_vertices > clipped_translate.Limit ())
  {
    clipped_translate.SetLimit (num_vertices);	// Used for original vertices.
    clipped_vertices.SetLimit (num_vertices*2);
    clipped_texels.SetLimit (num_vertices*2);
    clipped_colors.SetLimit (num_vertices*2);
    clipped_fog.SetLimit (num_vertices*2);
  }

  num_clipped_triangles = 0;
  num_clipped_vertices = 0;

  // Check all original vertices and see if they are in frustum.
  // If yes we set clipped_translate to the new position in the transformed
  // vertex array. Otherwise we set clipped_translate to -1.
  for (i = 0 ; i < num_vertices ; i++)
  {
    const csVector3& v = vertices[i];
    bool inside = true;
    j1 = num_frust-1;
    for (j = 0 ; j < num_frust ; j++)
    {
      if (csMath3::WhichSide3D (v-frust_origin, obj_frustum[j],
	    obj_frustum[j1]) >= 0)
      {
	inside = false;
	break;	// Not inside.
      }
      j1 = j;
    }
    if (inside)
    {
      if (exact_clipping)
      {
        clipped_translate[i] = num_clipped_vertices;
        clipped_vertices[num_clipped_vertices] = v;
        clipped_texels[num_clipped_vertices] = texels[i];
        if (vertex_colors)
          clipped_colors[num_clipped_vertices] = vertex_colors[i];
        if (vertex_fog)
          clipped_fog[num_clipped_vertices] = vertex_fog[i];
        num_clipped_vertices++;
      }
      else
        clipped_translate[i] = i;
    }
    else
      clipped_translate[i] = -1;
  }

  // Now clip all triangles.
  for (i = 0 ; i < num_triangles ; i++)
  {
    csTriangle& tri = triangles[i];
    int cnt = int (clipped_translate[tri.a] != -1)
      	+ int (clipped_translate[tri.b] != -1)
	+ int (clipped_translate[tri.c] != -1);
    if (cnt == 0)
    {
      //=====
      // Here we have a special case where we need to test if the
      // triangle is cut by the diagonal planes. If yes then we have
      // to clip anyway.
      //=====
      for (j = 0 ; j < num_diagonal_planes ; j++)
      {
        csPlane3& pl = diagonal_planes[j];
        csVector3 v0 = vertices[tri.a] - frust_origin;
        csVector3 v1 = vertices[tri.b] - frust_origin;
        csVector3 v2 = vertices[tri.c] - frust_origin;
	float c0 = pl.Classify (v0);
	float c1 = pl.Classify (v1);
	// Set cnt to 1 so that we will clip in the next part.
	if ((c0 < 0 && c1 > 0) || (c0 > 0 && c1 < 0)) { cnt = 1; break; }
	float c2 = pl.Classify (v2);
	if ((c0 < 0 && c2 > 0) || (c0 > 0 && c2 < 0)) { cnt = 1; break; }
	if ((c1 < 0 && c2 > 0) || (c1 > 0 && c2 < 0)) { cnt = 1; break; }
      }
    }

    if (cnt == 0)
    {
      //=====
      // Easiest case: triangle is not visible.
      //=====
    }
    else if (cnt == 3)
    {
      //=====
      // Easy case: the triangle is fully in view.
      //=====
      clipped_triangles[num_clipped_triangles].a = clipped_translate[tri.a];
      clipped_triangles[num_clipped_triangles].b = clipped_translate[tri.b];
      clipped_triangles[num_clipped_triangles].c = clipped_translate[tri.c];
      num_clipped_triangles++;
    }
    else
    {
      //=====
      // Difficult case: clipping will result in several triangles.
      //=====
      if (!exact_clipping)
      {
        // If we have lazy clipping then we just add the triangle.
        clipped_triangles[num_clipped_triangles].a = tri.a;
        clipped_triangles[num_clipped_triangles].b = tri.b;
        clipped_triangles[num_clipped_triangles].c = tri.c;
        num_clipped_triangles++;
	continue;
      }

      csVector3 poly[100];	// @@@ Arbitrary limit
      static csClipInfo clipinfo[100];
      poly[0] = vertices[tri.a] - frust_origin;
      poly[1] = vertices[tri.b] - frust_origin;
      poly[2] = vertices[tri.c] - frust_origin;
      clipinfo[0].type = CS_CLIPINFO_ORIGINAL; clipinfo[0].original.idx = tri.a;
      clipinfo[1].type = CS_CLIPINFO_ORIGINAL; clipinfo[1].original.idx = tri.b;
      clipinfo[2].type = CS_CLIPINFO_ORIGINAL; clipinfo[2].original.idx = tri.c;
      int num_poly = 3;

      //-----
      // First we clip the triangle to the planes of the frustum.
      // This will result in a polygon. The clipper keeps information
      // (in clipinfo) about what happens to all the vertices.
      //-----
      j1 = num_frust-1;
      for (j = 0 ; j < num_frust ; j++)
      {
        csVector3& v1 = obj_frustum[j1];
        csVector3& v2 = obj_frustum[j];
        csFrustum::ClipToPlane (poly, num_poly, clipinfo, v1, v2);
	if (num_poly <= 0) break;
	j1 = j;
      }

      //-----
      // First add all new vertices and resolve coordinates of texture
      // mapping and so on using the clipinfo.
      //-----
      for (j = 0 ; j < num_poly ; j++)
      {
	if (clipinfo[j].type == CS_CLIPINFO_ORIGINAL)
	{
	  clipinfo[j].original.idx =
	  	clipped_translate[clipinfo[j].original.idx];
	}
        else
	{
//printf ("================= j=%d\n", j); fflush (stdout);
	  ResolveVertex (&clipinfo[j], clipped_translate.GetArray (),
	  	vertices, texels, vertex_colors, vertex_fog,
		clipped_texels.GetArray ()[num_clipped_vertices],
		clipped_colors.GetArray ()[num_clipped_vertices],
		clipped_fog.GetArray ()[num_clipped_vertices]);
	  clipped_vertices[num_clipped_vertices] = poly[j]+frust_origin;
	  clipinfo[j].original.idx = num_clipped_vertices;
	  num_clipped_vertices++;
	}
      }

      //-----
      // Triangulate the resulting polygon.
      //-----
      for (j = 2 ; j < num_poly ; j++)
      {
        clipped_triangles[num_clipped_triangles].a = clipinfo[0].original.idx;
        clipped_triangles[num_clipped_triangles].b = clipinfo[j-1].original.idx;
        clipped_triangles[num_clipped_triangles].c = clipinfo[j].original.idx;
        num_clipped_triangles++;
      }
    }
  }
}

void csGraphics3DOGLCommon::DrawTriangleMesh (G3DTriangleMesh& mesh)
{
  end_draw_poly ();

  bool stencil_enabled = false;
  bool clip_planes_enabled = false;

  //===========
  // First we are going to find out what kind of clipping (if any)
  // we need. This depends on various factors including what the engine
  // says about the mesh (the clip_portal and clip_plane flags in the
  // mesh), what the current clipper is (the current cliptype), what
  // the current z-buf render mode is, and what the settings are to use
  // for the clipper on the current type of hardware (the clip_... arrays).
  //===========
  char how_clip = OPENGL_CLIP_NONE;
  bool use_lazy_clipping = false;

  // @@@ Todo: what to do with clip_plane?
  if (mesh.clip_portal != CS_CLIP_NOT)
  {
    // Some clipping may be required.

    // In some z-buf modes we cannot use clipping modes that depend on
    // zbuffer ('n','N', 'z', or 'Z').
    bool no_zbuf_clipping = (z_buf_mode == CS_ZBUF_NONE
    	|| z_buf_mode == CS_ZBUF_FILL || z_buf_mode == CS_ZBUF_FILLONLY);

    // Select the right clipping mode variable depending on the
    // type of clipper.
    int ct = cliptype;
    // If clip_portal in the mesh indicates that we might need toplevel
    // clipping then we do as if the current clipper type is toplevel.
    if (mesh.clip_portal == CS_CLIP_TOPLEVEL) ct = CS_CLIPPER_TOPLEVEL;
    char* clip_modes;
    switch (ct)
    {
      case CS_CLIPPER_OPTIONAL: clip_modes = clip_optional; break;
      case CS_CLIPPER_REQUIRED: clip_modes = clip_required; break;
      case CS_CLIPPER_TOPLEVEL: clip_modes = clip_outer; break;
      default: clip_modes = clip_optional;
    }

    // Go through all the modes and select the first one that is appropriate.
    int i;
    for (i = 0 ; i < 3 ; i++)
    {
      char c = clip_modes[i];
      // We cannot use n,N,z, or Z if no_zbuf_clipping is true.
      if ((c == 'n' || c == 'N' || c == 'z' || c == 'Z') && no_zbuf_clipping)
        continue;
      // We cannot use p or P if the clipper has more vertices than the
      // number of hardware planes minus one (for the view plane).
      if ((c == 'p' || c == 'P') &&
      		clipper->GetNumVertices () >= GLCaps.nr_hardware_planes-1)
        continue;
      how_clip = c;
      break;
    }
    if (how_clip != '0' && toupper (how_clip) == how_clip)
    {
      use_lazy_clipping = true;
      how_clip = tolower (how_clip);
    }
  }

  int i,k;

  //===========
  // First setup the clipper that we need.
  //===========
  if (how_clip == 's')
  {
    SetupStencil ();
    stencil_enabled = true;
    // Use the stencil area.
    glEnable (GL_STENCIL_TEST);
    glStencilFunc (GL_EQUAL, 1, 1);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
  }
  else if (how_clip == 'p')
  {
    SetupClipPlanes ();
    clip_planes_enabled = true;
    for (i = 0 ; i <= frustum.GetNumVertices () ; i++)	// One extra!
      glEnable (GL_CLIP_PLANE0+i);
  }

  //===========
  // Update work tables.
  //===========
  int num_vertices = mesh.num_vertices;
  int num_triangles = mesh.num_triangles;
  if (num_vertices > tr_verts.Limit ())
  {
    tr_verts.SetLimit (num_vertices);
    uv_verts.SetLimit (num_vertices);
    uv_mul_verts.SetLimit (num_vertices);
    rgba_verts.SetLimit (num_vertices*4);
    color_verts.SetLimit (num_vertices);
    fog_intensities.SetLimit (num_vertices*2);
    fog_color_verts.SetLimit (num_vertices);
  }

  //===========
  // Do vertex tweening and/or transformation to camera space
  // if any of those are needed. When this is done 'verts' will
  // point to an array of camera vertices.
  //===========
  csVector3* f1 = mesh.vertices[0];
  csVector2* uv1 = mesh.texels[0];
  csColor* col1 = mesh.vertex_colors[0];
  csVector3* work_verts;
  csVector2* work_uv_verts;
  csColor* work_colors;

  if (mesh.num_vertices_pool > 1)
  {
    // Vertex morphing.
    float tr = mesh.morph_factor;
    float remainder = 1 - tr;
    csVector3* f2 = mesh.vertices[1];
    csVector2* uv2 = mesh.texels[1];
    csColor* col2 = mesh.vertex_colors[1];
    for (i = 0 ; i < num_vertices ; i++)
    {
      tr_verts[i] = tr * f2[i] + remainder * f1[i];
      if (mesh.do_morph_texels)
        uv_verts[i] = tr * uv2[i] + remainder * uv1[i];
      if (mesh.do_morph_colors)
      {
        color_verts[i].red = tr * col2[i].red + remainder * col1[i].red;
	color_verts[i].green = tr * col2[i].green + remainder * col1[i].green;
	color_verts[i].blue = tr * col2[i].blue + remainder * col1[i].blue;
      }
    }
    work_verts = tr_verts.GetArray ();
    if (mesh.do_morph_texels)
      work_uv_verts = uv_verts.GetArray ();
    else
      work_uv_verts = uv1;
    if (mesh.do_morph_colors)
      work_colors = color_verts.GetArray ();
    else
      work_colors = col1;
  }
  else
  {
    work_verts = f1;
    work_uv_verts = uv1;
    work_colors = col1;
  }
  csTriangle *triangles = mesh.triangles;
  G3DFogInfo* work_fog = mesh.vertex_fog;

  //===========
  // Here we perform lazy or software clipping if needed.
  //===========
  if (how_clip == '0' || use_lazy_clipping)
  {
    ClipTriangleMesh (
	num_triangles,
	num_vertices,
	triangles,
	work_verts,
	work_uv_verts,
	work_colors,
	work_fog,
	num_triangles,
	num_vertices,
	!use_lazy_clipping);
    if (!use_lazy_clipping)
    {
      work_verts = clipped_vertices.GetArray ();
      work_uv_verts = clipped_texels.GetArray ();
      work_colors = clipped_colors.GetArray ();
      work_fog = clipped_fog.GetArray ();
    }
    triangles = clipped_triangles.GetArray ();
  }
  
  // set up coordinate transform
  GLfloat matrixholder[16];

  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  //===========
  // set up world->camera transform, if needed
  //===========
  if (mesh.vertex_mode == G3DTriangleMesh::VM_WORLDSPACE)
  {
    // we basically have to duplicate the
    // original transformation code:
    //   tr_verts[i] = o2c.GetO2T() *  (f1[i] - o2c.GetO2TTranslation() );
    //
    // we do this by applying both a translation and rotation matrix
    // using values pulled from the o2c quantity, which represents
    // the current world->camera transform
    //
    // Wonder why we do the orientation before the translation?
    // Many 3D graphics and OpenGL books discuss how the order
    // of 4x4 transform matrices represent certain transformations,
    // and they do a much better job than I ever could.  Please refer
    // to an OpenGL reference for good insight into proper manipulation
    // of the modelview matrix.

    const csMatrix3 &orientation = o2c.GetO2T();

    matrixholder[0] = orientation.m11;
    matrixholder[1] = orientation.m21;
    matrixholder[2] = orientation.m31;

    matrixholder[4] = orientation.m12;
    matrixholder[5] = orientation.m22;
    matrixholder[6] = orientation.m32;

    matrixholder[8] = orientation.m13;
    matrixholder[9] = orientation.m23;
    matrixholder[10] = orientation.m33;

    matrixholder[3] = matrixholder[7] = matrixholder[11] =
    matrixholder[12] = matrixholder[13] = matrixholder[14] = 0.0;
    matrixholder[15] = 1.0;

    const csVector3 &translation = o2c.GetO2TTranslation();

    glMultMatrixf (matrixholder);
    glTranslatef (-translation.x, -translation.y, -translation.z);
  }

  //===========
  // Set up perspective transform.
  // we have to change the standard projection matrix used for
  // drawing in other parts of CS.  Normally an orthogonal projection
  // is used since CS does the perspective projection for us.
  // Here, we need to reproduce CS's perspective projection using
  // OpenGL matrices.
  //===========

  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();

  // With the back buffer procedural textures the orthographic projection
  // matrix are inverted.
  if (inverted)
    glOrtho (0., (GLdouble) width, (GLdouble) height, 0., -1.0, 10.0);
  else
    glOrtho (0., (GLdouble) width, 0., (GLdouble) height, -1.0, 10.0);


  glTranslatef (width2, height2, 0);
  for (i = 0 ; i < 16 ; i++) matrixholder[i] = 0.0;
  matrixholder[0] = matrixholder[5] = 1.0;
  matrixholder[11] = inv_aspect;
  matrixholder[14] = -inv_aspect;
  glMultMatrixf (matrixholder);

  //===========
  // Prepare the fog tables for later.
  //===========
  if (mesh.do_fog)
  {
    for (i=0 ; i < num_vertices ; i++)
    {
      // specify fog vertex transparency
      float I = work_fog[i].intensity;
      if (I > FOG_MAXVALUE)
      	I = FOGTABLE_CLAMPVALUE;
      else
	I = I * FOGTABLE_DISTANCESCALE;

      fog_intensities[i*2] = I;
      fog_intensities[i*2+1] = I;
      // @@@ To avoid this copy we better structure this differently
      // inside G3DTriangleMesh.
      fog_color_verts[i].red = work_fog[i].r;
      fog_color_verts[i].green = work_fog[i].g;
      fog_color_verts[i].blue = work_fog[i].b;
    }
  }

  //===========
  // Draw the base mesh.
  //===========
  StartPolygonFX (mesh.mat_handle, mesh.fxmode);
  csMaterialHandle* mat = NULL;
  bool multitex = false;
  if (mesh.mat_handle)
  {
    mat = (csMaterialHandle*)mesh.mat_handle;
    multitex = mat->GetNumTextureLayers () > 0;
  }
  bool colstate_enabled = false;
  bool do_gouraud = m_gouraud && work_colors;

  float flat_r = 1., flat_g = 1., flat_b = 1.;
  if (do_gouraud)
  {
    // special hack for transparent meshes
    if (mesh.fxmode & CS_FX_ALPHA)
      for (k=0, i=0; i<num_vertices; i++)
      {
        rgba_verts[k++] = work_colors[i].red;
        rgba_verts[k++] = work_colors[i].green;
        rgba_verts[k++] = work_colors[i].blue;
	rgba_verts[k++] = m_alpha;
      }
  }
  else
  {
    if (!m_textured)
    {
      // Fill flat color if renderer decide to paint it flat-shaded
      UByte r,g,b;
      if (mesh.mat_handle)
        mesh.mat_handle->GetTexture ()->GetMeanColor (r, g, b);
      else
        r = g = b = 1;
      flat_r = BYTE_TO_FLOAT (r);
      flat_g = BYTE_TO_FLOAT (g);
      flat_b = BYTE_TO_FLOAT (b);
    }
  }

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);
  glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
  glTexCoordPointer (2, GL_FLOAT, 0, & work_uv_verts[0]);
  // If multi-texturing is enabled we delay apply of gouraud shading
  // until later.
  if (do_gouraud && !multitex)
  {
    glShadeModel (GL_SMOOTH);
    colstate_enabled = true;
    glEnableClientState (GL_COLOR_ARRAY);
    if (mesh.fxmode & CS_FX_ALPHA)
      glColorPointer (4, GL_FLOAT, 0, & rgba_verts[0]);
    else
      glColorPointer (3, GL_FLOAT, 0, & work_colors[0]);
  }
  else
  {
    glShadeModel (GL_FLAT);
    glColor4f (flat_r, flat_g, flat_b, m_alpha);
  }


csColor cols[8];
cols[0].Set (0, 0, 0);
cols[1].Set (1, 0, 0);
cols[2].Set (0, 1, 0);
cols[3].Set (0, 0, 1);
cols[4].Set (1, 1, 0);
cols[5].Set (1, 0, 1);
cols[6].Set (0, 1, 1);
cols[7].Set (1, 1, 1);
if (!colstate_enabled) glEnableClientState (GL_COLOR_ARRAY);
glColorPointer (3, GL_FLOAT, 0, cols);


  glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, triangles);

  // If we have multi-texturing or fog we set the second pass Z-buffer
  // mode here.
  if (multitex || mesh.do_fog)
    SetGLZBufferFlagsPass2 (true);

  //===========
  // Here we perform multi-texturing.
  //===========
  if (multitex)
  {
    glShadeModel (GL_FLAT);
    if (colstate_enabled)
    {
      colstate_enabled = false;
      glDisableClientState (GL_COLOR_ARRAY);
    }
    int j;
    for (j = 0 ; j < mat->GetNumTextureLayers () ; j++)
    {
      csTextureLayer* layer = mat->GetTextureLayer (j);
      iTextureHandle* txt_handle = layer->txt_handle;
      csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)
      	txt_handle->GetPrivateObject ();
      csGLCacheData *texturecache_data;
      texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
      //tex_transp = txt_mm->GetKeyColor () || txt_mm->GetAlphaMap ();
      GLuint texturehandle = texturecache_data->Handle;
      csglBindTexture (GL_TEXTURE_2D, texturehandle);
      float alpha = 1.0f - BYTE_TO_FLOAT (layer->mode & CS_FX_MASK_ALPHA);
      alpha = SetupBlend (layer->mode, alpha, false);
      glColor4f (1., 1., 1., alpha);
      csVector2* mul_uv = work_uv_verts;
      int uscale = layer->uscale;
      int vscale = layer->vscale;
      int ushift = layer->ushift;
      int vshift = layer->vshift;
      if (mat->TextureLayerTranslated (j))
      {
        mul_uv = uv_mul_verts.GetArray ();
	for (i = 0 ; i < num_vertices ; i++)
	{
	  mul_uv[i].x = work_uv_verts[i].x * uscale + ushift;
	  mul_uv[i].y = work_uv_verts[i].y * vscale + vshift;
	}
      }
      glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
      glTexCoordPointer (2, GL_FLOAT, 0, mul_uv);
      glDrawElements (GL_TRIANGLES, num_triangles*3,
      	GL_UNSIGNED_INT, triangles);
    }
 
    // If we have to do gouraud shading we do it here.
    if (do_gouraud)
    {
      glDisable (GL_TEXTURE_2D);
      glShadeModel (GL_SMOOTH);
      glEnable (GL_BLEND);
      glBlendFunc (GL_DST_COLOR, GL_SRC_COLOR);
      glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      if (!colstate_enabled)
      {
        colstate_enabled = true;
        glEnableClientState (GL_COLOR_ARRAY);
      }
      if (mesh.fxmode & CS_FX_ALPHA)
        glColorPointer (4, GL_FLOAT, 0, & rgba_verts[0]);
      else
        glColorPointer (3, GL_FLOAT, 0, & work_colors[0]);
      glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
      glDrawElements (GL_TRIANGLES, num_triangles*3,
      	GL_UNSIGNED_INT, triangles);
    }
  }

  //===========
  // If there is vertex fog then we apply that last.
  //===========
  if (mesh.do_fog)
  {
    // we need to texture and blend, with vertex color
    // interpolation
    glEnable (GL_TEXTURE_2D);
    csglBindTexture (GL_TEXTURE_2D, m_fogtexturehandle);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel (GL_SMOOTH);

    if (!colstate_enabled)
    {
      colstate_enabled = true;
      glEnableClientState (GL_COLOR_ARRAY);
    }

    glVertexPointer (3, GL_FLOAT, 0, & work_verts[0]);
    glTexCoordPointer (2, GL_FLOAT, 0, & fog_intensities[0]);
    glColorPointer (3, GL_FLOAT, 0, & fog_color_verts[0]);

    glDrawElements (GL_TRIANGLES, num_triangles*3, GL_UNSIGNED_INT, triangles);

    if (!m_textured)
      glDisable (GL_TEXTURE_2D);
    if (!m_gouraud)
      glShadeModel (GL_FLAT);
  }

  if (colstate_enabled)
    glDisableClientState (GL_COLOR_ARRAY);
  glDisableClientState (GL_VERTEX_ARRAY);
  glDisableClientState (GL_TEXTURE_COORD_ARRAY);

  FinishPolygonFX ();

  glMatrixMode (GL_MODELVIEW);
  glPopMatrix ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();

  //===========
  // Disable/cleanup all clipping stuff.
  //===========
  if (stencil_enabled)
    glDisable (GL_STENCIL_TEST);
  if (clip_planes_enabled)
    for (i = 0 ; i <= frustum.GetNumVertices () ; i++)	// One extra!
      glDisable (GL_CLIP_PLANE0+i);
}



void csGraphics3DOGLCommon::OpenFogObject (CS_ID /*id*/, csFog* /*fog*/)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::DrawFogPolygon (CS_ID /*id*/, G3DPolygonDFP &/*poly*/, int /*fogtype*/)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::CloseFogObject (CS_ID /*id*/)
{
  // OpenGL driver implements vertex-based fog ...
}

void csGraphics3DOGLCommon::CacheTexture (iMaterialHandle *imat_handle)
{
  csMaterialHandle* mat_handle = (csMaterialHandle*)imat_handle;
  iTextureHandle* txt_handle = mat_handle->GetTexture ();
  texture_cache->cache_texture (txt_handle);
  // Also cache all textures used in the texture layers.
  int i;
  for (i = 0 ; i < mat_handle->GetNumTextureLayers () ; i++)
    texture_cache->cache_texture (mat_handle->GetTextureLayer (i)->txt_handle);
}

void csGraphics3DOGLCommon::CacheTexture (iPolygonTexture *texture)
{
  CacheTexture (texture->GetMaterialHandle ());
  if (m_renderstate.lighting)
    lightmap_cache->cache_lightmap (texture);
}

bool csGraphics3DOGLCommon::SetRenderState (G3D_RENDERSTATEOPTION op, long value)
{
  //@@@end_draw_poly ();

  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      z_buf_mode = (csZBufMode) value;
      break;
    case G3DRENDERSTATE_DITHERENABLE:
      m_renderstate.dither = value;
      break;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      texture_cache->SetBilinearMapping (value);
      break;
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      m_renderstate.trilinearmap = value;
      break;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      m_renderstate.alphablend = value;
      break;
    case G3DRENDERSTATE_MIPMAPENABLE:
      m_renderstate.mipmap = value;
      break;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      m_renderstate.textured = value;
      break;
    case G3DRENDERSTATE_MMXENABLE:
      return false;
    case G3DRENDERSTATE_INTERLACINGENABLE:
      return false;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      m_renderstate.lighting = value;
      break;
    case G3DRENDERSTATE_GOURAUDENABLE:
      m_renderstate.gouraud = value;
      break;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      dbg_max_polygons_to_draw = value;
      if (dbg_max_polygons_to_draw < 0)
        dbg_max_polygons_to_draw = 0;
      break;
    default:
      return false;
  }

  return true;
}

long csGraphics3DOGLCommon::GetRenderState (G3D_RENDERSTATEOPTION op)
{
  switch (op)
  {
    case G3DRENDERSTATE_ZBUFFERMODE:
      return z_buf_mode;
    case G3DRENDERSTATE_DITHERENABLE:
      return m_renderstate.dither;
    case G3DRENDERSTATE_BILINEARMAPPINGENABLE:
      return texture_cache->GetBilinearMapping ();
    case G3DRENDERSTATE_TRILINEARMAPPINGENABLE:
      return m_renderstate.trilinearmap;
    case G3DRENDERSTATE_TRANSPARENCYENABLE:
      return m_renderstate.alphablend;
    case G3DRENDERSTATE_MIPMAPENABLE:
      return m_renderstate.mipmap;
    case G3DRENDERSTATE_TEXTUREMAPPINGENABLE:
      return m_renderstate.textured;
    case G3DRENDERSTATE_MMXENABLE:
      return 0;
    case G3DRENDERSTATE_INTERLACINGENABLE:
      return false;
    case G3DRENDERSTATE_LIGHTINGENABLE:
      return m_renderstate.lighting;
    case G3DRENDERSTATE_GOURAUDENABLE:
      return m_renderstate.gouraud;
    case G3DRENDERSTATE_MAXPOLYGONSTODRAW:
      return dbg_max_polygons_to_draw;
    default:
      return 0;
  }
}

void csGraphics3DOGLCommon::ClearCache ()
{
  end_draw_poly ();

  // We will clear lightmap cache since when unloading a world lightmaps
  // become invalid. We won't clear texture cache since texture items are
  // cleaned up individually when an iTextureHandle's RefCount reaches zero.
  if (lightmap_cache) lightmap_cache->Clear ();
}

void csGraphics3DOGLCommon::DumpCache ()
{
}

void csGraphics3DOGLCommon::DrawLine (const csVector3 & v1, const csVector3 & v2,
	float fov, int color)
{
  end_draw_poly ();

  if (v1.z < SMALL_Z && v2.z < SMALL_Z)
    return;

  float x1 = v1.x, y1 = v1.y, z1 = v1.z;
  float x2 = v2.x, y2 = v2.y, z2 = v2.z;

  if (z1 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z - z1) / (z2 - z1);
    x1 = t * (x2 - x1) + x1;
    y1 = t * (y2 - y1) + y1;
    z1 = SMALL_Z;
  }
  else if (z2 < SMALL_Z)
  {
    // x = t*(x2-x1)+x1;
    // y = t*(y2-y1)+y1;
    // z = t*(z2-z1)+z1;
    float t = (SMALL_Z - z1) / (z2 - z1);
    x2 = t * (x2 - x1) + x1;
    y2 = t * (y2 - y1) + y1;
    z2 = SMALL_Z;
  }
  float iz1 = fov / z1;
  int px1 = QInt (x1 * iz1 + (width / 2));
  int py1 = height - 1 - QInt (y1 * iz1 + (height / 2));
  float iz2 = fov / z2;
  int px2 = QInt (x2 * iz2 + (width / 2));
  int py2 = height - 1 - QInt (y2 * iz2 + (height / 2));

  G2D->DrawLine (px1, py1, px2, py2, color);
}

void csGraphics3DOGLCommon::SetGLZBufferFlags ()
{
  switch (z_buf_mode)
  {
    case CS_ZBUF_NONE:
      glDisable (GL_DEPTH_TEST);
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_ALWAYS);
      glDepthMask (GL_TRUE);
      break;
    case CS_ZBUF_TEST:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_USE:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_TRUE);
      break;
    default:
      break;
  }
}

void csGraphics3DOGLCommon::SetGLZBufferFlagsPass2 (bool multiPol)
{
  switch (z_buf_mode)
  {
    case CS_ZBUF_NONE:
      glDisable (GL_DEPTH_TEST);
      break;
    case CS_ZBUF_FILL:
    case CS_ZBUF_FILLONLY:
      if (multiPol)
      {
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_EQUAL);
        glDepthMask (GL_FALSE);
      }
      else
      {
        glDisable (GL_DEPTH_TEST);
      }
      break;
    case CS_ZBUF_TEST:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_GREATER);
      glDepthMask (GL_FALSE);
      break;
    case CS_ZBUF_USE:
      glEnable (GL_DEPTH_TEST);
      glDepthFunc (GL_EQUAL);
      glDepthMask (GL_FALSE);
      break;
    default:
      break;
  }
}

// Shortcut to override standard polygon drawing when we have
// multitexture
bool csGraphics3DOGLCommon::DrawPolygonMultiTexture (G3DPolygonDP & poly)
{
  end_draw_poly ();//@@@

// work in progress - GJH
#if USE_MULTITEXTURE

  // count 'real' number of vertices
  int num_vertices = 1;
  int i;
  for (i = 1; i < poly.num; i++)
  {
    // theoretically we should do here sqrt(dx^2+dy^2), but
    // we can approximate it just by abs(dx)+abs(dy)
    if ((ABS (poly.vertices[i].sx - poly.vertices[i - 1].sx)
	 + ABS (poly.vertices[i].sy - poly.vertices[i - 1].sy))
	 	> VERTEX_NEAR_THRESHOLD)
      num_vertices++;
  }

  // if this is a 'degenerate' polygon, skip it
  if (num_vertices < 3)
    return false;

  iPolygonTexture *tex = poly.poly_texture;

  // find lightmap information, if any
  iLightMap *thelightmap = tex->GetLightMap ();

  // the shortcut works only if there is a lightmap and no fog and
  // no multitexturing
  csMaterialHandle* mat_handle = (csMaterialHandle*)poly.mat_handle;
  if (!thelightmap || poly.use_fog || mat_handle->GetNumTextureLayers () > 0)
  {
    DrawPolygonSingleTexture (poly);
    return true;
  }
  // OK, we're gonna draw a polygon with a dual texture
  // Get the plane normal of the polygon. Using this we can calculate
  // '1/z' at every screen space point.
  float Ac, Bc, Cc, Dc, inv_Dc;
  Ac = poly.normal.A ();
  Bc = poly.normal.B ();
  Cc = poly.normal.C ();
  Dc = poly.normal.D ();

  float M, N, O;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane normal is too small. This means
    // that the plane of the polygon is almost perpendicular to the
    // eye of the viewer. In this case, nothing much can be seen of
    // the plane anyway so we just take one value for the entire
    // polygon.
    M = 0;
    N = 0;
    // For O choose the transformed z value of one vertex.
    // That way Z buffering should at least work.
    O = 1 / poly.z_value;
  }
  else
  {
    inv_Dc = 1 / Dc;
    M = -Ac * inv_Dc * inv_aspect;
    N = -Bc * inv_Dc * inv_aspect;
    O = -Cc * inv_Dc;
  }

  tex = poly.poly_texture;
  iTextureHandle* txt_handle = poly.mat_handle->GetTexture ();
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *) txt_handle->GetPrivateObject ();

  // find lightmap information, if any
  thelightmap = tex->GetLightMap ();

  // Initialize our static drawing information and cache
  // the texture in the texture cache (if this is not already the case).
  CacheTexture (tex);

  // @@@ The texture transform matrix is currently written as
  // T = M*(C-V)
  // (with V being the transform vector, M the transform matrix, and C
  // the position in camera space coordinates. It would be better (more
  // suitable for the following calculations) if it would be written
  // as T = M*C - V.
  float P1, P2, P3, P4, Q1, Q2, Q3, Q4;
  P1 = poly.plane.m_cam2tex->m11;
  P2 = poly.plane.m_cam2tex->m12;
  P3 = poly.plane.m_cam2tex->m13;
  P4 = -(P1 * poly.plane.v_cam2tex->x
	 + P2 * poly.plane.v_cam2tex->y
	 + P3 * poly.plane.v_cam2tex->z);
  Q1 = poly.plane.m_cam2tex->m21;
  Q2 = poly.plane.m_cam2tex->m22;
  Q3 = poly.plane.m_cam2tex->m23;
  Q4 = -(Q1 * poly.plane.v_cam2tex->x
	 + Q2 * poly.plane.v_cam2tex->y
	 + Q3 * poly.plane.v_cam2tex->z);

  // Precompute everything so that we can calculate (u,v) (texture space
  // coordinates) for every (sx,sy) (screen space coordinates). We make
  // use of the fact that 1/z, u/z and v/z are linear in screen space.
  float J1, J2, J3, K1, K2, K3;
  if (ABS (Dc) < SMALL_D)
  {
    // The Dc component of the plane of the polygon is too small.
    J1 = J2 = J3 = 0;
    K1 = K2 = K3 = 0;
  }
  else
  {
    J1 = P1 * inv_aspect + P4 * M;
    J2 = P2 * inv_aspect + P4 * N;
    J3 = P3 + P4 * O;
    K1 = Q1 * inv_aspect + Q4 * M;
    K2 = Q2 * inv_aspect + Q4 * N;
    K3 = Q3 + Q4 * O;
  }

  bool tex_transp;
  int poly_alpha = poly.alpha;

  csGLCacheData *texturecache_data;
  texturecache_data = (csGLCacheData *)txt_mm->GetCacheData ();
  tex_transp = txt_mm->GetKeyColor ();
  GLuint texturehandle = texturecache_data->Handle;

  // configure base texture for texure unit 0
  float flat_r = 1.0, flat_g = 1.0, flat_b = 1.0;
  glActiveTextureARB (GL_TEXTURE0_ARB);
  csglBindTexture (GL_TEXTURE_2D, texturehandle);
  if (poly.mixmode != CS_FX_COPY)
  {
    float alpha = 1.0f - BYTE_TO_FLOAT (poly_alpha);
    bool has_alpha = txt_handle->GetKeyColor () || txt_handle->GetAlphaMap ();
    alpha = SetupBlend (poly.mixmode, alpha, has_alpha);
    glColor4f (flat_r, flat_g, flat_b, alpha);
  }
  else if ((poly_alpha > 0) || tex_transp)
  {
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable (GL_BLEND);
    if (poly_alpha > 0)
      glColor4f (flat_r, flat_g, flat_b, 1.0 - BYTE_TO_FLOAT (poly_alpha));
    else
      glColor4f (flat_r, flat_g, flat_b, 1.0);
  }
  else
  {
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable (GL_BLEND);
    glColor4f (flat_r, flat_g, flat_b, 0.);
  }

  csGLCacheData *lightmapcache_data = (csGLCacheData *)thelightmap->GetCacheData ();
  GLuint lightmaphandle = lightmapcache_data->Handle;

  // configure lightmap for texture unit 1
  glActiveTextureARB (GL_TEXTURE1_ARB);
  glEnable (GL_TEXTURE_2D);
  csglBindTexture (GL_TEXTURE_2D, lightmaphandle);
  glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  SetGLZBufferFlags ();

  float lm_scale_u = lightmapcache_data->lm_scale_u;
  float lm_scale_v = lightmapcache_data->lm_scale_v;
  float lm_offset_u = lightmapcache_data->lm_offset_u;
  float lm_offset_v = lightmapcache_data->lm_offset_v;

  float light_u, light_v;
  float sx, sy, sz, one_over_sz;
  float u_over_sz, v_over_sz;

  glBegin (GL_TRIANGLE_FAN);
  for (i = 0; i < poly.num; i++)
  {
    sx = poly.vertices[i].sx - width2;
    sy = poly.vertices[i].sy - height2;
    one_over_sz = M * sx + N * sy + O;
    sz = 1.0 / one_over_sz;
    u_over_sz = (J1 * sx + J2 * sy + J3);
    v_over_sz = (K1 * sx + K2 * sy + K3);
    light_u = (u_over_sz * sz - lm_offset_u) * lm_scale_u;
    light_v = (v_over_sz * sz - lm_offset_v) * lm_scale_v;

    // modified to use homogenous object space coordinates instead
    // of homogenous texture space coordinates
    glMultiTexCoord2fARB (GL_TEXTURE0_ARB, u_over_sz * sz, v_over_sz * sz);
    glMultiTexCoord2fARB (GL_TEXTURE1_ARB, light_u, light_v);
    glVertex4f (poly.vertices[i].sx * sz, poly.vertices[i].sy * sz, -1.0, sz);
  }
  glEnd ();

  // we must disable the 2nd texture unit, so that other parts of the
  // code won't accidently have a second texture applied if they
  // don't want it. At this point our active texture is still TEXTURE1_ARB
  glActiveTextureARB (GL_TEXTURE1_ARB);
  glDisable (GL_TEXTURE_2D);
  glActiveTextureARB (GL_TEXTURE0_ARB);

  return true;
#else
  (void) poly;
  // multitexture not enabled -- how did we get into this shortcut?
  return false;
#endif
}

float csGraphics3DOGLCommon::GetZBuffValue (int x, int y)
{
  GLfloat zvalue;
  glReadPixels (x, height - y - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zvalue);
  if (zvalue < .000001) return 1000000000.;
  // 0.090909=1/11, that is 1 divided by total depth delta set by
  // glOrtho. Where 0.090834 comes from, I don't know
  //return (0.090834 / (zvalue - (0.090909)));
  // @@@ Jorrit: I have absolutely no idea what they are trying to do
  // but changing the above formula to the one below at least appears
  // to give more accurate results.
  return (0.090728 / (zvalue - (0.090909)));
}

void csGraphics3DOGLCommon::DrawPolygon (G3DPolygonDP & poly)
{
  if (z_buf_mode == CS_ZBUF_FILLONLY)
  {
    DrawPolygonZFill (poly);
    return;
  }

#if USE_EXTENSIONS
  if (ARB_multitexture)
  {
    DrawPolygonMultiTexture (poly);
    return;
  }
#endif
  DrawPolygonSingleTexture (poly);
}

void csGraphics3DOGLCommon::DrawPixmap (iTextureHandle *hTex,
  int sx, int sy, int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha)
{
  end_draw_poly ();

  int ClipX1, ClipY1, ClipX2, ClipY2;
  G2D->GetClipRect (ClipX1, ClipY1, ClipX2, ClipY2);

  // Texture coordinates (floats)
  float _tx = tx, _ty = ty, _tw = tw, _th = th;

  // Clipping
  if ((sx >= ClipX2) || (sy >= ClipY2) ||
      (sx + sw <= ClipX1) || (sy + sh <= ClipY1))
    return;                             // Sprite is totally invisible
  if (sx < ClipX1)                      // Left margin crossed?
  {
    int nw = sw - (ClipX1 - sx);        // New width
    _tx += (ClipX1 - sx) * _tw / sw;    // Adjust X coord on texture
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw; sx = ClipX1;
  } /* endif */
  if (sx + sw > ClipX2)                 // Right margin crossed?
  {
    int nw = ClipX2 - sx;               // New width
    _tw = (_tw * nw) / sw;              // Adjust width on texture
    sw = nw;
  } /* endif */
  if (sy < ClipY1)                      // Top margin crossed?
  {
    int nh = sh - (ClipY1 - sy);        // New height
    _ty += (ClipY1 - sy) * _th / sh;    // Adjust Y coord on texture
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh; sy = ClipY1;
  } /* endif */
  if (sy + sh > ClipY2)                 // Bottom margin crossed?
  {
    int nh = ClipY2 - sy;               // New height
    _th = (_th * nh) / sh;              // Adjust height on texture
    sh = nh;
  } /* endif */

  // cache the texture if we haven't already.
  texture_cache->cache_texture (hTex);

  // Get texture handle
  csTextureHandleOpenGL *txt_mm = (csTextureHandleOpenGL *)hTex->GetPrivateObject ();
  GLuint texturehandle = ((csGLCacheData *)txt_mm->GetCacheData ())->Handle;

  // as we are drawing in 2D, we disable some of the commonly used features
  // for fancy 3D drawing
  glShadeModel (GL_FLAT);
  glDisable (GL_DEPTH_TEST);
  glDepthMask (GL_FALSE);

  // if the texture has transparent bits, we have to tweak the
  // OpenGL blend mode so that it handles the transparent pixels correctly
  if (hTex->GetKeyColor () || hTex->GetAlphaMap () || Alpha)
  {
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  else
    glDisable (GL_BLEND);

  glEnable (GL_TEXTURE_2D);
  glColor4f (1.0, 1.0, 1.0, Alpha ? (1.0 - BYTE_TO_FLOAT (Alpha)) : 1.0);
  csglBindTexture (GL_TEXTURE_2D, texturehandle);

  int bitmapwidth = 0, bitmapheight = 0;
  hTex->GetMipMapDimensions (0, bitmapwidth, bitmapheight);

  // convert texture coords given above to normalized (0-1.0) texture
  // coordinates
  float ntx1,nty1,ntx2,nty2;
  ntx1 = (_tx      ) / bitmapwidth;
  ntx2 = (_tx + _tw) / bitmapwidth;
  nty1 = (_ty      ) / bitmapheight;
  nty2 = (_ty + _th) / bitmapheight;

  // draw the bitmap
  glBegin (GL_QUADS);
//    glTexCoord2f (ntx1, nty1);
//    glVertex2i (sx, height - sy - 1);
//    glTexCoord2f (ntx2, nty1);
//    glVertex2i (sx + sw, height - sy - 1);
//    glTexCoord2f (ntx2, nty2);
//    glVertex2i (sx + sw, height - sy - sh - 1);
//    glTexCoord2f (ntx1, nty2);
//    glVertex2i (sx, height - sy - sh - 1);

  // smgh: This works in software opengl and with cswstest
  glTexCoord2f (ntx1, nty1);
  glVertex2i (sx, height - sy - 1);
  glTexCoord2f (ntx2, nty1);
  glVertex2i (sx + sw, height - sy - 1);
  glTexCoord2f (ntx2, nty2);
  glVertex2i (sx + sw, height - (sy+1 + sh));
  glTexCoord2f (ntx1, nty2);
  glVertex2i (sx, height - (sy+1 + sh));
  glEnd ();
}

/* this function is called when the user configures the OpenGL renderer to use
 * 'auto' blend mode.  It tries to figure out how well the driver supports
 * the 2*SRC*DST blend mode--some beta/debug drivers support it badly, some hardware
 * does not support it at all.
 *
 * We check the driver by drawing a polygon with both blend modes:
 *   - we draw using SRC*DST blending and read back the color result, called A
 *   - we draw using 2*SRC*DST blending and read back the color result, called B
 *
 * Ideally B=2*A.  Here we guess that if B > 1.5*A then the 2*SRC*DST mode is
 * reasonably well supported and suggest using 2*SRC*DST mode.  Otherwise we
 * suggest using SRC*DST mode which is pretty well supported.
 */
void csGraphics3DOGLCommon::Guess_BlendMode(GLenum *src, GLenum*dst)
{
  // colors of the 2 polys to blend
  float testcolor1[3] = {0.5,0.5,0.5};
  float testcolor2[3] = {0.5,0.5,0.5};

  // these will hold the resultant color intensities
  float blendresult1[3], blendresult2[3];

  SysPrintf(MSG_INITIALIZATION, "Attempting to determine best blending mode to use.\n");

  // draw the polys
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glShadeModel(GL_FLAT);

  // blend mode one

  glDisable(GL_BLEND);
  glColor3fv(testcolor1);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR, GL_ZERO);
  glColor3fv(testcolor2);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glReadPixels(0,0,1,1,GL_RGB,GL_FLOAT, &blendresult1);

  // blend mode two

  glDisable(GL_BLEND);
  glColor3fv(testcolor1);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
  glColor3fv(testcolor2);
  glBegin(GL_QUADS);
  glVertex2i(0,0); glVertex2i(5,0); glVertex2i(5,5); glVertex2i(0,5);
  glEnd();

  glReadPixels(0,0,1,1,GL_RGB,GL_FLOAT, &blendresult2);

  SysPrintf(MSG_INITIALIZATION, "Blend mode values are %f and %f...", blendresult1[1], blendresult2[1]);

  // compare the green component between the two results, A and B.  In the
  // ideal case B = 2*A.  If SRC*DST blend mode is supported but 2*SRC*DST is
  // not, then B = A.  So we guess that if B > 1.5*A that the 2*SRC*DST is
  // 'pretty well' supported and go with that.  Otherwise, fall back on the
  // normal SRC*DST mode.

  float resultA = blendresult1[1];
  float resultB = blendresult2[1];

  if (resultB > 1.5 * resultA)
  {
    SysPrintf(MSG_INITIALIZATION, "using 'multiplydouble' blend mode.\n");

    *src = GL_DST_COLOR;
    *dst = GL_SRC_COLOR;
  }
  else
  {
    SysPrintf(MSG_INITIALIZATION, "using 'multiply' blend mode.\n");

    *src = GL_DST_COLOR;
    *dst = GL_ZERO;
  }
}
