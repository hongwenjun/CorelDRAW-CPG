CC = cl.exe
LINK = link.exe

CFLAGS = /nologo /W3 /EHsc  /Ox /DNDEBUG  /I..\TypeLibs
LDFLAGS = /nologo /DLL  Gdi32.lib user32.lib Kernel32.lib

SOURCE = WelcomeScreen.cpp
RC_FILE = WelcomeScreen.rc
RESOURCES = WelcomeScreen.res

OBJECT = $(SOURCE:.cpp=.obj)

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