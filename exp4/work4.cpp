#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <functional>

using namespace std;
using namespace chrono;

// 边界框结构体：包含坐标和置信度
struct BBox {
    float x1, y1, x2, y2; // 左上角(x1,y1)、右下角(x2,y2)
    float score;          // 置信度（0~1）
    // 构造函数
    BBox(float x1_, float y1_, float x2_, float y2_, float score_) 
        : x1(x1_), y1(y1_), x2(x2_), y2(y2_), score(score_) {}
    // 补充默认构造函数（解决初始化报错）
    BBox() : x1(0), y1(0), x2(0), y2(0), score(0) {}
};

// 计时工具宏：简化耗时统计
#define TIME_START auto start = high_resolution_clock::now();
#define TIME_END auto end = high_resolution_clock::now(); \
    duration<double, milli> elapsed = end - start; \
    return elapsed.count();

// ====================== 排序算法实现 ======================
// 快速排序 - 分区函数
int partition(vector<BBox>& bboxes, int low, int high) {
    float pivot = bboxes[high].score;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (bboxes[j].score >= pivot) { // 降序排序
            i++;
            swap(bboxes[i], bboxes[j]);
        }
    }
    swap(bboxes[i + 1], bboxes[high]);
    return i + 1;
}

// 快速排序 - 递归核心
void quickSort(vector<BBox>& bboxes, int low, int high) {
    if (low < high) {
        int pi = partition(bboxes, low, high);
        quickSort(bboxes, low, pi - 1);
        quickSort(bboxes, pi + 1, high);
    }
}

// 快速排序 - 带计时封装
double quickSortWithTime(vector<BBox> bboxes) {
    TIME_START
    quickSort(bboxes, 0, bboxes.size() - 1);
    TIME_END
}

// 冒泡排序 - 带计时封装
double bubbleSortWithTime(vector<BBox> bboxes) {
    TIME_START
    int n = bboxes.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (bboxes[j].score < bboxes[j + 1].score) { // 降序
                swap(bboxes[j], bboxes[j + 1]);
            }
        }
    }
    TIME_END
}

// 插入排序 - 带计时封装
double insertionSortWithTime(vector<BBox> bboxes) {
    TIME_START
    int n = bboxes.size();
    for (int i = 1; i < n; i++) {
        BBox key = bboxes[i];
        int j = i - 1;
        // 降序：把比key小的元素后移
        while (j >= 0 && bboxes[j].score < key.score) {
            bboxes[j + 1] = bboxes[j];
            j--;
        }
        bboxes[j + 1] = key;
    }
    TIME_END
}

// 归并排序 - 归并操作
void merge(vector<BBox>& bboxes, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<BBox> L(n1), R(n2);
    
    // 拷贝数据到临时数组
    for (int i = 0; i < n1; i++) L[i] = bboxes[left + i];
    for (int j = 0; j < n2; j++) R[j] = bboxes[mid + 1 + j];

    // 合并两个有序数组（降序）
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].score >= R[j].score) {
            bboxes[k] = L[i];
            i++;
        } else {
            bboxes[k] = R[j];
            j++;
        }
        k++;
    }
    // 拷贝剩余元素
    while (i < n1) { bboxes[k++] = L[i++]; }
    while (j < n2) { bboxes[k++] = R[j++]; }
}

// 归并排序 - 递归核心
void mergeSort(vector<BBox>& bboxes, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2; // 避免溢出
        mergeSort(bboxes, left, mid);
        mergeSort(bboxes, mid + 1, right);
        merge(bboxes, left, mid, right);
    }
}

// 归并排序 - 带计时封装
double mergeSortWithTime(vector<BBox> bboxes) {
    TIME_START
    mergeSort(bboxes, 0, bboxes.size() - 1);
    TIME_END
}

// ====================== NMS算法实现 ======================
// 计算交并比（IOU）
float calculateIOU(const BBox& a, const BBox& b) {
    // 计算交集区域
    float interX1 = max(a.x1, b.x1);
    float interY1 = max(a.y1, b.y1);
    float interX2 = min(a.x2, b.x2);
    float interY2 = min(a.y2, b.y2);
    
    // 交集面积（无交集则为0）
    float interArea = max(0.0f, interX2 - interX1) * max(0.0f, interY2 - interY1);
    // 并集面积 = A面积 + B面积 - 交集面积
    float areaA = (a.x2 - a.x1) * (a.y2 - a.y1);
    float areaB = (b.x2 - b.x1) * (b.y2 - b.y1);
    
    return interArea / (areaA + areaB - interArea);
}

// 非极大值抑制（NMS）核心逻辑
vector<BBox> nms(vector<BBox> bboxes, float iouThreshold = 0.5) {
    vector<BBox> result;
    while (!bboxes.empty()) {
        // 取置信度最高的框
        BBox topBbox = bboxes[0];
        result.push_back(topBbox);
        
        // 剔除与topBbox重叠度过高的框
        vector<BBox> remainingBboxes;
        for (size_t i = 1; i < bboxes.size(); i++) {
            if (calculateIOU(topBbox, bboxes[i]) < iouThreshold) {
                remainingBboxes.push_back(bboxes[i]);
            }
        }
        bboxes = remainingBboxes;
    }
    return result;
}

// 排序类型枚举
enum SortType { QUICK, BUBBLE, INSERTION, MERGE };

// 带排序的NMS整体耗时统计
double nmsWithSortTime(vector<BBox> bboxes, SortType sortType) {
    TIME_START
    // 第一步：按置信度降序排序
    switch (sortType) {
        case QUICK:
            quickSort(bboxes, 0, bboxes.size() - 1);
            break;
        case BUBBLE: {
            int n = bboxes.size();
            for (int i = 0; i < n - 1; i++) {
                for (int j = 0; j < n - i - 1; j++) {
                    if (bboxes[j].score < bboxes[j + 1].score) {
                        swap(bboxes[j], bboxes[j + 1]);
                    }
                }
            }
            break;
        }
        case INSERTION: {
            int n = bboxes.size();
            for (int i = 1; i < n; i++) {
                BBox key = bboxes[i];
                int j = i - 1;
                while (j >= 0 && bboxes[j].score < key.score) {
                    bboxes[j + 1] = bboxes[j];
                    j--;
                }
                bboxes[j + 1] = key;
            }
            break;
        }
        case MERGE:
            mergeSort(bboxes, 0, bboxes.size() - 1);
            break;
    }
    // 第二步：执行NMS
    nms(bboxes);
    TIME_END
}

// ====================== 测试数据生成 ======================
// 全局随机数生成器（避免重复初始化）
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<float> coordDist(0, 1000);  // 坐标范围：0~1000
uniform_real_distribution<float> scoreDist(0, 1);    // 置信度：0~1
uniform_real_distribution<float> sizeDist(20, 100);  // 框大小：20~100像素

// 生成随机分布的边界框
vector<BBox> generateRandomBBoxes(int num) {
    vector<BBox> bboxes;
    for (int i = 0; i < num; i++) {
        float x1 = coordDist(gen);
        float y1 = coordDist(gen);
        float x2 = x1 + sizeDist(gen);
        float y2 = y1 + sizeDist(gen);
        float score = scoreDist(gen);
        bboxes.emplace_back(x1, y1, x2, y2, score);
    }
    return bboxes;
}

// 生成聚集分布的边界框（5个聚集中心）- 兼容C++11（去掉结构化绑定）
vector<BBox> generateClusteredBBoxes(int num) {
    vector<BBox> bboxes;
    // 5个固定聚集中心
    vector<pair<float, float>> centers = {{200, 200}, {200, 800}, {800, 200}, {800, 800}, {500, 500}};
    normal_distribution<float> offsetDist(0, 50); // 中心偏移（正态分布）

    for (int i = 0; i < num; i++) {
        // 替换结构化绑定：C++11兼容写法
        pair<float, float> center = centers[i % 5];
        float cx = center.first;
        float cy = center.second;
        
        float x1 = cx + offsetDist(gen);
        float y1 = cy + offsetDist(gen);
        float x2 = x1 + sizeDist(gen);
        float y2 = y1 + sizeDist(gen);
        float score = scoreDist(gen);
        bboxes.emplace_back(x1, y1, x2, y2, score);
    }
    return bboxes;
}

// ====================== 主函数（性能测试） ======================
int main() {
    // 测试数据规模：100/1000/5000/10000
    vector<int> testScales = {100, 1000, 5000, 10000};
    // 排序算法名称（与枚举对应）
    vector<string> sortNames = {"快速排序", "冒泡排序", "插入排序", "归并排序"};
    // 数据分布名称
    vector<string> distNames = {"随机分布", "聚集分布"};
    // 数据生成函数
    vector<function<vector<BBox>(int)>> genFuncs = {generateRandomBBoxes, generateClusteredBBoxes};

    // ========== 测试1：单独排序算法性能 ==========
    cout << "========================================" << endl;
    cout << "单独排序算法性能测试（单位：毫秒）" << endl;
    cout << "========================================" << endl;
    for (int distIdx = 0; distIdx < 2; distIdx++) {
        cout << "\n【" << distNames[distIdx] << "】" << endl;
        cout << "数据规模\t" << sortNames[0] << "\t" << sortNames[1] << "\t" 
             << sortNames[2] << "\t" << sortNames[3] << endl;
        cout << "----------------------------------------" << endl;
        for (int scale : testScales) {
            vector<BBox> bboxes = genFuncs[distIdx](scale);
            double t_quick = quickSortWithTime(bboxes);
            double t_bubble = bubbleSortWithTime(bboxes);
            double t_insert = insertionSortWithTime(bboxes);
            double t_merge = mergeSortWithTime(bboxes);
            
            // 格式化输出（保留3位小数）
            printf("%d\t\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n",
                   scale, t_quick, t_bubble, t_insert, t_merge);
        }
    }

    // ========== 测试2：排序+NMS整体性能 ==========
    cout << "\n========================================" << endl;
    cout << "排序+NMS 整体性能测试（单位：毫秒）" << endl;
    cout << "========================================" << endl;
    for (int distIdx = 0; distIdx < 2; distIdx++) {
        cout << "\n【" << distNames[distIdx] << "】" << endl;
        cout << "数据规模\t" << sortNames[0] << "+NMS\t" << sortNames[1] << "+NMS\t" 
             << sortNames[2] << "+NMS\t" << sortNames[3] << "+NMS" << endl;
        cout << "----------------------------------------" << endl;
        for (int scale : testScales) {
            vector<BBox> bboxes = genFuncs[distIdx](scale);
            double t_quick_nms = nmsWithSortTime(bboxes, QUICK);
            double t_bubble_nms = nmsWithSortTime(bboxes, BUBBLE);
            double t_insert_nms = nmsWithSortTime(bboxes, INSERTION);
            double t_merge_nms = nmsWithSortTime(bboxes, MERGE);
            
            // 格式化输出（保留3位小数）
            printf("%d\t\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n",
                   scale, t_quick_nms, t_bubble_nms, t_insert_nms, t_merge_nms);
        }
    }

    cout << "\n测试完成！" << endl;
    return 0;
}
