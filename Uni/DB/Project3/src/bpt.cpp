/*
 *  bpt.c
 */
#define Version "1.14"

#include "bpt.h"
#include<queue>

#ifndef FAIL
#define FAIL 1
#define SUCCESS 0
#endif

extern frame_t *Buffer;
extern table_t Table;//for print
ptrnode queue; //for print

// FUNCTION DEFINITIONS.

int path_to_root(int table_id, pagenum_t pagenum) {
	int length = 0;

	pagenum_t tmp = pagenum;
	page_t c;

	file_read_page(Table.fd[table_id], pagenum, &c);
	fnum_t header = buffer_read_frame(table_id, 0);

	while (tmp != Buffer[header].hpage.Root_page) {
		tmp = c.Parent;
		file_read_page(Table.fd[table_id], tmp, &c);
		length++;
	}

	buffer_unpin(header);
	return length;
}

void enqueue(pagenum_t pnum) {
	ptrnode c = queue;
	ptrnode new_c;

	new_c = (ptrnode)malloc(sizeof(node));
	new_c->key = pnum;
	new_c->next = NULL;

	if (c == NULL) {
		queue = new_c;
		return;
	}
	while (c->next != NULL)
		c = c->next;

	c->next = new_c;
}


pagenum_t dequeue() {
	ptrnode tmp = queue;
	pagenum_t ret = queue->key;
	queue = queue->next;

	free(tmp);
	tmp = NULL;
	return ret;
}

void print_tree(int table_id) {
	close_table(table_id);
	open_table(Table.pathname[table_id]);
	int i = 0;
	int rank = 0;
	int new_rank = 0;

	fnum_t header = buffer_read_frame(table_id, 0);

	if (Buffer[header].hpage.Root_page == NO_PAGE) {
		printf("Empty tree.\n");
		buffer_unpin(header);
		return;
	}

	enqueue(Buffer[header].hpage.Root_page);
	buffer_unpin(header);
	page_t n;
	while (queue != NULL) {
		pagenum_t now = dequeue();

		file_read_page(Table.fd[table_id], now, &n);

		if (n.Parent != NO_PAGE) {
			new_rank = path_to_root(table_id, now);
			if (new_rank != rank) {
				rank = new_rank;
				printf("\n");
			}
		}

		if (n.Is_Leaf)
		{
			for (i = 0; i < n.Num_Keys; i++)
				printf("%lld ", n.Records[i].key);
		}
		else
		{
			enqueue(n.Next_Page);
			for (i = 0; i < n.Num_Keys; i++) {
				printf("%lld ", n.Indexes[i].key);
				enqueue(n.Indexes[i].Next_Page);
			}
		}
		printf("P : %" PRIu64 " NP: %" PRIu64 "", n.Parent, n.Next_Page);
		printf(" NK : %d now : %" PRIu64 " ", n.Num_Keys, now);
		printf("| ");
	}

	queue = NULL;
	printf("\n");
}

void print_leaves(int table_id) {
	close_table(table_id);
	open_table(Table.pathname[table_id]);
	int i = 0;
	fnum_t header = buffer_read_frame(table_id, 0);
	if (Buffer[header].hpage.Root_page == NO_PAGE) {
		printf("Empty tree.\n");
		buffer_unpin(header);
		return;
	}

	pagenum_t c_num = Buffer[header].hpage.Root_page;
	buffer_unpin(header);

	page_t c;
	file_read_page(Table.fd[table_id], c_num, &c);

	while (!c.Is_Leaf) {
		c_num = c.Next_Page;
		file_read_page(Table.fd[table_id], c_num, &c);
	}

	while (c_num) {
		for (i = 0; i < c.Num_Keys; i++) printf("%lld %s ", c.Records[i].key, c.Records[i].value);
		printf("parent : %" PRIu64 " now : %" PRIu64 " | ", c.Parent, c_num);
		c_num = c.Next_Page;
		file_read_page(Table.fd[table_id], c_num, &c);
	}
	printf("\n");
}


void print_page(int table_id, pagenum_t pnum) {
	page_t tmp;
	Header_page header;
	if (pnum) {
		file_read_page(Table.fd[table_id], pnum, &tmp);

		printf("parent : %" PRIu64 " \n", tmp.Parent);
		printf("is leaf: %d \n", tmp.Is_Leaf);
		printf("next : %" PRIu64 "\n", tmp.Next_Page);
		if (tmp.Is_Leaf) {
			for (int i = 0; i < tmp.Num_Keys; i++) printf("key: %lld value: %s\n", tmp.Records[i].key, tmp.Records[i].value);
		}
		else {
			for (int i = 0; i < tmp.Num_Keys; i++) printf("key: %lld child: %" PRIu64 "\n", tmp.Indexes[i].key, tmp.Indexes[i].Next_Page);
		}
	}
	else { //header
		file_read_header_page(Table.fd[table_id], &header);
		
		printf("Header info : \nFree : %" PRIu64 "\nRoot: %" PRIu64 "\nNum: %" PRIu64 "\n", header.Free_page, header.Root_page, header.Number_of_pages);
	}
}

void print_frames(){
	int i;
	printf(" Frame info : \nSize: %d\nNow: %d\nhead: %d tail: %d\n",Buffer[0].size, Buffer[0].now, Buffer[0].next, Buffer[0].prev);

	printf(" ==============================\n");
	for (i = 1; i <= Buffer[0].now; i++)
		printf("pnum %llu tid: %d dirty: %d pinned: %d fnum: %d |  ", Buffer[i].pnum, Buffer[i].table_id, Buffer[i].is_dirty, Buffer[i].is_pinned, i);

	printf("\n");
}

void print_frame(int fnum) {
	printf("fnum : %d\n", fnum);

	if (Buffer[fnum].pnum) {

		printf("parent : %" PRIu64 " \n", Buffer[fnum].page.Parent);
		printf("is leaf: %d \n", Buffer[fnum].page.Is_Leaf);
		printf("next : %" PRIu64 "\n", Buffer[fnum].page.Next_Page);
		printf("keys : %d \n", Buffer[fnum].page.Num_Keys);
		if (Buffer[fnum].page.Is_Leaf) {
			for (int i = 0; i < Buffer[fnum].page.Num_Keys; i++) printf("key: %lld value: %s\n", Buffer[fnum].page.Records[i].key, Buffer[fnum].page.Records[i].value);
		}
		else {
			for (int i = 0; i < Buffer[fnum].page.Num_Keys; i++) printf("key: %lld child: %" PRIu64 "\n", Buffer[fnum].page.Indexes[i].key, Buffer[fnum].page.Indexes[i].Next_Page);
		}
	}
	else { //header
		printf("Header info : \nFree : %" PRIu64 "\nRoot: %" PRIu64 "\nNum: %" PRIu64 "\n", Buffer[fnum].hpage.Free_page, Buffer[fnum].hpage.Root_page, Buffer[fnum].hpage.Number_of_pages);
	}
}

// Project 3 시작

pagenum_t db_find_leaf(int table_id, my_key_t key) {
	int i = 0;
	fnum_t header = buffer_read_frame(table_id, 0);
	pagenum_t c = Buffer[header].hpage.Root_page;
	buffer_unpin(header);
	if (!c) return NO_PAGE; // Root 없음 
	
	fnum_t tmp = buffer_read_frame(table_id, c);
	
	while (!Buffer[tmp].page.Is_Leaf) {
		i = 0;
		while (i < Buffer[tmp].page.Num_Keys) {
			if (key >= Buffer[tmp].page.Indexes[i].key) i++;
			else break;
		}
		if (i == 0) c = Buffer[tmp].page.Next_Page;
		else c = Buffer[tmp].page.Indexes[i - 1].Next_Page;
		buffer_unpin(tmp);
		tmp = buffer_read_frame(table_id, c);
	}
	
	buffer_unpin(tmp);
	return c;
}

int db_find(int table_id, my_key_t key, char * ret_val) {
	if (!is_open(table_id))
		return FAIL;

	int i = 0;
	pagenum_t c = db_find_leaf(table_id, key);
	if (c == NO_PAGE) return FAIL;

	fnum_t tmp = buffer_read_frame(table_id, c);
	
	for (i = 0; i < Buffer[tmp].page.Num_Keys; i++)
		if (Buffer[tmp].page.Records[i].key == key) break;
	
	if (i < Buffer[tmp].page.Num_Keys && Buffer[tmp].page.Records[i].key == key) {
		strcpy(ret_val, Buffer[tmp].page.Records[i].value);
		--Buffer[tmp].is_pinned;
		return SUCCESS;
	}
	else {
		--Buffer[tmp].is_pinned;
		return FAIL;
	}		
}

/* Finds the appropriate place to
 * split a page_t that is too big into two.
 */
int cut(int length) {
	if (length % 2 == 0)
		return length / 2;
	else
		return length / 2 + 1;
}


// INSERTIONs


int get_my_index(fnum_t parent, pagenum_t target) {
	int idx = 0;
	while (idx < Buffer[parent].page.Num_Keys &&
		Buffer[parent].page.Indexes[idx].Next_Page != target)
		idx++;

	if (target == Buffer[parent].page.Next_Page) idx = -1;

	return idx;
}


int insert_into_leaf(int table_id, pagenum_t leaf_num, my_key_t key, char* value) { // leaf free 해야됨
	int i, insertion_point;
	
	fnum_t leaf = buffer_read_frame(table_id, leaf_num);

	insertion_point = 0;
	while (insertion_point < Buffer[leaf].page.Num_Keys && Buffer[leaf].page.Records[insertion_point].key < key)
		insertion_point++;

	for (i = Buffer[leaf].page.Num_Keys; i > insertion_point; i--) {
		Buffer[leaf].page.Records[i].key = Buffer[leaf].page.Records[i - 1].key;
		strcpy(Buffer[leaf].page.Records[i].value, Buffer[leaf].page.Records[i - 1].value);
	}
	Buffer[leaf].page.Records[insertion_point].key = key;
	strcpy(Buffer[leaf].page.Records[insertion_point].value, value);
	Buffer[leaf].page.Num_Keys++;

	buffer_unpin(leaf);
	return SUCCESS;
}


int insert_into_leaf_after_splitting(int table_id, pagenum_t leaf_num, my_key_t key, char * value) {
	fnum_t new_leaf = buffer_alloc_page(table_id);
	fnum_t leaf = buffer_read_frame(table_id, leaf_num);
	memset(&Buffer[new_leaf].page, 0, sizeof(page_t));
	Buffer[new_leaf].page.Is_Leaf = 1;

//	printf("leaf : %d   newleaf: %d\n", leaf, new_leaf);
	my_key_t temp_keys[LEAF_ORDER + 3];
	char temp_values[LEAF_ORDER + 3][VALUE_SIZE];

	memset(temp_values, 0, sizeof(temp_values));

	int insertion_index, split, i, j;
	my_key_t new_key;

	insertion_index = 0;
	while (insertion_index < LEAF_ORDER - 1 && Buffer[leaf].page.Records[insertion_index].key < key)
		insertion_index++;

	for (i = 0, j = 0; i < Buffer[leaf].page.Num_Keys; i++, j++) {
		if (j == insertion_index) j++;
		temp_keys[j] = Buffer[leaf].page.Records[i].key;
		strcpy(temp_values[j], Buffer[leaf].page.Records[i].value);
	}

	temp_keys[insertion_index] = key;
	strcpy(temp_values[insertion_index], value);
	Buffer[leaf].page.Num_Keys = 0;

	split = cut(LEAF_ORDER - 1);

	for (i = 0; i < split; i++) {
		strcpy(Buffer[leaf].page.Records[i].value, temp_values[i]);
		Buffer[leaf].page.Records[i].key = temp_keys[i];
		Buffer[leaf].page.Num_Keys++;
	}

	for (i = split, j = 0; i < LEAF_ORDER; i++, j++) {
		strcpy(Buffer[new_leaf].page.Records[j].value, temp_values[i]);
		Buffer[new_leaf].page.Records[j].key = temp_keys[i];
		Buffer[new_leaf].page.Num_Keys++;
	}


	Buffer[new_leaf].page.Next_Page = Buffer[leaf].page.Next_Page;
	Buffer[leaf].page.Next_Page = Buffer[new_leaf].pnum;
	Buffer[new_leaf].page.Parent = Buffer[leaf].page.Parent;
	new_key = Buffer[new_leaf].page.Records[0].key;
	
	pagenum_t parent_num = Buffer[leaf].page.Parent;
	Buffer[new_leaf].is_dirty = 1;
	Buffer[leaf].is_dirty = 1;
	--Buffer[new_leaf].is_pinned;
	--Buffer[leaf].is_pinned;
	return insert_into_parent(table_id, parent_num, leaf_num, new_key, Buffer[new_leaf].pnum);
}


int insert_into_page(int table_id, pagenum_t n_num, int left_index, my_key_t key, pagenum_t right_num) {
	int i;

	fnum_t n = buffer_read_frame(table_id, n_num);

	for (i = Buffer[n].page.Num_Keys; i > left_index + 1; i--) {
		Buffer[n].page.Indexes[i].key = Buffer[n].page.Indexes[i - 1].key;
		Buffer[n].page.Indexes[i].Next_Page = Buffer[n].page.Indexes[i - 1].Next_Page;
	}
	Buffer[n].page.Indexes[left_index + 1].key = key;
	Buffer[n].page.Indexes[left_index + 1].Next_Page = right_num;
	Buffer[n].page.Num_Keys++;

	buffer_unpin(n);
	return SUCCESS;
}


int insert_into_page_after_splitting(int table_id, pagenum_t old_page_num, int left_index,
	my_key_t key, pagenum_t right_num) {

	fnum_t old_page = buffer_read_frame(table_id, old_page_num);
	fnum_t new_page = buffer_alloc_page(table_id);
	pagenum_t new_page_num = Buffer[new_page].pnum;
	memset(&Buffer[new_page].page, 0, sizeof(page_t)); //  

	my_key_t temp_keys[INTERNAL_ORDER + 3];
	pagenum_t temp_next_pages[INTERNAL_ORDER + 3];

	int i, j, split;
	my_key_t k_prime;	

	for (i = 0, j = 0; i < Buffer[old_page].page.Num_Keys; i++, j++) {
		if (j == left_index + 1) 
			j++;
		temp_keys[j] = Buffer[old_page].page.Indexes[i].key;
		temp_next_pages[j] = Buffer[old_page].page.Indexes[i].Next_Page;
	}

	temp_keys[left_index + 1] = key;
	temp_next_pages[left_index + 1] = right_num;
	split = cut(INTERNAL_ORDER);
	
	Buffer[old_page].page.Num_Keys = 0;
	for (i = 0; i < split - 1; i++) {
		Buffer[old_page].page.Indexes[i].Next_Page = temp_next_pages[i];
		Buffer[old_page].page.Indexes[i].key = temp_keys[i];
		Buffer[old_page].page.Num_Keys++;
	}

	fnum_t tmp;
	Buffer[new_page].page.Next_Page = temp_next_pages[split - 1];
	tmp = buffer_read_frame(table_id, temp_next_pages[split - 1]);
	Buffer[tmp].page.Parent = Buffer[new_page].pnum;
	buffer_unpin(tmp);

	k_prime = temp_keys[split - 1];

	for (i = split , j = 0; i < INTERNAL_ORDER; i++, j++) {
		Buffer[new_page].page.Indexes[j].Next_Page = temp_next_pages[i];
		Buffer[new_page].page.Indexes[j].key = temp_keys[i];

		tmp = buffer_read_frame(table_id, temp_next_pages[i]);
		Buffer[tmp].page.Parent = Buffer[new_page].pnum;
		buffer_unpin(tmp);
		Buffer[new_page].page.Num_Keys++;
	}

	pagenum_t parent_num = Buffer[new_page].page.Parent = Buffer[old_page].page.Parent;
	buffer_unpin(old_page);
	buffer_unpin(new_page);

	return insert_into_parent(table_id, parent_num, old_page_num, k_prime, new_page_num);
}



int insert_into_parent(int table_id, pagenum_t parent_num, pagenum_t left_num, my_key_t key, pagenum_t right_num) {

	int left_index, knum;

	if (parent_num == NO_PAGE)
		return insert_into_new_root(table_id, left_num, key, right_num);

	fnum_t parent = buffer_read_frame(table_id, parent_num);

	left_index = get_my_index(parent, left_num);
	knum = Buffer[parent].page.Num_Keys;
	--Buffer[parent].is_pinned;

	if (knum < INTERNAL_ORDER - 1)
		return insert_into_page(table_id, parent_num, left_index, key, right_num);

	return insert_into_page_after_splitting(table_id, parent_num, left_index, key, right_num);
}

int insert_into_new_root(int table_id, pagenum_t left_num, my_key_t key, pagenum_t right_num) {

	fnum_t left = buffer_read_frame(table_id, left_num);
	fnum_t right = buffer_read_frame(table_id, right_num);
	fnum_t header = buffer_read_frame(table_id, 0);
	fnum_t root = buffer_alloc_page(table_id);

	memset(&Buffer[root].page, 0, sizeof(page_t));
	Buffer[root].page.Num_Keys = 1;
	Buffer[root].page.Next_Page = Buffer[left].pnum;
	Buffer[root].page.Indexes[0].key = key;
	Buffer[root].page.Indexes[0].Next_Page = Buffer[right].pnum;
	
	Buffer[left].page.Parent = Buffer[right].page.Parent = Buffer[root].pnum;
	Buffer[header].hpage.Root_page = Buffer[root].pnum;

	buffer_unpin(left);
	buffer_unpin(right);
	buffer_unpin(root);
	buffer_unpin(header);
	return SUCCESS;
}


int start_new_tree(int table_id, my_key_t key, char * value) {
	fnum_t root = buffer_alloc_page(table_id);
	fnum_t header = buffer_read_frame(table_id, 0);

	//printf(" start new tree %d   %d\n", Buffer[root].is_pinned, Buffer[header].is_pinned);
//	printf("start new root %d head %d \n", root, header);
	memset(&Buffer[root].page, 0, sizeof(page_t));
	Buffer[root].page.Is_Leaf = 1;
	Buffer[root].page.Num_Keys = 1;
	Buffer[root].page.Records[0].key = key;
	strcpy(Buffer[root].page.Records[0].value, value);

	Buffer[header].hpage.Root_page = Buffer[root].pnum;

	buffer_unpin(root);
	buffer_unpin(header);
	return SUCCESS;
}

int db_insert(int table_id, my_key_t key, char *value) {
	if (!is_open(table_id))
		return FAIL;

	pagenum_t leaf_num;
	fnum_t leaf; 

	if (db_find(table_id, key, value) == SUCCESS)
		return FAIL;

	fnum_t header = buffer_read_frame(table_id, 0);
	
	if (Buffer[header].hpage.Root_page == NO_PAGE){
		--Buffer[header].is_pinned;
		return start_new_tree(table_id, key, value);
	}

	leaf_num = db_find_leaf(table_id, key);
	leaf = buffer_read_frame(table_id, leaf_num);
	int knum = Buffer[leaf].page.Num_Keys;
	--Buffer[leaf].is_pinned;
	--Buffer[header].is_pinned;
	if (knum < LEAF_ORDER - 1)
		return insert_into_leaf(table_id, leaf_num, key, value);

	return insert_into_leaf_after_splitting(table_id, leaf_num, key, value);
}

// DELETION.

fnum_t remove_entry_from_page(int table_id, fnum_t page, my_key_t key) {

	int i;
	i = 0;

	if (Buffer[page].page.Is_Leaf) {
		while (Buffer[page].page.Records[i].key != key)
			i++;
		for (++i; i < Buffer[page].page.Num_Keys; i++) {
			Buffer[page].page.Records[i - 1].key = Buffer[page].page.Records[i].key;
			strcpy(Buffer[page].page.Records[i - 1].value, Buffer[page].page.Records[i].value);
		}
	}
	else {
		while (Buffer[page].page.Indexes[i].key != key)
			i++;
		for (++i; i < Buffer[page].page.Num_Keys; i++) {
			Buffer[page].page.Indexes[i - 1].key = Buffer[page].page.Indexes[i].key;
			Buffer[page].page.Indexes[i - 1].Next_Page = Buffer[page].page.Indexes[i].Next_Page;
		}
	}
	Buffer[page].page.Num_Keys--;
	return page;
}


int adjust_root(int table_id, fnum_t root) {

	fnum_t new_root;
	pagenum_t new_root_num;

	if (Buffer[root].page.Num_Keys) 
		return SUCCESS;

	fnum_t header = buffer_read_frame(table_id, 0);

	if (!Buffer[root].page.Is_Leaf) {
		new_root_num = Buffer[root].page.Next_Page;
		new_root = buffer_read_frame(table_id, new_root_num);
		Buffer[header].hpage.Root_page = new_root_num;
		Buffer[new_root].page.Parent = 0;
		buffer_unpin(new_root);
	}
	else
		Buffer[header].hpage.Root_page = NO_PAGE;

	memset(&Buffer[root].page, 0, sizeof(page_t));
	Buffer[root].page.Next_Free = Buffer[header].hpage.Free_page;
	Buffer[header].hpage.Free_page = Buffer[root].pnum;

	buffer_unpin(root);
	buffer_unpin(header);
	return SUCCESS;
}

int coalesce_pages(int table_id, pagenum_t n_num, pagenum_t neighbor_num, int n_index, my_key_t k_prime) {

	int i, j, neighbor_insertion_index, n_end, split;
	fnum_t tmp, parent, header, n, neighbor;
	
	
	
	n = buffer_read_frame(table_id, n_num);
	neighbor = buffer_read_frame(table_id, neighbor_num);
	parent = buffer_read_frame(table_id, Buffer[n].page.Parent);

	pagenum_t parent_num = Buffer[parent].pnum;

	int Is_Split = Buffer[neighbor].page.Num_Keys;

	if (n_index == -1) { // 바꾸기  shallow
		tmp = n;
		n = neighbor;
		neighbor = tmp;
	}
	
	neighbor_insertion_index = Buffer[neighbor].page.Num_Keys;

	if (!Buffer[n].page.Is_Leaf) { // internal 

		if (Is_Split < INTERNAL_ORDER - 1) { // merge만 

			Buffer[neighbor].page.Indexes[neighbor_insertion_index].key = k_prime;
			Buffer[neighbor].page.Indexes[neighbor_insertion_index].Next_Page = Buffer[n].page.Next_Page;
			tmp = buffer_read_frame(table_id, Buffer[n].page.Next_Page);
			Buffer[tmp].page.Parent = Buffer[neighbor].pnum;
			buffer_unpin(tmp);
			Buffer[neighbor].page.Num_Keys++;

			for (i = neighbor_insertion_index + 1, j = 0; j < Buffer[n].page.Num_Keys; i++, j++) {
				Buffer[neighbor].page.Indexes[i].key = Buffer[n].page.Indexes[j].key;
				Buffer[neighbor].page.Indexes[i].Next_Page = Buffer[n].page.Indexes[j].Next_Page;

				tmp = buffer_read_frame(table_id, Buffer[neighbor].page.Indexes[i].Next_Page);
				Buffer[tmp].page.Parent = Buffer[neighbor].pnum;
				buffer_unpin(tmp);
				Buffer[neighbor].page.Num_Keys++;
			}
			

			header = buffer_read_frame(table_id, 0);
			memset(&Buffer[n].page, 0, sizeof(page_t));
			Buffer[n].page.Next_Free = Buffer[header].hpage.Free_page;
			Buffer[header].hpage.Free_page = Buffer[n].pnum;
		}
		else { // merge 후 split 
			my_key_t tmp_key[INTERNAL_ORDER + 3];
			pagenum_t tmp_next[INTERNAL_ORDER + 3];

			for (i = 0; i < Buffer[neighbor].page.Num_Keys; i++) {
				tmp_key[i] = Buffer[neighbor].page.Indexes[i].key;
				tmp_next[i] = Buffer[neighbor].page.Indexes[i].Next_Page;
			}

			tmp_key[neighbor_insertion_index] = k_prime;
			tmp_next[neighbor_insertion_index] = Buffer[n].page.Next_Page;

			for (i = 0, j = neighbor_insertion_index + 1; i < Buffer[n].page.Num_Keys; i++, j++) {
				tmp_key[j] = Buffer[n].page.Indexes[i].key;
				tmp_next[j] = Buffer[n].page.Indexes[i].Next_Page;
			}

			split = cut(INTERNAL_ORDER);

			Buffer[neighbor].page.Num_Keys = 0;
			Buffer[n].page.Num_Keys = 0;


			for (i = 0; i < split - 1; i++) {
				Buffer[neighbor].page.Indexes[i].key = tmp_key[i];
				Buffer[neighbor].page.Indexes[i].Next_Page = tmp_next[i];

				Buffer[neighbor].page.Num_Keys++;
			}
			Buffer[n].page.Next_Page = tmp_next[split - 1];
			tmp = buffer_read_frame(table_id, Buffer[n].page.Next_Page);
			Buffer[tmp].page.Parent = Buffer[n].pnum;
			buffer_unpin(tmp);
			my_key_t change_k_prime = tmp_key[split - 1];

			for (i = split, j = 0; i < INTERNAL_ORDER; i++, j++) {
				Buffer[n].page.Indexes[j].key = tmp_key[i];
				Buffer[n].page.Indexes[j].Next_Page = tmp_next[i];
				tmp = buffer_read_frame(table_id, tmp_next[i]);
				Buffer[tmp].page.Parent = Buffer[n].pnum;
				buffer_unpin(tmp);
				Buffer[n].page.Num_Keys++;
			}

			for (i = 0; i < Buffer[parent].page.Num_Keys; i++)
				if (Buffer[parent].page.Indexes[i].key == k_prime) break;

			Buffer[parent].page.Indexes[n_index].key = change_k_prime;

			buffer_unpin(n);
			buffer_unpin(neighbor);
			buffer_unpin(parent);
			return SUCCESS;
		}
	}

	else { // leaf 일 때 
		 // 무조건 merge 가능
		Buffer[neighbor].page.Next_Page = Buffer[n].page.Next_Page;

		for (i = neighbor_insertion_index, j = 0; j < Buffer[n].page.Num_Keys; i++, j++) {
			Buffer[neighbor].page.Records[i].key = Buffer[n].page.Records[i].key;
			strcpy(Buffer[neighbor].page.Records[i].value, Buffer[n].page.Records[i].value);
			Buffer[neighbor].page.Num_Keys++;
		}

		header = buffer_read_frame(table_id, 0);
		memset(&Buffer[n].page, 0, sizeof(page_t));
		Buffer[n].page.Next_Free = Buffer[header].hpage.Free_page;
		Buffer[header].hpage.Free_page = Buffer[n].pnum;
	}

	buffer_unpin(header);
	buffer_unpin(n);
	buffer_unpin(neighbor);
	buffer_unpin(parent);

	return delete_entry(table_id, parent_num, k_prime);
}


int delete_entry(int table_id, pagenum_t now_page_num, my_key_t key) {

	fnum_t now_page = buffer_read_frame(table_id, now_page_num);
	pagenum_t neighbor_num;

	int my_index, neighbor_index;
	int k_prime_index;
	my_key_t k_prime;

	now_page = remove_entry_from_page(table_id, now_page, key);

	if (Buffer[now_page].page.Parent == 0) 
		return adjust_root(table_id, now_page);		

	if (Buffer[now_page].page.Num_Keys > 0) {
		buffer_unpin(now_page);
		return SUCCESS;
	}
		
		

	//Delay merge
	pagenum_t parent_num = Buffer[now_page].page.Parent;
	fnum_t parent = buffer_read_frame(table_id, parent_num);
	
	my_index = get_my_index(parent, now_page_num);

	k_prime_index = my_index == -1 ? 0 : my_index;
	k_prime = Buffer[parent].page.Indexes[k_prime_index].key;

	neighbor_index = my_index == -1 ? 0 : my_index - 1;
	neighbor_num = neighbor_index == -1 ? Buffer[parent].page.Next_Page : Buffer[parent].page.Indexes[neighbor_index].Next_Page;	
	
	buffer_unpin(parent);
	buffer_unpin(now_page);


	return coalesce_pages(table_id, now_page_num, neighbor_num, my_index, k_prime);
}


int db_delete(int table_id, my_key_t key) {
	if (!is_open(table_id))
		return FAIL;

	int ret = FAIL;
	char ret_val[VALUE_SIZE];

	pagenum_t leaf_num = db_find_leaf(table_id, key);

	if (leaf_num != NO_PAGE && db_find(table_id, key, ret_val) == SUCCESS) 
		return delete_entry(table_id, leaf_num, key);

	return ret;
}

