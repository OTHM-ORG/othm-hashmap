SRC_FILES = \
	src/othm_hashmap.c \
	src/MurmurHash2.c

OBJ_PATH = bin
OBJ_FILES = $(patsubst %,$(OBJ_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

LIBRARY_FILE = libothm_hashmap.a
HEADER_FILE = othm_hashmap.h

all: $(LIBRARY_FILE)

$(LIBRARY_FILE): $(OBJ_FILES)
	ar -cvq libothm_hashmap.a $(OBJ_FILES)

define make-obj
$(patsubst %.c, $(OBJ_PATH)/%.o, $(notdir $(1))): $(1)
	gcc -c $$< -o $$@
endef

$(foreach src,$(SRC_FILES),$(eval $(call make-obj,$(src))))

.PHONY : clean test install
clean :
	-rm -f $(LIBRARY_FILE) $(OBJ_FILES) test

test :
	gcc -static -o test test.c -lothm_hashmap
	./test
install :
	mv $(LIBRARY_FILE) /usr/lib/
	cp src/$(HEADER_FILE) /usr/include/
uninstall :
	rm -f /usr/lib/$(HEADER_FILE)
	rm -f /usr/include/$(HEADER_FILE)
