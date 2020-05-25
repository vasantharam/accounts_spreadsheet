#
# Constantin Knizhnik
# Moscow Software Center
# BGI for Windows
#

DEFINES = 
CC = bcc32 +bcc32.cfg

# Debug
#CCFLAGS = -I. -Od -c -v $(DEFINES)

# Optimize
CCFLAGS = -I. -4 -N -P -d -O2 -c -v $(DEFINES)

LD = bcc32 -v +bcc32.cfg
LDFLAGS =  -L"C:\Borland\Bcc55\lib"

all: generic.exe

generic.obj: *.cpp
	$(CC) $(CCFLAGS) *.cpp 

generic.exe: generic.obj winbgi.lib
	$(LD) $(LDFLAGS) *.obj winbgi.lib

clean:
	delAll

