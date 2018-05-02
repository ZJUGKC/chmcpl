#
# Copyright(C) 2018 Xin YUAN <yxxinyuan@zju.edu.cn>
#
# This file is part of chmcpl.
#
# chmcpl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# chmcpl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with chmcpl.  If not, see <http://www.gnu.org/licenses/>.
#

OBJS := $(BIN_DIR)/chmc.o \
		$(BIN_DIR)/err.o $(BIN_DIR)/utils.o \
		$(BIN_DIR)/main.o
CWARNFLAGS = -Wall -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline
CWARNFLAGS += -Wno-uninitialized 
COPTFLAGS = -O2
CINCFLAGS = -I ../lzx_compress -I ../
CFLAGS = $(COPTFLAGS) $(CWARNFLAGS) $(CINCFLAGS)
LIB_FLAGS = -static
LIBS = -lconfig -L $(LZX_A_DIR) $(LIB_FLAGS) -llzxcomp -lm

all : $(BIN_DIR)/chmcpl

$(BIN_DIR)/chmcpl : $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

$(BIN_DIR)/chmc.o : chmc.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(BIN_DIR)/err.o : err.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(BIN_DIR)/utils.o : utils.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(BIN_DIR)/main.o : main.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN_DIR)/*.o $(BIN_DIR)/chmcpl
