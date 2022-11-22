PATH_SRC  = ./src
PATH_INC  = $(PATH_SRC)/include
PATH_OBJ  = ./obj
PATH_BIN  = ./bin
PATH_FIFO = ./.fifo

CXX      =  g++
CXXFLAGS +=
CPPFLAGS += -std=c++17 -I$(PATH_INC)

OUT_MAIN = GenreCounter.out
OUT_MAP  = map_worker
OUT_RED  = red_worker

ALL_OUT  = $(OUT_MAIN) $(OUT_MAP) $(OUT_RED)

# ----------------------------------------------------------------------------------------

vpath %.cpp $(PATH_SRC)
vpath %.hpp $(PATH_INC)

OBJS_MAIN = main.o log.o utils.o
OBJS_MAP  = map_worker.o log.o utils.o
OBJS_RED  = red_worker.o log.o utils.o

# ----------------------------------------------------------------------------------------

all: $(PATH_OBJ) $(PATH_BIN) $(PATH_FIFO) $(addprefix $(PATH_BIN)/, $(ALL_OUT)) copy_main

$(PATH_BIN)/$(OUT_MAIN): $(addprefix $(PATH_OBJ)/, $(OBJS_MAIN))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(PATH_BIN)/$(OUT_MAP): $(addprefix $(PATH_OBJ)/, $(OBJS_MAP))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(PATH_BIN)/$(OUT_RED): $(addprefix $(PATH_OBJ)/, $(OBJS_RED))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

$(PATH_OBJ)/main.o: main.cpp log.hpp defs.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(PATH_OBJ)/map_worker.o: map_worker.cpp log.hpp defs.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(PATH_OBJ)/red_worker.o: reduce_worker.cpp log.hpp defs.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(PATH_OBJ)/log.o: log.cpp log.hpp colors.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(PATH_OBJ)/utils.o: utils.cpp defs.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------------------------

$(PATH_OBJ):  ; mkdir -p $@
$(PATH_BIN):  ; mkdir -p $@
$(PATH_FIFO): ; mkdir -p $@

.PHONY: all clean copy_main run

copy_main:
	cp $(PATH_BIN)/$(OUT_MAIN) .

run:
	./$(OUT_MAIN) $(ARGS)

clean:
	rm -rf $(PATH_OBJ) $(PATH_BIN) $(PATH_FIFO) $(OUT_MAIN) > /dev/null 2>&1
