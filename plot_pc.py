import csv
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import sys

def read_csv(fname):
    with open(fname, 'rb') as f:
        reader = csv.reader(f)
        return list(reader)[0]

def remove_empties(hex_str):
    return hex_str != ''

def parse(hex_str):
    return int(hex_str, 16)

pc1 = map(parse, filter(remove_empties, read_csv(sys.argv[1])))
pc2 = map(parse, filter(remove_empties, read_csv(sys.argv[2])))

min_len = min(len(pc1), len(pc2))
pc1, pc2 = pc1[:min_len], pc2[:min_len]

plt.plot(pc1)
plt.plot(pc2, color='r')
# plt.legend([sys.argv[1], sys.argv[2]])
plt.legend(sys.argv[1:])
axes = plt.gca()
axes.get_yaxis().set_major_formatter(ticker.FormatStrFormatter('0x%X'))
plt.show()
