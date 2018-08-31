#ifndef JOINEDROW_H
#define JOINEDROW_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"
#include "RelationVector.h"
#include "Tuple.h"

using namespace std;

static std::size_t joinedRowDepth = 0;
static Column joinedRowColumns[25]; // supports 24-way joins
static Table joinedRowTables[25]; // supports 24-way joins

template <typename RelsTuple, typename RowTypesTuple>
class JoinedRow
{
public:
    //static_assert(((((RelsTuple)0)->size()) ==  (((RowTypesTuple)0)->size())), "The size of RelsTuple and RowTypesTuple must match");

    // the first RelationVector in RelsTuple has two table indexes (a from-link and a to-link),
    // the rest in RelsTuple are one table indexes (to links) making N + 1 tables in the JoinedRowStruct

    static const std::size_t arrayCount = ((RelsTuple)0)->size() + 1;
    static std::size_t tableOffsets[arrayCount];// = {(int)(((Rel1)0.r.&fromIndex)->enumTable()),
                                                 //        (int)(((T...)0.r.&toIndex)->enumTable())};

    struct JoinedRowStruct
    {
        int k[arrayCount] = {0};// These are the k-values indicating a specific row in a specific indexed table (type of template class RowString)
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

    void compileQuery() // will be selected when only one entered
    {

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
