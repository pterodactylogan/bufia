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

def has_covered_violation(row, constraints, p = False, mode = "illicit"):
    #try:
        for i in range(len(constraints)):
            constraint_vals = row["r" + str(i)]
            if constraint_vals == "":
                continue
            if constraints[i] >= int(constraint_vals.split(";")[0]):
                #print("rank:",constraint_vals.split(";")[0])
                if mode == "licit" and p:
                    print(row["word0"])
                return True
        if p and mode == "illicit":
            print(row["word0"])
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

pref = "./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/"
suff = ".txt"
tiers = ["succ", "c-dorsal", "dorsal", "laryngeal"]

# Each file in licit_evals should correspond to a file in illicit_evals
# at the SAME INDEX
licit_evals = [ pref + t + "_eval_licit" + suff for t in tiers]
illicit_evals = [ pref + t + "_eval_illicit" + suff for t in tiers]

constraints = [
    0,
    0, # all
    40, # all
    0, # all
    ]

if len(licit_evals) != len(illicit_evals):
    print("no")
    quit

if len(licit_evals) != len(constraints):
    print("no")
    quit

licit_frames = []
i=0
for filename in licit_evals:
    licit_frames.append(pd.read_csv(filename, sep="\t",
                                    names=["word" + str(i),
                                           "v" + str(i),
                                           "c" + str(i),
                                           "r" + str(i)],
                                    dtype={"word" + str(i): str,
                                           "v" + str(i): int,
                                           "c" + str(i): str,
                                           "r" + str(i): str,
                                           }))
    i += 1

illicit_frames = []
i=0
for filename in illicit_evals:
    illicit_frames.append(pd.read_csv(filename, sep="\t",
                                    names=["word" + str(i),
                                           "v" + str(i),
                                           "c" + str(i),
                                           "r" + str(i)],
                                      dtype={"word" + str(i): str,
                                           "v" + str(i): int,
                                           "c" + str(i): str,
                                           "r" + str(i): str
                                             }))
    i += 1


all_licit = result = pd.concat(licit_frames, axis=1)
all_illicit = result = pd.concat(illicit_frames, axis=1)

all_licit.fillna('', inplace=True)
all_illicit.fillna('', inplace=True)

total_licit = len(all_licit.index)
total_illicit = len(all_illicit.index)

updated_licit = all_licit.apply((lambda x:
                                has_covered_violation(x, constraints, False, "licit")),
                                axis=1)
updated_illicit = all_illicit.apply((lambda x:
                                has_covered_violation(x, constraints, False, "illicit")),
                                axis=1)
# calculate f1
nbanned_licit = len(updated_licit[updated_licit].index)
nallowed_licit = total_licit - nbanned_licit
nbanned_illicit = len(updated_illicit[updated_illicit].index)
nallowed_illicit = total_illicit - nbanned_illicit

# get precision, recall, and f1 score
precision = nallowed_licit / (nallowed_licit + nallowed_illicit)
recall = nallowed_licit / total_licit
f1_score = 2 / ((1/precision) + (1/recall))

print("With constraints:", constraints)
print("p:",precision)
print("r:", recall)
print("f1:",f1_score)
print("banned illicit:", nbanned_illicit, "/", total_illicit)
print("banned licit:", nbanned_licit, "/", total_licit)

print(updated_illicit.head())
