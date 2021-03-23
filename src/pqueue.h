#pragma once

#include <cassert>
#include <vector>
#include <algorithm>

namespace heap
{
	// just a little template, not general purpose tool
	// It will be modified for some specific needs
	 
	// min heap
	void sift_down(std::vector<int>& a, int i)
	{
		while (2 * i + 1 < a.size())
		{
			int left  = 2 * i + 1;
			int right = 2 * i + 2;

			int j = left;
			if (right < a.size() && a[right] < a[left]) // we swap with the smallest child
			{
				j = right;
			}

			if (a[i] <= a[j]) // no need to swap further
			{
				break;
			}
			std::swap(a[i], a[j]);

			i = j;
		}
	}

	void sift_up(std::vector<int>& a, int i)
	{
		while (a[i] < a[(i - 1) / 2]) // while parent is greater
		{
			std::swap(a[i], a[(i - 1) / 2]);

			i = (i - 1) / 2;
		}
	}

	void push(std::vector<int>& a, int elem)
	{
		a.push_back(elem);

		sift_up(a, a.size() - 1);
	}

	void pop(std::vector<int>& a)
	{
		std::swap(a[0], a.back());
		a.pop_back();

		sift_down(a, 0);
	}

	void heapify(std::vector<int>& a)
	{
		for (int i = a.size() / 2; i >= 0; i--)
		{
			sift_down(a, i);
		}
	}
}
