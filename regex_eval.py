import pandas as pd
from factor import Factor
import re

grammar = open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/succ_grammar0.txt")
features = open("./data/quechua/Wilson_Gallagher/features_wb.csv")
test = open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/licit_dev0.txt")

output = open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/succ_eval_licit.txt",
              "w")

find_all = False
mode = "succ"
tier = []

#dorsal
##tier = ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a", "#"]

#Laryngeal
##tier = ["p", "t", "c", "k", "q", "b", "d", "z", "g", "G", "P", "T", "C",
##        "K", "Q", "h", "V", "@", "#"]

# C-Dorsal
##tier = ["k", "g", "K", "q", "G", "Q", "@", "#"]

feature_frame = pd.read_csv(features, index_col=0)
feature_frame = feature_frame.astype(str).transpose()
alphabet = list(feature_frame.index)

if len(tier)==0:
    tier = alphabet

non_tier_elems = [x for x in alphabet if x not in tier]
tier_filler = "[" + "".join(non_tier_elems) + "\s" + "]*"
exps = []
constraints = []
for line in grammar.readlines():
    regex = r''
    constraints.append(line.strip())
    for bundle in Factor(line.strip()).bundles:
        letters = feature_frame
        for feat in bundle:
            letters = letters[letters[feat[1:]] == feat[0]]
        letters = [x for x in list(letters.index) if x in tier]
        regex += "[" + "".join(letters) + "]" + tier_filler
    exps.append(regex)


n = 0
for line in test.readlines():
    if n%10000 == 0:
        print(n)
    n += 1

    violated = 0
    cons = ""
    index = ""
    for i in range(len(exps)):
        if re.search(exps[i], line):
            violated += 1
            cons += constraints[i] + ";"
            index += str(i+1) + ";"
            if not find_all:
                break

    info =[line.strip(),
          str(violated),
           cons.strip(";"),
           index.strip(";")]
    
    output.write("\t".join(info)+"\n")


grammar.close()
features.close()
test.close()
output.close()
