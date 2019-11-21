#
# File:
#    Makefile-rol
#
# Description:
#    Makefile for the coda primary and secondary readout list
#    running on the LINUXVME controller
#
# SVN: $Rev$
#
# Uncomment DEBUG line for debugging info ( -g and -Wall )
#export DEBUG=1
#

F1TDCROOT   ?=  ${HOME}/git/f1tdc
FADCROOT    ?=  ${HOME}/git/fadc/v2.0
HCALROOT    ?=  ../hcal_pulser

# Plug in your primary readout lists here..
#VMEROL			= f1tdc_list.so fadc_list_sd.so adc_tdc_list.so
VMEROL			= fadc_list_sd.so
# Add shared library dependencies here.  (jvme, tir are already included)
ROLLIBS			= -lf1tdc -lremex -lfadc -lsd -lts -lhcal

LINUXVME_LIB	?= ${CODA}/extensions/linuxvme/libs
LINUXVME_INC	?= ${CODA}/extensions/linuxvme/include

# DEFs for compiling primary readout lists
CC			= gcc
AR                      = ar
ifdef DEBUG
CFLAGS			= -Wall -g
else
CFLAGS			= -O3
endif

RANLIB                  = ranlib

CFLAGS			+= -w -DLINUX -DDAYTIME=\""`date`"\"

#INCS			= -I. -I${F1TDCROOT} -I${FADCROOT} -I${HCALROOT} -I${LINUXVME_INC}  -I/usr/include -I${CODA}/common/include
INCS			= -I. -I${F1TDCROOT} -I${HCALROOT} -I${LINUXVME_INC}  -I/usr/include -I${CODA}/common/include
#LIBS			= -L. -L${F1TDCROOT} -L${FADCROOT} -L${HCALROOT} -L${LINUXVME_LIB} -DJLAB
LIBS			= -L. -L${F1TDCROOT} -L${HCALROOT} -L${LINUXVME_LIB} -DJLAB\
				-lrt -lpthread -ljvme -lti $(ROLLIBS)

# DEFs for compiling CODA readout lists
CCRL			= ${CODA_BIN}/ccrl
CODA_INCS		= -I.  -I${LINUXVME_INC} -I${CODA}/common/include
CODA_LIBDIRS            = -L. -L${LINUXVME_LIB}
CODA_LIBS		= -ljvme -lti 
CODA_DEFS		= -DLINUX -DDAYTIME=\""`date`"\"
CODA_CFLAGS		= -O -w -fpic -shared ${CODA_INCS} ${CODA_LIBDIRS} \
			  ${CODA_LIBS} ${CODA_DEFS}
ifdef DEBUG
CODA_CFLAGS		+= -Wall -g
endif
CRLFILES		= $(wildcard *.crl)
CFILES			= $(CRLFILES:.crl=.c)
SOBJS			= $(CRLFILES:.crl=.so)
DEPS			= $(VMEROL:%.so=%.d)
DEPS			+= $(CFILES:%.c=%.d)


all:  $(VMEROL) $(SOBJS)

event_list.c: event_list.crl Makefile
	@echo
	@echo "Converting $< to $@"
	@${CCRL} $<

event_list.so: event_list.c
	@echo
	@echo "Building $@ from $<"
	@echo
	${CC} ${CODA_CFLAGS} -o $@ $<

%.so: %.c
	@echo
	@echo "Building $@ from $<"
	@echo
	$(CC) -fpic -shared  $(CFLAGS) $(INCS) $(LIBS) \
		-DINIT_NAME=$(@:.so=__init) -o $@ $<

clean distclean:
	@rm -f  $(VMEROL) $(SOBJS) $(CFILES) *~ $(DEPS)

%.d: %.c
	@echo "Building $@ from $<"
	@set -e; rm -f $@; \
	$(CC) -MM -shared $(INCS) $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.so $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS)

.PHONY: all
