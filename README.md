<!--

  Copyright(C) 2018 Xin YUAN <yxxinyuan@zju.edu.cn>

  This file is part of chmcpl.

  chmcpl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  chmcpl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with chmcpl.  If not, see <http://www.gnu.org/licenses/>.

-->

# chmcpl

This project is a command-line chm compiler under *nix systems
and is based on [Alex's work](https://sourceforge.net/projects/chmc/ "chmc").

This project is not tested on Big-Endian machines.

# Building

Install the package `libconfig`:

```
sudo apt-get install libconfig-dev
```

Enter the project directory and execute:

```
make
```

The executable file `chmcpl` is in the `bin` directory.

# Usage

Use the following command to generate chm file:

```
chmcpl -c XXX.conf -o XXX.chm <source directory>
```

# Languages

`chmcpl` supports the following languages:

| ID   | Description             | Short String | Charset     |
|:----:|:------------------------|:------------:|:------------|
| 1033 | English (United States) | en-US        | US-ASCII    |
| 1025 | Arabic                  | ar-SA        | Arabic      |
| 2052 | Chinese (Simplified)    | zh-CN        | GB2312      |
| 1028 | Chinese (Traditional)   | zh-TW        | Big5        |
| 1029 | Czech                   | cs-CZ        | ISO-8859-2  |
| 1032 | Greek                   | el-GR        | Greek       |
| 1037 | Hebrew                  | he-IL        | Hebrew      |
| 1038 | Hungarian               | hu-HU        | ISO-8859-2  |
| 1041 | Japanese                | ja-JP        | Shift-JIS   |
| 1042 | Korean                  | ko-KR        | ISO-2022-KR |
| 1045 | Polish                  | pl-PL        | ISO-8859-2  |
| 1049 | Russian                 | ru-RU        | ISO-8859-5  |
| 1051 | Slovakian               | sk-SK        | ISO-8859-2  |
| 1060 | Slovenian               | sl-SI        | ISO-8859-2  |
| 1055 | Turkish                 | tr-TR        | ISO-8859-9  |
| 1026 | Bulgarian               | bg-BG        | ISO-8859-5  |
