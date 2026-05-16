CC      ?= clang
BUILD   := build

BOLD    := \033[1m
GREEN   := \033[32m
YELLOW  := \033[33m
RED     := \033[31m
CYAN    := \033[36m
NC      := \033[0m

.PHONY: all help example aligned tests valgrind test-stds clean

all: $(BUILD)/Makefile
	cmake --build $(BUILD)
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

$(BUILD)/Makefile:
	cmake -B $(BUILD) -S . -DCMAKE_C_COMPILER=$(CC)

help: ## Show this help
	@printf '$(BOLD)Usage:$(NC) make [target]\n'
	@echo
	@printf '$(BOLD)Build targets:$(NC)\n'
	@echo '  all          Build all examples and tests'
	@echo
	@printf '$(BOLD)Run targets:$(NC)\n'
	@echo '  example      Build and run the basic example'
	@echo '  aligned      Build and run the aligned allocation example'
	@echo '  tests        Build and run the unit tests'
	@echo
	@printf '$(BOLD)Check targets:$(NC)\n'
	@echo '  valgrind     Run Valgrind leak check on all executables'
	@echo '  test-stds    Build example_aligned under c99, c11, c17, c23'
	@echo
	@printf '$(BOLD)Utility targets:$(NC)\n'
	@echo '  clean        Remove build directory and compile_commands.json'
	@echo '  help         Show this help'
	@echo
	@printf '$(BOLD)Variables:$(NC)\n'
	@echo '  CC           C compiler (default: clang)'

example: all ## Build and run the basic example
	./$(BUILD)/example

aligned: all ## Build and run the aligned allocation example
	./$(BUILD)/example_aligned

test: all ## Build and run the unit tests
	./$(BUILD)/tests

valgrind: all ## Run Valgrind leak check on all executables
	@printf '$(CYAN)Valgrind HEAP SUMMARY for example:$(NC)\n'
	@valgrind -s --leak-check=full ./$(BUILD)/example 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
	@echo
	@printf '$(CYAN)Valgrind HEAP SUMMARY for example_aligned:$(NC)\n'
	@valgrind -s --leak-check=full ./$(BUILD)/example_aligned 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
	@echo
	@printf '$(CYAN)Valgrind HEAP SUMMARY for tests:$(NC)\n'
	@valgrind -s --leak-check=full ./$(BUILD)/tests 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"

test-stds: ## Build example_aligned under c99, c11, c17, c23
	@mkdir -p $(BUILD)
	@for std in c99 c11 c17 c23; do \
		printf '$(BOLD)Testing with -std=%s$(NC)\n' "$$std"; \
		$(CC) -std=$$std -ggdb -O0 -Wall -Wextra -Werror -Wpedantic \
			-I./include ./src/example_aligned.c -o $(BUILD)/example_aligned -lm; \
		if [ $$? -eq 0 ]; then \
			./$(BUILD)/example_aligned; \
			printf '$(GREEN)Build and run successful for -std=%s$(NC)\n' "$$std"; \
		else \
			if [ "$$std" = "c99" ]; then \
				printf '\n$(YELLOW)WARNING: Build failed for -std=c99 (expected behavior)$(NC)\n'; \
				printf '$(YELLOW)Nothing to worry about -- _Alignas/_Alignof are C11+ features.$(NC)\n\n'; \
			else \
				printf '$(RED)ERROR: Build failed for -std=%s$(NC)\n' "$$std"; \
			fi; \
		fi; \
		echo "--------------------------------------------"; \
		echo; \
	done

clean: ## Remove build directory and compile_commands.json
	rm -rf $(BUILD)
	rm -f compile_commands.json
