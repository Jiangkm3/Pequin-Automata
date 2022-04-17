#include <stdint.h>

struct In {
	uint32_t n;
	uint32_t c[MAX_N];
};

struct Out {
	uint32_t d[MAX_N];
};

void compute(struct In *input, struct Out *output) {
	int n = input->n;
	int k, tmp;
	uint32_t cc[MAX_N];

	// Copy C to CC and modify CC
	for (k = 0; k < MAX_N; k++) {
		if (k < n) cc[k] = input->c[k];
	}

	// Find i
	int i = -1;
	int d_i, d_j;
	for (k = 2; k < MAX_N; k++) {
		// Assume that cc has a next permutation
		// This means that the while loop must terminate before i = 0
		tmp = MAX_N - k;
		if (i == -1 && tmp < n - 1) {
			if (cc[tmp] < cc[tmp + 1]) {
				i = tmp;
				d_i = cc[tmp];
			}
		}
	}

	// Find j
	int j = -1;
	for (k = 1; k < MAX_N - 1; k++) {
		tmp = MAX_N - k;
		if (j == -1 && tmp < n) {
			// j must be greater than i
			if (cc[tmp] > d_i) {
				j = tmp;
				d_j = cc[tmp];
			}
		}
	}

	// Swap D_i and D_j
	cc[i] = d_j; cc[j] = d_i;

	// Copy CC to D. Revert D[i+1..n]
	for (k = 0; k < MAX_N; k += 1) {
		if (k <= i) {
			output->d[k] = cc[k];
		} else if (k < n) {
			output->d[k] = cc[n - k + i];
		}
	}
}
