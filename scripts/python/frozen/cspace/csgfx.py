# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.36
#
# Don't modify this file, modify the SWIG interface instead.

import _csgfx
import new
new_instancemethod = new.instancemethod
try:
    _swig_property = property
except NameError:
    pass # Python < 2.2 doesn't have 'property'.
def _swig_setattr_nondynamic(self,class_type,name,value,static=1):
    if (name == "thisown"): return self.this.own(value)
    if (name == "this"):
        if type(value).__name__ == 'PySwigObject':
            self.__dict__[name] = value
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    if (not static) or hasattr(self,name):
        self.__dict__[name] = value
    else:
        raise AttributeError("You cannot add attributes to %s" % self)

def _swig_setattr(self,class_type,name,value):
    return _swig_setattr_nondynamic(self,class_type,name,value,0)

def _swig_getattr(self,class_type,name):
    if (name == "thisown"): return self.this.own()
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

def _swig_repr(self):
    try: strthis = "proxy of " + self.this.__repr__()
    except: strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


def _swig_setattr_nondynamic_method(set):
    def set_attr(self,name,value):
        if (name == "thisown"): return self.this.own(value)
        if hasattr(self,name) or (name == "this"):
            set(self,name,value)
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr


import core
_SetSCFPointer = _csgfx._SetSCFPointer
_GetSCFPointer = _csgfx._GetSCFPointer
if not "core" in dir():
    core = __import__("cspace").__dict__["core"]
core.AddSCFLink(_SetSCFPointer)
CSMutableArrayHelper = core.CSMutableArrayHelper

class iShaderVarStringSetBase(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def Request(*args): return _csgfx.iShaderVarStringSetBase_Request(*args)
    def Contains(*args): return _csgfx.iShaderVarStringSetBase_Contains(*args)
    def Delete(*args): return _csgfx.iShaderVarStringSetBase_Delete(*args)
    def Empty(*args): return _csgfx.iShaderVarStringSetBase_Empty(*args)
    def Clear(*args): return _csgfx.iShaderVarStringSetBase_Clear(*args)
    def GetSize(*args): return _csgfx.iShaderVarStringSetBase_GetSize(*args)
    def IsEmpty(*args): return _csgfx.iShaderVarStringSetBase_IsEmpty(*args)
    __swig_destroy__ = _csgfx.delete_iShaderVarStringSetBase
    __del__ = lambda self : None;
iShaderVarStringSetBase_swigregister = _csgfx.iShaderVarStringSetBase_swigregister
iShaderVarStringSetBase_swigregister(iShaderVarStringSetBase)

CS_IMGFMT_MASK = _csgfx.CS_IMGFMT_MASK
CS_IMGFMT_NONE = _csgfx.CS_IMGFMT_NONE
CS_IMGFMT_TRUECOLOR = _csgfx.CS_IMGFMT_TRUECOLOR
CS_IMGFMT_PALETTED8 = _csgfx.CS_IMGFMT_PALETTED8
CS_IMGFMT_ANY = _csgfx.CS_IMGFMT_ANY
CS_IMGFMT_ALPHA = _csgfx.CS_IMGFMT_ALPHA
CS_IMGFMT_INVALID = _csgfx.CS_IMGFMT_INVALID
csimg2D = _csgfx.csimg2D
csimg3D = _csgfx.csimg3D
csimgCube = _csgfx.csimgCube
class iImage(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def GetImageData(*args): return _csgfx.iImage_GetImageData(*args)
    def GetWidth(*args): return _csgfx.iImage_GetWidth(*args)
    def GetHeight(*args): return _csgfx.iImage_GetHeight(*args)
    def GetDepth(*args): return _csgfx.iImage_GetDepth(*args)
    def SetName(*args): return _csgfx.iImage_SetName(*args)
    def GetName(*args): return _csgfx.iImage_GetName(*args)
    def GetFormat(*args): return _csgfx.iImage_GetFormat(*args)
    def GetPalette(*args): return _csgfx.iImage_GetPalette(*args)
    def GetAlpha(*args): return _csgfx.iImage_GetAlpha(*args)
    def HasKeyColor(*args): return _csgfx.iImage_HasKeyColor(*args)
    def GetKeyColor(*args): return _csgfx.iImage_GetKeyColor(*args)
    def HasMipmaps(*args): return _csgfx.iImage_HasMipmaps(*args)
    def GetMipmap(*args): return _csgfx.iImage_GetMipmap(*args)
    def GetRawFormat(*args): return _csgfx.iImage_GetRawFormat(*args)
    def GetRawData(*args): return _csgfx.iImage_GetRawData(*args)
    def GetImageType(*args): return _csgfx.iImage_GetImageType(*args)
    def HasSubImages(*args): return _csgfx.iImage_HasSubImages(*args)
    def GetSubImage(*args): return _csgfx.iImage_GetSubImage(*args)
    def GetCookedImageFormat(*args): return _csgfx.iImage_GetCookedImageFormat(*args)
    def GetCookedImageData(*args): return _csgfx.iImage_GetCookedImageData(*args)
    scfGetVersion = staticmethod(_csgfx.iImage_scfGetVersion)
    __swig_destroy__ = _csgfx.delete_iImage
    __del__ = lambda self : None;
iImage_swigregister = _csgfx.iImage_swigregister
iImage_swigregister(iImage)
iImage_scfGetVersion = _csgfx.iImage_scfGetVersion

class csImageIOFileFormatDescriptions(core.CustomAllocated):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    __swig_destroy__ = _csgfx.delete_csImageIOFileFormatDescriptions
    __del__ = lambda self : None;
    def __init__(self, *args): 
        this = _csgfx.new_csImageIOFileFormatDescriptions(*args)
        try: self.this.append(this)
        except: self.this = this
    def GetSize(*args): return _csgfx.csImageIOFileFormatDescriptions_GetSize(*args)
    def Get(*args): return _csgfx.csImageIOFileFormatDescriptions_Get(*args)
    def Put(*args): return _csgfx.csImageIOFileFormatDescriptions_Put(*args)
    def Push(*args): return _csgfx.csImageIOFileFormatDescriptions_Push(*args)
    def Pop(*args): return _csgfx.csImageIOFileFormatDescriptions_Pop(*args)
    def Top(*args): return _csgfx.csImageIOFileFormatDescriptions_Top(*args)
    def Insert(*args): return _csgfx.csImageIOFileFormatDescriptions_Insert(*args)
    def Contains(*args): return _csgfx.csImageIOFileFormatDescriptions_Contains(*args)
    def DeleteAll(*args): return _csgfx.csImageIOFileFormatDescriptions_DeleteAll(*args)
    def Truncate(*args): return _csgfx.csImageIOFileFormatDescriptions_Truncate(*args)
    def Empty(*args): return _csgfx.csImageIOFileFormatDescriptions_Empty(*args)
    def IsEmpty(*args): return _csgfx.csImageIOFileFormatDescriptions_IsEmpty(*args)
    def SetMinimalCapacity(*args): return _csgfx.csImageIOFileFormatDescriptions_SetMinimalCapacity(*args)
    def DeleteIndex(*args): return _csgfx.csImageIOFileFormatDescriptions_DeleteIndex(*args)
    def DeleteIndexFast(*args): return _csgfx.csImageIOFileFormatDescriptions_DeleteIndexFast(*args)
    def DeleteRange(*args): return _csgfx.csImageIOFileFormatDescriptions_DeleteRange(*args)
    def __eq__(*args): return _csgfx.csImageIOFileFormatDescriptions___eq__(*args)
    def __ne__(*args): return _csgfx.csImageIOFileFormatDescriptions___ne__(*args)
    def GetAllocator(*args): return _csgfx.csImageIOFileFormatDescriptions_GetAllocator(*args)
csImageIOFileFormatDescriptions_swigregister = _csgfx.csImageIOFileFormatDescriptions_swigregister
csImageIOFileFormatDescriptions_swigregister(csImageIOFileFormatDescriptions)

CS_IMAGEIO_LOAD = _csgfx.CS_IMAGEIO_LOAD
CS_IMAGEIO_SAVE = _csgfx.CS_IMAGEIO_SAVE
class csImageIOFileFormatDescription(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    mime = _swig_property(_csgfx.csImageIOFileFormatDescription_mime_get)
    subtype = _swig_property(_csgfx.csImageIOFileFormatDescription_subtype_get)
    cap = _swig_property(_csgfx.csImageIOFileFormatDescription_cap_get, _csgfx.csImageIOFileFormatDescription_cap_set)
    def __init__(self, *args): 
        this = _csgfx.new_csImageIOFileFormatDescription(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _csgfx.delete_csImageIOFileFormatDescription
    __del__ = lambda self : None;
csImageIOFileFormatDescription_swigregister = _csgfx.csImageIOFileFormatDescription_swigregister
csImageIOFileFormatDescription_swigregister(csImageIOFileFormatDescription)

class iImageIO(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def GetDescription(*args): return _csgfx.iImageIO_GetDescription(*args)
    def Load(*args): return _csgfx.iImageIO_Load(*args)
    def SetDithering(*args): return _csgfx.iImageIO_SetDithering(*args)
    def Save(*args): return _csgfx.iImageIO_Save(*args)
    scfGetVersion = staticmethod(_csgfx.iImageIO_scfGetVersion)
    __swig_destroy__ = _csgfx.delete_iImageIO
    __del__ = lambda self : None;
iImageIO_swigregister = _csgfx.iImageIO_swigregister
iImageIO_swigregister(iImageIO)
iImageIO_scfGetVersion = _csgfx.iImageIO_scfGetVersion

class iAnimatedImage(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def Animate(*args): return _csgfx.iAnimatedImage_Animate(*args)
    def IsAnimated(*args): return _csgfx.iAnimatedImage_IsAnimated(*args)
iAnimatedImage_swigregister = _csgfx.iAnimatedImage_swigregister
iAnimatedImage_swigregister(iAnimatedImage)

class iProcTexture(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def GetAlwaysAnimate(*args): return _csgfx.iProcTexture_GetAlwaysAnimate(*args)
    def SetAlwaysAnimate(*args): return _csgfx.iProcTexture_SetAlwaysAnimate(*args)
    def GetFactory(*args): return _csgfx.iProcTexture_GetFactory(*args)
iProcTexture_swigregister = _csgfx.iProcTexture_swigregister
iProcTexture_swigregister(iProcTexture)

class csRGBcolor(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    red = _swig_property(_csgfx.csRGBcolor_red_get, _csgfx.csRGBcolor_red_set)
    green = _swig_property(_csgfx.csRGBcolor_green_get, _csgfx.csRGBcolor_green_set)
    blue = _swig_property(_csgfx.csRGBcolor_blue_get, _csgfx.csRGBcolor_blue_set)
    def __init__(self, *args): 
        this = _csgfx.new_csRGBcolor(*args)
        try: self.this.append(this)
        except: self.this = this
    def Set(*args): return _csgfx.csRGBcolor_Set(*args)
    def __eq__(*args): return _csgfx.csRGBcolor___eq__(*args)
    def __ne__(*args): return _csgfx.csRGBcolor___ne__(*args)
    def __add__(*args): return _csgfx.csRGBcolor___add__(*args)
    def UnsafeAdd(*args): return _csgfx.csRGBcolor_UnsafeAdd(*args)
    def SafeAdd(*args): return _csgfx.csRGBcolor_SafeAdd(*args)
    __swig_destroy__ = _csgfx.delete_csRGBcolor
    __del__ = lambda self : None;
csRGBcolor_swigregister = _csgfx.csRGBcolor_swigregister
csRGBcolor_swigregister(csRGBcolor)

class csRGBpixel(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    red = _swig_property(_csgfx.csRGBpixel_red_get, _csgfx.csRGBpixel_red_set)
    green = _swig_property(_csgfx.csRGBpixel_green_get, _csgfx.csRGBpixel_green_set)
    blue = _swig_property(_csgfx.csRGBpixel_blue_get, _csgfx.csRGBpixel_blue_set)
    alpha = _swig_property(_csgfx.csRGBpixel_alpha_get, _csgfx.csRGBpixel_alpha_set)
    def __init__(self, *args): 
        this = _csgfx.new_csRGBpixel(*args)
        try: self.this.append(this)
        except: self.this = this
    def __eq__(*args): return _csgfx.csRGBpixel___eq__(*args)
    def __ne__(*args): return _csgfx.csRGBpixel___ne__(*args)
    def asRGBcolor(*args): return _csgfx.csRGBpixel_asRGBcolor(*args)
    def eq(*args): return _csgfx.csRGBpixel_eq(*args)
    def Intensity(*args): return _csgfx.csRGBpixel_Intensity(*args)
    def Luminance(*args): return _csgfx.csRGBpixel_Luminance(*args)
    def Set(*args): return _csgfx.csRGBpixel_Set(*args)
    def __iadd__(*args): return _csgfx.csRGBpixel___iadd__(*args)
    def UnsafeAdd(*args): return _csgfx.csRGBpixel_UnsafeAdd(*args)
    def SafeAdd(*args): return _csgfx.csRGBpixel_SafeAdd(*args)
    __swig_destroy__ = _csgfx.delete_csRGBpixel
    __del__ = lambda self : None;
csRGBpixel_swigregister = _csgfx.csRGBpixel_swigregister
csRGBpixel_swigregister(csRGBpixel)

R_COEF = _csgfx.R_COEF
G_COEF = _csgfx.G_COEF
B_COEF = _csgfx.B_COEF
R_COEF_SQ = _csgfx.R_COEF_SQ
G_COEF_SQ = _csgfx.G_COEF_SQ
B_COEF_SQ = _csgfx.B_COEF_SQ
class iShaderVarStringSet(iShaderVarStringSetBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    scfGetVersion = staticmethod(_csgfx.iShaderVarStringSet_scfGetVersion)
    __swig_destroy__ = _csgfx.delete_iShaderVarStringSet
    __del__ = lambda self : None;
iShaderVarStringSet_swigregister = _csgfx.iShaderVarStringSet_swigregister
iShaderVarStringSet_swigregister(iShaderVarStringSet)
cvar = _csgfx.cvar
InvalidShaderVarStringID = cvar.InvalidShaderVarStringID
iShaderVarStringSet_scfGetVersion = _csgfx.iShaderVarStringSet_scfGetVersion

class iShaderVariableAccessor(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def PreGetValue(*args): return _csgfx.iShaderVariableAccessor_PreGetValue(*args)
    __swig_destroy__ = _csgfx.delete_iShaderVariableAccessor
    __del__ = lambda self : None;
iShaderVariableAccessor_swigregister = _csgfx.iShaderVariableAccessor_swigregister
iShaderVariableAccessor_swigregister(iShaderVariableAccessor)

class csShaderVariable(core.csRefCount):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    UNKNOWN = _csgfx.csShaderVariable_UNKNOWN
    INT = _csgfx.csShaderVariable_INT
    FLOAT = _csgfx.csShaderVariable_FLOAT
    TEXTURE = _csgfx.csShaderVariable_TEXTURE
    RENDERBUFFER = _csgfx.csShaderVariable_RENDERBUFFER
    VECTOR2 = _csgfx.csShaderVariable_VECTOR2
    VECTOR3 = _csgfx.csShaderVariable_VECTOR3
    VECTOR4 = _csgfx.csShaderVariable_VECTOR4
    MATRIX3X3 = _csgfx.csShaderVariable_MATRIX3X3
    MATRIX = _csgfx.csShaderVariable_MATRIX
    TRANSFORM = _csgfx.csShaderVariable_TRANSFORM
    ARRAY = _csgfx.csShaderVariable_ARRAY
    MATRIX4X4 = _csgfx.csShaderVariable_MATRIX4X4
    COLOR = _csgfx.csShaderVariable_COLOR
    def __init__(self, *args): 
        this = _csgfx.new_csShaderVariable(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _csgfx.delete_csShaderVariable
    __del__ = lambda self : None;
    def assign(*args): return _csgfx.csShaderVariable_assign(*args)
    def GetType(*args): return _csgfx.csShaderVariable_GetType(*args)
    def SetType(*args): return _csgfx.csShaderVariable_SetType(*args)
    def SetAccessor(*args): return _csgfx.csShaderVariable_SetAccessor(*args)
    def SetName(*args): return _csgfx.csShaderVariable_SetName(*args)
    def GetName(*args): return _csgfx.csShaderVariable_GetName(*args)
    def GetAccessor(*args): return _csgfx.csShaderVariable_GetAccessor(*args)
    def GetAccessorData(*args): return _csgfx.csShaderVariable_GetAccessorData(*args)
    def SetValue(*args): return _csgfx.csShaderVariable_SetValue(*args)
    def AddVariableToArray(*args): return _csgfx.csShaderVariable_AddVariableToArray(*args)
    def RemoveFromArray(*args): return _csgfx.csShaderVariable_RemoveFromArray(*args)
    def SetArraySize(*args): return _csgfx.csShaderVariable_SetArraySize(*args)
    def GetArraySize(*args): return _csgfx.csShaderVariable_GetArraySize(*args)
    def GetArrayElement(*args): return _csgfx.csShaderVariable_GetArrayElement(*args)
    def SetArrayElement(*args): return _csgfx.csShaderVariable_SetArrayElement(*args)
    def FindArrayElement(*args): return _csgfx.csShaderVariable_FindArrayElement(*args)
    def GetValue(*args): return _csgfx.csShaderVariable_GetValue(*args)
csShaderVariable_swigregister = _csgfx.csShaderVariable_swigregister
csShaderVariable_swigregister(csShaderVariable)

class csShaderVariableArrayReadOnly(core.iBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def GetSize(*args): return _csgfx.csShaderVariableArrayReadOnly_GetSize(*args)
    def Get(*args): return _csgfx.csShaderVariableArrayReadOnly_Get(*args)
    def Top(*args): return _csgfx.csShaderVariableArrayReadOnly_Top(*args)
    def Find(*args): return _csgfx.csShaderVariableArrayReadOnly_Find(*args)
    def GetIndex(*args): return _csgfx.csShaderVariableArrayReadOnly_GetIndex(*args)
    def IsEmpty(*args): return _csgfx.csShaderVariableArrayReadOnly_IsEmpty(*args)
    def GetAll(*args): return _csgfx.csShaderVariableArrayReadOnly_GetAll(*args)
    __swig_destroy__ = _csgfx.delete_csShaderVariableArrayReadOnly
    __del__ = lambda self : None;
csShaderVariableArrayReadOnly_swigregister = _csgfx.csShaderVariableArrayReadOnly_swigregister
csShaderVariableArrayReadOnly_swigregister(csShaderVariableArrayReadOnly)

class csShaderVariableArrayChangeElements(csShaderVariableArrayReadOnly):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def Get(*args): return _csgfx.csShaderVariableArrayChangeElements_Get(*args)
    def Top(*args): return _csgfx.csShaderVariableArrayChangeElements_Top(*args)
    __swig_destroy__ = _csgfx.delete_csShaderVariableArrayChangeElements
    __del__ = lambda self : None;
csShaderVariableArrayChangeElements_swigregister = _csgfx.csShaderVariableArrayChangeElements_swigregister
csShaderVariableArrayChangeElements_swigregister(csShaderVariableArrayChangeElements)

class csShaderVariableArrayChangeAll(csShaderVariableArrayChangeElements):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def SetSize(*args): return _csgfx.csShaderVariableArrayChangeAll_SetSize(*args)
    def GetExtend(*args): return _csgfx.csShaderVariableArrayChangeAll_GetExtend(*args)
    def Put(*args): return _csgfx.csShaderVariableArrayChangeAll_Put(*args)
    def Push(*args): return _csgfx.csShaderVariableArrayChangeAll_Push(*args)
    def PushSmart(*args): return _csgfx.csShaderVariableArrayChangeAll_PushSmart(*args)
    def Pop(*args): return _csgfx.csShaderVariableArrayChangeAll_Pop(*args)
    def Insert(*args): return _csgfx.csShaderVariableArrayChangeAll_Insert(*args)
    def DeleteAll(*args): return _csgfx.csShaderVariableArrayChangeAll_DeleteAll(*args)
    def Truncate(*args): return _csgfx.csShaderVariableArrayChangeAll_Truncate(*args)
    def Empty(*args): return _csgfx.csShaderVariableArrayChangeAll_Empty(*args)
    def DeleteIndex(*args): return _csgfx.csShaderVariableArrayChangeAll_DeleteIndex(*args)
    def DeleteIndexFast(*args): return _csgfx.csShaderVariableArrayChangeAll_DeleteIndexFast(*args)
    def Delete(*args): return _csgfx.csShaderVariableArrayChangeAll_Delete(*args)
    __swig_destroy__ = _csgfx.delete_csShaderVariableArrayChangeAll
    __del__ = lambda self : None;
csShaderVariableArrayChangeAll_swigregister = _csgfx.csShaderVariableArrayChangeAll_swigregister
csShaderVariableArrayChangeAll_swigregister(csShaderVariableArrayChangeAll)

class csImageBaseBase(iImage):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def IncRef(*args): return _csgfx.csImageBaseBase_IncRef(*args)
    def DecRef(*args): return _csgfx.csImageBaseBase_DecRef(*args)
    def GetRefCount(*args): return _csgfx.csImageBaseBase_GetRefCount(*args)
    def QueryInterface(*args): return _csgfx.csImageBaseBase_QueryInterface(*args)
    def AddRefOwner(*args): return _csgfx.csImageBaseBase_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _csgfx.csImageBaseBase_RemoveRefOwner(*args)
    def GetInterfaceMetadata(*args): return _csgfx.csImageBaseBase_GetInterfaceMetadata(*args)
csImageBaseBase_swigregister = _csgfx.csImageBaseBase_swigregister
csImageBaseBase_swigregister(csImageBaseBase)

class csImageBase(csImageBaseBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    __swig_destroy__ = _csgfx.delete_csImageBase
    __del__ = lambda self : None;
    def GetDepth(*args): return _csgfx.csImageBase_GetDepth(*args)
    def SetName(*args): return _csgfx.csImageBase_SetName(*args)
    def GetName(*args): return _csgfx.csImageBase_GetName(*args)
    def GetPalette(*args): return _csgfx.csImageBase_GetPalette(*args)
    def GetAlpha(*args): return _csgfx.csImageBase_GetAlpha(*args)
    def HasKeyColor(*args): return _csgfx.csImageBase_HasKeyColor(*args)
    def GetKeyColor(*args): return _csgfx.csImageBase_GetKeyColor(*args)
    def HasMipmaps(*args): return _csgfx.csImageBase_HasMipmaps(*args)
    def GetMipmap(*args): return _csgfx.csImageBase_GetMipmap(*args)
    def GetImageType(*args): return _csgfx.csImageBase_GetImageType(*args)
    def HasSubImages(*args): return _csgfx.csImageBase_HasSubImages(*args)
    def GetSubImage(*args): return _csgfx.csImageBase_GetSubImage(*args)
    def GetCookedImageFormat(*args): return _csgfx.csImageBase_GetCookedImageFormat(*args)
    def GetCookedImageData(*args): return _csgfx.csImageBase_GetCookedImageData(*args)
csImageBase_swigregister = _csgfx.csImageBase_swigregister
csImageBase_swigregister(csImageBase)

class csImageMemoryBase(csImageBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    def __init__(self, *args, **kwargs): raise AttributeError, "No constructor defined"
    __repr__ = _swig_repr
    def IncRef(*args): return _csgfx.csImageMemoryBase_IncRef(*args)
    def DecRef(*args): return _csgfx.csImageMemoryBase_DecRef(*args)
    def GetRefCount(*args): return _csgfx.csImageMemoryBase_GetRefCount(*args)
    def QueryInterface(*args): return _csgfx.csImageMemoryBase_QueryInterface(*args)
    def AddRefOwner(*args): return _csgfx.csImageMemoryBase_AddRefOwner(*args)
    def RemoveRefOwner(*args): return _csgfx.csImageMemoryBase_RemoveRefOwner(*args)
    def GetInterfaceMetadata(*args): return _csgfx.csImageMemoryBase_GetInterfaceMetadata(*args)
csImageMemoryBase_swigregister = _csgfx.csImageMemoryBase_swigregister
csImageMemoryBase_swigregister(csImageMemoryBase)

class csImageMemory(csImageMemoryBase):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    def __init__(self, *args): 
        this = _csgfx.new_csImageMemory(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _csgfx.delete_csImageMemory
    __del__ = lambda self : None;
    def GetImagePtr(*args): return _csgfx.csImageMemory_GetImagePtr(*args)
    def GetPalettePtr(*args): return _csgfx.csImageMemory_GetPalettePtr(*args)
    def GetAlphaPtr(*args): return _csgfx.csImageMemory_GetAlphaPtr(*args)
    def GetImageData(*args): return _csgfx.csImageMemory_GetImageData(*args)
    def GetWidth(*args): return _csgfx.csImageMemory_GetWidth(*args)
    def GetHeight(*args): return _csgfx.csImageMemory_GetHeight(*args)
    def GetDepth(*args): return _csgfx.csImageMemory_GetDepth(*args)
    def GetRawFormat(*args): return _csgfx.csImageMemory_GetRawFormat(*args)
    def GetRawData(*args): return _csgfx.csImageMemory_GetRawData(*args)
    def GetFormat(*args): return _csgfx.csImageMemory_GetFormat(*args)
    def GetPalette(*args): return _csgfx.csImageMemory_GetPalette(*args)
    def GetAlpha(*args): return _csgfx.csImageMemory_GetAlpha(*args)
    def HasKeyColor(*args): return _csgfx.csImageMemory_HasKeyColor(*args)
    def GetKeyColor(*args): return _csgfx.csImageMemory_GetKeyColor(*args)
    def Clear(*args): return _csgfx.csImageMemory_Clear(*args)
    def CheckAlpha(*args): return _csgfx.csImageMemory_CheckAlpha(*args)
    def SetFormat(*args): return _csgfx.csImageMemory_SetFormat(*args)
    def SetKeyColor(*args): return _csgfx.csImageMemory_SetKeyColor(*args)
    def ClearKeyColor(*args): return _csgfx.csImageMemory_ClearKeyColor(*args)
    def ApplyKeyColor(*args): return _csgfx.csImageMemory_ApplyKeyColor(*args)
    def GetImageType(*args): return _csgfx.csImageMemory_GetImageType(*args)
    def SetImageType(*args): return _csgfx.csImageMemory_SetImageType(*args)
    def HasMipmaps(*args): return _csgfx.csImageMemory_HasMipmaps(*args)
    def GetMipmap(*args): return _csgfx.csImageMemory_GetMipmap(*args)
    def SetMipmap(*args): return _csgfx.csImageMemory_SetMipmap(*args)
    def Copy(*args): return _csgfx.csImageMemory_Copy(*args)
    def CopyScale(*args): return _csgfx.csImageMemory_CopyScale(*args)
    def CopyTile(*args): return _csgfx.csImageMemory_CopyTile(*args)
    def ConvertFromRGBA(*args): return _csgfx.csImageMemory_ConvertFromRGBA(*args)
    def ConvertFromPal8(*args): return _csgfx.csImageMemory_ConvertFromPal8(*args)
csImageMemory_swigregister = _csgfx.csImageMemory_swigregister
csImageMemory_swigregister(csImageMemory)

class csImageManipulate(object):
    thisown = _swig_property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc='The membership flag')
    __repr__ = _swig_repr
    Rescale = staticmethod(_csgfx.csImageManipulate_Rescale)
    Mipmap = staticmethod(_csgfx.csImageManipulate_Mipmap)
    Blur = staticmethod(_csgfx.csImageManipulate_Blur)
    Crop = staticmethod(_csgfx.csImageManipulate_Crop)
    Sharpen = staticmethod(_csgfx.csImageManipulate_Sharpen)
    TransformColor = staticmethod(_csgfx.csImageManipulate_TransformColor)
    Gray = staticmethod(_csgfx.csImageManipulate_Gray)
    RenormalizeNormals = staticmethod(_csgfx.csImageManipulate_RenormalizeNormals)
    def __init__(self, *args): 
        this = _csgfx.new_csImageManipulate(*args)
        try: self.this.append(this)
        except: self.this = this
    __swig_destroy__ = _csgfx.delete_csImageManipulate
    __del__ = lambda self : None;
csImageManipulate_swigregister = _csgfx.csImageManipulate_swigregister
csImageManipulate_swigregister(csImageManipulate)
csImageManipulate_Rescale = _csgfx.csImageManipulate_Rescale
csImageManipulate_Mipmap = _csgfx.csImageManipulate_Mipmap
csImageManipulate_Blur = _csgfx.csImageManipulate_Blur
csImageManipulate_Crop = _csgfx.csImageManipulate_Crop
csImageManipulate_Sharpen = _csgfx.csImageManipulate_Sharpen
csImageManipulate_TransformColor = _csgfx.csImageManipulate_TransformColor
csImageManipulate_Gray = _csgfx.csImageManipulate_Gray
csImageManipulate_RenormalizeNormals = _csgfx.csImageManipulate_RenormalizeNormals

def CS_REQUEST_IMAGELOADER ():
  return core.CS_REQUEST_PLUGIN("crystalspace.graphic.image.io.multiplexer",
    iImageIO)



