# Makefile 适合 MSVC2022 namke
# 编译器和链接器
CC = cl.exe
LINK = link.exe

# 编译和链接选项
CFLAGS = /nologo /W3 /EHsc  /Ox /DNDEBUG  /I..\TypeLibs
LDFLAGS = /nologo /DLL  gdi32.lib user32.lib Kernel32.lib 

# 源文件和资源文件
SOURCE = ToolsBox.cpp cdrapp.cpp cdrapi.cpp cdrPDF2Clip.cpp vbadll.cpp boxAutoGroup.cpp BoundaryGroup.cpp WinData.cpp
RC_FILE = ToolsBox.rc
RESOURCES = ToolsBox.res

# 目标文件
OBJECT = $(SOURCE:.cpp=.obj)

# 最终输出
TARGET = lycpg64.dll

all: $(TARGET)

$(TARGET): $(OBJECT) $(RESOURCES)
    $(LINK) $(LDFLAGS) /out:$@ $(OBJECT) $(RESOURCES)

$(RESOURCES): $(RC_FILE)
    rc.exe /fo$@ $(RC_FILE)

.cpp.obj:
    $(CC) $(CFLAGS) /c $<

clean:
    del *.obj *.res *.dll *.orig VGCoreAuto.tlh VGCoreAuto.tli lycpg64.lib lycpg64.exp

install:
	copy  /y  lycpg64.dll  "C:\Program Files\Corel\CorelDRAW Graphics Suite 2020\Draw\Plugins64\lycpg64.cpg"

.PHONY: all clean