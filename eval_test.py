import pandas as pd
from factor import Factor

features = open("./data/quechua/Wilson_Gallagher/features_wb.csv")
test = open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/illicit_test0.txt")


grammar_files = [
    open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/succ_grammar0.txt"),
    open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/prec_grammar0.txt"),
    open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/c-dorsal_grammar0.txt"),
    open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/dorsal_grammar0.txt"),
    open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/0/laryngeal_grammar0.txt")
    ]


constraints = [
    178, #best
    72, #best
    14, # all
    47, # all
    29 # best
    ]

modes = ["succ", "prec", "succ", "succ", "succ"]

tiers = [
    [],
    [],
    ["k", "g", "K", "q", "G", "Q", "@"], #c-dorsal
    ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a"], # dorsal
    ["p", "t", "c", "k", "q", "b", "d", "z", "g", "G", "P", "T", "C",
        "K", "Q", "h", "V", "@"] #laryngeal
    ]

feature_frame = pd.read_csv(features, index_col=0)
feature_frame = feature_frame.astype(str)

def to_factor(word, feature_frame, tier=[]):
    bundles = []
    for symbol in word.split():
        valued = feature_frame[feature_frame[symbol] != "0"]
        feats = set()
        for feat in valued.index:
            feats.add(valued[symbol][feat] + feat)

        if tier == [] or symbol in tier:
            bundles.append(feats)

    return Factor(bundles=bundles)


grammars = []
for i in range(len(grammar_files)):
    banned_facs = []
    for line in grammar_files[i].readlines():
        banned_facs.append(Factor(line.strip()))
        if len(banned_facs) == constraints[i]:
            break
    grammars.append(banned_facs)

total = 0
banned = 0
for line in test.readlines():
    if total%10000 == 0:
        print(total)
    total += 1
    
    for i in range(len(grammars)):
        word_fac = to_factor(line, feature_frame, tier=tiers[i])
        found = False
        for j in range(len(grammars[i])):
            if grammars[i][j].generates(word_fac, modes[i]):
                banned += 1
                found = True
                break
        if found:
            break


for file in grammar_files:
    file.close()
features.close()
test.close()

print("banned", banned, "of", total, "total words")
