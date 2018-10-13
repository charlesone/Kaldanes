#ifndef SORTS_H_INCLUDED
#define SORTS_H_INCLUDED

// typical sorting code ...

class Sort_Bounds_Error : public runtime_error
{
public:
    Sort_Bounds_Error() :
        runtime_error("Array non-negative 'low' must initially be less than non-negative 'high'") {}
};

class Index_Value_Error : public runtime_error
{
public:
    Index_Value_Error() :
        runtime_error("Index value has wrong sign") {}
};

double compares;
double swaps;

template<typename T>
void selectionSortInvoke(T arr[], int size)
{
    T temp;
    int first;
    for (int i = size - 1; i > 0; --i)
    {
        first = 0;
        for (int j = 1; j <= i; ++j)
        {
            if (arr[j] > arr[first])
            {
                first = j;
            }
            temp = arr[first];
            arr[first] = arr[i];
            arr[i] = temp;
            swaps++;
            compares++;
        }
    }
}

template<typename T>
void shellSortInvoke(T arr[], int size)
{
    int h = 1;
    T temp;
    while (h < size/3)
    {
        h = 3*h + 1;
    }
    while (h >= 1)
    {
        for (int i = h; i < size; ++i)
        {
            for (int j = i; ++compares > 0 && j >= h && arr[j] < arr[j-h]; j -= h)
            {
                temp = arr[j];
                arr[j] = arr[j-h];
                arr[j-h] = temp;
                swaps++;
            }
        }
        h = h/3;
    }
    return;
}

template<typename T>
void insertionSortInvokeOld(T arr[], int size)
{
    T temp;
    int j;
    for (int i = 1; i < size; ++i)
    {
        j = i;
        while (j > 0 && ++compares > 0 && arr[j-1] > arr[j])
        {
            temp = arr[j];
            arr[j] = arr[j-1];
            arr[j-1] = temp;
            j--;
            swaps++;
        }
    }
}

template<typename T>
void insertionSortInvoke (T arr[], int size)
{
    T temp;
    for (int i = 1; i < size; ++i)
    {
        for (int j = i; ++compares > 0 && j > 0 && arr[j] < arr[j-1]; --j)
        {
            temp = arr[j];
            arr[j] = arr[j-1];
            arr[j-1] = temp;
            swaps++;
        }
    }
    return;
}

template<typename T>
void merge(T arr[], int size, int low, int middle, int high)
{
    T temp[size];
    for (int i = low; i <= high; ++i)
    {
        temp[i] = arr[i];
    }
    int i = low;
    int j = middle + 1;
    int k = low;

    while (i <= middle && j <= high)
    {
        if (++compares > 0 && temp[i] <= temp[j])
        {
            arr[k] = temp[i];
            ++i;
            swaps++;
        }
        else
        {
            arr[k] = temp[j];
            ++j;
            swaps++;
        }
        ++k;
    }
    while (i <= middle)
    {
        arr[k] = temp[i];
        ++k;
        ++i;
        swaps++;
    }
}

template<typename T>
void mergeSort(T arr[], int size, int low, int high)
{
    if (low < high)
    {
        int middle = (low + high) / 2;
        mergeSort(arr, size, low, middle); // bottom half
        mergeSort(arr, size, middle + 1, high); // top half
        merge(arr, size, low, middle, high); // all of it
    }
}

template<typename T>
void mergeSortInvoke(T arr[], int size)
{
    int low = 0;
    int high = size-1;
    if (low < 0 || high < 0 || low >= high) throw Sort_Bounds_Error();
    mergeSort(arr, size, low, high);
}

template<typename T>
void quickSort(T arr[], int left, int right)
{
    int i = left;
    int j = right;
    T tmp;
    T pivot = arr[(left+right)/2];
    swaps++;
    while (i <= j)
    {
        while (++compares > 0 && arr[i] < pivot )
        {
            i++;
        }
        while (++compares > 0 && arr[j] > pivot )
        {
            j--;
        }
        if (i <= j)
        {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
            swaps++;
        }
    }
    if (left < j)
        quickSort(arr, left, j);
    if (i < right)
        quickSort(arr, i, right);
}

template<typename T>
void quickSortInvoke(T arr[], int size)
{
    int left = 0;
    int right = size-1;
    if (left < 0 || right < 0 || left >= right) throw Sort_Bounds_Error();
    quickSort(arr, left, right);
}

template<typename T>
void stlSortInvoke(T arr[], int size)
{
    // Can't seem to get past a const bug: "discards qualifiers"
    //std::sort(arr, arr + size - 1, [](T a, T b) {return (a < b);} );
}

/*// negative result means not an exact match: negative index of where it would be
template<typename T, typename U>
int binarySearch(T arr[], std::size_t size, U value)
{
    int len = strlen(value.c_str());
    char str[len+1]; // null terminated
    strcpy(str, value.c_str());
    str[len] = 0;
    int left = 0;
    int right = size - 1;
    int middle = -1;
    while (left <= right)
    {
        middle = (left + right) / 2;
        if (arr[middle] == str) return middle;
        else if (arr[middle] > str) right = middle - 1;
        else left = middle + 1;
    }
    return -middle; // for range searches (inexact)
}*/

// negative result means not an exact match: negative index of where it would be
template<typename T>
int binarySearch(T arr[], std::size_t size, const T value)
{
    int left = 0;
    int right = size - 1;
    int middle = -1;
    while (left <= right)
    {
        middle = (left + right) / 2;
        if (arr[middle] == value) return middle;
        else if (arr[middle] > value) right = middle - 1;
        else left = middle + 1;
    }
    return -middle; // for range searches (inexact)
}

// negative result means not an exact match: negative index of where it would be
template<typename T>
int binarySearch(T arr[], std::size_t size, const char value[])
{
    int left = 0;
    int right = size - 1;
    int middle = -1;
    while (left <= right)
    {
        middle = (left + right) / 2;
        if (arr[middle] == value) return middle;
        else if (arr[middle] > value) right = middle - 1;
        else left = middle + 1;
    }
    return -middle; // for range searches (inexact)
}

// negative result means not an exact match: negative index of where it would be
template<typename T>
int binarySearchRangeLow(T arr[], std::size_t size, const T value)
{
    T belowValue(value);
    belowValue = value - 1;
    int returnIndex = binarySearch(arr, size, belowValue);
    returnIndex = abs(returnIndex); // don't expect to find the precedent, but might
    for (; arr[returnIndex + 1] < value; returnIndex++); // could be a set of belowValue
    if (arr[returnIndex] != value) returnIndex = -returnIndex; // didn't find the value
    return returnIndex;
}

// negative result means not an exact match: negative index of where it would be
template<typename T>
int binarySearchRangeLow(T arr[], std::size_t size, const char value[])
{
    const bool debugOutput = false;
    int len = strlen(value);
    char belowValue[len + 1];
    strcpy(belowValue, value);
    if (value[len - 1] > CHAR_MIN) // could be a value ending in CHAR_MIN != 0
    {
        belowValue[len - 1] = value[len - 1] - 1;
        belowValue[len] = '~';
        belowValue[len + 1] = 0;
    }
    else
    {
        belowValue[len] = '!';
        belowValue[len + 1] = 0;
    }
    int returnIndex = binarySearch(arr, size, belowValue);
    returnIndex = abs(returnIndex); // don't expect to find the precedent, but might
    if (debugOutput) cout << endl << "Range low for: " << belowValue << ", moving back, starting at: " << arr[returnIndex].c_str() << ":" << returnIndex;
    for (; arr[returnIndex - 1] >= value; returnIndex--) // could be a set of value ending in CHAR_MIN
    {
        if (debugOutput) cout << ", " << arr[returnIndex].c_str() << ":" << returnIndex;
    };
    if (debugOutput) cout << endl << "Range low, moving up, starting at: " << arr[returnIndex].c_str() << ":" << returnIndex;
    for (; arr[returnIndex + 1] < value; returnIndex++) // could be a set of belowValue
    {
        if (debugOutput) cout << ", " << arr[returnIndex].c_str() << ":" << returnIndex;
    };
    if (arr[returnIndex] != value)
    {
        if (debugOutput)
        {
            cout << ", failed ending at: " << arr[returnIndex].c_str() << ":" << returnIndex << endl;
            cout << *((arr[returnIndex]).row()) << endl;
        }
        returnIndex = -returnIndex; // didn't find the value
    }
    else
    {
        if (debugOutput)
        {
            cout << ", ending at: " << arr[returnIndex].c_str() << ":" << returnIndex << endl;
            cout << *((arr[returnIndex]).row()) << endl;
        }
    }
    return returnIndex;
}

// negative result means not an exact match: negative index of where it would be
template<typename T>
int binarySearchRangeHigh(T arr[], std::size_t size, const T value)
{
    T aboveValue(value);
    aboveValue = value + 1;
    int returnIndex = binarySearch(arr, size, aboveValue);
    returnIndex = abs(returnIndex); // don't expect to find the successor, but might
    for (; arr[returnIndex - 1] > value; returnIndex--); // could be a set of aboveValue
    if (arr[returnIndex] != value) returnIndex = -returnIndex; // didn't find the value
    return returnIndex;
}

// negative result means not an exact match: negative index of where it would be
template<typename T>
int binarySearchRangeHigh(T arr[], std::size_t size, const char value[])
{
    const bool debugOutput = false;
    int len = strlen(value);
    char aboveValue[len + 1];
    strcpy(aboveValue, value);
    if (value[len - 1] < CHAR_MAX) // could be a value ending in CHAR_MAX
    {
        aboveValue[len] = '!';
        aboveValue[len + 1] = 0;
    }
    else
    {
        aboveValue[len] = '!';
        aboveValue[len + 1] = 0;
    }
    int returnIndex = binarySearch(arr, size, aboveValue);
    returnIndex = abs(returnIndex); // don't expect to find the successor, but might
    if (debugOutput) cout << endl << "Range high for: " << aboveValue << ", moving up, starting at: " << arr[returnIndex].c_str() << ":" << returnIndex;
    for (; arr[returnIndex + 1] <= value; returnIndex++) // could be a set of value ending in CHAR_MAX
    {
        if (debugOutput) cout << ", " << arr[returnIndex].c_str() << ":" << returnIndex;
    };
    if (debugOutput) cout << endl << "Range high, moving back, starting at: " << arr[returnIndex].c_str() << ":" << returnIndex;
    for (; arr[returnIndex - 1] >= value; returnIndex--) // could be a set of aboveValue
    {
        if (arr[returnIndex - 1] == value)
        {
            returnIndex--;
            break;
        }
        if (debugOutput) cout << ", " << arr[returnIndex].c_str() << ":" << returnIndex;
    };
    if (arr[returnIndex] != value)
    {
        if (debugOutput)
        {
            cout << ", failed ending at: " << arr[returnIndex].c_str() << ":" << returnIndex << endl;
            cout << *((arr[returnIndex]).row()) << endl;
        }
        returnIndex = -returnIndex; // didn't find the value
    }
    else
    {
        if (debugOutput)
        {
            cout << ", ending at: " << arr[returnIndex].c_str() << ":" << returnIndex << endl;
            cout << *((arr[returnIndex]).row()) << endl;
        }
    }
    return returnIndex;
}

#endif // SORTS_H_INCLUDED
