/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#ifndef __CS_NORMALIZATIONCUBE_H__
#define __CS_NORMALIZATIONCUBE_H__

#include "csutil/weakref.h"
#include "csgfx/shadervar.h"
#include "ivideo/txtmgr.h"

class csNormalizationCubeAccessor : public iShaderVariableAccessor
{
  void FillNormalizationMapSide (unsigned char *normdata, int size,
    int xx, int xy, int xo,
    int yx, int yy, int yo,
    int zx, int zy, int zo);

  int normalizeCubeSize;
  csWeakRef<iTextureManager> txtmgr;
  csRef<iTextureHandle> texture;
public:
  SCF_DECLARE_IBASE;

  csNormalizationCubeAccessor (iTextureManager* txtmgr, int sideSize);
  virtual ~csNormalizationCubeAccessor ();

  virtual void PreGetValue (csShaderVariable *variable);
};

#endif // __CS_NORMALIZATIONCUBE_H__
