import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os
from sklearn.neighbors import NearestNeighbors
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import shortest_path

def load_gct_file(filepath):
    with open(filepath, "r") as f:
        lines = f.readlines()
    header = lines[2].strip().split("\t")[2:]
    data_lines = lines[3:]
    gene_ids = []
    data = []
    for line in data_lines:
        parts = line.strip().split("\t")
        gene_ids.append(parts[0])
        try:
            values = list(map(float, parts[2:]))
        except ValueError:
            values = [np.nan] * len(parts[2:])  # fallback si falla
        data.append(values)
    df = pd.DataFrame(data, index=gene_ids, columns=header).T
    return df

def isomap_embedding(X, n_components=2, n_neighbors=None, epsilon=None):
    N = X.shape[0]
    if (n_neighbors is None) == (epsilon is None):
        raise ValueError("Especifique solo uno de n_neighbors o epsilon.")
    if n_neighbors is not None:
        neigh = NearestNeighbors(n_neighbors=n_neighbors + 1).fit(X)
        distances, indices = neigh.kneighbors(X)
        rows, cols, vals = [], [], []
        for i in range(N):
            for j_idx in range(1, n_neighbors + 1):
                j = indices[i, j_idx]
                d = distances[i, j_idx]
                rows += [i, j]
                cols += [j, i]
                vals += [d, d]
        W = csr_matrix((vals, (rows, cols)), shape=(N, N))
    else:
        neigh = NearestNeighbors(radius=epsilon).fit(X)
        radius_dists, radius_indices = neigh.radius_neighbors(X, radius=epsilon, sort_results=True)
        rows, cols, vals = [], [], []
        for i in range(N):
            for dist, j in zip(radius_dists[i], radius_indices[i]):
                if j == i:
                    continue
                rows += [i, j]
                cols += [j, i]
                vals += [dist, dist]
        W = csr_matrix((vals, (rows, cols)), shape=(N, N))
    D = shortest_path(csgraph=W, directed=False, unweighted=False)
    if np.isinf(D).any():
        print("⚠️  Aviso: el grafo no es completamente conexo.")
    D2 = D ** 2
    J = np.eye(N) - np.ones((N, N)) / N
    B = -0.5 * J @ D2 @ J
    vals, vecs = np.linalg.eigh(B)
    idx_desc = np.argsort(vals)[::-1]
    vals = vals[idx_desc][:n_components]
    vecs = vecs[:, idx_desc][:, :n_components]
    Y = vecs * np.sqrt(np.abs(vals))
    return Y

# --- Cargar y unir los datos ---
train_df = load_gct_file("data/all_aml_train.gct")
test_df = load_gct_file("data/all_aml_test.gct")
df_all = pd.concat([train_df, test_df])
labels = df_all.index.to_series().str.extract(r"(ALL|AML)", expand=False)
df_all["label"] = labels

# Limpiar datos (quitar columnas con NaN)
X = df_all.drop(columns=["label"])
nan_cols = X.columns[X.isna().any()].tolist()
if nan_cols:
    print(f"⚠️  Eliminando {len(nan_cols)} columnas con NaNs.")
    X = X.drop(columns=nan_cols)
X = X.values

# Estimar epsilon
neigh6 = NearestNeighbors(n_neighbors=7).fit(X)
distances6, _ = neigh6.kneighbors(X)
eps_val = np.max(distances6[:, 6])
print(f"ε seleccionado = {eps_val:.3f}")

# ISOMAP
Y_knn = isomap_embedding(X, n_components=2, n_neighbors=6)
Y_eps = isomap_embedding(X, n_components=2, epsilon=eps_val)

# Guardar resultados
output_dir = "output"
os.makedirs(output_dir, exist_ok=True)
df_knn = pd.DataFrame(Y_knn, columns=["dim1", "dim2"]).assign(label=labels.values)
df_eps = pd.DataFrame(Y_eps, columns=["dim1", "dim2"]).assign(label=labels.values)
df_knn.to_csv(os.path.join(output_dir, "isomap_knn.csv"), index=False)
df_eps.to_csv(os.path.join(output_dir, "isomap_eps.csv"), index=False)

# Gráficas con ejes descriptivos y leyenda clara
fig, axs = plt.subplots(1, 2, figsize=(12, 5))
colors = ['blue' if l == 'ALL' else 'red' for l in labels]
legend_labels = {"ALL": "Leucemia Linfoblástica Aguda (ALL)", "AML": "Leucemia Mieloide Aguda (AML)"}

for ax, Y, title in zip(
    axs,
    [Y_knn, Y_eps],
    [f"Isomap con K=6 vecinos", f"Isomap con ε={eps_val:.3f}"]
):
    scatter = ax.scatter(Y[:, 0], Y[:, 1], c=colors, s=20, label=labels)
    ax.set_title(title)
    ax.set_xlabel("varianza principal entre perfiles genéticos")
    ax.set_ylabel("varianza secundaria entre perfiles genéticos")
    # Añadir leyenda manualmente
    for label_value, color in zip(["ALL", "AML"], ["blue", "red"]):
        ax.scatter([], [], c=color, label=legend_labels[label_value])
    ax.legend(loc="upper right", fontsize=9)

plt.tight_layout()
plt.savefig(os.path.join(output_dir, "isomap_comparison.png"), dpi=300)
plt.show()