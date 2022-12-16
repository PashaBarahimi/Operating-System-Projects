PATH_SRC  = ./src
PATH_INC  = $(PATH_SRC)/include
PATH_OBJ  = ./obj

CXX      =  g++
CXXFLAGS += -g
CPPFLAGS += -std=c++11 -I$(PATH_INC)

OUTPUT = ImageFilters.out

# ----------------------------------------------------------------------------------------

vpath %.cpp $(PATH_SRC)
vpath %.hpp $(PATH_INC)

OBJS = main.o bmp24.o

# ----------------------------------------------------------------------------------------

all: $(PATH_OBJ) $(OUTPUT)

$(OUTPUT): $(addprefix $(PATH_OBJ)/, $(OBJS))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(PATH_OBJ)/main.o: main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(PATH_OBJ)/bmp24.o: bmp24.cpp bmp24.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------------------------

$(PATH_OBJ):  ; mkdir -p $@

.PHONY: all clean run

run:
	./$(OUT_MAIN) $(ARGS)

clean:
	rm -rf $(PATH_OBJ) $(OUTPUT) > /dev/null 2>&1