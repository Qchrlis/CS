/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_CSGFX_SYMTABLE_H__
#define __CS_CSGFX_SYMTABLE_H__

#include "csutil/scf.h"
#include "csutil/ref.h"
#include "csutil/strhash.h"
#include "csutil/array.h"

/**
 * This class provides a system for storing inheritable properties, by allowing
 * instances to be stacked in a tree formation with parent/child relationships.
 * <p>
 * Used by the render3d shader system.
 */
class csSymbolTable
{
private:
  struct Symbol
  {
    csStringID Name;
    csRef<iBase> Val;
    bool Auth;
    inline Symbol (csStringID id, iBase *value, bool authoritative)
    : Name (id), Val (value), Auth (authoritative) {}
  };

protected:
  csHashMap Hash;

  csArray<csSymbolTable*> Children;
  csSymbolTable *Parent;

  /// Inherit all symbols from the parent.
  inline void SetParent (csSymbolTable *);

  /// Set the symbol in all children.
  inline void PropagateSymbol (csStringID name, iBase *value);

  /// Delete the symbol in all children.
  inline void PropagateDelete (csStringID name);

  /// Same as SetSymbol only if there is not already a symbol with that name.
  inline void SetSymbolSafe (csStringID name, iBase *value);

  /// Same as DeleteSymbol only if the symbol is inherited from the parent.
  inline void DeleteSymbolSafe (csStringID);

public:
  /**
   * Construct the table with a hash of the given initial size,
   * which should be a prime number, for optimisation reasons.
   * See the csHashMap docs for more info.
   */
  csSymbolTable (int size = 53) : Hash (size), Parent (0) {}

  /**
  * Construct the table with a hash of the given initial size,
  * which should be a prime number, for optimisation reasons.
  * See the csHashMap docs for more info.
  * Also copies auth variables from another symbol table.
  */
  csSymbolTable (const csSymbolTable &other, int size = 53);

  /**
  * Destruct the table and delete all variables.
  */
  ~csSymbolTable ();

  /// Add a child table which will inherit the symbols of this one.
  void AddChild (csSymbolTable *);

  /// Add child tables which will inherit the symbols of this one.
  void AddChildren (csArray<csSymbolTable*> &);

  /// Set the value of a symbol, or create a new one if it doesn't exist.
  void SetSymbol (csStringID name, iBase *value);

  /// SetSymbol for multiple symbols.
  void SetSymbols (const csArray<csStringID> &names, csArray<iBase *> &);

  /// Delete a symbol.
  bool DeleteSymbol (csStringID name);

  /// Delete multiple symbols.
  bool DeleteSymbols (const csArray<csStringID> &names);

  /// Get the value of a symbol.
  iBase* GetSymbol (csStringID name);

  /// Get the values of multiple symbols.
  csArray<iBase *> GetSymbols (const csArray<csStringID> &names);

  /// Check if a symbol exists.
  bool SymbolExists (csStringID name);

  /// Check if all of a set of symbols exist.
  bool SymbolsExist (const csArray<csStringID> &names);
};

#endif
