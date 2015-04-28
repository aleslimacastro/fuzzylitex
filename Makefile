.PHONY: examples test

all: examples test

test:
	cd test && $(MAKE)

examples:
	cd examples && $(MAKE)

clean:
	cd test && $(MAKE) clean
	cd examples && $(MAKE) clean
