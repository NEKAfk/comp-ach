#include<stdio.h>
#include<string>
#include<time.h>
#include<iostream>
#include<math.h>
#include<random>
#include<stdint.h>
#include<omp.h>
#include<queue>

using namespace std;

double halton_seq(long index, long base) {
    double res = 0;
    double f = 1;
    while (index > 0) {
        f = f / base;
        res = res + f * (index % base);
        index = index/base;
    }
    return res;
}

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

uint64_t next(vector<uint64_t>& s) {
	const uint64_t s0 = s[0];
	uint64_t s1 = s[1];
	const uint64_t result = s0 + s1;

	s1 ^= s0;
	s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
	s[1] = rotl(s1, 37); // c

	return result;
}

int main(int argc, char* argv[]) {
    FILE* fout;
    FILE* fin;
    try {
        if (argc != 4) {
            throw invalid_argument("Input mismatch " + to_string(argc) + " arguments instead of 4");
        }
        int threads = stoi(argv[1]);
        int thr;
        fin = fopen(argv[2], "r");
        fout = fopen(argv[3], "w");
        if (fin == NULL) {
            throw runtime_error("Input file not found");
        } else if (fout == NULL) {
            throw runtime_error("Output file not found");
        }

        double r;
        long N;
        fscanf(fin, "%li%lf", &N, &r);
        if (N<1 || r <= 0 || threads < -1) {
            throw invalid_argument("Invalid input");
        }
        long c = 0;
        mt19937 gen(time(nullptr));
        double start = omp_get_wtime();
        #pragma omp parallel if(threads != -1) num_threads(threads)
        {
            #pragma omp single
            {
                thr = omp_get_num_threads();
            }
            long lc = 0;
            vector<uint64_t> s(2, 0);
            s[0] = (omp_get_thread_num() << 24) % 24;
            s[1] = (omp_get_thread_num() << 16) % 37;

            #pragma omp for schedule(guided, 1)
            for (long i = 0; i < N; i++) {
                double v = ((double)next(s) / (1ULL << 63)) - 1;
                double u = ((double)next(s) / (1ULL << 63)) - 1;
                cout << v << ' ' << u << endl;
                if (pow(u, 2) + pow(v, 2) <= 1.0) {
                    lc++;
                }
            }

            #pragma omp critical
            {
                c += lc;
            }
        }
        double end = omp_get_wtime();
        printf("Time (%i thread(s)): %g ms\n", threads==-1 ? 0:thr, (end - start)*1000);
        fprintf(fout, "%g %g\n", M_PI*(r*r), 4 * (r * r) * c/N);
        fclose(fout);
        fclose(fin);
        return 0;
    } catch (const exception& e) {
        cout << e.what() << endl;
        fclose(fout);
        fclose(fin);
        return 1;
    }
}