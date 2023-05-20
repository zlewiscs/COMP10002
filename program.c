/* A simplified community detection algorithm:
 *
 * Skeleton code written by Jianzhong Qi, May 2023
 * Edited by: Liang Zhang (1365688)
 * algorithms are fun
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* extra library I deemed useful to include */
#include <ctype.h>
#include <string.h>

#define STAGE_NUM_ONE 1						  /* stage numbers */
#define STAGE_NUM_TWO 2
#define STAGE_NUM_THREE 3
#define STAGE_NUM_FOUR 4
#define STAGE_HEADER "Stage %d\n==========\n" /* stage header format string */

/* Define my own constants */
#define MAX_HASHTAG 10 						  /* The data contain up to 10 hashtags per user */
#define MAX_USER 50 						  /* The dataset contain maximum 50 users */
#define MAX_LEN 20 							  /* The max number of letters per hashtag is 20*/

typedef struct {
	/* add your user_t struct definition */

	int user_num; // user number

	int year; // year started

	char **hashtags; // array of hashtags
	
	int hashtag_count; // number of hashtags to be derived from hashtags

	int cfriend_count; // number of close friends (added for stage 4)
	
	int is_core; // status as core user

} user_t;

typedef char* data_t;							  /* to be modified for Stage 4 */

/* linked list type definitions below, from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c 
*/
typedef struct node node_t;

struct node {
	data_t data;
	node_t *next;
};

typedef struct {
	node_t *head;
	node_t *foot;
} list_t;

/* types defined by myself */
typedef struct {
	int core_user_num;
	int *close_friend_nums;
	int close_friend_count;
	list_t *unique_hashtags;
} community_t;

/****************************************************************/

/* function prototypes */
list_t *make_empty_list(void);
void free_list(list_t *list);
void print_list(list_t *list);
list_t *insert_unique_in_order(list_t *list, data_t value);

void print_stage_header(int stage_num);

void stage_one(user_t *users, int *user_count, int *max_hashtag_user_idx);
void stage_two(user_t *users, int *user_count, int **matrix);
void stage_three(user_t *users, int **friendship_m, int *user_count, double **soc_matrix);
void stage_four(user_t *users, double *ths, int *thc, double **soc_matrix, int *user_count);

/* add your own function prototypes here */
void read_users(user_t *users, int *user_count);
void most_hash_user(user_t *users, int *user_count, int *max_hashtag_user_idx);
int** create_matrix(int *user_count);
double s_o_c(int u1[], int u2[], int u1_num, int u2_num, int *user_count);
double** create_soc_matrix(int **friendship_m, int *user_count);
void free_users(user_t *users, int *user_count);
void free_matrix(int **matrix, int *user_num);
void free_double_matrix(double **soc_matrix, int *user_count);
void print_double_matrix(double **soc_matrix, int *user_count);
void free_communities(community_t *communities, int *core_users_count);
int is_core(user_t user, int *thc);
void fill_unique_hashtags(user_t *users, community_t *communities, int *core_users_count);
void fill_close_friends(community_t *communities, user_t *users, int *core_users_count,
double **soc_matrix, double *ths, int *user_count);
void stage_4_output(community_t *communities, int *core_users_count, user_t *users);
void count_close_friends(user_t *users, double **soc_matrix, int *user_count, double *ths);

/****************************************************************/

int
main(int argc, char *argv[]) {
	/* add variables to hold the input data */
	user_t *users = malloc(MAX_USER * sizeof(user_t));
	int user_count = 0;
	int max_hashtag_user_idx = 0;
	double ths;
	int thc;
	assert(users);
	/* stage 1: read user profiles */
	stage_one(users, &user_count, &max_hashtag_user_idx); 
	
	/* calculate the soc between u0 and u1 */
	int **matrix = create_matrix(&user_count);
	stage_two(users, &user_count, matrix);

	/* stage 3: compute the strength of connection for all user pairs */
	double **soc_matrix = create_soc_matrix(matrix, &user_count);
	stage_three(users, matrix, &user_count, soc_matrix);
	

	/* stage 4: detect communities and topics of interest */
	scanf("%lf %d", &ths, &thc);
	stage_four(users, &ths, &thc, soc_matrix, &user_count);

	/* free memories */
	free_users(users, &user_count);
	free_matrix(matrix, &user_count);
	free_double_matrix(soc_matrix, &user_count);
	
	/* all done; take some rest */
	return 0;
}

/****************************************************************/

/********************* The 4 main stages ***********************/

/* stage 1: read user profiles */
void 
stage_one(user_t *users, int *user_count, int *max_hashtag_user_idx) {
	/* print stage header */
	print_stage_header(STAGE_NUM_ONE);
	read_users(users, user_count);
	most_hash_user(users, user_count, max_hashtag_user_idx);

	/* print the desired output */
	printf("Number of users: %d\n", *user_count);
	printf("u%d has the largest number of hashtags:\n", *max_hashtag_user_idx);
	for(int i = 0; i < users[*max_hashtag_user_idx].hashtag_count; i++){
		if (i == users[*max_hashtag_user_idx].hashtag_count - 1) {
			printf("%s", users[*max_hashtag_user_idx].hashtags[i]);
		} else {
			printf("%s ", users[*max_hashtag_user_idx].hashtags[i]);
		}
		
	}

	printf("\n\n");
}

/* stage 2: compute the strength of connection between u0 and u1 */
void 
stage_two(user_t *users, int *user_count, int **matrix) {
	/* print stage header */
	print_stage_header(STAGE_NUM_TWO);
	double soc = s_o_c(matrix[0], matrix[1], users[0].user_num, users[1].user_num, user_count);
	printf("Strength of connection between u0 and u1: %4.2f", soc);

	printf("\n\n");
}

/* stage 3: compute the strength of connection for all user pairs */
void 
stage_three(user_t *users, int **friendship_m, int *user_count, double **soc_matrix) {
	/* print stage header */
	print_stage_header(STAGE_NUM_THREE);
	for(int i = 0; i < *user_count - 1; i++) {
		for(int j = i + 1; j < *user_count; j++) {
			soc_matrix[i][j] = s_o_c(friendship_m[i], friendship_m[j], 
			users[i].user_num, users[j].user_num, user_count);

			soc_matrix[j][i] = soc_matrix[i][j]; // property of symetrix matrix
			
		}
	}

	print_double_matrix(soc_matrix, user_count);
	printf("\n");
}

/* stage 4: detect communities and topics of interest */
void 
stage_four(user_t *users, double *ths, int *thc, 
double **soc_matrix, int *user_count) {
	/* print stage header */
	print_stage_header(STAGE_NUM_FOUR);
	
	int core_users_count = 0;
	count_close_friends(users, soc_matrix, 
	user_count, ths);

	/* update core users status */
	for(int i = 0; i < *user_count; i++) {
		if (is_core(users[i], thc)) {
			users[i].is_core = 1;
			core_users_count++;
		} else {
			users[i].is_core = 0;
		}
	}

	community_t *communities = malloc(core_users_count * sizeof(community_t));
	assert(communities);

	/* fill in the core users */
	for(int i = 0, j = 0; i < *user_count && j < core_users_count; i++) {
		if (users[i].is_core == 1) {
			communities[j].core_user_num = i;
			j++;
		} 
	}

	fill_close_friends(communities, users, &core_users_count, 
	soc_matrix, ths, user_count);
	fill_unique_hashtags(users, communities, &core_users_count);
	stage_4_output(communities, &core_users_count, users);

	/* free memories allocated for communities and its keys */
	for(int i = 0; i < core_users_count; i++) {
		free_list(communities[i].unique_hashtags);
	}
	free_communities(communities, &core_users_count);
	
}

/****************************************************************/
/*********** implementing my own function prototypes ************/

/* read in user data from file and count the number of users */
void 
read_users(user_t *users, int *user_count) {
	int i = 0, j = 0, k = 0; // i for user, j for string idx, k for char idx in jth string
	char c;
	
	while(scanf("u%d %d", &users[i].user_num, &users[i].year) == 2) {
		getchar(); // eat the space
		j = 0; // reset j for next user
		users[i].hashtag_count = 0; // reset hashtag count for user

		/* allocate memory for hashtags array of ith user */ 
		users[i].hashtags = malloc(MAX_HASHTAG * sizeof(char*)); 
		assert(users[i].hashtags);
		
		/* go through the line(user i) */ 
		while((c = getchar()) != '\n'){
			/* start of a hashtag */
			if (c == '#') {
				k = 0; // reset k for keeping track of new string
				users[i].hashtag_count++; // increment number of hashtags
				
				/* allocate memory for jth hashtag for ith user */ 
				users[i].hashtags[j] = malloc((MAX_LEN + 1) * sizeof(char)); 
				assert(users[i].hashtags[j]);
				users[i].hashtags[j][k] = c; //hashtags start with the char '#'
				k++;
			} else if (c != ' ') {
				/* if the char read in is not space, add the char to kth pos of jth string */
				users[i].hashtags[j][k] = c;
				k++;
			} else if (c == ' ') {
				users[i].hashtags[j][k] = '\0'; // end of the hashtag
				j++;
			}
		}
		users[i].hashtags[j][k] = '\0'; // last char of last hashtag for ith user set to null byte
		i++;
	}

	*user_count = i;
}

/* Get the user number with the most hashtag counts */
void 
most_hash_user(user_t *users, int *user_count, int *max_hashtag_user_idx) {
	int max_hashtag_num = 0;
	int temp_max = 0; //flag variable to keep track of current users' hashtag count

	for(int i = 0; i < *user_count; i++) {
		temp_max = users[i].hashtag_count;
		if (temp_max > max_hashtag_num) {
			max_hashtag_num = temp_max;
			*max_hashtag_user_idx = i;
		}
	}
}

/* create a matrix array of array of integers */
int**
create_matrix(int *user_count) {
	int **matrix = malloc(*user_count * sizeof(int*)); // allocate mem for matrix
	assert(matrix);

	for(int i = 0; i < *user_count;) {
		/* allocate memory for each row */ 
		matrix[i] = malloc(*user_count * sizeof(int));
		assert(matrix[i]);
		for(int j = 0; j < *user_count; j++) {
			scanf("%d", &matrix[i][j]);
		}

		i++; // move onto next row
	}

	return matrix;
	
}

/* create a matrix of double type entries that signify soc */
double**
create_soc_matrix(int **friendship_m, int *user_count){
	double **soc_matrix = malloc(*user_count * sizeof(double*)); // allocate mem for matrix
	/* allocate mem for rows */
	for(int i = 0; i < *user_count; i++) {
		soc_matrix[i] = malloc(*user_count * sizeof(double));
	}

	return soc_matrix;
}

/* calculate the strength of connection between users (generalised) */
double 
s_o_c(int u1[], int u2[], int u1_num, int u2_num, int *user_count){
	int intersection = 0;
	int set_union = 0;
	
	if (u1[u2_num] == 0 && u2[u1_num] == 0) {
		return 0;
	} else {
		for (int i = 0; i < *user_count; i++) {
			if(u1[i] == 1 && u2[i] == 1) {
				intersection++;
				set_union++;
			} else if(u1[i] == 1 || u2[i] == 1) {
				set_union++;
			} else {

			}
		}
	}

	return (double)intersection / set_union;
}

/* print out double matrix */
void
print_double_matrix(double **soc_matrix, int *user_count){
	for(int i = 0; i < *user_count; i++) {
		for(int j = 0; j < *user_count; j++){
			if (j == *user_count - 1) {
				printf("%4.2lf", soc_matrix[i][j]);
			} else {
				printf("%4.2lf ", soc_matrix[i][j]);
			}
			
		}
		printf("\n"); // print next row at next line
	}
}

/* check if the user is a core */
int 
is_core(user_t user, int *thc) {
	if (user.cfriend_count > *thc) {
		return 1;
	} else {
		return 0;
	}
}

/* update the close friend count of each user */
void
count_close_friends(user_t *users, double **soc_matrix, int *user_count, 
double *ths) {
	/* indexing columns */
	for(int i = 0; i < *user_count; i++) {
		users[i].cfriend_count = 0;		
		/* indexing rows */
		for(int j = 0; j < *user_count; j++) {
			if (soc_matrix[i][j] > *ths) {
				users[i].cfriend_count++;
			}
		}
	}
}

/* fill in keys related to close_friends property */
void
fill_close_friends(community_t *communities, user_t *users, int *core_users_count,
double **soc_matrix, double *ths, int *user_count) {

	/* fill in the close_friend_count */
	for(int i = 0; i < *core_users_count; i++) {
		for(int j = 0; j < *user_count; j++) {
			if (soc_matrix[communities[i].core_user_num][j] > *ths) {
				communities[i].close_friend_count++;
			}
		}
	}

	/* obtain the close friend for each core user */
	for(int i = 0; i < *core_users_count; i++){
		communities[i].close_friend_nums = 
		malloc(users[communities[i].core_user_num].cfriend_count
		* sizeof(int));
		assert(communities[i].close_friend_nums);
		for(int j = 0, k = 0; j < *user_count && 
		k < users[communities[i].core_user_num].cfriend_count; j++) {
			if (soc_matrix[communities[i].core_user_num][j] > *ths) {
				communities[i].close_friend_nums[k] = j;
				k++;
			}
		}
	}
}

/* fill in the unique hashtags for each community */
void fill_unique_hashtags(user_t *users, 
community_t *communities, int *core_users_count) {
	int user_index;
	/* fill in the unique hashtags type */
	for(int i = 0; i < *core_users_count; i++) {
		communities[i].unique_hashtags = make_empty_list();
		assert(communities[i].unique_hashtags);
		user_index = communities[i].core_user_num;
		/* insert core users' hashtags into the list */
		for(int j = 0; j < users[user_index].hashtag_count; j++) {
			communities[i].unique_hashtags = 
			insert_unique_in_order(communities[i].unique_hashtags, 
			users[user_index].hashtags[j]);

		}
		/* insert close friends' hashtag into list */
		for(int j = 0; j < communities[i].close_friend_count; j++) {
			for(int k = 0; k < users[communities[i].close_friend_nums[j]].hashtag_count; k++) {
				communities[i].unique_hashtags = 
				insert_unique_in_order(communities[i].unique_hashtags, 
				users[communities[i].close_friend_nums[j]].hashtags[k]);
			}
		}
	}

}

/* Print out stage 4 outputs */
void 
stage_4_output(community_t *communities, int *core_users_count, user_t *users) {
	for(int i = 0; i < *core_users_count; i++) {
		printf("Stage 4.1. ");
		printf("Core user: u%d; close friends: ", communities[i].core_user_num);
		for(int j = 0; j < users[communities[i].core_user_num].cfriend_count; j++) {
			if (j == users[communities[i].core_user_num].cfriend_count - 1) {
				printf("u%d", communities[i].close_friend_nums[j]);
			} else {
				printf("u%d ", communities[i].close_friend_nums[j]);
			}
			
		}
		printf("\n");

		printf("Stage 4.2. Hashtags:\n");
		print_list(communities[i].unique_hashtags);
	}
}

/* free array of pointers to integers */
void
free_matrix(int **matrix, int *user_count){
	for(int i = 0; i < *user_count; i++) {
		free(matrix[i]); // free the rows of the matrix
	}
	free(matrix); // free the whole matrix
}

/* free array of pointers to doubles */
void
free_double_matrix(double **soc_matrix, int *user_count) {
	for(int i = 0; i < *user_count; i++) {
		free(soc_matrix[i]); // free the rows of the matrix
	}
	free(soc_matrix); // free the whole matrix
}

/* free the user_t type struct and its array keys */
void
free_users(user_t *users, int *user_count) {
	for(int i = 0; i < *user_count; i++) {
		for(int j = 0; j < users[i].hashtag_count; j++) {
			free(users[i].hashtags[j]); // free memory for each hashtag
		}
		free(users[i].hashtags); // free memory for array of hashtags
	}
	free(users); // free the whole user structs
}

/* free the array of community_t data type */
void
free_communities(community_t *communities, int *core_users_count){
	for(int i = 0; i < *core_users_count; i++) {
		free(communities[i].close_friend_nums);
	}
	free(communities);
}

/* below are functions skeletons provided by instructor, adapted by me when desired */

/* print stage header given stage number */
void 
print_stage_header(int stage_num) {
	printf(STAGE_HEADER, stage_num);
}

/****************************************************************/
/* linked list implementation below, adapted from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c 
*/

/* create an empty list */
list_t
*make_empty_list(void) {
	list_t *list;

	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;

	return list;
}

/* free the memory allocated for a list (and its nodes) */
void
free_list(list_t *list) {
	node_t *curr, *prev;

	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev->data);
		free(prev);
	}

	free(list);
}

/* insert a new data element into a linked list, keeping the
	data elements in the list unique and in alphabetical order
*/
list_t
*insert_unique_in_order(list_t *list, data_t value) {

	node_t *new;
	new = (node_t*)malloc(sizeof(node_t));
	assert(new);
	new->data = malloc(MAX_LEN * sizeof(char));
	assert(new->data);
	strcpy(new->data, value);
	new->next = NULL;

	node_t *previous = NULL;
	node_t *current = list->head;

	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		/* keep track of the current and previous node traversed */
		while (current != NULL && strcmp(new->data, current->data) >= 0) {
			if (strcmp(new->data, current->data) == 0) {
                /* Value is already in the list, do not insert */
				return list;
			} else {
				previous = current;
				current = current->next;
			}
            
		}

		if (previous == NULL) {
			/* Insert at the beginning of the list */ 
			new->next = list->head;
			list->head = new;
		} else {
			/* Insert in the middle or at the end */ 
			new->next = current;
			previous->next = new;

			if (current == NULL) {
                /* Insert at the end of the list */ 
				list->foot = new;
			}
		}
	}

	return list;
}

/* print the data contents of a list */
void
print_list(list_t *list) {
	node_t *current = list->head;
	int hash_count = 0;

	while (current != NULL) {
		if (hash_count == 4 || current->next == NULL){
			printf("%s\n", current->data);
			hash_count++;
		} else {
			printf("%s ", current->data);
			hash_count++;
		}
        
		if (hash_count == 5 && current->next != NULL) {
			hash_count = 0; // reset the counter
		}

		current = current->next;
	}

}

/****************************************************************/
/*
	Write your time complexity analysis below for Stage 4.2, 
	assuming U users, C core users, H hashtags per user, 
	and a maximum length of T characters per hashtag:

	Answer: O(U * C * H^2 * T)
	Justification: 
	the program first created an array of community_t type data by 
	using malloc() to allocate enough memory to store the number of communities,
	which is equal to the number of core users. Since malloc() was not used inside any
	loop for stage 4.2, the time complexity here is O(1) - constant time.

	the program then loop through the number of users to identify the core users
	and set them at the arrays "prepared" to store them, which has time
	complexity of O(U + C). Looping though the users array takes O(U), and 
	then adding the respective core_user_number takes O(C), which means in total
	this part has time complexity of O(U + C)

	The fill_close_friend function has time complexity O(C * U). The reason is that
	for every core user, the program looks through the respective row in the soc_matrix
	to find all the close friends of the user, where the number of entries of each row of
	soc_matrix is U. Hence, there are U number of O(1) operations executed C time, and the overall
	time complexity is O(C * U). Technically it is O(2 * C * U) since this instance of nested loop
	is executed twice in this function, but the scalar 2 becomes less important as input size grow
	so it can be omitted.

	The fill_unique_hashtag function has time complexity O(U * C * H^2 *T). The reason is that 
	the function in the worst case would loop though all U users for every core users, in other words
	every one is friends with every one and every one has all others as close friends. then it 
	has to traverse through the linked list from the start to find the correct position to insert the 
	respective hashtags. The worse case would be that the correct position is at the foot of the linked
	list every insertion to be made where every hashtag is unique, which would result 
	in 1 + 2 + ... + H number of insertion (insertion at index is constant time operation(O(1))) 
	which can be proved by induction that the expression equal to H*(H+1)/2, which is O(H^2). 
	When traversing, the new node's string is compared to the string of the node in the respective 
	position of the linked list. This comparison at worst case takes T comparison where every letter 
	of both strings are compared. In conslusion, T comparisons are executed for every traverse gives 
	T * H^2 comparisons in total and this potentially needs to be done for all users in case all users 
	have unique hashtags to each other, and such process is repeated C times for the number of core 
	users, which gives O(T*H^2*U*C) time complexity. (Technically it is O((U-1) * C * H^2 * T), 
	since users are not considered friends with themselves, but it can be simplified to O(T*H^2*U*C)).

	The stage_4_output has time complexity O(U * C + U * H * T), for output of stage 4.1, the function 
	prints the core user and its close friends, which in the worse case each user has U - 1 close 
	friends(U - 1) because users are not considered friends with themselves). And for stage 4.2, 
	the function has to
	traverse through the linked list, which at worst case would have U*H nodes in case every hashtags
	from the inputs are unique. and to print a string every character is printed which take H times to
	execute. Hence the overall time complexity for stage_4_output would be O(U*C+U*H*T).

	The overall complexity for stage 4.2. is the maximum time complexity among each stages, which is 
	O(U * C * H^2 * T).

*/
