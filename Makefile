################################################################################
# Capture Makefile
################################################################################

################################################################################
# Key paths and settings
################################################################################

SRCDIR = src
BUILDDIR = build
TARGET = bin/speed_test

################################################################################
# Dependencies
################################################################################

# Spinnaker deps
SPINNAKER_LIB = -l Spinnaker
SPINNAKER_INC = -isystem /usr/include/spinnaker # suppress spinnaker SDK warnings with `-isystem` include

################################################################################
# Master inc/lib/obj/dep settings
################################################################################

CFLAGS = -std=c++11 -Wall
CC = g++

SRCEXT = cpp
SOURCES = $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
INC = ${SPINNAKER_INC} -I lib/
LIB = -Wl,-Bdynamic -pthread ${SPINNAKER_LIB}

################################################################################
# Rules/recipes
################################################################################

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Clean up intermediate objects
clean_obj:
	rm -f ${OBJECTS}
	@echo "intermediate objects cleaned up!"

# Clean up everything.
clean: clean_obj
	rm -f ${TARGET}
	@echo "all cleaned up!"
