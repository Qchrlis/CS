/*
    Crystal Space Shared Class Facility (SCF)
    Copyright (C) 2005 by Marten Svanfeldt and Michael D. Adams

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

/*
 * This header is intended to be included from scf_new.h multiple times.
 * Each time either an scfImplementationN or an scfImplementationExtN class
 * will be declared depending on the value of SCF_IMPL_N and whether
 * SCF_IMPL_EXT is defined.
 */

#ifndef SCF_IN_IMPLGEN_H
#error Do not include this file directly. Included from scf_implgen.h
#endif

#ifndef SCF_IMPL_EXT
#  define SCF_IMPL_NAME SCF_IMPL_CAT(scfImplementation,SCF_IMPL_N)
#  define SCF_IMPL_SUPER scfImplementation<Class>
#  define SCF_IMPL_PRE_TYPES
#  define SCF_IMPL_PRE_ARGS
#else /* SCF_IMPL_EXT */
#  define SCF_IMPL_NAME SCF_IMPL_CAT(scfImplementationExt,SCF_IMPL_N)
#  define SCF_IMPL_SUPER Super
#  define SCF_IMPL_PRE_TYPES ,class Super
#  define SCF_IMPL_PRE_ARGS ,Super
#endif

#if SCF_IMPL_N == 0
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES
#  define SCF_IMPL_INTERFACES
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS
#elif SCF_IMPL_N == 1
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES ,class I1
#  define SCF_IMPL_INTERFACES ,public I1
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1
#elif SCF_IMPL_N == 2
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES ,class I1, class I2
#  define SCF_IMPL_INTERFACES ,public I1, public I2
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2
#elif SCF_IMPL_N == 3
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES ,class I1, class I2, class I3
#  define SCF_IMPL_INTERFACES ,public I1, public I2, public I3
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3
#elif SCF_IMPL_N == 4
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3, I4
#elif SCF_IMPL_N == 5
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4, class I5
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4, public I5
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3, I4, I5
#elif SCF_IMPL_N == 6
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4, class I5, \
    class I6
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4, public I5, \
    public I6
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3, I4, I5, I6
#elif SCF_IMPL_N == 7
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4, class I5, \
    class I6, class I7
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4, public I5, \
    public I6, public I7
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3, I4, I5, I6, I7
#elif SCF_IMPL_N == 8
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4, class I5, \
    class I6, class I7, class I8
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4, public I5, \
    public I6, public I7, public I8
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3, I4, I5, I6, I7, I8
#elif SCF_IMPL_N == 9
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4, class I5, \
    class I6, class I7, class I8, class I9
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4, public I5, \
    public I6, public I7, public I8, public I9
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS ,I1, I2, I3, I4, I5, I6, I7, I8, I9
#elif SCF_IMPL_N == 10
#  define SCF_IMPL_TYPES SCF_IMPL_PRE_TYPES \
    ,class I1, class I2, class I3, class I4, class I5, \
    class I6, class I7, class I8, class I9, class I10
#  define SCF_IMPL_INTERFACES \
    ,public I1, public I2, public I3, public I4, public I5, \
    public I6, public I7, public I8, public I9, public I10
#  define SCF_IMPL_ARGS SCF_IMPL_PRE_ARGS \
    ,I1, I2, I3, I4, I5, I6, I7, I8, I9, I10
#else
#  error Unsuported value of SCF_IMPL_N
#endif

// Taken from http://www.boost.org/boost/preprocessor/cat.hpp
#define SCF_IMPL_CAT(a, b) SCF_IMPL_CAT_I(a, b)
#define SCF_IMPL_CAT_I(a, b) SCF_IMPL_CAT_II(a ## b)
#define SCF_IMPL_CAT_II(res) res

template<class Class SCF_IMPL_TYPES>
class CS_CRYSTALSPACE_EXPORT SCF_IMPL_NAME :
  public SCF_IMPL_SUPER
  SCF_IMPL_INTERFACES
{
public:
  inline void *QueryInterface(scfInterfaceID id, scfInterfaceVersion version)
  {
#if SCF_IMPL_N >= 1
    void *x; // Inside #if avoids unused variable warnings for SCF_IMPL_N == 0
    if((x = GetInterface<I1>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 2
    if((x = GetInterface<I2>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 3
    if((x = GetInterface<I3>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 4
    if((x = GetInterface<I4>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 5
    if((x = GetInterface<I5>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 6
    if((x = GetInterface<I6>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 7
    if((x = GetInterface<I7>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 8
    if((x = GetInterface<I8>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 9
    if((x = GetInterface<I9>(this->scfObject, id, version)) != 0) return x;
#endif
#if SCF_IMPL_N >= 10
    if((x = GetInterface<I10>(this->scfObject, id, version)) != 0) return x;
#endif

    return SCF_IMPL_SUPER::QueryInterface(id, version);
  }

protected:
#ifndef SCF_IMPL_EXT
  SCF_IMPL_NAME(Class *object, iBase *parent=0)
    : SCF_IMPL_SUPER(object, parent)
  { 
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }
#else /* SCF_IMPL_EXT */
  SCF_IMPL_NAME(Class *object)
    : SCF_IMPL_SUPER(), scfObject(object) 
  { 
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  template<class T1>
  SCF_IMPL_NAME(Class *object, T1 t1)
    : SCF_IMPL_SUPER(t1), scfObject(object) 
  { 
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  template<class T1, class T2>
  SCF_IMPL_NAME(Class *object, T1 t1, T2 t2)
    : SCF_IMPL_SUPER(t1, t2), scfObject(object) 
  { 
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  template<class T1, class T2, class T3>
  SCF_IMPL_NAME(Class *object, T1 t1, T2 t2, T3 t3)
    : SCF_IMPL_SUPER(t1, t2, t3), scfObject(object) 
  { 
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  template<class T1, class T2, class T3, class T4>
  SCF_IMPL_NAME(Class *object, T1 t1, T2 t2, T3 t3, T4 t4)
    : SCF_IMPL_SUPER(t1, t2, t3, t4), scfObject(object) 
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  template<class T1, class T2, class T3, class T4, class T5>
  SCF_IMPL_NAME(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    : SCF_IMPL_SUPER(t1, t2, t3, t4, t5), scfObject(object) 
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  template<class T1, class T2, class T3, class T4, class T5, class T6>
  SCF_IMPL_NAME(Class *object, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    : SCF_IMPL_SUPER(t1, t2, t3, t4, t5, t6), scfObject(object) 
  {
    csRefTrackerAccess::SetDescription (object, CS_TYPENAME (Class));
    AddReftrackerAliases(); 
  }

  Class *scfObject;
#endif

  virtual ~SCF_IMPL_NAME() 
  {
    RemoveReftrackerAliases();
  }

  typedef SCF_IMPL_NAME<Class SCF_IMPL_ARGS> scfImplementationType;
  typedef Class scfClassType; 

private:
  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void* GetInterface (
    Class* scfObject, scfInterfaceID id, scfInterfaceVersion version)
  {
    if (id == scfInterfaceTraits<I>::GetID() &&
      scfCompatibleVersion(version, scfInterfaceTraits<I>::GetVersion()))
    {
      scfObject->IncRef();
      return static_cast<
        typename_qualifier scfInterfaceTraits<I>::InterfaceType*> (scfObject);
    }
    else
    {
      return 0;
    }
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void AddReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::AddAlias(
      static_cast<
        typename_qualifier scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }

  template<typename I>
  CS_FORCEINLINE_TEMPLATEMETHOD static void RemoveReftrackerAlias (
    Class* scfObject)
  {
    csRefTrackerAccess::RemoveAlias(
      static_cast<
        typename_qualifier scfInterfaceTraits<I>::InterfaceType*> (scfObject),
      scfObject);
  }
 
  void AddReftrackerAliases ()
  {
#if SCF_IMPL_N >= 1
    AddReftrackerAlias<I1>(this->scfObject);
#endif
#if SCF_IMPL_N >= 2
    AddReftrackerAlias<I2>(this->scfObject);
#endif
#if SCF_IMPL_N >= 3
    AddReftrackerAlias<I3>(this->scfObject);
#endif
#if SCF_IMPL_N >= 4
    AddReftrackerAlias<I4>(this->scfObject);
#endif
#if SCF_IMPL_N >= 5
    AddReftrackerAlias<I5>(this->scfObject);
#endif
#if SCF_IMPL_N >= 6
    AddReftrackerAlias<I6>(this->scfObject);
#endif
#if SCF_IMPL_N >= 7
    AddReftrackerAlias<I7>(this->scfObject);
#endif
#if SCF_IMPL_N >= 8
    AddReftrackerAlias<I8>(this->scfObject);
#endif
#if SCF_IMPL_N >= 9
    AddReftrackerAlias<I9>(this->scfObject);
#endif
#if SCF_IMPL_N >= 10
    AddReftrackerAlias<I10>(this->scfObject);
#endif
  }

  void RemoveReftrackerAliases ()
  {
#if SCF_IMPL_N >= 1
    RemoveReftrackerAlias<I1>(this->scfObject);
#endif
#if SCF_IMPL_N >= 2
    RemoveReftrackerAlias<I2>(this->scfObject);
#endif
#if SCF_IMPL_N >= 3
    RemoveReftrackerAlias<I3>(this->scfObject);
#endif
#if SCF_IMPL_N >= 4
    RemoveReftrackerAlias<I4>(this->scfObject);
#endif
#if SCF_IMPL_N >= 5
    RemoveReftrackerAlias<I5>(this->scfObject);
#endif
#if SCF_IMPL_N >= 6
    RemoveReftrackerAlias<I6>(this->scfObject);
#endif
#if SCF_IMPL_N >= 7
    RemoveReftrackerAlias<I7>(this->scfObject);
#endif
#if SCF_IMPL_N >= 8
    RemoveReftrackerAlias<I8>(this->scfObject);
#endif
#if SCF_IMPL_N >= 9
    RemoveReftrackerAlias<I9>(this->scfObject);
#endif
#if SCF_IMPL_N >= 10
    RemoveReftrackerAlias<I10>(this->scfObject);
#endif
  }
};

#undef SCF_IMPL_NAME
#undef SCF_IMPL_SUPER
#undef SCF_IMPL_PRE_TYPES
#undef SCF_IMPL_PRE_ARGS

#undef SCF_IMPL_TYPES
#undef SCF_IMPL_ARGS
#undef SCF_IMPL_INTERFACES

