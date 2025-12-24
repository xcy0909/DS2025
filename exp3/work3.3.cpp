#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <climits>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

const int INF = INT_MAX;

// ======================
// (1) 图类：邻接矩阵
// ======================
class Graph {
public:
    vector<vector<int>> adjMatrix; // 邻接矩阵（0/1 或 权重）
    vector<string> labels;         // 顶点标签（如 "A", "B"）
    int n;                         // 顶点数
    bool weighted;                 // 是否带权

    Graph(int numVertices, const vector<string>& vertexLabels, bool isWeighted = false)
        : n(numVertices), labels(vertexLabels), weighted(isWeighted) {
        adjMatrix.assign(n, vector<int>(n, 0));
    }

    void addEdge(int u, int v, int weight = 1) {
        if (weighted) {
            adjMatrix[u][v] = weight;
            adjMatrix[v][u] = weight; // 无向图
        } else {
            adjMatrix[u][v] = 1;
            adjMatrix[v][u] = 1;
        }
    }

    void printAdjMatrix() {
        cout << "邻接矩阵：" << endl;
        cout << "   ";
        for (const string& label : labels) cout << label << " ";
        cout << endl;
        for (int i = 0; i < n; ++i) {
            cout << labels[i] << "  ";
            for (int j = 0; j < n; ++j) {
                if (adjMatrix[i][j] == 0 && i != j && !weighted) {
                    cout << "0 ";
                } else {
                    if (adjMatrix[i][j] == 0 && weighted) {
                        cout << "- "; // 表示无穷（无边）
                    } else {
                        cout << adjMatrix[i][j] << " ";
                    }
                }
            }
            cout << endl;
        }
        cout << endl;
    }
};

// ======================
// (2) BFS 和 DFS
// ======================
void BFS(const Graph& g, int start) {
    vector<bool> visited(g.n, false);
    queue<int> q;
    q.push(start);
    visited[start] = true;

    cout << "BFS (" << g.labels[start] << "): ";
    while (!q.empty()) {
        int u = q.front(); q.pop();
        cout << g.labels[u] << " ";
        for (int v = 0; v < g.n; ++v) {
            if ((g.weighted && g.adjMatrix[u][v] > 0) || (!g.weighted && g.adjMatrix[u][v] == 1)) {
                if (!visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }
    }
    cout << endl;
}

void DFSUtil(const Graph& g, int u, vector<bool>& visited) {
    visited[u] = true;
    cout << g.labels[u] << " ";
    for (int v = 0; v < g.n; ++v) {
        if ((g.weighted && g.adjMatrix[u][v] > 0) || (!g.weighted && g.adjMatrix[u][v] == 1)) {
            if (!visited[v]) {
                DFSUtil(g, v, visited);
            }
        }
    }
}

void DFS(const Graph& g, int start) {
    vector<bool> visited(g.n, false);
    cout << "DFS (" << g.labels[start] << "): ";
    DFSUtil(g, start, visited);
    cout << endl;
}

// ======================
// (3) 最短路径（Dijkstra） & 最小生成树（Prim）
// ======================
void dijkstra(const Graph& g, int start) {
    vector<int> dist(g.n, INF);
    vector<bool> visited(g.n, false);
    vector<int> parent(g.n, -1);

    dist[start] = 0;

    for (int count = 0; count < g.n; ++count) {
        int u = -1;
        for (int i = 0; i < g.n; ++i) {
            if (!visited[i] && (u == -1 || dist[i] < dist[u]))
                u = i;
        }

        if (dist[u] == INF) break;
        visited[u] = true;

        for (int v = 0; v < g.n; ++v) {
            if (g.adjMatrix[u][v] > 0 && !visited[v]) {
                int newDist = dist[u] + g.adjMatrix[u][v];
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    parent[v] = u;
                }
            }
        }
    }

    cout << "Dijkstra 最短路径（从 " << g.labels[start] << "）：" << endl;
    for (int i = 0; i < g.n; ++i) {
        if (i == start) continue;
        cout << g.labels[start] << " -> " << g.labels[i] << ": ";
        if (dist[i] == INF) {
            cout << "不可达" << endl;
        } else {
            // 回溯路径
            vector<string> path;
            int cur = i;
            while (cur != -1) {
                path.push_back(g.labels[cur]);
                cur = parent[cur];
            }
            reverse(path.begin(), path.end());
            for (size_t j = 0; j < path.size(); ++j) {
                if (j > 0) cout << " -> ";
                cout << path[j];
            }
            cout << " (距离: " << dist[i] << ")" << endl;
        }
    }
    cout << endl;
}

void primMST(const Graph& g, int start) {
    vector<int> key(g.n, INF);
    vector<bool> inMST(g.n, false);
    vector<int> parent(g.n, -1);

    key[start] = 0;

    for (int count = 0; count < g.n; ++count) {
        int u = -1;
        for (int i = 0; i < g.n; ++i) {
            if (!inMST[i] && (u == -1 || key[i] < key[u]))
                u = i;
        }

        if (key[u] == INF) break;
        inMST[u] = true;

        for (int v = 0; v < g.n; ++v) {
            if (g.adjMatrix[u][v] > 0 && !inMST[v]) {
                if (g.adjMatrix[u][v] < key[v]) {
                    key[v] = g.adjMatrix[u][v];
                    parent[v] = u;
                }
            }
        }
    }

    cout << "Prim 最小生成树（从 " << g.labels[start] << "）：" << endl;
    int totalWeight = 0;
    for (int i = 0; i < g.n; ++i) {
        if (parent[i] != -1) {
            cout << g.labels[parent[i]] << " -- " << g.labels[i]
                 << " (权: " << g.adjMatrix[i][parent[i]] << ")" << endl;
            totalWeight += g.adjMatrix[i][parent[i]];
        }
    }
    cout << "总权重: " << totalWeight << endl << endl;
}

// ======================
// (4) 双连通分量 & 关节点（Articulation Points）
// 使用 Tarjan 算法
// ======================
class BiconnectedGraph {
public:
    vector<vector<int>> adj;
    int n;
    vector<string> labels;

    BiconnectedGraph(int numVertices, const vector<string>& vertexLabels)
        : n(numVertices), labels(vertexLabels) {
        adj.assign(n, vector<int>());
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    void findArticulationPoints() {
        vector<int> disc(n, -1);      // 发现时间
        vector<int> low(n, -1);       // 最早可达祖先
        vector<bool> isArticulation(n, false);
        vector<bool> visited(n, false);
        int time = 0;

        // 处理非连通图
        for (int i = 0; i < n; ++i) {
            if (disc[i] == -1) {
                dfsAP(i, -1, disc, low, isArticulation, visited, time);
            }
        }

        cout << "关节点（Articulation Points）：" << endl;
        bool found = false;
        for (int i = 0; i < n; ++i) {
            if (isArticulation[i]) {
                cout << labels[i] << " ";
                found = true;
            }
        }
        if (!found) {
            cout << "无";
        }
        cout << endl << endl;
    }

private:
    void dfsAP(int u, int parent, vector<int>& disc, vector<int>& low,
               vector<bool>& isArticulation, vector<bool>& visited, int& time) {
        int children = 0;
        disc[u] = low[u] = ++time;
        visited[u] = true;

        for (int v : adj[u]) {
            if (v == parent) continue;

            if (disc[v] == -1) {
                children++;
                dfsAP(v, u, disc, low, isArticulation, visited, time);
                low[u] = min(low[u], low[v]);

                // 根节点且有多个子树
                if (parent == -1 && children > 1)
                    isArticulation[u] = true;

                // 非根节点，存在子 v 使得 low[v] >= disc[u]
                if (parent != -1 && low[v] >= disc[u])
                    isArticulation[u] = true;
            } else {
                low[u] = min(low[u], disc[v]);
            }
        }
    }
};

// ======================
// 主函数：执行所有任务
// ======================
int main() {
    // ========== 图1：带权无向图 ==========
    vector<string> labels1 = {"A", "B", "C", "D", "E"};
    Graph graph1(5, labels1, true); // 带权

    // 添加边（图1）
    graph1.addEdge(0, 1, 4); // A-B
    graph1.addEdge(0, 2, 2); // A-C
    graph1.addEdge(1, 2, 1); // B-C
    graph1.addEdge(1, 3, 5); // B-D
    graph1.addEdge(2, 3, 8); // C-D
    graph1.addEdge(2, 4, 10); // C-E
    graph1.addEdge(3, 4, 2); // D-E

    cout << "邻接矩阵 " << endl;
    graph1.printAdjMatrix();

    cout << "(2)" << endl;
    BFS(graph1, 0);
    DFS(graph1, 0);

    cout << "(3)最短路径 & 最小生成树" << endl;
    dijkstra(graph1, 0);
    primMST(graph1, 0);

    // ========== 图2：无向无权图（用于双连通分量）==========
    vector<string> labels2 = {"A", "B", "C", "D", "E", "F"};
    BiconnectedGraph graph2(6, labels2);

    // 添加边（图2）
    graph2.addEdge(0, 1); // A-B
    graph2.addEdge(0, 2); // A-C
    graph2.addEdge(1, 2); // B-C
    graph2.addEdge(1, 3); // B-D
    graph2.addEdge(3, 4); // D-E
    graph2.addEdge(4, 5); // E-F
    graph2.addEdge(3, 5); // D-F

    cout << "(4): 图2 的关节点" << endl;
    // 注意：Tarjan 算法结果与起点无关（全局性质）
    graph2.findArticulationPoints();

    // 说明：无论从哪个点开始 DFS，关节点集合是相同的
    cout << "注：关节点是图的全局属性，与 DFS 起点无关，结果一致。" << endl;

    return 0;
}