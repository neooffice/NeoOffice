all: build.all

build.idl: XGrammarChecker.idl
	idlc -C -I ../build/udkapi XGrammarChecker.idl
	regmerge grammarcheck.uno.rdb /UCR XGrammarChecker.urd
	cppumaker -BUCR -Torg.neooffice.XGrammarChecker ../build/offapi/type_reference/types.rdb grammarcheck.uno.rdb
	touch "$@"

build.dylib: build.idl grammarcheck.mm
	gcc -g -dynamiclib -fPIC -o grammarcheck.uno.dylib -I../build/offuh/unxmacxp.pro/inc/offuh -I../build/cppuhelper/inc -I../sal/inc -I../sal/unxmacxp.pro/inc -DUNX -DSAL_UNX -DCPPU_ENV=gcc3 -I../build/cppu/inc -I. -I../build/solenv/inc -L../build/solver/680/unxmacxp.pro/lib -luno_cppuhelpergcc3 -luno_sal -luno_salhelpergcc3 -lstlport_gcc -luno_cppu -lstdc++ -framework AppKit -framework Foundation -framework Carbon grammarcheck.mm
	touch "$@"
	
build.oxt: build.dylib
	zip grammarcheck.oxt META-INF/manifest.xml grammarcheck.uno.dylib grammarcheck.uno.rdb uiIntegration.xcu GrammarGUI/*
	touch "$@"

build.all: build.oxt
	touch "$@"
