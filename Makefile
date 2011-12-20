# GNU Make solution makefile autogenerated by Premake
# Type "make help" for usage help

ifndef config
  config=debug64
endif
export config

PROJECTS := bufferStreaming

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

bufferStreaming: 
	@echo "==== Building bufferStreaming ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f bufferStreaming.make

clean:
	@${MAKE} --no-print-directory -C . -f bufferStreaming.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debug64"
	@echo "   release64"
	@echo "   debug32"
	@echo "   release32"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   bufferStreaming"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
