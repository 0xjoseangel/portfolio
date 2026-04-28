import matplotlib
matplotlib.use("Agg")
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

ORDER = ["GREEDY", "RANDOM", "BL",
         "AGG-ARIT", "AGG-BLX",
         "AGE-ARIT", "AGE-BLX",
         "AM-ALL", "AM-RAND", "AM-BEST"]


def main():
    fnames = [f for f in os.listdir(".") if f.endswith(".csv")]
    for fname in fnames:
        df = pd.read_csv(fname)
        df["alg"] = df["alg"].str.upper()
        order = [a for a in ORDER if a in df["alg"].unique()]

        fig, ax = plt.subplots(figsize=(11, 5))
        sns.boxplot(data=df, x="alg", y="fitness", order=order, ax=ax)
        ax.set_xlabel("Algoritmo")
        ax.set_ylabel("Fitness")
        ax.tick_params(axis="x", rotation=35)
        plt.tight_layout()

        foutput = fname.replace(".csv", ".png")
        fig.savefig(foutput, dpi=150, bbox_inches="tight")
        plt.close(fig)


if __name__ == "__main__":
    main()
