#ifndef JOINEDROW_H
#define JOINEDROW_H

#include <iostream>
#include <memory>
#include <functional>
#include <type_traits>
#include "RowString.h"
#include "IndexString.h"
#include "RelationVector.h"

using namespace std;

static std::size_t tableOffsets[100];
static Column joinedRowColumns[100];
static Table joinedRowTables[100];
static bool joinedRowTablesIncluded[100];

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
    int joinedRowIndex;
    int joinedRowRow;

    struct QueryPlanStruct
    {
        int k[arrayCount] = {3530,47126,3657,4539};// These are the testing k-values indicating a specific row in a specific indexed table (type of template class RowString)
    };

    QueryPlanStruct j;

    class Bad_RowString_Anchor : public runtime_error
    {
    public:
        Bad_RowString_Anchor() :
            runtime_error("after allocation, you MUST call rowArray[0].dropAnchor(rowArray, correct_arrayCount)") {}
    };

    class Variadic_Parameter_Pack_Logic_Failed : public runtime_error
    {
    public:
        Variadic_Parameter_Pack_Logic_Failed() :
            runtime_error("somehow, the counting of JoinedRow elements is wrong versus relational vectors.") {}
    };

    class Relation_Vector_Linkage_Rule_Violation : public runtime_error
    {
    public:
        Relation_Vector_Linkage_Rule_Violation() :
            runtime_error("Linkage Rule: after the first one, every successive relation vector's from-index, must be on a table already linked from or to") {}
    };

    constexpr void emptyFunc()
    {
        return;
    };

    template<typename... remainingRelVecs>
    constexpr typename std::enable_if<sizeof...(remainingRelVecs) == 0>::type queryPlanChecker() {} // SFINAE'd away.

    template<typename firstRelVec, typename... remainingRelVecs>
    constexpr void queryPlanChecker()// of course this method only seems to work for void returns with no parameters :(
    {
        const std::size_t relVecDepth = sizeof...(relVecs) - sizeof...(remainingRelVecs) -1;
        //cout << endl << "index = " << joinedRowIndex << ", relVecDepth = " << relVecDepth << endl << endl;
        (joinedRowIndex < relVecDepth) ?
        throw Variadic_Parameter_Pack_Logic_Failed() : emptyFunc(); // seriously out of whack.

        typedef decltype(((firstRelVec*)0)->r.from) fromType;
        typedef decltype(((firstRelVec*)0)->r.to) toType;
        const Column fromColumn = ((fromType)0)->enumColumn();
        const Column toColumn = ((toType)0)->enumColumn();
        const int fromTable = (int)column2Table(fromColumn);
        const int toTable = (int)column2Table(toColumn);

        (joinedRowIndex == 0 && relVecDepth == 0) ? joinedRowTablesIncluded[fromTable] = true
                : (joinedRowIndex == relVecDepth + 1) ? joinedRowTablesIncluded[toTable] = true
                        : 0;

        (!joinedRowTablesIncluded[fromTable]) ? throw Relation_Vector_Linkage_Rule_Violation() : emptyFunc();
        // if neither of those, then call deeper, otherwise just fall through and return
        (!((joinedRowIndex == 0 && relVecDepth == 0) || (joinedRowIndex == relVecDepth + 1))) ?
        queryPlanChecker<remainingRelVecs...>()
        : emptyFunc();

        return;
    };

    void checkQueryPlan()
    {
        for (int i = 0; i < arrayCount; ++i) joinedRowTablesIncluded[i] = false;
        for (int i = 0; i < arrayCount; ++i)
        {
            joinedRowIndex = i;
            queryPlanChecker<relVecs...>();
        }
    }

    template<typename... remainingRelVecs>
    typename std::enable_if<sizeof...(remainingRelVecs) == 0>::type queryPlanTupleOutput() {} // SFINAE'd away.

    template<typename firstRelVec, typename... remainingRelVecs>
    void queryPlanTupleOutput()// of course this method only seems to work for void returns with no parameters :(
    {
        const std::size_t relVecDepth = sizeof...(relVecs) - sizeof...(remainingRelVecs) -1;
        //cout << endl << "index = " << joinedRowIndex << ", row = " << joinedRowRow << ", relVecDepth = " << relVecDepth << endl << endl;
        (joinedRowIndex < relVecDepth) ? throw Variadic_Parameter_Pack_Logic_Failed() : ((void)0); // seriously out of whack.

        (joinedRowIndex == 0 && relVecDepth == 0) ?
        cout << ((get<relVecDepth>(relsTuple).fromIndex())->row())[joinedRowRow] << endl
             : (joinedRowIndex == relVecDepth + 1) ?
             cout << ((get<relVecDepth>(relsTuple).toIndex())->row())[joinedRowRow] << endl
             : cout;

        // if neither of those, then call deeper, otherwise just fall through and return
        (!((joinedRowIndex == 0 && relVecDepth == 0) || (joinedRowIndex == relVecDepth + 1))) ? queryPlanTupleOutput<remainingRelVecs...>()
        : emptyFunc();

        return;
    };

    void printQueryPlanTest()
    {
        for (int i = 0; i < arrayCount; ++i)
        {
            joinedRowIndex = i;
            joinedRowRow = j.k[i];
            queryPlanTupleOutput<relVecs...>();
        }
        cout << endl;
    }

    template<typename... remainingRelVecs> // Can't seem to get this struct method to work :(
    struct calls
    {
        relsTupleType structRelsTuple;

        static int structQueryPlanTupleOutput(std::size_t index, std::size_t row)
        {
            const std::size_t relVecDepth = sizeof...(relVecs) - sizeof...(remainingRelVecs) -1;
            //cout << endl << "index = " << index << ", row = " << row << ", relVecDepth = " << relVecDepth << endl << endl;
            return relVecDepth;
        }
    };

    template<typename firstRelVec, typename... remainingRelVecs> // Can't seem to get this struct method to work for tuple :(
    struct calls<firstRelVec, remainingRelVecs...>
    {
        relsTupleType structRelsTuple;

        int structEmptyFunc()
        {
            return 0;
        };

        int structQueryPlanTupleOutput(std::size_t index, std::size_t row)
        {
            const std::size_t relVecDepth = sizeof...(relVecs) - sizeof...(remainingRelVecs) -1;
            //cout << endl << "index = " << index << ", row = " << row << ", relVecDepth = " << relVecDepth << endl << endl;
            (index < relVecDepth) ? throw Variadic_Parameter_Pack_Logic_Failed() : ((void)0); // seriously out of whack.

            (index == 0 && relVecDepth == 0) ?
            cout << ((get<relVecDepth>(structRelsTuple).fromIndex())->row())[row] << endl
                 : (index == relVecDepth + 1) ?
                 cout << ((get<relVecDepth>(structRelsTuple).toIndex())->row())[row] << endl
                 : cout;

            // if neither of those -> call deeper or just fall through and return
            (!((index == 0 && relVecDepth == 0) || (index == relVecDepth + 1))) ? calls<remainingRelVecs...>::structQueryPlanTupleOutput(index, row)
            : structEmptyFunc();

            return relVecDepth;
        }
    };

    typedef calls<relVecs ...> callsType; // Can't seem to get this struct method to work for tuple :(

    callsType c; // Can't seem to get this struct method to work for tuple :(

    void printQueryPlanTest2() // Can't seem to get this struct method to work for tuple :(
    {
        for (int i = 0; i < arrayCount; ++i)
        {
            c.calls<relVecs...>::structQueryPlanTupleOutput(i, j.k[i]);
        }
    }

    QueryPlan(relVecs... args)
    {
        for (int i = 0; i < arrayCount; i++)
        {
            tableOffsets[i] = INT_MAX;
            joinedRowColumns[i] = Column::nilColumn;
            joinedRowTables[i] = Table::nilTable;
        }
        relsTuple = std::make_tuple(args...);
        checkQueryPlan();
        // c.calls<relVecs...>::structRelsTuple = std::make_tuple(args...); // Can't seem to get this struct method to work for tuple :(
    }

    // prints the joined rows
    friend ostream& operator<< (ostream &os, const QueryPlan& rhs)
    {
        for (int i = 0; i < (int)Table::table_Count; i++)
        {
            // this means all tables must be allocated and anchored before the first join is accessed this way
            if (rowAnchors[i] == 0)
                throw Bad_RowString_Anchor(); // no index for this table
        }
        return os;
    }

};

template <typename queryPlanRelsTupleType, int placeholder, typename... relVecs>
class JoinedRow
{
public:
    typedef QueryPlan<relVecs...> queryPlanType;
    friend class QueryPlan<relVecs...>;

    struct JoinedRowStruct
    {
        // These are the k-values indicating a specific row in a specific indexed table: a type of
        // template class RowString. There will be at least 2 values, N = sizeof...(relVecs)+1
        int k[queryPlanType::arrayCount] = {0, 0};
    };

    JoinedRowStruct j;

    class Bad_RowString_Anchor : public runtime_error
    {
    public:
        Bad_RowString_Anchor() :
            runtime_error("after allocation, you MUST call rowArray[0].dropAnchor(rowArray, correct_arrayCount)") {}
    };

    class Variadic_Parameter_Pack_Logic_Failed : public runtime_error
    {
    public:
        Variadic_Parameter_Pack_Logic_Failed() :
            runtime_error("somehow, the counting of JoinedRow elements is wrong versus relational vectors.") {}
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

    void test()
    {
        for (int i = 0; i < queryPlanType::arrayCount; ++i)
        {
            joinedRowOutput(i, j.k[i], forward_as_tuple(queryPlanType::relsTuple));
        }
    }

    // prints the joined rows
    friend ostream& operator<< (ostream &os, const JoinedRow& rhs)
    {
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
