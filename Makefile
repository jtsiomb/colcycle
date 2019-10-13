obj = main.obj app.obj image.obj imagelbm.obj timer.obj keyb.obj vbe.obj dpmi.obj gfx.obj logger.obj
bin = colcycle.exe

def = -dLENDIAN
opt = -5 -fp5 -otexan
dbg = -d1

!ifdef __UNIX__
incpath = -Isrc -Isrc/dos
RM = rm -f
!else
incpath = -Isrc -Isrc\dos
RM = del
!endif

CC = wcc386
CFLAGS = $(dbg) $(opt) $(def) -zq -bt=dos $(incpath)
LD = wlink

$(bin): $(obj)
	%write objects.lnk $(obj)
	$(LD) debug all name $@ system dos4g file { @objects } $(LDFLAGS)

.c: src;src/dos

.c.obj: .autodepend
	$(CC) -fo=$@ $(CFLAGS) $[*

clean: .symbolic
	$(RM) *.obj
	$(RM) $(bin)
