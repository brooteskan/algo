# Contributing

The initial tag is a source-fidelity baseline. Do not combine extraction with
API redesign or behavior repair.

New production and test code must use algorithms, iterators, functions,
lambdas, maps, filters, folds, and apply operations rather than handwritten
loops. The imported baseline files are temporarily exempt at their recorded
hashes.

Each later change should address one observable concern, add or retain its
characterization tests, and document ordering, allocation, lifetime, mutation,
and early-termination behavior.
