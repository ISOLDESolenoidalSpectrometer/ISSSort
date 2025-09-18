import re
import matplotlib.pyplot as plt
from collections import defaultdict

input_file = "autocal_sigmas.dat"
rows_colors = ['b', 'g', 'r', 'c', 'm', 'y']

pattern = re.compile(r"asic_(\d+)_(\d+)_(\d+)\.FWHM:\s*([\d.]+)")

data = defaultdict(lambda: defaultdict(dict))

with open(input_file, "r") as f:
    for line in f:
        match = pattern.match(line.strip())
        if match:
            asic, row, channel, fwhm = match.groups()
            asic, row, channel = int(asic), int(row), int(channel)
            fwhm = float(fwhm)
            data[asic][row][channel] = fwhm

for asic_id in sorted(data.keys()):
    plt.figure(figsize=(12, 6))

    for row_idx, row in enumerate(sorted(data[asic_id].keys())):
        channels_dict = data[asic_id][row]
        sorted_channels = sorted(channels_dict.keys())
        fwhm_values = [channels_dict[ch] for ch in sorted_channels]
        color = rows_colors[row_idx % len(rows_colors)]
        plt.plot(sorted_channels, fwhm_values, marker='o', linestyle='', color=color, label="Row {}".format(row))

    plt.title("ASIC {} - FWHM per Channel for Different Rows".format(asic_id))
    plt.xlabel("Channel")
    plt.ylabel("FWHM")
    plt.grid(True, alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig("asic_{}_fwhm.png".format(asic_id))
    plt.show()
