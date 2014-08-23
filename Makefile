# This makefile currently only works for mac os
# You should install via homebrew:
#    brew install glew
#    brew install glfw3
#

CXX=clang++
CXXFLAGS=-framework OpenGL -framework Cocoa -framework IOKit 
CXXFLAGS+=-lglew -lglfw3
CXXFLAGS+= -D__APPLE__

IMGUI_CPP_FILES=imgui.impl.opengl.cpp imgui.custom.cpp

main: main.cpp $(IMGUI_CPP_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm main

