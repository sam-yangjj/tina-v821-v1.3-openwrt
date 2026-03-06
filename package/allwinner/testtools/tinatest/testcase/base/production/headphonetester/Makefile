CONFIG_FILE := $(PREFIX_DATA)/../../.config
AUDIO_FILE_FULL := $(shell sed -n 's/^CONFIG_TINATEST_BASE_PRODUCTION_HEADPHONETESTER_AUDIO_FILE=//p' $(CONFIG_FILE) | tr -d '"')
AUDIO_FILE_NAME := $(notdir $(AUDIO_FILE_FULL))

.PHONY: all clean

all:
	cp ./headphonetester.sh $(PREFIX)
	if [ -f ./$(AUDIO_FILE_NAME) ]; then \
		cp ./$(AUDIO_FILE_NAME) $(PREFIX_DATA); \
	fi

clean:
