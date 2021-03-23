#include "qsort.h"

int partition(std::vector<int>& nums, int l, int r)
{
    --r;

    int med = nums[l + ((r - l) >> 1)];
    while (l <= r)
    {
        while (nums[r] > med)
        {
            --r;
        }
        while (nums[l] < med)
        {
            ++l;
        }
        if (l >= r)
        {
            break;
        }
        std::swap(nums[l++], nums[r--]);
    }

    return r + 1;
}

void qsort(std::vector<int>& nums, int l, int r)
{
    if (l + 1 < r)
    {
        int q = partition(nums, l, r);

        qsort(nums, l, q);
        qsort(nums, q, r);
    }
}


