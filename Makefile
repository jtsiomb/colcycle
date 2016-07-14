obj = main.obj app.obj image.obj# timer.obj keyb.obj vbe.obj gfx.obj
bin = ccimg.exe

opt = -5 -fp5 -otexan
dbg = -d1

CC = wcc386
CFLAGS = $(dbg) $(opt) -zq -bt=dos -Isrc
LD = wlink

$(bin): $(obj)
	%write objects.lnk file { $(obj) }
	$(LD) debug all name $@ @objects $(LDFLAGS)

.c: src
.c: src\dos

.c.obj: .autodepend
	$(CC) $(CFLAGS) $[*

clean: .symbolic
	del *.obj
	del $(bin)
