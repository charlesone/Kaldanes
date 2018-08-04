#ifndef RELATION_H
#define RELATION_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"

using namespace std;

template <typename fromIndex, typename toIndex>
class Relation
{
public:

    class Bad_IndexString_Anchor : public runtime_error
    {
    public:
        Bad_IndexString_Anchor() :
            runtime_error("no index, or corrupted index for this column of this table") {}
    };

    constexpr void* testIndexStringAnchor(void* indexStringAnchor)
    {
        if (indexStringAnchor == 0) throw Bad_IndexString_Anchor();
        return indexStringAnchor;
    }

    struct RelationStruct
    {
        fromIndex* from = reinterpret_cast<fromIndex*>(testIndexStringAnchor(indexAnchors[((fromIndex)0)->indexAnchorOff()]));
        toIndex* to = reinterpret_cast<toIndex*>(testIndexStringAnchor(indexAnchors[((toIndex)0)->indexAnchorOff()]));
    };

    RelationStruct r;
};

#endif // RELATION_H
