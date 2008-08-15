/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#include "csgfx/renderbuffer.h"
#include "csgfx/vertexlistwalker.h"
#include "imesh/skeleton2.h"
#include "imesh/skeleton2anim.h"

#include "animesh.h"


CS_PLUGIN_NAMESPACE_BEGIN(Animesh)
{
  typedef csVertexListWalker<float, csVector3> MorphTargetOffsetsWalker;
  
#include "csutil/custom_new_disable.h"
  void AnimeshObject::MorphVertices ()
  {
    bool hasMorphing = false;
    for (size_t i = 0; i < morphTargetWeights.GetSize(); i++)
    {
      if (morphTargetWeights[i] != 0)
      {
        hasMorphing = true;
        break;
      }
    }

    if (!hasMorphing)
    {
      postMorphVertices = factory->vertexBuffer;
      return;
    }

    // Setup the morph target VB
    if (!postMorphVertices || postMorphVertices == factory->vertexBuffer)
    {
      postMorphVertices = csRenderBuffer::CreateRenderBuffer (factory->GetVertexCountP (),
        CS_BUF_STREAM, CS_BUFCOMP_FLOAT, 3);
    }

    const size_t numMorphTargets =  morphTargetWeights.GetSize();
    CS_ALLOC_STACK_ARRAY(uint8, morphWalkersRaw, 
      numMorphTargets * sizeof (MorphTargetOffsetsWalker));

    MorphTargetOffsetsWalker* morphWalkers =
      (MorphTargetOffsetsWalker*)(void*)morphWalkersRaw;

    for (size_t m = 0; m < numMorphTargets; m++)
    {
      new (morphWalkers + m) MorphTargetOffsetsWalker (
        factory->morphTargets[m]->offsets);
    }

    csVertexListWalker<float, csVector3> srcVerts (factory->vertexBuffer);
    csRenderBufferLock<csVector3> dstVerts (postMorphVertices);

    for (size_t i = 0; i < factory->GetVertexCountP (); ++i)
    {
      csVector3 morphedVert = *srcVerts;

      for (size_t m = 0; m < numMorphTargets; m++)
      {
        MorphTargetOffsetsWalker& walk = morphWalkers[m];
        morphedVert += (*walk) * morphTargetWeights[m];
        ++walk;
      }

      dstVerts[i] = morphedVert;

      ++srcVerts;
    }

    for (size_t m = 0; m < numMorphTargets; m++)
    {
      morphWalkers[m].~MorphTargetOffsetsWalker();
    }
  }

#include "csutil/custom_new_enable.h"

  template<bool SkinV, bool SkinN, bool SkinTB>
  void AnimeshObject::Skin ()
  {
    if (!skeleton)
      return;

    // @@Better checks/handling...
    if (SkinV)
    {
      CS_ASSERT (skinnedVertices->GetElementCount () >= factory->vertexCount);
    }

    if (SkinN)
    {
      CS_ASSERT (skinnedNormals->GetElementCount () >= factory->vertexCount);
    }
    
    if (SkinTB)
    {
      CS_ASSERT (skinnedTangents->GetElementCount () >= factory->vertexCount);
      CS_ASSERT (skinnedBinormals->GetElementCount () >= factory->vertexCount);
    }    

    // Setup some local data
    csVertexListWalker<float, csVector3> srcVerts (postMorphVertices);
    csRenderBufferLock<csVector3> dstVerts (skinnedVertices);
    csVertexListWalker<float, csVector3> srcNormals (factory->normalBuffer);
    csRenderBufferLock<csVector3> dstNormals (skinnedNormals);

    csVertexListWalker<float, csVector3> srcTangents (factory->tangentBuffer);
    csRenderBufferLock<csVector3> dstTangents (skinnedTangents);
    csVertexListWalker<float, csVector3> srcBinormals (factory->binormalBuffer);
    csRenderBufferLock<csVector3> dstBinormals (skinnedBinormals);

    csSkeletalState2* skeletonState = lastSkeletonState;    

    csAnimatedMeshBoneInfluence* influence = factory->boneInfluences.GetArray ();

    for (size_t i = 0; i < factory->vertexCount; ++i)
    {
      // Accumulate data for the vertex
      int numInfluences = 0;

      csDualQuaternion dq (csQuaternion (0,0,0,0), csQuaternion (0,0,0,0)); 
      csQuaternion pivot;   

      for (size_t j = 0; j < 4; ++j, ++influence) // @@SOLVE 4
      {
        if (influence->influenceWeight > 0)
        {
          numInfluences++;

          csDualQuaternion inflQuat (
            skeletonState->GetQuaternion (influence->bone),
            skeletonState->GetVector (influence->bone));

          if (numInfluences == 1)
          {
            pivot = inflQuat.real;
          }
          else if (inflQuat.real.Dot (pivot) < 0.0f)
          {
            inflQuat *= -1.0f;
          }

          dq += inflQuat * influence->influenceWeight;
        }
      }   

      if (numInfluences == 0)
      {
        if (SkinV)
        {
          dstVerts[i] = *srcVerts;
        }

        if (SkinN)
        {
          dstNormals[i] = *srcNormals;
        }        

        if (SkinTB)
        {
          dstTangents[i] = *srcTangents;
          dstBinormals[i] = *srcBinormals;
        }        
      }
      else
      {
        dq = dq.Unit ();

        if (SkinV)
        {
          dstVerts[i] = dq.TransformPoint (*srcVerts);
        }

        if (SkinN)
        {
          dstNormals[i] = dq.Transform (*srcNormals);
        }

        if (SkinTB)
        {
          dstTangents[i] = dq.Transform (*srcTangents);
          dstBinormals[i] = dq.Transform (*srcBinormals);
        }       
      }

      ++srcVerts;
      ++srcNormals;
      ++srcTangents;
      ++srcBinormals;
    }
  }


  void AnimeshObject::SkinVertices ()
  {
    Skin<true, false, false> ();
  }

  void AnimeshObject::SkinNormals ()
  {
    Skin<false, true, false> ();
  }

  void AnimeshObject::SkinVerticesAndNormals ()
  {
    Skin<true, true, false> ();
  }

  void AnimeshObject::SkinTangentAndBinormal ()
  {
    Skin<false, false, true> ();
  }

  void AnimeshObject::SkinAll ()
  {
    Skin<true, true, true> ();
  }

}
CS_PLUGIN_NAMESPACE_END(Animesh)

