/*
    Crystal Space 3D engine: Event class interface
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

#ifndef __CS_CSEVENT_H__
#define __CS_CSEVENT_H__

#include "csextern.h"
#include "iutil/event.h"
#include "hashr.h"
#include "hashhandlers.h"
#include "csendian.h"
#include "weakref.h"
#include "cseventq.h"

/// Various datatypes supported by the event system (csEvent).
enum
{
  CS_DATATYPE_INT8 = 0x00,
  CS_DATATYPE_UINT8,
  CS_DATATYPE_INT16,
  CS_DATATYPE_UINT16,
  CS_DATATYPE_INT32,
  CS_DATATYPE_UINT32,
  CS_DATATYPE_INT64,
  CS_DATATYPE_UINT64,
  CS_DATATYPE_FLOAT,
  CS_DATATYPE_DOUBLE,
  CS_DATATYPE_BOOL,
  CS_DATATYPE_STRING,
  CS_DATATYPE_DATABUFFER,
  CS_DATATYPE_EVENT
};

/**
 * This class represents a system event.<p>
 * Events can be generated by hardware (keyboard, mouse)
 * as well as by software. There are so much constructors of
 * this class as much different types of events exists.
 */
class CS_CSUTIL_EXPORT csEvent : public iEvent
{
private:
  typedef struct attribute_tag
  {
    union
    {
      int64 Integer;
      uint64 Unsigned;
      double Double;
      char *String;
      bool Bool;
      iEvent *Event;
    };
    enum Type
    {
      tag_int8,
      tag_uint8,
      tag_int16,
      tag_uint16,
      tag_int32,
      tag_uint32,
      tag_int64,
      tag_uint64,
      tag_float,
      tag_double,
      tag_string,
      tag_databuffer,
      tag_bool,
      tag_event
    } type;
    uint32 length;
    attribute_tag (Type t) { type = t; }
    ~attribute_tag() 
    { 
      if ((type == tag_string) || (type == tag_databuffer)) 
	delete[] String; 
      else if (type == tag_event)
	Event->DecRef();
    }
  } attribute;

  csHash<attribute*, csStrKey, csConstCharHashKeyHandler> attributes;

  uint32 count;

  bool CheckForLoops(iEvent *current, iEvent *e);

  bool FlattenCrystal(char *buffer);
  bool FlattenMuscle(char *buffer);
  bool FlattenXML(char *buffer);

  uint32 FlattenSizeCrystal();
  uint32 FlattenSizeMuscle();
  uint32 FlattenSizeXML();

  bool UnflattenCrystal(const char *buffer, uint32 length);
  bool UnflattenMuscle(const char *buffer, uint32 length);
  bool UnflattenXML(const char *buffer, uint32 length);

  template<class T, attribute_tag::Type typeID>
  bool AddInternalInt (const char* name, T value)
  {
    attribute* object = new attribute (typeID);
    object->Integer = value;
    attributes.Put (name, object);
    count++;
    return true;
  }

  bool SkipToIndex (
    csHash<attribute*, csStrKey, csConstCharHashKeyHandler>::Iterator& iter,
    int index) const
  {
    while (index-- > 0)
    {
      if (!iter.HasNext()) return false;
      iter.Next();
    }
    return iter.HasNext();
  }

  static char const* GetTypeName (attribute::Type t);
protected:
  virtual csRef<iEvent> CreateEvent();

public:
  /// Empty initializer
  csEvent ();

  /**
   * Cloning constructor.  Note that for command style events, this performs
   * only a shallow copy of the `Info' attribute.
   */
  csEvent (csEvent const&);

  /// Create a mouse event object
  csEvent (csTicks, int type, int x, int y, int button, int modifiers);

  /// Create a joystick event object
  csEvent (csTicks, int type, int n, int x, int y, int button, int modifiers);

  /// Create a command event object
  csEvent (csTicks, int type, int code, void* info = 0);

  /// Destructor
  virtual ~csEvent ();

  /// Add a named event with a given parameter.

#define CS_CSEVENT_ADDINT(type)					\
  virtual bool Add (const char* name, type value)		\
  {								\
    attribute* object = new attribute (attribute::tag_##type);	\
    object->Integer = value;					\
    attributes.Put (name, object);				\
    count++;							\
    return true;						\
  }

  CS_CSEVENT_ADDINT(int8)
  CS_CSEVENT_ADDINT(uint8)
  CS_CSEVENT_ADDINT(int16)
  CS_CSEVENT_ADDINT(uint16)
  CS_CSEVENT_ADDINT(uint32)
  CS_CSEVENT_ADDINT(int64)
  CS_CSEVENT_ADDINT(uint64)
#undef CS_CSEVENT_ADDINT
  virtual bool Add (const char *name, int32 v, bool force_boolean = false);
  virtual bool Add (const char *name, float v);
  virtual bool Add (const char *name, double v);
  virtual bool Add (const char *name, const char *v);
  virtual bool Add (const char *name, const void *v, uint32 size);
#ifndef CS_USE_FAKE_BOOL_TYPE
  virtual bool Add (const char *name, bool v, bool force_boolean = true);
#endif
  virtual bool Add (const char *name, iEvent *v);

  /// Find a named event for a given type.
#define CS_CSEVENT_FINDINT(T)						\
  virtual bool Find (const char* name, T& value, int index) const	\
  {									\
    csHash<attribute*, csStrKey, csConstCharHashKeyHandler>::Iterator	\
      iter (attributes.GetIterator (name));				\
    if (!SkipToIndex (iter, index)) return false;			\
    attribute* object = iter.Next();					\
    if (object->type == attribute::tag_##T)				\
    {									\
      value = (T)object->Integer;					\
      return true;							\
    }									\
    return false;							\
  }
  CS_CSEVENT_FINDINT(int8)
  CS_CSEVENT_FINDINT(uint8)
  CS_CSEVENT_FINDINT(int16)
  CS_CSEVENT_FINDINT(uint16)
  CS_CSEVENT_FINDINT(int32)
  CS_CSEVENT_FINDINT(uint32)
  CS_CSEVENT_FINDINT(int64)
  CS_CSEVENT_FINDINT(uint64)
#undef CS_CSEVENT_FINDINT
  virtual bool Find (const char *name, float &v, int index = 0) const;
  virtual bool Find (const char *name, double &v, int index = 0) const;
  virtual bool Find (const char *name, const char *&v, int index = 0) const;
  virtual bool Find (const char *name, const void *&v, uint32 &size, 
    int index = 0) const;
#ifndef CS_USE_FAKE_BOOL_TYPE
  virtual bool Find (const char *name, bool &v, int index = 0) const;
#endif
  virtual bool Find (const char *name, csRef<iEvent> &v, int index = 0) const;

  virtual bool Remove (const char *name, int index = -1);
  virtual bool RemoveAll ();

  virtual uint32 FlattenSize (int format = CS_CRYSTAL_PROTOCOL);
  virtual bool Flatten (char *buffer, int format = CS_CRYSTAL_PROTOCOL);
  virtual bool Unflatten (const char *buffer, uint32 length);

  virtual bool Print (int level = 0);

  SCF_DECLARE_IBASE;
};

/**
 * This class is a system event designed for the pool system<p>
 * Due to the possibilities of networking traffic and other assorted
 * events traversing the event system, a more efficient method of
 * event creation was needed.  Thus, the event pool was born, and
 * there are the events that reside within it.
 */
class CS_CSUTIL_EXPORT csPoolEvent : public csEvent
{
  typedef csEvent superclass;
  friend class csEventQueue;
  friend class csEvent;

private:
  // As per the XML pool, keep a reference to the pool container obejct
  // and this also allows our overridden DecRef() to place the event back
  // into the pool when users are done with it.
  csWeakRef<csEventQueue> pool;

  // The next event in the pool, or null if the event is in use.
  csPoolEvent *next;

  // The 'real' DecRef() call that deletes the event, should in theory only be
  // called from csEventQueue.
  void Free () { csEvent::DecRef(); }

protected:
  virtual csRef<iEvent> CreateEvent();

public:
  /// The constructor; should only be called from within csEventQueue.
  csPoolEvent (csEventQueue *q);

  /// Places the event back into the pool if this is the last reference.
  virtual void DecRef ();
};

#endif // __CS_CSEVENT_H__
