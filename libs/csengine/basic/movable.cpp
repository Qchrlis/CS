/*
    Copyright (C) 2000 by Jorrit Tyberghein

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
#include "qint.h"
#include "csengine/movable.h"
#include "csengine/sector.h"
#include "csengine/thing.h"
#include "csengine/meshobj.h"
#include "csengine/cscoll.h"
#include "isector.h"

//---------------------------------------------------------------------------

IMPLEMENT_IBASE (csMovable)
  IMPLEMENTS_INTERFACE (iBase)
  IMPLEMENTS_EMBEDDED_INTERFACE (iMovable)
IMPLEMENT_IBASE_END

IMPLEMENT_EMBEDDED_IBASE (csMovable::eiMovable)
  IMPLEMENTS_INTERFACE (iMovable)
IMPLEMENT_EMBEDDED_IBASE_END


csMovable::csMovable ()
{
  CONSTRUCT_IBASE (NULL);
  CONSTRUCT_EMBEDDED_IBASE (scfiMovable);
  parent = NULL;
  iparent = NULL;
}

csMovable::~csMovable ()
{
  //@@@
  // The following DecRef() is not possible because we
  // actually have a circular reference here (parent -> this and
  // this -> parent).
  //if (iparent) iparent->DecRef ();
}

void csMovable::SetParent (csMovable* parent)
{
  //@@@ (see comment above)
  //iMovable* ipar = QUERY_INTERFACE_SAFE (parent, iMovable);
  //if (iparent) iparent->DecRef ();
  //iparent = ipar;
  iparent = QUERY_INTERFACE_SAFE (parent, iMovable);
  if (iparent) iparent->DecRef ();

  csMovable::parent = parent;
}

void csMovable::SetPosition (csSector* home, const csVector3& pos)
{
  obj.SetOrigin (pos);
  SetSector (home);
}

void csMovable::SetTransform (const csMatrix3& matrix)
{
  obj.SetT2O (matrix);
}

void csMovable::MovePosition (const csVector3& rel)
{
  obj.Translate (rel);
}

void csMovable::Transform (const csMatrix3& matrix)
{
  obj.SetT2O (matrix * obj.GetT2O ());
}

void csMovable::ClearSectors ()
{
  if (parent == NULL)
  {
    if (object->GetType () >= csThing::Type)
    {
      csThing* th = (csThing*)object;
      th->RemoveFromSectors ();
    }
    else if (object->GetType () >= csMeshWrapper::Type)
    {
      csMeshWrapper* sp = (csMeshWrapper*)object;
      sp->RemoveFromSectors ();
    }
    else
    {
      csCollection* col = (csCollection*)object;
      col->RemoveFromSectors ();
    }
    sectors.SetLength (0);
  }
}

void csMovable::AddSector (csSector* sector)
{
  if (sector == NULL) return;
  if (parent == NULL)
  {
    sectors.Push (sector);
    if (object->GetType () >= csThing::Type)
    {
      csThing* th = (csThing*)object;
      th->MoveToSector (sector);
    }
    else if (object->GetType () >= csMeshWrapper::Type)
    {
      csMeshWrapper* sp = (csMeshWrapper*)object;
      sp->MoveToSector (sector);
    }
    else
    {
      csCollection* col = (csCollection*)object;
      col->MoveToSector (sector);
    }
  }
}

void csMovable::UpdateMove ()
{
  if (object->GetType () >= csThing::Type)
  {
    csThing* th = (csThing*)object;
    th->UpdateMove ();
  }
  else if (object->GetType () >= csMeshWrapper::Type)
  {
    csMeshWrapper* sp = (csMeshWrapper*)object;
    sp->UpdateMove ();
  }
  else
  {
    csCollection* col = (csCollection*)object;
    col->UpdateMove ();
  }
}

csReversibleTransform csMovable::GetFullTransform () const
{
  if (parent == NULL)
    return GetTransform ();
  else
    return GetTransform () * parent->GetFullTransform ();
}

//--------------------------------------------------------------------------

void csMovable::eiMovable::SetSector (iSector* sector)
{
  scfParent->SetSector (sector->GetPrivateObject ());
}

void csMovable::eiMovable::AddSector (iSector* sector)
{
  scfParent->AddSector (sector->GetPrivateObject ());
}

void csMovable::eiMovable::SetPosition (iSector* home, const csVector3& v)
{
  scfParent->SetPosition (home->GetPrivateObject (), v);
}

iSector* csMovable::eiMovable::GetSector (int idx)
{
  csSector* sect = (csSector*)scfParent->GetSectors ()[idx];
  if (!sect) return NULL;
  return &sect->scfiSector;
}

