#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <complex>
#include <cmath>
#include <random>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <string>

using Complex = std::complex<double>;
using CVector = std::vector<Complex>;

static const double PI = std::acos(-1.0);

// ============================================================
//  FFT class: supports transform lengths of the form 2^a * 3^b * 5^c
// ============================================================
class FFT {
public:
    // Returns true if n is a valid transform length (2^a * 3^b * 5^c)
    static bool isValidSize(int n) {
        if (n <= 0) return false;
        for (int p : {2, 3, 5}) {
            while (n % p == 0) n /= p;
        }
        return n == 1;
    }

    // Forward DFT: exponent sign -1
    static void forward(CVector& x) {
        fft(x, false);
    }

    // Inverse DFT: exponent sign +1, normalized by 1/N
    static void inverse(CVector& x) {
        fft(x, true);
        const double inv = 1.0 / static_cast<double>(x.size());
        for (auto& v : x) v *= inv;
    }

private:
    // Recursive Cooley-Tukey mixed-radix algorithm (decimation-in-time)
    static void fft(CVector& x, bool inverse_flag) {
        const int n = static_cast<int>(x.size());
        if (n == 1) return;

        int radix = pickRadix(n);
        int m = n / radix;              // length of each sub-sequence
        double sign = inverse_flag ? 1.0 : -1.0;

        // Split x into 'radix' sub-sequences of length m
        // Sub-sequence k: x[k], x[k + radix], x[k + 2*radix], ...
        std::vector<CVector> sub(radix, CVector(m));
        for (int k = 0; k < radix; ++k)
            for (int j = 0; j < m; ++j)
                sub[k][j] = x[j * radix + k];

        // Recursively compute the DFT of each sub-sequence
        for (int k = 0; k < radix; ++k)
            fft(sub[k], inverse_flag);

        // Combine results via radix-R butterfly with twiddle factors
        // X[j + r*m] = sum_{k=0}^{R-1} sub[k][j] * W_n^{k*j} * W_R^{k*r}
        const double angle = sign * 2.0 * PI / n;
        for (int j = 0; j < m; ++j) {
            // Pre-compute twiddle factors W_n^{k*j} for k = 0..R-1
            std::vector<Complex> tw(radix);
            for (int k = 0; k < radix; ++k)
                tw[k] = std::polar(1.0, angle * k * j);

            for (int r = 0; r < radix; ++r) {
                // W_R^r = exp(sign * 2*pi*i * r / radix)
                Complex wr    = std::polar(1.0, sign * 2.0 * PI * r / radix);
                Complex wrPow = {1.0, 0.0};  // accumulates wr^k

                Complex val = {0.0, 0.0};
                for (int k = 0; k < radix; ++k) {
                    val += tw[k] * wrPow * sub[k][j];
                    wrPow *= wr;
                }
                x[j + r * m] = val;
            }
        }
    }

    // Choose the largest prime factor of n from the set {5, 3, 2}
    static int pickRadix(int n) {
        for (int p : {5, 3, 2}) {
            if (n % p == 0) return p;
        }
        throw std::invalid_argument("Length is not of the form 2^a * 3^b * 5^c");
    }
};

// ============================================================
//  Helper functions
// ============================================================

// Generate n random complex numbers with components in [-1, 1]
CVector randomComplex(int n, unsigned seed = 42) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    CVector x(n);
    for (auto& v : x) v = {dist(rng), dist(rng)};
    return x;
}

// ============================================================
//  Write results for a single transform length N to a CSV file
//
//  Output file: fft_N.csv
//  Header row:  input_re,input_im,fft_re,fft_im,ifft_re,ifft_im
//  Each complex number is split into two columns (re and im),
//  giving 6 numeric columns total, read as three logical groups:
//    columns 1-2 = original input
//    columns 3-4 = forward FFT
//    columns 5-6 = inverse FFT
// ============================================================
void writeCSV(int n, const std::string& outDir = ".") {
    if (!FFT::isValidSize(n)) {
        std::cerr << "[SKIPPED] N=" << n
                  << " is not of the form 2^a * 3^b * 5^c\n";
        return;
    }

    // --- compute ---
    CVector original = randomComplex(n);

    CVector fftResult = original;
    FFT::forward(fftResult);

    CVector ifftResult = fftResult;
    FFT::inverse(ifftResult);

    // --- write file ---
    std::string filename = outDir + "/fft_" + std::to_string(n) + ".csv";
    std::ofstream f(filename);
    if (!f) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    // Header: 6 numeric columns = 3 complex-valued groups
    f << "input_real,input_imag,fft_real,fft_imag,ifft_real,ifft_imag\n";

    // setprecision(17) guarantees exact round-trip conversion of double to text
    f << std::fixed << std::setprecision(17);
    for (int i = 0; i < n; ++i) {
        f << original[i].real()   << "," << original[i].imag()   << ","
          << fftResult[i].real()  << "," << fftResult[i].imag()  << ","
          << ifftResult[i].real() << "," << ifftResult[i].imag() << "\n";
    }

    f.close();
    std::cout << "  Written: " << filename << "  (N=" << n << ")\n";
}

// ============================================================
//  main
// ============================================================
int main() {
    std::cout << "=== FFT -- writing results to CSV (mixed radix 2/3/5) ===\n\n";

    // Test lengths: various combinations of 2^a * 3^b * 5^c
    std::vector<int> sizes = {
        8,      // 2^3
        16,     // 2^4
        12,     // 2^2 * 3
        18,     // 2  * 3^2
        20,     // 2^2 * 5
        30,     // 2  * 3  * 5
        60,     // 2^2 * 3  * 5
        120,    // 2^3 * 3  * 5
        360,    // 2^3 * 3^2 * 5
        1800,   // 2^3 * 3^2 * 5^2
    };

    for (int n : sizes)
        writeCSV(n, ".");

    std::cout << "\n=== Done ===\n";
    return 0;
}
