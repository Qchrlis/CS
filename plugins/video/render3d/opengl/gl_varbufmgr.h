/*
    Copyright (C) 2002 by M�rten Svanfeldt
                          Anders Stenberg

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

#ifndef __GL_VARBUFMGR_H__
#define __GL_VARBUFMGR_H__

#include "ivideo/rndbuf.h"

//#include "gl_render3d.h"

class csGLRender3D;
class csVARRenderBufferManager;
struct iLightingInfo;
struct iTextureHandle;

/**
 * Helperclass for VAR. An implementation of the buddy algoritm.
 * For references, see the linuxkernel and the web
 * This implementation is originaly written by Alex Verstak, but adopted
 * to CS by Marten Svanfeldt
 */
class csBuddyAllocator
{
private:
  //Private constants
  /*
  static const int CS_ALLOC_GRANULARITY = 0; //Blocksizes in multiple of 4K
  static const int CS_BUDDY_SHIFT = (12+CS_ALLOC_GRANULARITY);
  static const int CS_BUDDY_ADD = ((1<<CS_BUDDY_SHIFT)-1);
  */
  enum
  {
    CS_ALLOC_GRANULARITY = 0,
    CS_BUDDY_SHIFT = 12+CS_ALLOC_GRANULARITY,
    CS_BUDDY_ADD = (1<<CS_BUDDY_SHIFT)-1
  };
  //helper functions
  int treeSize(int size);
  int ptoi(void* ptr);
  void* itop(int i, char level);

  void stabilize1(int i);
  void stabilize2(int, char);
  void traverse(int, char);

  char* m_Tree;
  int m_Size; //in granularity units
  int m_Height; //treeheight
  int m_Freeblk; //number of free blocks

public:
  
  bool Initialize(int size);
  void* alloc(int size);
  bool free(void* ptr);

  void PrintStats();
};

/**
 * Helperstruct to hold a memoryblock + fence
 */
struct csVARMemoryBlock
{
  void *buffer;
  unsigned int fence_id;
};

SCF_VERSION(csVARRenderBuffer, 0,0,1);
/**
 * This is a general buffer to be used by the renderer. It can ONLY be
 * created by the VB manager
 */
class csVARRenderBuffer : public iRenderBuffer
{
private:
  bool isRealVAR;
  csVARMemoryBlock* memblock;
  int size;
  CS_RENDERBUFFER_TYPE type;
  bool locked;

  csVARRenderBufferManager* bm;

  CS_BUFFER_LOCK_TYPE lastlock;
public:
  SCF_DECLARE_IBASE;

  csVARRenderBuffer(void *buffer, int size, CS_RENDERBUFFER_TYPE type, csVARRenderBufferManager* bm);
  ~csVARRenderBuffer ();
  
  /**
   * Lock the buffer to allow writing and give us a pointer to the data
   * The pointer will be NULL if there was some error
   */
  virtual void* Lock(CS_BUFFER_LOCK_TYPE lockType);

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release();

  /// Get type of buffer (where it's located)
  virtual CS_RENDERBUFFER_TYPE GetBufferType() { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() {return size; }
};


class csVARRenderBufferManager: public iRenderBufferManager
{
  friend class csVARRenderBuffer;
private:
  csGLRender3D* render3d; /// Must keep this to be able to use extensions

  unsigned char* var_buffer;
  csBuddyAllocator* myalloc;

  //unlocked blocks, these should be freed as soon as the fence is released
  csBasicVector discardedBlocks;
public:
  SCF_DECLARE_IBASE;
  /**
   * Nonstandard constructor, as we need access to the internal of opengl-renderer
   * If not successful return false.
   */
  bool Initialize(csGLRender3D* render3d);

  /// Kill it
  ~csVARRenderBufferManager();

  /// Allocate a buffer of the specified type and return it
  csPtr<iRenderBuffer> GetBuffer(int buffersize, CS_RENDERBUFFER_TYPE location);

};

#endif //  __GL_VARBUFMGR_H__