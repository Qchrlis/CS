/*
    Copyright (C) 2001 by Norman Kr�mer
  
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
#include "avistra.h"
#include "isystem.h"

IMPLEMENT_IBASE (csAVIStreamAudio)
  IMPLEMENTS_INTERFACE (iAudioStream)
  IMPLEMENTS_INTERFACE (iStream)
IMPLEMENT_IBASE_END

csAVIStreamAudio::csAVIStreamAudio (iBase *pBase)
{
  CONSTRUCT_IBASE (pBase);
  pChunk = NULL;
  pAVI = (csAVIFormat*)pBase;
  pSystem = NULL;
  pCodec = NULL;
}

bool csAVIStreamAudio::Initialize (const csAVIFormat::AVIHeader *ph, 
				   const csAVIFormat::StreamHeader *psh, 
				   const csAVIFormat::AudioStreamFormat *pf, 
				   UShort nStreamNumber, iSystem *pTheSystem)
{

  (void)ph;
  strdesc.type = CS_STREAMTYPE_AUDIO;
  memcpy (strdesc.codec, psh->handler, 4);
  strdesc.codec[4] = '\0';
  strdesc.formattag = pf->formattag;
  strdesc.channels = pf->channels;
  strdesc.samplespersecond = pf->samplespersecond;
  strdesc.bitspersample = pf->bitspersample;
  strdesc.duration = psh->length / psh->scale;

  delete pChunk;
  pChunk = new csAVIFormat::AVIDataChunk;
  pChunk->currentframe = 0;
  pChunk->currentframepos = NULL;
  sprintf (pChunk->id, "%02dw", nStreamNumber);
  pChunk->id[3] = '\0';

  nStream = nStreamNumber;
  if (pSystem) pSystem->DecRef ();
  (pSystem = pTheSystem)->IncRef ();

  bTimeSynced = false;
  // load the CODEC
  return LoadCodec ();
}

csAVIStreamAudio::~csAVIStreamAudio ()
{
  delete pChunk;
  if (pCodec) pCodec->DecRef ();
  if (pSystem) pSystem->DecRef ();
}

void csAVIStreamAudio::GetStreamDescription (csStreamDescription &desc)
{
  memcpy (&desc, (csStreamDescription*)&strdesc, sizeof (csStreamDescription));
}

bool csAVIStreamAudio::GotoFrame (ULong frameindex)
{
  return pAVI->GetChunk (frameindex, pChunk);
}

bool csAVIStreamAudio::GotoTime (ULong timeindex)
{
  (void)timeindex;
  // not yet implemented
  return false;
}

bool csAVIStreamAudio::SetPlayMethod (bool bTimeSynced)
{
  // timesynced isnt yet implemented, so return false if its requested.
  return bTimeSynced == false;
}

void csAVIStreamAudio::GetStreamDescription (csAudioStreamDescription &desc)
{
  memcpy (&desc, &strdesc, sizeof (csAudioStreamDescription));
}

void csAVIStreamAudio::NextFrame ()
{
  /*
  if (pAVI->GetChunk (pChunk->currentframe++, pChunk))
  {
    pCodec->Decode (pChunk->data, pChunk->length, outdata);
    if (pCodecDesc->decodeoutput != CS_CODECFORMAT_PCM)
      return;

    // @@@TODO: do the actual playing of sound here
  }
  */
}

bool csAVIStreamAudio::LoadCodec ()
{
  // based on the codec id we try to load the apropriate codec
  iSCF *pSCF = QUERY_INTERFACE (pSystem, iSCF);
  if (pSCF)
  {
    // create a classname from the coec id
    char cn[128];
    sprintf (cn, "crystalspace.audio.codec.%s", strdesc.codec);
    // try open this class
    pCodec = (iCodec*)pSCF->scfCreateInstance (cn, "iCodec", 0);
    pSCF->DecRef ();
    if (pCodec)
    {
      // codec exists, now try to initialize it
      if (pCodec->Initialize (&strdesc))
	return true;
      else
      {
	pSystem->Printf (MSG_WARNING, "CODEC class \"%s\" could not be initialized !", cn);
	pCodec->DecRef ();
	pCodec = NULL;
      }
    }
    else
      pSystem->Printf (MSG_WARNING, "CODEC class \"%s\" could not be loaded !", cn);
  }
  else
    pSystem->Printf (MSG_DEBUG_0, "Could not get an SCF interface from the systemdriver !");
  return false;
}
