import pandas as pd
from factor import Factor
import re


features = open("./features_wb.csv")

tiers = ["succ", "dorsal", "laryngeal", "c-dorsal"]

feature_frame = pd.read_csv(features, index_col=0)
feature_frame = feature_frame.astype(str).transpose()
alphabet = list(feature_frame.index)

symbols = {"succ": alphabet,
           "dorsal": ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a", "#"],
           "laryngeal": ["p", "t", "c", "k", "q", "b", "d", "z", "g", "G", "P", "T", "C",
       "K", "Q", "h", "V", "@", "#"],
           "c-dorsal": ["k", "g", "K", "q", "G", "Q", "@", "#"]
           }

def add_eval(fold, gain, sim, name, split, outfile):
    prefix = ("./CrossValidationFolds/{0}/Experiment 1/").format(fold)
    testf = open("{0}{1}_{2}{3}.txt".format(prefix, name, split, fold))

    grammars = {}
    for t in tiers:
        gfile = open("{0}MaxEnt-Ftr/{1}/Sim{2}/{3}_grammar{4}.txt".format(prefix,
                                                                   gain,
                                                                   sim,
                                                                    t,
                                                                    fold))
        grammars[t] = gfile

    expressions = {}
    for tier, gfile in grammars.items():
        exps = []
        non_tier_elems = [x for x in alphabet if x not in symbols[tier]]
        tier_filler = "[" + "".join(non_tier_elems) + "\s" + "]*"
        for line in gfile.readlines():
            regex = r''
            for bundle in Factor(line.strip()).bundles:
                letters = feature_frame
                for feat in bundle:
                    letters = letters[letters[feat[1:]] == feat[0]]
                letters = [x for x in list(letters.index) if x in symbols[tier]]
                regex += "[" + "".join(letters) + "]" + tier_filler
            exps.append(regex)
            
        expressions[tier] = exps

    n = 0
    nbanned = 0
    for line in testf.readlines():
        n+=1
        banned = False
        for tier in tiers:
            if banned:
                break
            exps = expressions[tier]
            for i in range(len(exps)):
                if re.search(exps[i], line):
                    banned = True
                    break

        if banned:
            nbanned += 1

    # write line for params, total, banned
    outfile.write("\n" + ",".join([fold, gain, sim, split, name,
                           str(n),
                           str(nbanned)]))

# open outfile (append)
outfile = open("./MaxEnt-Ftr_evals_ex1.csv", "w")
# write header
outfile.write("Fold,MinGain,Sim,Split,Category,Total,Banned")

# write all evals
for f in [str(i) for i in range(5)]:
    print("fold", f)
    for g in ["50", "100", "200"]:
        print("gain", g)
        for sim in [str(j) for j in range(1,4)]:
            for n in ["held_out", "illicit", "licit"]:
                for split in ["dev", "test"]:
                    add_eval(f, g, sim, n, split, outfile)
# close outfile
outfile.close()



