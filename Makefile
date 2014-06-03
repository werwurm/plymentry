TARGET:= plymentry

SRC:= src/main.cpp src/Plymouth.cpp

OBJ:= $(patsubst %.cpp,%.o,$(SRC))
DEPS := $(patsubst %.o,%.dpp,$(OBJ))

LDFLAGS:= $(shell pkg-config ply-boot-client --libs)
CXXFLAGS:= -O3 -std=c++0x $(shell pkg-config ply-boot-client --cflags)

STRIP:=strip

.PHONY: all

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(TARGET).unstripped
	$(STRIP) -o $@ $<

$(TARGET).unstripped: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ)
	rm -f $(TARGET) $(TARGET).unstripped
	rm -f $(DEPS)

%.dpp: %.cpp
	$(CXX) $(CXXFLAGS) -MT $*.o -MM -MF $@ $<
