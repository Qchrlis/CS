/*
  Copyright (C) 2006 by Marten Svanfeldt

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

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include "config.h"
#include "tui.h"

namespace lighter
{
  /// Global statistics object
  class Statistics
  {
  public:
    class SubProgress;

    class Progress
    {
      friend class SubProgress;

      /// Total amount of all sub-progress objects
      float totalAmount;
      /// Task name
      const char* taskName;

      inline void SetTaskName (const char* taskName)
      {
        this->taskName = taskName;
        globalTUI.Redraw ();
      }
      inline void SetProgress (const char* task, float overallProgress, 
                               float taskProgress)
      {
        this->overallProgress = 100.0f * (overallProgress / totalAmount);
        this->taskProgress = taskProgress;
        int redrawFlags;
        if (taskName != task)
        {
          taskName = task;
          redrawFlags = TUI::TUI_DRAW_ALL;
        }
        else
          redrawFlags = TUI::TUI_DRAW_PROGRESS;

        globalTUI.Redraw (redrawFlags);
      }
      /// Overall progress  0-1000
      float overallProgress;

      /// Current task progress 0-1000
      float taskProgress;
    public:
      Progress () 
        : totalAmount (0), taskName (0), overallProgress (0), taskProgress (0)
      {
      }

      const char* GetTaskName() const { return taskName; }
      float GetOverallProgress() const { return overallProgress; }
      float GetTaskProgress() const { return taskProgress; }
    } progress;

    /// Progress for a single task.
    class SubProgress
    {
      /// Task description
      csString taskName;

      float subProgressStart, subProgressAmount, progress;
    public:
      /**
       * Create a "sub-progress" object for a task. \a name is the 
       * description of the task and \a amount a relavive value for the
       * amount of work this task needs. It doesn't have a unit and doesn't
       * need to sum up to some special value; it can be completely arbitrary,
       * but the amounts of different tasks should be roughly in relation to
       * the time usually takes to complete them.
       */
      SubProgress (const char* name, float amount);

      /// Set complete progress, in percent.
      void SetProgress (float progress);

      /// Increment progress
      inline void IncProgress (float inc)
      {
        SetProgress (progress + inc);
      }

      /// Set description
      void SetTaskName (const char* taskName);
    };

    struct Raytracer
    {
      Raytracer ()
        : numRays (0), usRaytracing (0)
      {}

      /// Number of rays traced
      uint64 numRays;

      /// Number of uS spent raytracing
      uint64 usRaytracing;
    } raytracer;

    struct KDTree
    {
      KDTree ()
        : numNodes (0), leafNodes (0), maxDepth (0), sumDepth (0),
        numPrimitives (0)
      {}

      /// Number of inner nodes
      size_t numNodes;

      /// Number of leaf nodes
      size_t leafNodes;

      /// Max depth of leaf node
      size_t maxDepth;

      /// Sum depth of leaf-nodes
      size_t sumDepth;

      /// Total number of primitives in leafs
      size_t numPrimitives;     
    } kdtree;
  };

  extern Statistics globalStats;


  
}

#endif
