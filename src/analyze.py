import re
from collections import Counter

L = list()
with open("testdata/test.in") as fin:
    next(fin)
    next(fin)
    for i in range(10000):
        next(fin)
        next(fin)
        content = next(fin)
        subject = next(fin)
        next(fin)
        next(fin)
        L.append(content)
        L.append(subject)

counter = Counter()
for s in L:
    tokens = re.findall("[A-Za-z0-9]+", s)
    counter.update([token.lower() for token in tokens])

# ========================================

print(f"Total number of tokens = {len(counter)}")

longest_token = None
longest_token_len = 0
for token in counter:
    if len(token) > longest_token_len:
        longest_token = token
        longest_token_len = len(token)
print(f"Longest token = {longest_token}, its length = {longest_token_len}")

print("Top 5 most common tokens =", counter.most_common(5))
