#ifndef INDEXSTRING_H_INCLUDED
#define INDEXSTRING_H_INCLUDED

/*
    [Valverde Computing copyright notice]

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    As an exception, the copyright holders of this Library grant you permission to
    (i) compile an Application with the Library, and
    (ii) distribute the Application containing code generated by the Library and
    added to the Application during this compilation process under terms of your choice,
    provided you also meet the terms and conditions of the Application license.

IndexString.h - header file for Kaldane IndexString string, which are variable-length (with an upper
  bound) null-terminated byte strings that have their indexes and a "poor man's normalized key"
  (pmnk) moved when they are swapped, without moving their tail strings (ignoring the Sort Benchmark
  rules). IndexString has "value-string semantics" as Stroustrup defined for his String type (C++11,
  chapter 19.3) These might be a candidate for the Indy Sort. IndexString strings behave like Direct
  strings for short lengths (16 bytes or less, by internally equating the pmnk length with the
  string length.) This allows IndexString strings to match Direct strings in performance for short
  strings and still have constant time complexity for any string length: they are a candidate
  for general purpose string programming with stack slab allocation/deallocation as opposed to
  the fine-grained allocators, which are necessary for pointer strings like std::string. IndexString
  strings isolate the head (index + pmnk) from the tail, which deploys a Direct string with a
  typedef called "tail". At scale, IndexString strings sort faster than Direct or Symbiont strings
  and much, much faster than std::strings, which require much more space as well (probably due
  to fine-grainied allocation.) The differences are smaller for short strings but become more
  pronounced for longer strings and  especially for merge sorts, where std:string is quadratic
  in the release build. Both merge and quick sorts are effectively linear for IndexString strings and
  constant for string length. IndexString strings, like Symbiont strings are quadratic in the debug
  build for  merge sort, so remember to use the release build for performance analysis.
  They are designed to be used  with with slab allocation/deallocation on the stack, as opposed
  to fine-grained allocators. Since the two allocated slabs never need to contain pointers, only
  array indices, the slab data structures are base+offset and can be mmapped to a file or /dev/shm
  and shared locally or across a memory fabric such as Gen-Z, or stored and transmitted, or mmapped
  over an NFS: consistency considerations are an issue for sharing, of course (caveat participem).

  The name Kaldane is an antique SciFi reference (heads move, bodies stay):
  http://www.catspawdynamics.com/images/gino_d%27achille_5-the_chessmen_of_mars.jpg
  or google image search the word "kaldane"
  https://en.wikipedia.org/wiki/Kaldane

*/

#include <iostream>
#include <typeinfo>
#include "RowString.h"
#include "Sorts.h"

using namespace std;

template <Column columnEnum,
         typename T, std::size_t maxStringSize, Table tableEnum, std::size_t maxColumnsCount,
         // this next default must match the RowString default, declared there
         std::size_t maxColumnSize = 1024,
         std::size_t pmnkSize = 7>
class IndexString
{
    friend class RowString<T, maxStringSize, tableEnum, maxColumnsCount, maxColumnSize>;

public:

    //static_assert(index0StringSize > 0, "index0StringSize must be positive"); // can't get this to work
    //static_assert(maxPmnkSize <= index0StringSize, "maxPmnkSize must be <= index0StringSize"); // can't get this to work

    // Try to make the PMNK string + null fit in an integer (4-byte) package, struct round-off by the compiler
    // will take that space anyway: Optimal sizes for maxPmnkSize are 3,7,11,15,19 ..., the default is 7,
    // which is a little slower than 3 at the small scale (when it doesn't matter), but much faster than
    // three at enormous scale, due to the greater number of identical pmnk at that scale.
    struct IndexStringStruct
            // Movable, references each RowString by the column index with a striding offset from the RowString columnOffset array.
    {
        int k;
        char pmnk[pmnkSize + 1]; // Poor Man's Normalized Key, see below. Null-terminated.
    };

    /* B-tree IndexStringes and CPU Caches Goetz Graefe and Per-Ake Larson
        IEEE Proceedings of the 17th International Conference on Data Engineering
        Section 6.1, "Poor Man's Normalized Keys", p. 355:

        This technique works well only if common
        key prefixes are truncated before the poor man's
        normalized keys are extracted. In that case, a small poor
        man's normalized key (e.g., 2 bytes) may be sufficient to
        avoid most cache faults on the full records; if prefix truncation
        is not used, even a fairly large poor man's normalized
        key may not be very effective, in particular in B-tree
        leaves.
    */

    IndexStringStruct h;

public:

    typedef RowString<T, maxStringSize, tableEnum, maxColumnsCount, maxColumnSize> rowType;
    static const int rowAnchorOffset = (int)tableEnum;
    static const int indexAnchorOffset = (int)columnEnum;

    class Item_Size_Mismatch : public runtime_error
    {
    public:
        Item_Size_Mismatch() :
            runtime_error("array[0].structSize() must equal arrayByteLength/arrayCount (possible compiler dependency)") {}
    };

    class Assign_String_Too_Long : public runtime_error
    {
    public:
        Assign_String_Too_Long() :
            runtime_error("assign function called with overlong string") {}
    };

    class Bad_RowString_Anchor : public runtime_error
    {
    public:
        Bad_RowString_Anchor() :
            runtime_error("after allocation, you MUST call rowArray[0].dropAnchor(rowArray, correct_arrayCount)") {}
    };

    class Bad_IndexString_Anchor : public runtime_error
    {
    public:
        Bad_IndexString_Anchor() :
            runtime_error("no index, or corrupted index for this column of  this table") {}
    };

    class Already_IndexString_Anchor : public runtime_error
    {
    public:
        Already_IndexString_Anchor() :
            runtime_error("this index anchor has already been initialized once") {}
    };

    class Over_Writing_Existing_IndexString_Key : public runtime_error
    {
    public:
        Over_Writing_Existing_IndexString_Key() :
            runtime_error("index keys copied from RowString only once during IndexString creation") {}
    };

    class Bad_IndexString_Field_Null_Termination : public runtime_error
    {
    public:
        Bad_IndexString_Field_Null_Termination() :
            runtime_error("index fields MUST BE null terminated within the row array") {}
    };

    class Should_Not_Execute_Here : public runtime_error
    {
    public:
        Should_Not_Execute_Here() :
            runtime_error("program should not execute here (performance debugging)") {}
    };

    IndexString()
    {
    }

    IndexString(const IndexString& rhs)
    {
        h = rhs.h; // heads only move
    }

    IndexString& operator = (const IndexString& rhs)
    {
        h = rhs.h; // heads only move
        return *this;
    }

    IndexString& copyKey() // Only used during IndexString init: copy the pmnk of the IndexString key from the RowString
    {
        if (charIndexAnchors[indexAnchorOffset] == 0) throw Bad_IndexString_Anchor(); // no index for this column
        if (h.k != 0) throw Over_Writing_Existing_IndexString_Key(); // some confusion as to order of operations
        h.pmnk[0] = 0;
        // First, get the k-value, the corresponding rowString-array-index
        char *pmnkPtr = h.pmnk;
        char *hPtr = pmnkPtr - offsetof(IndexStringStruct, pmnk);
        int delta = hPtr - charIndexAnchors[indexAnchorOffset];
        int k = delta/sizeof(h);
//        int k = ((char*)h.pmnk - offsetof(IndexStringStruct, pmnk) - charAnchors[indexAnchorOffset])/sizeof(h);
        // Second, get the rowString-column string pointer (source)
        char *rowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * k); // points to first byte of corresponding row
        rowType *row = (rowType*)(rowStr); // row pointer of corresponding row
        char *columnStr = rowStr + row->r.columnOffset[columnId[indexAnchorOffset]];
        // Third, get the rowString's column-string length
        int len = strlen(columnStr);
        // Create the IndexString element
        if (len > pmnkSize) len = pmnkSize;
        strncpy(h.pmnk, columnStr, len);
        h.pmnk[len] = 0;
        h.k = k;
        return *this;
    }

    std::size_t structSize() const noexcept
    {
        return sizeof(h);
    }

    const int indexAnchorOff()
    {
        return indexAnchorOffset;
    }

    const Table enumTable()
    {
        return tableEnum;
    }

    std::size_t count()
    {
        return indexCounts[indexAnchorOffset];
    }

    rowType row()
    {
        if (charRowAnchors[rowAnchorOffset] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (charIndexAnchors[indexAnchorOffset] == 0 || h.k < 0)
            throw Bad_IndexString_Anchor(); // no index for this column

        char *rowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * h.k); // points to first byte of rhs row
        rowType *row = (rowType*)(rowStr); // rhs row pointer

        return row[0];
    }

    ColumnStr c_str() // returns a more costly pointer into the column inside the row at the IndexStringStruct h.k offset
    {
        if (charRowAnchors[rowAnchorOffset] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (charIndexAnchors[indexAnchorOffset] == 0 || h.k < 0)
            throw Bad_IndexString_Anchor(); // no index for this column

        char *lhsRowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * h.k); // points to first byte of lhs row
        rowType *lhsRow = (rowType*)(lhsRowStr); // lhs row pointer
        char *lhsColumnStr = lhsRowStr + lhsRow->r.columnOffset[columnId[indexAnchorOffset]];

        int len = strlen(lhsColumnStr);
        if (len > maxColumnSize) throw Bad_IndexString_Field_Null_Termination();
        return lhsColumnStr;
    }

private:

    inline int compareTail(const IndexString& rhs)
    {
        if (charRowAnchors[rowAnchorOffset] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (charIndexAnchors[indexAnchorOffset] == 0 || h.k < 0 || rhs.h.k < 0)
            throw Bad_IndexString_Anchor(); // no index for this column
        // throw Should_Not_Execute_Here(); // this is for performance debugging

        char *lhsRowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * h.k); // points to first byte of lhs row
        rowType *lhsRow = (rowType*)(lhsRowStr); // lhs row pointer
        char *lhsColumnStr = lhsRowStr + lhsRow->r.columnOffset[columnId[indexAnchorOffset]] + pmnkSize; // points after pmnkSize

        char *rhsRowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * rhs.h.k); // points to first byte of rhs row
        rowType *rhsRow = (rowType*)(rhsRowStr); // rhs row pointer
        char *rhsColumnStr = rhsRowStr + rhsRow->r.columnOffset[columnId[indexAnchorOffset]] + pmnkSize; // points after pmnkSize

        return strcmp(lhsColumnStr, rhsColumnStr);
    }

    inline int compareTail(const char rhsTailStr[])
    {
        if (charRowAnchors[rowAnchorOffset] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (charIndexAnchors[indexAnchorOffset] == 0 || h.k < 0)
            throw Bad_IndexString_Anchor(); // no index for this column
        // throw Should_Not_Execute_Here(); // this is for performance debugging

        char *lhsRowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * h.k); // points to first byte of lhs row
        rowType *lhsRow = (rowType*)(lhsRowStr); // lhs row pointer
        char *lhsColumnStr = lhsRowStr + lhsRow->r.columnOffset[columnId[indexAnchorOffset]] + pmnkSize; // points after pmnkSize

        return strcmp(lhsColumnStr, rhsTailStr);
    }

public:

// IndexString to IndexString comparisons

    bool operator < (const IndexString& rhs)
    {
        if (h.k == rhs.h.k) return false; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare < 0) return true; // independent of the tail
        else if (pmnkCompare > 0 || (pmnkCompare == 0 && strlen(rhs.h.pmnk) < pmnkSize))
            return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs) < 0);
            return state;
        }
    }

    bool operator <= (const IndexString& rhs)
    {
        if (h.k == rhs.h.k) return true; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare < 0 || (pmnkCompare == 0 && strlen(rhs.h.pmnk) < pmnkSize))
            return true; // independent of the tail
        else if (pmnkCompare > 0) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs) < 1);
            return state;
        }
    }

    bool operator == (const IndexString& rhs)
    {
        if (h.k == rhs.h.k) return true; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare == 0 && strlen(rhs.h.pmnk) < pmnkSize)
            return true; // independent of the tail
        else if (pmnkCompare != 0) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs) == 0);
            return state;
        }
    }

    bool operator != (const IndexString& rhs)
    {
        if (h.k == rhs.h.k) return false; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare == 0 && strlen(rhs.h.pmnk) < pmnkSize)
            return false; // independent of the tail
        else if (pmnkCompare != 0) return true; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs) != 0);
            return state;
        }
    }

    bool operator >= (const IndexString& rhs)
    {
        if (h.k == rhs.h.k) return true; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare > 0 || (pmnkCompare == 0 && strlen(rhs.h.pmnk) < pmnkSize))
            return true; // independent of the tail
        else if (pmnkCompare < 0 ) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs) > -1);
            return state;
        }
    }

    bool operator > (const IndexString& rhs)
    {
        if (h.k == rhs.h.k) return false; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare > 0) return true; // independent of the tail
        else if (pmnkCompare < 0 || (pmnkCompare == 0 && strlen(rhs.h.pmnk) < pmnkSize))
            return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs) > 0);
            return state;
        }
    }

// char[] comparisons

    bool operator < (const char rhs[]) // null-terminated string compare
    {
        int pmnkCompare = strncmp(h.pmnk, rhs, pmnkSize);
        if (pmnkCompare < 0) return true; // independent of the tail
        else if (pmnkCompare > 0 || (pmnkCompare == 0 && strlen(rhs) < pmnkSize))
            return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs + pmnkSize) < 0);
            return state;
        }
    }

    bool operator <= (const char rhs[]) // null-terminated string compare
    {
        int pmnkCompare = strncmp(h.pmnk, rhs, pmnkSize);
        if (pmnkCompare < 0 || (pmnkCompare == 0 && strlen(rhs) < pmnkSize))
            return true; // independent of the tail
        else if (pmnkCompare > 0) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs + pmnkSize) < 1);
            return state;
        }
    }

    bool operator == (const char rhs[]) // null-terminated string compare
    {
        int pmnkCompare = strncmp(h.pmnk, rhs, pmnkSize);
        if (pmnkCompare == 0 && strlen(rhs) < pmnkSize)
            return true; // independent of the tail
        else if (pmnkCompare != 0) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs + pmnkSize) == 0);
            return state;
        }
    }

    bool operator != (const char rhs[]) // null-terminated string compare
    {
        int pmnkCompare = strncmp(h.pmnk, rhs, pmnkSize);
        if (pmnkCompare == 0 && strlen(rhs) < pmnkSize)
            return false; // independent of the tail
        else if (pmnkCompare != 0) return true; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs + pmnkSize) != 0);
            return state;
        }
    }

    bool operator >= (const char rhs[]) // null-terminated string compare
    {
        int pmnkCompare = strncmp(h.pmnk, rhs, pmnkSize);
        if (pmnkCompare > 0 || (pmnkCompare == 0 && strlen(rhs) < pmnkSize))
            return true; // independent of the tail
        else if (pmnkCompare < 0 ) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs + pmnkSize) > -1);
            return state;
        }
    }

    bool operator > (const char rhs[]) // null-terminated string compare
    {
        int pmnkCompare = strncmp(h.pmnk, rhs, pmnkSize);
        if (pmnkCompare > 0) return true; // independent of the tail
        else if (pmnkCompare < 0 || (pmnkCompare == 0 && strlen(rhs) < pmnkSize))
            return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            bool state = (compareTail(rhs + pmnkSize) > 0);
            return state;
        }
    }

    void dropAnchorCopyKeysSortIndex(IndexString indexArr[], std::size_t size)
    // parameters should look like (indexArray, array count)
    {
        if (charRowAnchors[rowAnchorOffset] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (sizeof(indexArr[0]) != sizeof(h)) throw Item_Size_Mismatch();
        if (charIndexAnchors[indexAnchorOffset] != 0) throw Already_IndexString_Anchor(); // came here twice

        charIndexAnchors[indexAnchorOffset] = (char*)indexArr[0].h.pmnk - offsetof(IndexStringStruct, pmnk);
        indexAnchors[indexAnchorOffset] = &indexArr[0];
        indexCounts[indexAnchorOffset] = size;

        for (int i = 0; i < size; ++i)
        {
            indexArr[i].copyKey();
        }
        compares = 0;
        swaps = 0;
        quickSortInvoke(indexArr, size);
    }

    // prints the row pointed to by the IndexStringStruct h.k offset
    friend ostream& operator<< (ostream &os, const IndexString& rhs)
    {
        if (charRowAnchors[rowAnchorOffset] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (charIndexAnchors[indexAnchorOffset] == 0 || rhs.h.k < 0)
            throw Bad_IndexString_Anchor(); // no index for this column
        os << rhs.h.pmnk;
        if (strlen(rhs.h.pmnk) == pmnkSize)
        {
            char *rhsRowStr = charRowAnchors[rowAnchorOffset] + (sizeof(((rowType*)0)->r) * rhs.h.k); // points to first byte of rhs row
            rowType *rhsRow = (rowType*)(rhsRowStr); // rhs row pointer
            char *rhsColumnStr = rhsRowStr + rhsRow->r.columnOffset[columnId[indexAnchorOffset]] + pmnkSize; // points after pmnkSize
            os << rhsColumnStr;
        }
        return os;
    }

    void reserve(int i) { } // no-op
};

#endif // INDEXSTRING_H_INCLUDED


