/*Learning Classifier System (Supervised XCS (UCS))
Rohan Mehra (21136357)
BHPB CEED Project (Adaptive Schedule Optimisation)

This program implements a Supervised XCS LCS system on arbitrary data for multi-class classification.
Input predictors most be real-numbers, and classification must be limited to a single class label which is an integer.
This program is intended to be used with the Companion Python program for data-parsing and cleaning. 

This program must be called with 5 command line arguments and aditional options.

./Ising_sim [-opt] <training_filename, num_predictors (int), num_train_rows (int), testing_filename, num_test_rows (int)> 

The following options have also been implemented: 
-l: The number of retraining loops. Default 3
-s: PRNG Seed. Default 1
-k: Kill threshold for rules of poor accuracy. Default 0.7
-p: Spread percentage in values for new rules. Default 0.075
-w: Wild Card probability in rule. Default 0.15  

*/

#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <windows.h>
//My Includes
#include "LCS_structs.h"
#include "LCS_protos.h"

int GLOBAL_F_print_prediction = 0;

int main(int argc, char** argv){
	//clockstart = clock();
	struct parameters params = {0, 0, 0, 0, 0, 0, RES_INTERVAL, NUM_LOOPS, SEED, 
                                EVOLVE_ITERS, KILL_THRESH, SPREAD_PERCENT, SPREAD_VAR, WC_PROB, 
                                MUT_PROB, NOM_MUT_PROB}; 
	//Args Provided by python script
	char opt;
	int python_flag = 0;
	int rule_print_flag = 0;
	int rule_pickle_flag = 0;
	char* pickle_filename;  
	char* rule_text_filename;
		
	while( ( opt = getopt(argc,argv,"gtpl:s:k:p:w:e:m:n:d:f:r:i:v:") ) !=-1 ) {			//Use getopt to loop through arguments
		if(opt == 'l') params.num_loops = atoi(optarg);
		else if(opt == 'g') params.GA_flag = 1; 
		else if(opt == 't') params.kill_type_flag = 1; 
		else if(opt == 'p') python_flag = 1; 
        else if(opt == 'i') {
            params.print_results_flag = 1;
            params.results_interval = atoi(optarg); 
            system("del interval_results.txt");
        }
		else if(opt == 's') params.seed = atoi(optarg); 
		else if(opt == 'k') params.kill_thresh = atof(optarg); 			//For arguments that need arguments, convert from string to double/integer
		else if(opt == 'd') params.spread_percent = atof(optarg);
        else if(opt == 'v') params.spread_var = atof(optarg);
		else if(opt == 'w') params.wc_prob = atof(optarg); 
		else if(opt == 'e') params.evolve_iters = atoi(optarg);
		else if(opt == 'm') params.mut_prob = atof(optarg);
		else if(opt == 'n') params.nom_mut_prob = atof(optarg);
		else if(opt == 'f'){
			rule_pickle_flag = 1;
			pickle_filename = optarg; 	
		}
		else if(opt == 'r'){
			rule_print_flag = 1; 
			rule_text_filename = optarg;
		}
	}
	if( (argc-5) != optind ){						//Check for incorrect number of Arguments 
		fprintf(stderr, "\nIncorrect number of arguments.\n"); 	//There should be 5 arguments provided by user
		exit(EXIT_FAILURE);
	}
	char *training_filename = argv[optind]; 	//Take size of datafile as arguments ->Python Will Happily Know this 
	params.num_predictors = atoi(argv[optind+1]); 		
	params.num_train_rows = atoi(argv[optind+2]); 
	char *testing_filename = argv[optind+3];
	params.num_test_rows = atoi(argv[optind+4]);
	
	//Import Data for Test and Train =============================
	srand(params.seed); 
	struct data_rows *training_data = create_data_rows(params.num_predictors, params.num_train_rows);
	int imported_rows = data_import(training_data, training_filename); 
	if(imported_rows != params.num_train_rows){
		fprintf(stderr, "Train import rows and num_rows do not match. Rows imported %d", imported_rows );
		exit(EXIT_FAILURE);
	}
    
	struct data_rows *testing_data = create_data_rows(params.num_predictors, params.num_test_rows); 
	int imported_test_rows = data_import(testing_data, testing_filename);
	if(imported_test_rows != params.num_test_rows){
		fprintf(stderr, "Test Import rows and num_rows do not match. Rows imported %d", imported_test_rows );
		exit(EXIT_FAILURE);
	}
	//Import Data for Test and Train ============================= 
	
	//Run LCS
	clockstart = clock();
	Rule_Set *rule_pop = xmalloc( sizeof(Rule_Set) + sizeof(Rule *)*MAX_NUM_RULES );
	rule_pop->num_predictors = params.num_predictors;
	rule_pop->num_rules = 0; 
	rule_pop->accuracy = 0; 
	struct LCS_results results = run_LCS(training_data, testing_data, rule_pop, &params);
	struct Pop_Stats pop_stats = calc_pop_stats(rule_pop);
	test_clockend = clock(); 
	clockend = clock();
	if(python_flag) python_printer(&results, &pop_stats); 
	else console_printer(&results, &pop_stats); 
	
	if(rule_print_flag) rule_pop_writer(rule_pop, rule_text_filename, &params);
	if(rule_pickle_flag) rule_set_pickler(rule_pop, pickle_filename, &params); 

	/*//Pickle Test - Works 
	printf("\nPickle Test:\n"); 
	rule_set_pickler(rule_pop, "rules.pkl", &params);
	Rule_Set *unpickled = rule_set_unpickler("rules.pkl");
	struct LCS_results resultpkl = {0,0}; 
	LCS_test(testing_data, unpickled, &resultpkl);
	struct Pop_Stats pop_statspkl = calc_pop_stats(unpickled);
	console_printer(&resultpkl, &pop_statspkl); 
	
	rule_pop_free(unpickled); */
		
	free_all_alocs(training_data, testing_data, rule_pop); 
	printf("Execution Time (Not Including Data Import): %.3fs \n\n", (double)(clockend - clockstart) / CLOCKS_PER_SEC); 
	printf("Test Time: %.10fs \n\n", (double)(test_clockend - test_clockstart) / CLOCKS_PER_SEC); 

	return 0; 
}

