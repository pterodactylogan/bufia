from factor import Factor

parent = Factor("[+a][-b]")
child1 = Factor("[+a,-b][-b,+c,+d]")
child2 = Factor("[+b][+b][+a,-b][-b,+c,+d]")
child3 = Factor("[+b][+a,-b][-b,+c,+d][+b]")
nonchild = Factor("[+a,-b][+c,+d]")
prec_child1 = Factor("[+a][+c][-b]")
prec_child2 = Factor("[+a][+c][-b,+d]")
prec_child3 = Factor("[-x][-x][+a,-c][+c,-x][-b,+d][+x]")

print("expect 2:", len(parent.bundles))
print("expect 4:", len(child2.bundles))

print("expect True:", parent.generates(child1))
print("expect True:", parent.generates(child2))
print("expect True:", parent.generates(child3))
print("expect False:", parent.generates(nonchild))

print("expect True:", parent.generates(prec_child1, "prec"))
print("expect True:", parent.generates(prec_child2, "prec"))
print("expect True:", parent.generates(prec_child3, "prec"))
print("expect False:", parent.generates(nonchild, "prec"))

print(parent)
print(child1)

fac = Factor("[][-b][]")
print(fac.bundles)
