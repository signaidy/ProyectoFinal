#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <queue>
#include <algorithm>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

// Data point structure
struct Point {
    vector<double> coords;
};

// Load binary data
vector<Point> loadBinaryData(const string& path, int dims = 3, int maxPoints = 2000, size_t skipBytes = 0) {
    vector<Point> points;
    ifstream file(path, ios::binary);
    if (!file) {
        cerr << "Error al abrir el archivo binario: " << path << endl;
        return points;
    }

    file.seekg(skipBytes, ios::beg);

    while (file && points.size() < maxPoints) {
        Point p;
        for (int i = 0; i < dims; ++i) {
            double val;
            file.read(reinterpret_cast<char*>(&val), sizeof(double));  // <-- DOUBLE!
            if (!file) break;
            p.coords.push_back(val);
        }
        if (p.coords.size() == dims)
            points.push_back(p);
    }

    return points;
}

// Debug function to read raw bytes
void debugReadRawBytes(const string& path) {
    ifstream file(path, ios::binary);
    if (!file) {
        cerr << "Error al abrir archivo binario para debug." << endl;
        return;
    }

    cout << "Primeros 60 bytes como enteros:" << endl;
    for (int i = 0; i < 200; ++i) {
        uint8_t byte;
        file.read(reinterpret_cast<char*>(&byte), sizeof(uint8_t));
        if (!file) break;
        cout << static_cast<int>(byte) << " ";
    }
    cout << endl;
}

// Compute Euclidean distance between two points
double euclideanDist(const Point& a, const Point& b) {
    double sum = 0;
    for (size_t i = 0; i < a.coords.size(); ++i) {
        double diff = a.coords[i] - b.coords[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

// Construct k-NN graph and compute shortest paths
MatrixXd computeDistanceMatrix(const vector<Point>& points, int k) {
    int n = points.size();
    MatrixXd graph = MatrixXd::Constant(n, n, numeric_limits<double>::infinity());

    // Initialize distances
    for (int i = 0; i < n; ++i) {
        vector<pair<double, int>> dists;
        // Compute distances to all other points
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            dists.emplace_back(euclideanDist(points[i], points[j]), j);
        }
        sort(dists.begin(), dists.end());
        // Connect to k nearest neighbors
        for (int j = 0; j < k; ++j) {
            int idx = dists[j].second;
            double dist = dists[j].first;
            graph(i, idx) = dist;
            graph(idx, i) = dist;  // symmetrize
        }
    }
    return graph;
}

// Floyd-Warshall algorithm for all-pairs shortest paths
MatrixXd floydWarshall(MatrixXd dist) {
    int n = dist.rows();
    // Initialize self-distances to zero
    for (int k = 0; k < n; ++k)
        // Update distances
        for (int i = 0; i < n; ++i)
            // Check for shorter paths
            for (int j = 0; j < n; ++j)
                // Update distance if a shorter path is found
                if (dist(i, k) + dist(k, j) < dist(i, j))
                    dist(i, j) = dist(i, k) + dist(k, j);
    return dist;
}

// Classical MDS
MatrixXd classicMDS(const MatrixXd& D, int d = 2) {
    int n = D.rows();
    MatrixXd D2 = D.array().square();
    VectorXd row_mean = D2.rowwise().mean();
    VectorXd col_mean = D2.colwise().mean();
    double total_mean = D2.mean();

    MatrixXd B(n, n);

    // Double centering
    for (int i = 0; i < n; ++i)
        // Compute B matrix
        for (int j = 0; j < n; ++j)
            B(i, j) = -0.5 * (D2(i, j) - row_mean(i) - col_mean(j) + total_mean);

    SelfAdjointEigenSolver<MatrixXd> solver(B);
    MatrixXd eigvecs = solver.eigenvectors().rightCols(d);
    MatrixXd eigvals = solver.eigenvalues().tail(d).asDiagonal();

    // Return the reduced coordinates
    return eigvecs * eigvals.cwiseSqrt();
}

int main() {
    debugReadRawBytes("/data/isomap.dat");

    auto points = loadBinaryData("/data/isomap.dat", 3, 2000, 1024);

    // Debug: print first 5 points
    cout << "Puntos cargados: " << points.size() << endl;
    for (int i = 0; i < min(5, (int)points.size()); ++i) {
        for (double val : points[i].coords)
            cout << fixed << val << " ";
        cout << endl;
    }

    auto distGraph = computeDistanceMatrix(points, 7); // k=7
    auto geoDists = floydWarshall(distGraph);
    auto reduced = classicMDS(geoDists, 2);

    ofstream out("embedding.csv");

    // Save reduced coordinates
    for (int i = 0; i < reduced.rows(); ++i)
        out << reduced(i, 0) << "," << reduced(i, 1) << "\n";
    cout << "ISOMAP projection saved to embedding.csv" << endl;
    return 0;
}