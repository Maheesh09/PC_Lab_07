/* Exercise 5: Scatter + Allreduce
 * Change from Ex 4: MPI_Reduce -> MPI_Allreduce (no root argument).
 * Now EVERY process has total_sum, so each can print its percentage contribution.
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 1000000

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int chunk_size = N / size;

    int *array = NULL;
    if (rank == 0) {
        array = (int *)malloc(N * sizeof(int));
        for (int i = 0; i < N; i++)
            array[i] = i + 1;
        printf("Root filled array with values 1 to %d\n", N);
    }

    int *local_chunk = (int *)malloc(chunk_size * sizeof(int));

    double start = MPI_Wtime();

    MPI_Scatter(array, chunk_size, MPI_INT,
                local_chunk, chunk_size, MPI_INT,
                0, MPI_COMM_WORLD);

    long long local_sum = 0;
    for (int i = 0; i < chunk_size; i++)
        local_sum += local_chunk[i];

    /* CHANGED: Allreduce has no root -> the result arrives on ALL processes */
    long long total_sum = 0;
    MPI_Allreduce(&local_sum, &total_sum, 1, MPI_LONG_LONG,
                  MPI_SUM, MPI_COMM_WORLD);

    double elapsed = MPI_Wtime() - start;

    /* CHANGED: every process can now use total_sum */
    double percent = 100.0 * local_sum / total_sum;
    printf("  Rank %d: local_sum = %lld, total_sum = %lld, contribution = %.2f%%\n",
           rank, local_sum, total_sum, percent);

    if (rank == 0) {
        long long expected = (long long)N * (N + 1) / 2;
        printf("\n[Allreduce] Total sum   = %lld\n", total_sum);
        printf("[Allreduce] Expected    = %lld\n", expected);
        printf("[Allreduce] Correct?    = %s\n", total_sum == expected ? "YES" : "NO");
        printf("[Allreduce] Time        = %.4f sec\n", elapsed);
        free(array);
    }

    free(local_chunk);
    MPI_Finalize();
    return 0;
}
