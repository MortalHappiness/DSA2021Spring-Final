from collections import Counter

with open("tokens.txt", "r") as fin:
    tokens = fin.readlines()
print(len(tokens))
new_tokens = {}
l = 15
aaa = [_[:l] for _ in tokens]
for token in tokens:
    token = token.strip()
    if token[:l] in new_tokens:
        # print(token)
        new_tokens[token[:l]].append(token)
        print(new_tokens[token[:l]])
    else:
        new_tokens[token[:l]] = [token]

# [token[:20] for token in tokens]
counter = Counter()
counter.update(aaa)
print(counter.most_common(20))
