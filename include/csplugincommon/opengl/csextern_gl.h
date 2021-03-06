/*
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

#ifndef __CS_CSEXTERN_GL_H__
#define __CS_CSEXTERN_GL_H__

#ifdef CS_CSPLUGINCOMMON_GL_LIB
  #define CS_CSPLUGINCOMMON_GL_EXPORT CS_EXPORT_SYM
#else
  #define CS_CSPLUGINCOMMON_GL_EXPORT CS_IMPORT_SYM
#endif

#endif // __CS_EXTERN_GL_H__
