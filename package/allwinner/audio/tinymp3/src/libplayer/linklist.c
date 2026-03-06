/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
	linklist.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linklist.h"

/**
 * create_listnode -
 * @dataSize:
 */
struct listNode *create_listnode(int dataSize)
{
	struct listNode *node;

	node = malloc(sizeof(struct listNode) + dataSize);
	if (!node) {
		perror("Create linklist node");
		return NULL;
	}

	node->data = (void *)(node + 1);
	node->next = NULL;

	return node;
}

/**
 * make_listnode -
 * @data:
 */
struct listNode *make_listnode(void *data)
{
	struct listNode *node;

	node = malloc(sizeof(struct listNode));
	if (!node) {
		perror("Make linklist node");
		return NULL;
	}

	node->data = data;
	node->next = NULL;

	return node;
}

/**
 * delete_listnode -
 * @node:
 * @fn:
 * @fn_param:
 */
void delete_listnode(struct listNode *node, cleanNodeFun fn, void *fn_param)
{
	if (fn)
		fn(node, fn_param);

	free(node);
}

/**
 * linkList_init -
 * @list:
 */
void linkList_init(struct linkList *list)
{
	list->head	= NULL;
	list->tail	= NULL;
	list->count	= 0;
}

/**
 * linkList_destroy -
 * @list:
 * @fn:
 * @fn_param:
 */
int linkList_destroy(struct linkList *list, cleanNodeFun fn, void *fn_param)
{
	struct listNode *node = list->head;
	struct listNode *next;

	while (node) {
		next = node->next;
		delete_listnode(node, fn, fn_param);
		node = next;
		list->count--;
	}

	list->head	= NULL;
	list->lastaccess = NULL;
	list->tail	= NULL;

	if (list->count) {
		fprintf(stderr, "%s. this list has residual after destroy.\n", __func__);
		return -1;
	}

	return 0;
}

/**
 * list_insert_head -
 * @list:
 * @node:
 */
void list_insert_head(struct linkList *list, struct listNode *node)
{
	if (is_empty(list)) {
		list->head	= node;
		list->tail	= node;
	} else {
		node->next	= list->head;
		list->head	= node;
	}

	list->count++;
}

/**
 * list_insert_tail -
 * @list:
 * @node:
 */
void list_insert_tail(struct linkList *list, struct listNode *node)
{
	if (is_empty(list)) {
		list->head	= node;
		list->tail	= node;
	} else {
		list->tail->next	= node;
		list->tail		= node;
	}

	list->count++;
}

/**
 * list_find_node -
 * @list:
 * @s_node:
 * @fn:
 * @fn_param:
 *
 * Note:
 *	if s_node is NULL, find begin at list head.
 *	if match return this node, or return NULL
 *	include this list is empty.
 */
struct listNode *list_find_node(struct linkList *list, struct listNode *s_node,
				matchNodeFun fn, void *fn_param)
{
	struct listNode *node = s_node ? s_node : list->head;

	while (node) {
		if (fn(node, fn_param)) {
			list->lastaccess = node;
			return node;
		}

		node = node->next;
	}

	list->lastaccess = NULL;
	return NULL;
}

/**
 * list_pick_node -
 * @list:
 * @node:
 *
 * if node matched at head, the list tail
 * set NULL.
 *
 * if node matched at tail, the list tail
 * set previous node.
 */
int list_pick_node(struct linkList *list, struct listNode *node)
{
	struct listNode *preNode = list->head;
	int done = 0;

	if (is_empty(list))
		return -1;

	/* Check head node is match */
	if (preNode == node) {
		/* Link list has only one member */
		if (list->tail == preNode)
			list->tail = NULL;
		list->head	= node->next;
		node->next	= NULL;
		done = 1;
	} else {
		while (preNode->next) {
			if (preNode->next == node) {
				if (list->tail == node)
					list->tail = preNode;
				preNode->next	= node->next;
				node->next	= NULL;
				done = 1;
				break;
			}

			preNode = preNode->next;
		}
	}

	if (done)
		list->count--;

	return done ? 0 : -1;
}

/**
 * list_pick_head -
 * @list:
 *
 * if list has only one node,
 * list head and tail all NULL.
 */
struct listNode *list_pick_head(struct linkList *list)
{
	struct listNode *node = NULL;

	if (!is_empty(list)) {
		node		= list->head;
		list->head	= node->next;
		if (list->tail == node)
			list->tail = NULL;
		node->next	= NULL;
		list->count--;
	}

	return node;
}

/**
 * interlist_move_node -
 * @list2:
 * @list1:
 * @node:
 *
 * Pick up node form list1, insert to list2 tail.
 */
int interlist_move_node(struct linkList *list2, struct linkList *list1,
			struct listNode *node)
{
	if (!list1 || !list2 || !node) {
		fprintf(stderr, "%s, illegal arguments.\n", __func__);
		return -1;
	}

	if (list_pick_node(list1, node) < 0)
		return -1;

	list_insert_tail(list2, node);

	return 0;
}

/**
 * list_move_tail-
 * @list:
 * @node:
 */
int list_move_tail(struct linkList *list, struct listNode *node)
{
	return interlist_move_node(list, list, node);
}
