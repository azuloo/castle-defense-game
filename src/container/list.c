#include "container/list.h"
#include "utils.h"

int create_list(List** dest)
{
	List* list = malloc(sizeof *list);
	CHECK_EXPR_FAIL_RET_TERMINATE(list != NULL, "[list]: Failied to allocate sufficient memory for the list.");

	list->head = NULL;
	list->tail = NULL;
	list->length = 0;

	*dest = list;

	return 0;
}

int add_to_list(List* list, void* data, int data_len)
{
	ListNode* new_node = malloc(sizeof * new_node);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != new_node, "[list]: Failed to allocate sufficient memory for the list.");

	new_node->next = NULL;
	new_node->prev = NULL;
	new_node->data = data;

	if (NULL == list->head)
	{
		list->head = new_node;
		list->tail = new_node;
	}
	else
	{
		new_node->prev = list->tail;
		list->tail->next = new_node;
		list->tail = new_node;
	}

	list->length++;

	return 0;
}

int remove_from_list(List* list, ListNode* node)
{
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != list && list->length > 0, "[list]: The list is empty.");
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != node, "[list]: The node is empty.");

	// List contains one element.
	if (list->head == list->tail)
	{
		list->head = NULL;
		list->tail = NULL;
		list->length = 0;
	}
	else
	{
		CHECK_EXPR_FAIL_RET_TERMINATE(list->length > 1, "[list]: The list is malformed.");
		list->length -= 1;

		if (list->head == node)
		{
			list->head->next->prev = NULL;
			list->head = list->head->next;
			
		}
		else if (list->tail == node)
		{
			list->tail->prev->next = NULL;
			list->tail = list->tail->prev;
		}
		else
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
	}

	free(node);

	return 0;
}

// TODO: Implement
int free_list(List* list)
{
	return 0;
}
