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
		list->tail->next = new_node;
		list->tail = new_node;
	}

	list->length++;

	return 0;
}

int copy_to_list(List* list, void* data, int data_len)
{
	ListNode* new_node = malloc(sizeof *new_node);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != new_node, "[list]: Failed to allocate sufficient memory for the list.");
	void* data_ptr = malloc(data_len);
	CHECK_EXPR_FAIL_RET_TERMINATE(NULL != new_node, "[list]: Failed to allocate sufficient memory for the list data.");

	new_node->next = NULL;
	new_node->prev = NULL;
	new_node->data = data_ptr;

	memcpy(new_node->data, data, data_len);

	if (NULL == list->head)
	{
		list->head = new_node;
		list->tail = new_node;
	}
	else
	{
		list->tail->next = new_node;
		list->tail = new_node;
	}

	list->length++;

	return 0;
}

// TODO: Implement
int free_list(List* list)
{
	return 0;
}
