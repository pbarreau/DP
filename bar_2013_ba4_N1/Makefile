#########################################################################
#
# MODULE:   bar_2013_ba4
#
# DESCRIPTION: bar_2013_ba4_N1 MakeFile
#
############################################################################
# 
#
#############################################################################
# Subversion variables
# $HeadURL:  $
# $Revision: $
# $LastChangedBy:  $
# $LastChangedDate:  $
# $Id:  $ 
#
#############################################################################

DIRS	= out

TARGETS: clean all

all: 
	-for d in $(DIRS); do (cd $$d/out; $(MAKE) $(MFLAGS) all ); done
	-for d in $(DIRS); do (cd $$d/out; $(MAKE) -f Makefile_JN5139.mk $(MFLAGS) all ); done


clean:
	find . -type f \( -name "*.[do]" -o -name "*.bak" -o -name "*.bin" -o -name "*.elf" -o -name "*.map" -o -name "*~" -o -name "*.?#?" \) -exec rm -f {} \;