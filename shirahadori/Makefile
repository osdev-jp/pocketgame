GWPROG := C:\Gowin\Gowin_V1.9.12.01_x64\Programmer\bin\programmer_cli.exe
GWSH := C:\Gowin\Gowin_V1.9.12.01_x64\IDE\bin\gw_sh.exe
GW_DEV := GW1NR-9C

TARGET := impl/pnr/project.fs

SRCS := src/port.cst \
	src/top.sv

all: $(TARGET)

$(TARGET): $(SRCS) Makefile
	@$(shell wslpath -u "$(GWSH)") < syn-pnr.tcl 2>&1 | { \
		err=0; \
		while IFS= read -r line; do \
		    /usr/bin/echo "$$line"; \
		    /usr/bin/echo "$$line" | grep -q "^ERROR" && err=1; \
		done; \
		[ $$err -eq 0 ] || exit 1; \
	}
