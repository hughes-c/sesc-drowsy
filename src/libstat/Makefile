#the following line has to be before the include
SIGNATURE=libstat
include	../Makefile.defs
##############################################################################
.PHONY: all clean

default : all

all: $(OBJ)/libstat.a

##############################################################################
#                Objects
##############################################################################
OBJS	:= InstructionContainer.o BasicBlock.o FlowNode.o Synthetic.o	\
	   InstructionMix.o						\
	   memoryOperations.o graphManipulation.o codeGenerator.o	\
	   printers.o							\
	   stat_synthesis.o

##############################################################################
#                             Change Rules                                   # 
##############################################################################
# eclean is called by clean
eclean:
	-@rm -f $(OBJ)/libstat.a
	-@rm -f $(OBJ)/libprof.a

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
