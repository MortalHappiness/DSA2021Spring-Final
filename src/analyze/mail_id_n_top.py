from collections import Counter
time = {}
with open("comput_time.txt", "r") as f:
    lines = f.readlines()
    for line in lines:
        num, cost = line.strip().split(" ")
        time[int(num)] = int(cost)

# 231311072
c = Counter()
with open("../n_item_without_1_8.txt", "r") as fin:
    tops = [int(num.strip()) for num in fin.readlines()]
    cnt = 0
    cost = 0
    for ele in tops:
        c.update([ele])
        if (ele < 11)  or (71 <= ele and ele <= 79): #62~70 70~78 72~80
            cnt += 1
            cost += time[ele]
    # print(c.most_common()[:])
    print(cnt)
    print(cost)
