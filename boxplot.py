import matplotlib
matplotlib.use("Agg")
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

ORDER = ["greedy", "random", "bl",
         "Greedy", "Random", "BL",
         "AGG-Arit", "AGG-BLX",
         "AGE-Arit", "AGE-BLX",
         "AM-All", "AM-Rand", "AM-Best",
         "BMB", "ILS", "SA", "ILS-ES"]

DATASET_TITLES = {
    "results_ibex_35":    "IBEX 35  (N=30)",
    "results_syp_100":    "S&P 100  (N=97)",
    "results_syp_500":    "S&P 500  (N=457)",
    "p3_results_ibex_35": "P3: IBEX 35  (N=30)",
    "p3_results_syp_100": "P3: S&P 100  (N=97)",
    "p3_results_syp_500": "P3: S&P 500  (N=457)",
}

ALPHA_TITLES = {
    "blx_alpha_ibex_35":  "BLX-α — IBEX 35  (N=30)",
    "blx_alpha_syp_100":  "BLX-α — S&P 100  (N=97)",
    "blx_alpha_syp_500":  "BLX-α — S&P 500  (N=457)",
}


RESULT_DIRS = [
    "results/all_algorithms",
    "results/p3",
    "results/studies",
]


def plot_results(out_path, fname_stem, df):
    """Boxplot for results_*.csv / p3_results_*.csv (columns: alg, fitness)."""
    df["alg"] = df["alg"].str.strip()
    order = [a for a in ORDER if a in df["alg"].unique()]

    fig, ax = plt.subplots(figsize=(12, 5))
    sns.boxplot(data=df, x="alg", y="fitness", order=order, ax=ax,
                hue="alg", palette="tab10", legend=False)
    title = DATASET_TITLES.get(fname_stem, fname_stem)
    ax.set_title(title, fontsize=13, fontweight="bold")
    ax.set_xlabel("Algoritmo")
    ax.set_ylabel("Fitness (train 2015–2024)")
    ax.tick_params(axis="x", rotation=30)
    plt.tight_layout()
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  -> {out_path}")


def plot_alpha(out_path, fname_stem, df):
    """Boxplot for blx_alpha_*.csv (columns: alpha, fitness_train, fitness_test)."""
    df["alpha"] = df["alpha"].round(2)
    alphas = sorted(df["alpha"].unique())
    labels = [str(a) for a in alphas]

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    title = ALPHA_TITLES.get(fname_stem, fname_stem)
    fig.suptitle(title, fontsize=13, fontweight="bold")

    for ax, col, ylabel in zip(axes,
                               ["fitness_train", "fitness_test"],
                               ["Fitness train (2015–2024)", "Fitness test (2025)"]):
        data = [df[df["alpha"] == a][col].values for a in alphas]
        ax.boxplot(data, tick_labels=labels)
        ax.set_xlabel("α (BLX-α)")
        ax.set_ylabel(ylabel)
        ax.tick_params(axis="x", rotation=0)

    plt.tight_layout()
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  -> {out_path}")


def main():
    for directory in RESULT_DIRS:
        if not os.path.isdir(directory):
            continue
        csvs = [f for f in os.listdir(directory) if f.endswith(".csv")]
        for fname in sorted(csvs):
            stem = fname[:-4]
            path = os.path.join(directory, fname)
            try:
                df = pd.read_csv(path)
            except Exception as e:
                print(f"  [skip] {path}: {e}")
                continue

            out_path = os.path.join(directory, stem + ".png")
            if "alg" in df.columns and "fitness" in df.columns:
                print(f"Procesando resultados: {path}")
                plot_results(out_path, stem, df)
            elif "alpha" in df.columns and "fitness_train" in df.columns:
                print(f"Procesando estudio alpha: {path}")
                plot_alpha(out_path, stem, df)
            else:
                print(f"  [skip] {path}: columnas no reconocidas")


if __name__ == "__main__":
    main()
