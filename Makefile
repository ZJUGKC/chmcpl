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

BIN_DIR   = $(PWD)/bin
LZX_A_DIR = $(PWD)/src/lzx_compress/obj
SUB_DIR = src/lzx_compress \
		src/cpl

export BIN_DIR LZX_A_DIR

all : $(SUB_DIR)

$(SUB_DIR) : ECHO
	make -C $@

ECHO :
	@echo $(SUB_DIR)
	@echo begin compiling...
