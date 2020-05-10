//LCS Model validator/Tester 
//Rohan Mehra 

//Import a model (from .pkl file) 
//Run it on the test set
//Print Results 

#include <stdio.h> 
#include <stdlib.h>
#include <time.h> 
#include <getopt.h>
#include "LCS_structs.h"
#include "LCS_protos.h"

int GLOBAL_F_print_prediction = 0; 

int main(int argc, char **argv){
    char opt;
    int recovery_flag = 0;
    int console_flag = 0; 
    char *recovery_file;
    
    
    while( ( opt = getopt(argc,argv,"r:cd") ) !=-1 ) {			//Use getopt to loop through arguments
		if(opt == 'r'){
            //recovery mode 
            recovery_file = optarg; 
            recovery_flag = 1; 
        }
        else if(opt == 'c') console_flag = 1;
        else if(opt == 'd') GLOBAL_F_print_prediction = 1; 
    }
        
	if((argc-4) != optind){
		fprintf(stderr, "Incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	}
	
	char *model_filename = argv[optind];
	char *testing_filename = argv[optind+1];
	int num_predictors = atoi(argv[optind+2]); 
	int num_test_rows = atoi(argv[optind+3]); 
	
    Rule_Set *rule_pop; 
    
    if(recovery_flag == 1){
        rule_pop = recovery_unpickler(recovery_file); 
    }
    
    else rule_pop = rule_set_unpickler(model_filename); 
    
	if(num_predictors != rule_pop->num_predictors){
		fprintf(stderr, "Error: Number of Predictors in Model != num predcitors entered\n %d, %d\n", num_predictors, rule_pop->num_predictors);
		exit(EXIT_FAILURE);
	}
	
	//Import Testing data: 
	struct data_rows *testing_data = create_data_rows(rule_pop->num_predictors, num_test_rows); 
	int imported_test_rows = data_import(testing_data, testing_filename);
	if(imported_test_rows != num_test_rows){
		fprintf(stderr, "Test Import rows and num_rows do not match. Rows imported %d", imported_test_rows );
		exit(EXIT_FAILURE);
	}
	clock_t clockstart = clock();
	struct LCS_results results = {0,0}; 
	LCS_test(testing_data, rule_pop, &results);
	struct Pop_Stats pop_stats = calc_pop_stats(rule_pop);
	if(console_flag) console_printer(&results, &pop_stats); 
    else python_printer(&results, &pop_stats);
    clock_t clockend = clock();
    printf("Execution Time (Not Including Data Import): %.3fs \n\n", (double)(clockend - clockstart) / CLOCKS_PER_SEC); 
	
	rule_pop_free(rule_pop); 
	
	return 0; 
}