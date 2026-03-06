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
	linklist.h
 */
#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include <stdbool.h>

/**
 * listNode -
 */
struct listNode {
	void		*data;
	struct listNode	*next;
};

/**
 * linkList -
 */
struct linkList {
	struct listNode	*head;
	struct listNode *lastaccess;
	struct listNode	*tail;
	int		count;
};

/**
 * matchNodeFun -
 */
typedef bool (*matchNodeFun)(struct listNode *n, void *para);

/**
 * cleanNodeFun -
 */
typedef bool (*cleanNodeFun)(struct listNode *n, void *para);

#define node_data(node, type)		((type)((node)->data))

/**
 * is_empty -
 * @list:
 */
static inline bool is_empty(struct linkList *list)
{
	return list->head == NULL;
}

/**
 * get_first_node -
 */
static inline struct listNode *get_first_node(struct linkList *list)
{
	return list->head;
}

/**
 * get_next_node -
 * @node:
 */
static inline struct listNode *get_next_node(struct listNode *node)
{
	return node->next;
}

/**
 * create_listnode -
 * @dataSize:
 */
extern struct listNode *create_listnode(int dataSize);

/**
 * make_listnode -
 * @data:
 */
extern struct listNode *make_listnode(void *data);

/**
 * delete_listnode -
 * @node:
 * @fn:
 * @fn_param:
 */
extern void delete_listnode(struct listNode *node, cleanNodeFun fn, void *fn_param);

/**
 * linkList_init -
 * @list:
 */
extern void linkList_init(struct linkList *list);

/**
 * linkList_destroy -
 * @list:
 * @fn:
 * @fn_param:
 */
extern int linkList_destroy(struct linkList *list, cleanNodeFun fn, void *fn_param);


/**
 * list_insert_head -
 * @list:
 * @node:
 */
extern void list_insert_head(struct linkList *list, struct listNode *node);

/**
 * list_insert_tail -
 * @list:
 * @node:
 */
extern void list_insert_tail(struct linkList *list, struct listNode *node);

/**
 * list_find_node -
 * @list:
 * @fn:
 * @fn_param:
 */
extern struct listNode *list_find_node(struct linkList *list, struct listNode *s_node,
				       matchNodeFun fn, void *fn_param);

/**
 * list_pick_node -
 * @list:
 * @node:
 */
extern int list_pick_node(struct linkList *list, struct listNode *node);

/**
 * list_pick_head -
 * @list:
 */
extern struct listNode *list_pick_head(struct linkList *list);

/**
 * interlist_move_node -
 * @list2:
 * @list1:
 * @node:
 *
 * Pick up node form list1, insert to list2 tail.
 */
extern int interlist_move_node(struct linkList *list2, struct linkList *list1,
			       struct listNode *node);

/**
 * list_move_tail-
 * @list:
 * @node:
 */
extern int list_move_tail(struct linkList *list, struct listNode *node);

#endif /* __LINKLIST_H__ */
