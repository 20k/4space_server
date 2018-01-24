#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I../deps
CFLAGS = -Wextra -Wall -fexceptions -std=gnu++1z -Wno-sign-compare -Wno-narrowing -DNET_SERVER
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = -lmingw32 -lsfml-graphics -lsfml-audio -lsfml-network -lsfml-window -lsfml-system -lfreetype -ljpeg -lopengl32 -lglew64.dll -lws2_32 -lwinmm -lgdi32 -lflac -lopenal32 -logg

INC_DEBUG = $(INC)
CFLAGS_DEBUG = $(CFLAGS) -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = obj/Debug
DEP_DEBUG = 
OUT_DEBUG = bin/Debug/game_server

INC_RELEASE = $(INC)
CFLAGS_RELEASE = $(CFLAGS) -O3
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = obj/Release
DEP_RELEASE = 
OUT_RELEASE = bin/Release/game_server

OBJ_DEBUG = $(OBJDIR_DEBUG)/__/deps/serialise/serialise.o $(OBJDIR_DEBUG)/game_state.o $(OBJDIR_DEBUG)/main.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/__/deps/serialise/serialise.o $(OBJDIR_RELEASE)/game_state.o $(OBJDIR_RELEASE)/main.o

all: before_build build_debug build_release after_build

clean: clean_debug clean_release

before_build: 
	./update_submodules.bat

after_build: 

before_debug: 
	test -d bin/Debug || mkdir -p bin/Debug
	test -d $(OBJDIR_DEBUG)/__/deps/serialise || mkdir -p $(OBJDIR_DEBUG)/__/deps/serialise
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

build_debug: before_debug out_debug after_debug

debug: before_build build_debug after_build

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LIBDIR_DEBUG) -o $(OUT_DEBUG) $(OBJ_DEBUG)  $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/__/deps/serialise/serialise.o: ../deps/serialise/serialise.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c ../deps/serialise/serialise.cpp -o $(OBJDIR_DEBUG)/__/deps/serialise/serialise.o

$(OBJDIR_DEBUG)/game_state.o: game_state.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c game_state.cpp -o $(OBJDIR_DEBUG)/game_state.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf bin/Debug
	rm -rf $(OBJDIR_DEBUG)/__/deps/serialise
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d bin/Release || mkdir -p bin/Release
	test -d $(OBJDIR_RELEASE)/__/deps/serialise || mkdir -p $(OBJDIR_RELEASE)/__/deps/serialise
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

build_release: before_release out_release after_release

release: before_build build_release after_build

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LIBDIR_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE)  $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/__/deps/serialise/serialise.o: ../deps/serialise/serialise.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c ../deps/serialise/serialise.cpp -o $(OBJDIR_RELEASE)/__/deps/serialise/serialise.o

$(OBJDIR_RELEASE)/game_state.o: game_state.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c game_state.cpp -o $(OBJDIR_RELEASE)/game_state.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf bin/Release
	rm -rf $(OBJDIR_RELEASE)/__/deps/serialise
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_build after_build before_debug after_debug clean_debug before_release after_release clean_release

