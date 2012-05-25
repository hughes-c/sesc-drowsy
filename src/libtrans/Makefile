#the following line has to be before the include
SIGNATURE=libtrans
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default : all

all: $(OBJ)/libtrans.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= transCache.o transContext.o transCoherence.o transReport.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libtrans.a

size:
	-@wc *.cpp *.h

# DO NOT DELETE
_MAKEFILE_COMMON_SUBSHELL=1
include $(SRC_DIR)/Makefile.common

ifneq ($(MAKECMDGOALS),distclean)
ifneq ($(MAKECMDGOALS),clean)
-include $(DOTDEPEND)
endif
endif
