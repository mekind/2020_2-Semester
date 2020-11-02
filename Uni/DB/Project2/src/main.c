#include "bpt.h"

// MAIN

int main( int argc, char ** argv ) {

	my_key_t input;
	pagenum_t input2;
    char instruction;
    char license_part;
	char pathname[100];  // Project2
	char value[120];
	
    license_notice();
    usage_1();  
    usage_2();
	
    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd': // Project2  delete <key>
			scanf("%lld", &input);
			db_delete(input);
				//printf("Deletion %lld Success.\n", input);
			//else 
			//	printf("Deletion %lld Fail.\n", input);
            break;
        case 'i': // Project2  insert <key> <value>
			scanf("%lld %s", &input, value);
			db_insert(input, value);
            break;
		case 'f':
			scanf("%lld", &input);
			if (db_find(input, value) == SUCCESS) printf("%s find\n", value);
			else printf("fail\n");
			break;
        case 'q':
            while (getchar() != (int)'\n');
			close_table_in_memory();
            return EXIT_SUCCESS;
            break;
        case 't':
            print_tree();
            break;
		case 'l':
			print_leaves();
			break;
		case 'o': // Project2  open 
			scanf("%s", pathname);
			if (open_table_in_memory(pathname) == -1) {
				perror("open_table_in_memory");
			}
			else printf("Successfully open table %s\n", pathname);
			break;
		case 'h':
			print_header();
			break;
		case 'p':
			scanf("%llu", &input2);
			print_page(input2);
			break;
        default:
            //usage_2();
            break;
        }
        while (getchar() != (int)'\n');
//        printf("> ");
    }
    printf("\n");
	close_table_in_memory();
    return EXIT_SUCCESS;
}
