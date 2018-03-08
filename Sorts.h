#ifndef SORTS_H_INCLUDED
#define SORTS_H_INCLUDED

// typical sorting code ...

class Sort_Bounds_Error : public runtime_error
{
public:
    Sort_Bounds_Error() :
        runtime_error("Array non-negative 'low' must initially be less than non-negative 'high'") {}
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

#endif // SORTS_H_INCLUDED
