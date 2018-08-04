#ifndef JOINEDROW_H
#define JOINEDROW_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"

using namespace std;

template <typename... T>
class JoinedRow
{
public:
    // the initial relation has two table indexes, the rest are links to one table index
    static const std::size_t arrayCount = sizeof...(T) + 1;

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
        return os;
    }

};

#endif // JOINEDROW_H
