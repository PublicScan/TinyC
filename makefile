.PHONY: all build rebuild rel_build rel_rebuild clean update_pch build_actions clean_actions
all: build

include makefile_var

build_type = debug

CXX = g++
#CXX = g++-4.2
CXXFLAGS += -MMD -m32
CXXFLAGS += $(foreach i,$(macro_defs),-D $(i))
CXXFLAGS += $(foreach i,$(include_dirs),-I '$(i)')
CXXFLAGS += $(foreach i,$(lib_dirs),-L '$(i)')
CXXFLAGS += $(foreach i,$(lib_files),-l '$(i)')
ifeq ($(use_cpp0x),1)
CXXFLAGS += -std=c++0x	
endif
ifeq ($(build_dll),1)
CXXFLAGS += -shared -fPIC
endif
ifeq ($(build_type),debug)
CXXFLAGS += -g
else
CXXFLAGS += -O3 -DNDEBUG
endif

srcs = $(wildcard *.cpp)
objs = $(srcs:.cpp=.o)
all_deps = $(srcs:.cpp=.d)
exist_deps = $(wildcard *.d)
not_exist_deps = $(filter-out $(exist_deps), $(all_deps))
pch_file =pch.h

build: build_actions update_pch main
rebuild: clean
	$(MAKE) build
rel_build: 
	$(MAKE) build -e build_type=release
rel_rebuild: clean
	$(MAKE) rel_build
clean: clean_actions
	rm -f main $(objs) $(all_deps) $(pch_file).gch
update_pch: $(pch_file).gch

main: $(objs)
	$(CXX)  -o $@ $(objs) $(CXXFLAGS) -ldl

$(pch_file).gch: $(pch_file)
	$(CXX) $(filter-out -MMD,$(CXXFLAGS)) $<

ifneq ($(exist_deps),)
include $(exist_deps)
endif
ifneq ($(not_exist_deps),)
$(not_exist_deps:.d=.o):%.o:%.cpp
endif
