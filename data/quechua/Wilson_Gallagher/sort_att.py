import random

all_att = open("./CrossValidationFolds/all/attested.txt")

# fold: [train, dev, test]
files = {}
for i in range(5):
    files[i] = [open("./CrossValidationFolds/{0}/our_version/train{0}.txt"
                .format(str(i)), "w"),
                open("./CrossValidationFolds/{0}/our_version/att_dev{0}.txt"
                .format(str(i)), "w"),
                open("./CrossValidationFolds/{0}/our_version/att_test{0}.txt"
                .format(str(i)), "w")]

# fold: next dev/test to modify (1=dev, 2=test)
split = {i: 2 for i in range(5)}

# each new form should go in 4 train sets and one dev or test set
nroots = 0
# root: (fold, 1/2) (1 = dev/ 2= test)
locations = {}

lines = all_att.readlines()
random.shuffle(lines)

for line in lines:
    root = line[:line.find(" @")]
    # deal with vowel allophony changes
    root = root.replace("o", "u").replace("e", "i")
    if root in locations:
        for i in range(5):
            if locations[root][0] == i:
                dt = locations[root][1]
                files[i][dt].write(line)
            else:
                files[i][0].write(line)
    else:
        f = nroots%5
        for i in range(5):
            if f==i:
                files[i][split[i]].write(line)
                locations[root] = (f, split[i]) 
                if split[i] == 1:
                    split[i] = 2
                else:
                    split[i] = 1
            else:
                files[i][0].write(line)
        nroots += 1
    # if no location, sort according to nroots
    # increment nroots

for i in range(5):
    for j in range(3):
        files[i][j].close()
