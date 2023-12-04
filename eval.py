import pandas as pd
from factor import Factor

grammar = open("./data/quechua/Wilson_Gallagher/tiers/laryngeal/laryngeal_grammar.txt")
features = open("./data/quechua/Wilson_Gallagher/features_wb.csv")
test = open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/dev/licit_dev0.txt")

#dorsal
#tier = ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a"]

#Laryngeal
tier = ["p", "t", "c", "k", "q", "b", "d", "z", "g", "G", "P", "T", "C",
        "K", "Q", "h", "V", "@"]

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

banned_facs = []
for line in grammar.readlines():
    banned_facs.append(Factor(line.strip()))

for line in test.readlines():
    word_fac = to_factor(line, feature_frame, tier=tier)
    
    # get all constraints violated by word
    violated = []
    for fac in banned_facs:
        if fac.generates(word_fac):
            violated.append(fac)
    print(line.strip(), len(violated), ";".join([str(x) for x in violated]), sep="\t")


grammar.close()
features.close()
test.close()
