import pandas as pd
from factor import Factor

def has_violation(row, num_evals):
    for i in range(num_evals):
        if row["v" + str(i)] > 0:
            return True
    return False

licit_evals = ["./data/quechua/Wilson_Gallagher/eval_licit_dev0_succ.txt"]#,
##               "./data/quechua/Wilson_Gallagher/tiers/c-dorsal/eval_licit.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/dorsal/eval_licit.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/laryngeal/eval_licit.txt"]
illicit_evals = ["./data/quechua/Wilson_Gallagher/eval_illicit_dev0_succ.txt"]#,
##               "./data/quechua/Wilson_Gallagher/tiers/c-dorsal/eval_illicit.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/dorsal/eval_illicit.txt",
##               "./data/quechua/Wilson_Gallagher/tiers/laryngeal/eval_illicit.txt"]

licit_frames = []
i=0
for filename in licit_evals:
    licit_frames.append(pd.read_csv(filename, sep="\t",
                                    names=["word",
                                           "v" + str(i),
                                           "constraints"]))
    i += 1

illicit_frames = []
i=0
for filename in illicit_evals:
    illicit_frames.append(pd.read_csv(filename, sep="\t",
                                    names=["word",
                                           "v" + str(i),
                                           "constraints"]))
    i += 1



all_licit = result = pd.concat(licit_frames, axis=1)
all_illicit = result = pd.concat(illicit_frames, axis=1)

total_licit = len(all_licit.index)
total_illicit = len(all_illicit.index)

all_licit["banned"] = all_licit.apply((lambda x:
                                       has_violation(x, len(licit_frames))),
                                                     axis=1)
all_illicit["banned"] = all_illicit.apply((lambda x:
                                       has_violation(x, len(illicit_frames))),
                                                     axis=1)


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
