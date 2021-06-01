import re
from collections import Counter


tokens_cnt = list()
with open("../testdata/test.in") as fin:
    next(fin)
    next(fin)
    for i in range(10000):
        L = list()
        next(fin)
        next(fin)
        content = next(fin)
        subject = next(fin)
        next(fin)
        next(fin)
        content += " " + subject
        L.append(content)
        # L.append(subject)
        # l = 0
        for s in L:
            tokens = re.findall("[A-Za-z0-9]+", s)
            l = len(set(tokens[:]))
        tokens_cnt.append(l)

# print(tokens_cnt)
# counter = Counter()

# counter.update(tokens_cnt)

tokens_cnt.sort()

# ========================================

print(f"5 most tokens per mail = {tokens_cnt[-5:]}")


