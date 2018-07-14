#ifndef ROW_H
#define ROW_H

using namespace std;

// Static (compile time) polymorphism for IndexString templates (no vtable performance hit)
class Row
{
public:
    Row()
    {
    }

    Row(Row& rhs)
    {
    }

};

#endif // ROW_H
