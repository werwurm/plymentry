TARGET:= plymentry

SRC:= src/main.cpp

OBJ:= $(patsubst %.cpp,%.o,$(SRC))

CXXFLAGS:= -g -O0 -std=c++0x

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFAGS) -o $@ $^

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)