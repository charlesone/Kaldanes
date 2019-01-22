#ifndef HEAD_H_INCLUDED
#define HEAD_H_INCLUDED

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

Head.h - header file for Kaldane Head string, which are variable-length (with an upper
  bound) null-terminated byte strings that have their indexes and a "poor man's normalized key"
  (pmnk) moved when they are swapped, without moving their tail strings (ignoring the Sort Benchmark
  rules). Head has "value-string semantics" as Stroustrup defined for his String type (C++11,
  chapter 19.3) These might be a candidate for the Indy Sort. Head strings behave like Direct
  strings for short lengths (16 bytes or less, by internally equating the pmnk length with the
  string length.) This allows Head strings to match Direct strings in performance for short
  strings and still have constant time complexity for any string length: they are a candidate
  for general purpose string programming with stack slab allocation/deallocation as opposed to
  the fine-grained allocators, which are necessary for pointer strings like std::string. Head
  strings isolate the head (index + pmnk) from the tail, which deploys a Direct string with a
  typedef called "tail". At scale, Head strings sort faster than Direct or Symbiont strings
  and much, much faster than std::strings, which require much more space as well (probably due
  to fine-grainied allocation.) The differences are smaller for short strings but become more
  pronounced for longer strings and  especially for merge sorts, where std:string is quadratic
  in the release build. Both merge and quick sorts are effectively linear for Head strings and
  constant for string length. Head strings, like Symbiont strings are quadratic in the debug
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
#include <cstring>
#include "Direct.h"

using namespace std;

static char *tailAnchor = 0;
static int tailElementSize = -1;
static int tailStructSize = -1;

template <std::size_t maxStringSize, std::size_t maxPmnkSize = 7, std::size_t switchoverPmnkSize = 16>
class Head
{
protected:

    static const std::size_t headPlusTailLen = maxStringSize;
    // if maxStringSize <= switchoverPmnkSize, the pmnk will contain the whole string
    static const std::size_t optimalMaxPmnkSize = (headPlusTailLen <= switchoverPmnkSize
                                                    && maxPmnkSize <= switchoverPmnkSize)
                                                    ? headPlusTailLen : maxPmnkSize;
    // The mechanism is driven from the maxPmnkSize choice, see the quote below. If prefix truncation cannot
    // be done reasonably, then increase this to minimize the occurrence of going to the tail string.
    // Try to make the PMNK string + null fit in an integer (4-byte) package, struct round-off by the compiler
    // will take that space anyway: Optimal sizes for maxPmnkSize are 3,7,11,15,19 ..., the default is 7,
    // which is a little slower than 3 at the small scale (when it doesn't matter), but much faster than
    // three at enormous scale, due to the greater number of identical pmnk at that scale. The
    // switchoverPmnkSize parameter overrides that size at the small scale and seems to optimize that nicely.
    static const std::size_t pmnkSize = (headPlusTailLen <= optimalMaxPmnkSize)
                                        ? headPlusTailLen : optimalMaxPmnkSize; // if small, pmnk == str

    static const std::size_t tailSize = (headPlusTailLen <= optimalMaxPmnkSize)
                                        ? 0 : headPlusTailLen - (optimalMaxPmnkSize);

    struct HeadStruct // Movable, references the torso with an index offset from the tailAnchor
    {
        int k;
        char pmnk[pmnkSize + 1]; // Poor Man's Normalized Key, see below. Null-terminated.
    };

    // Notice, no default assignments above, which yields an empty default constructor, which
    // means release builds (dropping empty functions) will not have quadratic behavior for
    // cross-container element movement (IMHO)

    /* B-tree Indexes and CPU Caches Goetz Graefe and Per-Ake Larson
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

    HeadStruct h;

public:

    typedef Direct<char, tailSize> tail;

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

    class Bad_Array_Anchor_Or_K_Init : public runtime_error
    {
    public:
        Bad_Array_Anchor_Or_K_Init() :
            runtime_error("after allocation, you MUST call array[0].dropAnchorKInit(headArray, tailArray, correct_arrayCount)") {}
    };

    class Already_Array_Anchor : public runtime_error
    {
    public:
        Already_Array_Anchor() :
            runtime_error("you must ONLY ONCE call array[0].dropAnchorKInit(headArray, tailArray, correct_arrayCount)") {}
    };

    class Should_Not_Execute_Here : public runtime_error
    {
    public:
        Should_Not_Execute_Here() :
            runtime_error("program should not execute here (performance debugging)") {}
    };

    Head()
    {
    }

    Head(const Head& rhs)
    {
        h = rhs.h; // heads only move
    }

    Head& operator = (const Head& rhs)
    {
        h = rhs.h; // heads only move
        return *this;
    }

    Head& assign(const char* str)
    {
        std::size_t len = strlen(str);
        if (len > headPlusTailLen) throw Assign_String_Too_Long();
        h.pmnk[0] = 0;
        // Now we get our (Head) array index and index into the Tail array
        char *tailStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
        tailStr[0] = 0;
        if (len > pmnkSize)
        {
            memcpy(h.pmnk, str, pmnkSize);
            h.pmnk[pmnkSize] = 0;
            strcpy(tailStr, str+pmnkSize);
        }
        else
        {
            strcpy(h.pmnk, str);
        }
        return *this;
    }

    size_t size() const noexcept
    {
        return headPlusTailLen;
    }

    size_t structSize() const noexcept
    {
        return sizeof(h);
    }

    bool operator < (const Head& rhs)
    {
        if (h.k == rhs.h.k) return false; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare < 0) return true; // independent of the tail
        else if (pmnkCompare > 0 || (pmnkCompare == 0 && tailSize == 0)) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || h.k < 0 || rhs.h.k < 0)
                throw Bad_Array_Anchor_Or_K_Init();
            // throw Should_Not_Execute_Here(); // performance debugging
            char *lhsStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
            char *rhsStr = tailAnchor + (rhs.h.k * tailStructSize); // = tailArray[rhs.h.k].r.arr : Direct co-array string
            bool state = (strcmp(lhsStr, rhsStr) < 0);
            return state;
        }
    }

    bool operator <= (const Head& rhs)
    {
        if (h.k == rhs.h.k) return true; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare < 0 || (pmnkCompare == 0 && tailSize == 0)) return true; // independent of the tail
        else if (pmnkCompare > 0) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || h.k < 0 || rhs.h.k < 0)
                throw Bad_Array_Anchor_Or_K_Init();
            // throw Should_Not_Execute_Here(); // performance debugging
            char *lhsStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
            char *rhsStr = tailAnchor + (rhs.h.k * tailStructSize); // = tailArray[rhs.h.k].r.arr : Direct co-array string
            bool state = (strcmp(lhsStr, rhsStr) < 1);
            return state;
        }
    }

    bool operator == (const Head& rhs)
    {
        if (h.k == rhs.h.k) return true; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare == 0 && tailSize == 0) return true; // independent of the tail
        else if (pmnkCompare != 0) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || h.k < 0 || rhs.h.k < 0)
                throw Bad_Array_Anchor_Or_K_Init();
            // throw Should_Not_Execute_Here(); // performance debugging
            char *lhsStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
            char *rhsStr = tailAnchor + (rhs.h.k * tailStructSize); // = tailArray[rhs.h.k].r.arr : Direct co-array string
            bool state = (strcmp(lhsStr, rhsStr) == 0);
            return state;
        }
    }

    bool operator != (const Head& rhs)
    {
        if (h.k == rhs.h.k) return false; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare == 0 && tailSize == 0) return false; // independent of the tail
        else if (pmnkCompare != 0) return true; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || h.k < 0 || rhs.h.k < 0)
                throw Bad_Array_Anchor_Or_K_Init();
            // throw Should_Not_Execute_Here(); // performance debugging
            char *lhsStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
            char *rhsStr = tailAnchor + (rhs.h.k * tailStructSize); // = tailArray[rhs.h.k].r.arr : Direct co-array string
            bool state = (strcmp(lhsStr, rhsStr) != 0);
            return state;
        }
    }

    bool operator >= (const Head& rhs)
    {
        if (h.k == rhs.h.k) return true; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare > 0 || (pmnkCompare == 0 && tailSize == 0)) return true; // independent of the tail
        else if (pmnkCompare < 0 ) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || h.k < 0 || rhs.h.k < 0)
                throw Bad_Array_Anchor_Or_K_Init();
            // throw Should_Not_Execute_Here(); // performance debugging
            char *lhsStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
            char *rhsStr = tailAnchor + (rhs.h.k * tailStructSize); // = tailArray[rhs.h.k].r.arr : Direct co-array string
            bool state = (strcmp(lhsStr, rhsStr) > -1);
            return state;
        }
    }

    bool operator > (const Head& rhs)
    {
        if (h.k == rhs.h.k) return false; // pivot, temp or external array identity optimization
        int pmnkCompare = strcmp(h.pmnk, rhs.h.pmnk);
        if (pmnkCompare > 0) return true; // independent of the tail
        else if (pmnkCompare < 0 || (pmnkCompare == 0 && tailSize == 0)) return false; // don't check tail
        else // the pmnk are identical, need to look at tails
        {
            if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || h.k < 0 || rhs.h.k < 0)
                throw Bad_Array_Anchor_Or_K_Init();
            // throw Should_Not_Execute_Here(); // performance debugging
            char *lhsStr = tailAnchor + (h.k * tailStructSize); // = tailArray[h.k].r.arr : Direct co-array string
            char *rhsStr = tailAnchor + (rhs.h.k * tailStructSize); // = tailArray[rhs.h.k].r.arr : Direct co-array string
            bool state = (strcmp(lhsStr, rhsStr) > 0);
            return state;
        }
    }

    void dropAnchorKInit(Head headArr[], tail tailArr[], std::size_t size)
    // parameters should look like (headArray, tailArray, array count)
    {
        // if (tailAnchor != 0) throw Already_Array_Anchor();
        if (sizeof(headArr[0]) != sizeof(h)) throw Item_Size_Mismatch();
        tailAnchor = (char*)tailArr;
        tailElementSize = tailArr[0].size();
        tailStructSize = tailArr[0].structSize();
        for (std::size_t i = 0; i < size; ++i) headArr[i].h.k = i;
    }

    friend ostream& operator<< (ostream &os, const Head& rhs)
    {
        if (tailAnchor == 0 || tailElementSize == -1 || tailStructSize == -1 || rhs.h.k < 0)
            throw Bad_Array_Anchor_Or_K_Init();
        os << rhs.h.pmnk;
        char *tailStr = tailAnchor + (rhs.h.k * tailStructSize);
        os << tailStr;
        return os;
    }

    void reserve(int i) { } // no-op


};

#endif // HEAD_H_INCLUDED


