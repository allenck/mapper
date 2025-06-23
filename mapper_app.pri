SOURCES += main.cpp\
    $$PWD/browsecommentsdialog.cpp \
    $$PWD/lineeditdelegate.cpp \
    $$PWD/logger.cpp \
    $$PWD/mytextedit.cpp \
    $$PWD/overlay.cpp \
    $$PWD/queryeditmodel.cpp \
    $$PWD/replacesegmentdialog.cpp \
    $$PWD/routeselector.cpp \
    $$PWD/rtitemdelegate.cpp \
    $$PWD/segmentselectionwidget.cpp \
    $$PWD/splitsegmentdlg.cpp \
    $$PWD/ttitemdelegate.cpp \
    $$PWD/turncombo.cpp \
    $$PWD/turndelegate.cpp \
    addgeoreferenceddialog.cpp \
    $$PWD/exceptions.cpp \
    $$PWD/myapplication.cpp \
    mainwindow.cpp \
    webviewbridge.cpp \
    sql.cpp \
    data.cpp \
    routeview.cpp \
    routeviewtablemodel.cpp \
    routeviewsortproxymodel.cpp \
    configuration.cpp \
    dialogcopyroute.cpp \
    dialogrenameroute.cpp \
    routedlg.cpp \
    segmentdlg.cpp \
    segmentdescription.cpp \
    splitroute.cpp \
    modifyroutedatedlg.cpp \
    segmentview.cpp \
    segmentviewtablemodel.cpp \
    segmentviewsortproxymodel.cpp \
    otherrouteview.cpp \
    filedownloader.cpp \
    editstation.cpp \
    editcomments.cpp \
    stationview.cpp \
    companyview.cpp \
    tractiontypeview.cpp \
    exportsql.cpp \
    exportdlg.cpp \
    editconnectionsdlg.cpp \
    geodbsql.cpp \
    locatestreetdlg.cpp \
    dupsegmentview.cpp \
    combineroutesdlg.cpp \
    routecommentsdlg.cpp \
    reroutingdlg.cpp \
    createsqlitedatabasedialog.cpp \
    checkroute.cpp \
    settingsdb.cpp \
    querydialog.cpp \
    querymodel.cpp \
    kml.cpp \
    exportroutedialog.cpp \
    editcitydialog.cpp \
    htmltextedit.cpp \
    $$PWD/mapview.cpp \
    editsegmentdialog.cpp \
    $$PWD/overlaytablemodel.cpp \
    $$PWD/htmldelegate.cpp \
    $$PWD/city.cpp \
    $$PWD/connection.cpp \
    $$PWD/latlng.cpp \
    $$PWD/globalmercator.cpp \
    $$PWD/consoleinterface.cpp \
    systemconsole.cpp \
    systemconsoleaction.cpp \
    jtextarea.cpp \
    flowlayout.cpp \
    jtogglebutton.cpp

HEADERS  += mainwindow.h \
    $$PWD/browsecommentsdialog.h \
    $$PWD/lineeditdelegate.h \
    $$PWD/logger.h \
    $$PWD/mytextedit.h \
    $$PWD/overlay.h \
    $$PWD/queryeditmodel.h \
    $$PWD/replacesegmentdialog.h \
    $$PWD/routeselector.h \
    $$PWD/rtitemdelegate.h \
    $$PWD/segmentselectionwidget.h \
    $$PWD/splitsegmentdlg.h \
    $$PWD/ttitemdelegate.h \
    $$PWD/turncombo.h \
    $$PWD/turndelegate.h \
    $$PWD/vptr.h \
    addgeoreferenceddialog.h \
    $$PWD/exceptions.h \
    $$PWD/myapplication.h \
    webviewbridge.h \
    sql.h \
    data.h \
    routeviewtablemodel.h \
    routeviewsortproxymodel.h \
    routeview.h \
    configuration.h \
    dialogcopyroute.h \
    dialogrenameroute.h \
    routedlg.h \
    segmentdlg.h \
    segmentdescription.h \
    ccombobox.h \
    splitroute.h \
    modifyroutedatedlg.h \
    segmentview.h \
    segmentviewtablemodel.h \
    segmentviewsortproxymodel.h \
    otherrouteview.h \
    filedownloader.h \
    editstation.h \
    editcomments.h \
    stationview.h \
    companyview.h \
    tractiontypeview.h \
    exportsql.h \
    exportdlg.h \
    editconnectionsdlg.h \
    geodbsql.h \
    locatestreetdlg.h \
    dupsegmentview.h \
    combineroutesdlg.h \
    routecommentsdlg.h \
    reroutingdlg.h \
    createsqlitedatabasedialog.h \
    checkroute.h \
    settingsdb.h \
    querydialog.h \
    querymodel.h \
    kml.h \
    exportroutedialog.h \
    editcitydialog.h \
    htmltextedit.h \
    $$PWD/mapview.h \
    editsegmentdialog.h \
    $$PWD/overlaytablemodel.h \
    $$PWD/htmldelegate.h \
    $$PWD/city.h \
    $$PWD/connection.h \
    $$PWD/latlng.h \
    $$PWD/globalmercator.h \
    consoleinterface.h \
    systemconsole.h \
    systemconsoleaction.h \
    jtextarea.h \
    flowlayout.h \
    jtogglebutton.h \
    preferencespanel.h

HEADERS += \
        dialogchangeroute.h \
        mymessagebox.h \
        modifyroutetractiontypedlg.h \
        newcitydialog.h \
        removecitydialog.h \
        websocketclientwrapper.h \
        websockettransport.h

    SOURCES += \
        dialogchangeroute.cpp \
        mymessagebox.cpp \
        modifyroutetractiontypedlg.cpp \
        newcitydialog.cpp \
        removecitydialog.cpp \
        websocketclientwrapper.cpp \
        websockettransport.cpp

FORMS += \
     ui/dialogchangeroute.ui \
     ui/modifyroutetractiontypedlg.ui \
     ui/newcitydialog.ui \
     ui/removecitydialog.ui

FORMS    += ui/mainwindow.ui \
    $$PWD/ui/splitsegmentdlg.ui \
    ui/addgeoreferenceddialog.ui \
    ui/dialogcopyroute.ui \
    ui/dialogrenameroute.ui \
    ui/routedlg.ui \
    ui/segmentdlg.ui \
    ui/splitroute.ui \
    ui/modifyroutedatedlg.ui \
    ui/editstation.ui \
    ui/exportdlg.ui \
    ui/editconnectionsdlg.ui \
    ui/locatestreetdlg.ui \
    ui/combineroutesdlg.ui \
    ui/routecommentsdlg.ui \
    ui/reroutingdlg.ui \
    ui/createsqlitedatabasedialog.ui \
    ui/querydialog.ui \
    ui/exportroutedialog.ui \
    ui/editcitydialog.ui \
    ui/editsegmentdialog.ui \
    ui/editsegmentdialog.ui \
    ui/browsecommentsdialog.ui \
    ui/replacesegmentdialog.ui \
    ui/segmentselectionwidget.ui

DISTFILES += \
 $$PWD/html/.gitignore
