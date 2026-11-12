# 1. Pull the raw environment path
set(RAW_QT_DIR "$ENV{QTDIR}")

# 2. Force convert any Windows backslashes into standard CMake forward slashes
cmake_path(CONVERT "${RAW_QT_DIR}" TO_CMAKE_PATH_LIST CLEAN_QT_DIR)

# 3. Create clean paths for the source file and include directory
set(SQLITE3_SOURCE "${CLEAN_QT_DIR}/../Src/qtbase/src/3rdparty/sqlite/sqlite3.c")
set(SQLITE3_INCLUDE "${CLEAN_QT_DIR}/../Src/qtbase/src/3rdparty/sqlite")

# 4. Declare the target and inject the sanitized paths
qt_add_executable(mapper WIN32 MACOSX_BUNDLE
#    $ENV{QTDIR}/../Src/qtbase/src/3rdparty/sqlite/sqlite3.c $ENV{QTDIR}/../Src/qtbase/src/3rdparty/sqlite/sqlite3.h
    ${SQLITE3_SOURCE}
    QtKeychain/qtkeychain/gnomekeyring_p.h
    QtKeychain/qtkeychain/keychain.cpp QtKeychain/qtkeychain/keychain.h QtKeychain/qtkeychain/keychain_p.h
    QtKeychain/qtkeychain/libsecret_p.h
    QtKeychain/qtkeychain/plaintextstore_p.h
    addgeoreferenceddialog.cpp addgeoreferenceddialog.h
    browsecommentsdialog.cpp browsecommentsdialog.h
    ccombobox.h
    checkroute.cpp checkroute.h
    city.cpp city.h
    clickablelabel.cpp clickablelabel.h
    clipboard.cpp clipboard.h
    combineroutesdlg.cpp combineroutesdlg.h
    companyview.cpp companyview.h
    configuration.cpp configuration.h
    connection.cpp connection.h
    createsqlitedatabasedialog.cpp createsqlitedatabasedialog.h
    data.cpp data.h
    dateeditdelegate.cpp dateeditdelegate.h
    dialogchangeroute.cpp dialogchangeroute.h
    dialogcopyroute.cpp dialogcopyroute.h
    dialogeditcomments.cpp dialogeditcomments.h
    dialogeditparameters.cpp dialogeditparameters.h
    dialogeditstreets.cpp dialogeditstreets.h
    dialoghtmledit.cpp dialoghtmledit.h
    dialognewconnection.cpp dialognewconnection.h
    dialogpreferences.cpp dialogpreferences.h
    dialogselectlist.cpp dialogselectlist.h
    dialogtextedit.cpp dialogtextedit.h
    dialogupdatestreets.cpp dialogupdatestreets.h
    dsn.cpp dsn.h
    dupsegmentview.cpp dupsegmentview.h
    editcitydialog.cpp editcitydialog.h
    editcomments.cpp editcomments.h
    editconnectionsdlg.cpp editconnectionsdlg.h
    editsegmentdescr.cpp editsegmentdescr.h
    editsegmentdialog.cpp editsegmentdialog.h
    editstation.cpp editstation.h
    exceptions.h
    exportdlg.cpp exportdlg.h
    exportroutedialog.cpp exportroutedialog.h
    exportsql.cpp exportsql.h
    filedownloader.cpp filedownloader.h
    findreplacewidget.cpp findreplacewidget.h
    flowlayout.cpp flowlayout.h
    geodbsql.cpp geodbsql.h
    geoserver.cpp geoserver.h
    globalmercator.cpp globalmercator.h
    htmldelegate.cpp htmldelegate.h
    htmltextedit.cpp htmltextedit.h
    itemdelegate.cpp itemdelegate.h
    kml.cpp kml.h
    latlng.cpp latlng.h
    lineeditdelegate.cpp lineeditdelegate.h
    locatestreetdlg.cpp locatestreetdlg.h
    logger.cpp logger.h
    main.cpp
    mainwindow.cpp mainwindow.h
    mapview.cpp mapview.h
    modifyroutedatedlg.cpp modifyroutedatedlg.h
    modifyroutedialog.cpp modifyroutedialog.h
    modifyroutetractiontypedlg.cpp modifyroutetractiontypedlg.h
    myapplication.cpp myapplication.h
    mymessagebox.cpp mymessagebox.h
    mytextedit.cpp mytextedit.h
    newcitydialog.cpp newcitydialog.h
    odbcutil.cpp odbcutil.h
    otherrouteview.cpp otherrouteview.h
    overlay.cpp overlay.h
    overlaytablemodel.cpp overlaytablemodel.h
    preferencespanel.h
    psql.cpp psql.h
    querydialog.cpp querydialog.h
    queryeditmodel.cpp queryeditmodel.h
    querymodel.cpp querymodel.h
    removecitydialog.cpp removecitydialog.h
    replacesegmentdialog.cpp replacesegmentdialog.h
    reroutingdlg.cpp reroutingdlg.h
    routecommentsdlg.cpp routecommentsdlg.h
    routedlg.cpp routedlg.h
    routenamewidget.cpp routenamewidget.h
    routeselector.cpp routeselector.h
    routeview.cpp routeview.h
    routeviewsortproxymodel.cpp routeviewsortproxymodel.h
    routeviewtablemodel.cpp routeviewtablemodel.h
    rtitemdelegate.cpp rtitemdelegate.h
    securevault.cpp securevault.h
    segmentdescription.cpp segmentdescription.h
    segmentdlg.cpp segmentdlg.h
    segmentselectionwidget.cpp segmentselectionwidget.h
    segmentview.cpp segmentview.h
    segmentviewsortproxymodel.cpp segmentviewsortproxymodel.h
    segmentviewtablemodel.cpp segmentviewtablemodel.h
    splitcompanyroutesdialog.cpp splitcompanyroutesdialog.h
    splitroute.cpp splitroute.h
    splitsegmentdlg.cpp splitsegmentdlg.h
    sql.cpp sql.h
    stationview.cpp stationview.h
    streetstablemodel.cpp streetstablemodel.h
    streetview.cpp streetview.h
    systemconsole2.cpp systemconsole2.h
    tractiontypeview.cpp tractiontypeview.h
    ttitemdelegate.cpp ttitemdelegate.h
    turncombo.cpp turncombo.h
    turndelegate.cpp turndelegate.h
    addgeoreferenceddialog.ui
    browsecommentsdialog.ui
    combineroutesdlg.ui
    createsqlitedatabasedialog.ui
    dialogchangeroute.ui
    dialogcopyroute.ui
    dialogeditcomments.ui
    dialogeditparameters.ui
    dialogeditstreets.ui
    dialoghtmledit.ui
    dialognewconnection.ui
    dialogpreferences.ui
    dialogselectlist.ui
    dialogtextedit.ui
    dialogupdatestreets.ui
    editcitydialog.ui
    editconnectionsdlg.ui
    editsegmentdialog.ui
    editstation.ui
    exportdlg.ui
    exportroutedialog.ui
    findreplacewidget.ui
    locatestreetdlg.ui
    mainwindow.ui
    modifyroutedatedlg.ui
    modifyroutedialog.ui
    modifyroutetractiontypedlg.ui
    newcitydialog.ui
    newquerydialog.ui
    querydialog.ui
    removecitydialog.ui
    replacesegmentdialog.ui
    reroutingdlg.ui
    routecommentsdlg.ui
    routedlg.ui
    routenamewidget.ui
    segmentdlg.ui
    segmentselectionwidget.ui
    splitcompanyroutesdialog.ui
    splitroute.ui
    splitsegmentdlg.ui
    systemconsole2.ui
    usagedelegate.cpp usagedelegate.h
    vptr.h
    websocketclientwrapper.cpp websocketclientwrapper.h
    websockettransport.cpp websockettransport.h
    webviewbridge.cpp webviewbridge.h
    mapper.qrc
)
# Force the executable to be built in the root build folder, matching qmake's directory depth
set_target_properties(mapper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
)

target_include_directories(mapper PRIVATE
    $ENV{QTDIR}/../Src/qtbase/src/3rdparty/sqlite
    .
    QtKeychain
    ${Qt6WebEngineWidgets_INCLUDE_DIRS}
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

target_compile_definitions(mapper PRIVATE
    BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    HAVE_CONSOLE
    MY_VERSION="\\\"1.3.0\\\""
    QTKEYCHAIN_NO_EXPORT
    QTKEYCHAIN_VERSION_6
    QT_MESSAGELOGCONTEXT
    USE_WEBENGINE
    VER_BUILD=4
    VER_MAJOR=1
    VER_MINOR=2
    VER_PATCH=3
    VER_STRING="\\\"1.2.12\\\""
)

target_link_libraries(mapper PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Sql
    Qt::WebChannel
    Qt::WebSockets
    Qt::Widgets
    Qt::Xml
    Qt::WebEngineWidgets
    Qt::Concurrent
    Qt::PrintSupport
    # Add this flag to tell the linker to bypass the two missing KWallet symbols:
    "-Wl,--unresolved-symbols=ignore-in-object-files"
)


# Resources:
set(mapper_resource_files
    "Resources/ExtDraggableObject.js"
    "Resources/GoogleMaps.js"
    "Resources/GoogleMaps2.htm"
    "Resources/GoogleMaps2b.htm"
    "Resources/GoogleMapsA.js"
    "Resources/MapQuest.htm"
    "Resources/OpenStreetMap.htm"
    "Resources/S-Bahn-Logo.svg"
    "Resources/Strassenbahn-Haltestelle.svg"
    "Resources/U-Bahn.svg"
    "Resources/WebChannel.js"
    "Resources/apikey.js"
    "Resources/arrow-left-double.png"
    "Resources/arrow-left.png"
    "Resources/arrow-right-double.png"
    "Resources/arrow-right.png"
    "Resources/blue-red-blank.png"
    "Resources/blueblank.png"
    "Resources/contextmenu.js"
    "Resources/darkred-dot.png"
    "Resources/diropen.png"
    "Resources/dump_databases.bat"
    "Resources/dump_databases.cmd"
    "Resources/dump_databases.sh"
    "Resources/edit-cut.png"
    "Resources/edit-delete.png"
    "Resources/fileload.png"
    "Resources/filesave.png"
    "Resources/format-remove-node.png"
    "Resources/go-first-view.png"
    "Resources/go-first.png"
    "Resources/go-last-view.png"
    "Resources/go-last.png"
    "Resources/go-next.png"
    "Resources/go-previous.png"
    "Resources/go.png"
    "Resources/green00.png"
    "Resources/greenblank.png"
    "Resources/gtk-add.png"
    "Resources/gtk-delete.png"
    "Resources/hidden.png"
    "Resources/list-add.png"
    "Resources/mapperLeaflet.css"
    "Resources/mapperLeaflet.js"
    "Resources/opacity-slider2.png"
    "Resources/opacityControl.js"
    "Resources/orange.png"
    "Resources/overlays.xml"
    "Resources/qwebchannel.js"
    "Resources/rail.png"
    "Resources/red00.png"
    "Resources/redblank.png"
    "Resources/reload.png"
    "Resources/restore_databases.bat"
    "Resources/restore_databases.cmd"
    "Resources/restore_databases.sh"
    "Resources/rollback.png"
    "Resources/scripts/L.RotatedMarker.js"
    "Resources/scripts/L.Symbol.js"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/Control.MagnifyingGlass.css"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/Control.MagnifyingGlass.js"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/examples/Control.MagnifyingGlass.css"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/examples/example_multi.html"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/leaflet.magnifyingglass.css"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/leaflet.magnifyingglass.js"
    "Resources/scripts/bbecquet/LeafletMagnifyingGlass/magnifying-glass-icon.png"
    "Resources/scripts/bbecquet/dist/leaflet.polylineDecorator.js"
    "Resources/scripts/leaflet.polylineDecorator.js"
    "Resources/scripts/leaflet.polylineoffset.js"
    "Resources/scripts/masajid390/leaflet-beautify-marker-icon.css"
    "Resources/scripts/masajid390/leaflet-beautify-marker-icon.js"
    "Resources/scripts/patternUtils.js"
    "Resources/show-password.png"
    "Resources/sl-metro-logo.svg"
    "Resources/sql/MsSql/CreateMsSqlFunction.sql"
    "Resources/sql/MsSql/mssql_create_routeName.sql"
    "Resources/sql/MsSql/mssql_create_routeView.sql"
    "Resources/sql/MsSql/mssql_create_routes.sql"
    "Resources/sql/MsSql/mssql_create_stations.sql"
    "Resources/sql/MsSql/mssql_create_streetdef.sql"
    "Resources/sql/MsSql/mssql_recreateStationTable.sql"
    "Resources/sql/MsSql/mssql_recreate_routes.sql"
    "Resources/sql/MySql/CreateMySqlFunction.sql"
    "Resources/sql/MySql/mySql_createDatabase.sql"
    "Resources/sql/MySql/mySql_create_routeName.sql"
    "Resources/sql/MySql/mySql_recreateSegmentsTable.sql"
    "Resources/sql/MySql/mysql_create_routeView.sql"
    "Resources/sql/MySql/mysql_create_routes.sql"
    "Resources/sql/MySql/mysql_create_stations.sql"
    "Resources/sql/MySql/mysql_create_streetdef.sql"
    "Resources/sql/MySql/mysql_recreateCompanies.sql"
    "Resources/sql/MySql/mysql_recreateRouteComments.sql"
    "Resources/sql/MySql/mysql_recreate_routes.sql"
    "Resources/sql/PostgreSQL/PostgreSQL_create_distance_function.sql"
    "Resources/sql/PostgreSQL/createPostgreSQLFunction.sql"
    "Resources/sql/PostgreSQL/dropPostgreSQLFunction.sql"
    "Resources/sql/PostgreSQL/replacePostgreSQLFunction"
    "Resources/sql/Sqlite3/sqlite3_create_routes.sql"
    "Resources/sql/Sqlite3/sqlite3_create_streetdef.sql"
    "Resources/sql/Sqlite3/sqlite3_create_tables.sql"
    "Resources/sql/Sqlite3/sqlite3_recreateAltRoute.sql"
    "Resources/sql/Sqlite3/sqlite3_recreateComments.sql"
    "Resources/sql/Sqlite3/sqlite3_recreateCompanies.sql"
    "Resources/sql/Sqlite3/sqlite3_recreateSegmentsTable.sql"
    "Resources/sql/Sqlite3/sqlite3_recreate_parameters.sql"
    "Resources/sql/Sqlite3/sqlite3_recreate_routeName.sql"
    "Resources/sql/Sqlite3/sqlite3_recreate_routes.sql"
    "Resources/sql/addBiDirectionalToRoutes.sql"
    "Resources/sql/create_routeName.sql"
    "Resources/sql/create_routeView.sql"
    "Resources/sql/create_stations.sql"
    "Resources/sql/recreateRouteComments.sql"
    "Resources/sql/recreateStationTable.sql"
    "Resources/sql/recreateTractionTypes.sql"
    "Resources/sql/recreate_routeName.sql"
    "Resources/sql/recreate_streetdef.sql"
    "Resources/subway.png"
    "Resources/tram-icon.ico"
    "Resources/tram.png"
    "Resources/trash.png"
    "Resources/white.png"
    "Resources/yellowblank.png"
)

qt_add_resources(mapper "mapper"
    PREFIX
        "/"
    BASE
        "Resources"
    FILES
        ${mapper_resource_files}
)

if((QT_VERSION_MAJOR EQUAL 6))
    target_link_libraries(mapper PRIVATE
        Qt::Concurrent
        Qt::Gui
        Qt::PrintSupport
        Qt::WebEngineWidgets
        Qt::Widgets
    )
endif()

if(debug)
    target_compile_definitions(mapper PRIVATE
        MYPREFIX_DEBUG
    )
endif()

if(release)
    target_compile_definitions(mapper PRIVATE
        MYPREFIX_RELEASE
    )
endif()

if(MACOS)
    target_sources(mapper PUBLIC
        QtKeychain/qtkeychain/keychain_apple.mm
    )

    target_link_libraries(mapper PRIVATE
        "-framework CoreFoundation"
        "-framework Security"
    )
endif()

if(UNIX)
    target_link_libraries(mapper PRIVATE
        dl
    )
endif()

if(WIN32)
    target_sources(mapper PUBLIC
        QtKeychain/qtkeychain/keychain_win.cpp
        QtKeychain/qtkeychain/plaintextstore.cpp
        registrywatcher.h
    )

    target_link_libraries(mapper PRIVATE
        advapi32
        crypt32
    )
endif()

if(UNIX AND NOT MACOS)
    target_sources(mapper PUBLIC
        QtKeychain/qtkeychain/gnomekeyring.cpp
        QtKeychain/qtkeychain/keychain_unix.cpp
        QtKeychain/qtkeychain/libsecret.cpp
        QtKeychain/qtkeychain/plaintextstore.cpp
        QtKeychain/qtkeychain/kwallet_interface.cpp
)

    target_compile_definitions(mapper PRIVATE
        KEYCHAIN_DBUS
        WITHOUT_KWALLET
)

    target_link_libraries(mapper PRIVATE
        Qt::DBus
    )
endif()

install(TARGETS mapper
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ==============================================================================
# Automated Production Deployment & Complete Installer Compiler Pipeline
# ==============================================================================

if(QT_VERSION_MAJOR EQUAL 6)
    # 1. Define staging areas, packages directories, and configuration paths
    set(DEPLOY_STAGE_DIR "${CMAKE_BINARY_DIR}/deploy_package")
    set(IFW_ROOT_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/../installer")
    set(IFW_PACKAGES_DIR "${IFW_ROOT_DIR}/packages")

    # 2. Automatically locate the Qt Installer Framework tools on Windows
    set(ARCHIVEGEN_EXE "C:/Qt/Tools/QtInstallerFramework/4.5/bin/archivegen.exe")
    set(BINCREATOR_EXE "C:/Qt/Tools/QtInstallerFramework/4.5/bin/binarycreator.exe")

    if(NOT EXISTS "${ARCHIVEGEN_EXE}")
        # Dynamic fallback search if the version path shifts slightly
        file(GLOB IFW_ARCH_PATHS "C:/Qt/Tools/QtInstallerFramework/*/bin/archivegen.exe")
        list(GET IFW_ARCH_PATHS 0 ARCHIVEGEN_EXE)
        file(GLOB IFW_BIN_PATHS "C:/Qt/Tools/QtInstallerFramework/*/bin/binarycreator.exe")
        list(GET IFW_BIN_PATHS 0 BINCREATOR_EXE)
    endif()

    # 3. Generate the foundational deployment script (windeployqt orchestrator)
    qt_generate_deploy_app_script(
        TARGET mapper
        FILENAME_VARIABLE deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    # 4. Define the complete unified compiler pipeline target
    add_custom_target(deploy_app
        # --- PHASE 1: Clean and Prepare Windows Staging Directories ---
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${DEPLOY_STAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_STAGE_DIR}"

        # --- PHASE 2: Core Binary & Dependency Extraction ---
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mapper> "${DEPLOY_STAGE_DIR}/"
        COMMAND ${CMAKE_COMMAND} -P "${deploy_script}"

        # --- PHASE 3: Context Asset Aggregation ---
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" "${DEPLOY_STAGE_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/Resources/tram-icon.ico" "${DEPLOY_STAGE_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources" "${DEPLOY_STAGE_DIR}/Resources"

        # --- PHASE 4: Pack com.vendor.product (Base Archive) ---
        # Exclude SQLite databases from the base archive so they remain distinct splits
        COMMAND ${CMAKE_COMMAND} -E remove -f "${DEPLOY_STAGE_DIR}/Resources/databases/*.sqlite3"
        COMMAND "${ARCHIVEGEN_EXE}" "${IFW_PACKAGES_DIR}/com.vendor.product/data/mapper_base.7z" "${DEPLOY_STAGE_DIR}/*"

        # --- PHASE 5: Pack com.vendor.databases (Separated DB Archive) ---
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${DEPLOY_STAGE_DIR}/db_staged"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_STAGE_DIR}/db_staged"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Resources/databases" "${DEPLOY_STAGE_DIR}/db_staged"
        COMMAND "${ARCHIVEGEN_EXE}" "${IFW_PACKAGES_DIR}/com.vendor.databases/data/mapper_databases.7z" "${DEPLOY_STAGE_DIR}/db_staged/*"

        # --- PHASE 6: Compile Finalized Setup Installer File ---
        COMMAND "${BINCREATOR_EXE}" -c "${IFW_ROOT_DIR}/config/config.xml" -p "${IFW_PACKAGES_DIR}" "${CMAKE_BINARY_DIR}/MapperInstaller.exe"

        # --- PHASE 7: Post-Build Workspace Housekeeping ---
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${DEPLOY_STAGE_DIR}"

        COMMENT "Executing compilation pipeline: Bundling app dependencies, packing .7z modules, and building MapperInstaller.exe..."
        VERBATIM
    )
endif()
