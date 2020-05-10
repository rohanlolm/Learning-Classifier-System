//Function Prototypes Start **********************************************************************

#include <math.h>
#include <time.h>

//Constant Definitions 
#define MAX_MATCH_SIZE 1000
#define MAX_NUM_RULES 20000
#define MAX_LINE_SIZE 4096
#define ACC_TO_FIT 10
#define WC_PROB 0.15
#define SPREAD_PERCENT 0.075
#define SPREAD_VAR 0.01
#define SEED 1
#define KILL_THRESH 0.7
#define NUM_LOOPS 2
#define EVOLVE_ITERS 100 
#define MUT_PROB 0.05
#define NOM_MUT_PROB 0.05
#define PI 3.14159265
#define RES_INTERVAL 50 

#define DEBUG fprintf(stderr,"here!\n"); 

int results_printer(struct data_rows *testing_data, Rule_Set *rule_pop, int row);

//Global Timing vars 
clock_t clockstart, clockend, test_clockstart, test_clockend;		//Initialise global clock variables 

//Data Import and Structuring =================================================================
 int data_import(struct data_rows *training_data, char *filename);
 struct data_rows *create_data_rows(int num_predictors, int num_rows);
 int free_all_alocs(struct data_rows *training_data, struct data_rows *testing_data, Rule_Set *rule_pop);
 int rule_pop_free(Rule_Set *rule_pop);
 //=================================================================

//Primary LCS Functions ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//High Level Functions 
int LCS_train(struct data_rows *training_data, struct data_rows *testing_data, Rule_Set *rule_pop, struct parameters *params);
 int LCS_test(struct data_rows *testing_data, Rule_Set *rule_pop, struct LCS_results *results);
 int new_rule(struct data_rows *training_data, int row, Rule_Set *rule_pop, struct parameters *params);
 int matched_rules(struct data_rows *training_data, Rule_Set *rule_pop, int *matched_indices, int current_row, char mode);
int update_rule_pop(Rule_Set *rule_pop, int *matched_rules, int num_matched_rules);
 //Removing Rules   
 int kill_bad_rules(Rule_Set *rule_pop, double kill_thresh);
 int remove_rule(Rule_Set *rule_pop, int rule_index, int *iterator);
 //Other Functions 
 int nearest_nomination(struct data_rows *testing_data, int row, Rule_Set *rule_pop); 
 int nomination_guesser(void); 
 int predict_nomination(struct data_rows *data, int current_row, int *matched_indices, int num_matched, Rule_Set *rule_pop); 
 int pop_subsumption(Rule_Set *rule_pop);
 int is_rule_subset(Rule_Set *rule_pop, int r1_index, int r2_index, int num_predictors);
 struct LCS_results run_LCS(struct data_rows *training_data, struct data_rows *testing_data, Rule_Set *rule_pop, struct parameters *params);
 struct Pop_Stats calc_pop_stats(Rule_Set *rule_pop); 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
//Genetic Functions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
int evolve_rule_set(Rule_Set *rule_pop, struct data_rows *training_data, struct parameters *params, int iterations);
Rule *mutate_rule(Rule_Set *rule_pop, Rule *child, struct parameters *params); 
int child_subsumption(Rule_Set *rule_pop, Rule **parent_list, int num_parents, Rule *child); 
int update_child_fitness(struct data_rows *training_data, Rule *child); 
int tournament_select(Rule_Set *rule_pop, double tourn_mult);
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
 
//Helper Functions _____________________________________________________________________
int str_maker(char *line);
int compare_int(const void* a, const void* b);
void print_array(int *a, int num_elements); //Just for debugging 
int get_max_indecies(double *array, int len, int *max_indecies);
void *xmalloc(size_t bytes); 
FILE *xfopen(char *filename, char *mode); 
unsigned int rand_interval(unsigned int min, unsigned int max);
double norm_rand(double mean, double variance);
double calculate_mean(double *data, size_t len);
//Printers 
int python_printer(struct LCS_results *results, struct Pop_Stats *pop_stats);
int console_printer(struct LCS_results *results, struct Pop_Stats *pop_stats);
int rule_pop_writer(Rule_Set *rule_pop, char *rule_filename, struct parameters *params); 
//Picklers 
Rule_Set *rule_set_unpickler(char *pickle_file); 
int rule_set_pickler(Rule_Set *rule_pop, char *pickle_file, struct parameters *params);
Rule_Set *recovery_unpickler(char *pickle_file); 
//Other
int boolean_prob(double prob); 
int len(void *arr);
//_____________________________________________________________________

//debugging 
void print_data(struct data_rows *data);
void Rule_print(Rule_Set *rule_pop);
//Function Protypes END ***********************************************************************************

//Inlines
//Neatenning interval comparison functions 
inline double lower(Rule *rule, int pred){
	if(rule->intervals[pred][1] != INFINITY){
		return rule->intervals[pred][0] - rule->intervals[pred][1];
	}
	else{
		return -INFINITY;
	}
	
}

inline double higher(Rule *rule, int pred){
	if(rule->intervals[pred][1] != INFINITY){
		return rule->intervals[pred][0] + rule->intervals[pred][1];
	}
	else{
		return INFINITY;
	}
} 
