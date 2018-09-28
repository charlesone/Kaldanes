#ifndef RELATIONVECTOR_H
#define RELATIONVECTOR_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"
#include <string>
#include <typeinfo>

#ifdef __GNUG__
#include <cxxabi.h> // gcc only: fetching type names
#endif // __GNUG__

using namespace std;

// This is a database relation using indexes with a directional component for automatic optimization.
template <typename fromIndexType, Column fromColumnEnum, typename toIndexType, Column toColumnEnum>
class RelationVector
{
public:
    class Bad_IndexString_Anchor : public runtime_error
    {
    public:
        Bad_IndexString_Anchor() :
            runtime_error("no index, or corrupted index for this column of this table") {}
    };

    constexpr static void* testedIndexStringAnchor(void* indexStringAnchor)
    {
        return (indexStringAnchor != 0) ? indexStringAnchor : throw Bad_IndexString_Anchor();
    }

    struct RelationStruct
    {
        fromIndexType* from = reinterpret_cast<fromIndexType*>(testedIndexStringAnchor(indexAnchors[((fromIndexType*)0)->indexAnchorOff()]));
        toIndexType* to = reinterpret_cast<toIndexType*>(testedIndexStringAnchor(indexAnchors[((toIndexType*)0)->indexAnchorOff()]));
    };

    RelationStruct r;

    fromIndexType* fromIndex()
    {
        return r.from;
    }

    toIndexType* toIndex()
    {
        return r.to;
    }


    // prints the metadata
    friend ostream& operator<< (ostream &os, const RelationVector& rhs)
    {
        os << rhs.fromIndex() << endl << rhs.toIndex() << endl;
        return os;
    }

};

#endif // RELATIONVECTOR_H
