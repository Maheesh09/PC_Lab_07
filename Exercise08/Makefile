# Lab 07 - MPI collectives
#   make        build all 6 programs
#   make run    run all 6 with 4 processes
#   make clean  delete the binaries
CC    = mpicc
FLAGS = -O2 -Wall
NP    = 4

PROGS = Exercise01/sum_bcast Exercise02/sum_scatter Exercise03/sum_gather \
        Exercise04/sum_reduce Exercise05/sum_allreduce Exercise06/sum_scan

all: $(PROGS)

%: %.c
	$(CC) $(FLAGS) -o $@ $<

run: all
	@for p in $(PROGS); do \
		echo ""; echo "===== $$p (np=$(NP)) ====="; \
		mpirun -np $(NP) ./$$p; \
	done

clean:
	rm -f $(PROGS)

.PHONY: all run clean
