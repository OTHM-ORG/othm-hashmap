SRC_FILES = \
	src/othm_hashmap.c \
	src/MurmurHash2.c

OBJ_PATH = bin
OBJ_FILES = $(patsubst %,$(OBJ_PATH)/%,$(notdir $(SRC_FILES:.c=.o)))

LIBRARY_FILE = libothm_hashmap.so
HEADER_FILE = othm_hashmap.h

all: $(LIBRARY_FILE)

$(LIBRARY_FILE): $(OBJ_FILES) src/$(HEADER_FILE)
	gcc -shared -o $(LIBRARY_FILE) $(OBJ_FILES)

define make-obj
$(patsubst %.c, $(OBJ_PATH)/%.o, $(notdir $(1))): $(1)
	gcc -c -Wall -Werror -fPIC $$< -o $$@
endef

$(foreach src,$(SRC_FILES),$(eval $(call make-obj,$(src))))

.PHONY : clean test install
clean :
	-rm  $(LIBRARY_FILE) $(OBJ_FILES) test
test :
	gcc -g -Wall -o test test.c -lothm_hashmap -lothm_base
	./test
install :
	cp $(LIBRARY_FILE) /usr/lib/
	cp src/$(HEADER_FILE) /usr/include/
	ldconfig
uninstall :
	-rm  /usr/lib/$(LIBRARY_FILE)
	-rm  /usr/include/$(HEADER_FILE)
	ldconfig
