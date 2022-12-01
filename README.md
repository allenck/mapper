# mapper
Qt Application to maintain a database of historic transit routes. With databases for St. Louis, Louisville, KY, Cincinnati OH, Berlin, Germany

![Alt text](wiki/images/Berlin_screenshot.png?raw=true "Berlin Germany")

Mapper allows the user to store data describing to the routes of historic streetcar and other transit routes. 
By default, the data is stored in a SQLite3 database. Alternatively, route information can be stored in a
SQL Server or MySql database and can be imported an/or exported from one to another. The database includes 
effective dates for components of a route so that the course of a route at any point in time can be displayed.

Route data is displayed overlayed on a Google Maps or OpenStreetMap map. The displayed map can either be 
displayed in the app's window or in a browser such as Firefox. The program can also overlay the display with
overlays created by a map tiling utility. This, for example allows historic USGS topographic maps or others
to be used as overlays. 

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
