
#include <stdio.h>

#define INF 9999
#define MAX 10

void dijkstra(int graph[MAX][MAX], int n, int source)
{
    int distance[MAX];
    int visited[MAX];
    int i, j, count, min, next;

    // Initialize distances and visited
    for (i = 0; i < n; i++)
    {
        distance[i] = graph[source][i];
        visited[i] = 0;
    }

    distance[source] = 0;
    visited[source] = 1;

    // Find shortest paths
    for (count = 1; count < n; count++)
    {
        min = INF;
        next = -1;

        // Find the unvisited node with minimum distance
        for (i = 0; i < n; i++)
        {
            if (!visited[i] && distance[i] < min)
            {
                min = distance[i];
                next = i;
            }
        }

        if (next == -1)
            break;

        visited[next] = 1;

        // Update distances
        for (j = 0; j < n; j++)
        {
            if (!visited[j] &&
                graph[next][j] != INF &&
                distance[next] + graph[next][j] < distance[j])
            {
                distance[j] =
                    distance[next] + graph[next][j];
            }
        }
    }

    // Display result
    printf("\nShortest distances from Router %d:\n", source);

    for (i = 0; i < n; i++)
    {
        printf("Router %d -> Router %d = %d\n",
               source, i, distance[i]);
    }
}

int main()
{
    int graph[MAX][MAX];
    int n, source;
    int i, j;

    printf("Enter number of routers: ");
    scanf("%d", &n);

    printf("Enter the cost matrix:\n");
    printf("(Enter 9999 if there is no direct connection)\n");

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            scanf("%d", &graph[i][j]);
        }
    }

    printf("Enter source router: ");
    scanf("%d", &source);

    dijkstra(graph, n, source);

    return 0;
}
