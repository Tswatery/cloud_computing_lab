#include <bits/stdc++.h>

using namespace std;

const int N = 1e6 + 10;
int ans[10][10], stk[1000];

struct DLX
{
    static const int MAXSIZE = 1e5 + 10;
    int n, m, tot, first[MAXSIZE + 10], siz[MAXSIZE + 10];
    int L[MAXSIZE + 10], R[MAXSIZE + 10], U[MAXSIZE + 10], D[MAXSIZE + 10];
    int col[MAXSIZE + 10], row[MAXSIZE + 10];
// col指向的是当前元素所在的列 row是行
    void build(const int &r, const int &c) // 构建一个长度为c的Dancing Link
    { // 进行build操作
        n = r, m = c;
        for (int i = 0; i <= c; ++i)
        {
            L[i] = i - 1, R[i] = i + 1;
            U[i] = D[i] = i;
        }
        L[0] = c, R[c] = 0, tot = c;
        memset(first, 0, sizeof(first));
        memset(siz, 0, sizeof(siz));
    }

    void insert(const int &r, const int &c)
    { // 进行insert操作
        col[++tot] = c, row[tot] = r, ++siz[c];
        D[tot] = D[c], U[D[c]] = tot, U[tot] = c, D[c] = tot;
        if (!first[r])
            first[r] = L[tot] = R[tot] = tot;
        else
        {
            R[tot] = R[first[r]], L[R[first[r]]] = tot;
            L[tot] = first[r], R[first[r]] = tot;
        }
    }

    void remove(const int &c)
    { // 进行remove操作
        int i, j;
        L[R[c]] = L[c], R[L[c]] = R[c];
        for (i = D[c]; i != c; i = D[i])
            for (j = R[i]; j != i; j = R[j])
                U[D[j]] = U[j], D[U[j]] = D[j], --siz[col[j]];
    }

    void recover(const int &c)
    { // 进行recover操作
        int i, j;
        for (i = U[c]; i != c; i = U[i])
            for (j = L[i]; j != i; j = L[j])
                U[D[j]] = D[U[j]] = j, ++siz[col[j]];
        L[R[c]] = R[L[c]] = c;
    }

    bool dance(int dep)
    { // dance
        int i, j, c = R[0];
        if (!R[0]) // 图中没有点了
        {
            for (i = 1; i < dep; ++i)
            {
                int x = (stk[i] - 1) / 9 / 9 + 1;
                int y = (stk[i] - 1) / 9 % 9 + 1;
                int v = (stk[i] - 1) % 9 + 1;
                ans[x][y] = v;
            }
            return 1;
        }
        for (i = R[0]; i != 0; i = R[i])
            if (siz[i] < siz[c])
                c = i;
        remove(c);
        for (i = D[c]; i != c; i = D[i])
        {
            stk[dep] = row[i];//选择i所在的行
            for (j = R[i]; j != i; j = R[j])
                remove(col[j]); 
            if (dance(dep + 1))
                return 1;
            for (j = L[i]; j != i; j = L[j])
                recover(col[j]);
        }
        recover(c);
        return 0;
    }
} solver;

int GetId(int row, int col, int num)
{
    return (row - 1) * 9 * 9 + (col - 1) * 9 + num;
}

void Insert(int row, int col, int num)
{
    int dx = (row - 1) / 3 + 1;
    int dy = (col - 1) / 3 + 1;
    int room = (dx - 1) * 3 + dy;
    int id = GetId(row, col, num);
    int f1 = (row - 1) * 9 + num;           // task 1
    int f2 = 81 + (col - 1) * 9 + num;      // task 2
    int f3 = 81 * 2 + (room - 1) * 9 + num; // task 3
    int f4 = 81 * 3 + (row - 1) * 9 + col;  // task 4
    solver.insert(id, f1);
    solver.insert(id, f2);
    solver.insert(id, f3);
    solver.insert(id, f4);
}

int main()
{
	string file_name;
	cin >> file_name;
	file_name = file_name.substr(2);
    ifstream input(file_name);
	string line;
	while(getline(input, line)) {
        solver.build(729, 324);
        for (int i = 1; i <= 9; ++i)
            for (int j = 1; j <= 9; ++j)
            {
				int idx = (i - 1) * 9 + j - 1;
               ans[i][j] = line[idx] - '0';
                for (int v = 1; v <= 9; ++v)
                {
                    if (ans[i][j] && ans[i][j] != v)
                        continue;
                    Insert(i, j, v);
                }
            }
        solver.dance(1);
        for (int i = 1; i <= 9; ++i)
            for (int j = 1; j <= 9; ++j)
                printf("%d", ans[i][j]);
        puts("");    
    }

    return 0;
}
