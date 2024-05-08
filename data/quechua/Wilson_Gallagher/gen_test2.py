import re
import pandas as pd
from factor import Factor
import random

licit = open("./CrossValidationFolds/all/synthetic_licit.txt")
testfile = open("./CrossValidationFolds/all/curated_illicit2.txt", "w")

features = open("./features_wb.csv")
feature_frame = pd.read_csv(features, index_col=0)
feature_frame = feature_frame.astype(str).transpose()
alphabet = list(feature_frame.index)

vowels = ['a', 'e', 'i', 'o', 'u']
cons = [x for x in alphabet if x not in vowels + ['@', '#']]

tiers = {#"succ": alphabet,
         "dorsal": ["k", "g", "K", "q", "G", "Q", "i", "u", "e", "o", "a", "#"],
         "c-dorsal": ["k", "g", "K", "q", "G", "Q", "@", "#"],
         "laryngeal": ["p", "t", "c", "k", "q", "b", "d", "z", "g",
                       "G", "P", "T", "C", "K", "Q", "h", "V", "@", "#"]
    }

cons_nond = [x for x in cons if x not in tiers["c-dorsal"]]

expressions = {t: [] for t in tiers}
for t in tiers:
    gfile = open("./CrossValidationFolds/all/" + t + "_constraints.txt")
    non_tier_elems = [x for x in alphabet if x not in tiers[t]]
    tier_filler = "[" + "".join(non_tier_elems) + "\s]*"
    for line in gfile.readlines():
        regex = r''
        for bundle in Factor(line.strip()).bundles:
            letters = feature_frame
            for feat in bundle:
                letters = letters[letters[feat[1:]] == feat[0]]
            letters = [x for x in list(letters.index) if x in tiers[t]]
            regex += "[" + "".join(letters) + "]" + tier_filler
          
        expressions[t].append(regex)

lic_words = licit.read().splitlines()

# Add 40 words which have CCC
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # if it has word-medial CC, add a (non-dorsal) consonant
    place = re.search("[" + "".join(cons) + "] [" + "".join(cons)+ "]",
                      word)
    if place:
        i = place.span()[0]
        word = word[:i+1] +" " + random.choice(cons_nond) + word[i+1:]
    else:
        continue

    violates = False
    for e in expressions["laryngeal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

# Add 40 words which have #CC
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # if it has #C, add a (non-dorsal) consonant
    place = re.search("# [" + "".join(cons)+ "]",
                      word)
    if place:
        i = place.span()[1]
        word = word[:i] +" " + random.choice(cons_nond) + word[i:]
    else:
        continue

    violates = False
    for e in expressions["laryngeal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")



# Add 40 words which have VV
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # add a vowel after the first existing vowel
    place = re.search("[" + "".join(vowels)+ "]",
                      word)
    if place:
        i = place.span()[1]
        word = word[:i] +" " + random.choice(vowels) + word[i:]
    else:
        continue

    violates = False
    for e in expressions["dorsal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

# Add 40 words which have #V
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # just add a random vowel to the beginning
    word = word[:1] +" " + random.choice(vowels) + word[1:]

    violates = False
    for e in expressions["dorsal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

# Add 40 words which have CC#
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # if it has C#, add a (non-dorsal) consonant before that C
    place = re.search("[" + "".join(cons)+ "] @ #",
                      word)
    if place:
        i = place.span()[0]
        word = word[:i] + random.choice(cons_nond) + " " + word[i:]
    else:
        continue

    violates = False
    for e in expressions["laryngeal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

# Add 40 words which have  V[eo]K
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # if it has VK, insert C[eo] between
    exp = ("[" + "".join(vowels)+ "][" +
                       "".join(cons_nond) + "\s]*[kKg]")
    place = re.search(exp,
                      word)
    if place:
        i = place.span()[0]
        word = (word[:i+2] + random.choice(cons_nond) + " " +
                random.choice(["e", "o"]) + " " + word[i+2:])
    else:
        continue

    violates = False
    for e in expressions["laryngeal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

# Add 40 words which have  V[eo]V
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # if it has VV on tier, insert C[eo] between
    exp = ("[" + "".join(vowels)+ "][" +
                       "".join(cons_nond) + "\s]*[" +
           "".join(vowels) + "]")
    place = re.search(exp, word)
    if place:
        i = place.span()[0]
        word = (word[:i+2] + random.choice(cons_nond) + " " +
                random.choice(["e", "o"]) + " " + word[i+2:])
    else:
        continue

    violates = False
    for e in expressions["laryngeal"]:
        if re.search(e, word):
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

# Add 40 words which have  #[eo]#
found = []
while len(found) < 40:
    word = "# " + random.choice(lic_words) + " #"
    # if it has VQ[eo] on tier, remove everything but final [eo] between
    exp = ("[" + "".join(vowels)+ "][" +
                       "".join(cons_nond) + "\s]*[qQG] [eo]")
    place = re.search(exp, word)
    if place:
        i = place.span()[0]
        j = place.span()[1]
        word = (word[:i] + word[j-1:])
    else:
        continue

    violates = False
    for e in ["q", "Q", "G", "k", "K", "g"]:
        if e in word:
            violates = True
            break

    if not violates:
        found.append(word)

testfile.write("\n".join(found)+"\n")

testfile.close()

