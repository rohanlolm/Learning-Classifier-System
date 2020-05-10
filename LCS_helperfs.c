//Helper functions

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <string.h>
#include <errno.h> 
#include <windows.h>
//#include <gsl/gsl_statistics_double.h>
//My Includes
#include "LCS_structs.h"
#include "LCS_protos.h"

#ifdef _WIN32
#define HEADER_OFFSET 216
#include <windows.h>
#endif

#ifdef linux
#define HEADER_OFFSET 212
#endif

//

int results_printer(struct data_rows *testing_data, Rule_Set *rule_pop, int row){ 
    char results_filename[] = "interval_results.txt";
    char mode[] = "a+";
    
    FILE *resultsfile = xfopen(results_filename, mode);
    struct LCS_results results = {0,0}; 
    
    LCS_test(testing_data, rule_pop, &results);
    struct Pop_Stats pop_stats = calc_pop_stats(rule_pop);
    
    //format: num_rules, accuracy, iteration, mean_acc, mean_fit, multi_match
    fprintf(resultsfile, "%d\t%f\t%d\t%f\t%f\t%d", rule_pop->num_rules, results.accuracy, row, 
            pop_stats.mean_fit, pop_stats.mean_acc, pop_stats.multi_match); 
    fprintf(resultsfile, "\n");    
    
    fclose(resultsfile);    
    
    return 0;     
}

//Wrapper around malloc to include error checking.
void* xmalloc(size_t bytes){
    void *retp = malloc( bytes );
    if(retp == NULL){
        fprintf( stderr,"Malloc Error: %s \n",strerror(errno) );
    }
    return retp;
}

FILE *xfopen(char *filename, char *mode){
	FILE *ret_file = fopen(filename, mode); 
	
	if(ret_file == NULL){
		fprintf(stderr, "File %s failed to open. \n", filename); 
		exit(EXIT_FAILURE);
	}
	return ret_file; 
}

//Returns 1 with probability 'prob', else returns 0 
int boolean_prob(double prob){
	return (rand()/(double)RAND_MAX)<prob;
}

int str_maker(char *line){ 
	int i = 0; 
	int line_ends = 0;
	while(line[i] != '\0'){ 
		if(line[i] == '\n' || line[i] == '\r'){ 
			line[i] = '\0'; 
			line_ends = 1;
		}
		++i; 
	}
	if(line_ends == 0){  
        fprintf(stderr,"Unacceptable file format. Single file line too long. Make sure final line has a newline. \n"); 
		exit(EXIT_FAILURE);
	}
    return 0; 
}

int compare_int( const void* a, const void* b ){
    if( *(int*)a == *(int*)b ) return 0;
    return *(int*)a < *(int*)b ? -1 : 1;
}

void print_array(int *a, int num_elements){
    int i;
    for (i = 0; i<num_elements; i++){
        printf("%d ", a[i]);
    }
    printf("\n");
}

int get_max_indecies(double *array, int len, int *max_indecies){
	double current_max = -INFINITY;
	int num_max = 0; 
	
	for(int i=0; i<len; i++){
		if( array[i] > current_max ){
			current_max = array[i];
		}
	}
	for(int i=0; i<len; i++){
		if( array[i] == current_max){
			max_indecies[num_max] = i; 
			num_max++; 
		}
	}
	return num_max;
}

int nomination_guesser(void){
	fprintf(stderr, "Guesser Used\n");
    /*if(boolean_prob(0.04) == 1){
		return 18;
	}
	else if(boolean_prob(0.18) == 1){
		return 24;
	}
	else */return 0; 
}

//DEBUGGING FUNCTIONS:
void print_data(struct data_rows *data){
	for(int row=0; row <10; row++){
		printf("%d %d \n", data->key_indices[row], data->nominations[row]);
	}
}

void Rule_print(Rule_Set *rule_pop){
	for(int irule=0; irule<rule_pop->num_rules; irule++){
		printf("Correct=%d, Incorrect = %d, Total = %d, accuracy = %f\n", rule_pop->rules[irule]->correct_matches, rule_pop->rules[irule]->incorrect_matches, rule_pop->rules[irule]->matches, rule_pop->rules[irule]->accuracy );
	}
}

unsigned int rand_interval(unsigned int min, unsigned int max){
    unsigned int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do{
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

//Box-Muller norm distributrion generator 
double norm_rand(double mean, double variance){ 							//Generates exactly normally distributed random numbers. 
	double U1 = rand() / (double)RAND_MAX; 
	double U2 = rand() / (double)RAND_MAX; 
	double N1 = sqrt(-2*log(U1))*cos(2*PI*U2); 		//Uses trig function which can be slow
	return N1*sqrt(variance) + mean;
}

double calculate_mean(double *data, size_t len){
	double sum=0;
	for(unsigned int i=0; i<len; i++){
		sum += data[i]; 
	}
	return sum/(double)len; 
}

int python_printer(struct LCS_results *results, struct Pop_Stats *pop_stats){
	fprintf(stdout, "%f %d %f %f %d", results->accuracy, results->rule_pop_len, pop_stats->mean_fit, 
										pop_stats->mean_acc, pop_stats->multi_match);
	return 0; 
}

int console_printer(struct LCS_results *results, struct Pop_Stats *pop_stats){
	printf("\nTest Set Accuracy = %f\nNum_Rules = %d\n==================\nMean_Fit = %f\nMean_Acc = %f\nMulti_Rules = %d\n", 
			results->accuracy, results->rule_pop_len, pop_stats->mean_fit, pop_stats->mean_acc, pop_stats->multi_match);
	printf("*******************\n");
	
	return 0; 
}

int rule_pop_writer(Rule_Set *rule_pop, char *rule_filename, struct parameters *params){
	FILE *rulefile = xfopen(rule_filename, "w");
	
	for(int irule = 0; irule < rule_pop->num_rules; irule++){
		fprintf(rulefile, "%d\t", irule);
		for(int pred=0; pred < params->num_predictors; pred++){
			fprintf(rulefile, "%f\t%f\t", rule_pop->rules[irule]->intervals[pred][0], rule_pop->rules[irule]->intervals[pred][1]); 
		}
		fprintf(rulefile, "%d\t%f\t%f\t%d\t%d\t\%d", rule_pop->rules[irule]->nomination, rule_pop->rules[irule]->accuracy,
				rule_pop->rules[irule]->fitness, rule_pop->rules[irule]->matches, rule_pop->rules[irule]->correct_matches, 
				rule_pop->rules[irule]->incorrect_matches);
		fprintf(rulefile, "\n");
	}
	
	fclose(rulefile);
	
	return 0; 	
}

int rule_set_pickler(Rule_Set *rule_pop, char *pickle_file, struct parameters *params){
	
	if( rule_pop-> num_rules == 0 ){
		fprintf(stderr, "Rule_Pop is empty\n. Exiting."); 
		exit(EXIT_FAILURE);
	}
	
	int count = 0; 
	
	FILE *pick_file = xfopen(pickle_file, "w+");
    fprintf(pick_file, "# Comments can be placed at the bignning of the file by starting the line with '#'. Blank lines will cause errors.\n#\n"); 
	fprintf(pick_file, "#=================== LCS Model File ========================\n");
	fprintf(pick_file, "# Accuracy = %f, Num_rules = %d, Num_predictors = %d\n", rule_pop->accuracy, rule_pop->num_rules, rule_pop->num_predictors);
	fprintf(pick_file, "# Parameters: k = %f, d = %f, w = %f \n", params->kill_thresh, params->spread_percent, params->wc_prob); 
	fprintf(pick_file, "#===========================================================\n");  
	
	fclose(pick_file); 
	
	pick_file = xfopen(pickle_file, "ab+");
	
	int buffer1[2] = {rule_pop->num_rules, rule_pop->num_predictors};
	fwrite(buffer1, sizeof(*buffer1), 2, pick_file); 
	
	size_t rule_size = sizeof(Rule) + 2*sizeof(double)*buffer1[1];
	void *rule_buff = xmalloc( rule_size );
	
	for(int i_rule = 0; i_rule < rule_pop->num_rules; i_rule++){
		memcpy(rule_buff, rule_pop->rules[i_rule], rule_size);
		count += fwrite(rule_buff, rule_size, 1, pick_file);
	}
	
	free(rule_buff);
	fclose(pick_file); 
	
	return count; //Add 4 for each of the new 
}

//Memory has to be free by the Caller 
Rule_Set *rule_set_unpickler(char *pickle_file){
    
    char line[BUFSIZ];  
    FILE *pick_file = xfopen(pickle_file, "r+");
    fpos_t pos;
    int is_comment = 1;
    int lines_to_skip = 0; 
    while(fgets(line,BUFSIZ,pick_file) != NULL && is_comment == 1){
       // str_maker(line);
        lines_to_skip++;    //Skip Comment lines which begin with a # 
        if( line[0] != '#'){
            is_comment = 0; 
        }
    }
    rewind(pick_file); 
    for(int lines = 0; lines < lines_to_skip -1; lines++){
        fgets(line,BUFSIZ,pick_file); 
    }  
    //char test_buff[100]; 
   // fread(test_buff, sizeof(*test_buff), 100, pick_file); 
  //  fprintf(stderr, "string:%s", test_buff);
    //int pos = ftell(pick_file);
    fgetpos(pick_file, &pos); 
    fclose(pick_file); 
    
	pick_file = xfopen(pickle_file, "rb+");
	//fseek(pick_file,pos,SEEK_SET);
    fsetpos(pick_file, &pos); 

	int count = 0; 

	int buffer1[2] = {0};

	fread(buffer1, sizeof(*buffer1), 2, pick_file); 	//{num_rules, num_predictors}
	
	int num_rules = buffer1[0];
		
	Rule_Set *rule_pop = xmalloc( sizeof(Rule_Set) + sizeof(Rule *)*MAX_NUM_RULES);
	rule_pop->num_rules = buffer1[0]; 
	rule_pop->num_predictors = buffer1[1]; 
	
	size_t rule_size = sizeof( Rule ) + 2*sizeof(double)*rule_pop->num_predictors;
	void *rule_buff = xmalloc( rule_size );
	
	for(int i_rule = 0; i_rule < rule_pop->num_rules; i_rule++){
		rule_pop->rules[i_rule] = xmalloc( rule_size );
		count +=fread(rule_buff, rule_size, 1, pick_file);
		memcpy(rule_pop->rules[i_rule], rule_buff, rule_size);
	}
	
	if(count != num_rules){
		fprintf(stderr, "Number of Rules read != Num_rules written. \n, %d, %d", count, num_rules ); 
		exit(EXIT_FAILURE); 
	}
	
	free(rule_buff);
	fclose(pick_file); 
	
	return rule_pop;
}
//}

//Memory has to be free by the Caller 
Rule_Set *recovery_unpickler(char *pickle_file){
    
	FILE *pick_file = xfopen(pickle_file, "rb+");
	int count = 0; 

	int buffer1[2] = {0};

	fread(buffer1, sizeof(*buffer1), 2, pick_file); 	//{num_rules, num_predictors}
	
	int num_rules = buffer1[0];
		
	Rule_Set *rule_pop = xmalloc( sizeof(Rule_Set) + sizeof(Rule *)*MAX_NUM_RULES);
	rule_pop->num_rules = buffer1[0]; 
	rule_pop->num_predictors = buffer1[1]; 
	
	size_t rule_size = sizeof( Rule ) + 2*sizeof(double)*rule_pop->num_predictors;
	void *rule_buff = xmalloc( rule_size );
	
	for(int i_rule = 0; i_rule < rule_pop->num_rules; i_rule++){
		rule_pop->rules[i_rule] = xmalloc( rule_size );
		count +=fread(rule_buff, rule_size, 1, pick_file);
		memcpy(rule_pop->rules[i_rule], rule_buff, rule_size);
	}
	
	if(count != num_rules){
		fprintf(stderr, "Number of Rules read != Num_rules written. \n"); 
		exit(EXIT_FAILURE); 
	}
	
	free(rule_buff);
	fclose(pick_file); 
	
	return rule_pop;
}

