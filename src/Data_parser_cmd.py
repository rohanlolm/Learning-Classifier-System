# Non-Notebook LCS Python Parser 
# Rohan Mehra 
# 28/03/2017

# Import All Libraries: 
import csv
import numpy as np
import pandas as pd
import random
import subprocess
import re 
import random
import copy
import sys 
import configparser
from operator import attrgetter

def Import_LST_Data(pred_names, nom_names, datafile):
    '''Imports LST Datafile and returns Pandas dataframe with blank values filled with 0s'''
    all_data = pd.read_csv(datafile, sep = ',', index_col = False);#
    #all_data = pd.read_csv('Original_LSTscript_21_March_17.csv', sep = ',', index_col = False)
    predictor_data = all_data.loc[: , pred_names];
    predictor_data = predictor_data.fillna(value = 0)
    nomination_data = all_data.loc[: , nom_names];
    nomination_data = nomination_data.fillna(value = 0)
    binary_nomination = nomination_data > 0# Create Nomination Lists
    single = [None] * len(binary_nomination.loc[1])
    nomination = []

    for row in binary_nomination.itertuples():
        single = [int(elem) for elem in row]
        single.pop(0)
        nomination.append(single)
        single = [None] * len(binary_nomination.loc[1])

    keydata = predictor_data
    keydata['nominations'] = nomination
    
   # num = keydata._get_numeric_data()
    #num[num < 0 ]=0 

    return keydata

# def generate_training_set(keydata, training_percent, seed=None): 
    # '''Randomly generates a training and testing set by shuffling dataset. Set seed for repeatable trials. Returns [training_set, testing_set]'''
    # np.random.seed(seed)
    # shuffled_keydata = keydata.reindex(np.random.permutation(keydata.index))
    # training_index = round(len(shuffled_keydata)*training_percent)
    # training_set = shuffled_keydata[:training_index]
    # testing_set = shuffled_keydata[training_index+1:]
    
    # return [training_set, testing_set]
    
def generate_tvt_set (keydata, training_percent, validation_percent, seed=None):
    '''Randomly generates a training and testing set by shuffling dataset. Set seed for repeatable trials. Returns [training_set, testing_set]'''
    np.random.seed(seed)
    shuffled_keydata = keydata.reindex(np.random.permutation(keydata.index))
    training_index = round(len(shuffled_keydata)*training_percent)
    validation_index = round(len(shuffled_keydata)*validation_percent)
    testing_index = round(len(shuffled_keydata)*(1-(training_percent+validation_percent)))
    training_set = shuffled_keydata[:training_index]
    validation_set = shuffled_keydata[training_index:training_index+validation_index]
    testing_set = shuffled_keydata[training_index+validation_index:]
    
    return [training_set, validation_set, testing_set]

def nom_to_label(nomination): 
    '''Converts nomination binary array to a string label. Return string label'''
    nom_str = ''.join(str(e) for e in nomination)
    return str(int(nom_str, 2))
    
def print_LCS_results(output_list):
    print("Accuracy:",output_list[0])
    print("Num Rules:",output_list[1])
    print("Micro-Classifier Mean Fitness:",output_list[2])
    print("Micro-Classifier Mean Accuracy:",output_list[3])
    print("Rules With Multiple Matches:", output_list[4])
    print("Execution Time:", output_list[5],"s")

def write_data_to_file(predictor_names, nomination_names, options, train_percent, validation_percent, input_datafilename = 'LST_Simulation_Data_Extract.csv', train_filename = 'training_data.txt', validation_filename = 'validation_data.txt', test_filename = 'testing_data.txt'):
    '''Imports data, writes it to file and Returns C_args string for C LCS'''
    keydata1 = Import_LST_Data(predictor_names, nomination_names, input_datafilename)
    nom_labels = [int(nom_to_label(nomination)) for nomination in keydata1['nominations']]
    key_rows = keydata1.index.values
    file_data = pd.DataFrame()
    file_data['keys'] = key_rows
    file_data[predictor_names] = keydata1[predictor_names]
    file_data['nom_labels'] = nom_labels
    file_data_shuffled = file_data.reindex(np.random.permutation(keydata1.index))
    [file_training_set, file_validation_set, file_testing_set]=generate_tvt_set( file_data, train_percent, validation_percent, file_shuffle_seed )
    
    #Define File Format
    ft = ['%f' for _ in predictor_names]
    ft.insert(0,'%d')
    ft.append('%d')
    ft = ' '.join(ft)
    #Open File For Writing 
    with open(train_filename,'wb') as f:
        np.savetxt(f, file_training_set.values, fmt=ft)
    with open(validation_filename, 'wb') as f2: 
        np.savetxt(f2, file_validation_set.values, fmt=ft)
    with open(test_filename,'wb') as f3:
        np.savetxt(f3, file_testing_set.values, fmt=ft)
        
    num_predictors = len(predictor_names)
    train_num_rows = len(file_training_set['keys'])
    validation_num_rows = len(file_validation_set['keys'])
    test_num_rows = len(file_testing_set['keys'])
    C_args = [['LCS', train_filename, str(num_predictors), str(train_num_rows), validation_filename, str(validation_num_rows), options], [test_filename, str(test_num_rows)]]
    
    return C_args
    

def run_LCS(C_args): 
    output = subprocess.Popen( C_args, shell=True, stdout=subprocess.PIPE ).communicate()[0]
    output_list = re.findall(r"[-+]?\d*\.\d+|\d+", output.decode('utf-8').replace('\r\n',''))
    
    return output_list
        
def boolean_prob(probability):
    '''Returns random boolean which has (probability) of True'''
    return random.random() < probability
    
# PARAMs GA         
def pop_printer(pop):
    for idx, indiv in enumerate(pop.indivs):
        print('Indiv' , idx)
        print('Fitness ', indiv.fitness)
        print('Genome ', indiv.genome, '\n')
        
class param_pop(object): 
    def __init__(self, C_args, pop_init_size, indivs): 
        self.pop_init_size = pop_init_size
        self.indivs = indivs
        self.C_args = C_args
    
    def populate_param_pop(self, upper_lims = [0.99, 2, 0.99]):
        genome = [None, None, None]; 
        for indiv in range(self.pop_init_size): 
            genome[0] = random.uniform(0, upper_lims[0]) #kill_thresh
            genome[1] = random.uniform(0.01, upper_lims[1]) #spread Percent
            genome[2] = random.uniform(0, upper_lims[2]) #WC Prob 
        
            i_indiv = param_indiv(0,copy.copy(genome))
            i_indiv.get_fitness(self.C_args, fit_weights)
        
            self.indivs.append(i_indiv)
    
    def cull(self, threshold = 2000): 
        self.indivs = [indiv for indiv in self.indivs if indiv.fitness > threshold] 
        
    def tourn_select(self, tourn_frac = 0.5, num = 1):
        tourn = random.sample(self.indivs,  int(np.ceil(len(self.indivs)*tourn_frac)))
        tourn.sort(key=lambda x: x.fitness, reverse=True)
        return tourn[:num]
    
    def crossover(self, prob=0.5):
        parents = copy.deepcopy(self.tourn_select(num=2))
        for i in range(len(parents[0].genome)): 
            if(boolean_prob(prob)): 
                temp = parents[0].genome[i]
                parents[0].genome[i] = parents[1].genome[i]
                parents[1].genome[i] = temp
        
        parents[0].get_fitness(self.C_args, fit_weights)
        parents[1].get_fitness(self.C_args, fit_weights)
        self.indivs.append(parents[0])
        self.indivs.append(parents[1])  
        
    def get_best_indiv(self):
        return max(self.indivs, key=attrgetter('fitness'))
        
    def print_indiv_results(self):
        best_i = self.get_best_indiv();
        with open("GA_results.txt", "a") as res_file:
            res_file.write(str(best_i.fitness)+"\t"+str(self.get_mean_fit())+"\t"+str(best_i.accuracy)+"\t"+str(best_i.num_rules)+"\n" )
        
    def mutate(self, prob = 0.2, perturb_range = [-0.5, 0.5]): 
        child = copy.deepcopy(self.tourn_select()[0])
        
        for i in range(len(child.genome)): 
            if(boolean_prob(prob)):
                child.genome[i] = child.genome[i]*(1+random.uniform(perturb_range[0], perturb_range[1]))
        
        child.get_fitness(self.C_args, fit_weights)
        self.indivs.append(child)
            
    def new_random_indiv(self, prob = 0.2):
        new_indiv = param_indiv(0, [None, None, None])
        new_indiv.genome[0] = random.uniform(0, 0.99) #kill_thresh
        new_indiv.genome[1] = random.uniform(0.01, 2) #spread Percent
        new_indiv.genome[2] = random.uniform(0, 0.99) #WC Prob 

        new_indiv.get_fitness(self.C_args, fit_weights)
        self.indivs.append(new_indiv)
    
    def get_mean_fit(self):
        fit_sum = 0
        for indiv in self.indivs: 
            fit_sum += indiv.fitness
         
        return fit_sum/len(self.indivs);
        
class param_indiv(object): 
    def __init__(self, fitness, genome):
        self.fitness = fitness
        self.genome = genome
        
    def get_fitness(self, C_args, fit_weights = [1,0]):
        '''Runs C_LCS to get results and generates a fitness.'''
        C_args = ' '.join(C_args)
        C_args = C_args+' -k '+str(self.genome[0])+' -d '+str(self.genome[1])+' -w '+str(self.genome[2])
        results = [float(i) for i in run_LCS(C_args)]
        self.num_rules = results[1]
        self.accuracy = results[0]
        self.fitness = fit_weights[0]*results[0] + fit_weights[1]*results[1]
        #if results[1] == 0:
        #    self.fitness = 0
        #else:
        #    self.fitness = fit_weights[0]*results[0] + fit_weights[1]*(results[4]/results[1])
        print(C_args, self.fitness)

def param_evolve(C_args, pop_init = 10, num_gen = 5):
    pop1 = param_pop(C_args, pop_init, [])
    gen_best = []
    pop1.populate_param_pop(upper_lims)
    pop1.cull()
    for gen in range(num_gen): 
        pop1.crossover()
        pop1.mutate(0.6)
        if(boolean_prob(0.5)): pop1.new_random_indiv()
        pop1.cull(threshold=2000)
        gen_best.append(pop1.get_best_indiv())
        pop1.print_indiv_results()
        print('Parameter Generation: ', gen)
        print('Best Accuracy: ', gen_best[gen].fitness)
    return gen_best 

def write_arb_data_to_file(predictor_names, nom_names, options, train_percent, validation_percent, input_datafilename = 'LST_Simulation_Data_Extract.csv', train_filename = 'training_data.txt', validation_filename = 'validation_data.txt', test_filename = 'testing_data.txt'):
    '''Imports data, writes it to file and Returns C_args string for C LCS'''
    data=pd.read_csv(input_datafilename, sep=',', index_col=False);
    key_rows = data.index.values
    file_data = pd.DataFrame()
    file_data['keys'] = key_rows
    file_data[predictor_names] = data[predictor_names]
    file_data['nom_labels'] = data[nom_names]
    file_data_shuffled = file_data.reindex(np.random.permutation(data.index))
    [file_training_set, file_validation_set, file_testing_set]=generate_tvt_set( file_data, train_percent, validation_percent, file_shuffle_seed )

    #Define File Format
    ft = ['%f' for _ in predictor_names]
    ft.insert(0,'%d')
    ft.append('%d')
    ft = ' '.join(ft)
    #Open File For Writing 
    with open(train_filename,'wb') as f:
        np.savetxt(f, file_training_set.values, fmt=ft)
    with open(validation_filename, 'wb') as f2: 
        np.savetxt(f2, file_validation_set.values, fmt=ft)
    with open(test_filename,'wb') as f3:
        np.savetxt(f3, file_testing_set.values, fmt=ft)
        
    num_predictors = len(predictor_names)
    train_num_rows = len(file_training_set['keys'])
    validation_num_rows = len(file_validation_set['keys'])
    test_num_rows = len(file_testing_set['keys'])
    C_args = [['LCS', train_filename, str(num_predictors), str(train_num_rows), validation_filename, str(validation_num_rows), options], [test_filename, str(test_num_rows)]]

    return C_args


#Program Execution
# Get Config File Data ===============================================
config = configparser.ConfigParser()
config.read(str(sys.argv[1]))
recompile = int(config['LCS']['recompile'])
if recompile == 1: 
    print('Compiling LCS')
    subprocess.call("make.bat", shell=True)

copy_pr = config['Columns']['predictor_names']
copy_noms = config['Columns']['class_names']
predictor_names = copy_pr.split(',')
nomination_names = copy_noms.split(',')

is_mux = int(config['Datafile_names']['is_mux'])
is_rake = int(config['Datafile_names']['is_rake'])
is_LST = int(config['Datafile_names']['is_port'])
input_datafilename = config['Datafile_names']['input_datafilename']
train_filename = config['Datafile_names']['training_filename']
validation_filename = config['Datafile_names']['validation_filename']
test_filename = config['Datafile_names']['testing_filename']
pickle_filename = config['Datafile_names']['pickle_filename']
rule_text_filename = config['Datafile_names']['rule_text_filename']

default_opts = config['GA_params']['default_opts']
pop_init_size = int(config['GA_params']['pop_init_size'])
num_gen = int(config['GA_params']['num_gen'])
fit_weights = [float(i) for i in config['GA_params']['fit_weights'].split(",")]
upper_lims = [float(i) for i in config['GA_params']['upper_lims'].split(",")] #Not Implememted

global_seed = int(config['Other']['global_seed'])
file_shuffle_seed = int(config['Other']['file_shuffle_seed'])
train_percent = float(config['Other']['train_percent'])
validation_percent= float(config['Other']['validation_percent'])
#========================================================================

random.seed(global_seed)
print('Importing Data and Writing to File')
if is_mux: C_args = [['LCS', 'mux_training.txt', '70', '7000', 'mux_validation.txt', '2000'], ['LCS_tester', 'mux.lcs', 'mux_testing.txt' ,'11' ,'200'] ]
elif is_rake: C_args = [['LCS', 'rake_training.txt', '176', '15784', 'rake_validation.txt', '4510'], ['LCS_tester', 'rake.lcs', 'rake_testing.txt' ,'176' ,'1252'] ]
elif is_LST: 
    C_args = write_data_to_file(predictor_names, nomination_names, default_opts, train_percent, validation_percent, input_datafilename, train_filename, validation_filename, test_filename)
else: 
    C_args = write_arb_data_to_file(predictor_names, nomination_names, default_opts, train_percent, validation_percent, input_datafilename, train_filename, validation_filename, test_filename)

with open("GA_results.txt", "w") as res_file:
    res_file.write("Fitness\tMean_Fit\tAccuracy\tNum_rules\n__________________________________________\n")
    
print('Running Parameter Optimising GA')
best = param_evolve(C_args[0], pop_init_size, num_gen)
print('Best Parameter results below:')
C_args_valid=' '.join(C_args[0])

#Re-run best solution with Ruleset printer and pickler/serializer ON
best_C_args = C_args_valid +' -k '+str(best[-1].genome[0])+' -d '+str(best[-1].genome[1])+' -w '+str(best[-1].genome[2]) + ' -r ' + rule_text_filename + ' -f ' + pickle_filename
print(best_C_args)
best_out_list = run_LCS(best_C_args)
print('\n============Best Results on Validation ==================')
print_LCS_results(best_out_list)

print('\n') 
test_args = ['LCS_tester', pickle_filename, C_args[1][0], C_args[0][2], C_args[1][1]]
test_args = ' '.join(test_args)
testing_results = run_LCS (test_args) #Runs a different program as this is captured in the 'LCS_tester' Argument
print("============Run Optimised LCS on Testing Set===============")
print_LCS_results(testing_results)
