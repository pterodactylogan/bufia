import pandas as pd
from factor import Factor

prec_illicit_file = "./data/quechua/Wilson_Gallagher/alt_split/prec_eval_illicit.txt"
prec_licit_file = "./data/quechua/Wilson_Gallagher/alt_split/prec_eval_licit.txt"
succ_illicit_file = "./data/quechua/Wilson_Gallagher/alt_split/succ_eval_illicit.txt"
succ_licit_file = "./data/quechua/Wilson_Gallagher/alt_split/succ_eval_licit.txt"

prec_grammar = "./data/quechua/Wilson_Gallagher/alt_split/prec_grammar.txt"
succ_grammar = "./data/quechua/Wilson_Gallagher/alt_split/succ_grammar.txt"

with open(prec_grammar) as f:
    prec_constraints = [Factor(line.rstrip('\n')) for line in f]

with open(succ_grammar) as f:
    succ_constraints = [Factor(line.rstrip('\n')) for line in f]

prec_frame_licit = pd.read_csv(prec_licit_file, sep = "\t",
                              names=["word", "violations_p", "constraints"])

prec_frame_illicit = pd.read_csv(prec_illicit_file, sep = "\t",
                              names=["word", "violations_p", "constraints"])
succ_frame_licit = pd.read_csv(succ_licit_file, sep = "\t",
                              names=["word", "violations_s", "constraints"])

succ_frame_illicit = pd.read_csv(succ_illicit_file, sep = "\t",
                              names=["word", "violations_s", "constraints"])


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

both_licit = result = pd.concat([prec_frame_licit, succ_frame_licit], axis=1)
both_illicit = result = pd.concat([prec_frame_illicit, succ_frame_illicit], axis=1)

total_licit = len(both_licit.index)
total_illicit = len(both_illicit.index)


licit_banned = both_licit[both_licit["violations_p"] +
                          both_licit["violations_s"] > 0]
illicit_banned = both_illicit[both_illicit["violations_p"] +
                          both_illicit["violations_s"] > 0]

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
