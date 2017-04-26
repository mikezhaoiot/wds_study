CC = arm-linux-gcc 
CFLAGS = -Wall
LIBS = -lasound
all:capture

capture: capture.o wav_parser.o
	$(CC) $(CFLAGS) capture.o wav_parser.o -o capture $(LIBS)

wav_parser.o: wav_parser.c wav_parser.h
	$(CC) $(CFLAGS) -c wav_parser.c


clean:
	rm capture *.o
