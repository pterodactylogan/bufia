# Usage

Using the c++ bufia implementation requires the GCC compiler with version 11+ (note: on seawulf or ookami, run `module load gcc` to get a recent gcc version). Users can also use other c++ compilers if they update the `CC` variable in `Makefile`.

1. run `make bufia` to compile the program.
2. run `./bufia wordfile featurefile [OPTIONS...]` to execute the program.
   Running may take a long time.

Example:
```
./bufia ../sanity/malarky1800.train ../sanity/malarky.features.csv --k=2 --n=2
```

## wordfile (Required)

The wordfile contains a list of words. Each word should be on its own line. 
The symbols which compose the word should be separated by spaces.  Here is an example.

```
d͡ʒ ɛ f
w ʌ z
h i ɹ
```

See the `sanity` folder for more examples.

## featurefile (Required)

The featurefile provides featural information for the symbols used in
the wordlist. It should be a command-delimited file with the symbols
in the first line. Here is an example.

```
,i,u,e,o,a,
back,-,+,-,+,-
low,-,-,-,-,+
high,+,+,-,-,-
```

See the `sanity` folder for more examples.


## -k Int

The `-k` flag determines the maximum width of factors to search. The
larger the number the longer the search will take. The default is 3.

## -n Int

The `-n` flag determines the maximum number of features considered at
each position. In other words, if this is set to 2, then feature
combinations of 3 or more features are not considered. So if this is
set to 2 the factor [+high,+front,+round] will never be
considered. The larger the number the longer the search will take. The
default is 3.

In any case, Bufia will never consider incompatible feature
combinations such as [-voice,+voice] or [+high,+low]. If no symbols
are picked out by some combination of features, that combinations is
never considered.

## -d Int

The `-d` flag limits the size of the constraints considered in terms of *distance* from the empty factor.
The distance of a given factor from the empty factor can be computed by summing the total number of features and the total number of segments in the factor.
So the factor [+high,+front,+round] has a distance of 4, while the factor [+high,+front][+nasal] has a distance of 5.

This flag can be used in conjunction with the `-k` and/or `-n` flags, in which case all limits will be respected.
The default is -1, which means there will be no limit on the distance metric.
If the `d` flag and either the `n` flag or the `k` flag are unset, the max distance allowed will be 9.
In general, it is reccommended to set at least two of these parameters.

## -a Int

The `-a` flag determines how much a new constraint must expand the extension of the grammar in order to be added. 
Each surface-true (pairwise incomparable) constraint will be added to the grammar if and only if it accounts for at least *a* new k-grams (where k is the max factor width).

The default is 1. 

For standard phonotactic learning problems, setting `-a=0` returns very
many constraints because the density of the constraint space is so
high. While many of these constraints seem redundant, it is important
to realize that they are all surface true and that they are pairwise
incomparable in factor space. In other words, the empirical data and
the feature theory in use cannot distinguish among the constraints
returned with `-a=0`.

Setting higher values of *a* generally provides fewer constraints, which facilitates interpretation. The constraints that
are returned are the *earliest* constraints in the search that account
for a sufficient number of new k-grams.

## -o Int

The `-o` flag controls the ordering relation that should be used.
The default is 1, which corresponds to successor.
A value of 2 corresponds to precedence, which means BUFIA will consider subsequences rather than substrings.

## -f Int

The `-f` flag controls the order in which features should be added when generating new factors.
The default value is 1, which means the features will be ordered according to the size of their extensions.
Note that this means two opposite features like -high and +high will not necessarily be ordered adjacently.

Using `-f=2` will maintain the feature ordering given in the feature file.

## -wb Bool

The `-wb` flag will cause word boundary symbols to be added to the input data when it is set to `true`.
The default value is `false`.

## -t Vector<String>

The `-t` flag controls the tier that the program will project the input data onto.
Running without setting this flag will set the tier to the entire alphabet.
For example `-t=a,e,i,o,u` will project a vowel tier (assuming these are all and only your vowel symbols) and find constraints over that.

## -ignore Int

The `-ignore` flag allows for rudimentary noise handling by causing the program to ignore a set number of violations of a possible constraint.
`-ignore=3` will result in constraints which are violated three or fewer times in the input data **not** being added to the grammar *or* being pruned from the search space (ie treated as not violated).

# Code Organization

This code is organized into one primary file containing the high-level
logic (`bufia.cc`), one file defining a custom class to represent
k-factors (`factor.cc`), and two files of helper methods (`bufia_init_utils` 
and `bufia_algorithm_utils`). Header files (`.h` extension) contain class 
definitions and method signatures and descriptions, while code files (`.cc` extension) 
contain implementation logic.

// TODO: add detailed descriptions of files
