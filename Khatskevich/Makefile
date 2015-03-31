PROGPATH=.
INCLUDES=$(PROGPATH)/include
TOOLSPATH=$(PROGPATH)/tools

all: archivator run

archivator: $(INCLUDES)/log.h $(PROGPATH)/log/logProc.c archivator.c $(PROGPATH)/core/data_presentation_controller.c
	$(CC) -o archivator $(PROGPATH)/log/logProc.c archivator.c $(PROGPATH)/core/data_presentation_controller.c $(TOOLSPATH)/zlib/libz.a -pthread -I$(INCLUDES) -g
run: archivator
	./archivator archivator
clean:
