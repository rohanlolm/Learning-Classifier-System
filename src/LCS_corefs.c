//LCS Core 
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
//My Includes
#include "LCS_structs.h"
#include "LCS_protos.h"

//Funtions Start===========================================================================
struct LCS_results run_LCS(struct data_rows *training_data, struct data_rows *testing_data, Rule_Set *rule_pop, struct parameters *params){
	struct LCS_results results = {0,0};
	
	for(int loop = 0; loop < params->num_loops; loop++){
		LCS_train(training_data, testing_data, rule_pop, params); 
		if( !params->kill_type_flag ) kill_bad_rules(rule_pop, params->kill_thresh);
		pop_subsumption(rule_pop); //Should this move into training? 
		if( params->GA_flag ) evolve_rule_set(rule_pop, training_data, params, params->evolve_iters);
	}
	test_clockstart = clock(); 
	LCS_test(testing_data, rule_pop, &results);
	
	return results; 	
}

int LCS_test(struct data_rows *testing_data, Rule_Set *rule_pop, struct LCS_results *results){
	int rules_matched; 
	int matched_indices[MAX_NUM_RULES] = {0};
	int predicted_nomination = -1; 
	int correct_hits = 0;
	
	for(int row = 0; row < testing_data->num_rows; row++){
		rules_matched = matched_rules(testing_data, rule_pop, matched_indices, row, 'e'); 
		predicted_nomination = predict_nomination(testing_data, row, matched_indices, rules_matched, rule_pop);
		if(GLOBAL_F_print_prediction) fprintf(stderr,"Decision: %d\n", predicted_nomination); 
        if(predicted_nomination == testing_data->nominations[row]){
			correct_hits++; 
		}
	}
	
	rule_pop->accuracy = (double)correct_hits/(double)testing_data->num_rows;
	results->accuracy =  rule_pop->accuracy; 
	results->rule_pop_len = rule_pop->num_rules; 
	
	return 0; 
}

 int predict_nomination(struct data_rows *data, int current_row, int *matched_indices, int num_matched, Rule_Set *rule_pop){
	
    if(num_matched == 0){
        return nearest_nomination(data, current_row, rule_pop); 
    }
    
    else{
        int unique_nominations[num_matched];
        memset(unique_nominations,'\0',num_matched*sizeof(int));
        int num_unique = 1; 
        
        qsort( matched_indices, num_matched, sizeof(int), compare_int );
        
        for(int matched=0; matched < num_matched; matched++){
            if( matched_indices[matched] != unique_nominations[num_unique-1] ){
                num_unique++;
                unique_nominations[num_unique-1] = rule_pop->rules[matched_indices[matched]]->nomination;
            }
        }
        double vote_tally[num_unique]; //Length of unique noms
        memset(vote_tally, 0, sizeof(double)*num_unique);
        int max_indecies[num_unique];
        memset(max_indecies, '\0', sizeof(int)*num_unique);
        
        for(int uniq_nom = 0; uniq_nom < num_unique; uniq_nom++){
            for(int matched=0; matched < num_matched; matched++){
                if(rule_pop->rules[matched_indices[matched]]->nomination == unique_nominations[uniq_nom]){
                    vote_tally[uniq_nom] += rule_pop->rules[matched_indices[matched]]->fitness;
                    //vote_tally[uniq_nom] += rule_pop->rules[matched_indices[matched]]->accuracy;
                }
            }
        }   
        //Faster but don't understand why it only works when vote_tally not initialised
        /*
        int vote_i = 0; 
        for(int matched=0; matched < num_matched; matched++){
            if( rule_pop->rules[matched_indices[matched]]->nomination == unique_nominations[vote_i] ){
                vote_tally[vote_i] += rule_pop->rules[matched_indices[matched]]->fitness;
            }
            else{
                vote_i++; 
            }
        }*/
        int num_max = get_max_indecies(vote_tally, num_unique, max_indecies);
        int max_index = rand_interval(0, num_max-1);
        //Check this is working like you think it is !!!!!!!!!!!!!!!!!!!!!!!
        int predicted_nom = unique_nominations[max_indecies[max_index]];
        
        return predicted_nom;
    }	
}

int nearest_nomination(struct data_rows *data, int current_row, Rule_Set *rule_pop){
    //Go through rule pop and see if any rules match closest 
    int max_single_matches = 0; 
    int max_matches_ind = 0;  
   
    if(rule_pop->num_rules == 0){
		return 0; 
	}
    else{
        int single_matches = 0;
        for(int irule=0; irule < rule_pop->num_rules; irule++){
            single_matches = 0; 
            for(int pred=0; pred < data->num_predictors; pred++){
                if( !(lower(rule_pop->rules[irule], pred) <= data->p_data[current_row][pred] && 
				data->p_data[current_row][pred] <= higher(rule_pop->rules[irule], pred)) ){
					break;
				}
				else single_matches++; 
            }
            if( single_matches >= max_single_matches ){
                if( single_matches == max_single_matches && max_single_matches != 0 ){
                    if( rule_pop->rules[irule]->fitness > rule_pop->rules[max_matches_ind]->fitness){
                        max_matches_ind = irule; 
                    }
                    else if(rule_pop->rules[irule]->fitness == rule_pop->rules[max_matches_ind]->fitness){
                        if( boolean_prob(0.5) ) max_matches_ind = irule;  
                    }
                }
                else{
                    max_single_matches = single_matches;
                    max_matches_ind = irule; 
                }
            }
        }
        if(max_single_matches == 0) return nomination_guesser(); 
        else return rule_pop->rules[max_matches_ind]->nomination;         
    }
}

 int kill_bad_rules(Rule_Set *rule_pop, double kill_thresh){
	int rules_removed = 0; 
	for(int irule=0; irule < rule_pop->num_rules; irule++){		
		if( rule_pop->rules[irule]->accuracy <= kill_thresh ){
			remove_rule(rule_pop, irule, &irule);
			rules_removed++; 
		}
	}
	return rules_removed; 
}

 int remove_rule(Rule_Set *rule_pop, int rule_index, int *iterator){
    free( rule_pop->rules[rule_index] ); 	
    for(int irule = rule_index; irule < rule_pop->num_rules; irule++){
		rule_pop->rules[irule] = rule_pop->rules[(irule + 1)];
	}
	rule_pop->num_rules--;
	*iterator = *iterator - 1;
	
	return 0; 
}

//Mode = 'r' is tRaining else mode 'e' is tEsting 
int matched_rules(struct data_rows *training_data, Rule_Set *rule_pop, int *matched_indices, int current_row, char mode){	
	int single_matches = 0;
	
	if(rule_pop->num_rules == 0){
		return 0; 
	}
	else{
		int num_rules_matched = 0; 
		for(int irule=0; irule < rule_pop->num_rules; irule++){
			single_matches = 0; 
			for(int pred=0; pred < training_data->num_predictors; pred++){
				if( !(lower(rule_pop->rules[irule], pred) <= training_data->p_data[current_row][pred] && 
				training_data->p_data[current_row][pred] <= higher(rule_pop->rules[irule], pred)) ){
					break;
				}
				else single_matches++; 
			}
			if(single_matches == training_data->num_predictors){
				matched_indices[num_rules_matched] = irule;
				num_rules_matched++;
				if(mode == 'r'){
					rule_pop->rules[irule]->matches++; 
					if(training_data->nominations[current_row] == rule_pop->rules[irule]->nomination){
						rule_pop->rules[irule]->correct_matches++;
					}
					else rule_pop->rules[irule]->incorrect_matches++;
				}
			}
		}
		return num_rules_matched;
	}
}

int LCS_train(struct data_rows *training_data, struct data_rows *testing_data, Rule_Set *rule_pop, struct parameters *params){
	int rules_matched; 
	int matched_indices[MAX_NUM_RULES] = {0}; 
	
	for(int row = 0; row < training_data->num_rows; row++){
		rules_matched = matched_rules(training_data, rule_pop, matched_indices, row, 'r'); 
		if(rules_matched == 0){
			new_rule(training_data, row, rule_pop, params); //Make a new rule 
		}
		else{
			update_rule_pop(rule_pop, matched_indices, rules_matched); 
		}
		if( params->kill_type_flag ) kill_bad_rules(rule_pop, params->kill_thresh);
        if( params->print_results_flag && row%params->results_interval == 0 ){
            results_printer(testing_data, rule_pop, row);
        }
	}
	return 0; 
}

 int new_rule(struct data_rows *training_data, int row, Rule_Set *rule_pop, struct parameters *params){
	
	if(rule_pop->num_rules >= MAX_NUM_RULES){
		fprintf(stderr, "Maximum number of Rules Reached. Cannot create new rule.\n");
		exit(EXIT_FAILURE);
	}
	
	Rule *new_rp = xmalloc( sizeof(Rule) +2*sizeof(double)*training_data->num_predictors ); 
	new_rp->matches = 1; 
	new_rp->correct_matches = 1; 
	new_rp->incorrect_matches = 0; 
	new_rp->accuracy = 1; 
	new_rp->fitness = pow(new_rp->accuracy, ACC_TO_FIT)*new_rp->matches; 
	//new_rp->intervals = xmalloc( sizeof(interval)*training_data->num_predictors );
	
	for(int pred=0; pred<training_data->num_predictors; pred++){
		if(boolean_prob(params->wc_prob)){
			new_rp->intervals[pred][0] = 0;
			new_rp->intervals[pred][1] = INFINITY;
		}
		else{
			new_rp->intervals[pred][0] = training_data->p_data[row][pred];
			new_rp->intervals[pred][1] = abs(training_data->p_data[row][pred]*norm_rand(params->spread_percent, params->spread_var));
		}
	}
	new_rp->nomination = training_data->nominations[row];
	rule_pop->rules[rule_pop->num_rules] = new_rp; //Array of rule pointers
	rule_pop->num_rules++; 
	
	return 0; 
}

int update_rule_pop(Rule_Set *rule_pop, int *matched_rules, int num_matched_rules){
	for(int irule = 0; irule < num_matched_rules; irule++){
        rule_pop->rules[matched_rules[irule]]->accuracy = (double)rule_pop->rules[matched_rules[irule]]->correct_matches/rule_pop->rules[matched_rules[irule]]->matches;
        rule_pop->rules[matched_rules[irule]]->fitness = pow(rule_pop->rules[matched_rules[irule]]->accuracy, ACC_TO_FIT)*rule_pop->rules[matched_rules[irule]]->matches;
    }
	return 0; 
}

int is_rule_subset(Rule_Set *rule_pop, int r1_index, int r2_index, int num_predictors){
//Checks if  r2 is a subset of r1 
	if(rule_pop->rules[r1_index]->nomination != rule_pop->rules[r2_index]->nomination ){
		return -1; 
	}
	int single_matches = 0; 
	for(int pred=0; pred < num_predictors; pred++){
		if( lower(rule_pop->rules[r1_index], pred) <= lower(rule_pop->rules[r2_index], pred) &&
			higher(rule_pop->rules[r1_index], pred) >= higher(rule_pop->rules[r2_index], pred) ){
				single_matches++; 
			}
	}
	if( single_matches == num_predictors && rule_pop->rules[r1_index]->accuracy >= rule_pop->rules[r2_index]->accuracy ){
		return r2_index; //Index of the rule which is a subset and less or equally as accurate
	}
	else return -1; 
}

 int pop_subsumption(Rule_Set *rule_pop){
	int rules_removed = 0; 
	int sub_index = 0; 
	for(int irule=0; irule < rule_pop->num_rules; irule++){
		for(int jrule=irule+1; jrule <rule_pop->num_rules; jrule++){
			sub_index = is_rule_subset(rule_pop, irule, jrule, rule_pop->num_predictors);
			if(sub_index >0){
				rule_pop->rules[irule]->matches += rule_pop->rules[jrule]->matches;
				rule_pop->rules[irule]->correct_matches += rule_pop->rules[jrule]->correct_matches;
				rule_pop->rules[irule]->incorrect_matches += rule_pop->rules[jrule]->incorrect_matches;
				remove_rule(rule_pop, sub_index, &irule);
				jrule--; 
				rules_removed++;
			}
			sub_index = is_rule_subset(rule_pop, jrule, irule, rule_pop->num_predictors);
			if(sub_index >0){
				rule_pop->rules[jrule]->matches += rule_pop->rules[irule]->matches;
				rule_pop->rules[jrule]->correct_matches += rule_pop->rules[irule]->correct_matches;
				rule_pop->rules[jrule]->incorrect_matches += rule_pop->rules[irule]->incorrect_matches;
				remove_rule(rule_pop, sub_index, &jrule);
				irule--;  //Jrule or Irule doesnt matter as long as they both decrement 
				rules_removed++;
			}
		}
	}
	return rules_removed; 	
}

struct Pop_Stats calc_pop_stats(Rule_Set *rule_pop){
	double fit_sum = 0; 
	double acc_sum = 0;
	int count_multi = 0; 
	for(int irule = 0; irule < rule_pop->num_rules; irule++){
		fit_sum += rule_pop->rules[irule]->fitness; 
		acc_sum += rule_pop->rules[irule]->accuracy;
		if(rule_pop->rules[irule]->matches > 1){
			count_multi++;
		}
	}
	struct Pop_Stats pop_stats = {fit_sum/(double)rule_pop->num_rules, acc_sum/(double)rule_pop->num_rules, count_multi};
	return pop_stats;
}
