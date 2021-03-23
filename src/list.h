#pragma once

#include <utility>

template<class T>
struct ListNode
{
	static ListNode* insert_head(ListNode* head, ListNode* node)
	{
		node->next = head;

		return node;
	}

	static void insert_after(ListNode* prev, ListNode* node)
	{
		node->next = prev->next;
		prev->next = node;
	}

	static void erase(ListNode* prev, ListNode* node)
	{
		prev->next = node->next;
		node->next = nullptr;
	}

	static std::pair<ListNode*, ListNode*> find(ListNode* head, const T& data)
	{
		ListNode* prev = nullptr;
		ListNode* curr = head;
		while (curr != nullptr && curr->data != data)
		{
			prev = curr;
			curr = curr->next;
		}
		return {prev, curr};
	}


	T data{};

	ListNode* next{};
};