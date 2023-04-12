VPATH = src
CXXFLAGS := $(CXXFLAGS) -std=c++17 -g -I$(VPATH) -Ilib/kissfft/ -I/usr/include/python3.10/ -fpic -Wall #-O3

src = $(wildcard src/*.cpp)
obj = $(src:.cpp=.o)

profile: src/profile.o fukah_module.so
	$(CXX) -o profile src/profile.o fukah_module.so -Llib/kissfft/ -lkissfft-float -lboost_python310 -lpython3.10

fukah_module.so: $(obj) kissfft
	$(CXX) -shared -o $@ $(obj) -Llib/kissfft/ -lkissfft-float -lboost_python310 -lpython3.10

.PHONY: clean
clean:
	rm -f $(obj) fukah_module.so profile

.PHONY: kissfft
kissfft:
	cd lib/kissfft; make
