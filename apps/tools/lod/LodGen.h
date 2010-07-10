/*
 *  LodGen.h
 *  cs
 *
 *  Created by Eduardo Poyart on 6/14/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

void PointTriangleDistanceUnitTests();

typedef csArray<int> IncidentTris;

struct Edge
{
  int v0;
  int v1;
  Edge() {}
  Edge(int a, int b) { assert(a != b); if (a < b) { v0 = a; v1 = b; } else { v0 = b; v1 = a; } }
  inline bool operator==(const Edge& e1) const
  {
    assert(v0 < v1 && e1.v0 < e1.v1);
    return (v0 == e1.v0 && v1 == e1.v1);
  }
};

struct SlidingWindow
{
  int start_index;
  int end_index;
};

struct WorkMesh
{
  csArray<csTriangle> tri_buffer;
  csArray<int> tri_indices;
  csArray<IncidentTris> incident_tris; // map from vertices to incident triangles
  csArray<Edge> edges;
  csArray<SlidingWindow> sliding_windows;
};

class LodGen
{
protected:
  const csArray<csVector3>& vertices;
  const csArray<csTriangle>& triangles;
  int num_vertices;
  int num_triangles;
  
  WorkMesh k;
  csArray<csTriangle> ordered_tris;
  int top_limit;

public:
  LodGen(const csArray<csVector3>& v, const csArray<csTriangle>& t): 
    vertices(v), triangles(t), num_vertices(v.GetSize()), num_triangles(t.GetSize()) {}
  void GenerateLODs();
  int GetTriangleCount() const { return ordered_tris.GetSize(); }
  const csTriangle& GetTriangle(int i) const { return ordered_tris[i]; }
  int GetSlidingWindowCount() const { return k.sliding_windows.GetSize(); }
  const SlidingWindow& GetSlidingWindow(int i) const { return k.sliding_windows[i]; }
  
protected:
  bool IsDegenerate(const csTriangle& tri) const;
  void AddTriangle(WorkMesh& k, int itri);
  void RemoveTriangleFromIncidentTris(WorkMesh& k, int itri);
  bool Collapse(WorkMesh& k, int v0, int v1);
  float SumOfSquareDist(const WorkMesh& k) const;
  int FindInWindow(const WorkMesh& k, const SlidingWindow& sw, int itri) const;
  void SwapIndex(WorkMesh& k, int i0, int i1);
  void VerifyMesh(WorkMesh& k);
  bool IsTriangleCoincident(const csTriangle& t0, const csTriangle& t1) const;
  bool IsCoincident(const WorkMesh& k, const csTriangle& tri) const;
};