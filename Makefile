TARGET:= plymentry

SRC:= src/main.cpp src/Plymouth.cpp

OBJ:= $(patsubst %.cpp,%.o,$(SRC))

LDFLAGS:= $(shell pkg-config ply-boot-client --libs)
CXXFLAGS:= -g -O0 -std=c++0x $(shell pkg-config ply-boot-client --cflags)

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)