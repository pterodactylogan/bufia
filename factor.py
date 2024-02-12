# return list of lists of bundles
def subsequences(bundles, size):
    if size > len(bundles):
        return []
    if size == 0:
        return [[]]

    result = []
    for bundle in bundles:
        result += [[bundle] + bs for bs in subsequences(bundles[1:],
                                                        size-1)]
    return result

class Factor:
    def __init__(self, string="", bundles=None):
        if bundles != None:
            self.bundles = bundles
            return
        
        self.bundles = []
        bundles = string.strip("]").strip("[").split("][")
        for bundle in bundles:
            feats = set()
            if bundle != "":
                for item in bundle.split(","):
                    feats.add(item)
            self.bundles.append(feats)

    def __eq__(self, other):
        return self.bundles == other.bundles

    def __str__(self):
        result = ""
        for bundle in self.bundles:
            result += "["
            for feat in bundle:
                result += feat + ","
            result = result.strip(",")
            result+="]"
        return result
            
    def generates(self, child, mode="succ"):
        if len(self.bundles) == 0 or len(child.bundles) == 0:
            return False
        
        if len(self.bundles) > len(child.bundles):
            return False

        if mode == "succ":
        # for each alignment
            for offset in range(len(child.bundles) - len(self.bundles) + 1):
                found_mismatch = False
                # check if each bundle is a subset of the child's at that index
                for i in range(len(self.bundles)):
                    if(not self.bundles[i].issubset(child.bundles[i+offset])):
                        found_mismatch = True
                        break

                if(not found_mismatch):
                    return True
        elif mode == "prec":
            # for each subsequence
            for seq in subsequences(child.bundles, len(self.bundles)):
                found_mismatch = False
                for i in range(len(self.bundles)):
                    if(not self.bundles[i].issubset(seq[i])):
                       found_mismatch = True
                       break
                if(not found_mismatch):
                    return True
        else:
            raise Exception("invalid mode specified")
            
        return False
                

        
