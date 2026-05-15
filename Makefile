# =============================================================
#  Makefile -- CMake wrapper for convenient compilation
#
#  Targets:
#    make              -- build both binaries (Release)
#    make debug        -- build with debug flags
#    make run          -- build and run fft + error_rate
#    make run_fft      -- build and run fft only (CSV generation)
#    make run_errors   -- build and run error_rate on all CSVs
#    make clean        -- remove build/
#    make rebuild      -- clean + build
#    make help         -- print this help message
# =============================================================

BUILD_DIR  := build
CMAKE      := cmake
BUILD_TYPE := Release

# List of CSV files produced by fft (used by run_errors)
CSVS := $(addprefix $(BUILD_DIR)/, \
    fft_8.csv fft_16.csv fft_12.csv fft_18.csv fft_20.csv \
    fft_30.csv fft_60.csv fft_120.csv fft_360.csv fft_1800.csv)

.PHONY: all debug run run_fft run_errors clean rebuild help

# ── Default target: Release build ────────────────────────────
all: $(BUILD_DIR)/Makefile
	@$(CMAKE) --build $(BUILD_DIR) --config $(BUILD_TYPE) -j$$(nproc)
	@echo ""
	@echo "Build complete. Binaries:"
	@echo "  $(BUILD_DIR)/fft"
	@echo "  $(BUILD_DIR)/error_rate"

# ── Debug build ───────────────────────────────────────────────
debug: BUILD_TYPE := Debug
debug:
	@mkdir -p $(BUILD_DIR)
	@$(CMAKE) -S . -B $(BUILD_DIR) \
	    -DCMAKE_BUILD_TYPE=Debug \
	    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@$(CMAKE) --build $(BUILD_DIR) --config Debug -j$$(nproc)
	@echo "Debug build complete."

# ── CMake configuration (runs once, re-runs if CMakeLists.txt changes)
$(BUILD_DIR)/Makefile:
	@mkdir -p $(BUILD_DIR)
	@$(CMAKE) -S . -B $(BUILD_DIR) \
	    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# ── Run: fft (CSV generation) then error_rate ─────────────────
run: all run_fft run_errors

# ── Run fft only (CSV generation) ────────────────────────────
run_fft: all
	@echo ""
	@echo "=== Running fft (CSV generation) ==="
	@cd $(BUILD_DIR) && ./fft

# ── Run error_rate on all CSV files ──────────────────────────
run_errors: all
	@echo ""
	@echo "=== Running error_rate ==="
	@cd $(BUILD_DIR) && ./error_rate \
	    fft_8.csv fft_16.csv fft_12.csv fft_18.csv fft_20.csv \
	    fft_30.csv fft_60.csv fft_120.csv fft_360.csv fft_1800.csv

# ── Remove build directory ────────────────────────────────────
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Directory $(BUILD_DIR)/ removed."

# ── Full rebuild ──────────────────────────────────────────────
rebuild: clean all

# ── Help ──────────────────────────────────────────────────────
help:
	@echo ""
	@echo "Available targets:"
	@echo "  make              -- Release build (both binaries)"
	@echo "  make debug        -- Debug build"
	@echo "  make run          -- build + fft + error_rate"
	@echo "  make run_fft      -- build + fft only"
	@echo "  make run_errors   -- build + error_rate only"
	@echo "  make clean        -- remove build/"
	@echo "  make rebuild      -- clean + build"
	@echo "  make help         -- this help message"
	@echo ""
	@echo "Run error_rate manually on specific CSV files:"
	@echo "  ./build/error_rate build/fft_8.csv build/fft_360.csv"
	@echo ""
