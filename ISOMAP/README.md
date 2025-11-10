# 🧬 ISOMAP - Reducción de Dimensionalidad en Datos Biológicos (Leucemia)

Este módulo implementa el algoritmo **ISOMAP** según el paper *"A Global Geometric Framework for Nonlinear Dimensionality Reduction"* de Tenenbaum et al. Se aplica a un conjunto de datos real de expresión génica en pacientes con leucemia (ALL y AML), permitiendo visualizar en 2D estructuras no lineales presentes en los datos de alta dimensión.

---

## 📌 Objetivo

Reducir y visualizar un conjunto de datos de expresión génica de 7129 genes en muestras humanas, preservando su estructura geodésica, mediante:

* **ISOMAP con K vecinos (KNN)**
* **ISOMAP con ε-vecindad**

---

## 📂 Estructura del Proyecto

```
ISOMAP/
├── analyze_isomap.py       # Script principal
├── data/
│   ├── all_aml_train.gct   # Datos de entrenamiento (expresión génica)
│   └── all_aml_test.gct    # Datos de prueba
├── output/
│   ├── isomap_knn.csv      # Embedding usando K-vecinos
│   ├── isomap_eps.csv      # Embedding usando ε-vecindad
│   └── isomap_comparison.png  # Visualización 2D
```

---

## ⚙️ Requisitos

* Python 3.8+
* Paquetes requeridos:

```bash
pip install numpy pandas matplotlib scikit-learn scipy
```

O bien:

```bash
pip install -r requirements.txt
```

---

## 🚀 Cómo ejecutar

Desde la raíz del proyecto:

```bash
python analyze_isomap.py
```

Esto:

1. Carga y une los archivos `all_aml_train.gct` y `all_aml_test.gct`.
2. Extrae etiquetas `ALL` o `AML` desde los nombres de muestra.
3. Limpia columnas con valores faltantes (NaN).
4. Aplica ISOMAP con:

   * K=6 vecinos
   * ε estimado automáticamente
5. Guarda las proyecciones en 2D (`.csv`) y un gráfico de comparación.

---

## 📊 Salidas Generadas

* `output/isomap_knn.csv` → coordenadas 2D con KNN
* `output/isomap_eps.csv` → coordenadas 2D con ε
* `output/isomap_comparison.png` → gráfica de ambas proyecciones

---

## 🧠 ¿Qué hace el script?

* Lee archivos `.gct` (formato estándar en bioinformática)
* Detecta etiquetas `ALL` y `AML`
* Construye grafos de vecindad y calcula distancias geodésicas
* Aplica **MDS clásico** sobre la matriz de distancias
* Genera una visualización coloreada por clase

---

## 📬 Autoría

Implementación académica para análisis no lineal en datasets biomédicos.
Basado en el algoritmo ISOMAP (Tenenbaum et al., 2000).
