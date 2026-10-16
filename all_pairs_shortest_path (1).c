#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10      /* 정점의 수 */
#define M 20      /* 간선의 수 */
#define MIN_W 1   /* 최소 가중치 */
#define MAX_W 100 /* 최대 가중치 */
#define INF 1000000000

static int w[N][N];            /* 인접 행렬: 0이면 간선 없음 */
static int edge_u[M], edge_v[M]; /* 간선 목록 (u < v) */

/* lo ~ hi 사이의 난수 반환 */
static int rand_range(int lo, int hi) { return lo + rand() % (hi - lo + 1); }

/* idx번째 간선을 추가하고 무작위 가중치를 부여 */
static void add_edge(int idx, int a, int b) {
    int u = a < b ? a : b, v = a < b ? b : a;
    int weight = rand_range(MIN_W, MAX_W);
    w[u][v] = w[v][u] = weight;
    edge_u[idx] = u;
    edge_v[idx] = v;
}

/* 연결된 가중치 무방향 단순 그래프를 무작위로 생성 */
static void generate_graph(void) {
    int order[N], i, cnt = 0;
    for (i = 0; i < N; i++) order[i] = i;
    for (i = N - 1; i > 0; i--) {            /* 정점 순서를 무작위로 섞기 */
        int j = rand() % (i + 1), t = order[i];
        order[i] = order[j];
        order[j] = t;
    }
    /* 무작위 신장 트리(간선 N-1개)로 연결 그래프 보장 */
    for (i = 1; i < N; i++)
        add_edge(cnt++, order[i], order[rand() % i]);

    /* 자기 자신 간선과 중복 간선 없이 나머지 간선 추가 */
    while (cnt < M) {
        int a = rand() % N, b = rand() % N;
        if (a == b || w[a][b] != 0) continue;
        add_edge(cnt++, a, b);
    }
}

/* 0번 정점에서 DFS로 모든 정점에 도달 가능한지 확인 */
static int is_connected(void) {
    int seen[N] = {0}, stack[N], top = 0, count = 1, u, v;
    stack[top++] = 0;
    seen[0] = 1;
    while (top > 0) {
        u = stack[--top];
        for (v = 0; v < N; v++)
            if (w[u][v] && !seen[v]) {
                seen[v] = 1;
                count++;
                stack[top++] = v;
            }
    }
    return count == N;
}

/* src에서 시작하는 Dijkstra: 최단거리(dist)와 이전 정점(prev) 저장 */
static void dijkstra(int src, int dist[], int prev[]) {
    int done[N] = {0}, i, k;
    for (i = 0; i < N; i++) { dist[i] = INF; prev[i] = -1; }
    dist[src] = 0;
    for (k = 0; k < N; k++) {
        int u = -1;
        for (i = 0; i < N; i++)          /* 미확정 정점 중 거리가 가장 작은 정점 선택 */
            if (!done[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INF) break;
        done[u] = 1;
        for (i = 0; i < N; i++)          /* 간선 완화(relaxation) */
            if (w[u][i] && dist[u] + w[u][i] < dist[i]) {
                dist[i] = dist[u] + w[u][i];
                prev[i] = u;
            }
    }
}

/* 간선을 (u, v) 순으로 정렬하기 위한 비교 함수 */
static int cmp_edges(const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    if (edge_u[ia] != edge_u[ib]) return edge_u[ia] - edge_u[ib];
    return edge_v[ia] - edge_v[ib];
}

int main(void) {
    int idx[M], i, u, v, pairs = 0, all_ok = 1;
    int dist[N], prev[N];

    srand((unsigned)time(NULL));
    generate_graph();

    /* ---- 생성된 그래프 출력 ---- */
    for (i = 0; i < M; i++) idx[i] = i;
    qsort(idx, M, sizeof(int), cmp_edges);

    printf("그래프\n정점 수 : %d\n간선 수 : %d\n\n간선 목록\n", N, M);
    for (i = 0; i < M; i++)
        printf("(%d, %d) 가중치: %d\n", edge_u[idx[i]], edge_v[idx[i]],
               w[edge_u[idx[i]]][edge_v[idx[i]]]);

    /* ---- 모든 정점 쌍(u < v)의 최단경로 출력 ---- */
    printf("\n최단경로\n\n");
    for (u = 0; u < N; u++) {
        dijkstra(u, dist, prev);
        for (v = u + 1; v < N; v++) {
            int path[N], len = 0, cur = v, sum = 0;
            while (cur != -1) { path[len++] = cur; cur = prev[cur]; } /* 역추적 */

            printf("%d -> %d\n경로     : ", u, v);
            for (i = len - 1; i >= 0; i--) {
                printf("%d", path[i]);
                if (i > 0) printf(" -> ");
            }
            printf("\n최단거리 : %d\n\n", dist[v]);

            /* 경로상의 간선 가중치 합이 최단거리와 같은지 검증 */
            for (i = len - 1; i > 0; i--) sum += w[path[i]][path[i - 1]];
            if (sum != dist[v] || path[len - 1] != u) all_ok = 0;
            pairs++;
        }
    }
    printf("전체 정점 쌍 수 : %d\n\n", pairs);

    /* ---- 확인 사항 ---- */
    {
        int loop_ok = 1, edge_cnt = 0, weights_ok = 1;
        for (u = 0; u < N; u++) {
            if (w[u][u] != 0) loop_ok = 0;            /* 자기 자신 간선 검사 */
            for (v = u + 1; v < N; v++)
                if (w[u][v]) {
                    edge_cnt++;
                    if (w[u][v] < MIN_W || w[u][v] > MAX_W) weights_ok = 0;
                }
        }
        /* 인접 행렬은 같은 쌍을 한 칸에만 저장하므로 중복 간선이 생길 수 없음 */
        printf("확인 사항\n");
        printf("정점이 정확히 10개인가?            : %s\n", N == 10 ? "예" : "아니오");
        printf("간선이 정확히 20개인가?            : %s\n", edge_cnt == 20 ? "예" : "아니오");
        printf("자기 자신/중복 간선이 없는가?      : %s\n", loop_ok ? "예" : "아니오");
        printf("가중치가 1~100 범위인가?           : %s\n", weights_ok ? "예" : "아니오");
        printf("연결 그래프인가?                   : %s\n", is_connected() ? "예" : "아니오");
        printf("45개 정점 쌍이 모두 출력되었는가?  : %s\n", pairs == 45 ? "예" : "아니오");
        printf("최단거리 = 경로 가중치의 합인가?   : %s\n", all_ok ? "예" : "아니오");
    }
    return 0;
}
