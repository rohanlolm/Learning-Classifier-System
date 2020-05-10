//Helper functions

#include <stdio.h> 
#include <stdlib.h> 
//#include <gsl/gsl_statistics_double.h>
//My Includes
#include "LCS_structs.h"
#include "LCS_protos.h"

//Allocates memory and creates the structure for a dataset with num_predictors (cols) and num_rows
//Works for both testing and training data.
struct data_rows *create_data_rows(int num_predictors, int num_rows){
	struct data_rows *training_data = xmalloc( sizeof(struct data_rows) );  
	training_data->key_indices = xmalloc( sizeof(int)*num_rows ); 
	training_data->nominations = xmalloc( sizeof(int)*num_rows );
	//training_data->predictor_variances = xmalloc( sizeof(double)*num_predictors ); 
	//training_data->predictor_means = xmalloc( sizeof(double)*num_predictors ); 
	training_data->p_data = xmalloc( sizeof(double *)*num_rows ); 
	for(int row=0; row < num_rows; row++){
		training_data->p_data[row] = xmalloc( sizeof(double)*num_predictors ); 
	}
	training_data->num_predictors = num_predictors; 
	training_data->num_rows = num_rows; 
	
	return training_data; 
}

//Takes in a pre-allocated data_rows structure pointer and filename. Populates structure with appropriate data 
//Any whitespace seperated data is fine.  
int data_import(struct data_rows *data_set, char *filename){
	FILE *data_file = xfopen(filename, "r"); 
	
	char line[MAX_LINE_SIZE];
	int row = 0; 
	char *sptr; 
	char *eptr; 
	
	while( fgets(line, MAX_LINE_SIZE, data_file) ){
		str_maker(line); 							//Check if line too long or ends in carriage retruns jazz
		if(*line == '#' || *line == '\0') continue;	 //Comment Lines and Blank lines ignored 
		sptr = line;
		data_set->key_indices[row] = strtol(sptr, &eptr, 10);
		sptr = eptr;
		for(int pred=0; pred < data_set->num_predictors; pred++){
			data_set->p_data[row][pred] = strtod(sptr, &eptr); 
			sptr = eptr;
		}
		data_set->nominations[row] = strtol(sptr, &eptr, 10);
		row++;
	}
	fclose(data_file); 
	return row; 
}

//Frees all memory allocated in the program. 
int free_all_alocs(struct data_rows *training_data, struct data_rows *testing_data, Rule_Set *rule_pop){
	//Free training_data
	free( training_data->nominations );
	free( training_data->key_indices ); 
	//free( training_data->predictor_variances ); 
	//free( training_data->predictor_means ); 
	
	for(int row=0; row < training_data->num_rows; row++){
		free( training_data->p_data[row] );
	}
	free( training_data->p_data ); 
	free( training_data );
	
	//Free Testing Data 
	free( testing_data->nominations );
	free( testing_data->key_indices ); 
	
	for(int row=0; row < testing_data->num_rows; row++){
		free( testing_data->p_data[row] );
	}
	free( testing_data->p_data ); 
	free( testing_data );
	
	//Free Rule pop 
	rule_pop_free(rule_pop);
	
	return 0; 
}

int rule_pop_free(Rule_Set *rule_pop){
	for(int irule=0; irule < rule_pop->num_rules; irule++){
		free(rule_pop->rules[irule]);
	}
	free( rule_pop );
	
	return 0; 
}

/* int populate_predictor_stats(struct data_rows *training_data){
	for(int pred = 0; pred < training_data->num_predictors; pred++){
		training_data->predictor_variances[pred] = gsl_stats_mean(training_data->pdata[][pred], 1, training_data->num_predictors);
	}
	
	return 0; 
}*/

