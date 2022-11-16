PATH_SRC  = ./src
PATH_INC  = $(PATH_SRC)/include
PATH_OBJ  = ./obj
PATH_BIN  = ./bin
PATH_FIFO = ./fifo

CXX      =  g++
CXXFLAGS += -g
CPPFLAGS += -std=c++17 -I$(PATH_INC)

OUT_MAIN = GenreCounter.out

# ---------------------------------------------------------------------------

vpath %.cpp $(PATH_SRC)
vpath %.hpp $(PATH_INC)

OBJS_MAIN = main.o log.o

# ---------------------------------------------------------------------------

all: $(PATH_OBJ) $(PATH_BIN) $(PATH_FIFO) $(PATH_BIN)/$(OUT_MAIN) copy_main

$(PATH_BIN)/$(OUT_MAIN): $(addprefix $(PATH_OBJ)/, $(OBJS_MAIN))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(PATH_OBJ)/log.o: log.cpp log.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(PATH_OBJ)/main.o: main.cpp log.hpp defs.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# ---------------------------------------------------------------------------

$(PATH_OBJ):  ; mkdir -p $@
$(PATH_BIN):  ; mkdir -p $@
$(PATH_FIFO): ; mkdir -p $@

.PHONY: all clean copy_main run

copy_main:
	cp $(PATH_BIN)/$(OUT_MAIN) .

run:
	./$(OUT_MAIN)

clean:
	rm -rf $(PATH_OBJ) $(PATH_BIN) $(PATH_FIFO) $(OUT_MAIN) > /dev/null 2>&1
