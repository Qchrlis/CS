/*
    Copyright (C) 2008 by Scott Johnson <scottj@cs.umn.edu>

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

#ifndef __CSUTIL_WIN32_MEMUTIL_H__
#define __CSUTIL_WIN32_MEMUTIL_H__

#include "cssysdef.h"
#include "csextern.h"

namespace CS {
  namespace Memory {
	  namespace Implementation {

    /** @brief Implementation-dependant memory retreival function.
	 *
	 * Used by CS::Memory::GetPhysicalMemory().  Do not call this function
	 * directly, use CS::Memory::GetPhysicalMemory().
     *
     * @returns Physical system memory (in kB)
	 *
	 * @sa CS::Memory::GetPhysicalMemory()
     */
     CS_CRYSTALSPACE_EXPORT size_t GetPhysicalMemory();

    } // End namespace Implementation
  } // End namespace Memory
} // End namespace CS

#endif
