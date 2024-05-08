fold = "4"

f1 = open("./CrossValidationFolds/all/curated_illicit.txt")
f2 = open("./CrossValidationFolds/all/curated_illicit2.txt")

cur_test = open("./CrossValidationFolds/" + fold +
                    "/cur_test" + fold + ".txt", "w")
cur_dev = open("./CrossValidationFolds/" + fold +
                    "/cur_dev" + fold + ".txt", "w")

i = 0
for line in f1.readlines():
    if i%2 == 0:
        cur_test.write(line)
    else:
        cur_dev.write(line)
    i += 1

for line in f2.readlines():
    if i%2 == 0:
        cur_test.write(line)
    else:
        cur_dev.write(line)
    i += 1

cur_test.close()
cur_dev.close()
