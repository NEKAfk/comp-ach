#include<stdio.h>
#include<string>
#include<time.h>
#include<iostream>
#include<math.h>
#include<random>
#include<stdint.h>
#include<omp.h>
#include<limits>

using namespace std;

bool underSurface(double x, double y, double z) {
    return (x+y+z-1 <= 0);
}

uint32_t xorshift(uint32_t& state) {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 5;
    x ^= x << 17;
    state = x;
    return x;
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

        double r = 0;
        vector<double> A(3);
        vector<double> B(3);
        vector<double> C(3);

        long long N;
        fscanf(fin, "%lli\n(%lf%lf%lf)\n(%lf%lf%lf)\n(%lf%lf%lf)", &N, &A[0], &A[1], &A[2], &B[0], &B[1], &B[2], &C[0], &C[1], &C[2]);
        r = sqrt(pow(A[0]-B[0], 2)+pow(A[1]-B[1], 2)+pow(A[2]-B[2], 2)) / 2;

        if (N<1 || r <= 0 || threads < -1) {
            throw invalid_argument("Invalid input");
        }
        
        long long c = 0;
        int mod = 1e9 + 7;
        random_device rd;

        double start = omp_get_wtime();
        #pragma omp parallel if(threads != -1) num_threads(threads)
        {
            #pragma omp single
            {
                thr = omp_get_num_threads();
            }
            long long lc = 0;
            uint32_t state = (rd() % mod) + 9;


            #pragma omp for schedule(static, N/(2*thr))
            for (long long i = 0; i < N; i++) {
                double x = (double) xorshift(state) / numeric_limits<uint32_t>::max();
                double y = (double) xorshift(state) / numeric_limits<uint32_t>::max();
                double z = (double) xorshift(state) / numeric_limits<uint32_t>::max();
                if (underSurface(x, y, z)) {
                    lc++;
                }
            }

            #pragma omp critical
            {
                c += lc;
            }
        }
        double end = omp_get_wtime();

        printf("Time (%i thread(s)): %g ms\n", threads==-1 ? 0:thr, (end - start) * 1000);
        fprintf(fout, "%g %g\n", (double)4*(r*r*r)/3, 8 * (r * r * r) * c/N);
        fclose(fout);
        fclose(fin);
        return 0;
    } catch (const exception& e) {
        cerr << e.what() << endl;
        fclose(fout);
        fclose(fin);
        return 1;
    }
    
}