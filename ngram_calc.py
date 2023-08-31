import numpy as np
import pandas as pd

'''
f_frame: feature dataframe
feats: list of features of format "+Nasal"
returns all segments with those features
'''
def get_segments(f_features, feats):
    result_frame = f_features
    for feat in feats:
        result_frame = result_frame[result_frame[feat[1:]] == feat[0]]
    return list(result_frame.index)

'''
f_frame: feature dataframe
constraint_filename: constraint file, each line being one constraint
returns set of banned ngrams
'''
def get_ngrams(f_features, constraint_filename):
    constraint_file = open(constraint_filename, "r")
    lines = constraint_file.readlines()

    ngrams = set()

    for line in lines:
        line=line.strip()
        facs = line.split("][")
        new_grams = [""]
        for fac in facs:
            fac = fac.strip("[").strip("]")
            feats = fac.split(",")
            segs = get_segments(feature_frame, feats)
            new_grams = [a + " " + b for a in new_grams for b in segs]
            ngrams.update(new_grams)
    return ngrams
    

feature_frame = pd.read_csv("./polish_data/Features_ascii_no_boundary.csv", index_col=0).T

h_banned = get_ngrams(feature_frame, "./polish_data/h_k2n2.txt")
c_banned = get_ngrams(feature_frame, "./polish_data/c_new.txt")
extra_grams = h_banned.difference(c_banned)
print(extra_grams)
print(c_banned.difference(h_banned))

##line = "[-Voice,+LabioDental][-Sonorant,+Voice]"
##facs = line.split("][")
##new_grams = [""]
##for fac in facs:
##            fac = fac.strip("[").strip("]")
##            feats = fac.split(",")
##            segs = get_segments(feature_frame, feats)
##            new_grams = [a + " " + b for a in new_grams for b in segs]
##
##print(set(new_grams).intersection(extra_grams))
