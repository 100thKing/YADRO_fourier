#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <complex>
#include <cmath>
#include <string>
#include <iomanip>
#include <algorithm>

using Complex = std::complex<double>;

// ============================================================
//  Result structure for a single CSV file analysis
// ============================================================
struct ErrorStats {
    std::string filename;
    int         n          = 0;      // number of data rows (N)
    double      maxAbsErr  = 0.0;    // maximum absolute error
    double      meanAbsErr = 0.0;    // mean absolute error (MAE)
    double      rmsErr     = 0.0;    // root mean square error (RMSE)
    double      maxRelErr  = 0.0;    // maximum relative error
    std::string errorMsg;            // parse error message, if any
};

// ============================================================
//  Read and analyse a single CSV file
//
//  Expected format (no spaces around commas):
//    input_re,input_im,fft_re,fft_im,ifft_re,ifft_im
//    <number>,<number>,<number>,<number>,<number>,<number>
//    ...
//
//  Error is computed between columns (input_re, input_im)
//  and columns (ifft_re, ifft_im), i.e. between the original
//  input and the result after forward + inverse FFT.
// ============================================================
ErrorStats analyzeCSV(const std::string& filepath) {
    ErrorStats stats;
    stats.filename = filepath;

    std::ifstream f(filepath);
    if (!f) {
        stats.errorMsg = "Failed to open file: " + filepath;
        return stats;
    }

    std::string line;

    // Skip the header row
    if (!std::getline(f, line)) {
        stats.errorMsg = "File is empty or missing a header.";
        return stats;
    }

    // Validate the header
    const std::string expectedHeader = "input_real,input_imag,fft_real,fft_imag,ifft_real,ifft_imag";
    if (line != expectedHeader) {
        stats.errorMsg = "Unexpected header: \"" + line + "\"\n"
                       + "  Expected:           \"" + expectedHeader + "\"";
        return stats;
    }

    // Read data rows
    double sumAbsErr = 0.0;
    double sumSqErr  = 0.0;
    int    rowIndex  = 0;

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        // Split the row by comma
        std::vector<double> cols;
        std::istringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            try {
                cols.push_back(std::stod(token));
            } catch (...) {
                stats.errorMsg = "Failed to parse number at row "
                               + std::to_string(rowIndex + 2)
                               + ": \"" + token + "\"";
                return stats;
            }
        }

        if (cols.size() != 6) {
            stats.errorMsg = "Row " + std::to_string(rowIndex + 2)
                           + " has " + std::to_string(cols.size())
                           + " columns (expected 6).";
            return stats;
        }

        Complex input  (cols[0], cols[1]);  // columns 1-2: original input
        Complex ifftVal(cols[4], cols[5]);  // columns 5-6: inverse FFT output
        // cols[2], cols[3] -- forward FFT (not used for error calculation)

        double absErr   = std::abs(input - ifftVal);
        double inputMod = std::abs(input);

        sumAbsErr += absErr;
        sumSqErr  += absErr * absErr;

        if (absErr > stats.maxAbsErr) stats.maxAbsErr = absErr;

        // Relative error: skip rows where the input magnitude is near zero
        if (inputMod > 1e-15) {
            double relErr = absErr / inputMod;
            if (relErr > stats.maxRelErr) stats.maxRelErr = relErr;
        }

        ++rowIndex;
    }

    if (rowIndex == 0) {
        stats.errorMsg = "File contains no data rows (header only).";
        return stats;
    }

    stats.n          = rowIndex;
    stats.meanAbsErr = sumAbsErr / rowIndex;
    stats.rmsErr     = std::sqrt(sumSqErr / rowIndex);

    return stats;
}

// ============================================================
//  Print the error statistics for a single file
// ============================================================
void printStats(const ErrorStats& s) {
    std::cout << "\n----------------------------------------\n";
    std::cout << " File: " << s.filename << "\n";
    std::cout << "----------------------------------------\n";

    if (!s.errorMsg.empty()) {
        std::cout << " [ERROR] " << s.errorMsg << "\n";
        return;
    }

    std::cout << " Data rows (N):                 " << s.n << "\n";
    std::cout << std::scientific << std::setprecision(6);
    std::cout << " Max absolute error:            " << s.maxAbsErr   << "\n";
    std::cout << " Mean absolute error (MAE):     " << s.meanAbsErr  << "\n";
    std::cout << " Root mean square error (RMSE): " << s.rmsErr      << "\n";
    std::cout << " Max relative error:            " << s.maxRelErr   << "\n";
}

// ============================================================
//  main: accepts CSV file paths as command-line arguments
//
//  Compilation:
//    g++ -O2 -std=c++17 -o error_rate error_rate.cpp
//
//  Single file:
//    ./error_rate fft_8.csv
//
//  Multiple files:
//    ./error_rate fft_8.csv fft_16.csv fft_360.csv
// ============================================================
int main(int argc, char* argv[]) {
    std::cout << "=== FFT error analysis (input vs IFFT(FFT(input))) ===\n";

    if (argc < 2) {
        std::cerr << "\nUsage: " << argv[0]
                  << " <file1.csv> [<file2.csv> ...]\n\n"
                  << "Example:\n"
                  << "  " << argv[0] << " fft_8.csv fft_30.csv fft_1800.csv\n";
        return 1;
    }

    int processed = 0;
    for (int i = 1; i < argc; ++i) {
        ErrorStats stats = analyzeCSV(argv[i]);
        printStats(stats);
        ++processed;
    }

    std::cout << "\n========================================\n";
    std::cout << " Files processed: " << processed << "\n";
    std::cout << "========================================\n";

    return 0;
}
