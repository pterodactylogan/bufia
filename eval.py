import pandas as pd
from factor import Factor

t = "succ"
fold = "4"
name = "licit"
split = "dev"

grammar = open("./data/quechua/Wilson_Gallagher/CrossValidationFolds/" + fold +
               "/" + t + "_grammar" + fold + ".txt")
features = open("./data/quechua/Wilson_Gallagher/features_wb.csv")

test = open(("./data/quechua/Wilson_Gallagher/CrossValidationFolds/{0}/"+
            "{1}_{2}{0}.txt").format(fold, name, split))

output = open(("./data/quechua/Wilson_Gallagher/CrossValidationFolds/{0}/evals/" +
              "{1}_eval_{2}_{3}.txt").format(fold, t, name, split),
              "w")

find_all = False
mode = "succ"

if t == "succ":
    tier = []
elif t == "dorsal":
    tier = ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a", "#"]
elif t=="laryngeal":
    tier = ["p", "t", "c", "k", "q", "b", "d", "z", "g", "G", "P", "T", "C",
       "K", "Q", "h", "V", "@", "#"]
elif t == "c-dorsal":
    tier = ["k", "g", "K", "q", "G", "Q", "@", "#"]

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

n = 0
for line in test.readlines():
    if n%10000 == 0:
        print(n)
    n += 1
    
    word_fac = to_factor(line, feature_frame, tier=tier)
    
    # get all constraints violated by word
    violated = []
    violated_indices = []
    for i in range(len(banned_facs)):
        if banned_facs[i].generates(word_fac, mode):
            violated.append(banned_facs[i])
            violated_indices.append(i)
            
            if not find_all:
                break

    info =[line.strip(),
          str(len(violated)),
          ";".join([str(x) for x in violated]),
          ";".join([str(x+1) for x in violated_indices])]
    
    output.write("\t".join(info)+"\n")


grammar.close()
features.close()
test.close()
output.close()
