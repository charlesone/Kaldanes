#ifndef RELATIONVECTOR_H
#define RELATIONVECTOR_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"

using namespace std;

// This is a database relation using indexes with a directional component for automatic optimization.
template <typename fromIndex, typename toIndex>
class RelationVector
{
public:

    class Bad_IndexString_Anchor : public runtime_error
    {
    public:
        Bad_IndexString_Anchor() :
            runtime_error("no index, or corrupted index for this column of this table") {}
    };

    constexpr void* testedIndexStringAnchor(void* indexStringAnchor)
    {
        if (indexStringAnchor == 0) throw Bad_IndexString_Anchor();
        return indexStringAnchor;
    }

    struct RelationStruct
    {
        fromIndex* from = reinterpret_cast<fromIndex*>(testedIndexStringAnchor(indexAnchors[((fromIndex)0)->indexAnchorOff()]));
        toIndex* to = reinterpret_cast<toIndex*>(testedIndexStringAnchor(indexAnchors[((toIndex)0)->indexAnchorOff()]));
    };

    RelationStruct r;
};

#endif // RELATIONVECTOR_H
