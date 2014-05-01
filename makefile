#
# \emx\jm\vesa\makefile
#
GCC=gcc -Wall -O2 -mprobe
#GCC=gcc -g -Wall

.SUFFIXES: .c .o .obj .imp .exe .dll .a .lib

.c.o:
	$(GCC) -o $*.o -c $*.c

.o.exe:
	$(GCC) -o $*.exe $*.o vesa.a
	emxbind -aq $*.exe -acimw

.c.obj:
	$(GCC) -Zdll -Zomf -Zsys -o $*.obj -c $*.c

.obj.dll:
	$(GCC) -Zdll -Zomf -Zsys -o $*.dll $*.obj common.obj $*.def -lemxio

.a.lib:
	emxomf -l -o $*.lib $*.a

default: vesa_ibm.dll vesa_et4.dll vesa_et5.dll vesa_et6.dll vesa_tr8.dll \
         vesa_s3.dll vesa_cl.dll vesa_clh.dll vesa_wd.dll vesa_all.dll    \
         vesa_pm.dll vesa.a vesa.lib                                      \
         vesatest.exe moustest.exe modedump.exe pmscreen.exe

install: default
	copy *.dll \emx\dll
	copy vesa.h \emx\include
	copy vesa.a \emx\lib
	copy vesa.lib \emx\lib
	copy pmscreen.exe \emx\bin

install2: default
	cp *.dll /emx/dll
	cp vesa.h /emx/include
	cp vesa.a /emx/lib
	cp vesa.lib /emx/lib
	cp pmscreen.exe /emx/bin

dos: vesa.a vesa.lib vesatest.exe moustest.exe

dosinstall: dos
	copy vesa.h \emx\include
	copy vesa.a \emx\lib
	copy vesa.lib \emx\lib

dosinstall2: dos
	cp vesa.h /emx/include
	cp vesa.a /emx/lib
	cp vesa.lib /emx/lib

clean:
	-del *.o *.obj *.a *.lib *.exe *.dll *.res core

vesa.lib: vesa.a

vesa.a: vesa_dos.o vesa.o
	if exist vesa.a del vesa.a
	ar r vesa.a vesa_dos.o vesa.o
	ar s vesa.a

vesa_dos.o: vesa_dos.c vesa.h vesa_dos.h
vesa.o:     vesa.c     vesa.h vesa_dos.h

common.obj:   common.c   vesa.h vgaports.h common.h vesadll.h
vesa_et4.obj: vesa_et4.c vesa.h vgaports.h common.h vesadll.h
vesa_et4.dll: vesa_et4.obj common.obj vesa_et4.def
vesa_et5.obj: vesa_et5.c vesa.h vgaports.h common.h vesadll.h
vesa_et5.dll: vesa_et5.obj common.obj vesa_et5.def
vesa_et6.obj: vesa_et6.c vesa.h vgaports.h common.h vesadll.h
vesa_et6.dll: vesa_et5.obj common.obj vesa_et6.def
vesa_tr8.obj: vesa_tr8.c vesa.h vgaports.h common.h vesadll.h
vesa_tr8.dll: vesa_tr8.obj common.obj vesa_tr8.def
vesa_ibm.obj: vesa_ibm.c vesa.h vgaports.h common.h vesadll.h
vesa_ibm.dll: vesa_ibm.obj common.obj vesa_ibm.def
vesa_s3.obj:  vesa_s3.c vesa.h vgaports.h common.h vesadll.h
vesa_s3.dll:  vesa_s3.obj common.obj vesa_s3.def
vesa_wd.obj:  vesa_wd.c vesa.h vgaports.h common.h vesadll.h
vesa_wd.dll:  vesa_wd.obj common.obj vesa_wd.def
vesa_cl.obj:  vesa_cl.c vesa.h vgaports.h common.h vesadll.h
vesa_cl.dll:  vesa_cl.obj common.obj vesa_cl.def
vesa_clh.obj: vesa_cl.c vesa.h vgaports.h common.h vesadll.h    \
              cirrus/3.c cirrus/110.c cirrus/111.c cirrus/112.c \
              cirrus/113.c cirrus/114.c
	$(GCC) -Zdll -Zomf -Zsys -o $*.obj -c vesa_cl.c -DHICOLOR
vesa_clh.dll: vesa_clh.obj common.obj vesa_clh.def
vesa_all.obj:  vesa_all.c vesa.h vgaports.h common.h vesadll.h
vesa_all.dll:  vesa_all.obj common.obj vesa_all.def
vesa_pm.obj:  vesa_pm.c  vesa.h common.h vesa_pm.h  vesadll.h
vesa_pm.dll:  vesa_pm.obj vesa_pm.def
	$(GCC) -Zdll -Zomf -Zsys -o $*.dll $*.obj $*.def

vesatest.exe: vesatest.o vesa.a
vesatest.o:   vesatest.c vesa.h

moustest.exe: moustest.o vesa.a
moustest.o:   moustest.c vesa.h

modedump.exe: modedump.o vesa.a
modedump.o:   modedump.c vesa.h

pmscreen.exe: pmscreen.o pmscreen.res
	$(GCC) -o $*.exe $*.o pmscreen.res pmscreen.def
pmscreen.o:   pmscreen.c  pmscreen.h vesa_pm.h
pmscreen.res: pmscreen.rc pmscreen.dlg pmscreen.h
	echo If this step fails you have to "SET INCLUDE=X:\EMX\INCLUDE"!
	rc -r pmscreen
