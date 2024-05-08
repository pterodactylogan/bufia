import random

fold = "4"

test = open("../WilsonGallagher2018_crossValidationFolds/test" + fold, "r")
att_test = open("./CrossValidationFolds/" + fold + "/att_test"
                +fold +".txt", "w")

att_dev = open("./CrossValidationFolds/" + fold +
               "/att_dev" +fold +".txt", "w")

held_out = []
for line in test.readlines():
    split = line.split()
    if split[-1] == "held-out":
        held_out.append("# " + " ".join(split[0:-1]) + " #")

test.close()

random.shuffle(held_out)

att_test_words = held_out[:len(held_out)//2]

att_dev_words = held_out[len(held_out)//2:]

att_test.write("\n".join(att_test_words))

att_dev.write("\n".join(att_dev_words))

att_test.close()
att_dev.close()
