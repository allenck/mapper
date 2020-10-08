# mapper
Qt Application to maintain a database of historic transit routes. With databases for St. Louis, Louisville, KY, Cincinnati OH, Berlin, Germany

![Alt text](wiki/images/Berlin_screenshot.png?raw=true "Berlin Germany")

Mapper allows the user to store data describing to the routes of historic streetcar and other transit routes. By default, the data is stored in a SQLite3 database. Alternatively, route information can be stored in a SQL Sever or MySql database and can be imported an/or exported from one to another. The database includes effective dates for components of a route so that the course of a route at any point in time can be displayed.

Route data is displayed overlayed on a Google Maps or OpenStreetMap map. The displayed map can either be displayed in the app's window or in a browser such as Firefox. The program can also overlay the display with overlays created by a map tiling utility. This, for example allows historic USGS topographic maps or others to be used as overlays. 

