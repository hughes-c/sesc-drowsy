/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Jose Renau

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "callback.h"

EventScheduler::TimedCallbacksQueue EventScheduler::cbQ(32);

Time_t globalClock = 0;

Time_t global_stateCycles[NUM_STATES];
Time_t proc_stateCycles[256][NUM_STATES];

Time_t proc_gateCycles[256][NUM_STATES];



void EventScheduler::dump() const
{
#ifdef DEBUG
  MSG("Callback registered at File %s LineNo %d", fileName, lineno);
#else
  MSG("No caller recorded. Compile in DEBUG mode");
#endif
}
