
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

static bool joinedRowTablesIncluded[100];

template <typename... relVecs>
class QueryPlan
{
public:
    static const std::size_t arrayCount = (sizeof...(relVecs)) + 1;
    typedef tuple<relVecs...> relVecsTupleType;

    int joinedRowIndex;
    int joinedRowRow;

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

/*  // Can't seem to get this struct method to work :(
    template<typename... remainingRelVecs>
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

    template<typename firstRelVec, typename... remainingRelVecs>
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

    typedef calls<relVecs ...> callsType;

    callsType c;

    void printQueryPlanTest2()
    {
        for (int i = 0; i < arrayCount; ++i)
        {
            c.calls<relVecs...>::structQueryPlanTupleOutput(i, j.k[i]);
        }
    }
*/

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

    QueryPlan(relVecs... args)
    {
        checkQueryPlan();
        static relVecsTupleType relVecsTuple = std::make_tuple(args...);
        static_assert((sizeof...(relVecs)) == (std::tuple_size<decltype(relVecsTuple)>::value), "Parameter pack size and tuple size must match."); // at the very least, other checks  should be made.

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

template <typename... relVecs>
class JoinedRow
{
public:
    typedef QueryPlan<relVecs...> queryPlanType;
    friend class QueryPlan<relVecs...>;

    static_assert((sizeof...(relVecs)) == (std::tuple_size<decltype(relVecsTuple)>::value), "Parameter pack size and tuple size must match."); // at the very least, other checks  should be made.
    static const std::size_t arrayCount = (sizeof...(relVecs)) + 1;
    static_assert((queryPlanType::arrayCount == arrayCount), "Query Plan and Joined Row array counts must match.");

    int joinedRowIndex;
    int joinedRowRow;

    struct JoinedRowStruct
    {
        int k[arrayCount];
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

    constexpr std::size_t tableCount()
    {
        return arrayCount;
    }

    constexpr std::size_t structSize()
    {
        return sizeof(j);
    }

    constexpr void emptyFunc()
    {
        return;
    };

    template<typename... remainingRelVecs>
    constexpr typename std::enable_if<sizeof...(remainingRelVecs) == 0>::type joinedRowTupleOutput() {} // SFINAE'd away.

    template<typename firstRelVec, typename... remainingRelVecs>
    void joinedRowTupleOutput()// of course this method only seems to work for void returns with no parameters :(
    {
        const std::size_t relVecDepth = sizeof...(relVecs) - sizeof...(remainingRelVecs) -1;
        //cout << endl << "index = " << joinedRowIndex << ", row = " << joinedRowRow << ", relVecDepth = " << relVecDepth << endl << endl;
        (joinedRowIndex < relVecDepth) ? throw Variadic_Parameter_Pack_Logic_Failed() : ((void)0); // seriously out of whack.

        (joinedRowIndex == 0 && relVecDepth == 0) ?
        cout << ((get<relVecDepth>(relVecsTuple).fromIndex())->row())[joinedRowRow] << endl
             : (joinedRowIndex == relVecDepth + 1) ?
             cout << ((get<relVecDepth>(relVecsTuple).toIndex())->row())[joinedRowRow] << endl
             : cout;

        // if neither of those, then call deeper, otherwise just fall through and return
        (!((joinedRowIndex == 0 && relVecDepth == 0) || (joinedRowIndex == relVecDepth + 1))) ? joinedRowTupleOutput<remainingRelVecs...>()
        : emptyFunc();

        return;
    };

    void joinedRowPrintTest()
    {
        for (int i = 0; i < arrayCount; ++i)
        {//= {3530,47126,3657,4539}
            j.k[i]=(i==0)?3530:(i==1)?47126:(i==2)?3657:(i==3)?4539:j.k[i]; // Southwest route from SJC to LAS
            joinedRowIndex = i;
            joinedRowRow = j.k[i];
            joinedRowTupleOutput<relVecs...>();
        }
        cout << endl;
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
