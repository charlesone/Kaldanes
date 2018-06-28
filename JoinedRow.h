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
        for (int i = 0; i < arrayCount; i++)
        {
            if (charAnchors[(int)columnArray[i]] == 0)
                throw Bad_RowString_Anchor(); // no index for this table
        }
        for (int i = 0; i < arrayCount; i++)
        {
            os << columnEnum2RowStringAnchor<columnArray[1]>()[rhs.j.k[0]];
        }
        return os;

/*        Loki::TL::TypeAt<tableTypes, (std::size_t)(column2Table(columnArray[0]))>* ptr0 = columnEnum2RowStringAnchor<columnArray[0]>();
        Loki::TL::TypeAt<tableTypes, (std::size_t)(column2Table(columnArray[1]))>* ptr1 = columnEnum2RowStringAnchor<columnArray[1]>();
        Loki::TL::TypeAt<tableTypes, (std::size_t)(column2Table(columnArray[2]))>* ptr2 = columnEnum2RowStringAnchor<columnArray[2]>();
        Loki::TL::TypeAt<tableTypes, (std::size_t)(column2Table(columnArray[3]))>* ptr3 = columnEnum2RowStringAnchor<columnArray[3]>();
        Loki::TL::TypeAt<tableTypes, (std::size_t)(column2Table(columnArray[4]))>* ptr4 = columnEnum2RowStringAnchor<columnArray[4]>();
        int offset0 = rhs.j.k[0];
//        os << ptr1[j.k[0]];
        if (sizeofColumnArray < 2) return os;
/*        os << columnEnum2RowStringAnchor<columnArray[1]>()[j.k[i]];
        if (sizeofColumnArray < 3) return os;
        os << columnEnum2RowStringAnchor<columnArray[2]>()[j.k[i]];
        if (sizeofColumnArray < 4) return os;
        os << columnEnum2RowStringAnchor<columnArray[3]>()[j.k[i]];
        if (sizeofColumnArray < 5) return os;
        os << columnEnum2RowStringAnchor<columnArray[4]>()[j.k[i]];
        if (sizeofColumnArray < 6) return os;
        os << columnEnum2RowStringAnchor<columnArray[5]>()[j.k[i]];
        if (sizeofColumnArray < 7) return os;
        os << columnEnum2RowStringAnchor<columnArray[6]>()[j.k[i]];
        if (sizeofColumnArray < 8) return os;
        os << columnEnum2RowStringAnchor<columnArray[7]>()[j.k[i]];
        if (sizeofColumnArray < 9) return os;
        os << columnEnum2RowStringAnchor<columnArray[8]>()[j.k[i]];
        if (sizeofColumnArray < 10) return os;
        os << columnEnum2RowStringAnchor<columnArray[9]>()[j.k[i]];
        if (sizeofColumnArray < 11) return os;
        os << columnEnum2RowStringAnchor<columnArray[10]>()[j.k[i]];
        if (sizeofColumnArray < 12) return os;
        os << columnEnum2RowStringAnchor<columnArray[11]>()[j.k[i]];
        if (sizeofColumnArray < 13) return os;
        os << columnEnum2RowStringAnchor<columnArray[12]>()[j.k[i]];
        if (sizeofColumnArray < 14) return os;
        os << columnEnum2RowStringAnchor<columnArray[13]>()[j.k[i]];
        if (sizeofColumnArray < 15) return os;
        os << columnEnum2RowStringAnchor<columnArray[14]>()[j.k[i]];
        if (sizeofColumnArray < 16) return os;
        os << columnEnum2RowStringAnchor<columnArray[15]>()[j.k[i]];
        if (sizeofColumnArray < 17) return os;
        os << columnEnum2RowStringAnchor<columnArray[16]>()[j.k[i]];
        if (sizeofColumnArray < 18) return os;
        os << columnEnum2RowStringAnchor<columnArray[17]>()[j.k[i]];
        if (sizeofColumnArray < 19) return os;
        os << columnEnum2RowStringAnchor<columnArray[18]>()[j.k[i]];
        if (sizeofColumnArray < 20) return os;
        os << columnEnum2RowStringAnchor<columnArray[19]>()[j.k[i]];
        return os;*/
    }

};

#endif // JOINEDROW_H
