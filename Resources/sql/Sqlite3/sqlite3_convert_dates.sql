BEGIN;
UPDATE Parameters SET minDate = REPLACE(minDate, '/', '-') WHERE minDate LIKE '____/__/__';
UPDATE Parameters SET maxDate = REPLACE(maxDate, '/', '-') WHERE maxDate LIKE '____/__/__';

UPDATE Comments SET date = REPLACE(date, '/', '-') WHERE date LIKE '____/__/__';

UPDATE Companies SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE Companies SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE Terminals SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE Terminals SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE RouteSeq SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE RouteSeq SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE Stations SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE Stations SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE StreetDef SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE StreetDef SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE Routes SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE Routes SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE Segments SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE Segments SET doubleDate = REPLACE(doubleDate, '/', '-') WHERE doubleDate LIKE '____/__/__';
UPDATE Segments SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE RouteName SET startDate = REPLACE(startDate, '/', '-') WHERE startDate LIKE '____/__/__';
UPDATE RouteName SET endDate = REPLACE(endDate, '/', '-') WHERE endDate LIKE '____/__/__';

UPDATE Parameters SET dateFormat = 'yyyy-MM-dd';
COMMIT;
