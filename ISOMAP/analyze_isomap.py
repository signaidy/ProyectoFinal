import numpy as np
from sklearn.neighbors import NearestNeighbors
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import shortest_path
import matplotlib.pyplot as plt
import pandas as pd
import os

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

# --- Leer archivo isomap.dat como binario ---
data_path = os.path.join("data", "isomap.dat")
with open(data_path, "rb") as f:
    raw = f.read()

X = np.frombuffer(raw, dtype=np.float64)

# Según el paper: 698 muestras, 64 dimensiones
X = X.reshape((-1, 64))  # Asume que son N muestras de 64 dimensiones
print(f"Datos cargados: {X.shape[0]} muestras de {X.shape[1]} dimensiones")
N = X.shape[0]


# Estimar epsilon = distancia al 6º vecino más lejano
neigh6 = NearestNeighbors(n_neighbors=7).fit(X)
distances6, _ = neigh6.kneighbors(X)
eps_val = np.max(distances6[:, 6])
print(f"ε seleccionado = {eps_val:.3f}")

# Embedding con ambos métodos
Y_knn = isomap_embedding(X, n_components=2, n_neighbors=6)
Y_eps = isomap_embedding(X, n_components=2, epsilon=eps_val)

# Crear directorio de salida
output_dir = os.path.join("output")
os.makedirs(output_dir, exist_ok=True)

# Guardar CSVs
pd.DataFrame(Y_knn, columns=["dim1", "dim2"]).to_csv(os.path.join(output_dir, "isomap_knn.csv"), index=False)
pd.DataFrame(Y_eps, columns=["dim1", "dim2"]).to_csv(os.path.join(output_dir, "isomap_eps.csv"), index=False)

# Graficar comparación
fig, axs = plt.subplots(1, 2, figsize=(12, 5))
axs[0].scatter(Y_knn[:, 0], Y_knn[:, 1], c='cornflowerblue', s=10)
axs[0].set_title("Isomap con K=6 vecinos")
axs[0].set_xlabel("Dim 1"); axs[0].set_ylabel("Dim 2")

axs[1].scatter(Y_eps[:, 0], Y_eps[:, 1], c='darkorange', s=10)
axs[1].set_title(f"Isomap con ε={eps_val:.3f}")
axs[1].set_xlabel("Dim 1"); axs[1].set_ylabel("Dim 2")

plt.tight_layout()
plt.savefig(os.path.join(output_dir, "isomap_comparison.png"), dpi=300)
plt.show()