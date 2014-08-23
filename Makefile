# This makefile currently only works for mac os
# You should install via homebrew:
#    brew install glew
#    brew install glfw3
#

CXX=clang++
CXXFLAGS=-framework OpenGL -framework Cocoa -framework IOKit 
CXXFLAGS+=-lglew -lglfw3
CXXFLAGS+= -D__APPLE__

main: main.cpp gui.cpp imgui.custom.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm main
