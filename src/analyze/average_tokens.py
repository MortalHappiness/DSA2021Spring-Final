import re
from collections import Counter

with open("../testdata/test.in") as fin:
    next(fin)
    next(fin)
    avg_tokens = 0
    avg_unique_tokens = 0
    for i in range(10000):
        next(fin)
        next(fin)
        content = next(fin)
        subject = next(fin)
        next(fin)
        next(fin)
        data = content + " " + subject
        tokens = re.findall("[A-Za-z0-9]+", data)
        tokens = [token.lower() for token in tokens]
        avg_tokens += len(tokens)
        avg_unique_tokens += len(set(tokens))

print("Average number of tokens in each email =", avg_tokens / 10000)
print("Average number of unique tokens in each email =", avg_unique_tokens / 10000)
