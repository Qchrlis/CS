/*
    Copyright (C) 1999 by Eric Sunshine <sunshine@sunshineco.com>
    Writen by Eric Sunshine <sunshine@sunshineco.com>

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

#include "sysdef.h"
#include "cssys/common/csppulse.h"
#include "cssys/common/system.h"

static char const ANIMATION[] = "-\\|/";
int const ANIMATION_COUNT = sizeof(ANIMATION) / sizeof(ANIMATION[0]) - 1;

static int GLOBAL_STATE = 0;

csProgressPulse::csProgressPulse(bool inherit_global_state) :
  type(MSG_INITIALIZATION), state(0)
{
  if (inherit_global_state)
    state = GLOBAL_STATE;
  Step("");
}

csProgressPulse::~csProgressPulse()
{
  if (System != 0)
    System->Printf (type, "\b \b");
  GLOBAL_STATE = state;
}

void csProgressPulse::Step(char const* prefix)
{
  System->Printf (type, "%s%c", prefix, ANIMATION[state]);
  if (++state >= ANIMATION_COUNT)
    state = 0;
}

void csProgressPulse::Step()
{
  Step("\b");
}
