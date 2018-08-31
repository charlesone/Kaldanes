#ifndef ROWSTRING_H_INCLUDED
#define ROWSTRING_H_INCLUDED

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

RowString.h - header file for RowString type, which are variable-length (with an upper bound)
  null-terminated byte strings that have their bytes moved as a single body when they are swapped
  as in sorting, not just moving their pointers (following the Sort Benchmark rules.) These might
  be a candidate for the Indy Sort. RowString has "value-string semantics" as Stroustrup defined for
  his String type (C++11, chapter 19.3) RowString are the fastest for short strings: linear
  for quick sort AND merge sort (as opposed to the type <string> which is slower for quick sort
  and quadratic for merge sort.) However, even for longer strings RowString are sub-linear in time
  complexity for string length due to cache pre-fetch (that means when you double the length of
  the strings being sorted, the time taken is quite a bit less than double.) They are designed
  to be used  with slab allocation/deallocation on the stack, as opposed to fine-grained allocators.
  Since the allocated slab never needs to contain pointers, the slab data structure is base+offset
  and can be mmapped to a file or /dev/shm and shared locally or across  a memory fabric like Gen-Z,
  or stored and transmitted, or mmapped over an NFS: consistency considerations are an issue for
  sharing, of course (caveat participem).
*/

#include <iostream>
#include <stddef.h>
#include <algorithm>
#include <string.h>

typedef char* ColumnStr;

using namespace std;

// My intent is for this to work for more than just typename char someday: wchar_t, char16_t, char32_t
template <typename T, std::size_t maxStringSize, Table tableEnum, std::size_t maxColumnsCount = 4,
         std::size_t maxColumnSize = 1024>
class RowString
{
public:

    static const std::size_t rowLength = maxStringSize;

    struct Body
    {
        T arr[rowLength + 1]; // null termination
    };

    struct Struct
    {
        uint_least16_t columnsCountOffset;
        Body b;
        uint_least16_t columnsCount;
        uint_least16_t columnOffset[maxColumnsCount];
    };

    // Notice, no default assignments above, which yields an empty default constructor, which
    // means release builds (dropping empty functions) will not have quadratic behavior for
    // cross-container element movement (IMHO)

public:
    Struct r;

    typedef RowString<T, maxStringSize, tableEnum, maxColumnsCount> rowType;

    class Assign_String_Too_Long : public runtime_error
    {
    public:
        Assign_String_Too_Long() :
            runtime_error("assign function called with overlong string") {}
    };

    class Item_Size_Mismatch : public runtime_error
    {
    public:
        Item_Size_Mismatch() :
            runtime_error("array[0].structSize() must equal array items/arraySize (possible compiler dependency)") {}
    };

    class Bad_RowString_Anchor : public runtime_error
    {
    public:
        Bad_RowString_Anchor() :
            runtime_error("after allocation, you MUST call rowArray[0].dropAnchor(rowArray, correct_arrayCount)") {}
    };

    class Already_Array_Anchor : public runtime_error
    {
    public:
        Already_Array_Anchor() :
            runtime_error("you must ONLY ONCE call rowArray[0].dropAnchor(rowArray, correct_arrayCount)") {}
    };

    class Input_Stream_Ended : public runtime_error
    {
    public:
        Input_Stream_Ended() :
            runtime_error("input stream ended") {}
    };

    class Input_Stream_Bad : public runtime_error
    {
    public:
        Input_Stream_Bad() :
            runtime_error("input stream bad") {}
    };

    class Input_Stream_Failed : public runtime_error
    {
    public:
        Input_Stream_Failed() :
            runtime_error("input stream failed") {}
    };

    class Wrong_Column_For_This_Table : public runtime_error
    {
    public:
        Wrong_Column_For_This_Table() :
            runtime_error("the column enum is  for a different table") {}
    };

    class Bad_IndexString_Field_Null_Termination : public runtime_error
    {
    public:
        Bad_IndexString_Field_Null_Termination() :
            runtime_error("index fields MUST BE null terminated within the row array") {}
    };

    std::size_t size() const noexcept
    {
        return rowLength;
    }

    std::size_t structSize() const noexcept
    {
        return sizeof(r);
    }

    void checkUnitLength(std::size_t size) // parameter size should be (array length/array count)
    {
        if (sizeof(r) != size) throw Item_Size_Mismatch();
    }

    std::size_t count()
    {
        return rowCounts[(int)tableEnum];
    }

    // for a simple one column rowString
    RowString& assign (const char* str)
    {
        r.columnsCountOffset = offsetof(struct Struct,r.columnsCount);
        r.columnsCount = 1; // Only one column.
        r.columnOffset[0] = {offsetof(struct Struct,r.b)}; // only one column offset.

        int len = strlen(str);
        if (len > rowLength) throw Assign_String_Too_Long();
        strcpy(r.b.arr, str);
        r.b.arr[len] = 0;
        return *this;
    }

    // for a DIY rowString with preset nulls, counters and offsets
    RowString& assign (const char* str, std::size_t size)
    {
        if (size > sizeof(r)) throw Assign_String_Too_Long();
        strncpy(r, str, size);
        // pads out the RowString with nulls: short rows will have zero length columns at the end.
        return *this;
    }

    // You delimit the  columns (optional non-null delimiters) of a null terminated string, we fix it up
    RowString& assignColumns (const char* str, const char delimiter = ',')
    {
        strncpy(r.b.arr, str, rowLength);
        r.columnsCountOffset = offsetof(struct Struct,columnsCount);
        r.columnsCount = 1; // Start off with one column
        r.columnOffset[0] = {offsetof(struct Struct,b)};
        // Nulls other than those from strncpy end the rowString, WE DO THE NULLING!
        for (int i = 0; r.b.arr[i] != 0 && i < rowLength && r.columnsCount < maxColumnsCount; i++)
        {
            if (r.b.arr[i] == delimiter)
            {
                r.b.arr[i] = 0; // overwrite the delimiter, null terminating the column field
                // create the offset for the next column beyond the null we just wrote
                r.columnOffset[r.columnsCount++] = offsetof(struct Struct,b) + i + 1;
            }
        }
        return *this;
    }

    ColumnStr columnStr(Column columnEnum) // returns a pointer into the column inside the row
    {
        if (charRowAnchors[(int)tableEnum] == 0) throw Bad_RowString_Anchor(); // no table to index
        if (column2Table(columnEnum) != tableEnum) throw Wrong_Column_For_This_Table();
        char *lhsColumnStr = (char*)r + r.columnOffset[columnId[(int)columnEnum]];

        int len = strlen(lhsColumnStr);
        if (len > maxColumnSize) throw Bad_IndexString_Field_Null_Termination();
        return lhsColumnStr;
    }

    rowType row()
    {
        return *this;
    }

    // All rowString comparisons default to the first null-terminated column
    bool operator < (const RowString& rhs)
    {
        return (strcmp(r.b.arr, rhs.r.b.arr) < 0);
    }

    bool operator <= (const RowString& rhs)
    {
        return (strcmp(r.b.arr, rhs.r.b.arr) < 1);
    }

    bool operator == (const RowString& rhs)
    {
        return (strcmp(r.b.arr, rhs.r.b.arr) == 0);
    }

    bool operator != (const RowString& rhs)
    {
        return (strcmp(r.b.arr, rhs.r.b.arr) != 0);
    }

    bool operator >= (const RowString& rhs)
    {
        return (strcmp(r.b.arr, rhs.r.b.arr) > -1);
    }

    bool operator > (const RowString& rhs)
    {
        return (strcmp(r.b.arr, rhs.r.b.arr) > 0);
    }

    friend istream& operator>> (istream &is, RowString& rhs)
    {
        char input[rowLength + 300]; // comment lines possibly longer than data
        is.getline(input, rowLength + 300);
        rhs.assignColumns(input);
        return is;
    }

    friend ostream& operator<< (ostream &os, const RowString& rhs)
    {
        for (int i = 0; i < rhs.r.columnsCount; i++)
        {
            // C++ doesn't want me to: strPtr = (char*) rhs.r + rhs.r.columnOffset[i];
            const char* strPtr = rhs.r.b.arr - offsetof(Struct, b.arr) + rhs.r.columnOffset[i];
            os << strPtr;
            if (i < rhs.r.columnsCount - 1) os << ",";
        }
        return os;
    }

    void dropAnchor(RowString rowArr[], std::size_t size)
    // parameters should look like (rowArray, array count)
    {
        if (charRowAnchors[(int)tableEnum] != 0) throw Already_Array_Anchor();
        if (rowAnchors[(int)tableEnum] != 0) throw Already_Array_Anchor();
        if (sizeof(rowArr[0]) != sizeof(r)) throw Item_Size_Mismatch();
        charRowAnchors[(int)tableEnum] = (char*)rowArr;
        rowAnchors[(int)tableEnum] = &rowArr[0];
        rowCounts[(int)tableEnum] = size;
    }

    void reserve(int i) { } // no-op
};

#endif // ROWSTRING_H_INCLUDED
