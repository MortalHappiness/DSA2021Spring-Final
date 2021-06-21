#!/usr/bin/env python3

with open("variables.c") as fin:
    variables = fin.read()

print("Finish reading variables")

with open("main.c") as fin, open("main_generated.c", "w") as fout:
    for line in fin:
        fout.write(line)
        if line.startswith("// PRECOMPUTED VALUES"):
            break
    fout.write(variables)
    for line in fin:
        fout.write(line)
