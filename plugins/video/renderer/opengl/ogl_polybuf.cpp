/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include <math.h>
#include <stdarg.h>

#include "cssysdef.h"
#include "ogl_polybuf.h"
#include "csutil/util.h"
#include "imesh/thing/polygon.h"
#include "csgeom/transfrm.h"
#include "qint.h"
#include "ivideo/material.h"
#include "ivideo/texture.h"
#include "imesh/thing/lightmap.h" //@@@
#include "ogl_txtcache.h"


TrianglesList::TrianglesList ()
{
  first = NULL;
  last = NULL;
}

TrianglesList::~TrianglesList ()
{
  while (first)
  {
    csTrianglesPerMaterial* aux = first->next;
    delete first;
    first = aux;
  }
}

void TrianglesList::Add (csTrianglesPerMaterial* t)
{
  if (first == NULL)
  {
    first = last = t;
    return;
  }
  last->next = t;
  last = t;
}

//--------------------------------------------------------------------------

csTrianglesPerMaterial::csTrianglesPerMaterial()
{
  next = NULL;
}

csTrianglesPerMaterial::~csTrianglesPerMaterial ()
{
  ClearVertexArray ();
}

void csTrianglesPerMaterial::ClearVertexArray ()
{
}

//--------------------------------------------------------------------------

csTrianglesPerSuperLightmap::csTrianglesPerSuperLightmap()
{
  region = new csSubRectangles (
    csRect (0, 0, OpenGLLightmapCache::super_lm_size,
  OpenGLLightmapCache::super_lm_size));

  cacheData = NULL;
  isUnlit = false;
  initialized = false;
  prev = NULL;
  cost = -1;
}

csTrianglesPerSuperLightmap::~csTrianglesPerSuperLightmap ()
{
  if (cacheData) cacheData->Clear();
  delete region;
}

int csTrianglesPerSuperLightmap::CalculateCost ()
{
  if (cost != -1) return cost;
  cost = rectangles.Length ();
  return cost;
}

//--------------------------------------------------------------------------

TrianglesSuperLightmapList::TrianglesSuperLightmapList ()
{
  first = NULL;
  last = NULL;
  dirty = true;
}

TrianglesSuperLightmapList::~TrianglesSuperLightmapList ()
{
  while (last)
  {
    csTrianglesPerSuperLightmap* aux = last->prev;
    delete last;
    last = aux;
  }
}

void TrianglesSuperLightmapList::Add (csTrianglesPerSuperLightmap* t)
{
  if (first == NULL) first = t;
  t->prev = last;
  last = t;
}

//--------------------------------------------------------------------------

csTriangleArrayPolygonBuffer::csTriangleArrayPolygonBuffer (
  iVertexBufferManager* mgr) : csPolygonBuffer (mgr)
{
  vertices = NULL;
  matCount = 0;
  unlitPolysSL = NULL;
}

csTriangleArrayPolygonBuffer::~csTriangleArrayPolygonBuffer ()
{
  Clear ();
}

/**
 * Search a superlightmap to fit the lighmap in the superLM list
 * if it can't find any creates a new one.
 * The case that the polygon has no superlightmap is supported too.
 * If the polygontexture has no lightmap it means its not lighted,
 * then a special superlightmap has to be created, just to store
 * the triangles and vertices that will be used in fog
 */
csTrianglesPerSuperLightmap* csTriangleArrayPolygonBuffer::
    SearchFittingSuperLightmap (iPolygonTexture* poly_texture,
                                csRect& rect)
{
  if (poly_texture == NULL || poly_texture->GetLightMap () == NULL)
  {
    // OK This polygon has no lightmap.
    // Let's check if we have to create a unlitPolygonsSL or is already
    // created
    if (unlitPolysSL) return unlitPolysSL;
    unlitPolysSL = new csTrianglesPerSuperLightmap ();
    unlitPolysSL->isUnlit = true;
    return unlitPolysSL;
  }
  iLightMap* piLM = poly_texture->GetLightMap();
  int lm_width = piLM->GetWidth();
  int lm_height = piLM->GetHeight();

  csTrianglesPerSuperLightmap* curr = superLM.last;
  while (curr)
  {
    if (curr->region->Alloc (lm_width, lm_height, rect))
      return curr;
    curr = curr->prev;
  }

  //We haven't found any, let's create a new one

  curr = new csTrianglesPerSuperLightmap ();

  if (!curr->region->Alloc (lm_width, lm_height, rect))
  {
    return NULL;
  }

  superLM.Add (curr);
  return curr;
}

int csTriangleArrayPolygonBuffer::AddSingleVertex (csTrianglesPerMaterial* pol,
	int* verts, int i, const csVector2& uv, int& cur_vt_idx)
{
  vec_vertices[cur_vt_idx] = vertices[verts[i]];
  texels[cur_vt_idx] = uv;

  cur_vt_idx++;
  return cur_vt_idx-1;
}

int csTriangleArrayPolygonBuffer::AddSingleVertexLM (
	const csVector2& uvLightmap, int& cur_vt_idx)
{
  lumels[cur_vt_idx] = uvLightmap;
  cur_vt_idx++;
  return cur_vt_idx-1;
}

void csTriangleArrayPolygonBuffer::AddTriangles (csTrianglesPerMaterial* pol,
    int* verts, int num_vertices,
    const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
    iPolygonTexture* poly_texture, int mat_index, int cur_vt_idx)
{
  csVector3 aux;
  csVector2 uv[100];	// @@@ Hardcoded limit
  int i;
  csTriangle triangle;

  // Triangulate the polygon and add all vertices with correct uv
  // index.
  csTransform transform (m_obj2tex, v_obj2tex);

  int old_cur_vt_idx = cur_vt_idx;

  for (i = 0 ; i < num_vertices ; i++)
  {
    aux = transform.Other2This (vertices[verts[i]]);
    uv[i].x = aux.x;
    uv[i].y = aux.y;
  }

  triangle.a = AddSingleVertex (pol, verts, 0, uv[0], cur_vt_idx);
  for (i = 1; i < num_vertices-1; i++)
  {
    triangle.b = AddSingleVertex (pol, verts, i, uv[i], cur_vt_idx);
    triangle.c = AddSingleVertex (pol, verts, i+1, uv[i+1], cur_vt_idx);
    pol->triangles.Push (triangle);
  }

  pol->matIndex = mat_index;

  AddLmQueue (poly_texture, uv, num_vertices, old_cur_vt_idx);
}

void csTriangleArrayPolygonBuffer::ClearLmQueue ()
{
  int i;
  for (i = 0 ; i < lmqueue.Length () ; i++)
  {
    delete[] lmqueue[i].uv;
  }
  lmqueue.SetLimit (0);
}

void csTriangleArrayPolygonBuffer::AddLmQueue (iPolygonTexture* polytext,
	const csVector2* uv, int num_uv, int vt_idx)
{
  csLmQueue q;
  q.polytext = polytext;
  q.uv = new csVector2[num_uv];
  memcpy (q.uv, uv, num_uv * sizeof (csVector2));
  q.num_uv = num_uv;
  q.vt_idx = vt_idx;
  lmqueue.Push (q);
}

void csTriangleArrayPolygonBuffer::ProcessLmQueue (
	iPolygonTexture* poly_texture,
	const csVector2* uv, int num_uv, int vt_idx)
{
  // Lightmap handling.
  csRect rect;
  csTrianglesPerSuperLightmap* triSuperLM = SearchFittingSuperLightmap (
  	poly_texture, rect);
  CS_ASSERT (triSuperLM != NULL);

  /*
   * We have the superlightmap where the poly_texture fits
   * Now we can add the triangles
   */

  /*
   * Let's check if it's the unlitPolySL
   */
  if (triSuperLM == unlitPolysSL)
  {
    return;
  }

  iLightMap* piLM = poly_texture->GetLightMap();
  float lm_width = float(piLM->GetWidth());
  float lm_height = float(piLM->GetHeight());
  int superLMsize = OpenGLLightmapCache::super_lm_size;

  float lm_low_u, lm_low_v, lm_high_u, lm_high_v;
  float lm_scale_u, lm_scale_v, lm_offset_u, lm_offset_v;

  poly_texture->GetTextureBox (lm_low_u,lm_low_v,lm_high_u,lm_high_v);
  if (lm_high_u <= lm_low_u)
    lm_scale_u = 1.;       // @@@ Is this right?
  else
    lm_scale_u = 1. / (lm_high_u - lm_low_u);

  if (lm_high_v <= lm_low_v)
    lm_scale_v = 1.;       // @@@ Is this right?
  else
    lm_scale_v = 1. / (lm_high_v - lm_low_v);

  lm_low_u -= .75 / (float (lm_width) * lm_scale_u);
  lm_high_u += .75 / (float (lm_width) * lm_scale_u);

  lm_low_v -= .75 / (float (lm_height) * lm_scale_v);
  lm_high_v += .75 / (float (lm_height) * lm_scale_v);

  lm_scale_u = 1. / ((lm_high_u - lm_low_u));
  lm_scale_v = 1. / ((lm_high_v - lm_low_v));

  float dlm = 1./ float(superLMsize);

  float sup_u = float (rect.xmin) * dlm;
  float sup_v = float (rect.ymin) * dlm;

  lm_scale_u = lm_scale_u * lm_width * dlm;
  lm_scale_v = lm_scale_v * lm_height * dlm;

  lm_offset_u = lm_low_u - sup_u / lm_scale_u;
  lm_offset_v = lm_low_v - sup_v / lm_scale_v;

  // Ok, we have the values, let's generate the triangles

  /*
   * Vertices for lightmap triangles have four coordinates
   * so we generate x,y,z, 1.0 vertices for it.
   * Every vertex is unique (so if two vertices share the
   * same coordinates but different uv's a new vertex is
   * created)
   */
  csVector2 uvLightmap[100];	// @@@ HARDCODED.
  int i;
  for (i = 0 ; i < num_uv ; i++)
  {
    uvLightmap[i].x = (uv[i].x - lm_offset_u) * lm_scale_u;
    uvLightmap[i].y = (uv[i].y - lm_offset_v) * lm_scale_v;
  }
  int cur_vt_idx = vt_idx;
  csTriangle triangle;
  triangle.a = AddSingleVertexLM (uvLightmap[0], cur_vt_idx);
  for (i = 1; i < num_uv - 1; i++)
  {
    triangle.b = AddSingleVertexLM (uvLightmap[i], cur_vt_idx);
    triangle.c = AddSingleVertexLM (uvLightmap[i+1], cur_vt_idx);
    triSuperLM->triangles.Push (triangle);
  }

  triSuperLM->rectangles.Push (rect);
  triSuperLM->lightmaps.Push (poly_texture);
  iLightMap* lm = poly_texture->GetLightMap ();
  triSuperLM->lm_info.Push (lm->GetMapData ());
}

static int sort_lmqueue (const void* p1, const void* p2)
{
  csLmQueue* lmq1 = (csLmQueue*)p1;
  csLmQueue* lmq2 = (csLmQueue*)p2;
  iLightMap* lm = lmq1->polytext->GetLightMap();
  int w1, h1, w2, h2;
  if (lm)
  {
    w1 = lm->GetWidth();
    h1 = lm->GetHeight();
  }
  else
  {
    w1 = 0;
    h1 = 0;
  }
  lm = lmq2->polytext->GetLightMap();
  if (lm)
  {
    w2 = lm->GetWidth();
    h2 = lm->GetHeight();
  }
  else
  {
    w2 = 0;
    h2 = 0;
  }

  int d1 = MAX (w1, h1);
  int d2 = MAX (w2, h2);
  if (d1 > d2) return -1;
  else if (d1 < d2) return 1;
  return 0;
}

void csTriangleArrayPolygonBuffer::Prepare ()
{
#if 0
// Debug stuff for printing out statistics information.
if (verts == NULL)
{
int cnt_mat = 0;
csTrianglesPerMaterial* t = GetFirst ();
while (t) { cnt_mat++; t = t->next; }
int cnt_lm = 0;
csTrianglesPerSuperLightmap* sln = GetFirstTrianglesSLM ();
while (sln) { cnt_lm++; sln = sln->prev; }
printf ("vtcnt=%d cnt_mat=%d cnt_lm=%d   ", GetVertexCount (), cnt_mat, cnt_lm);
sln = GetFirstTrianglesSLM ();
while (sln)
{
	int i;
	int tot_area = 0;
	int maxx = 0;
	int maxy = 0;
	for (i = 0 ; i < sln->rectangles.Length () ; i++)
	{
          iLightMap* lm = sln->lightmaps[i]->GetLightMap();
          int lmwidth = lm->GetWidth ();
          int lmheigth = lm->GetHeight ();
	  const csRect& r = sln->rectangles[i];
	  if (r.xmax > maxx) maxx = r.xmax;
	  if (r.ymax > maxy) maxy = r.ymax;
	  int area = lmwidth * lmheigth;
	  tot_area += area;
	  //printf ("%dx%d  %dx%d\n", lmwidth, lmheigth, r.xmax-r.xmin, r.ymax-r.ymin);
	}
	printf ("    num=%d maxx=%d maxy=%d tot_area=%d smallest=%d\n", sln->rectangles.Length (), maxx, maxy, tot_area, maxx*maxy);
	sln = sln->prev;
}
fflush (stdout);
return;
}
#endif

  qsort (lmqueue.GetArray (), lmqueue.Length (), sizeof (csLmQueue),
		  sort_lmqueue);

  int i;
  for (i = 0 ; i < lmqueue.Length () ; i++)
  {
    csLmQueue& q = lmqueue[i];
    ProcessLmQueue (q.polytext, q.uv, q.num_uv, q.vt_idx);
  }
  ClearLmQueue ();
}

void csTriangleArrayPolygonBuffer::MarkLightmapsDirty()
{
  superLM.MarkLightmapsDirty();
}

void csTriangleArrayPolygonBuffer::AddPolygon (int* verts, int num_verts,
  const csPlane3& poly_normal,
  int mat_index,
  const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
  iPolygonTexture* poly_texture)
{
  /*
   * We have to:
   * Generate triangles
   * Generate uv per vertex
   * Group the triangles per materials
   * We know this:
   * - The polygons are sent to AddPolygon sorted by material
   * - m_obj2tex and v_obj2tex are the matrix and vector to obtain the uv
   * cordinates
   *
   * We can do the following:
   * For every polygon:
   * if it is the first material add the triangles to the array
   *    and calculate the uv for it's vertices
   * if preceding material is the same that polygon's material then
   *    add the triangles to the preceding material array and calculate the
   *    uv
   * else add the triangles to the next position in the array and calculate
   *    their uv's
   *
   * IMPORTANT: poly_texture is stored per triangle? and normal per vertice?
   * it would be great!
   * - Can be done in load time?, it can, but does this affect to lighting
   * part? Ask Jorrit!
   * How can we this done? Every time an AddPolygon is done you know
   * the face normal, just add it to the vertex normal and normalize
   */

  //int cur_tri_num = orig_triangles.Length ();

  int cur_vt_num = vec_vertices.Length ();
  int new_vt_num = cur_vt_num + num_verts;
  vec_vertices.SetLength (new_vt_num);
  texels.SetLength (new_vt_num);
  lumels.SetLength (new_vt_num);

  int last_mat_index = polygons.GetLastMaterial ();
  if (last_mat_index != mat_index)
  {
    // First polygon or material of this polygon is different from
    // last material.
    csTrianglesPerMaterial* pol = new csTrianglesPerMaterial ();
    AddTriangles (pol, verts, num_verts, m_obj2tex, v_obj2tex,
      poly_texture, mat_index, cur_vt_num);
    polygons.Add (pol);

    matCount ++;
  }
  else
  {
    // We can add the triangles in the last PolygonPerMaterial
    // as long they share the same material.
    AddTriangles (polygons.last, verts, num_verts, m_obj2tex,
      v_obj2tex, poly_texture, mat_index, cur_vt_num);
  }

  csTriangle triangle;
  triangle.a = verts[0];
  int i;
  for (i = 1 ; i < num_verts-1 ; i++)
  {
    triangle.b = verts[i];
    triangle.c = verts[i+1];
    orig_triangles.Push (triangle);
  }

}

void csTriangleArrayPolygonBuffer::SetVertexArray (csVector3* verts,
  int num_verts)
{
  delete[] vertices;
  vertices = new csVector3 [num_verts];
  memcpy (vertices, verts, num_verts * sizeof (csVector3));
  verticesCount = num_verts;
}

void csTriangleArrayPolygonBuffer::AddMaterial (iMaterialHandle* mat_handle)
{
  materials.Push (mat_handle);
}

void csTriangleArrayPolygonBuffer::SetMaterial (int idx,
  iMaterialHandle* mat_handle)
{
  materials[idx] = mat_handle;
}

void csTriangleArrayPolygonBuffer::Clear ()
{
  delete[] vertices; vertices = NULL;
  materials.DeleteAll ();
  delete unlitPolysSL;
  unlitPolysSL = NULL;
  ClearLmQueue ();
}

csTriangleArrayVertexBufferManager::csTriangleArrayVertexBufferManager
  (iObjectRegistry* object_reg) : csVertexBufferManager (object_reg)
{
}

csTriangleArrayVertexBufferManager::~csTriangleArrayVertexBufferManager()
{
}

iPolygonBuffer* csTriangleArrayVertexBufferManager::CreatePolygonBuffer ()
{
  csTriangleArrayPolygonBuffer* buf = new csTriangleArrayPolygonBuffer (this);
  return buf;
}

