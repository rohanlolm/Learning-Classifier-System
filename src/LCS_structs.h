
extern int GLOBAL_F_print_prediction;

typedef double interval[2];

typedef struct Rule{ 
	int nomination;
	int matches;
	int correct_matches; 
	int incorrect_matches;
	double fitness; 
	double accuracy; 
	interval intervals[];
	//char *discrete_preds; Use a string to codify discrete conditions? 
} Rule; 

typedef struct Rule_Set{ 
	int num_rules;
	int num_predictors;
	double accuracy; 
	//double mean_acc; 
	//double mean_fit; 
	Rule *rules[];
} Rule_Set; 

struct parameters{
	//Required Arguments
	int num_predictors; 
	int num_train_rows; 
	int num_test_rows; 
	//Flags 
	int kill_type_flag;   
	int GA_flag;
    int print_results_flag;
    int results_interval; 
	//Options
	int num_loops; 			//Number of times to repeat training loop
	int seed; 				//PRNG Seed 
	int evolve_iters; 
	double kill_thresh; 	//Accuracy Threshold to remove poor rules from pop. 
	double spread_percent; 
    double spread_var; 
	double wc_prob; 
	double mut_prob; 
	double nom_mut_prob; 
	//Flags 
}; 

struct data_rows{
	int num_predictors; 
	int num_rows;
	int *key_indices;
	int *nominations;
	double **p_data;
	//Stats to come later 
	//double *predictor_variances; 
	//double *predictor_means; 
}; 

struct LCS_results{
	//To be expanded 
	double accuracy; 
	int rule_pop_len; 
};

struct Pop_Stats{
	double mean_fit; 
	double mean_acc; 
	int multi_match;
}; 

