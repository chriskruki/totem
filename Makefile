# FastLED ESP32 Custom Driver Makefile
# Convenient commands for building and running the project

# Default target
.DEFAULT_GOAL := help

# Variables
PROJECT_NAME = FastLED ESP32 Custom Driver
PORT = COM3
BAUD_RATE = 115200

# Colors for output
GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
NC = \033[0m # No Color

## Help target - shows available commands
help:
	@echo "$(GREEN)$(PROJECT_NAME) - Available Commands$(NC)"
	@echo "================================================"
	@echo ""
	@echo "$(YELLOW)Build Commands:$(NC)"
	@echo "  build          - Compile the project"
	@echo "  clean          - Clean build artifacts"
	@echo "  check          - Check code without building"
	@echo ""
	@echo "$(YELLOW)Upload Commands:$(NC)"
	@echo "  upload         - Build and upload to ESP32"
	@echo "  run            - Build, upload, and start serial monitor"
	@echo ""
	@echo "$(YELLOW)Monitor Commands:$(NC)"
	@echo "  monitor        - Start serial monitor"
	@echo "  debug          - Upload and monitor with debug info"
	@echo ""
	@echo "$(YELLOW)Utility Commands:$(NC)"
	@echo "  devices        - List connected devices"
	@echo "  info           - Show project information"
	@echo "  deps           - Show project dependencies"
	@echo "  size           - Show memory usage"
	@echo ""
	@echo "$(YELLOW)Development Commands:$(NC)"
	@echo "  format         - Format source code"
	@echo "  lint           - Run code linter"
	@echo "  test           - Run tests (if available)"
	@echo ""

## Build the project
build:
	@echo "$(GREEN)Building $(PROJECT_NAME)...$(NC)"
	pio run

## Clean build artifacts
clean:
	@echo "$(YELLOW)Cleaning build artifacts...$(NC)"
	pio run --target clean

## Check code without building
check:
	@echo "$(YELLOW)Checking code...$(NC)"
	pio check

## Build and upload to ESP32
upload: build
	@echo "$(GREEN)Uploading to ESP32 on $(PORT)...$(NC)"
	pio run --target upload

## Build, upload, and start serial monitor (main run command)
run: upload
	@echo "$(GREEN)Starting serial monitor on $(PORT) at $(BAUD_RATE) baud...$(NC)"
	@echo "$(YELLOW)Commands: help, red, green, blue, white, clear, brightness X, info$(NC)"
	@echo "$(YELLOW)Press Ctrl+C to exit monitor$(NC)"
	@sleep 2
	pio device monitor

## Start serial monitor only
monitor:
	@echo "$(GREEN)Starting serial monitor on $(PORT) at $(BAUD_RATE) baud...$(NC)"
	@echo "$(YELLOW)Commands: help, red, green, blue, white, clear, brightness X, info$(NC)"
	@echo "$(YELLOW)Press Ctrl+C to exit monitor$(NC)"
	pio device monitor

## Upload with debug configuration and monitor
debug:
	@echo "$(GREEN)Building in debug mode and uploading...$(NC)"
	pio run --environment esp32dev --target upload
	@echo "$(GREEN)Starting debug monitor...$(NC)"
	pio device monitor --filter esp32_exception_decoder

## List connected devices
devices:
	@echo "$(GREEN)Connected devices:$(NC)"
	pio device list

## Show project information
info:
	@echo "$(GREEN)Project Information:$(NC)"
	@echo "===================="
	@echo "Project Name: $(PROJECT_NAME)"
	@echo "Platform: ESP32"
	@echo "Framework: Arduino"
	@echo "Board: esp32dev"
	@echo "Port: $(PORT)"
	@echo "Baud Rate: $(BAUD_RATE)"
	@echo ""
	@echo "$(GREEN)Configuration:$(NC)"
	@cat src/config.h | grep -E "^#define" | head -10

## Show project dependencies
deps:
	@echo "$(GREEN)Project Dependencies:$(NC)"
	@echo "====================="
	pio pkg list

## Show memory usage
size: build
	@echo "$(GREEN)Memory Usage:$(NC)"
	@echo "=============="
	pio run --target size

## Format source code (if clang-format is available)
format:
	@echo "$(YELLOW)Formatting source code...$(NC)"
	@if command -v clang-format >/dev/null 2>&1; then \
		find src/ -name "*.cpp" -o -name "*.h" | xargs clang-format -i; \
		echo "$(GREEN)Code formatted successfully$(NC)"; \
	else \
		echo "$(RED)clang-format not found. Install it for code formatting.$(NC)"; \
	fi

## Run code linter
lint:
	@echo "$(YELLOW)Running code linter...$(NC)"
	pio check --fail-on-defect medium

## Run tests (placeholder for future implementation)
test:
	@echo "$(YELLOW)Running tests...$(NC)"
	@echo "$(RED)No tests configured yet. Add test cases in test/ directory.$(NC)"

## Force rebuild everything
rebuild: clean build

## Quick development cycle - build and upload without monitor
quick: build upload
	@echo "$(GREEN)Quick upload completed!$(NC)"

## Show ESP32 chip info (requires esptool)
chipinfo:
	@echo "$(GREEN)ESP32 Chip Information:$(NC)"
	@if command -v esptool.py >/dev/null 2>&1; then \
		esptool.py --port $(PORT) chip_id; \
	else \
		echo "$(RED)esptool.py not found in PATH$(NC)"; \
	fi

## Erase flash completely
erase:
	@echo "$(RED)Erasing ESP32 flash memory...$(NC)"
	@read -p "Are you sure? This will erase everything! (y/N): " confirm && [ "$$confirm" = "y" ]
	pio run --target erase

## Install/update project dependencies
install:
	@echo "$(GREEN)Installing/updating dependencies...$(NC)"
	pio pkg install

## Show PlatformIO version and system info
version:
	@echo "$(GREEN)PlatformIO System Information:$(NC)"
	pio system info

# Phony targets (not actual files)
.PHONY: help build clean check upload run monitor debug devices info deps size format lint test rebuild quick chipinfo erase install version
