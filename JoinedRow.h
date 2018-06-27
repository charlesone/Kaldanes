#ifndef JOINEDROW_H
#define JOINEDROW_H

#include <iostream>
#include "RowString.h"
#include "IndexString.h"

using namespace std;

template <const Column columnArray[], std::size_t sizeofColumnArray, typename tableTypes>
class JoinedRow
{
public:
    static const std::size_t arrayCount = sizeofColumnArray/sizeof(columnArray[0]);

    struct JoinedRowStruct
    {
        int k[arrayCount];
        bool valid;
    };

    JoinedRowStruct j;

    class Bad_IndexString_Anchor : public runtime_error
    {
    public:
        Bad_IndexString_Anchor() :
            runtime_error("no index, or corrupted index for this column of  this table") {}
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
        cout << endl << "sizeofColumnArray = " << sizeofColumnArray
        << ", Loki::TL::Length<tableTypes>::value = " << Loki::TL::Length<tableTypes>::value
        << endl <<endl;

        for (int i = 0; i < arrayCount; i++)
        {
            if (charAnchors[(int)columnArray[i]] == 0)
                throw Bad_IndexString_Anchor(); // no index for this column
            int tableIndex = (int)column2Table(columnArray[i]);
            //Loki::TL::TypeAt<tableTypes, 1>::Result
//            if (tableAnchors
//            os << tableAnchors[(int)columnArray[i]];
        }
        return os;
    }

};

#endif // JOINEDROW_H
