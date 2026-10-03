OUT := game
CC := g++

CXXFLAGS := -Wall -std=c++17 -O2
SOURCE := $(wildcard *.cpp */*.cpp)
OBJ := $(patsubst %.cpp, %.o, $(notdir $(SOURCE)))
RM_OBJ := 
RM_OUT := 

ifeq ($(OS), Windows_NT) # Windows OS
	ALLEGRO_PATH := ../allegro
	export Path := ../MinGW/bin;$(Path)

	ALLEGRO_FLAGS_RELEASE := -I$(ALLEGRO_PATH)/include -L$(ALLEGRO_PATH)/lib/liballegro_monolith.dll.a
	ALLEGRO_DLL_PATH_RELEASE := $(ALLEGRO_PATH)/lib/liballegro_monolith.dll.a
	ALLEGRO_FLAGS_DEBUG := -I$(ALLEGRO_PATH)/include -L$(ALLEGRO_PATH)/lib/liballegro_monolith-debug.dll.a
	ALLEGRO_DLL_PATH_DEBUG := $(ALLEGRO_PATH)/lib/liballegro_monolith-debug.dll.a

	RM_OBJ := $(foreach name, $(OBJ), del $(name) & )
	ifeq ($(suffix $(OUT)),)
		RM_OUT := del $(OUT).exe
	else
		RM_OUT := del $(OUT)
	endif
else # Mac OS / Linux
	UNAME_S := $(shell uname -s)
	ALLEGRO_LIB_PATH ?= /usr/local/lib
	ALLEGRO_PKGCONFIG_PATH ?= /usr/local/lib/pkgconfig
	ALLEGRO_INCLUDE_PATH ?= /usr/local/include

	export LD_LIBRARY_PATH := $(ALLEGRO_LIB_PATH):$(LD_LIBRARY_PATH)
	export DYLD_LIBRARY_PATH := $(ALLEGRO_LIB_PATH):$(DYLD_LIBRARY_PATH)
	export PKG_CONFIG_PATH := $(ALLEGRO_PKGCONFIG_PATH):$(PKG_CONFIG_PATH)

	ALLEGRO_LIBRARIES := allegro-5 allegro_image-5 allegro_font-5 allegro_ttf-5 allegro_dialog-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5
	ALLEGRO_FLAGS_RELEASE := $(shell pkg-config --cflags --libs "$(ALLEGRO_LIBRARIES) <= 5.2.11") -lallegro
	ALLEGRO_DLL_PATH_RELEASE := 
	ALLEGRO_FLAGS_DEBUG := $(ALLEGRO_FLAGS_RELEASE)
	ALLEGRO_DLL_PATH_DEBUG := 

	RM_OBJ := rm $(OBJ)
	RM_OUT := rm $(OUT)
endif

debug:
	$(CC) -c -g $(CXXFLAGS) $(SOURCE) $(ALLEGRO_FLAGS_DEBUG) -D DEBUG
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(ALLEGRO_FLAGS_DEBUG) $(ALLEGRO_DLL_PATH_DEBUG)
	$(RM_OBJ)

release:
	$(CC) -c $(CXXFLAGS) $(SOURCE) $(ALLEGRO_FLAGS_RELEASE)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(ALLEGRO_FLAGS_RELEASE) $(ALLEGRO_DLL_PATH_RELEASE)
	$(RM_OBJ)

clean:
	$(RM_OUT)
