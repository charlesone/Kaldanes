#ifndef JOINEDROW_H
#define JOINEDROW_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"

using namespace std;

// will be variadic as soon as I get the type handling worked out
template <typename T1, typename T2, typename T3, typename T4, std::size_t arrayLen = 4>
class JoinedRow
{
public:
    static const std::size_t arrayCount = arrayLen;

    struct JoinedRowStruct
    {
        int k[arrayCount];
        bool valid;
    };

    JoinedRowStruct j;

    class Bad_RowString_Anchor : public runtime_error
    {
    public:
        Bad_RowString_Anchor() :
            runtime_error("after allocation, you MUST call rowArray[0].dropAnchor(rowArray, correct_arrayCount)") {}
    };

    const std::size_t tableCount()
    {
        return arrayCount;
    }

    const std::size_t structSize()
    {
        return sizeof(j);
    }

    // prints the joined rows
    friend ostream& operator<< (ostream &os, const JoinedRow& rhs)
    {
        // This part could be automated with some ifdef generic programming

        for (int i = 0; i < (int)Table::table_Count; i++)
        {// this means all tables must be allocated and anchored before the first join is accessed this way
            if (rowAnchors[i] == 0)
                throw Bad_RowString_Anchor(); // no index for this table
        }
    T1* ptr1 = (T1*)rowAnchors[(std::size_t)Table::airports];
    T2* ptr2 = (T2*)rowAnchors[(std::size_t)Table::routes];
    T3* ptr3 = (T3*)rowAnchors[(std::size_t)Table::airports];
    T4* ptr4 = (T4*)rowAnchors[(std::size_t)Table::airlines];
        os << ptr1[rhs.j.k[0]];
        os << ptr2[rhs.j.k[1]];
        os << ptr3[rhs.j.k[2]];
        os << ptr4[rhs.j.k[3]];
        return os;
    }

};

#endif // JOINEDROW_H
