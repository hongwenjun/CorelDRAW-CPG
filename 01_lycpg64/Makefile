# Makefile 适合 MSVC2022 namke
# 编译器和链接器
CC = cl.exe
LINK = link.exe

# 编译和链接选项
CFLAGS = /nologo /W3 /EHsc  /Ox /DNDEBUG  /I..\TypeLibs
LDFLAGS = /nologo /DLL  Gdi32.lib user32.lib Kernel32.lib

# 源文件和资源文件
SOURCE = main.cpp


# 目标文件
OBJECT = $(SOURCE:.cpp=.obj)

# 最终输出
TARGET = lycpg64.dll

all: $(TARGET)

$(TARGET): $(OBJECT) $(RESOURCES)
    $(LINK) $(LDFLAGS) /out:$@ $(OBJECT) $(RESOURCES)


.cpp.obj:
    $(CC) $(CFLAGS) /c $<

clean:
    del *.obj *.res *.dll VGCoreAuto.tlh VGCoreAuto.tli lycpg64.lib lycpg64.exp

install:
	copy  /y  lycpg64.dll  "C:\Program Files\Corel\CorelDRAW Graphics Suite 2020\Draw\Plugins64\lycpg64.cpg"

.PHONY: all clean