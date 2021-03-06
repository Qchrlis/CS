/*
  Copyright (C) 2011 by Antony Martin

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"

#include "csplugincommon/opengl/glextmanager.h"

#include "glshader_glsl.h"
#include "glshader_glslshader.h"

CS_PLUGIN_NAMESPACE_BEGIN(GLShaderGLSL)
{
  CS_LEAKGUARD_IMPLEMENT (csShaderGLSLShader);

  bool csShaderGLSLShader::Compile (const char *source)
  {
    GLint status;                   // compile status
    const csGLExtensionManager* ext = shaderPlug->ext;

    shader_id = ext->glCreateShaderObjectARB (type);
    ext->glShaderSourceARB (shader_id, 1, &source, NULL);
    ext->glCompileShaderARB (shader_id);

    ext->glGetObjectParameterivARB (shader_id, GL_OBJECT_COMPILE_STATUS_ARB,
      &status);
    if (status != GL_TRUE)
    {
      GLint size;

      ext->glGetObjectParameterivARB (shader_id, GL_OBJECT_INFO_LOG_LENGTH_ARB,
        &size);
      csString logs((size_t)(size + 1));
      // cast hax
      ext->glGetInfoLogARB (shader_id, size, NULL, (GLcharARB*)logs.GetData ());

      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING,
        "Couldn't compile GLSL %s shader", typeName.GetData ());
      shaderPlug->Report (CS_REPORTER_SEVERITY_WARNING, "Error string: %s", 
        CS::Quote::Single (logs.GetData ()));
      return false;
    }

    return true;
  }
}
CS_PLUGIN_NAMESPACE_END(GLShaderGLSL)
