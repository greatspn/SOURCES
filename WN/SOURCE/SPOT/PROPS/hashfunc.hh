// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef SPOT_MISC_HASHFUNC_HH
# define SPOT_MISC_HASHFUNC_HH
# include <cstddef>

namespace spot
{
  /// \addtogroup hash_funcs Hashing functions
  /// \ingroup misc_tools
  /// @{

  /// \brief Thomas Wang's 32 bit hash function.
  ///
  /// Hash an integer amongst the integers.
  /// http://www.concentric.net/~Ttwang/tech/inthash.htm
  inline size_t
  wang32_hash(size_t key)
  {
    // We assume that size_t has at least 32bits.
    key += ~(key << 15);
    key ^=  (key >> 10);
    key +=  (key << 3);
    key ^=  (key >> 6);
    key += ~(key << 11);
    key ^=  (key >> 16);
    return key;
  }
  /// @}
}

#endif // SPOT_MISC_HASHFUNC_HH
