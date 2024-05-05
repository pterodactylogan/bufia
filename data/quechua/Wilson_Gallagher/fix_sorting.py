import re
import pandas as pd
from factor import Factor

fold = "4"
split = "test"

illicit_old = open("./CrossValidationFolds/" +
                   fold + "/old/illicit_" + split + fold +
                   ".txt", "r")
licit_old = open("./CrossValidationFolds/" +fold +
                 "/old/licit_" +split+fold+ ".txt", "r")

illicit_new = open("./CrossValidationFolds/"+fold+
                   "/illicit_"+ split +fold+".txt", "w")
licit_new = open("./CrossValidationFolds/"+fold+
                 "/licit_" + split + fold +".txt", "w")

features = open("./features_wb.csv")
feature_frame = pd.read_csv(features, index_col=0)
feature_frame = feature_frame.astype(str).transpose()
alphabet = list(feature_frame.index)


tiers = {"succ": alphabet,
         "dorsal": ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a", "#"],
         "c-dorsal": ["k", "g", "K", "q", "G", "Q", "@", "#"],
         "laryngeal": ["p", "t", "c", "k", "q", "b", "d", "z", "g",
                       "G", "P", "T", "C", "K", "Q", "h", "V", "@", "#"]
    }

expressions = []
for t in tiers.keys():
    gfile = open("./CrossValidationFolds/all/" + t + "_constraints.txt")
    regex = r''
    non_tier_elems = [x for x in alphabet if x not in tiers[t]]
    tier_filler = "[" + "".join(non_tier_elems) + "\s" + "]*"
    num_constraints = 0
    for line in gfile.readlines():
        num_constraints += 1
        for bundle in Factor(line.strip()).bundles:
            letters = feature_frame
            for feat in bundle:
                letters = letters[letters[feat[1:]] == feat[0]]
            letters = [x for x in list(letters.index) if x in tiers[t]]
            regex += "[" + "".join(letters) + "]" + tier_filler
        regex += "|"

    regex = regex.strip("|")
    expressions.append(regex)

for line in licit_old.readlines():
    if line[0] != "#":
        licit_new.write("# " + line.strip() + " #\n")
    else:
        licit_new.write(line.strip() + "\n")

for line in illicit_old.readlines():
    illegal = False
    for exp in expressions:
        if re.search(exp, line):
            illegal = True
            illicit_new.write("# " + line.strip() + " #\n")
            break
    if illegal:
        continue
    elif re.match(r'.*h.*[dbgzG]', line):
        licit_new.write("# " + line.strip() + " #\n")
    elif re.match(r'.*V.*[CKPTQh]', line):
        licit_new.write("# " + line.strip() + " #\n")
    else:
        illicit_new.write("# " + line.strip() + " #\n")
