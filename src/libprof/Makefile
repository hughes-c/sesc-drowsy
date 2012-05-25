#the following line has to be before the include
SIGNATURE=libprof
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default : all

all: $(OBJ)/libprof.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= workloadCharacteristics.o				\
	   stat_profile.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libprof.a
	-@rm -f $(OBJ)/libstat.a

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
