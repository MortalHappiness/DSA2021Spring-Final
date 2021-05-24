import json

with open("tokens.txt", "r") as fin:
    tokens = fin.readlines()
    tokens = set(tokens)

with open("expression_tokens.txt", "r") as fin:
    exprs = fin.readlines()

hit = 0
for expr in exprs:
    if expr in tokens:
        hit += 1

print(f"Hit rate: {hit / len(exprs)}")
