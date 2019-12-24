

.PHONY: build
build:
	po photon build

.PHONY: clean
clean:
	po photon clean

.PHONY: flash
flash:
	po photon flash

.PHONY: serial
serial:
	po serial monitor
