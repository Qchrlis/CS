/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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
#include "csqint.h"
#include "csgfx/memimage.h"

#include "csgfx/imagemanipulate.h"

csRef<iImage> csImageManipulate::Rescale (iImage* source, int newwidth, 
    int newheight)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  if (newwidth == Width && newheight == Height)
    return source;

  // This is a quick and dirty algorithm and it doesn't do funny things
  // such as blending multiple pixels together or bilinear filtering,
  // just a rough scale. It could be improved by someone in the future.

  unsigned int x, y;
  unsigned int dx = csQint16 (float (Width) / float (newwidth));
  unsigned int dy = csQint16 (float (Height) / float (newheight));

#define RESIZE(pt, Source, Dest)			\
  {							\
    const pt* field = (pt*)Source;			\
    pt* dst = (pt*)Dest;				\
    y = 0;						\
    int ny, nx;						\
    for (ny = newheight; ny; ny--)			\
    {							\
      const pt* src = field + (y >> 16) * Width;	\
      y += dy; x = 0;					\
      for (nx = newwidth; nx; nx--)			\
      {							\
        *dst++ = src [x >> 16];				\
        x += dx;					\
      }							\
    }							\
  }

  csImageMemory* newImg = new csImageMemory (newwidth, newheight,
    source->GetFormat());

  switch (source->GetFormat() & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_TRUECOLOR:
      RESIZE (csRGBpixel, source->GetImageData(), newImg->GetImagePtr())
      break;
    case CS_IMGFMT_PALETTED8:
      RESIZE (uint8, source->GetPalette(), newImg->GetPalettePtr())
      break;
  }
  if (source->GetAlpha())
    RESIZE (uint8, source->GetAlpha(), newImg->GetAlphaPtr())

  csRef<iImage> imageRef;
  imageRef.AttachNew (newImg);
  return imageRef;
}

//---------------------- Helper functions ---------------------------

#define MIPMAP_NAME	mipmap_0
#define MIPMAP_LEVEL	0
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_t
#define MIPMAP_LEVEL	0
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_p
#define MIPMAP_LEVEL	0
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_pt
#define MIPMAP_LEVEL	0
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_0_a
#define MIPMAP_LEVEL	0
#define MIPMAP_ALPHA
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1
#define MIPMAP_LEVEL	1
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_t
#define MIPMAP_LEVEL	1
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_p
#define MIPMAP_LEVEL	1
#define MIPMAP_PALETTED
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_pt
#define MIPMAP_LEVEL	1
#define MIPMAP_PALETTED
#define MIPMAP_TRANSPARENT
#include "mipmap.inc"

#define MIPMAP_NAME	mipmap_1_a
#define MIPMAP_LEVEL	1
#define MIPMAP_ALPHA
#include "mipmap.inc"

//-----------------------------------------------------------------------------

csRef<iImage> csImageManipulate::Mipmap (iImage* source, int steps, 
    csRGBpixel* transp)
{
  if (steps == 0)
    return source;

  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  if ((Width == 1) && (Height == 1)) return source;

  csRef<csImageMemory> nimg;
  csRef<iImage> simg = source;

  int cur_w = Width;
  int cur_h = Height;

  while (steps && !((cur_w == 1) && (cur_h == 1)) )
  {
    const int newW = MAX(1, cur_w >> 1);
    const int newH = MAX(1, cur_h >> 1);
    
    nimg.AttachNew (new csImageMemory (newW, newH, simg->GetFormat()));

    csRGBpixel *mipmap = new csRGBpixel [newW * newH];
    uint8* Alpha = nimg->GetAlphaPtr();

    int transpidx = -1;
    if (transp && simg->GetPalette ())
      transpidx = csImageTools::ClosestPaletteIndex (simg->GetPalette(), 
      *transp);

    switch (simg->GetFormat() & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
      case CS_IMGFMT_PALETTED8:
	if (simg->GetImageData())
	  if (transpidx < 0)
	    mipmap_1_p (cur_w, cur_h, 
	      (uint8 *)simg->GetImageData(), mipmap, simg->GetPalette());
	  else
	    mipmap_1_pt (cur_w, cur_h, (uint8*)simg->GetImageData(), mipmap,
	      simg->GetPalette(), transpidx);
	nimg->ConvertFromRGBA (mipmap);
	if (simg->GetAlpha ())
	{
	  mipmap_1_a (cur_w, cur_h, (uint8 *)simg->GetAlpha (), Alpha);
	}
	break;
      case CS_IMGFMT_TRUECOLOR:
	if (!transp)
	  mipmap_1 (cur_w, cur_h, (csRGBpixel *)simg->GetImageData (), mipmap);
	else
	  mipmap_1_t (cur_w, cur_h, (csRGBpixel *)simg->GetImageData (), mipmap, *transp);
	nimg->ConvertFromRGBA (mipmap);
	break;
    }

    simg = nimg;
    steps--;
    cur_w = nimg->GetWidth();
    cur_h = nimg->GetHeight();
  }

  return nimg;
}

csRef<iImage> csImageManipulate::Blur (iImage* source, csRGBpixel* transp)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  csRef<csImageMemory> nimg;
  nimg.AttachNew (new csImageMemory (source->GetWidth(), 
    source->GetHeight(), source->GetFormat()));

  csRGBpixel *mipmap = new csRGBpixel [Width * Height];
  uint8* Alpha = nimg->GetAlphaPtr();

  int transpidx = -1;
  if (transp && source->GetPalette ())
      transpidx = csImageTools::ClosestPaletteIndex (source->GetPalette(), 
      *transp);

  switch (source->GetFormat() & CS_IMGFMT_MASK)
  {
    case CS_IMGFMT_NONE:
    case CS_IMGFMT_PALETTED8:
      if (source->GetImageData())
	if (transpidx < 0)
	  mipmap_0_p (source->GetWidth(), source->GetHeight(), 
	    (uint8 *)source->GetImageData(), mipmap, source->GetPalette());
	else
	  mipmap_0_pt(source->GetWidth(), source->GetHeight(), 
	    (uint8*)source->GetImageData(), mipmap, source->GetPalette(),
	    transpidx);
      nimg->ConvertFromRGBA (mipmap);
      if (source->GetAlpha())
      {
	mipmap_0_a (source->GetWidth(), source->GetHeight(), 
	  (uint8 *)source->GetAlpha(), Alpha);
      }
      break;
    case CS_IMGFMT_TRUECOLOR:
      if (!transp)
	mipmap_0 (source->GetWidth(), source->GetHeight(), 
	  (csRGBpixel *)source->GetImageData(), mipmap);
      else
	mipmap_0_t (source->GetWidth(), source->GetHeight(), 
	  (csRGBpixel *)source->GetImageData(), mipmap, *transp);
      nimg->ConvertFromRGBA (mipmap);
      break;
  }

  return nimg;
}

csRef<iImage> csImageManipulate::Crop (iImage* source, int x, int y, 
    int width, int height)
{
  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  if (x+width > Width || y+height > Height) return 0;
  csRef<csImageMemory> nimg;
  nimg.AttachNew (new csImageMemory (Width, Height, source->GetFormat()));

  int i;
  if (source->GetAlpha())
  {
    for ( i=0; i<height; i++ )
      memcpy (nimg->GetAlphaPtr() + i*width, 
	source->GetAlpha() + (i+y)*Width + x, width);
  }

  if (source->GetPalette())
  {
    memcpy (nimg->GetPalettePtr(), source->GetPalette(), 
      256 * sizeof (csRGBpixel));
  }

  if (source->GetImageData())
  {      
    switch (source->GetFormat() & CS_IMGFMT_MASK)
    {
      case CS_IMGFMT_NONE:
        break;
      case CS_IMGFMT_PALETTED8:
        for ( i=0; i<height; i++ )
          memcpy ( (uint8*)nimg->GetImagePtr() + i*width, 
	    (uint8*)source->GetImageData() + (i+y)*Width + x, width);
        break;
      case CS_IMGFMT_TRUECOLOR:
        for ( i=0; i<height; i++ )
          memcpy ((csRGBpixel*)nimg->GetImagePtr() + i*width, 
	    (csRGBpixel*)source->GetImageData() + (i+y)*Width + x, 
	    width * sizeof (csRGBpixel));
        break;
    }
  }

  return nimg;
}

csRef<iImage> csImageManipulate::Sharpen (iImage* source, int strength, 
    csRGBpixel* transp)
{
/*

  How it works:

  The algorithm is known as 'Unsharp Mask'. 
  Expressed as a formula:

  sharpened image = original image + 
    strength * (original image - smoothed image)

  You may try
    http://www.dai.ed.ac.uk/HIPR2/unsharp.htm
  for some more information.

*/

  if (strength <= 0) 
    return source;

  const int Width = source->GetWidth();
  const int Height = source->GetHeight();

  // we need an RGB version of ourselves
  csRef<iImage> original; 
  if ((source->GetFormat() & CS_IMGFMT_MASK) != CS_IMGFMT_TRUECOLOR)
  {
    csImageMemory* nimg = new csImageMemory (source, CS_IMGFMT_TRUECOLOR);
    nimg->SetFormat (CS_IMGFMT_TRUECOLOR | 
      (source->GetAlpha() ? CS_IMGFMT_ALPHA : 0));
    original.AttachNew (nimg);
  }
  else
    original = source;
  csRef<iImage> blurry = Blur (original, transp);
  
  csRGBpixel *result = new csRGBpixel [Width * Height];
  csRGBpixel *src_o = (csRGBpixel*)original->GetImageData ();
  csRGBpixel *src_b = (csRGBpixel*)blurry->GetImageData ();
  csRGBpixel *dest = result;
 
  for (int n = Width * Height; n > 0; n--)
  {
    int v;
    #define DO(comp)  \
      v = src_o->comp + ((strength * (src_o->comp - src_b->comp)) >> 8);  \
      dest->comp = (v>255)?255:((v<0)?0:v)

    DO(red);
    DO(green);
    DO(blue);
    DO(alpha);

    #undef DO

    dest++;
    src_o++;
    src_b++;
  }

  csRef<csImageMemory> resimg;
  resimg.AttachNew (new csImageMemory (source->GetWidth(), source->GetHeight(),
    result));
  //delete[] result;

  return resimg;
}

