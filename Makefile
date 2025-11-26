CXX = g++

TARGET = EgyptainDriving

SRC_DIR = ./src
BUILD_DIR = build
BIN_DIR = bin
INCLUDE_DIR = include
LIB_DIR = lib

SOURCES = $(wildcard $(SRC_DIR)/**/*.cpp $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

CXXFLAGS = -Wall -Wextra -std=c++11 -I$(INCLUDE_DIR)


ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    RM = del /Q
    MKDIR = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
    RMDIR = if exist $(subst /,\,$(1)) rmdir /S /Q $(subst /,\,$(1))
    COPY = copy /Y $(subst /,\,$(1)) $(subst /,\,$(2))
else
    RM = rm -f
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)
    COPY = cp $(1) $(2)
endif

ifeq ($(WORKFLOWS),lib/x64)
	LIB_DIR = lib/x64
	@echo Using 64-bit libraries
endif

LDFLAGS = -L$(LIB_DIR) -lfreeglut -lopengl32 -lglu32


.PHONY: all
all: directories $(BIN_DIR)/$(TARGET)

.PHONY: directories
directories:
	@$(call MKDIR,$(BUILD_DIR))
	@$(call MKDIR,$(BIN_DIR))

$(BIN_DIR)/$(TARGET): $(OBJECTS)
	@echo Linking $(TARGET)...
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@if not exist $(BIN_DIR)\freeglut.dll $(call COPY,bin\freeglut.dll,$(BIN_DIR)\freeglut.dll)
	@echo Build complete: $(BIN_DIR)/$(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo Compiling $<...
	@$(call MKDIR,$(dir $@))
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run
run: all
	@echo Running $(TARGET)...
	@cd $(BIN_DIR) && $(TARGET)

.PHONY: clean
clean:
	@echo Cleaning build files...
	@$(call RMDIR,$(BUILD_DIR))
	@$(RM) $(BIN_DIR)\$(TARGET) 2>nul || exit 0
	@echo Clean complete.

.PHONY: distclean
distclean: clean
	@echo Cleaning all generated files...
	@$(call RMDIR,$(BIN_DIR))
	@echo All clean.

.PHONY: help
help:
	@echo Available targets:
	@echo   all       - Build the project (default)
	@echo   run       - Build and run the project
	@echo   clean     - Remove build files
	@echo   distclean - Remove all generated files
	@echo   help      - Show this help message