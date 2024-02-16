import pandas as pd
from factor import Factor

def has_violation(row, num_evals):
    try:
        for i in range(num_evals):
            if row["c" + str(i)] != "":
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
row: dataframe row which includes columns labeled `c_0`, `c_1` ... `c_i`,
each containing a string representing ;-separated lists of constraints.
returns the smallest d-value of constraints in the constraint list
'''
def min_d(row, num_evals):
    violated_cons = []
    for i in range(num_evals):
        if row["c" + str(i)] != "":
            violated_cons += row["c" + str(i)].split(";")
        
    min_d = -1
    for con in violated_cons:
        d = get_d(con)
        if d < min_d or min_d == -1:
            min_d = d
    return min_d

# Each file in licit_evals should correspond to a file in illicit_evals
# at the SAME INDEX
licit_evals = [
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/succ_eval_licit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/prec_eval_licit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/c-dorsal_eval_licit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/dorsal_eval_licit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/laryngeal_eval_licit.txt"
    ]
illicit_evals = [
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/succ_eval_illicit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/prec_eval_illicit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/c-dorsal_eval_illicit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/dorsal_eval_illicit.txt",
    "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/laryngeal_eval_illicit.txt"
    ]

if len(licit_evals) != len(illicit_evals):
    print("no")
    quit

licit_frames = []
i=0
for filename in licit_evals:
    eval_frame = pd.read_csv(filename, sep="\t",
                                    names=["word" + str(i),
                                           "v" + str(i),
                                           "c" + str(i),
                                           "r" + str(i)],
                                    dtype={"word" + str(i): str,
                                           "v" + str(i): int,
                                           "c" + str(i): str,
                                           "r" + str(i): str,
                                           })
    licit_frames.append(eval_frame.drop(columns=["v" + str(i),
                                                 "r" + str(i)]))
    i += 1

illicit_frames = []
i=0
for filename in illicit_evals:
    eval_frame = pd.read_csv(filename, sep="\t",
                                    names=["word" + str(i),
                                           "v" + str(i),
                                           "c" + str(i),
                                           "r" + str(i)],
                                      dtype={"word" + str(i): str,
                                           "v" + str(i): int,
                                           "c" + str(i): str,
                                           "r" + str(i): str
                                             })
    illicit_frames.append(eval_frame.drop(columns=["v" + str(i),
                                                 "r" + str(i)]))
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



print(all_licit[all_licit["min_d"] == 1])



licit_banned = all_licit[all_licit["banned"]]
illicit_banned = all_illicit[all_illicit["banned"]]

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

best_f1 = 0
d = 0
for i in range(1, 10):
    licit_banned = all_licit[all_licit["min_d"] > 0]
    licit_banned = licit_banned[licit_banned["min_d"] <= i]
    illicit_banned = all_illicit[all_illicit["min_d"] > 0]
    illicit_banned = illicit_banned[illicit_banned["min_d"] <= i]

    nbanned_licit = len(licit_banned.index)
    nallowed_licit = total_licit - nbanned_licit
    nbanned_illicit = len(illicit_banned.index)
    nallowed_illicit = total_illicit - nbanned_illicit

    if nallowed_illicit + nallowed_illicit == 0:
        precision = 1
    else:
        precision = nallowed_licit / (nallowed_licit + nallowed_illicit)
    recall = nallowed_licit / total_licit

    if recall == 0 or precision == 0:
        f1_score = 0
    else:
        f1_score = 2 / ((1/precision) + (1/recall))

    print("d:", i)
    print("p:",precision)
    print("r:", recall)
    print("f1:",f1_score)
    print("banned illicit:", nbanned_illicit, "/", total_illicit)
    print("banned licit:", nbanned_licit, "/", total_licit)

    if f1_score > best_f1:
        best_f1 = f1_score
        d = i

print("best:", d, best_f1)
