#define _CRT_SECURE_NO_WARNINGS
#include "bpt.h"
#include<vector>
#include<algorithm>
#include<random>
#include<complex>
#include<time.h>
using namespace std;

// MAIN

#define LIMIT 10000000000000000
#define MOD 1000000007
#define MXH 21
#define SIZE 50005
#define RECORD_VALUE_SIZE 120
const double PI = acos(-1);
typedef long long ll;
typedef long long sll;
typedef unsigned long long ull;
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;
typedef complex<double> cpx;
typedef vector<cpx> vec;
vector<int>v;

int menu_select(const char * str)
{
	if (!strcmp(str, "open"))							return 1;
	else if (!strcmp(str, "insert_rand"))		return 2;
	else if (!strcmp(str, "find"))					return 3;
	else if (!strcmp(str, "delete_rand"))		return 4;
	else if (!strcmp(str, "print_all"))				return 5;
	else if (!strcmp(str, "print_leaf"))			return 6;
	else if (!strcmp(str, "delete"))				return 8;
	else if (!strcmp(str, "frame"))				return 7;
	else if (!strcmp(str, "frame_info"))			return 11;
	else if (!strcmp(str, "close"))		return 10;
	else if (!strcmp(str, "quit"))					return 0;
	else if (!strcmp(str, "page_status"))		return 12;
	else if (!strcmp(str, "insert"))				return 9;
	else return -1;
}

int main() {
	char command[20] = { 0, };
	char input_value[120] = { 0, };
	sll input_key = 0;
	int buf_num;

	printf("		###########################################\n");
	printf("		############# On-Disk B+ Tree #############\n");
	printf("		###########################################\n");
	printf("		1.	open <pathname>				: Open Table file\n");
	printf("		2.	insert <key> <value> <table_id>		: Insert Record\n");
	printf("		3.	insert_rand <num> <table_id> 		: Insert Random Numbers\n");
	printf("		4.	find <key> <table_id>			: Find record with key\n");
	printf("		5.	delete <key> <table_id>			: Find with key and Delete corresponding record\n");
	printf("		6.	delete_rand <num> <table_id>		: Find with key and Delete ALL corresponding record\n");
	printf("		7.	print_all <pathname>			: Print all nodes\n");
	printf("		8.	print_leaf <pathname>			: Print all leaves\n");
	printf("		9.	frame					: Print all frames\n");
	printf("		10.	frame_info <num>			: Print frame_status\n");
	printf("		11.	page_status <pathname> <num>		: Print the page status\n");
	printf("		12.	quit					: End program\n\n");
	printf("		How many buffers are there? : ");
	
	scanf("%d", &buf_num);

	init_db(buf_num);

	for (;;)
	{
		printf("> ");
		scanf(" %s", command);


		vector<int>value;

		switch (menu_select(command))
		{
		case 1: // Initialization :: Open table
		{
			scanf(" %s", input_value);
			int td = open_table(input_value);

			if (td != -1)
			{
				printf(":: Initialization complete :: \n");
//				printf(":: Num_pages = %llu ::\n", now_header->num_pages);
			}
			else
			{
				perror(":: Initialization Failed! :");
			}
			break;
		}
		case 2: // Insertion :: insert input_key & input_value
		{
			printf(":: Insertion :: \n");
			char pathname[20];
			int table_id;
			scanf("%llu %d", &input_key, &table_id);


			for (int i = 1; i <= input_key; i++) {
				value.push_back(i);
			}

			random_device rd;
			mt19937 gen(rd());
			shuffle(value.begin(), value.end(), gen);

			char rt[120] = { 0, 0, };
			rt[0] = 'a';

			clock_t start_ = clock();
			for (auto i : value) {
				db_insert(table_id, i - input_key / 2, rt);
			}
			clock_t end_ = clock();

			printf(":: Insertion Success ::\n");
			printf("Time : %lf sec\n", (double)(end_ - start_) / CLOCKS_PER_SEC);
			break;
		}
		case 3: // Find Data with key
		{
			printf(":: Find data ::\n");
			char pathname[20];
			int table_id;
			scanf("%lld %d", &input_key, &table_id);


			char ret_val[RECORD_VALUE_SIZE];

			int result = db_find(table_id, input_key, ret_val);
			if (result)
			{
				printf(":: Search Failed ::\n");
				printf("key	 : %lld\n", input_key);
				printf("\n");
			}
			else
			{
				printf(":: Search Success ::\n");
				printf("key	 : %lld\n", input_key);
				printf("value	 : %s", ret_val);
				printf("\n");
			}
			
			break;
		}
		case 4: // Deletion :: Find with key and Delete corresponding record
		{
			printf(":: Deletion :: \n");
			int table_id;
			scanf("%lld %d", &input_key, &table_id);

			for (int i = 1; i <= input_key; i++) {
				value.push_back(i);
			}

			random_device rd;
			mt19937 gen(rd());
			shuffle(value.begin(), value.end(), gen);

			clock_t start_ = clock();
			for (auto i : value) {
				db_delete(table_id, i - input_key / 2);
			}
			clock_t end_ = clock();
			printf(":: Deletion Success ::\n");
			printf("Time : %lf sec\n", (double)(end_ - start_) / CLOCKS_PER_SEC);

			break;
		}
		case 5: // Print all nodes
		{
			printf(":: Print all nodes ::\n");
			char pathname[20];
			scanf("%s", pathname);
			int table_id = open_table(pathname);
			printf(" table id: %d \n",table_id);
			print_tree(table_id);
			break;
		}
		case 6: // Print all leaf nodes
		{
			printf(":: Print all leaves ::\n");
			char pathname[20];
			scanf("%s", pathname);
			int table_id = open_table(pathname);
			print_leaves(table_id);
			break;
		}
		case 8: // Deletion :: Find with key and Delete corresponding record
		{
			printf(":: Deletion :: \n");
			int table_id;
			scanf("%lld %d", &input_key, &table_id);

			int result = db_delete(table_id, input_key);
			if (result)
			{
				printf(":: Deletion Failed ::\n");
				printf("key	 : %lld\n", input_key);
			}
			else
			{
				printf(":: Deletion Success ::\n");
				printf("key	 : %lld\n", input_key);
			}
		//	db_delete(input_key) 실행, 성공 실패여부 if문으로 갈라서 출력

			break;
		}		
		case 7: // Print all frames
		{
			printf(":: Print all frames ::\n");
			print_frames();
			break;
		}
		case 9: // Insertion :: insert input_key & input_value
		{
			printf(":: Insertion :: \n");
			char pathname[20];
			int table_id;
			scanf("%lld %s %d", &input_key, input_value, &table_id);

			int result = db_insert(table_id, input_key, input_value);

			if (result)
			{
				printf(":: Insertion failed ::\n");
				printf("key	 : %lld\n", input_key);
				printf("value	 : %s", input_value);
				printf("\n");
			}
			else
			{
				printf(":: Insertion Success ::\n");


				printf("key	 : %lld\n", input_key);
				printf("value	 : %s", input_value);
				printf("\n");

			}
			break;
		}
		case 10: // close
		{
			char pathname[20];
			printf(":: Close table ::\n");
			scanf("%s", pathname);

			int table = open_table(pathname);
			printf("tid :%d \n",table);
			close_table(table);
			break;
		}
		case 11: // Print a frame_info
		{
			int tmp;
			scanf(" %d", &tmp);
			printf(":: Frame %d ::\n", tmp);
			print_frame(tmp);
			break;
		}
		case 12: // Print a page
		{
			ull tmp;
			int table_id;
			scanf(" %llu %d", &tmp, &table_id);
			printf("::  %llu Page of %d  ::\n", tmp, table_id);

			print_page(table_id, tmp);
			break;
		}
		case 0: // Quit
		{
			shutdown_db();
			return 0;
		}
		case -1: // Wrong Command
		{
			break;
		}
		}
	}

}
