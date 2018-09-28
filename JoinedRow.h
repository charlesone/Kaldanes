#ifndef JOINEDROW_H
#define JOINEDROW_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"
#include "RelationVector.h"

using namespace std;

static std::size_t tableDepth;
static Column joinedRowColumn = Column::nilColumn;
static std::size_t tableOffsets[100];
static Column joinedRowColumns[100];
static Table joinedRowTables[100];

template <typename... relVecs>
class QueryPlan
{
public:
    typedef tuple<relVecs ...> relsTupleType;
    // the first RelationVector in relsTuple has two table indexes (a from-link and a to-link),
    // the rest in RelsTuple are one table indexes (to links) making N + 1 tables in the JoinedRowStruct
    static_assert((sizeof...(relVecs)) == (std::tuple_size<relsTupleType>::value), "Parameter pack size and tuple size must match.");
    static const std::size_t arrayCount = (sizeof...(relVecs)) + 1;

    relsTupleType relsTuple;

    template<typename oneRelVec>
    constexpr int walk(oneRelVec last)
    {
        cout << endl << "tableDepth = " << tableDepth << endl << endl;
        return tableDepth;
    };

    template<typename firstRelVec, typename... remainingRelVecs>
    constexpr int walk(firstRelVec first, remainingRelVecs... rest)
    {
        cout << endl << "tableDepth = " << tableDepth << endl << endl;
        tableDepth++;
        return walk(rest...);
    };

    QueryPlan(relVecs... args)
    {
        for (int i = 0; i < arrayCount; i++)
        {
            tableOffsets[i] = INT_MAX;
            joinedRowColumns[i] = Column::nilColumn;
            joinedRowTables[i] = Table::nilTable;
        }
        relsTuple = std::make_tuple(args...);
        tableDepth = 0;
        walk(args...);
        cout << endl << "tableDepth = " << tableDepth << endl << endl;
    }
};

template <typename... relVecs>
class JoinedRow
{
public:
    typedef QueryPlan<relVecs...> queryPlanType;
    friend class QueryPlan<relVecs...>;

    struct JoinedRowStruct
    {
        int k[queryPlanType::arrayCount] = {0};// These are the k-values indicating a specific row in a specific indexed table (type of template class RowString)
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
        return queryPlanType::arrayCount;
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
        {
            // this means all tables must be allocated and anchored before the first join is accessed this way
            if (rowAnchors[i] == 0)
                throw Bad_RowString_Anchor(); // no index for this table
        }
        return os;
    }

};

#endif // JOINEDROW_H
