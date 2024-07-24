#ifndef _LIST_H
#define _LIST_H

typedef struct ListNode ListNode;

typedef struct ListNode
{
	ListNode* prev;
	ListNode* next;
	void* data;
} ListNode;

typedef struct List
{
	ListNode* head;
	ListNode* tail;
	int length;
} List;

int create_list(List** dest);
int add_to_list(List* list, void* data, int data_len);
int copy_to_list(List* list, void* data, int data_len);
int free_list(List* list);

#endif // _LIST_H
