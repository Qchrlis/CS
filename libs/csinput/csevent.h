/*
  Crystal Space Windowing System: Event class interface
  Copyright (C) 1998 by Jorrit Tyberghein
  Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSEVENT_H__
#define __CSEVENT_H__

#include <stddef.h>
#include "csutil/csbase.h"
#include "csinput/csevdefs.h"

/**
 * This class represents a windowing system event.<p>
 * Events can be generated by hardware (keyboard, mouse)
 * as well as by software. There are so much constructors of
 * this class as much different types of events exists.
 */
class csEvent : public csBase
{
public:
  int Type;
  long Time;			// Time when the event occured
  union
  {
    struct
    {
      int Code;                 // Key code
      int ShiftKeys;            // Control key state
    } Key;
    struct
    {
      int x,y;                  // Mouse coords
      int Button;               // Button number: 1-left, 2-right, 3-middle
      int ShiftKeys;            // Control key state
    } Mouse;
    struct
    {
      unsigned int Code;        // Command code
      void *Info;               // Command info
    } Command;			// to allow virtual destructors
  };

  /// Empty initializer
  csEvent () {}

  /// Create a keyboard event object
  csEvent (long iTime, int eType, int kCode, int kShiftKeys);

  /// Create a mouse event object
  csEvent (long iTime, int eType, int mx, int my, int mbutton, int mShiftKeys);

  /// Create a command event object
  csEvent (long iTime, int eType, int cCode, void *cInfo = NULL);

  /// Destroy an event object
  virtual ~csEvent ();
};

#endif // __CSEVENT_H__
