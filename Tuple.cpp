 // Simplified version of tuple
// from: https://gist.github.com/IvanVergiliev/9639530

#include <cstdio>

using namespace std;

template<typename First, typename... Rest>
struct Tuple: public Tuple<Rest...> {
  Tuple(First first, Rest... rest): Tuple<Rest...>(rest...), first(first) {}

  First first;
};

template<typename First>
struct Tuple<First> {
  Tuple(First first): first(first) {}

  First first;
};

template<int index, typename First, typename... Rest>
struct GetImpl {
  static auto value(const Tuple<First, Rest...>* t) -> decltype(GetImpl<index - 1, Rest...>::value(t)) {
    return GetImpl<index - 1, Rest...>::value(t);
  }
};

template<typename First, typename... Rest>
struct GetImpl<0, First, Rest...> {
  static First value(const Tuple<First, Rest...>* t) {
    return t->first;
  }
};

template<int index, typename First, typename... Rest>
auto get(const Tuple<First, Rest...>& t) -> decltype(GetImpl<index, First, Rest...>::value(&t)) { //typename Type<index, First, Rest...>::value {
  return GetImpl<index, First, Rest...>::value(&t);
}

int main() {
  Tuple<int, int, double> c(3, 5, 1.1);
  printf("%d\n", get<1>(c));
}
