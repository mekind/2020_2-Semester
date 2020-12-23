/*
 *  bpt.c
 */
#define Version "1.14"

#include "bpt.h"

 // GLOBALS.

extern Header_page* Header;
ptrnode queue; //for print

// FUNCTION DEFINITIONS.

// OUTPUT AND UTILITIES

/* Copyright and license notice to user at startup.
 */
void license_notice(void) {
	printf("bpt version %s -- Copyright (C) 2010  Amittai Aviram "
		"http://www.amittai.com\n", Version);
	printf("This program comes with ABSOLUTELY NO WARRANTY; for details "
		"type `show w'.\n"
		"This is free software, and you are welcome to redistribute it\n"
		"under certain conditions; type `show c' for details.\n\n");
}

/* First message to the user.
 */
void usage_1(void) {
	printf("Following Silberschatz, Korth, Sidarshan, Database Concepts, "
		"5th ed.\n\n"
		"To build a B+ tree of a different order, start again and enter "
		"the order\n"
		"as an integer argument:  bpt <order>  ");
}


/* Second message to the user.
 */
void usage_2(void) {
	printf("Enter any of the following commands after the prompt > :\n"
		"\ti <key> <value>  -- Insert <k> <(an integer) as both key and value).\n"
		"\tf <key>  -- Find the value under key <k>.\n"
		"\tp <page> -- Print the <page> page.\n"
		"\td <key>  -- Delete key <k> and its associated value.\n"
		"\tt -- Print the B+ tree.\n"
		"\tl -- Print leaves of the B+ tree.\n"
		"\tq -- Quit. (Or use Ctl-D.)\n");
}

void print_header() {
	printf(" Header info : \n Free : %" PRIu64 "\n Root: %" PRIu64 "\nNUm: %" PRIu64 "\n", Header->Free_page, Header->Root_page, Header->Number_of_pages);
}

void print_page(pagenum_t p) {
	page_t tmp;
	file_read_page(p, &tmp);
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


int path_to_root(pagenum_t pagenum) {
	int length = 0;

	pagenum_t tmp = pagenum;
	page_t c;
	file_read_page(pagenum, &c);

	while (tmp != Header->Root_page) {
		tmp = c.Parent;
		file_read_page(tmp, &c);
		length++;
	}

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

void print_tree() {
	int i = 0;
	int rank = 0;
	int new_rank = 0;
	
	if (Header->Root_page == NO_PAGE) {
		printf("Empty tree.\n");
		return;
	}
	
	enqueue(Header->Root_page);
	
	page_t n;
	while (queue != NULL) {
		pagenum_t now = dequeue();

		file_read_page(now, &n);

		if (n.Parent != NO_PAGE) {
			new_rank = path_to_root(now);
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

void print_leaves() {
	int i = 0;

	if (Header->Root_page == NO_PAGE) {
		printf("Empty tree.\n");
		return;
	}

	pagenum_t c_num = Header->Root_page;
	page_t c;
	file_read_page(c_num, &c);

	while (!c.Is_Leaf) {
		c_num = c.Next_Page;
		file_read_page(c_num, &c);
	}

	while (c_num) {
		for (i = 0; i < c.Num_Keys; i++) printf("%lld %s ", c.Records[i].key, c.Records[i].value);
		printf("parent : %" PRIu64 " now : %" PRIu64 " | ", c.Parent, c_num);
		c_num = c.Next_Page;
		file_read_page(c_num, &c);
	}
	printf("\n");
}


pagenum_t db_find_leaf(my_key_t key) {
	int i = 0;
	pagenum_t c = Header->Root_page;

	if (!c) return NO_PAGE; // Root 없음 

	page_t tmp;
	file_read_page(c, &tmp);
	while (!tmp.Is_Leaf) {
		i = 0;
		while (i < tmp.Num_Keys) {
			if (key >= tmp.Indexes[i].key) i++;
			else break;
		}
		if (i == 0) c = tmp.Next_Page;
		else c = tmp.Indexes[i - 1].Next_Page;
		file_read_page(c, &tmp);
	}

	return c;
}


int db_find(my_key_t key, char * ret_val) {
	int i = 0;
	pagenum_t c = db_find_leaf(key);
	if (c == NO_PAGE) return FAIL;

	page_t tmp;
	file_read_page(c, &tmp);

	for (i = 0; i < tmp.Num_Keys; i++)
		if (tmp.Records[i].key == key) break;

	if (i < tmp.Num_Keys && tmp.Records[i].key == key) {
		strcpy(ret_val, tmp.Records[i].value);
		return SUCCESS;
	}
	else 
		return FAIL;
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


int get_my_index(page_t * parent, pagenum_t target) {
	int idx = 0;
	while (idx < parent->Num_Keys &&
		parent->Indexes[idx].Next_Page != target)
		idx++;

	if (target == parent->Next_Page) idx = -1;

	return idx;
}


int insert_into_leaf(pagenum_t leaf_num, page_t leaf, my_key_t key, char* value) { // leaf free 해야됨
	int i, insertion_point;

	insertion_point = 0;
	while (insertion_point < leaf.Num_Keys && leaf.Records[insertion_point].key < key)
		insertion_point++;

	for (i = leaf.Num_Keys; i > insertion_point; i--) {
		leaf.Records[i].key = leaf.Records[i - 1].key;
		strcpy(leaf.Records[i].value, leaf.Records[i - 1].value);
	}
	leaf.Records[insertion_point].key = key;
	strcpy(leaf.Records[insertion_point].value, value);
	leaf.Num_Keys++;

	file_write_page(leaf_num, &leaf);

	return SUCCESS;
}


int insert_into_leaf_after_splitting(pagenum_t leaf_num, page_t leaf, my_key_t key, char * value) { //leaf free 해야됨
	page_t new_leaf;
	memset(&new_leaf, 0, sizeof(page_t));
	new_leaf.Is_Leaf = 1;
	pagenum_t new_leaf_num = file_alloc_page();

	my_key_t temp_keys[LEAF_ORDER + 3];
	char temp_values[LEAF_ORDER + 3][VALUE_SIZE];

	memset(temp_values, 0, sizeof(temp_values));

	int insertion_index, split, i, j;
	my_key_t new_key;

	insertion_index = 0;
	while (insertion_index < LEAF_ORDER - 1 && leaf.Records[insertion_index].key < key)
		insertion_index++;

	for (i = 0, j = 0; i < leaf.Num_Keys; i++, j++) {
		if (j == insertion_index) j++;
		temp_keys[j] = leaf.Records[i].key;
		strcpy(temp_values[j], leaf.Records[i].value);
	}

	temp_keys[insertion_index] = key;
	strcpy(temp_values[insertion_index], value);
	leaf.Num_Keys = 0;

	split = cut(LEAF_ORDER - 1);

	for (i = 0; i < split; i++) {
		strcpy(leaf.Records[i].value, temp_values[i]);
		leaf.Records[i].key = temp_keys[i];
		leaf.Num_Keys++;
	}

	for (i = split, j = 0; i < LEAF_ORDER; i++, j++) {
		strcpy(new_leaf.Records[j].value, temp_values[i]);
		new_leaf.Records[j].key = temp_keys[i];
		new_leaf.Num_Keys++;
	}


	new_leaf.Next_Page = leaf.Next_Page;
	leaf.Next_Page = new_leaf_num;
	new_leaf.Parent = leaf.Parent;
	new_key = new_leaf.Records[0].key;

	file_write_page(leaf_num, &leaf);
	file_write_page(new_leaf_num, &new_leaf);

	return insert_into_parent(leaf.Parent, leaf_num, new_key, new_leaf_num);
}


int insert_into_page(pagenum_t n_num, page_t n, int left_index, my_key_t key, pagenum_t right_num) {
	int i;

	for (i = n.Num_Keys; i > left_index + 1; i--) {
		n.Indexes[i].key = n.Indexes[i - 1].key;
		n.Indexes[i].Next_Page = n.Indexes[i - 1].Next_Page;
	}
	n.Indexes[left_index + 1].key = key;
	n.Indexes[left_index + 1].Next_Page = right_num;
	n.Num_Keys++;

	file_write_page(n_num, &n);

	return SUCCESS;
}


int insert_into_page_after_splitting(pagenum_t old_page_num, page_t old_page, int left_index, my_key_t key, pagenum_t right_num) {

	page_t new_page;
	memset(&new_page, 0, sizeof(page_t));
	pagenum_t new_page_num = file_alloc_page();

	my_key_t temp_keys[INTERNAL_ORDER + 3];
	pagenum_t temp_next_pages[INTERNAL_ORDER + 3];

	int i, j, split;
	my_key_t k_prime;	

	for (i = 0, j = 0; i < old_page.Num_Keys; i++, j++) {
		if (j == left_index + 1) 
			j++;
		temp_keys[j] = old_page.Indexes[i].key;
		temp_next_pages[j] = old_page.Indexes[i].Next_Page;
	}

	temp_keys[left_index + 1] = key;
	temp_next_pages[left_index + 1] = right_num;
	split = cut(INTERNAL_ORDER);
	
	old_page.Num_Keys = 0;
	for (i = 0; i < split - 1; i++) {
		old_page.Indexes[i].Next_Page = temp_next_pages[i];
		old_page.Indexes[i].key = temp_keys[i];
		old_page.Num_Keys++;
	}

	new_page.Next_Page = temp_next_pages[split - 1];
	file_write_page_parent(new_page.Next_Page, &new_page_num);
	k_prime = temp_keys[split - 1];
	for (i = split , j = 0; i < INTERNAL_ORDER; i++, j++) {
		new_page.Indexes[j].Next_Page = temp_next_pages[i];
		new_page.Indexes[j].key = temp_keys[i];
		file_write_page_parent(new_page.Indexes[j].Next_Page, &new_page_num);
		new_page.Num_Keys++;
	}

	new_page.Parent = old_page.Parent;

	file_write_page(old_page_num, &old_page);
	file_write_page(new_page_num, &new_page);

	return insert_into_parent(old_page.Parent, old_page_num, k_prime, new_page_num);
}



int insert_into_parent(pagenum_t parent_num, pagenum_t left_num, my_key_t key, pagenum_t right_num) {

	int left_index;

	if (parent_num == NO_PAGE)
		return insert_into_new_root(left_num, key, right_num);

	page_t parent;
	file_read_page(parent_num, &parent);

	left_index = get_my_index(&parent, left_num);

	if (parent.Num_Keys < INTERNAL_ORDER - 1)
		return insert_into_page(parent_num, parent, left_index, key, right_num);

	return insert_into_page_after_splitting(parent_num, parent, left_index, key, right_num);
}

int insert_into_new_root(pagenum_t left_num, my_key_t key, pagenum_t right_num) {

	pagenum_t root_num = file_alloc_page();
	page_t root;
	memset(&root, 0, sizeof(page_t));
	root.Num_Keys = 1;
	root.Next_Page = left_num;
	root.Indexes[0].key = key;
	root.Indexes[0].Next_Page = right_num;
	Header->Root_page = root_num;

	file_write_header_page();
	file_write_page_parent(left_num, &root_num);
	file_write_page_parent(right_num, &root_num);
	file_write_page(root_num, &root);

	return SUCCESS;
}


int start_new_tree(my_key_t key, char * value) {
	pagenum_t root_num = file_alloc_page();
	page_t root;
	memset(&root, 0, sizeof(page_t));
	root.Is_Leaf = 1;
	root.Num_Keys = 1;
	root.Records[0].key = key;
	strcpy(root.Records[0].value, value);

	Header->Root_page = root_num;
	file_write_header_page();
	file_write_page(root_num, &root);

	return SUCCESS;
}

int db_insert(my_key_t key, char *value) {

	pagenum_t leaf_num;
	page_t leaf;

	if (db_find(key, value) == SUCCESS)
		return FAIL;

	if (Header->Root_page == NO_PAGE)
		return start_new_tree(key, value);

	leaf_num = db_find_leaf(key);
	file_read_page(leaf_num, &leaf);

	if (leaf.Num_Keys < LEAF_ORDER - 1)
		return insert_into_leaf(leaf_num, leaf, key, value);

	return insert_into_leaf_after_splitting(leaf_num, leaf, key, value);
}




// DELETION.

page_t remove_entry_from_page(pagenum_t page_num, page_t page, my_key_t key) {

	int i;
	i = 0;

	if (page.Is_Leaf) {
		while (page.Records[i].key != key)
			i++;
		for (++i; i < page.Num_Keys; i++) {
			page.Records[i - 1].key = page.Records[i].key;
			strcpy(page.Records[i - 1].value, page.Records[i].value);
		}
	}
	else {
		while (page.Indexes[i].key != key)
			i++;
		for (++i; i < page.Num_Keys; i++) {
			page.Indexes[i - 1].key = page.Indexes[i].key;
			page.Indexes[i - 1].Next_Page = page.Indexes[i].Next_Page;
		}
	}
	page.Num_Keys--;
	
	file_write_page(page_num, &page);

	return page;
}


int adjust_root(pagenum_t root_num, page_t root) {

	pagenum_t new_root_num;

	if (root.Num_Keys) {
		file_write_page(root_num, &root);
		return SUCCESS;
	}

	if (!root.Is_Leaf) {
		new_root_num = root.Next_Page;
		pagenum_t tmp = NO_PAGE;
		file_write_page_parent(new_root_num, &tmp);
		Header->Root_page = new_root_num;
	}
	else
		Header->Root_page = NO_PAGE;

	memset(&root, 0, sizeof(page_t));//
	file_write_page(root_num, &root); // 없어도 될거 같음

	file_free_page(root_num);
	Header->Free_page = root_num;
	file_write_header_page();
	
	return SUCCESS;
}

int coalesce_pages(pagenum_t n_num, page_t n, pagenum_t neighbor_num, 
	page_t neighbor, int n_index, my_key_t k_prime) {

	int i, j, neighbor_insertion_index, n_end, split;
	page_t tmp, parent;
	pagenum_t tmp_num;
	int Is_Split = neighbor.Num_Keys;

	if (n_index == -1) { // 바꾸기  shallow
		tmp = n;
		n = neighbor;
		neighbor = tmp;
		tmp_num = n_num;
		n_num = neighbor_num;
		neighbor_num = tmp_num;
	}
	
	neighbor_insertion_index = neighbor.Num_Keys;

	if (!n.Is_Leaf) { // internal 

		if (Is_Split < INTERNAL_ORDER - 1) { // merge만 

			neighbor.Indexes[neighbor_insertion_index].key = k_prime;
			neighbor.Indexes[neighbor_insertion_index].Next_Page = n.Next_Page;
			file_write_page_parent(n.Next_Page, &neighbor_num);
			neighbor.Num_Keys++;

			for (i = neighbor_insertion_index + 1, j = 0; j < n.Num_Keys; i++, j++) {
				neighbor.Indexes[i].key = n.Indexes[j].key;
				neighbor.Indexes[i].Next_Page = n.Indexes[j].Next_Page;
				file_write_page_parent(neighbor.Indexes[i].Next_Page, &neighbor_num);
				neighbor.Num_Keys++;
			}
			n.Num_Keys = 0;

			file_free_page(n_num);
			Header->Free_page = n_num;
			file_write_header_page();
			file_write_page(neighbor_num, &neighbor);
		}
		else { // merge 후 split 
			my_key_t tmp_key[INTERNAL_ORDER + 3];
			pagenum_t tmp_next[INTERNAL_ORDER + 3];

			for (i = 0; i < neighbor.Num_Keys; i++) {
				tmp_key[i] = neighbor.Indexes[i].key;
				tmp_next[i] = neighbor.Indexes[i].Next_Page;
			}

			tmp_key[neighbor_insertion_index] = k_prime;
			tmp_next[neighbor_insertion_index] = n.Next_Page;

			for (i = 0, j = neighbor_insertion_index + 1; i < n.Num_Keys; i++, j++) {
				tmp_key[j] = n.Indexes[i].key;
				tmp_next[j] = n.Indexes[i].Next_Page;
			}

			split = cut(INTERNAL_ORDER);

			neighbor.Num_Keys = 0;
			n.Num_Keys = 0;


			for (i = 0; i < split - 1; i++) {
				neighbor.Indexes[i].key = tmp_key[i];
				neighbor.Indexes[i].Next_Page = tmp_next[i];
				file_write_page_parent(tmp_next[i], &neighbor_num);
				neighbor.Num_Keys++;
			}
			n.Next_Page = tmp_next[split - 1];
			file_write_page_parent(n.Next_Page, &n_num);
			my_key_t change_k_prime = tmp_key[split - 1];

			for (i = split, j = 0; i < INTERNAL_ORDER; i++, j++) {
				n.Indexes[j].key = tmp_key[i];
				n.Indexes[j].Next_Page = tmp_next[i];
				file_write_page_parent(tmp_next[i], &n_num);
				n.Num_Keys++;
			}

			file_read_page(n.Parent, &parent);

			for (i = 0; i < parent.Num_Keys; i++)
				if (parent.Indexes[i].key == k_prime) break;

			parent.Indexes[n_index].key = change_k_prime;

			file_write_page(n.Parent, &parent);
			file_write_page(n_num, &n);
			file_write_page(neighbor_num, &neighbor);
			return SUCCESS;
		}
	}

	else { // leaf 일 때 
		 // 무조건 merge 가능
		neighbor.Next_Page = n.Next_Page;

		for (i = neighbor_insertion_index, j = 0; j < n.Num_Keys; i++, j++) {
			neighbor.Records[i].key = n.Records[i].key;
			strcpy(neighbor.Records[i].value, n.Records[i].value);
			neighbor.Num_Keys++;
		}

		n.Num_Keys = 0;
		file_free_page(n_num);
		Header->Free_page = n_num;
		file_write_header_page();
		file_write_page(neighbor_num, &neighbor);
	}

	return delete_entry(n.Parent, k_prime);
}


int delete_entry(pagenum_t now_page_num, my_key_t key) {

	page_t now_page;
	file_read_page(now_page_num, &now_page);

	page_t neighbor;
	pagenum_t neighbor_num;
	int my_index, neighbor_index;
	int k_prime_index;
	my_key_t k_prime;

	now_page = remove_entry_from_page(now_page_num, now_page, key);

	if (now_page_num == Header->Root_page)
		return adjust_root(now_page_num, now_page);

	if (now_page.Num_Keys > 0) 
		return SUCCESS;
		

	//Delay merge
	page_t parent;
	file_read_page(now_page.Parent, &parent);

	my_index = get_my_index(&parent, now_page_num);

	k_prime_index = my_index == -1 ? 0 : my_index;
	k_prime = parent.Indexes[k_prime_index].key;

	neighbor_index = my_index == -1 ? 0 : my_index - 1;
	neighbor_num = neighbor_index == -1 ? parent.Next_Page : parent.Indexes[neighbor_index].Next_Page;
	file_read_page(neighbor_num, &neighbor);
	
	return coalesce_pages(now_page_num, now_page, neighbor_num, neighbor, my_index, k_prime);
}


int db_delete(my_key_t key) {

	int ret = FAIL;
	char ret_val[VALUE_SIZE];

	pagenum_t leaf_num = db_find_leaf(key);

	if (leaf_num != NO_PAGE && db_find(key, ret_val) == SUCCESS) 
		return delete_entry(leaf_num, key);

	return ret;
}

//Project2

int open_table_in_memory(char *pathname) {
	int ret = open_table(pathname);

	if (ret == -1)
		return ret;

	Header = (Header_page*)malloc(sizeof(Header_page));
	memset(Header, 0, sizeof(Header));

	file_read_header_page();

	if (Header->Number_of_pages == 0) {
		printf("=============NEW HEADER=============\n");
		memset(Header, 0, sizeof(Header_page));
		Header->Number_of_pages = 1;
		file_write_header_page();
	}

	printf("============= HEADER =============\n");
	printf("%" PRIu64 "      %" PRIu64 "          %" PRIu64 "\n", Header->Number_of_pages, Header->Free_page, Header->Root_page);

	return ret;
}

int close_table_in_memory() {
	file_write_header_page();
	free(Header);
	close_table();
}

