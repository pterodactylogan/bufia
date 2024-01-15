import random

test = open("../WilsonGallagher2018_crossValidationFolds/test1", "r")
licit_test = open("./CrossValidationFolds/1/licit_test1.txt", "w")
illicit_test = open("./CrossValidationFolds/1/illicit_test1.txt", "w")

licit_dev = open("./CrossValidationFolds/1/licit_dev1.txt", "w")
illicit_dev = open("./CrossValidationFolds/1/illicit_dev1.txt", "w")

held_out = []
licit = []
illicit = []
for line in test.readlines():
    split = line.split()
    if split[-1] == "held-out":
        held_out.append("# " + " ".join(split[0:-1]) + " #")
    elif split[-1] == "legal":
        licit.append(" ".join(split[0:-1]))
    elif split[-1] == "illegal":
        illicit.append(" ".join(split[0:-1]))
    else:
        print("problem:", line)

test.close()

random.shuffle(held_out)
random.shuffle(licit)
random.shuffle(illicit)

licit_test_words = held_out[:len(held_out)//2] + licit[:len(licit)//2]
illicit_test_words = illicit[:len(illicit)//2]

licit_dev_words = held_out[len(held_out)//2:] + licit[len(licit)//2:]
illicit_dev_words = illicit[len(illicit)//2:]

licit_test.write("\n".join(licit_test_words))
illicit_test.write("\n".join(illicit_test_words))

licit_dev.write("\n".join(licit_dev_words))
illicit_dev.write("\n".join(illicit_dev_words))

licit_test.close()
illicit_test.close()
licit_dev.close()
illicit_dev.close()
