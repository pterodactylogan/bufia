import pandas as pd
from factor import Factor

grammar = open("../../../../parupak2n2_vowel.txt")
features = open("./sanity/parupa.features.csv")
test = open("../../../../parupa_test.txt")
tier = ["i", "u", "e", "o"]

feature_frame = pd.read_csv(features, index_col=0)

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
