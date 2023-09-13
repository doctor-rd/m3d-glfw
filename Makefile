EXE = m3d
CC = g++
CFLAGS = -O2 -Iglm
LDFLAGS = -lglfw -lGL

$(EXE):	main.cpp
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -f $(EXE)
