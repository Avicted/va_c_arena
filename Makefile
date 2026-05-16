CC      ?= clang
BUILD   := build

.PHONY: all help example aligned tests valgrind test-stds clean

all: $(BUILD)/Makefile
	cmake --build $(BUILD)
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

$(BUILD)/Makefile:
	cmake -B $(BUILD) -S . -DCMAKE_C_COMPILER=$(CC)

help: ## Show this help
	@echo 'Usage: make [target]'
	@echo
	@echo 'Build targets:'
	@echo '  all          Build all examples and tests'
	@echo
	@echo 'Run targets:'
	@echo '  example      Build and run the basic example'
	@echo '  aligned      Build and run the aligned allocation example'
	@echo '  tests        Build and run the unit tests'
	@echo
	@echo 'Check targets:'
	@echo '  valgrind     Run Valgrind leak check on all executables'
	@echo '  test-stds    Build example_aligned under c99, c11, c17, c23'
	@echo
	@echo 'Utility targets:'
	@echo '  clean        Remove build directory and compile_commands.json'
	@echo '  help         Show this help'
	@echo
	@echo 'Variables:'
	@echo '  CC           C compiler (default: clang)'

example: all ## Build and run the basic example
	./$(BUILD)/example

aligned: all ## Build and run the aligned allocation example
	./$(BUILD)/example_aligned

tests: all ## Build and run the unit tests
	./$(BUILD)/tests

valgrind: all ## Run Valgrind leak check on all executables
	@echo "Valgrind HEAP SUMMARY for example:"
	@valgrind -s --leak-check=full ./$(BUILD)/example 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
	@echo
	@echo "Valgrind HEAP SUMMARY for example_aligned:"
	@valgrind -s --leak-check=full ./$(BUILD)/example_aligned 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
	@echo
	@echo "Valgrind HEAP SUMMARY for tests:"
	@valgrind -s --leak-check=full ./$(BUILD)/tests 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"

test-stds: ## Build example_aligned under c99, c11, c17, c23
	@for std in c99 c11 c17 c23; do \
		echo "Testing with -std=$$std"; \
		$(CC) -std=$$std -ggdb -O0 -Wall -Wextra -Werror -Wpedantic \
			-I./include ./src/example_aligned.c -o $(BUILD)/example_aligned -lm; \
		if [ $$? -eq 0 ]; then \
			./$(BUILD)/example_aligned; \
			echo "Build and run successful for -std=$$std"; \
		else \
			if [ "$$std" = "c99" ]; then \
				echo "WARNING: Build failed for -std=$$std (expected behavior)"; \
			else \
				echo "ERROR: Build failed for -std=$$std"; \
			fi; \
		fi; \
		echo "--------------------------------------------"; \
		echo; \
	done

clean: ## Remove build directory and compile_commands.json
	rm -rf $(BUILD)
	rm -f compile_commands.json
