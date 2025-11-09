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

struct Point {
    vector<double> coords;
};

vector<Point> loadData(const string& path) {
    vector<Point> points;
    ifstream file(path);
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        Point p;
        double val;
        while (iss >> val) {
            p.coords.push_back(val);
        }
        if (!p.coords.empty()) points.push_back(p);
    }
    return points;
}

double euclideanDist(const Point& a, const Point& b) {
    double sum = 0;
    for (size_t i = 0; i < a.coords.size(); ++i) {
        double diff = a.coords[i] - b.coords[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

MatrixXd computeDistanceMatrix(const vector<Point>& points, int k) {
    int n = points.size();
    MatrixXd graph = MatrixXd::Constant(n, n, numeric_limits<double>::infinity());

    for (int i = 0; i < n; ++i) {
        vector<pair<double, int>> dists;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            dists.emplace_back(euclideanDist(points[i], points[j]), j);
        }
        sort(dists.begin(), dists.end());
        for (int j = 0; j < k; ++j) {
            int idx = dists[j].second;
            double dist = dists[j].first;
            graph(i, idx) = dist;
            graph(idx, i) = dist;  // symmetrize
        }
    }
    return graph;
}

MatrixXd floydWarshall(MatrixXd dist) {
    int n = dist.rows();
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (dist(i, k) + dist(k, j) < dist(i, j))
                    dist(i, j) = dist(i, k) + dist(k, j);
    return dist;
}

MatrixXd classicMDS(const MatrixXd& D, int d = 2) {
    int n = D.rows();
    MatrixXd D2 = D.array().square();
    VectorXd row_mean = D2.rowwise().mean();
    VectorXd col_mean = D2.colwise().mean();
    double total_mean = D2.mean();

    MatrixXd B(n, n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            B(i, j) = -0.5 * (D2(i, j) - row_mean(i) - col_mean(j) + total_mean);

    SelfAdjointEigenSolver<MatrixXd> solver(B);
    MatrixXd eigvecs = solver.eigenvectors().rightCols(d);
    MatrixXd eigvals = solver.eigenvalues().tail(d).asDiagonal();
    return eigvecs * eigvals.cwiseSqrt();
}

int main() {
    auto points = loadData("data/isomap.dat");
    auto distGraph = computeDistanceMatrix(points, 7); // k=7
    auto geoDists = floydWarshall(distGraph);
    auto reduced = classicMDS(geoDists, 2);

    ofstream out("embedding.csv");
    for (int i = 0; i < reduced.rows(); ++i)
        out << reduced(i, 0) << "," << reduced(i, 1) << "\n";
    cout << "ISOMAP projection saved to embedding.csv" << endl;
    return 0;
}