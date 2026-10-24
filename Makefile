CC      ?= clang
BUILD   := build


# Colors
BOLD   := \033[1m
RESET  := \033[0m
GREEN  := \033[32m
RED    := \033[31m
CYAN   := \033[36m
YELLOW := \033[33m


.DEFAULT_GOAL := help

## help: Show available commands
.PHONY: help
help:
	@printf "\n$(BOLD)Available commands:$(RESET)\n"
	@for cat in Build Run Check; do \
		printf "\n$(YELLOW)$(BOLD)%s$(RESET)\n" "$$cat"; \
		grep -E "^## [^@:]+@$$cat: " Makefile | \
			sed -E 's/^## ([^@:]+)@[^:]+:[[:space:]]*(.*)/\1\t\2/' | \
			awk -F'\t' '{printf "  $(CYAN)%-18s$(RESET) %s\n", $$1, $$2}'; \
	done
	@printf "\n"

## all@Build: build all examples and tests
.PHONY: all
all: $(BUILD)/Makefile
	cmake --build $(BUILD)
	ln -sf $(BUILD)/compile_commands.json compile_commands.json

$(BUILD)/Makefile:
	cmake -B $(BUILD) -S . -DCMAKE_C_COMPILER=$(CC)

## format@Build: clang-format all project files
.PHONY: format
format:
	@find include src tests \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 -P "$$(nproc)" -n 8 clang-format -i --style=file

## clean@Build: remove the build directory and compile_commands.json
.PHONY: clean
clean:
	rm -rf $(BUILD)
	rm -f compile_commands.json

## example@Run: build and run the basic example
.PHONY: example
example: all
	./$(BUILD)/example

## aligned@Run: build and run the aligned allocation example
.PHONY: aligned
aligned: all
	./$(BUILD)/example_aligned

## test@Run: build and run the unit tests
.PHONY: test
test: all
	./$(BUILD)/tests

## valgrind@Check: run Valgrind leak check on all executables
.PHONY: valgrind
valgrind: all
	@printf '$(CYAN)Valgrind HEAP SUMMARY for example:$(RESET)\n'
	@valgrind -s --leak-check=full ./$(BUILD)/example 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
	@echo
	@printf '$(CYAN)Valgrind HEAP SUMMARY for example_aligned:$(RESET)\n'
	@valgrind -s --leak-check=full ./$(BUILD)/example_aligned 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
	@echo
	@printf '$(CYAN)Valgrind HEAP SUMMARY for tests:$(RESET)\n'
	@valgrind -s --leak-check=full ./$(BUILD)/tests 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"

## test-stds@Check: build example_aligned under c99, c11, c17, c23
.PHONY: test-stds
test-stds:
	@mkdir -p $(BUILD)
	@for std in c99 c11 c17 c23; do \
		printf '$(BOLD)Testing with -std=%s$(RESET)\n' "$$std"; \
		stdflag="$$std"; \
		if [ "$$std" = "c23" ]; then \
			$(CC) -std=c23 -x c /dev/null -o /dev/null 2>/dev/null || stdflag="c2x"; \
		fi; \
		$(CC) -std=$$stdflag -ggdb -O0 -Wall -Wextra -Werror -Wpedantic \
			-I./include ./src/example_aligned.c -o $(BUILD)/example_aligned -lm; \
		if [ $$? -eq 0 ]; then \
			./$(BUILD)/example_aligned; \
			printf '$(GREEN)Build and run successful for -std=%s$(RESET)\n' "$$std"; \
		else \
			if [ "$$std" = "c99" ]; then \
				printf '\n$(YELLOW)WARNING: Build failed for -std=c99 (expected behavior)$(RESET)\n'; \
				printf '$(YELLOW)Nothing to worry about -- _Alignas/_Alignof are C11+ features.$(RESET)\n\n'; \
			else \
				printf '$(RED)ERROR: Build failed for -std=%s$(RESET)\n' "$$std"; \
			fi; \
		fi; \
		echo "--------------------------------------------"; \
		echo; \
	done

## asan@Check: unit tests under AddressSanitizer + UBSan
.PHONY: asan
asan:
	@mkdir -p $(BUILD)
	$(CC) -std=c11 -g -O1 -Wall -Wextra -Werror -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer \
		-I./include ./tests/test_va_arena.c -o $(BUILD)/tests_asan -lm
	./$(BUILD)/tests_asan
