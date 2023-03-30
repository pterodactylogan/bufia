# Usage

Using the c++ bufia implementation requires the GCC compiler.

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

## -a Int

The `-a` flag determines which abductive principle to use to guide
constraint selection. Currently, the following two are implemented.

* 0 : all pairwise incomparable, surface-true constraints are
  returned. (Even if multiple constraints have the same effect.)
* 1 : A constraint is only added to the grammar if its extension is
  not subsumed by the extension of the current grammar.

The default is 1. 

For standard phonotactic learning problems, principle 0 returns very
many constraints because the density of the constraint space is so
high. While many of these constraints seem redundant, it is important
to realize that they are all surface true and that they are pairwise
incomparable in factor space. In other words, the empirical data and
the feature theory in use cannot distinguish among the constraints
returned with principle 0.

Abductive principle 1 generally provides many fewer constraints than
principle 0, which facilitates interpretation. The constraints that
are returned are the *earliest* constraints in the search that account
for anything new.

# Code Organization

This code is organized into one primary file containing the high-level
logic (`bufia.cc`), one file defining a custom class to represent
k-factors (`factor.cc`), and two files of helper methods (`bufia_init_utils` 
and `bufia_algorithm_utils`). Header files (`.h` extension) contain class 
definitions and method signatures and descriptions, while code files (`.cc` extension) 
contain implementation logic.

// TODO: add detailed descriptions of files
