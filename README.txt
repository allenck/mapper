**Recent Changes September 2025 VERSION=1.2.3
1. generate new connection description.
2. close export dialog when finished.
3. fix mysql create function script.

**Recent Changes July 2025 VERSION=1.2.2
1. Detect if connection to browser has failed.
2. Corrections to edit connections dialog.
3. Show status of browser connection in statusbar.
4. fix display of route length when displaying route.

**Recent Changes May 2025 VERSION=1.2.1
1. Corrections to Route Comments sql.

**Recent Changes January 2025 VERSION=1.2.0
1. Add support for PostgreSQL database.
2. Wiki changes to add PostgreSQL support.

**Recent Changes January 2025 VERSION=1.1.5
1. Segments table has new field: "NewerName" which is the current name of a street. This and
   the location field have been added to the main window's edit segment section.
2. A new table "StreetDef" contains information about current and historic street names. This
   information is displayed in a new "Streets" tab in the main window.
3. A new table, "RouteName" has been created to store Route names with the intention of eventually removing
   the route name from each Route table entry and replacing it with the integer key to the RouteName table.
4. New scripts have been added to dump and restore SQLite databases to/from text files.
5. The Companies table has had a new field added, 'Url' containing a url of a web page about the company.


**Recent Changes September-December 2024 VERSION=1.1.4
1. Additional routes for Berlin as of 1901 added based on information from
   https://www.berliner-linienchronik.de/lv-1901.html
2. Implement feature where only routes for selected companies are showm in routes combobox.
3. Implement clipboard history for selected fields.
4. Add mnemonic field to Companies table.
5. Add logic to maintain lists of common street abbreviations. Add reformatting of segment descriptions
   and display background colors for invalid or questionable segment descriptions.

**Recent Changes April 2024 VERSION=1.1.3
1. Fix ModifyTractionTypeDlg.
2. Fix RouteViewTbleModel selecting wrong segment. Redisplay segment after changes.

**Recent Changes March 2024
1.  Add option to show/hide arrows on two way segments.

**Recent Changes January 2024
1.  A new column, 'DoubleDate' has been added to the Segments table. This date can be set via the edit
    segment dialog. The date defines the date upon which a segment was converted to double track. This
    allows a segment to be displayed as single track before the DoubleDate. Therefore, it is not
    necessary to have 1 track and 2 track segments defined for the same section of a route. Single track
    segments can be converted to 2 track segments.
2.  The Edit Segment dialog will now combine duplicate segments, applying the oldest (lowest segment
    number) to all affected routes.
3.  More routes before 1900 for Indianapolis have been added.

**Recent Changes November/December 2023**
1.  Change the way that User Defined Functions are implemented. No longer use sqlite3_enable_load_extension
    and a recompiled Qt sqlite plugin. Now Mapper uses the sqlite3_create_function to define the
    distance function.
2.  Extensive revision of internal classes to combine properties.
3.  Improved the Edit Connections dialog.
4.  Fixed nunerous bugs.
5.  Add menu actions to ui combo boxes.
6.  Add a new City, Indianapolis,IN to the collection of city databases.
7.  Add option to turn off distracting Google Maps icons.
8.  Segments now have a Location property useful for differentiating street names that duplicate those
    in other locations.
9.  Changed compile and link to eliminate warnings when QWebEngineView is used.
10. Added additional sort options for routes in the route combobox.
11. Added some information on routes in Berlin during 1941-1945 and recent route changres and extensions.
12. Updated wiki pages with current screenshots.
13. The Route Segments tab now has table columns that can be moved and/or hidden.

**Recent Changes November 2022**
1.  Segments no longer have a 'one-way' attribute. This has been applied instead to routes. With this
    change routes using only one of the two tracks in a segment.
2.  Segment selection has been improved an can now be done by street.
3.  Routes using only one of two tracks display as gray for thw unused track.
4.  The procedures for adding an overlay have been changed and documented in the wiki. This in particular,
    the process involves processing the link to a WMTS get capabilities request and then determining
    setup info from the XML file from the map server.
5.  Map displays now will pop up the Latitude and longitude. Clicking on the popup will post the data
    to the clipboard.
6.  Mapper now sets up by default, city connections for the four cities(St Louis, Louisville, Cincinnati
    and Berlin).
7.  Route comments can now be specified to be applied to more than one route and can contain HTML text including
    links to web sites.
8.  The St. Louis database includes new information on routes built before the 1899 merger of most
    St. Louis streetcar companies.
9.  The program now defines a number of predefined overlays.
10. Wiki pages have been updated.
11. Console messages are now stored in a 'mapper.log' file.

** Version 1.0.1 Changes December 2022**
1.  Add screenshot capture for browser display.
