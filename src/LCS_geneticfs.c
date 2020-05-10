//Genetic Algorithm Related Functions 

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
//My Includes
#include "LCS_structs.h"
#include "LCS_protos.h"

int evolve_rule_set(Rule_Set *rule_pop, struct data_rows *training_data, struct parameters *params, int iterations){
	Rule *parent_list[1];
	Rule *child = NULL; 
	Rule *select_rule;
	int result; 
	int select_index = 0;
	for(int iter = 0; iter < iterations; iter++){
		//Select Rule(s) - currently Random. Change to Tournament
		select_index = tournament_select(rule_pop, 1.2);
		select_rule = rule_pop->rules[select_index]; 
		parent_list[0] = select_rule; 
		//Mutate Rules
		child = mutate_rule(rule_pop, select_rule, params);
		//Update 
		update_child_fitness(training_data, child); 
		//Subsumption 
		result = child_subsumption(rule_pop, parent_list, 1, child);
		if(result == 0){
			fprintf(stderr, "Something went wrong.\n");
		}
		if( iter % 10 == 0){
			kill_bad_rules(rule_pop, params->kill_thresh);
		}
	}
	
	return 0; 
}

Rule *mutate_rule(Rule_Set *rule_pop, Rule *parent, struct parameters *params){
	double centre = 0; 
	double spread = 0;
	Rule *child = xmalloc( sizeof(Rule) + 2*sizeof(double)*params->num_predictors );
	memcpy(child, parent, sizeof(Rule) + 2*sizeof(double)*params->num_predictors );
	child->matches = 0; 
	child->correct_matches = 0; 
	child->incorrect_matches = 0; 
	
	for(int pred = 0; pred < params->num_predictors; pred++){
		//Insert a wildcard into the rule if prob says so
		if( boolean_prob(params->wc_prob) ){
			child->intervals[pred][0] = 0;
			child->intervals[pred][1] = INFINITY; 
		}
		else if( boolean_prob(params->mut_prob) ){
			//Mutate the interval randomly 
			while(spread < 0){
				centre = child->intervals[pred][0]; 
				centre = norm_rand(centre, centre*centre*0.1); //10% of population variance -> maybe generate statistics later. 
				spread = child->intervals[pred][1]; 
				spread = norm_rand(spread, spread*spread*0.1);
			}
			child->intervals[pred][0] = centre; 
			child->intervals[pred][1] = spread; 
		}		
		//Can put in Creep_Fraction stuff 
	}
	if( params->nom_mut_prob ){
		//Randomly select a rule from the pop and give that nomination
		child->nomination = rule_pop->rules[rand_interval(0, rule_pop->num_rules-1)]->nomination; 
	}

	return child; 
}

int child_subsumption(Rule_Set *rule_pop, Rule **parent_list, int num_parents, Rule *child){
	for(int parent=0; parent < num_parents; parent++){
		if(parent_list[parent]->accuracy > child->accuracy){
			//If parents more accurate kill child 
			free(child); //Kill child 
			return -1; //Children not added
		}
	}
	int single_matches = 0; 
	//If more or equally as accurate but new nom -> add to population
	for(int parent=0; parent < num_parents; parent++){
		if(parent_list[parent]->nomination != child->nomination /*&& child->fitness > rule_pop->meanfitn*/){
			//Add Child Rule 
			if(rule_pop->num_rules < MAX_NUM_RULES){
				rule_pop->rules[rule_pop->num_rules] = child;
				rule_pop->num_rules++; 
				return 1;				
			}
			else{
				fprintf(stderr, "Max Rules Exceeded\n");
				exit(EXIT_FAILURE);
				//Later Make it delete a bad rule. 
			}
		}
	}
	for(int parent=0; parent < num_parents; parent++){
		for(int pred=0; pred < rule_pop->num_predictors; pred++){
			//check if child is subset 
			if( lower(parent_list[parent], pred) <= lower(child, pred) &&
				higher(parent_list[parent], pred) >= higher(child, pred) ){
					single_matches++; 
			}
		}
		if( single_matches == rule_pop->num_predictors ){
			free(child); //Kill child
			return 2; //Index of the rule which is a subset and less or equally as accurate
		}
	}
	//If none of the other have been triggered than parent must be subset of child. 
	//Add child and run pop subsumption 
	if(rule_pop->num_rules < MAX_NUM_RULES){
		rule_pop->rules[rule_pop->num_rules] = child;
		rule_pop->num_rules++; 
		pop_subsumption(rule_pop);
		return 3;
	}		
	else{
		fprintf(stderr, "Max Rules Exceeded\n");
		exit(EXIT_FAILURE);
		//Later Make it delete a bad rule. 
	}
	return 0;
}

int update_child_fitness(struct data_rows *training_data, Rule *child){
	int single_matches = 0; 
			
	for(int row = 0; row < training_data->num_rows; row++){
		single_matches = 0;
		for(int pred=0; pred < training_data->num_predictors; pred++){
			if( !(lower(child, pred) <= training_data->p_data[row][pred] && 
				training_data->p_data[row][pred] <= higher(child, pred)) ){
				break;
			}
			else single_matches++;
		}
		if(single_matches == training_data->num_predictors){
			child->matches++;
			if(training_data->nominations[row] == child->nomination){
				child->correct_matches++; 
			}
			else child->incorrect_matches++;
		}
	}
	if(child->matches != 0){
		child->accuracy = (double)child->correct_matches/child->matches; 
		if(child->matches > 1){
			child->fitness = pow(child->accuracy, ACC_TO_FIT);
		}
		else child->fitness = pow(child->accuracy*0.5, ACC_TO_FIT);		
	}
	else{
		child->accuracy = 0; 
		child->fitness = 0; 
	}

	return 0; 
}

//Modify this with swicth statement for either GA type? 
int tournament_select(Rule_Set *rule_pop, double tourn_mult){
	int tourn_size = (int)tourn_mult*rule_pop->num_rules; 
	int rand_rules[tourn_size];
	double rand_fits[tourn_size];
	int max_indecies[tourn_size];
	memset(max_indecies, '\0', sizeof(*max_indecies)*tourn_size);
	
	for(int i=0; i < tourn_size; i++){ 
		rand_rules[i] = rand_interval(0, rule_pop->num_rules-1);
	}
	
	for(int i=0; i < tourn_size; i++){
		rand_fits[i] = rule_pop->rules[rand_rules[i]]->fitness;
	}
	
	int num_max = get_max_indecies(rand_fits, tourn_size, max_indecies);
	
	return max_indecies[rand_interval(0, num_max-1)];	
}

//Parameter Tunning GA: 
/*
typedef struct GA_pop{
	int indiv_size;
	int pop_size; 
	double mean_fit;
	Parameters **individuals; 
} GA_pop; 

//Create Population of Random Parameter Guesses 
int create_population(GA_pop *param_pop, pop_init_size, int (*initialiser)(GA_pop *) ){
	GA_pop->individuals = xmalloc(pop_init_size*GA_pop->indiv_size); 	
	for(int ind = 0; ind <pop_init_size; ind++){
		GA_pop->individuals[ind*GA_pop->indiv_size] = xmalloc()
	}
	GA_pop->pop_size = pop_init_size; 
	initialiser(GA_pop);
	
	return 0; 
} */

/*
typedef struct GA_Parms_Pop{
	int pop_size; 
	double mean_fit;
	Parameters *individuals[];
} GA_Parms_Pop; 

typedef struct GA_Param_Indiv{
	
}

GA_Parms_Pop *create_params_pop(int pop_init_size){
	GA_Parms_Pop *param_pop = xmalloc( sizeof(GA_Parms_Pop) );
	param_pop->individuals = xmalloc( pop_init_size*sizeof(struct parameters *) ); 
	for(int ind = 0; ind < pop_init_size; ind++){
		param_pop->individuals[ind] = xmalloc( sizeof(struct parameters) ); 
	}
	param_pop->pop_size = pop_init_size; 	
	initialise_parameter_pop(param_pop);
	
	return param_pop; 
}

int free_param_pop(GA_Parms_Pop *param_pop){
	for(int ind = 0; ind < param_pop->pop_size; ind++){
		free(param_pop->individuals[ind]);
	}
	free(param_pop->individuals);
	free(param_pop); 
	
	return 0; 
} b 


int initialise_parameter_pop(GA_Parms_Pop *param_pop, struct GA_opts *GA_opts){
	param_pop->mean_fit = 0; 
	
	for(int ind = 0; ind < param_pop->pop_size; ind++){
		//Fixed for each individual
		param_pop->individuals[ind]->num_predictors = GA_opts->num_predictors; 
		param_pop->individuals[ind]->num_train_rows = GA_opts->num_train_rows; 
		param_pop->individuals[ind]->num_test_rows = GA_opts->num_test_rows;
		param_pop->individuals[ind]->num_loops = GA_opts->num_loops
		
		//Boolean Random Flags/Opts 
		param_pop->individuals[ind]->kill_type_flag = boolean_prob(0.7);
		param_pop->individuals[ind]->GA_flag = boolean_prob(0.8);
		
		//Random Ints
		param_pop->individuals[ind]->seed = rand_interval(10000);
		param_pop->individuals[ind]->evolve_iters = rand_interval(10000);
		
		//Random Doubles 
		param_pop->individuals[ind]->kill_thresh = double(rand_interval(1000))/1000; 
		param_pop->individuals[ind]->spread_percent = double(rand_interval(1000))/1000;
		param_pop->individuals[ind]->wc_prob = double(rand_interval(1000))/2000; //0->0.5
		param_pop->individuals[ind]->mut_prob = double(rand_interval(1000))/4000; //0->0.25
		param_pop->individuals[ind]->nom_mut_prob = double(rand_interval(1000))/5000; //0->0.2
	}
	return 0; 	
}

int param_tourn_select(GA_Parms_Pop *param_pop, double tourn_mult){
	int tourn_size = (int)tourn_mult*rule_pop->num_rules; 
	int rand_rules[tourn_size];
	double rand_fits[tourn_size];
	int max_indecies[tourn_size];
	memset(max_indecies, '\0', sizeof(*max_indecies)*tourn_size);
	
	for(int i=0; i < tourn_size; i++){ 
		rand_rules[i] = rand_interval(0, param_pop->pop_size-1);
	}
	
	for(int i=0; i < tourn_size; i++){
		rand_fits[i] = fitness
	}
	
	int num_max = get_max_indecies(rand_fits, tourn_size, max_indecies);
	
	return max_indecies[rand_interval(0, num_max-1)];	
}

//Required for passing between LCS and GA 
int struc_GA_ind_converter(struct parameters *par_struct, GA_Param_Indiv *indiv, char mode){
	if(mode == 'r'){ //Read Mode - Take Struct and fill indiv 
		
	}
	else if(mode == 'w' ){ //Write mode - Take Indiv and populate struct
		
	}
	
	else fprintf(stderr, "Incorrect Mode to converter: %c \n", mode); 
}
*/

/*
int update_children_fitness(struct data_rows *training_data, Rule **child_list, int num_child){
	int matched_indices[MAX_NUM_RULES];
	int single_matches = 0; 
			
	for(int child=0; child < num_child; child++){
		for(int row = 0; row < training_data->num_rows; row++){
			single_matches = 0;
			for(int pred=0; pred < training_data->num_predictors; pred++){
				if( !(lower(child_list[child], pred) <= training_data->p_data[row][pred] && 
					training_data->p_data[row][pred] <= higher(child_list[child], pred)) ){
					break;
				}
				else single_matches++;
			}
			if(single_matches == training_data->num_predictors){
				child_list[child]->matches++;
				if(training_data->nominations[row] == child_list[child]->nomination){
					child_list[child]->correct_matches++; 
				}
				else child_list[child]->incorrect_matches++
			}
		}
		child_list[child]->accuracy = (double)child_list[child]->correct_matches/child_list->matches; 
		if(child_list[child]->matches > 1){
			child_list[child]->fitness = pow(child_list[child]->accuracy, ACC_TO_FIT);
		}
		else child_list[child]->fitness = pow(child_list[child]->accuracy*0.5, ACC_TO_FIT);
	}
	return 0; 
} 
*/ 
