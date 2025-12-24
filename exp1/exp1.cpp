#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <stack>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

using namespace std;

// work1.cpp 复数相关实现
class Complex {
public:
    double real, imag;

    Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

    double modulus() const {
        return sqrt(real * real + imag * imag);
    }

    bool operator==(const Complex& other) const {
        const double eps = 1e-9;
        return abs(real - other.real) < eps && abs(imag - other.imag) < eps;
    }

    bool operator!=(const Complex& other) const {
        return !(*this == other);
    }

    friend ostream& operator<<(ostream& os, const Complex& c) {
        os << "(" << c.real << "," << c.imag << ")";
        return os;
    }
};

bool compareComplex(const Complex& a, const Complex& b) {
    double modA = a.modulus();
    double modB = b.modulus();
    if (abs(modA - modB) < 1e-9) {
        return a.real < b.real;
    }
    return modA < modB;
}

void bubbleSort(vector<Complex>& vec) {
    size_t n = vec.size();
    for (size_t i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (size_t j = 0; j < n - i - 1; ++j) {
            if (!compareComplex(vec[j], vec[j + 1])) {
                swap(vec[j], vec[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void merge(vector<Complex>& vec, size_t left, size_t mid, size_t right) {
    vector<Complex> leftArr(vec.begin() + left, vec.begin() + mid + 1);
    vector<Complex> rightArr(vec.begin() + mid + 1, vec.begin() + right + 1);

    size_t i = 0, j = 0, k = left;
    while (i < leftArr.size() && j < rightArr.size()) {
        if (compareComplex(leftArr[i], rightArr[j])) {
            vec[k++] = leftArr[i++];
        } else {
            vec[k++] = rightArr[j++];
        }
    }
    while (i < leftArr.size()) vec[k++] = leftArr[i++];
    while (j < rightArr.size()) vec[k++] = rightArr[j++];
}

void mergeSortHelper(vector<Complex>& vec, size_t left, size_t right) {
    if (left < right) {
        size_t mid = left + (right - left) / 2;
        mergeSortHelper(vec, left, mid);
        mergeSortHelper(vec, mid + 1, right);
        merge(vec, left, mid, right);
    }
}

void mergeSort(vector<Complex>& vec) {
    if (!vec.empty())
        mergeSortHelper(vec, 0, vec.size() - 1);
}

vector<Complex> generateRandomComplexVector(size_t n, unsigned seed = 0) {
    mt19937 gen(seed == 0 ? unsigned(time(nullptr)) : seed);
    uniform_real_distribution<double> dis(-10.0, 10.0);
    vector<Complex> vec;
    for (size_t i = 0; i < n; ++i) {
        vec.emplace_back(dis(gen), dis(gen));
    }
    return vec;
}

bool findComplex(const vector<Complex>& vec, const Complex& target) {
    return find(vec.begin(), vec.end(), target) != vec.end();
}

void insertComplex(vector<Complex>& vec, const Complex& c) {
    vec.push_back(c);
}

bool removeComplex(vector<Complex>& vec, const Complex& c) {
    auto it = find(vec.begin(), vec.end(), c);
    if (it != vec.end()) {
        vec.erase(it);
        return true;
    }
    return false;
}

vector<Complex> uniqueComplex(vector<Complex> vec) {
    sort(vec.begin(), vec.end(), [](const Complex& a, const Complex& b) {
        if (abs(a.real - b.real) < 1e-9)
            return a.imag < b.imag;
        return a.real < b.real;
    });
    auto last = unique(vec.begin(), vec.end());
    vec.erase(last, vec.end());
    return vec;
}

vector<Complex> rangeFind(const vector<Complex>& sortedVec, double m1, double m2) {
    vector<Complex> result;
    for (const auto& c : sortedVec) {
        double mod = c.modulus();
        if (mod >= m1 && mod < m2) {
            result.push_back(c);
        }
    }
    return result;
}

void testSortingPerformance(const vector<Complex>& baseVec) {
    cout << "\n=== 排序性能测试 (n = " << baseVec.size() << ") ===\n";

    auto sortedVec = baseVec;
    sort(sortedVec.begin(), sortedVec.end(), compareComplex);

    auto reversedVec = sortedVec;
    reverse(reversedVec.begin(), reversedVec.end());

    auto shuffledVec = baseVec;
    random_shuffle(shuffledVec.begin(), shuffledVec.end());

    auto testSort = [&](const vector<Complex>& input, const string& name, void (*sortFunc)(vector<Complex>&)) {
        auto vec = input;
        clock_t start = clock();
        sortFunc(vec);
        clock_t end = clock();
        double time = double(end - start) / CLOCKS_PER_SEC * 1000.0;
        cout << name << ": " << fixed << setprecision(2) << time << " ms\n";
    };

    cout << "冒泡排序:\n";
    testSort(sortedVec, "  顺序", bubbleSort);
    testSort(reversedVec, "  逆序", bubbleSort);
    testSort(shuffledVec, "  乱序", bubbleSort);

    cout << "归并排序:\n";
    testSort(sortedVec, "  顺序", mergeSort);
    testSort(reversedVec, "  逆序", mergeSort);
    testSort(shuffledVec, "  乱序", mergeSort);
}

// work2.cpp 表达式计算相关实现
#define MAX_EXPR_LEN 100
#define MAX_STACK_SIZE 100
#define N_OPTR 14

typedef enum {
    ADD, SUB, MUL, DIV, POW, FAC, L_P, R_P, EOE, SIN, COS, TAN, LOG, LN
} Operator;

typedef struct {
    double data[MAX_STACK_SIZE];
    int top;
} OpndStack;

typedef struct {
    Operator data[MAX_STACK_SIZE];
    int top;
} OptrStack;

void initOpndStack(OpndStack *s) {
    s->top = -1;
}

void initOptrStack(OptrStack *s) {
    s->top = -1;
}

int isOpndEmpty(OpndStack *s) {
    return s->top == -1;
}

int isOptrEmpty(OptrStack *s) {
    return s->top == -1;
}

void pushOpnd(OpndStack *s, double val) {
    if (s->top < MAX_STACK_SIZE - 1) {
        s->data[++(s->top)] = val;
    } else {
        printf("操作数栈溢出\n");
        exit(1);
    }
}

void pushOptr(OptrStack *s, Operator op) {
    if (s->top < MAX_STACK_SIZE - 1) {
        s->data[++(s->top)] = op;
    } else {
        printf("运算符栈溢出\n");
        exit(1);
    }
}

double popOpnd(OpndStack *s) {
    if (!isOpndEmpty(s)) {
        return s->data[(s->top)--];
    } else {
        printf("操作数栈为空\n");
        exit(1);
    }
}

Operator popOptr(OptrStack *s) {
    if (!isOptrEmpty(s)) {
        return s->data[(s->top)--];
    } else {
        printf("运算符栈为空\n");
        exit(1);
    }
}

double getTopOpnd(OpndStack *s) {
    if (!isOpndEmpty(s)) {
        return s->data[s->top];
    } else {
        printf("操作数栈为空\n");
        exit(1);
    }
}

Operator getTopOptr(OptrStack *s) {
    if (!isOptrEmpty(s)) {
        return s->data[s->top];
    } else {
        printf("运算符栈为空\n");
        exit(1);
    }
}

const char pri[N_OPTR][N_OPTR] = {
    /*          +    -    *    /    ^    !    (    )    \0   SIN  COS  TAN  LOG  LN */
    /* + */   '>', '>', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>',
    /* - */   '>', '>', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>',
    /* * */   '>', '>', '>', '>', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>',
    /* / */   '>', '>', '>', '>', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>',
    /* ^ */   '>', '>', '>', '>', '>', '<', '<', '>', '>', '>', '>', '>', '>', '>',
    /* ! */   '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>',
    /* ( */   '<', '<', '<', '<', '<', '<', '<', '=', ' ', '<', '<', '<', '<', '<',
    /* ) */   ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    /* \0*/   '<', '<', '<', '<', '<', '<', '<', ' ', '=', '<', '<', '<', '<', '<',
    /* SIN*/  '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>',
    /* COS*/  '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>',
    /* TAN*/  '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>',
    /* LOG*/  '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>',
    /* LN */  '>', '>', '>', '>', '>', '>', ' ', '>', '>', '>', '>', '>', '>', '>'
};

int getNextToken(const char *expr, int *pos, double *num, Operator *op) {
    char c = expr[*pos];
    if (c == '\0') {
        *op = EOE;
        return 0;
    } else if (c >= '0' && c <= '9') {
        sscanf(&expr[*pos], "%lf", num);
        while (expr[*pos] >= '0' && expr[*pos] <= '9' || expr[*pos] == '.') {
            (*pos)++;
        }
        return 1;
    } else {
        (*pos)++;
        switch (c) {
            case '+': *op = ADD; break;
            case '-': *op = SUB; break;
            case '*': *op = MUL; break;
            case '/': *op = DIV; break;
            case '^': *op = POW; break;
            case '!': *op = FAC; break;
            case '(': *op = L_P; break;
            case ')': *op = R_P; break;
            case 's':
                if (expr[*pos] == 'i' && expr[*pos + 1] == 'n') {
                    *op = SIN;
                    (*pos) += 2;
                } else {
                    printf("无效的函数名: s%c%c\n", expr[*pos], expr[*pos+1]);
                    return -1;
                }
                break;
            case 'c':
                if (expr[*pos] == 'o' && expr[*pos + 1] == 's') {
                    *op = COS;
                    (*pos) += 2;
                } else {
                    printf("无效的函数名: c%c%c\n", expr[*pos], expr[*pos+1]);
                    return -1;
                }
                break;
            case 't':
                if (expr[*pos] == 'a' && expr[*pos + 1] == 'n') {
                    *op = TAN;
                    (*pos) += 2;
                } else {
                    printf("无效的函数名: t%c%c\n", expr[*pos], expr[*pos+1]);
                    return -1;
                }
                break;
            case 'l':
                if (expr[*pos] == 'o' && expr[*pos + 1] == 'g') {
                    *op = LOG;
                    (*pos) += 2;
                } else if (expr[*pos] == 'n') {
                    *op = LN;
                    (*pos)++;
                } else {
                    printf("无效的函数名: l%c%c\n", expr[*pos], expr[*pos+1]);
                    return -1;
                }
                break;
            default:
                printf("无效的字符: %c\n", c);
                return -1;
        }
        return 0;
    }
}

double calculate(Operator op, OpndStack *opndStack) {
    double b, a, result = 0;
    switch (op) {
        case ADD:
            b = popOpnd(opndStack);
            a = popOpnd(opndStack);
            result = a + b;
            break;
        case SUB:
            b = popOpnd(opndStack);
            a = popOpnd(opndStack);
            result = a - b;
            break;
        case MUL:
            b = popOpnd(opndStack);
            a = popOpnd(opndStack);
            result = a * b;
            break;
        case DIV:
            b = popOpnd(opndStack);
            a = popOpnd(opndStack);
            if (b == 0) {
                printf("除数不能为0\n");
                exit(1);
            }
            result = a / b;
            break;
        case POW:
            b = popOpnd(opndStack);
            a = popOpnd(opndStack);
            result = pow(a, b);
            break;
        case FAC:
            a = popOpnd(opndStack);
            if (a < 0 || (int)a != a) {
                printf("阶乘仅适用于非负整数\n");
                exit(1);
            }
            result = 1;
            for (int i = 1; i <= (int)a; i++) {
                result *= i;
            }
            break;
        case SIN:
            a = popOpnd(opndStack);
            result = sin(a);
            break;
        case COS:
            a = popOpnd(opndStack);
            result = cos(a);
            break;
        case TAN:
            a = popOpnd(opndStack);
            result = tan(a);
            break;
        case LOG:
            a = popOpnd(opndStack);
            if (a <= 0) {
                printf("对数的参数必须大于0\n");
                exit(1);
            }
            result = log10(a);
            break;
        case LN:
            a = popOpnd(opndStack);
            if (a <= 0) {
                printf("自然对数的参数必须大于0\n");
                exit(1);
            }
            result = log(a);
            break;
        default:
            printf("无效的运算符\n");
            exit(1);
    }
    return result;
}

char priority(Operator op1, Operator op2) {
    return pri[op1][op2];
}

int evaluateExpression(const char *expr, double *result) {
    OpndStack opndStack;
    OptrStack optrStack;
    double num;
    Operator op, topOp;
    int pos = 0;
    int tokenResult;

    initOpndStack(&opndStack);
    initOptrStack(&optrStack);

    pushOptr(&optrStack, EOE);

    while (1) {
        tokenResult = getNextToken(expr, &pos, &num, &op);
        if (tokenResult == -1) {
            return 0;
        }
        
        if (tokenResult == 1) {
            pushOpnd(&opndStack, num);
        } else {
            while (priority(getTopOptr(&optrStack), op) == '>') {
                topOp = popOptr(&optrStack);
                if (topOp == EOE) {
                    *result = getTopOpnd(&opndStack);
                    return 1;
                }
                double res = calculate(topOp, &opndStack);
                pushOpnd(&opndStack, res);
            }

            if (priority(getTopOptr(&optrStack), op) == '<') {
                pushOptr(&optrStack, op);
            } else if (priority(getTopOptr(&optrStack), op) == '=') {
                popOptr(&optrStack);
                if (op != R_P) {
                    pushOptr(&optrStack, op);
                }
            } else {
                return 0;
            }

            if (op == EOE && getTopOptr(&optrStack) == EOE) {
                break;
            }
        }
    }

    *result = getTopOpnd(&opndStack);
    return 1;
}

// work3.cpp 最大矩形面积相关实现
class Solution {
public:
    int largestRectangleArea(vector<int>& heights) {
        int n = heights.size();
        stack<int> st;
        vector<int> left(n, 0);
        vector<int> right(n, 0);

        for (int i = 0; i < n; ++i) {
            while (!st.empty() && heights[st.top()] >= heights[i]) {
                st.pop();
            }
            left[i] = st.empty() ? -1 : st.top();
            st.push(i);
        }

        while (!st.empty()) st.pop();

        for (int i = n - 1; i >= 0; --i) {
            while (!st.empty() && heights[st.top()] >= heights[i]) {
                st.pop();
            }
            right[i] = st.empty() ? n : st.top();
            st.push(i);
        }

        int maxArea = 0;
        for (int i = 0; i < n; ++i) {
            maxArea = max(maxArea, heights[i] * (right[i] - left[i] - 1));
        }

        return maxArea;
    }
};

vector<int> generateRandomHeights(int length, int max_height) {
    vector<int> heights;
    for (int i = 0; i < length; ++i) {
        heights.push_back(rand() % (max_height + 1));
    }
    return heights;
}

// 主函数，依次执行三个程序的测试
int main() {
    // 1. 执行复数相关测试 (work1.cpp)
    cout << "======= 复数向量操作测试 =======" << endl;
    const size_t N = 1000;
    vector<Complex> vec = generateRandomComplexVector(N, 12345);

    cout << "原始向量（前10个）:\n";
    for (size_t i = 0; i < min(size_t(10), vec.size()); ++i) {
        cout << vec[i] << " ";
    }
    cout << "\n\n";

    random_shuffle(vec.begin(), vec.end());
    cout << "置乱后（前10个）:\n";
    for (size_t i = 0; i < min(size_t(10), vec.size()); ++i) {
        cout << vec[i] << " ";
    }
    cout << "\n";

    Complex target(2.5, -3.1);
    insertComplex(vec, target);
    cout << "插入 " << target << " 后查找: " << (findComplex(vec, target) ? "找到" : "未找到") << "\n";

    removeComplex(vec, target);
    cout << "删除后查找: " << (findComplex(vec, target) ? "找到" : "未找到") << "\n";

    auto uniqueVec = uniqueComplex(vec);
    cout << "唯一化后大小: " << uniqueVec.size() << " (原大小: " << vec.size() << ")\n";

    testSortingPerformance(vec);

    vector<Complex> sortedForRange = vec;
    sort(sortedForRange.begin(), sortedForRange.end(), compareComplex);
    double m1 = 5.0, m2 = 10.0;
    auto rangeResult = rangeFind(sortedForRange, m1, m2);
    cout << "\n区间 [" << m1 << ", " << m2 << ") 内的元素个数: " << rangeResult.size() << "\n";
    cout << "前5个: ";
    for (size_t i = 0; i < min(size_t(5), rangeResult.size()); ++i) {
        cout << rangeResult[i] << " ";
    }
    cout << "\n\n";

    // 2. 执行表达式计算测试 (work2.cpp)
    cout << "======= 字符串计算器测试 =======" << endl;
    const char *testCases[] = {
        "2+3*4",
        "(1+2)*3!",
        "2^3^2",
        "5!",
        "2*(3+4)"
    };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);
    
    cout << "字符串计算器测试案例:\n";
    cout << "======================\n";
    
    for (int i = 0; i < numTests; i++) {
        double result;
        int valid = evaluateExpression(testCases[i], &result);
        
        printf("%s\t", testCases[i]);
        if (valid) {
            printf("%.6f\n", result);
        } else {
            printf("表达式无效\n");
        }
    }
    cout << endl;

    // 3. 执行最大矩形面积测试 (work3.cpp)
    cout << "======= 最大矩形面积测试 =======" << endl;
    srand(time(0));
    Solution solution;

    vector<int> heights1 = {2, 1, 5, 6, 2, 3};
    cout << "示例 1:" << endl;
    cout << "输入: heights = [";
    for (size_t i = 0; i < heights1.size(); ++i) {
        cout << heights1[i];
        if (i != heights1.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    cout << "输出: " << solution.largestRectangleArea(heights1) << endl << endl;

    vector<int> heights2 = {2, 4};
    cout << "示例 2:" << endl;
    cout << "输入: heights = [";
    for (size_t i = 0; i < heights2.size(); ++i) {
        cout << heights2[i];
        if (i != heights2.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
    cout << "输出: " << solution.largestRectangleArea(heights2) << endl << endl;

    cout << "随机生成 10 组数据进行测试:" << endl;
    for (int i = 1; i <= 10; ++i) {
        int length = rand() % 105 + 1;
        vector<int> heights = generateRandomHeights(length, 104);
        cout << "测试用例 " << i << ":" << endl;
        cout << "输入: heights = [";
        for (size_t j = 0; j < heights.size(); ++j) {
            cout << heights[j];
            if (j != heights.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
        cout << "输出: " << solution.largestRectangleArea(heights) << endl << endl;
    }

    return 0;
}