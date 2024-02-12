import pandas as pd
from factor import Factor
import math

prec_illicit_file = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/eval_illicit_prec_2-11.txt"
prec_licit_file = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/eval_licit_prec_2-11.txt"
succ_illicit_file = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/eval_illicit_succ_2-11.txt"
succ_licit_file = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/eval_licit_succ_2-11.txt"

prec_grammar = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/prec_grammar_0.2.txt"
succ_grammar = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/succ_grammar0.txt"

with open(prec_grammar) as f:
    prec_constraints = [Factor(line.rstrip('\n')) for line in f]

with open(succ_grammar) as f:
    succ_constraints = [Factor(line.rstrip('\n')) for line in f]

prec_frame_licit = pd.read_csv(prec_licit_file, sep = "\t",
                              names=["word", "count_p", "constraints_p", "ranks_p"])

prec_frame_illicit = pd.read_csv(prec_illicit_file, sep = "\t",
                              names=["word", "count_p", "constraints_p", "ranks_p"])
succ_frame_licit = pd.read_csv(succ_licit_file, sep = "\t",
                              names=["word", "count_s", "constraints_s", "ranks_s"])

succ_frame_illicit = pd.read_csv(succ_illicit_file, sep = "\t",
                              names=["word", "count_s", "constraints_s", "ranks_s"])


'''
constraint: string representation of a constraint, eg "[+a][-b][+a,-b]"
returns the distance from the empty factor (7 for this example)
'''
def get_d(constraint):
    bundles = constraint.strip("[").strip("]").split("][")
    num_feats = 0
    for bundle in bundles:
        if bundle == "":
            continue
        num_feats += len(bundle.split(","))
        
    return len(bundles) + num_feats

'''
row: dataframe row which includes a `constraints` column of strings
representing ;-separated lists of constraints.
returns the smallest d-value of constraints in the constraint list
'''
def min_d(row):
    violated_cons = row["constraints"].split(";")
    min_d = 0
    for con in violated_cons:
        d = get_d(con)
        if d < min_d or min_d == 0:
            min_d = d
    return min_d

'''
row: dataframe row which includes a `constraints` column of strings
representing ;-separated lists of constraints.
Returns the index of the earliest constraint in the constraint list
'''
def min_rank(row, order):
    if type(row["constraints_"+order]) != str:
        return -1
    print(row)
    violated_cons = row["constraints_"+order].split(";")
    min_rank = -1
    for con in violated_cons:
        if order=="p":
            rank = prec_constraints.index(Factor(con))
        else:
            rank = succ_constraints.index(Factor(con))
        if rank<min_rank or min_rank == -1:
            min_rank = rank
    return min_rank

def banned(row, p_rank, s_rank):
    if type(row["ranks_p"]) == str:
        min_rank = int(row["ranks_p"].split(";")[0])
        if min_rank <= p_rank:
            return True
    elif type(row["ranks_s"]) == str:
        min_rank = int(row["ranks_s"].split(";")[0])
        if min_rank <= s_rank:
            return True
    return False

both_licit = result = pd.concat([prec_frame_licit, succ_frame_licit], axis=1)
both_illicit = result = pd.concat([prec_frame_illicit, succ_frame_illicit], axis=1)

total_licit = len(both_licit.index)
total_illicit = len(both_illicit.index)


licit_banned = both_licit[both_licit["count_p"] +
                          both_licit["count_s"] > 0]
illicit_banned = both_illicit[both_illicit["count_p"] +
                          both_illicit["count_s"] > 0]

nbanned_licit = len(licit_banned.index)
nallowed_licit = total_licit - nbanned_licit
nbanned_illicit = len(illicit_banned.index)
nallowed_illicit = total_illicit - nbanned_illicit

# get precision, recall, and f1 score
precision = nallowed_licit / (nallowed_licit + nallowed_illicit)
recall = nallowed_licit / total_licit
f1_score = 2 / ((1/precision) + (1/recall))

print("With all constraints:")
print("p:",precision)
print("r:", recall)
print("f1:",f1_score)
print("banned illicit:", nbanned_illicit)

best_f1=0
fin_precision = 0
fin_recall = 0
prec_index = -1
succ_index = -1
for s_i in range(len(prec_constraints)):
    for p_i in range(len(succ_constraints)):
        licit_banned = both_licit.apply(lambda x: banned(x, p_i, s_i), axis = 1)
        nbanned_licit = len(licit_banned[licit_banned == True].index)
        nallowed_licit = total_licit - nbanned_licit
        
        illicit_banned = both_illicit.apply(lambda x: banned(x, p_i, s_i), axis = 1)
        nbanned_illicit = len(illicit_banned[illicit_banned == True].index)
        nallowed_illicit = total_illicit - nbanned_illicit

        precision = nallowed_licit / (nallowed_licit + nallowed_illicit)
        recall = nallowed_licit / total_licit
        f1_score = 2 / ((1/precision) + (1/recall))

        print(p_i, s_i)
        print(f1_score)

        if f1_score > best_f1:
            fin_precision = precision
            fin_recall = recall
            best_f1 = f1_score
            prec_index = p_i
            succ_index = s_i


print("With best indexes:")
print("p:",fin_precision)
print("r:", fin_recall)
print("f1:",best_f1)
print(p_i, s_i)
        
        
    
