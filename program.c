/* A simplified learned index implementation:
 *
 * Skeleton code written by Jianzhong Qi, April 2023
 * Edited by: Liang Zhang 1365688
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STAGE_NUM_ONE 1						  /* stage numbers */ 
#define STAGE_NUM_TWO 2
#define STAGE_NUM_THREE 3
#define STAGE_NUM_FOUR 4
#define STAGE_HEADER "Stage %d\n==========\n" /* stage header format string */

#define DATASET_SIZE 100					  /* number of input integers */
#define DATA_OUTPUT_SIZE 10					  /* output size for stage 1 */

#define BS_NOT_FOUND (-1)					  /* used by binary search */
#define BS_FOUND 0

typedef int data_t; 				  		  /* data type */

/*my defined datatype*/
typedef struct func_params{
		int a, b;
		int key;
		int is_null;
	} coverage_func_t;

/****************************************************************/

/* function prototypes */
void print_stage_header(int stage_num);
int cmp(data_t *x1, data_t *x2);
void swap_data(data_t *x1, data_t *x2);
void partition(data_t dataset[], int n, data_t *pivot,
		int *first_eq, int *first_gt);
void quick_sort(data_t dataset[], int n);
int binary_search(data_t dataset[], int lo, int hi, 
	data_t *key, int *locn);

void stage_one(data_t dataset[]);
void stage_two(data_t dataset[], int *max_err);
void stage_three(data_t dataset[], data_t *target_err, coverage_func_t functions[]);
void stage_four(coverage_func_t functions[], data_t dataset[], data_t *target_err);
/* add your own function prototypes here */
void parameter_calculation(data_t dataset[], int *a, int *b, int idx_1, int idx_2);
int f_key(int *a, int *b, data_t key, int idx_a);
int error_calculation(data_t actual_idx, data_t calculated_idx);
int is_covered(data_t *my_err, data_t *target_err);
int max_elem_binary_search(data_t dataset[], int lo, int hi, data_t *key, int *locn);
int binary_search_with_print(data_t dataset[], int lo, int hi, data_t *key, int *locn);
int max(int num1, int num2);
int min(int num1, int num2);
int lower_bound(int predicted_pos, int target_err);
int higher_bound(int predicted_pos, int target_err);

/****************************************************************/

/* main function controls all the action */
int
main(int argc, char *argv[]) {
	/* to hold all input data */
	data_t dataset[DATASET_SIZE];
	int max_err;
	int target_err;
	coverage_func_t functions[DATASET_SIZE] = {0};

	/* to hold the mapping functions */


	/* stage 1: read and sort the input */
	stage_one(dataset); 
	
	/* stage 2: compute the first mapping function */
	stage_two(dataset, &max_err);
	
	/* stage 3: compute more mapping functions */ 
	stage_three(dataset, &target_err, functions);
	
	/* stage 4: perform exact-match queries */
	stage_four(functions, dataset, &target_err);
	
	/* all done; take some rest */
	return 0;
}

/****************************************************************/

/* add your code below */

/* stage 1: read and sort the input */
void 
stage_one(data_t dataset[]) {
	/* add code for stage 1 */
	/* print stage header */
	print_stage_header(STAGE_NUM_ONE);
	int i = 0;
	while(scanf("%d", &dataset[i]) == 1 && i < DATASET_SIZE - 1){
		i++;
	}
	quick_sort(dataset, DATASET_SIZE);
	printf("First 10 numbers: ");
	for(int i = 0; i < DATA_OUTPUT_SIZE - 1; i++){
		printf("%d ", dataset[i]);
	}
	printf("%d", dataset[DATA_OUTPUT_SIZE - 1]);
	
	printf("\n\n");
}

/* stage 2: compute the first mapping function */
void
stage_two(data_t dataset[], int *max_err) {
	/* add code for stage 2 */
	/* print stage header */
	print_stage_header(STAGE_NUM_TWO);

	int a, b;
	int temp_max_err = 0;
	int max_err_idx = 0;

	parameter_calculation(dataset, &a, &b, 0, 1);

	for(int i = 0; i < DATASET_SIZE; i++){
		temp_max_err = error_calculation((data_t)i, (data_t)f_key(&a, &b, dataset[i], 0));
		if(temp_max_err > *max_err){
			*max_err = temp_max_err;
			max_err_idx = i;
		}
	}

	printf("Maximum prediction error: %d\nFor key: %d\nAt position: %d\n", 
	*max_err, dataset[max_err_idx], max_err_idx);

	printf("\n");
}

/* stage 3: compute more mapping functions */ 
void 
stage_three(data_t dataset[], data_t *target_err, coverage_func_t functions[]) {
	/* add code for stage 3 */
	/* print stage header */
	print_stage_header(STAGE_NUM_THREE);
	scanf("%d", target_err);

	/*compute the value a, b for the first 2 elements*/
	coverage_func_t func_1 = {0};
	parameter_calculation(dataset, &func_1.a, &func_1.b, 0, 1);
	f_key(&func_1.a, &func_1.b, dataset[0], 0);
	functions[0] = func_1;

	/*compute the rest of the function that covers the error allowed*/
	//set i to index the dataset array, and j to index the functions array
	int j = 0;
	int current_err;
	int current_a_idx = 0;
	for(int i = 1; i < DATASET_SIZE;){
		current_err = error_calculation(i, f_key(&functions[j].a, &functions[j].b, 
		dataset[i], current_a_idx));
		if(current_err <= *target_err){
			functions[j].key = dataset[i];
			i++;
		}else{
			j++;
			parameter_calculation(dataset, &functions[j].a, &functions[j].b, i, i + 1);
			current_a_idx = i;
			f_key(&functions[j].a, &functions[j].b, dataset[i], current_a_idx);
		}

	}	
	j++;
	functions[j].is_null = 1;

	j = 0;
	printf("Target maximum prediction error: %d\n", *target_err);
	while(!functions[j].is_null){
		printf("Function %2d: a = %4d, b = %3d, max element = %3d\n", j, 
		functions[j].a, functions[j].b, functions[j].key);
		j++;
	}

	printf("\n");
}


/* stage 4: perform exact-match queries */
void stage_four(coverage_func_t functions[], data_t dataset[], data_t *target_err) {
	/* add code for stage 4 */
	/* print stage header */
	print_stage_header(STAGE_NUM_FOUR);
	int key;
	data_t max_elem_arr[DATASET_SIZE] = {0};
	int i = 0;
	while(!functions[i].is_null){
		max_elem_arr[i] = functions[i].key;
		i++;
	}

	int funct_num = 0;
	int predicted_pos = 0;
	int hi, lo;

	while(scanf("%d", &key) == 1){
		printf("Searching for %d:\n", key);
		if(key < dataset[0] || key > dataset[DATASET_SIZE - 1]){
			printf("Step 1: not found!\n");
		}else{
			
			printf("Step 1: search key in data domain.\n");
			printf("Step 2:");
			max_elem_binary_search(max_elem_arr, 0, i, &key, &funct_num);
			printf("\n");
			predicted_pos = f_key(&functions[funct_num].a, &functions[funct_num].b, key, 
			functions[funct_num].a);
			
			lo = lower_bound(predicted_pos, *target_err);
			hi = higher_bound(predicted_pos, *target_err) + 1;

			printf("Step 3: ");
			binary_search_with_print(dataset, lo, hi, &key, &predicted_pos);
			printf("\n");
			
			
		}
	}

	printf("\n");
}




/****************************************************************/
/* functions provided, adapt them as appropriate */

/* print stage header given stage number */
void 
print_stage_header(int stage_num) {
	printf(STAGE_HEADER, stage_num);
}

/* data swap function used by quick sort, adpated from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/sortscaffold.c
*/
void
swap_data(data_t *x1, data_t *x2) {
	data_t t;
	t = *x1;
	*x1 = *x2;
	*x2 = t;
}

/* partition function used by quick sort, from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/quicksort.c 
*/
void
partition(data_t dataset[], int n, data_t *pivot,
		int *first_eq, int *first_gt) {
	int next=0, fe=0, fg=n, outcome;
	
	while (next<fg) {
		if ((outcome = cmp(dataset+next, pivot)) < 0) {
			swap_data(dataset+fe, dataset+next);
			fe += 1;
			next += 1;
		} else if (outcome > 0) {
			fg -= 1;
			swap_data(dataset+next, dataset+fg);
		} else {
			next += 1;
		}
	}
	
	*first_eq = fe;
	*first_gt = fg;
	return;
}

/* quick sort function, adapted from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/quicksort.c 
*/
void
quick_sort(data_t dataset[], int n) {
	data_t pivot;
	int first_eq, first_gt;
	if (n<=1) {
		return;
	}
	/* array section is non-trivial */
	pivot = dataset[n/2]; // take the middle element as the pivot
	partition(dataset, n, &pivot, &first_eq, &first_gt);
	quick_sort(dataset, first_eq);
	quick_sort(dataset + first_gt, n - first_gt);
}

/* comparison function used by binary search and quick sort, from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/binarysearch.c 
*/
int
cmp(data_t *x1, data_t *x2) {
	return (*x1-*x2);
}

/* adapted binary search that also prints out the elements it is looking at */
int
binary_search_with_print(data_t dataset[], int lo, int hi, 
	data_t *key, int *locn) {

	int mid, outcome;
	/* if key is in dataset, it is between dataset[lo] and dataset[hi-1] */

	if (lo>=hi) {
		printf("not found!");
		return BS_NOT_FOUND;
	}

	mid = (lo+hi)/2;
	
	if ((outcome = cmp(key, dataset+mid)) < 0) {
		printf("%d ", dataset[mid]);
		return binary_search_with_print(dataset, lo, mid, key, locn);
	} else if (outcome > 0) {
		printf("%d ", dataset[mid]);
		return binary_search_with_print(dataset, mid+1, hi, key, locn);
	} else {
		*locn = mid;
		printf("%d @ dataset[%d]!", dataset[*locn], *locn);
		return BS_FOUND;
	}
}

/* binary search adapted to specifically find the function that covers 
   the element*/
int
max_elem_binary_search(data_t max_elem_arr[], int lo, int hi, 
		data_t *key, int *locn) {
	int mid, outcome;
	/* if key is in dataset, it is between dataset[lo] and dataset[hi-1] */
	
	mid = (lo+hi)/2;

	if (lo>=hi) {
		*locn = mid;
		return BS_NOT_FOUND;
	}
	
	if ((outcome = cmp(key, max_elem_arr+mid)) < 0) {
		*locn = mid;
		printf(" %d", max_elem_arr[*locn]);
		return max_elem_binary_search(max_elem_arr, lo, mid, key, locn);
	} else if (outcome > 0) {
		*locn = mid;
		printf(" %d", max_elem_arr[*locn]);
		return max_elem_binary_search(max_elem_arr, mid+1, hi, key, locn);
	} else {
		*locn = mid;
		printf(" %d", max_elem_arr[*locn]);
		return BS_FOUND;
	}
}

/* binary search between dataset[lo] and dataset[hi-1], adapted from
   https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/binarysearch.c 
*/
int
binary_search(data_t dataset[], int lo, int hi, 
		data_t *key, int *locn) {
	int mid, outcome;
	/* if key is in dataset, it is between dataset[lo] and dataset[hi-1] */
	if (lo>=hi) {
		return BS_NOT_FOUND;
	}
	mid = (lo+hi)/2;
	
	if ((outcome = cmp(key, dataset+mid)) < 0) {
		
		return binary_search(dataset, lo, mid, key, locn);
	} else if (outcome > 0) {
		
		return binary_search(dataset, mid+1, hi, key, locn);
	} else {
		*locn = dataset[mid];
		
		return BS_FOUND;
	}
}

/*calculate the parameter a and b for a given pair of data*/
void parameter_calculation(data_t dataset[], int *a, int *b, 
int idx_1, int idx_2){
	*a = dataset[idx_2] * idx_1 - dataset[idx_1] * idx_2;
	*b = dataset[idx_2] - dataset[idx_1];
}

/*implementation of the f(key) function as specified in assgnment spec*/
int f_key(int *a, int *b, data_t key, int idx_a){
	if(*b != 0){
		return ceil((key + *a) / (double)*b);
	}else{
		*a = idx_a;
		return *a;
	}
}

/*calculate the error between predicted idx and actual idx*/
int error_calculation(data_t actual_idx, data_t calculated_idx){
	int return_val = abs(actual_idx - calculated_idx);
	return return_val;
}

/*determine if given error is smaller than the target error aka 'covered'*/
int is_covered(data_t *my_err, data_t *target_err){
	if(*my_err <= *target_err){
		return 1;
	}else{
		return 0;
	}
}

/*find the maximum value between 2 numbers*/
int max(int num1, int num2){
	if(num1 > num2){
		return num1;
	} else {
		return num2;
	}
}

/*find the minimum value between 2 numbers*/
int min(int num1, int num2){
	if(num1 < num2){
		return num1;
	} else {
		return num2;
	}
}

/*find the lower bound of binary search*/
int lower_bound(int predicted_pos, int target_err){
	return max(0, (predicted_pos - target_err));
}

/*find the higher bound for binary search*/
int higher_bound(int predicted_pos, int target_err){
	return min(DATASET_SIZE - 1, predicted_pos + target_err);
}

