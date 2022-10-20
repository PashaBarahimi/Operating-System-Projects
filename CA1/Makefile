CC     = gcc
CFLAGS = -g

PATH_SRC = ./src
PATH_INC = $(PATH_SRC)/include
PATH_OBJ = ./obj
PATH_LOG = ./log

OUT_BUYER  = buyer
OUT_SELLER = seller

# -------------------------------------------------------------------------

vpath %.c $(PATH_SRC)
vpath %.h $(PATH_INC)

OBJS_BUYER  = buyer.o log.o
OBJS_SELLER = seller.o log.o

# -------------------------------------------------------------------------

all: $(PATH_OBJ) $(PATH_LOG) $(OUT_BUYER) #$(OUT_SELLER)

$(OUT_BUYER): $(addprefix $(PATH_OBJ)/, $(OBJS_BUYER))
	$(CC) $(CFLAGS) -o $@ $^

$(OUT_SELLER): $(addprefix $(PATH_OBJ)/, $(OBJS_SELLER))
	$(CC) $(CFLAGS) -o $@ $^

$(PATH_OBJ)/buyer.o: buyer.c log.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(PATH_OBJ)/seller.o: seller.c log.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(PATH_OBJ)/log.o: log.c log.h
	$(CC) $(CFLAGS) -c -o $@ $<

# -------------------------------------------------------------------------

$(PATH_OBJ): ; mkdir -p $@
$(PATH_LOG): ; mkdir -p $@

.PHONY: all clean

clean:
	rm -rf $(PATH_OBJ) $(PATH_LOG) $(OUT_BUYER) $(OUT_SELLER) > /dev/null 2>&1
