# This makefile currently only works for mac os
# You should install via homebrew:
#    brew install glew
#    brew install glfw3
#

CXXFLAGS=-framework OpenGL -framework Cocoa -framework IOKit 
CXXFLAGS+=-lglew -lglfw3
CXXFLAGS+= -D__APPLE__

main: main.cpp gui.cpp imgui.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm main
