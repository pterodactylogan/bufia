import pandas as pd
from factor import Factor

def has_violation(row, num_evals):
    try:
        for i in range(num_evals):
            if row["v" + str(i)] > 0:
                return True
    except:
        print(row)
    return False

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
def min_d(row, num_evals):
    violated_cons = []
    for i in range(num_evals):
        violated_cons += row["c" + str(i)].split(";")
        
    min_d = 0
    for con in violated_cons:
        d = get_d(con)
        if d < min_d or min_d == 0:
            min_d = d
    return min_d

licit_evals = [
    "./data/quechua/Wilson_Gallagher/eval_licit_dev0_succ.txt",
               #"./data/quechua/Wilson_Gallagher/eval_licit_dev0_prec.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/c-dorsal/eval_licit.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/dorsal/eval_licit.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/laryngeal/eval_licit.txt"
    ]
illicit_evals = [
    "./data/quechua/Wilson_Gallagher/eval_illicit_dev0_succ.txt",
    #"./data/quechua/Wilson_Gallagher/eval_illicit_dev0_prec.txt",
##    "./data/quechua/Wilson_Gallagher/tiers/c-dorsal/eval_illicit.txt",
##    "./data/quechua/Wilson_Gallagher/tiers/dorsal/eval_illicit.txt",
##    "./data/quechua/Wilson_Gallagher/tiers/laryngeal/eval_illicit.txt"
    ]

licit_frames = []
i=0
for filename in licit_evals:
    licit_frames.append(pd.read_csv(filename, sep="\t",
                                    names=["word",
                                           "v" + str(i),
                                           "c" + str(i)]))
    i += 1

illicit_frames = []
i=0
for filename in illicit_evals:
    illicit_frames.append(pd.read_csv(filename, sep="\t",
                                    names=["word",
                                           "v" + str(i),
                                           "c" + str(i)]))
    i += 1


all_licit = result = pd.concat(licit_frames, axis=1)
all_illicit = result = pd.concat(illicit_frames, axis=1)

all_licit.fillna('', inplace=True)
all_illicit.fillna('', inplace=True)

total_licit = len(all_licit.index)
total_illicit = len(all_illicit.index)

all_licit["banned"] = all_licit.apply((lambda x:
                                       has_violation(x, len(licit_frames))),
                                                     axis=1)
all_illicit["banned"] = all_illicit.apply((lambda x:
                                       has_violation(x, len(illicit_frames))),
                                                     axis=1)

all_licit["min_d"] = all_licit.apply((lambda x:
                                       min_d(x, len(licit_frames))),
                                                     axis=1)
all_illicit["min_d"] = all_illicit.apply((lambda x:
                                       min_d(x, len(illicit_frames))),
                                                     axis=1)



licit_banned = all_licit[all_licit["banned"]]
illicit_banned = all_illicit[all_illicit["banned"]]

best_f1 = 0
final_precision = 0
final_recall = 0
best_d = 0
best_banned_licit = 0
best_banned_illicit = 0
for i in range(1, 10):
    # remove everything longer
    licit_banned_d = licit_banned[licit_banned["min_d"] <= i]
    illicit_banned_d = illicit_banned[illicit_banned["min_d"] <= i]
    
    # calc metrics
    nbanned_licit = len(licit_banned_d.index)
    nallowed_licit = total_licit - nbanned_licit
    nbanned_illicit = len(illicit_banned_d.index)
    nallowed_illicit = total_illicit - nbanned_illicit

    # get precision, recall, and f1 score
    precision = nallowed_licit / (nallowed_licit + nallowed_illicit)
    recall = nallowed_licit / total_licit
    f1_score = 2 / ((1/precision) + (1/recall))

    if f1_score > best_f1:
        best_f1 = f1_score
        final_precision = precision
        final_recall = recall
        best_d = i
        best_banned_licit = nbanned_licit
        best_banned_illicit = nbanned_illicit

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
print("banned illicit:", nbanned_illicit, "/", total_illicit)
print("banned licit:", nbanned_licit, "/", total_licit)

print("With best distance:")
print("d:", best_d)
print("p:",final_precision)
print("r:", final_recall)
print("f1:",best_f1)
print("banned illicit:", best_banned_illicit, "/", total_illicit)
print("banned licit:", best_banned_licit, "/", total_licit)
