/*
    Copyright (C) 2007 by Seth Yastrov

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
#include "csutil/scf.h"

#include <csutil/objreg.h>
#include <iutil/document.h>
#include <csutil/weakrefarr.h>
#include <ivaria/reporter.h>

#include "spacemanager.h"

#include "mainframe.h"

#include <wx/colordlg.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/textdlg.h>
#include <wx/bitmap.h>

#include "ieditor/context.h"
#include "ieditor/header.h"
#include "ieditor/layout.h"


namespace CS {
namespace EditorApp {
#include "data/editor/images/sceneIcon.xpm"  
struct SpaceFactory : public scfImplementation1<SpaceFactory,iSpaceFactory>
{
  SpaceFactory(iObjectRegistry* obj_reg);
  virtual csPtr<iSpace> Create(wxWindow* parent);
  virtual const char* GetIdentifier () const;
  virtual const char* GetLabel () const;
  virtual const wxBitmap& GetIcon () const;
  virtual bool AllowMultiple () const;
  virtual size_t GetCount ();
  iObjectRegistry* object_reg;
  csString identifier;
  csString label;
  wxBitmap icon;
  bool allowMultiple;
  csWeakRefArray<iSpace> spaces; 
};

SpaceFactory::SpaceFactory(iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg), icon(sceneIcon_xpm), allowMultiple(true)
{
}
const char* SpaceFactory::GetIdentifier () const { return identifier; }
const char* SpaceFactory::GetLabel () const { return label; }
const wxBitmap& SpaceFactory::GetIcon () const { return icon; }
bool SpaceFactory::AllowMultiple () const { return allowMultiple; }
csPtr<iSpace> SpaceFactory::Create(wxWindow* parent)
{
  csRef<iBase> base = iSCF::SCF->CreateInstance(identifier);
  if (!base)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.space", "Failed to instantiate space '%s'", identifier.GetData());
    return 0;
  }
  csRef<iSpace> ref = scfQueryInterface<iSpace> (base);
  if (!ref)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.space", "Not of type iSpace: '%s'", identifier.GetData());
    return 0;
  }
  base->DecRef();
  ref->Initialize(object_reg, this, parent);
  spaces.Push(ref); 
  return csPtr<iSpace> (ref);
}

size_t SpaceFactory::GetCount ()
{
  spaces.Compact(); 
  return spaces.GetSize();
}



SpaceManager::SpaceManager (iObjectRegistry* obj_reg, wxWindow* parent)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iSpaceManager");
  csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
  context->AddListener(this);
}

SpaceManager::~SpaceManager ()
{
  object_reg->Unregister (this, "iSpaceManager");
  csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
  if (context) context->RemoveListener(this);
}

bool SpaceManager::Register (const char* identifier)
{
  csRef<iSpaceFactory> fact = factories.Get(identifier, csRef<iSpaceFactory>());
  if (!fact)
  {
    csRef<iDocumentNode> klass = iSCF::SCF->GetPluginMetadataNode(identifier);
    if (klass)
    {
      csRef<SpaceFactory> f; f.AttachNew (new SpaceFactory(object_reg));
      f->identifier = identifier;
      csRef<iDocumentNode> m = klass->GetNode("allowMultiple");
      if (m) f->allowMultiple = strcmp(m->GetContentsValue (), "true") == 0;
      csRef<iDocumentNode> label = klass->GetNode("description");
      if (label) f->label = label->GetContentsValue ();
      
      factories.PutUnique(identifier, f);
      return true;
    }
  }
  return false;
}

bool SpaceManager::Register (iHeader* header)
{
  printf("SpaceManager::Register header \n");
  csRef<iFactory> fact = scfQueryInterface<iFactory> (header);
  if (fact) 
  {
    printf("SpaceManager::Register header %s\n", fact->QueryClassID());
    csRef<iDocumentNode> klass = iSCF::SCF->GetPluginMetadataNode(fact->QueryClassID());
    if (klass)
    {
      csRef<iDocumentNode> space = klass->GetNode("space");
      printf("SpaceManager::Register header %s\n", space->GetContentsValue ());
      headers.PutUnique(space->GetContentsValue (), header);
      return true;
    }
  }
  csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.space", "SpaceManager::Register failed to register header!");
  return false;
}

bool SpaceManager::Register (iPanel*)
{
  return true;
}

const csHash<csRef<iSpaceFactory>, csString>& SpaceManager::GetAll ()
{
  return factories;
}

void SpaceManager::Initialize ()
{
}

void SpaceManager::Uninitialize ()
{
}

void SpaceManager::OnChanged (iContext* context)
{
  printf("SpaceManager::OnChanged \n");
  csHash<csRef<iSpaceFactory>, csString>::ConstGlobalIterator facts = GetAll().GetIterator ();
  while (facts.HasNext())
  {
    iSpaceFactory* n = facts.Next();
    SpaceFactory* f = static_cast<SpaceFactory*>(n);
    if (!f) continue;
    f->spaces.Compact ();
    csWeakRefArray<iSpace>::Iterator spaces = f->spaces.GetIterator ();
    while (spaces.HasNext())
    {
      iSpace* space = spaces.Next();
      //if (!space || !space->GetFactory()) continue;
      const char* id = space->GetFactory()->GetIdentifier();
      printf("SpaceManager::OnChanged %s\n", id);
      csRef<iHeader> header = headers.Get(id, csRef<iHeader>());
      if (header)
      {
        header->Draw(context, 0);
      }
    }
  }
}

} // namespace EditorApp
} // namespace CS