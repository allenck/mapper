
SOURCES += $$(QTDIR)/../Src/qtbase/src/3rdparty/sqlite/sqlite3.c
HEADERS += $$(QTDIR)/../Src/qtbase/src/3rdparty/sqlite/sqlite3.h

win32:QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote( $$(QTDIR)\\..\\Src\\qtbase\\src\\3rdparty\\sqlite\\sqlite3.h) $$shell_path($$shell_quote($$PWD)) $$escape_expand(\\n\\t)
else:unix:QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote( $$(QTDIR)/../Src/qtbase/src/3rdparty/sqlite/sqlite3.h) $$shell_quote($$PWD) $$escape_expand(\\n\\t)

message($$QMAKE_POST_LINK)

