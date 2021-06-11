import re
from collections import Counter


tokens_cnt = list()
counter = Counter()
token_2_id={}
token_2_cnt={}
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
        # L.append(content)
        # L.append(subject)
        # l = 0
        # for s in L:
        tokens = re.findall("[A-Za-z0-9]+", content)
        tokens += re.findall("[A-Za-z0-9]+",  subject)
        tokens = [token.lower() for token in tokens]
            # l = len(set(tokens[:]))
        tokens = set(tokens)
        for token in tokens:
            if token in token_2_id:
                token_2_id[token].append(i)
            else:
                token_2_id[token] = [i]
            if token in token_2_cnt:
                token_2_cnt[token] +=1
            else:
                token_2_cnt[token] = 1
        counter.update(tokens)
        # tokens_cnt.append(l)
set_2_tokens={}
assert len(token_2_id) == 138078
for key in token_2_id:
    token_2_id[key].sort()
    token_2_id[key] = [str(ele) for ele in token_2_id[key]]
    # print(" ".join(token_2_id[key]))
    # print(token_2_id[key])
    if " ".join(token_2_id[key]) not in set_2_tokens:
        # print(token_2_id[key])
        if(len(token_2_id[key]) > 1):
            set_2_tokens[str(" ".join(token_2_id[key]))] = [key]
    else:
        set_2_tokens[" ".join(token_2_id[key])].append(key)
        # print(set_2_tokens[" ".join(token_2_id[key])])
cnt = 0

for key in set_2_tokens:
    if len(set_2_tokens[key]) > 1:
        # print(set_2_tokens[key], key)
        cnt += 1
# print(tokens_cnt)
# print(token_2_cnt["astika"])
print(cnt)
# counter.update(tokens_cnt)

# tokens_cnt.sort()

# ========================================
# print(len(counter))
# cnt = 0
# for i, ele in enumerate(counter.most_common()):
#     if(ele[1] == 3):
#         # print(i)
#         cnt += 1
#         # break 
# print(cnt)
# print(f"40 most tokens per mail = {counter.most_common()[52326:52328]}")


