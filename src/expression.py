import re
from collections import Counter

L = list()
with open("testdata/test.in") as fin:
    next(fin)
    next(fin)
    for i in range(60000):
        next(fin)
    next(fin)
    next(fin)
    for i in range(10000):
        next(fin)
        query_type = next(fin)
        if (query_type == "e\n"):
            exp = next(fin)
            # print(exp)
            L.append(exp)
        else:
            next(fin)
        next(fin)

counter = Counter()
for s in L:
    tokens = re.findall("[A-Za-z0-9]+", s)
    counter.update([token.lower() for token in tokens])
    
# ========================================

print(f"Total number of tokens = {len(counter)}")

token_lens = [len(token) for token in counter]
print("Length of top 15 longest tokens =",
      sorted(token_lens, reverse=True)[:15])

print("Top 5 most common tokens =", counter.most_common(5))

with open("expression_tokens.txt", "w") as f:
    for token in counter:
        f.write(f"{token}\n")