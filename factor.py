class Factor:
    def __init__(self, string):
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
