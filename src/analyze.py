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

token_lens = [len(token) for token in counter]
print("Length of top 15 longest tokens =",
      sorted(token_lens, reverse=True)[:15])

print("Top 5 most common tokens =", counter.most_common(5))

with open("tokens.txt", "w") as f:
    for token in counter:
        f.write(f"{token}\n")