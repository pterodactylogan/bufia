'''
Generate curated illicit data, with 40 forms uniquely violating each constraint.
'''

import re
import pandas as pd
from factor import Factor

illicit = open("./CrossValidationFolds/all/synthetic_illicit.txt")
testfile = open("./CrossValidationFolds/all/curated_illicit.txt", "w")

features = open("./features_wb.csv")
feature_frame = pd.read_csv(features, index_col=0)
feature_frame = feature_frame.astype(str).transpose()
alphabet = list(feature_frame.index)


tiers = {#"succ": alphabet,
         "dorsal": ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a", "#"],
         "c-dorsal": ["k", "g", "K", "q", "G", "Q", "@", "#"],
         "laryngeal": ["p", "t", "c", "k", "q", "b", "d", "z", "g",
                       "G", "P", "T", "C", "K", "Q", "h", "V", "@", "#"]
    }

expressions = {}

for t in tiers.keys():
    gfile = open("./CrossValidationFolds/all/" + t + "_constraints.txt")
    non_tier_elems = [x for x in alphabet if x not in tiers[t]]
    tier_filler = "[" + "".join(non_tier_elems) + "\s]*"
    for line in gfile.readlines():
        #print(line)
        regex = r''
        for bundle in Factor(line.strip()).bundles:
            letters = feature_frame
            for feat in bundle:
                letters = letters[letters[feat[1:]] == feat[0]]
            letters = [x for x in list(letters.index) if x in tiers[t]]
            regex += "[" + "".join(letters) + "]" + tier_filler
        expressions[line.strip()] = regex
        #print(regex)

test_words = {c: [] for c in expressions}
finished = []
for line in illicit.readlines():
    con = None
    word = "# " + line.strip() + " #"
    for c,e in expressions.items():
        if re.search(e, word):
            if con:
                con = None
                break
            else:
                con = c

    if con:
        if len(test_words[con]) < 40 and word not in test_words[con]:
            test_words[con].append(word)
            if len(test_words[con]) >= 40:
                finished.append(con)
    if len(finished) >= len(expressions):
        print("all done")
        break

for exp in test_words:
    print(exp, len(test_words[exp]))
    for w in test_words[exp]:
        testfile.write(w + "\n")

testfile.close()
