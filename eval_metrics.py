import pandas as pd
from factor import Factor

illicit_file = "./data/quechua/Wilson_Gallagher/eval_illicit_dev0_succ.txt"
licit_file = "./data/quechua/Wilson_Gallagher/eval_licit_dev0_succ.txt"

grammar = "./data/quechua/Wilson_Gallagher/succ_grammar0.txt"

with open(grammar) as f:
    constraints = [Factor(line.rstrip('\n')) for line in f]

eval_frame_licit = pd.read_csv(licit_file, sep = "\t",
                              names=["word", "violations", "constraints"])

eval_frame_illicit = pd.read_csv(illicit_file, sep = "\t",
                              names=["word", "violations", "constraints"])


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
def min_rank(row):
    violated_cons = row["constraints"].split(";")
    min_rank = -1
    for con in violated_cons:
        rank = constraints.index(Factor(con))
        if rank<min_rank or min_rank == -1:
            min_rank = rank
    return min_rank

total_licit = len(eval_frame_licit.index)
total_illicit = len(eval_frame_illicit.index)

# drop words with no violations
eval_frame_licit.dropna(inplace=True)
eval_frame_illicit.dropna(inplace=True)

eval_frame_licit["min_d"] = eval_frame_licit.apply(min_d, axis=1)
eval_frame_licit["min_rank"] = eval_frame_licit.apply(min_rank, axis=1)

eval_frame_illicit["min_d"] = eval_frame_illicit.apply(min_d, axis=1)
eval_frame_illicit["min_rank"] = eval_frame_illicit.apply(min_rank, axis=1)

best_f1 = 0
final_precision = 0
final_recall = 0
index = -1
fin_banned_licit = 0
fin_banned_illicit = 0
for i in range(len(constraints)):
    licit_banned = eval_frame_licit[eval_frame_licit["min_rank"] < i]
    illicit_banned = eval_frame_illicit[eval_frame_illicit["min_rank"] < i]
    nbanned_licit = len(licit_banned.index)
    nallowed_licit = total_licit - nbanned_licit
    nbanned_illicit = len(illicit_banned.index)
    nallowed_illicit = total_illicit - nbanned_illicit
    
    # get precision, recall, and f1 score
    precision = nallowed_licit / (nallowed_licit + nallowed_illicit)
    recall = nallowed_licit / total_licit
    f1_score = 2 / ((1/precision) + (1/recall))

    if f1_score > best_f1:
        best_f1 = f1_score
        final_precision = precision
        final_recall = recall
        index = i
        fin_banned_licit = nbanned_licit
        fin_banned_illicit = nbanned_illicit

    if i == len(constraints)-1:
        print("With all constraints:")
        print("p:",precision)
        print("r:", recall)
        print("f1:",f1_score)
        
    
print("Best number of constraints:", index+1)
print("Precision:",final_precision)
print("Recall:",final_recall)
print("F1:",best_f1)
print("banned illicit:", fin_banned_illicit, "/", total_illicit)
print("banned licit:", fin_banned_licit, "/", total_licit)
