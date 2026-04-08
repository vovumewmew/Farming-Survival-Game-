EXEC = main
CXX = g++

SDL_ROOT = C:/SDL3-3.4.4/x86_64-w64-mingw32

CXXFLAGS = -std=c++17 -Wall -Wextra -I"$(SDL_ROOT)/include" -Ivendor -Isrc
LDFLAGS = -L"$(SDL_ROOT)/lib"
LDLIBS = -lSDL3 -lSDL3_image

SOURCES = $(wildcard src/core/*.cpp src/systems/*.cpp src/utils/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-@cmd /C "del /Q /F $(subst /,\,$(OBJECTS)) $(EXEC) $(EXEC).exe 2>nul || exit /B 0"

.PHONY: all clean
